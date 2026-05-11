using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed record RoiUiSyncState(
    string ActiveRoiText,
    string RoiText,
    RoiRect? SearchSizeRoi);

public interface IRoiUiStateService
{
    RoiUiSyncState CreateSyncState(
        InspectionModel model,
        string selectedAlgorithm,
        RoiRect? previewRoi,
        int sourceWidth,
        int sourceHeight,
        Func<RoiRect?, string> formatRoi);

    string CreateRoiText(
        InspectionModel model,
        string selectedAlgorithm,
        RoiRect? previewRoi,
        int sourceWidth,
        int sourceHeight,
        Func<RoiRect?, string> formatRoi);
}
