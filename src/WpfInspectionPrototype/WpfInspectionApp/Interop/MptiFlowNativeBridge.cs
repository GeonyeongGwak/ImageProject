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

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    public static extern int MptiBridgeSetAlgoParamsPadBW(
        int wndIndex, int algoIndex, ref MptiBridgeFlowPadBWParams parameters);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    public static extern int MptiBridgeSetAlgoParamsBlob(
        int wndIndex, int algoIndex, ref MptiBridgeFlowBlobParams parameters);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    public static extern int MptiBridgeSetAlgoParamsBGA(
        int wndIndex, int algoIndex, ref MptiBridgeFlowBGAParams parameters);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    public static extern int MptiBridgeSetAlgoParamsEdge(
        int wndIndex, int algoIndex, ref MptiBridgeFlowEdgeParams parameters);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    public static extern int MptiBridgeSetAlgoParamsPattern(
        int wndIndex, int algoIndex, ref MptiBridgeFlowPatternParams parameters);

    [DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
    public static extern int MptiBridgeSetAlgoParamsShapeX(
        int wndIndex, int algoIndex, ref MptiBridgeFlowShapeXParams parameters);

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

    // Algorithm type enum (subset used here). Indices match the InspAlgoType enum in
    // NativeSources/MPTILib_Algo/PInsp_Algo/InspParamDef_Algo.h.
    public const int EALGO_BLOB    = 1;
    public const int EALGO_ALIGN   = 2;
    public const int EALGO_PATTERN = 6;
    public const int EALGO_EDGE    = 26;
    public const int EALGO_BGA     = 37;
    public const int EALGO_PADBW   = 40;
    public const int EALGO_SHAPEX  = 48;
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

// Mirrors MptiBridgeFlowPadBWParams in MptiBridgeFlow.h. The bridge fills only the
// subset of AlgoPadBW fields the UI typically tunes; the rest keep their native
// default-constructed values (tagAlgoPadBW ctor).
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public struct MptiBridgeFlowPadBWParams
{
    public int BinaryMin;
    public int BinaryMax;
    public int UseInsp2D;
    public int InvertCheck;
    public int UseTeachArea;
    public double TeachArea;
    public double TeachAreaRateMin;
    public double TeachAreaRateMax;
    public int UseShift;
    public double TeachShiftX;
    public double TeachShiftY;
    public int UseBlobWidth;
    public double BlobSizeWidth;
    public int UseBlobLength;
    public double BlobSizeLength;
    public int UseBlobArea;
    public double BlobArea;
    public int FilterLevel;
    public int UseFillHole;
}

// Mirrors MptiBridgeFlowBlobParams in MptiBridgeFlow.h (subset of stAlgoBlob).
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public struct MptiBridgeFlowBlobParams
{
    public int UseInsp2D;
    public int MinBinary;
    public int MaxBinary;
    public int TypeRange2D;
    public int InvertCheck;
    public int TypeSelectBlob;
    public int FillHole;
    public int UseArea;
    public double AreaMin;
    public double AreaMax;
    public int UseShift;
    public double ShiftX;
    public double ShiftY;
    public int UseTeachWidth;
    public double TeachWidth;
    public double TeachWidthRateMin;
    public double TeachWidthRateMax;
    public int UseTeachLength;
    public double TeachLength;
    public double TeachLengthRateMin;
    public double TeachLengthRateMax;
}

// Mirrors MptiBridgeFlowBGAParams in MptiBridgeFlow.h (subset of stAlgoBGA).
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public struct MptiBridgeFlowBGAParams
{
    public int UseInsp2D;
    public int MinBinary;
    public int MaxBinary;
    public int TypeRange2D;
    public int InvertCheck;
    public int TypeSelectBlob;
    public int FillHole;
    public int UseArea;
    public double AreaMin;
    public double AreaMax;
    public double TeachArea;
    public double TeachVolume;
    public int UseShift;
    public double ShiftX;
    public double ShiftY;
    public int UseTeachWidth;
    public double TeachWidth;
    public double TeachWidthRateMin;
    public double TeachWidthRateMax;
    public int UseTeachLength;
    public double TeachLength;
    public double TeachLengthRateMin;
    public double TeachLengthRateMax;
    public int UseCircleRate;
    public double TeachCircleRate;
}

// Mirrors MptiBridgeFlowEdgeParams in MptiBridgeFlow.h (subset of stAlgoEdge).
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public struct MptiBridgeFlowEdgeParams
{
    public int UseInsp2D;
    public int MinBinary;
    public int MaxBinary;
    public int TypeRange2D;
    public int InvertCheck;
    public int UseShift;
    public double ShiftX;
    public double ShiftY;
    public int UseArea;
    public double AreaMin;
    public double AreaMax;
    public int SetLineCnt;
    public int UseGroup;
    public int LineFindType;
    public double LineFindRate;
    public int UseAngle;
    public double TeachRotate;
    public int UseDistanceX;
    public double TeachDistanceX;
    public int UseDistanceY;
    public double TeachDistanceY;
}

// Mirrors MptiBridgeFlowPatternParams in MptiBridgeFlow.h (subset of tagAlgoPattern).
// Model paths are ByValTStr (wchar_t[260] inline). Set to empty string to keep the
// native default-constructed value.
[StructLayout(LayoutKind.Sequential, Pack = 8, CharSet = CharSet.Unicode)]
public struct MptiBridgeFlowPatternParams
{
    public int UsePolarity;
    public double AcceptScore;
    public int UseShift;
    public double ShiftX;
    public double ShiftY;
    public double RangeAngle;
    public double WndAngle;
    public double SearchAngleRangeMin;
    public double SearchAngleRangeMax;
    public int SamplingAngle;
    public int UseNgOpt;
    public int UseCharacter;
    public int ModelFilter;
    public int CntPatternPath;
    public double FactorRed;
    public double FactorGreen;
    public double FactorBlue;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)] public string ModelPathInspect1;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)] public string ModelPathTeach;
}

// Mirrors MptiBridgeFlowShapeXParams in MptiBridgeFlow.h (subset of tagAlgoShapeX).
// `RoiCnt=0` means global-only inspection — per-ROI ShapeXROI[] entries are not yet
// exposed (would need a separate AddShapeXRoi API).
[StructLayout(LayoutKind.Sequential, Pack = 8)]
public struct MptiBridgeFlowShapeXParams
{
    public double PartAngle;
    public int LeadTipDirection;
    public int RoiCnt;
    public short MatchScore;
    public short Hist1;
    public short Hist1Low;
    public short Hist1Up;
    public short Hist2;
    public short Hist2Low;
    public short Hist2Up;
    public int InspOption;
    public float AspectRatio;
    public float MinScarThickness;
    public float VerticalMaxLength;
    public float HorizontalMaxLength;
    public float MaxNgArea;
    public float CriticalArea;
    public float MaxChippingLength;
    public int StartIdx;
    public float TieBarRate;
    public int ContrastValue;
}
