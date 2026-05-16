using System.Runtime.InteropServices;
using System.Text;

namespace WpfInspectionApp.Interop;

// P/Invoke wrapper for the MptiBridge flow API (MPTI_SetInspParam -> MPTI_InspProc ->
// MPTI_GetInspectionResult). Mirrors the verified MptiBridgeHarness/Program.cs signatures.
// Use via InspectionFlowService — not directly from view models.
public static class MptiFlowNativeBridge
{
    private const string Dll = "MptiBridge.dll";

    // --- shared helpers ---
    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    public static extern int MptiBridgeGetVersion(StringBuilder output, int outputLength);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    public static extern int MptiBridgeLoadPtt(
        [MarshalAs(UnmanagedType.LPWStr)] string pttPath,
        out int width,
        out int height,
        int callSpi,
        int useFactor,
        StringBuilder message,
        int messageLength);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    public static extern int MptiBridgeSetRawDataFovInfo(
        [MarshalAs(UnmanagedType.LPWStr)] string pttPath,
        [MarshalAs(UnmanagedType.LPWStr)] string? potPath,
        int x,
        int y,
        int width,
        int height,
        int separatedPart,
        out int resultNumber,
        StringBuilder message,
        int messageLength);

    // --- flow builders (native owns memory; each BeginPart resets prior state) ---

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    public static extern int MptiBridgeBeginPart(
        double cx, double cy, double w, double h, double angle,
        int srcW, int srcH);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    public static extern int MptiBridgeAddWindow(
        int wndInspType, double cx, double cy, double w, double h,
        int alignWndId, int parentWndId);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    public static extern int MptiBridgeAddAlgo(int wndIndex, int algoType, int algoId);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    public static extern int MptiBridgeSetAlgoParamsAlign(
        int wndIndex, int algoIndex, ref MptiBridgeFlowAlignParams parameters);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    public static extern int MptiBridgeCommitInspParam(StringBuilder message, int messageLength);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    public static extern int MptiBridgeInspProc(StringBuilder message, int messageLength);

    // --- result readers (valid after MptiBridgeInspProc) ---

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    public static extern int MptiBridgeResultAlignCount();

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    public static extern int MptiBridgeResultAlign(int i, ref MptiBridgeFlowAlignResult outResult);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    public static extern int MptiBridgeDumpAlignDiag(StringBuilder output, int outputLength);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    public static extern int MptiBridgeDebugInfo(
        ref int rawSizeX, ref int rawSizeY,
        ref int partImgSizeX, ref int partImgSizeY, ref int partImgTopRNull,
        ref int alignItemCnt, ref int alignGroupCnt);

    // Inspection type enum (subset used here).
    public const int EINSP_ALIGN = 1;

    // Algorithm type enum (subset used here).
    public const int EALGO_ALIGN = 2;
}

// Flat mirror of tagAlgoAlign (InspParamDef_Align.h). Pack=8 required on x64 to match
// the native struct layout.
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public struct MptiBridgeFlowAlignParams
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

[StructLayout(LayoutKind.Sequential, Pack = 8)]
public struct MptiBridgeFlowAlignResult
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
