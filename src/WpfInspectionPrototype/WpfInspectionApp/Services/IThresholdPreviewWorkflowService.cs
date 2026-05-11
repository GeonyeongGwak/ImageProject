using System.Windows.Media;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed record ThresholdPreviewWorkflowResult(
    bool RanPreview,
    bool Canceled,
    ImageSource? BinaryImage,
    bool UsedNative,
    string TimingText,
    string StatusMessage,
    string ResultText,
    string? UpdatedAlgorithmId);

public interface IThresholdPreviewWorkflowService
{
    Task<ThresholdPreviewWorkflowResult> RunAsync(
        InspectionModel model,
        RoiRect? activeInspectionRoi,
        InspectionWindowData? activeWindow,
        InspectionAlgorithmData? activeAlgorithm,
        Func<RoiRect?, string> formatRoi);
}
