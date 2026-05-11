namespace WpfInspectionApp.Services;

public sealed record PartInspectionWorkflowResult(
    string StatusMessage,
    string ResultText,
    string? TimingText,
    string? RefreshSelectedId,
    bool RanInspection);

public interface IInspectionWorkflowService
{
    Task<PartInspectionWorkflowResult> RunPartAsync(
        Models.InspectionModel model,
        PartRuntimeImage? image,
        string? activeAlgorithmId);
}
