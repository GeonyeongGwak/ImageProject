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
    int SearchSizeW = 80,
    int SearchSizeH = 80,
    int MinBinary = 100,
    int MaxBinary = 255,
    double MaxShiftX = 20,
    double MaxShiftY = 20,
    double MaxAngle = 5,
    int MinBlobArea = 10,
    double PixelResolutionX = 0,
    double PixelResolutionY = 0);

public interface IInspectionFlowService
{
    Task<AlignFlowResult> RunAlignAsync(AlignFlowRequest request);
}
