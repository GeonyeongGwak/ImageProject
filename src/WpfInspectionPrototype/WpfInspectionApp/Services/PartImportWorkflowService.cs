using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class PartImportWorkflowService : IPartImportWorkflowService
{
    private readonly IPartImportService _partImportService;
    private readonly IPartModelApplyService _partModelApplyService;

    public PartImportWorkflowService(
        IPartImportService partImportService,
        IPartModelApplyService partModelApplyService)
    {
        _partImportService = partImportService;
        _partModelApplyService = partModelApplyService;
    }

    public PartImportWorkflowResult ImportIntoModel(InspectionModel model, string path)
    {
        DiagnosticsLog.Write($"ImportPartFromPath: {path}");
        if (!_partImportService.TryLoadImportPart(path, out var importedPart, out var status, out var importedImagePath, out var importedPttPath))
        {
            DiagnosticsLog.Write(status);
            return new PartImportWorkflowResult(false, status, null, null, null, null);
        }

        var apply = _partModelApplyService.Apply(model, importedPart, path);
        return new PartImportWorkflowResult(
            true,
            CreateStatusMessage(path, importedImagePath, importedPttPath),
            importedImagePath,
            importedPttPath,
            apply.SelectedWindowId,
            apply.Summary);
    }

    private static string CreateStatusMessage(string path, string? imagePath, string? pttPath)
    {
        if (!string.IsNullOrWhiteSpace(imagePath))
        {
            return !string.IsNullOrWhiteSpace(pttPath)
                ? $"Part imported with 2D/3D image: {path}"
                : $"Part imported with image: {path}";
        }

        return !string.IsNullOrWhiteSpace(pttPath)
            ? $"Part imported with PTT data: {path}"
            : $"Part imported: {path}";
    }
}
