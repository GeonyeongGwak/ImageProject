using System.Diagnostics;
using System.IO;
using System.Text;
using WpfInspectionApp.Interop;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

// End-to-end MPTI_InspProc flow runner used by the WPF UI. Mirrors the verified
// MptiBridgeHarness sequence: LoadPtt -> SetRawDataFovInfo -> BeginPart -> AddWindow ->
// AddAlgo -> SetAlgoParamsAlign -> CommitInspParam -> InspProc -> ResultAlign.
public sealed class InspectionFlowService : IInspectionFlowService
{
    private const int LightArrayLength = 10;

    public Task<AlignFlowResult> RunAlignAsync(AlignFlowRequest request)
    {
        // The native bridge isn't thread-safe (single global g_pMPTI / g_pInspMng state);
        // run on a worker thread but never in parallel.
        return Task.Run(() => RunAlignCore(request));
    }

    private static AlignFlowResult RunAlignCore(AlignFlowRequest request)
    {
        if (string.IsNullOrWhiteSpace(request.PttPath) || !File.Exists(request.PttPath))
        {
            return Failure($"PTT file not found: {request.PttPath}");
        }

        NativeDependencyPath.EnsureInitialized();

        var sb = new StringBuilder(512);
        var sw = Stopwatch.StartNew();

        try
        {
            int code = MptiFlowNativeBridge.MptiBridgeLoadPtt(
                request.PttPath, out int partW, out int partH, 0, 1, sb, sb.Capacity);
            if (code != 0 || partW <= 0 || partH <= 0)
            {
                return Failure($"LoadPtt failed ({code}): {sb}");
            }

            // Pixel resolution priority: explicit request override > .pot file >
            // (none — Commit falls back to 1.0). The .pot ships with every Part
            // Import bundle in the reference workflow, so this is the normal path.
            double resolX = request.PixelResolutionX;
            double resolY = request.PixelResolutionY;
            if (resolX <= 0 || resolY <= 0)
            {
                if (LegacyPttImageLoader.TryReadPotResolution(request.PttPath, out var rx, out var ry))
                {
                    resolX = rx;
                    resolY = ry;
                }
            }
            if (resolX > 0 && resolY > 0)
            {
                MptiFlowNativeBridge.MptiBridgeSetFlowResolution(resolX, resolY);
            }

            sb.Clear();
            // SetRawDataFovInfo without .pot returns -1 in minimal flow; ignore — Commit
            // force-sets m_resolX/Y so the missing .pot doesn't block downstream.
            MptiFlowNativeBridge.MptiBridgeSetRawDataFovInfo(
                request.PttPath, null, 0, 0, 0, 0, 0, out _, sb, sb.Capacity);

            // Build the part + Align window/search points from the current model.
            // When the model has no ROI yet, fall back to the old centered/quadrant
            // harness defaults so smoke testing still works from a bare PTT.
            MptiFlowNativeBridge.MptiBridgeBeginPart(
                partW / 2.0, partH / 2.0, partW, partH, 0.0, partW, partH);

            var window = ResolveWindow(request, partW, partH);
            int wndIdx = MptiFlowNativeBridge.MptiBridgeAddWindow(
                MptiFlowNativeBridge.EINSP_ALIGN, window.CenterX, window.CenterY, window.Width, window.Height, 0, 0);
            if (wndIdx < 0) return Failure($"AddWindow failed: {wndIdx}");

            int algoIdx = MptiFlowNativeBridge.MptiBridgeAddAlgo(
                wndIdx, MptiFlowNativeBridge.EALGO_ALIGN, 1);
            if (algoIdx < 0) return Failure($"AddAlgo failed: {algoIdx}");

            var lightParams = CreateLightParams(request);
            int salRet = MptiFlowNativeBridge.MptiBridgeSetAlgoLight(wndIdx, algoIdx, ref lightParams);
            if (salRet != 0) return Failure($"SetAlgoLight failed: {salRet}");

            var search = ResolveSearch(request, partW, partH);
            var alignParams = new MptiBridgeFlowAlignParams
            {
                SearchNum = search.Count,
                SearchPointsX = search.X,
                SearchPointsY = search.Y,
                SearchSizeW = search.W,
                SearchSizeH = search.H,
                SearchMargin = Math.Max(0, request.SearchMargin),
                MinBinary = ClampInt(Math.Min(request.MinBinary, request.MaxBinary), 0, 255),
                MaxBinary = ClampInt(Math.Max(request.MinBinary, request.MaxBinary), 0, 255),
                TypeRange2D = ClampInt(request.TypeRange2D, 0, 3),
                UseInsp2D = request.UseInsp2D ? 1 : 0,
                InvertCheck = request.InvertCheck ? 1 : 0,
                UseInsp3D = request.UseInsp3D ? 1 : 0,
                HeightRateMin = Math.Min(request.HeightRateMin, request.HeightRateMax),
                HeightRateMax = Math.Max(request.HeightRateMin, request.HeightRateMax),
                HeightAverage = request.HeightAverage,
                TypeRange3D = ClampInt(request.TypeRange3D, 0, 3),
                UseIpc = request.UseIpc ? 1 : 0,
                IpcClass = ClampInt(request.IpcClass, 0, 2),
                UseShift = request.UseShift ? 1 : 0,
                MaxShiftX = request.MaxShiftX,
                MaxShiftY = request.MaxShiftY,
                UseAngle = request.UseAngle ? 1 : 0,
                MaxAngle = request.MaxAngle,
                SameSize = request.SameSize ? 1 : 0,
                MinBlobArea = request.MinBlobArea,
                FillHole = request.FillHole ? 1 : 0,
                InspOption = request.InspOption,
            };
            int sapRet = MptiFlowNativeBridge.MptiBridgeSetAlgoParamsAlign(wndIdx, algoIdx, ref alignParams);
            if (sapRet != 0) return Failure($"SetAlgoParamsAlign failed: {sapRet}");

            sb.Clear();
            int commitCode = MptiFlowNativeBridge.MptiBridgeCommitInspParam(sb, sb.Capacity);
            if (commitCode != 0) return Failure($"CommitInspParam failed ({commitCode}): {sb}");
            string commitMsg = sb.ToString();

            sb.Clear();
            int procCode = MptiFlowNativeBridge.MptiBridgeInspProc(sb, sb.Capacity);
            string procMsg = sb.ToString();

            var result = new MptiBridgeFlowAlignResult
            {
                DetectedCentersX = new int[4],
                DetectedCentersY = new int[4]
            };
            int resCode = MptiFlowNativeBridge.MptiBridgeResultAlign(0, ref result);

            var diag = new StringBuilder(4096);
            MptiFlowNativeBridge.MptiBridgeDumpAlignDiag(diag, diag.Capacity);

            sw.Stop();

            return new AlignFlowResult(
                Available: true,
                Success: resCode == 0 && result.IsInsp != 0,
                StatusMessage: $"InspProc={procCode} ({procMsg}); Commit: {commitMsg}",
                PartWidth: partW,
                PartHeight: partH,
                IsInsp: result.IsInsp,
                IsOk: result.IsOk,
                DefectCode: result.DefectCode,
                OkCount: result.OkCount,
                OffsetX: result.OffsetX,
                OffsetY: result.OffsetY,
                Theta: result.Theta,
                OkShiftX: result.OkShiftX,
                OkShiftY: result.OkShiftY,
                OkAngle: result.OkAngle,
                CentersX: result.DetectedCentersX ?? new int[4],
                CentersY: result.DetectedCentersY ?? new int[4],
                DiagDump: diag.ToString(),
                ElapsedMs: sw.Elapsed.TotalMilliseconds);
        }
        catch (DllNotFoundException ex) { return BridgeUnavailable($"DLL not found: {ex.Message}"); }
        catch (EntryPointNotFoundException ex) { return BridgeUnavailable($"Entry point missing: {ex.Message}"); }
        catch (BadImageFormatException ex) { return BridgeUnavailable($"Bad image format: {ex.Message}"); }
    }

    private static int ClampWindow(int value, int partExtent)
    {
        if (value <= 0) return Math.Max(64, partExtent / 3);
        return Math.Min(value, partExtent);
    }

    private static (double CenterX, double CenterY, int Width, int Height) ResolveWindow(
        AlignFlowRequest request,
        int partW,
        int partH)
    {
        var width = ClampWindow(request.WindowWidth, partW);
        var height = ClampWindow(request.WindowHeight, partH);
        var centerX = request.WindowCenterX > 0 ? request.WindowCenterX : partW / 2.0;
        var centerY = request.WindowCenterY > 0 ? request.WindowCenterY : partH / 2.0;

        centerX = ClampDouble(centerX, width / 2.0, partW - width / 2.0);
        centerY = ClampDouble(centerY, height / 2.0, partH - height / 2.0);
        return (centerX, centerY, width, height);
    }

    private static (int Count, int[] X, int[] Y, int[] W, int[] H) ResolveSearch(
        AlignFlowRequest request,
        int partW,
        int partH)
    {
        var count = request.SearchNum > 0 ? ClampInt(request.SearchNum, 1, 4) : 4;
        var x = new int[4];
        var y = new int[4];
        var w = new int[4];
        var h = new int[4];
        var qx = partW / 4;
        var qy = partH / 4;
        var fallbackX = new[] { qx, partW - qx, qx, partW - qx };
        var fallbackY = new[] { qy, qy, partH - qy, partH - qy };

        for (var i = 0; i < 4; i++)
        {
            var px = ReadPositive(request.SearchPointsX, i, fallbackX[i]);
            var py = ReadPositive(request.SearchPointsY, i, fallbackY[i]);
            x[i] = ClampInt(px, 0, Math.Max(0, partW - 1));
            y[i] = ClampInt(py, 0, Math.Max(0, partH - 1));
            w[i] = Math.Max(1, ReadPositive(request.SearchSizeWidths, i, request.SearchSizeW));
            h[i] = Math.Max(1, ReadPositive(request.SearchSizeHeights, i, request.SearchSizeH));
        }

        return (count, x, y, w, h);
    }

    private static MptiBridgeFlowLightParams CreateLightParams(AlignFlowRequest request)
    {
        return new MptiBridgeFlowLightParams
        {
            LightType = ClampInt(request.LightType, AlgorithmLightService.TopLight, AlgorithmLightService.ThreeDLight),
            RedValue = ClampInt(request.RedValue, 0, 200),
            GreenValue = ClampInt(request.GreenValue, 0, 200),
            BlueValue = ClampInt(request.BlueValue, 0, 200),
            WhiteValue = ClampInt(request.WhiteValue, 0, 200),
            LightCnt = ClampInt(request.LightCnt, 0, LightArrayLength),
            ArrRedValue = CreateLightArray(request.ArrRedValue, 0, 200),
            ArrGreenValue = CreateLightArray(request.ArrGreenValue, 0, 200),
            ArrBlueValue = CreateLightArray(request.ArrBlueValue, 0, 200),
            ArrWhiteValue = CreateLightArray(request.ArrWhiteValue, 0, 200),
            ArrCalculation = CreateLightArray(request.ArrCalculation, 0, 2),
            ArrLightPosition = CreateLightArray(request.ArrLightPosition, AlgorithmLightService.TopLight, AlgorithmLightService.BottomLight)
        };
    }

    private static int[] CreateLightArray(int[]? source, int min, int max)
    {
        var values = new int[LightArrayLength];
        if (source == null)
        {
            return values;
        }

        var count = Math.Min(values.Length, source.Length);
        for (var index = 0; index < count; index++)
        {
            values[index] = ClampInt(source[index], min, max);
        }

        return values;
    }

    private static int ReadPositive(int[]? values, int index, int fallback)
    {
        if (values == null || index < 0 || index >= values.Length || values[index] <= 0)
        {
            return fallback;
        }

        return values[index];
    }

    private static int ClampInt(int value, int min, int max)
    {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    private static double ClampDouble(double value, double min, double max)
    {
        if (max < min) return value;
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    private static AlignFlowResult Failure(string message) => new(
        Available: true, Success: false, StatusMessage: message,
        PartWidth: 0, PartHeight: 0, IsInsp: 0, IsOk: 0, DefectCode: 0, OkCount: 0,
        OffsetX: 0, OffsetY: 0, Theta: 0, OkShiftX: 0, OkShiftY: 0, OkAngle: 0,
        CentersX: new int[4], CentersY: new int[4],
        DiagDump: string.Empty, ElapsedMs: 0);

    private static AlignFlowResult BridgeUnavailable(string message) => new(
        Available: false, Success: false, StatusMessage: message,
        PartWidth: 0, PartHeight: 0, IsInsp: 0, IsOk: 0, DefectCode: 0, OkCount: 0,
        OffsetX: 0, OffsetY: 0, Theta: 0, OkShiftX: 0, OkShiftY: 0, OkAngle: 0,
        CentersX: new int[4], CentersY: new int[4],
        DiagDump: string.Empty, ElapsedMs: 0);
}
