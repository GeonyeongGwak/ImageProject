using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed record ModelLoadWorkflowResult(
    bool Success,
    string StatusMessage,
    InspectionModel? Model);

public interface IModelWorkflowService
{
    string Save(InspectionModel model, string modelDirectory);

    ModelLoadWorkflowResult Load(string path);
}
