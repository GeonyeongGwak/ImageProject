using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Models;
using WpfInspectionApp.Services;
using WpfInspectionApp.ViewModels;

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

            RunServiceRegressionChecks(services, Write);

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

    private static void RunServiceRegressionChecks(AppServices services, Action<string> write)
    {
        write("running service regression checks...");
        VerifyPartRuntimeHandlesCaseDuplicateKeys();
        VerifyLightServiceRoundTrip(services.AlgorithmLight);
        VerifyUserLightViewModelReferenceFlow(services.AlgorithmLight);
        VerifyLegacyLightImportParsing();
        write("service regression checks passed");
    }

    private static void VerifyPartRuntimeHandlesCaseDuplicateKeys()
    {
        var model = CreateSyntheticModel(128, 128);
        var algorithm = model.Part.Windows[0].Algorithms[0];
        algorithm.ApplyCatalogDefaults();
        algorithm.Parameters["Common.bAlgoEnable"] = "true";
        algorithm.Parameters["common.balgoenable"] = "false";
        algorithm.Parameters["Align.Threshold"] = "128";
        algorithm.Parameters["align.threshold"] = "64";

        var runtime = new PartInspectionRuntime();
        var result = runtime.Run(model, image: null);
        Assert(result.TotalCount == 3, "Part runtime should keep all synthetic algorithms.");

        var runtimeAlgorithm = result.Packet.Windows[0].Algorithms[0];
        Assert(CountKeys(runtimeAlgorithm.Parameters, "Common.bAlgoEnable") == 1, "Runtime packet should collapse Common.bAlgoEnable duplicates.");
        Assert(CountKeys(runtimeAlgorithm.Parameters, "Align.Threshold") == 1, "Runtime packet should collapse Align.Threshold duplicates.");
        Assert(CountKeys(algorithm.Parameters, "Runtime.LastRun") == 1, "Runtime source parameters should be written without duplicate Runtime.LastRun keys.");
    }

    private static void VerifyLightServiceRoundTrip(IAlgorithmLightService service)
    {
        var algorithm = new InspectionAlgorithmData { Type = "AlgoShapeX", DisplayName = "Light RoundTrip" };
        algorithm.ApplyCatalogDefaults();

        service.SaveState(algorithm, new AlgorithmLightState
        {
            LightType = AlgorithmLightService.UserLight,
            UserCells =
            [
                new AlgorithmLightCell { Position = AlgorithmLightService.TopLight, Operator = 1, RedValue = 120, GreenValue = 30, BlueValue = 20, WhiteValue = 10 },
                new AlgorithmLightCell { Position = AlgorithmLightService.BottomLight, Operator = 2, RedValue = 80, GreenValue = -1, BlueValue = 40, WhiteValue = -1 }
            ]
        });

        var read = service.ReadState(algorithm);
        Assert(read.LightType == AlgorithmLightService.UserLight, "Light service should preserve User Light mode.");
        Assert(read.UserCells.Count == 2, "Light service should preserve user light cell count.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.ArrRedValueString") == "120|80", "Light service should store red user-light array.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.ArrCalculationString") == "1|2", "Light service should store user-light operators.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.ArrLightPositionString") == "0|2", "Light service should store user-light positions.");
    }

    private static void VerifyUserLightViewModelReferenceFlow(IAlgorithmLightService service)
    {
        var algorithm = new InspectionAlgorithmData { Type = "AlgoShapeX", DisplayName = "Light ViewModel" };
        algorithm.ApplyCatalogDefaults();
        service.SaveState(algorithm, new AlgorithmLightState
        {
            LightType = AlgorithmLightService.UserLight,
            UserCells =
            [
                new AlgorithmLightCell { Position = AlgorithmLightService.TopLight, Operator = 0, RedValue = 100, GreenValue = 0, BlueValue = 0, WhiteValue = 0 }
            ]
        });

        var previewRequests = 0;
        var viewModel = new LightControlViewModel(service, () => previewRequests++);
        viewModel.Load(algorithm);

        Assert(viewModel.UserCells.Count == 1, "User Light view model should load one cell.");
        var first = viewModel.UserCells[0];
        viewModel.ToggleUserCellOperatorCommand.Execute(first);
        Assert(viewModel.UserCells.Count == 2, "Last blank operator should append a new User Light cell.");
        Assert(first.OperatorType == 1, "First operator should become Add.");
        Assert(viewModel.SelectedUserCell == viewModel.UserCells[1], "Newly appended User Light cell should be selected.");

        viewModel.ToggleUserCellOperatorCommand.Execute(first);
        Assert(viewModel.UserCells.Count == 2, "Add operator should cycle to Sub without changing cell count.");
        Assert(first.OperatorType == 2, "First operator should become Sub.");

        viewModel.ToggleUserCellOperatorCommand.Execute(first);
        Assert(viewModel.UserCells.Count == 1, "Second-last operator returning to blank should remove the trailing cell.");
        Assert(first.OperatorType == 0, "First operator should return to None.");

        first.Channels[2].PresetCommand?.Execute(null);
        Assert(first.Channels[0].Value == 0 && first.Channels[2].Value == 100, "Channel preset should solo the selected channel.");

        viewModel.ToggleUserPreviewModeCommand.Execute(null);
        var previewState = viewModel.CreatePreviewState(service.ReadState(algorithm));
        Assert(!viewModel.IsUserMixPreview, "Preview mode should toggle to selected-cell preview.");
        Assert(previewState.UserCells.Count == 1, "Selected-cell preview should render only one User Light cell.");
        Assert(previewRequests > 0, "User Light view model should request preview refreshes.");
    }

    private static void VerifyLegacyLightImportParsing()
    {
        const string xml = """
            <RawDataContainer>
              <PartData>
                <Name>SmokeLightPart</Name>
              </PartData>
              <WindowDataList>
                <WindowData>
                  <ID>W1</ID>
                  <Name>Main</Name>
                  <RelRoi>
                    <cx>64</cx>
                    <cy>64</cy>
                    <w>100</w>
                    <h>100</h>
                  </RelRoi>
                  <AlgorithmDataList>
                    <AlgorithmData>
                      <ID>A1</ID>
                      <Type>Align</Type>
                      <LightType>User</LightType>
                      <RedValue>35</RedValue>
                      <GreenValue>25</GreenValue>
                      <BlueValue>15</BlueValue>
                      <WhiteValue>5</WhiteValue>
                      <LightCnt>2</LightCnt>
                      <ArrRedValue>
                        <Value>120</Value>
                        <Value>80</Value>
                      </ArrRedValue>
                      <ArrGreenValue>30, -1</ArrGreenValue>
                      <ArrBlueValue>20;40</ArrBlueValue>
                      <ArrWhiteValue>10|-1</ArrWhiteValue>
                      <ArrCalculation>1|2</ArrCalculation>
                      <ArrLightPosition>0|2</ArrLightPosition>
                    </AlgorithmData>
                  </AlgorithmDataList>
                </WindowData>
              </WindowDataList>
            </RawDataContainer>
            """;

        var parsed = LegacyRawPartImportAdapter.TryParse(xml, out var part, out var status);
        Assert(parsed, $"Legacy light XML should parse. Status: {status}");
        var algorithm = part.Windows.Single().Algorithms.Single();
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.LightTypeNum") == "3", "Import should normalize User Light type.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.RedValue") == "35", "Import should parse normal red value.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.LightCnt") == "2", "Import should parse light count.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.ArrRedValueString") == "120|80", "Import should parse light array container.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.ArrGreenValueString") == "30|-1", "Import should parse comma-separated light array.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.ArrBlueValueString") == "20|40", "Import should parse semicolon-separated light array.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.ArrWhiteValueString") == "10|-1", "Import should parse pipe-separated light array.");
    }

    private static void Assert(bool condition, string message)
    {
        if (!condition)
        {
            throw new InvalidOperationException(message);
        }
    }

    private static int CountKeys(Dictionary<string, string> parameters, string key)
    {
        return parameters.Keys.Count(candidate => string.Equals(candidate, key, StringComparison.OrdinalIgnoreCase));
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
