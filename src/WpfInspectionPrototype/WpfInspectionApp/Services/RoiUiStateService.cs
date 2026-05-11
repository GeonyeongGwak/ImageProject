using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class RoiUiStateService : IRoiUiStateService
{
    private readonly IRoiModelService _roiModelService;

    public RoiUiStateService(IRoiModelService roiModelService)
    {
        _roiModelService = roiModelService;
    }

    public RoiUiSyncState CreateSyncState(
        InspectionModel model,
        string selectedAlgorithm,
        RoiRect? previewRoi,
        int sourceWidth,
        int sourceHeight,
        Func<RoiRect?, string> formatRoi)
    {
        var window = _roiModelService.GetActiveWindow(model);
        var activeRoiText = window == null ? "Window - none" : window.Name;
        var searchSizeRoi = _roiModelService.SyncSearchSizeFromActiveRoi(model, previewRoi);
        var roiText = CreateRoiText(model, selectedAlgorithm, previewRoi, sourceWidth, sourceHeight, formatRoi);

        return new RoiUiSyncState(activeRoiText, roiText, searchSizeRoi);
    }

    public string CreateRoiText(
        InspectionModel model,
        string selectedAlgorithm,
        RoiRect? previewRoi,
        int sourceWidth,
        int sourceHeight,
        Func<RoiRect?, string> formatRoi)
    {
        return _roiModelService.FormatActiveRoiText(
            model,
            selectedAlgorithm,
            previewRoi,
            sourceWidth,
            sourceHeight,
            formatRoi);
    }
}
