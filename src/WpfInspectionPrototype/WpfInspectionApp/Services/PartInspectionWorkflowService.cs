using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class PartInspectionWorkflowService : IInspectionWorkflowService
{
    private readonly IInspectionEngineService _inspectionEngineService;
    private readonly IInspectionResultTextService _inspectionResultTextService;

    public PartInspectionWorkflowService(
        IInspectionEngineService inspectionEngineService,
        IInspectionResultTextService inspectionResultTextService)
    {
        _inspectionEngineService = inspectionEngineService;
        _inspectionResultTextService = inspectionResultTextService;
    }

    public async Task<PartInspectionWorkflowResult> RunPartAsync(
        InspectionModel model,
        PartRuntimeImage? image,
        string? activeAlgorithmId)
    {
        model.EnsureStructure();
        if (model.Part.Windows.Count == 0)
        {
            return new PartInspectionWorkflowResult(
                "Draw a Window ROI before running Part inspection.",
                "Part inspection requires at least one Window ROI.",
                null,
                null,
                RanInspection: false);
        }

        var run = await _inspectionEngineService.RunPartAsync(model, image);
        var refreshSelectedId = string.IsNullOrWhiteSpace(activeAlgorithmId)
            ? model.SelectedWindowId
            : activeAlgorithmId;
        return new PartInspectionWorkflowResult(
            $"Part inspection completed: {run.ExecutedCount} executed, {run.SkippedCount} skipped.",
            _inspectionResultTextService.FormatPartRun(run),
            $"Part {run.ElapsedMs:F3} ms / {run.ExecutedCount} Algo",
            refreshSelectedId,
            RanInspection: true);
    }
}
