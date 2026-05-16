using System.Diagnostics;
using System.IO;
using System.Text;
using WpfInspectionApp.Interop;

namespace WpfInspectionApp.Services;

// End-to-end MPTI_InspProc flow runner used by the WPF UI. Mirrors the verified
// MptiBridgeHarness sequence: LoadPtt -> SetRawDataFovInfo -> BeginPart -> AddWindow ->
// AddAlgo -> SetAlgoParamsAlign -> CommitInspParam -> InspProc -> ResultAlign.
public sealed class InspectionFlowService : IInspectionFlowService
{
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

            sb.Clear();
            // SetRawDataFovInfo without .pot returns -1 in minimal flow; ignore — Commit
            // force-sets m_resolX/Y so the missing .pot doesn't block downstream.
            MptiFlowNativeBridge.MptiBridgeSetRawDataFovInfo(
                request.PttPath, null, 0, 0, 0, 0, 0, out _, sb, sb.Capacity);

            // Build the part + one centered Align window with 4 quadrant search points.
            MptiFlowNativeBridge.MptiBridgeBeginPart(
                partW / 2.0, partH / 2.0, partW, partH, 0.0, partW, partH);

            int wndW = ClampWindow(request.WindowWidth, partW);
            int wndH = ClampWindow(request.WindowHeight, partH);
            int wndIdx = MptiFlowNativeBridge.MptiBridgeAddWindow(
                MptiFlowNativeBridge.EINSP_ALIGN, partW / 2.0, partH / 2.0, wndW, wndH, 0, 0);
            if (wndIdx < 0) return Failure($"AddWindow failed: {wndIdx}");

            int algoIdx = MptiFlowNativeBridge.MptiBridgeAddAlgo(
                wndIdx, MptiFlowNativeBridge.EALGO_ALIGN, 1);
            if (algoIdx < 0) return Failure($"AddAlgo failed: {algoIdx}");

            int qx = partW / 4, qy = partH / 4;
            var alignParams = new MptiBridgeFlowAlignParams
            {
                SearchNum = 4,
                SearchPointsX = new[] { qx, partW - qx, qx, partW - qx },
                SearchPointsY = new[] { qy, qy, partH - qy, partH - qy },
                SearchSizeW = new[] { request.SearchSizeW, request.SearchSizeW, request.SearchSizeW, request.SearchSizeW },
                SearchSizeH = new[] { request.SearchSizeH, request.SearchSizeH, request.SearchSizeH, request.SearchSizeH },
                SearchMargin = 10,
                MinBinary = request.MinBinary,
                MaxBinary = request.MaxBinary,
                UseInsp2D = 1,
                InvertCheck = 0,
                UseShift = 1,
                MaxShiftX = request.MaxShiftX,
                MaxShiftY = request.MaxShiftY,
                UseAngle = 1,
                MaxAngle = request.MaxAngle,
                SameSize = 1,
                MinBlobArea = request.MinBlobArea,
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
