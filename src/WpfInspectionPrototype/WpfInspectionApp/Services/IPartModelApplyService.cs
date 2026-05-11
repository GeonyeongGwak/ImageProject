using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed record PartModelApplyResult(string Summary, string? SelectedWindowId);

public interface IPartModelApplyService
{
    PartModelApplyResult Apply(InspectionModel model, PartInspectionData importedPart, string source);
}
