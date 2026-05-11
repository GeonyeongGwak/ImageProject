using WpfInspectionApp.Interop;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed record ThresholdResultApplyResult(
    string ResultText,
    string? UpdatedAlgorithmId);

public interface IThresholdResultService
{
    ThresholdResultApplyResult Apply(
        InspectionModel model,
        NativeThresholdResponse response,
        string nativeLabel,
        InspectionWindowData? activeWindow,
        InspectionAlgorithmData? activeAlgorithm,
        RoiRect? activeInspectionRoi,
        Func<RoiRect?, string> formatRoi);
}
