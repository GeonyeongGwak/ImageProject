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

// Inputs for a minimal Align flow run. Coordinates are in pixel-space (the bridge
// force-sets the MPTI resolution to 1.0 when .pot isn't loaded — see MptiBridgeFlow.cpp
// MptiBridgeCommitInspParam).
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
    int MinBlobArea = 10);

public interface IInspectionFlowService
{
    Task<AlignFlowResult> RunAlignAsync(AlignFlowRequest request);
}
