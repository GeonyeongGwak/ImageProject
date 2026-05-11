using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed record PartImportWorkflowResult(
    bool Success,
    string StatusMessage,
    string? ImagePath,
    string? PttPath,
    string? SelectedWindowId,
    string? Summary);

public interface IPartImportWorkflowService
{
    PartImportWorkflowResult ImportIntoModel(InspectionModel model, string path);
}
