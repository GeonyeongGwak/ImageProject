using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed record RoiModelOperationResult(
    RoiRect? Roi,
    string? SelectedId,
    string? StatusMessage,
    bool Changed);

public interface IRoiModelService
{
    RoiRect? GetActiveRoi(InspectionModel model, RoiRect? previewRoi);

    RoiRect? GetActiveInspectionRoi(InspectionModel model, string selectedAlgorithm, RoiRect? previewRoi);

    InspectionWindowData? GetActiveWindow(InspectionModel model);

    InspectionAlgorithmData? GetActiveAlgorithm(InspectionModel model, string selectedAlgorithm);

    RoiModelOperationResult UpsertActiveWindow(InspectionModel model, RoiRect roi);

    RoiModelOperationResult CreateWindowFromRoi(InspectionModel model, RoiRect roi);

    RoiModelOperationResult SetActiveAlgorithmRoi(InspectionModel model, string selectedAlgorithm, RoiRect roi, Func<RoiRect?, string> formatRoi);

    RoiModelOperationResult SelectNextWindow(InspectionModel model);

    RoiModelOperationResult DeleteActiveWindow(InspectionModel model);

    RoiModelOperationResult ResizeActiveRoiFromSearchInputs(InspectionModel model, RoiRect? previewRoi, int sourceWidth, int sourceHeight);

    RoiRect? SyncSearchSizeFromActiveRoi(InspectionModel model, RoiRect? previewRoi);

    string FormatActiveRoiText(InspectionModel model, string selectedAlgorithm, RoiRect? previewRoi, int sourceWidth, int sourceHeight, Func<RoiRect?, string> formatRoi);
}
