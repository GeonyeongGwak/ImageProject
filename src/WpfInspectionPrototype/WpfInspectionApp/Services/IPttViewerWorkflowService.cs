namespace WpfInspectionApp.Services;

public sealed record PttViewerWorkflowResult(
    bool Success,
    string StatusMessage,
    string Path);

public interface IPttViewerWorkflowService
{
    PttViewerWorkflowResult LoadIntoControl(string path, System.Windows.Forms.Panel hostPanel, bool prepareMpti);
}
