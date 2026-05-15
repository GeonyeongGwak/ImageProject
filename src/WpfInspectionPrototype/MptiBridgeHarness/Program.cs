using System;
using System.Runtime.InteropServices;
using System.Text;

namespace MptiBridgeHarness;

// Console harness for the MptiBridge native inspection flow (no WPF UI).
// Stage 1: PTT load -> build one Align window -> MPTI_SetInspParam -> MPTI_InspProc -> read result.
internal static class Program
{
    private const string Dll = "MptiBridge.dll";
    private const string PemtronSystemPath = @"C:\Program Files\Pemtron System";

    // --- existing bridge ---
    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeGetVersion(StringBuilder output, int outputLength);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeDebugProbe(int breakIntoDebugger, StringBuilder output, int outputLength);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeLoadPtt(
        [MarshalAs(UnmanagedType.LPWStr)] string pttPath,
        out int width, out int height, int callSpi, int useFactor,
        StringBuilder message, int messageLength);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeSetRawDataFovInfo(
        [MarshalAs(UnmanagedType.LPWStr)] string pttPath,
        [MarshalAs(UnmanagedType.LPWStr)] string? potPath,
        int x, int y, int width, int height, int separatedPart,
        out int resultNumber, StringBuilder message, int messageLength);

    // --- new flow API ---
    [StructLayout(LayoutKind.Sequential)]
    private struct FlowAlignParams
    {
        public int SearchNum;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)] public int[] SearchPointsX;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)] public int[] SearchPointsY;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)] public int[] SearchSizeW;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)] public int[] SearchSizeH;
        public int SearchMargin;
        public int MinBinary;
        public int MaxBinary;
        public int UseInsp2D;
        public int InvertCheck;
        public int UseShift;
        public double MaxShiftX;
        public double MaxShiftY;
        public int UseAngle;
        public double MaxAngle;
        public int SameSize;
        public int MinBlobArea;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct FlowAlignResult
    {
        public int OkCount;
        public double OffsetX;
        public double OffsetY;
        public double Theta;
        public int OkShiftX;
        public int OkShiftY;
        public int OkAngle;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)] public int[] DetectedCentersX;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)] public int[] DetectedCentersY;
        public int IsInsp;
        public int IsOk;
        public int DefectCode;
    }

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    private static extern int MptiBridgeBeginPart(double cx, double cy, double w, double h, double angle, int srcW, int srcH);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    private static extern int MptiBridgeAddWindow(int wndInspType, double cx, double cy, double w, double h, int alignWndId, int parentWndId);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    private static extern int MptiBridgeAddAlgo(int wndIndex, int algoType, int algoId);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    private static extern int MptiBridgeSetAlgoParamsAlign(int wndIndex, int algoIndex, ref FlowAlignParams p);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeCommitInspParam(StringBuilder message, int messageLength);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeInspProc(StringBuilder message, int messageLength);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    private static extern int MptiBridgeDebugInfo(
        ref int rawSizeX, ref int rawSizeY,
        ref int partImgSizeX, ref int partImgSizeY, ref int partImgTopRNull,
        ref int alignItemCnt, ref int alignGroupCnt);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    private static extern int MptiBridgeResultAlignCount();

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    private static extern int MptiBridgeResultAlign(int i, ref FlowAlignResult outResult);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeDumpAlignDiag(StringBuilder output, int outputLength);

    [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    private static extern bool SetDllDirectory(string lpPathName);

    // eINSP_ALIGN = 1, eAlgoAlign = 2 (see InspParamDef.h / InspParamDef_Algo.h)
    private const int EINSP_ALIGN = 1;
    private const int EALGO_ALIGN = 2;

    private static int Main(string[] args)
    {
        var debugProbeOnly = HasArg(args, "--debug-probe-only");
        var breakIntoDebugger = HasArg(args, "--native-debug-break");
        var pttPath = args.FirstOrDefault(arg => !arg.StartsWith("--", StringComparison.Ordinal))
            ?? @"D:\Work\GGY\pemtoFrameworkAll_R_4.0.0.7\bin\x64\Release\.ptt";
        Console.WriteLine($"[harness] CWD={Environment.CurrentDirectory}");
        Console.WriteLine($"[harness] PTT={pttPath} exists={System.IO.File.Exists(pttPath)}");
        if (System.IO.Directory.Exists(PemtronSystemPath))
        {
            Console.WriteLine($"[harness] SetDllDirectory={PemtronSystemPath} ok={SetDllDirectory(PemtronSystemPath)}");
        }

        var sb = new StringBuilder(512);

        Step("DebugProbe", () =>
        {
            sb.Clear();
            var c = MptiBridgeDebugProbe(breakIntoDebugger ? 1 : 0, sb, sb.Capacity);
            return $"code={c} '{sb}'";
        });

        if (debugProbeOnly)
        {
            Console.WriteLine("[harness] debug probe only.");
            return 0;
        }

        Step("GetVersion", () => { var c = MptiBridgeGetVersion(sb, sb.Capacity); return $"code={c} '{sb}'"; });

        int partW = 0, partH = 0;
        if (!Step("LoadPtt", () =>
        {
            sb.Clear();
            var c = MptiBridgeLoadPtt(pttPath, out partW, out partH, 0, 1, sb, sb.Capacity);
            return $"code={c} size={partW}x{partH} '{sb}'";
        })) return 11;

        Step("SetRawDataFovInfo", () =>
        {
            sb.Clear();
            var c = MptiBridgeSetRawDataFovInfo(pttPath, null, 0, 0, 0, 0, 0, out var rn, sb, sb.Capacity);
            return $"code={c} resultNumber={rn} '{sb}'";
        });

        if (partW <= 0 || partH <= 0) { partW = 1655; partH = 1137; }

        // --- build one Align window centered in the part ---
        Step("BeginPart", () => $"ret={MptiBridgeBeginPart(partW / 2.0, partH / 2.0, partW, partH, 0.0, partW, partH)}");

        int wndW = Math.Max(64, partW / 3);
        int wndH = Math.Max(64, partH / 3);
        int wndIdx = -1;
        Step("AddWindow(ALIGN)", () =>
        {
            wndIdx = MptiBridgeAddWindow(EINSP_ALIGN, partW / 2.0, partH / 2.0, wndW, wndH, 0, 0);
            return $"wndIdx={wndIdx}";
        });

        int algoIdx = -1;
        Step("AddAlgo(Align)", () =>
        {
            algoIdx = MptiBridgeAddAlgo(wndIdx, EALGO_ALIGN, 1);
            return $"algoIdx={algoIdx}";
        });

        Step("SetAlgoParamsAlign", () =>
        {
            int qx = partW / 4, qy = partH / 4;
            var p = new FlowAlignParams
            {
                SearchNum = 4,
                SearchPointsX = new[] { qx, partW - qx, qx, partW - qx },
                SearchPointsY = new[] { qy, qy, partH - qy, partH - qy },
                SearchSizeW = new[] { 80, 80, 80, 80 },
                SearchSizeH = new[] { 80, 80, 80, 80 },
                SearchMargin = 10,
                MinBinary = 100,
                MaxBinary = 255,
                UseInsp2D = 1,
                InvertCheck = 0,
                UseShift = 1,
                MaxShiftX = 20,
                MaxShiftY = 20,
                UseAngle = 1,
                MaxAngle = 5,
                SameSize = 1,
                MinBlobArea = 10,
            };
            return $"ret={MptiBridgeSetAlgoParamsAlign(wndIdx, algoIdx, ref p)}";
        });

        if (!Step("CommitInspParam", () =>
        {
            sb.Clear();
            var c = MptiBridgeCommitInspParam(sb, sb.Capacity);
            return $"code={c} '{sb}'";
        })) return 20;

        Step("DebugInfo(after commit)", () =>
        {
            int rsx = 0, rsy = 0, pisx = 0, pisy = 0, prn = 0, aic = 0, agc = 0;
            MptiBridgeDebugInfo(ref rsx, ref rsy, ref pisx, ref pisy, ref prn, ref aic, ref agc);
            return $"rawSize={rsx}x{rsy} partImgSize={pisx}x{pisy} partImgTopR_null={prn} alignItemCnt={aic} alignGroupCnt={agc}";
        });

        if (!Step("InspProc", () =>
        {
            sb.Clear();
            var c = MptiBridgeInspProc(sb, sb.Capacity);
            return $"code={c} '{sb}'";
        })) return 21;

        Step("ResultAlignCount", () => $"count={MptiBridgeResultAlignCount()}");
        Step("ResultAlign[0]", () =>
        {
            var r = new FlowAlignResult { DetectedCentersX = new int[4], DetectedCentersY = new int[4] };
            var c = MptiBridgeResultAlign(0, ref r);
            return $"code={c} isInsp={r.IsInsp} isOk={r.IsOk} defect={r.DefectCode} okCount={r.OkCount} " +
                   $"offset=({r.OffsetX:F2},{r.OffsetY:F2}) theta={r.Theta:F3} okShift=({r.OkShiftX},{r.OkShiftY}) okAngle={r.OkAngle} " +
                   $"centers=[{string.Join(";", r.DetectedCentersX.Zip(r.DetectedCentersY, (x, y) => $"({x},{y})"))}]";
        });

        Step("DumpAlignDiag", () =>
        {
            var diag = new StringBuilder(4096);
            var c = MptiBridgeDumpAlignDiag(diag, diag.Capacity);
            return $"code={c}\n----- begin diag -----\n{diag}----- end diag -----";
        });

        Console.WriteLine("[harness] done.");
        return 0;
    }

    private static bool Step(string name, Func<string> action)
    {
        try
        {
            var msg = action();
            Console.WriteLine($"[harness] {name}: {msg}");
            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"[harness] {name}: FAILED {ex.GetType().Name}: {ex.Message}");
            return false;
        }
    }

    private static bool HasArg(string[] args, string value)
    {
        return args.Any(arg => string.Equals(arg, value, StringComparison.OrdinalIgnoreCase));
    }
}
