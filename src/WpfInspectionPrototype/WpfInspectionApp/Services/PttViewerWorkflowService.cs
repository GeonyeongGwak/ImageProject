namespace WpfInspectionApp.Services;

public sealed class PttViewerWorkflowService : IPttViewerWorkflowService
{
    private readonly IPttLoadService _pttLoadService;

    public PttViewerWorkflowService(IPttLoadService pttLoadService)
    {
        _pttLoadService = pttLoadService;
    }

    public PttViewerWorkflowResult Load(string path, bool prepareMpti)
    {
        try
        {
            DiagnosticsLog.Write($"LoadPtt: {path}");
            DiagnosticsLog.Write(_pttLoadService.CreateFileDiagnostics(path));

            var load = _pttLoadService.PrepareMptiBridge(path);
            DiagnosticsLog.Write(load.Success ? $"LoadPtt succeeded: {path}" : $"LoadPtt failed via MPTI: {load.StatusMessage}");
            var mode = prepareMpti ? "requested" : "import";
            return new PttViewerWorkflowResult(
                load.Success,
                $"Loaded PTT data for internal 2D/light preview ({mode}): {path} | {load.StatusMessage}",
                path,
                load.Width,
                load.Height);
        }
        catch (Exception ex)
        {
            DiagnosticsLog.Write($"LoadPtt failed: {ex}");
            return new PttViewerWorkflowResult(false, $"PTT data load failed: {ex.Message}", path);
        }
    }
}
