namespace WpfInspectionApp.Services;

public sealed record PttViewerWorkflowResult(
    bool Success,
    string StatusMessage,
    string Path,
    int Width = 0,
    int Height = 0);

public interface IPttViewerWorkflowService
{
    PttViewerWorkflowResult Load(string path, bool prepareMpti);
}
