namespace WpfInspectionApp.Services;

// Result of a single MPTI_InspProc flow run targeting one Align window with one algorithm.
public sealed record AlignFlowResult(
    bool Available,
    bool Success,
    string StatusMessage,
    int PartWidth,
    int PartHeight,
    int IsInsp,
    int IsOk,
    int DefectCode,
    int OkCount,
    double OffsetX,
    double OffsetY,
    double Theta,
    int OkShiftX,
    int OkShiftY,
    int OkAngle,
    int[] CentersX,
    int[] CentersY,
    string DiagDump,
    double ElapsedMs);

// Inputs for a minimal Align flow run.
//
// Resolution: PixelResolutionX/Y are mm/pixel. When 0 (default), the service tries to
// read them from the .pot file next to the PTT. If neither is available the native
// bridge falls back to 1.0 (treats coords as already in pixel-space). Coordinates in
// this request are always in pixel-space regardless — only the inspection pipeline's
// internal mm <-> pixel math is affected by the resolution.
public sealed record AlignFlowRequest(
    string PttPath,
    int WindowWidth,
    int WindowHeight,
    int WindowCenterX = 0,
    int WindowCenterY = 0,
    int SearchNum = 0,
    int[]? SearchPointsX = null,
    int[]? SearchPointsY = null,
    int[]? SearchSizeWidths = null,
    int[]? SearchSizeHeights = null,
    int SearchMargin = 10,
    int SearchSizeW = 80,
    int SearchSizeH = 80,
    int MinBinary = 100,
    int MaxBinary = 255,
    int TypeRange2D = 2,
    bool UseInsp2D = true,
    bool InvertCheck = false,
    bool UseInsp3D = false,
    double HeightRateMin = 0,
    double HeightRateMax = 120,
    double HeightAverage = 0,
    int TypeRange3D = 2,
    bool UseShift = true,
    double MaxShiftX = 20,
    double MaxShiftY = 20,
    bool UseAngle = true,
    double MaxAngle = 5,
    bool SameSize = true,
    int MinBlobArea = 10,
    bool FillHole = false,
    int InspOption = 0,
    bool UseIpc = false,
    int IpcClass = 1,
    double PixelResolutionX = 0,
    double PixelResolutionY = 0,
    int LightType = 0,
    int RedValue = 100,
    int GreenValue = 0,
    int BlueValue = 0,
    int WhiteValue = 0,
    int LightCnt = 0,
    int[]? ArrRedValue = null,
    int[]? ArrGreenValue = null,
    int[]? ArrBlueValue = null,
    int[]? ArrWhiteValue = null,
    int[]? ArrCalculation = null,
    int[]? ArrLightPosition = null);

public interface IInspectionFlowService
{
    Task<AlignFlowResult> RunAlignAsync(AlignFlowRequest request);
}
