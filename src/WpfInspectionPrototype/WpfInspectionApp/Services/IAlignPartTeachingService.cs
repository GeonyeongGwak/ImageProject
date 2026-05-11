using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed record AlignPartTeachingResult(
    bool Success,
    string Status,
    string Summary,
    string? SelectedWindowId,
    int WindowCount,
    int TaughtCount);

public interface IAlignPartTeachingService
{
    AlignPartTeachingResult Apply(InspectionModel model, bool useGerber, Func<RoiRect?, string> formatRoi);
}
