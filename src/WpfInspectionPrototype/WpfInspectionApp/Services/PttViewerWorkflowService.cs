namespace WpfInspectionApp.Services;

public sealed class PttViewerWorkflowService : IPttViewerWorkflowService
{
    private readonly IPem3DViewerHostService _pem3DViewerHostService;
    private readonly IPttLoadService _pttLoadService;

    public PttViewerWorkflowService(
        IPem3DViewerHostService pem3DViewerHostService,
        IPttLoadService pttLoadService)
    {
        _pem3DViewerHostService = pem3DViewerHostService;
        _pttLoadService = pttLoadService;
    }

    public PttViewerWorkflowResult LoadIntoControl(string path, System.Windows.Forms.Panel hostPanel, bool prepareMpti)
    {
        try
        {
            DiagnosticsLog.Write($"LoadPtt: {path}");
            DiagnosticsLog.Write(_pttLoadService.CreateFileDiagnostics(path));
            _pem3DViewerHostService.LoadIntoControl(path, hostPanel);

            var mptiStatus = prepareMpti
                ? _pttLoadService.PrepareMptiBridge(path).StatusMessage
                : "MPTI prep skipped for Part Import";
            DiagnosticsLog.Write($"LoadPtt succeeded: {path}");
            return new PttViewerWorkflowResult(true, $"Loaded 3D PTT in PEM3DControl: {path} | {mptiStatus}", path);
        }
        catch (Exception ex)
        {
            DiagnosticsLog.Write($"LoadPtt failed: {ex}");
            return new PttViewerWorkflowResult(false, $"3D PTT load failed: {ex.Message}", path);
        }
    }
}
