using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Models;
using WpfInspectionApp.Services;

namespace WpfInspectionApp.Diagnostics;

// Headless smoke test that exercises Align/ShapeX/PadBW native bridges using
// the default 2D image and synthetic Window/Algorithm data. Used by the
// `--smoke-test` CLI argument and CI to detect regressions without UI.
internal static class SmokeTestRunner
{
    public static int Run(AppServices services)
    {
        var log = new System.Text.StringBuilder();
        void Write(string message)
        {
            DiagnosticsLog.Write($"[SMOKE] {message}");
            log.AppendLine(message);
        }

        try
        {
            Write("smoke test starting");

            // Use a small synthetic BGRA image with a clear checkerboard pattern.
            // This avoids the heavy 15.8M pixel default JPG and isolates bridge correctness.
            const int width = 256;
            const int height = 256;
            const int stride = width * 4;
            var pixels = CreateSyntheticImage(width, height);
            Write($"synthetic image {width}x{height} prepared");

            var model = CreateSyntheticModel(width, height);
            Write("model built");
            var runtimeImage = new PartRuntimeImage(pixels, width, height, stride, model.Threshold2D);
            Write("runtime image built");

            // Call each bridge directly to isolate any hang
            Write("calling MptiNativeBridge.GetVersion (sanity check DLL load)...");
            var verSw = System.Diagnostics.Stopwatch.StartNew();
            var verResp = Interop.MptiNativeBridge.GetVersion();
            verSw.Stop();
            Write($"GetVersion ({verSw.ElapsedMilliseconds}ms): available={verResp.Available} success={verResp.Success} message={verResp.Message}");

            Write("calling MptiAlignNativeBridge.Run directly (sync)...");
            var alignSw = System.Diagnostics.Stopwatch.StartNew();
            var alignParams = new Interop.MptiBridgeAlignParams
            {
                SearchNum = 4,
                SearchPointsX = new[] { 64, 192, 64, 192 },
                SearchPointsY = new[] { 64, 64, 192, 192 },
                SearchSizeW = new[] { 40, 40, 40, 40 },
                SearchSizeH = new[] { 40, 40, 40, 40 },
                SearchMargin = 5,
                MinBinary = 128,
                MaxBinary = 255,
                UseInsp2D = 1,
                InvertCheck = 0,
                UseShift = 1,
                MaxShiftX = 10,
                MaxShiftY = 10,
                UseAngle = 1,
                MaxAngle = 5,
                SameSize = 1,
                MinBlobArea = 5
            };
            var alignResp = Interop.MptiAlignNativeBridge.Run(pixels, width, height, stride, new RoiRect(0, 0, width, height), alignParams);
            alignSw.Stop();
            Write($"Align bridge ({alignSw.ElapsedMilliseconds}ms): available={alignResp.Available} success={alignResp.Success} okCount={alignResp.OkCount} message={alignResp.Message}");

            Write("calling MptiAlgorithmNativeBridge.RunShapeX...");
            var shapeXParams = new Interop.MptiBridgeShapeXParams
            {
                BinaryMin = 128, BinaryMax = 255, UseInsp2D = 1, UseShape = 1, UseExist = 1, UseShift = 1,
                ShapeAreaMin = 0.05f, ShapeAreaMax = 0.95f, ShiftXTolerance = 10, ShiftYTolerance = 10,
                ExpectedCenterX = 128, ExpectedCenterY = 128, MinBlobArea = 5
            };
            var shapeXSw = System.Diagnostics.Stopwatch.StartNew();
            var shapeXResp = Interop.MptiAlgorithmNativeBridge.RunShapeX(pixels, width, height, stride, new RoiRect(50, 50, 156, 156), shapeXParams);
            shapeXSw.Stop();
            Write($"ShapeX bridge ({shapeXSw.ElapsedMilliseconds}ms): available={shapeXResp.Available} success={shapeXResp.Success} isOK={shapeXResp.IsOK} areaRate={shapeXResp.AreaRate:F3} message={shapeXResp.Message}");

            Write("calling MptiAlgorithmNativeBridge.RunPadBW...");
            var padBWParams = new Interop.MptiBridgePadBWParams
            {
                BinaryMin = 128, BinaryMax = 255, UseInsp2D = 1, UseTeachArea = 1,
                TeachArea = 1000, TeachAreaRateMin = 80, TeachAreaRateMax = 120,
                UseShift = 1, TeachShiftX = 10, TeachShiftY = 10,
                ExpectedCenterX = 128, ExpectedCenterY = 128, UseBlobArea = 1, BlobAreaMin = 50,
                MinBlobArea = 5
            };
            var padBWSw = System.Diagnostics.Stopwatch.StartNew();
            var padBWResp = Interop.MptiAlgorithmNativeBridge.RunPadBW(pixels, width, height, stride, new RoiRect(50, 50, 156, 156), padBWParams);
            padBWSw.Stop();
            Write($"PadBW bridge ({padBWSw.ElapsedMilliseconds}ms): available={padBWResp.Available} success={padBWResp.Success} isOK={padBWResp.IsOK} areaRate={padBWResp.AreaRate:F1}% message={padBWResp.Message}");

            var allAvailable = alignResp.Available && shapeXResp.Available && padBWResp.Available;
            var allSuccess = alignResp.Success && shapeXResp.Success && padBWResp.Success;
            Write(allAvailable && allSuccess ? "PASS: all bridges native + success" : $"PARTIAL: available={allAvailable} success={allSuccess}");
            return allAvailable && allSuccess ? 0 : 1;
        }
        catch (Exception ex)
        {
            DiagnosticsLog.Write($"[SMOKE] EXCEPTION {ex}");
            return 5;
        }
    }

    private static bool IsBridgeAlgorithm(string type)
    {
        return string.Equals(type, "AlgoAlign", StringComparison.OrdinalIgnoreCase)
            || string.Equals(type, "AlgoShapeX", StringComparison.OrdinalIgnoreCase)
            || string.Equals(type, "AlgoPadBW", StringComparison.OrdinalIgnoreCase);
    }

    private static string ReadParam(Dictionary<string, string> parameters, string key)
    {
        return parameters.TryGetValue(key, out var value) ? value : "-";
    }

    private static byte[] CreateSyntheticImage(int width, int height)
    {
        // BGRA buffer with bright square in the center (used as fiducial target)
        var pixels = new byte[width * height * 4];
        var cx0 = width / 4;
        var cy0 = height / 4;
        var cx1 = width * 3 / 4;
        var cy1 = height * 3 / 4;
        for (var y = 0; y < height; y++)
        {
            for (var x = 0; x < width; x++)
            {
                var offset = (y * width + x) * 4;
                var inSquare = x >= cx0 && x < cx1 && y >= cy0 && y < cy1;
                var gray = inSquare ? (byte)220 : (byte)40;
                pixels[offset + 0] = gray;
                pixels[offset + 1] = gray;
                pixels[offset + 2] = gray;
                pixels[offset + 3] = 255;
            }
        }
        return pixels;
    }

    private static InspectionModel CreateSyntheticModel(int imageWidth, int imageHeight)
    {
        var model = new InspectionModel
        {
            ModelName = "SmokeTest",
            Algorithm = "AlgoAlign",
            Threshold2D = 128
        };
        model.EnsureStructure();

        var windowW = Math.Max(64, imageWidth / 4);
        var windowH = Math.Max(64, imageHeight / 4);
        var windowX = (imageWidth - windowW) / 2;
        var windowY = (imageHeight - windowH) / 2;
        var window = new InspectionWindowData
        {
            Name = "SmokeWindow",
            Roi = new RoiRect(windowX, windowY, windowW, windowH)
        };

        var alignSearchSize = 40;
        for (var i = 0; i < 4; i++)
        {
            var sx = i % 2 == 0 ? windowX + 16 : windowX + windowW - 16;
            var sy = i < 2 ? windowY + 16 : windowY + windowH - 16;
            model.AlignSearchRois[i] = new RoiRect(
                sx - alignSearchSize / 2,
                sy - alignSearchSize / 2,
                alignSearchSize,
                alignSearchSize);
        }
        model.AlignSearchSizeX = alignSearchSize;
        model.AlignSearchSizeY = alignSearchSize;
        model.AlignSearchNum = 4;
        model.AlignShiftEnabled = true;
        model.AlignAngleEnabled = true;

        window.Algorithms.Add(new InspectionAlgorithmData { Type = "AlgoAlign", DisplayName = "Smoke Align" });
        window.Algorithms.Add(new InspectionAlgorithmData
        {
            Type = "AlgoShapeX",
            DisplayName = "Smoke ShapeX",
            AlgorithmRoi = new RoiRect(windowX + 8, windowY + 8, windowW - 16, windowH - 16)
        });
        window.Algorithms.Add(new InspectionAlgorithmData
        {
            Type = "AlgoPadBW",
            DisplayName = "Smoke PadBW",
            AlgorithmRoi = new RoiRect(windowX + 12, windowY + 12, windowW - 24, windowH - 24)
        });

        model.Part.Windows.Add(window);
        model.SelectedWindowId = window.Id;
        model.EnsureStructure();
        return model;
    }
}
