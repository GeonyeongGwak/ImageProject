using WpfInspectionApp.Interop;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public interface IInspectionResultTextService
{
    string FormatPartRun(PartRunResult run);

    string FormatThresholdResult(
        InspectionModel model,
        NativeThresholdResponse response,
        string nativeLabel,
        InspectionWindowData? activeWindow,
        InspectionAlgorithmData? activeAlgorithm,
        RoiRect? activeInspectionRoi,
        Func<RoiRect?, string> formatRoi);
}
