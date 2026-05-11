namespace WpfInspectionApp.Services;

public sealed record PttLoadPreparationResult(
    bool Success,
    string StatusMessage,
    int Width = 0,
    int Height = 0);

public interface IPttLoadService
{
    string CreateFileDiagnostics(string path);

    PttLoadPreparationResult PrepareMptiBridge(string path);
}
