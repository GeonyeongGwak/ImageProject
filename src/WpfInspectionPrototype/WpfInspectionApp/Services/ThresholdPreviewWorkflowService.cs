using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class ThresholdPreviewWorkflowService : IThresholdPreviewWorkflowService
{
    private readonly IPreviewProcessingService _previewProcessingService;
    private readonly IImageRuntimeStateService _imageRuntimeStateService;
    private readonly IThresholdResultService _thresholdResultService;
    private CancellationTokenSource? _cancellation;

    public ThresholdPreviewWorkflowService(
        IPreviewProcessingService previewProcessingService,
        IImageRuntimeStateService imageRuntimeStateService,
        IThresholdResultService thresholdResultService)
    {
        _previewProcessingService = previewProcessingService;
        _imageRuntimeStateService = imageRuntimeStateService;
        _thresholdResultService = thresholdResultService;
    }

    public async Task<ThresholdPreviewWorkflowResult> RunAsync(
        InspectionModel model,
        RoiRect? activeInspectionRoi,
        InspectionWindowData? activeWindow,
        InspectionAlgorithmData? activeAlgorithm,
        Func<RoiRect?, string> formatRoi)
    {
        if (!_imageRuntimeStateService.HasSourceImage || _imageRuntimeStateService.SourcePixels == null)
        {
            return Empty(false);
        }

        _cancellation?.Cancel();
        var cancellation = new CancellationTokenSource();
        _cancellation = cancellation;

        var threshold = model.Use2D ? model.Threshold2D : 255;

        try
        {
            var preview = await _previewProcessingService.ThresholdAsync(
                _imageRuntimeStateService.SourcePixels,
                _imageRuntimeStateService.SourceWidth,
                _imageRuntimeStateService.SourceHeight,
                _imageRuntimeStateService.SourceStride,
                threshold,
                activeInspectionRoi,
                cancellation.Token);

            if (cancellation.IsCancellationRequested)
            {
                return Empty(true);
            }

            var binaryImage = _imageRuntimeStateService.UpdateBinaryBitmap(
                preview.Pixels,
                preview.Width,
                preview.Height,
                preview.Stride);

            var nativeLabel = preview.Response.UsedNative ? "Native C++" : "Managed fallback";
            var thresholdResult = preview.Response.Result;
            var applied = _thresholdResultService.Apply(
                model,
                preview.Response,
                nativeLabel,
                activeWindow,
                activeAlgorithm,
                activeInspectionRoi,
                formatRoi);

            return new ThresholdPreviewWorkflowResult(
                true,
                false,
                binaryImage,
                preview.Response.UsedNative,
                $"{nativeLabel} {thresholdResult.ElapsedMs:F3} ms / UI {preview.UiElapsedMs:F1} ms",
                "Threshold updated.",
                applied.ResultText,
                applied.UpdatedAlgorithmId);
        }
        catch (OperationCanceledException)
        {
            return Empty(true);
        }
    }

    private static ThresholdPreviewWorkflowResult Empty(bool canceled)
    {
        return new ThresholdPreviewWorkflowResult(false, canceled, null, false, string.Empty, string.Empty, string.Empty, null);
    }
}
