namespace WpfInspectionApp.Services;

public sealed class ImageLoadWorkflowService : IImageLoadWorkflowService
{
    private readonly IImageRuntimeStateService _imageRuntimeStateService;

    public ImageLoadWorkflowService(IImageRuntimeStateService imageRuntimeStateService)
    {
        _imageRuntimeStateService = imageRuntimeStateService;
    }

    public ImageLoadWorkflowResult Load(string path)
    {
        try
        {
            var frame = _imageRuntimeStateService.LoadImage(path);
            return new ImageLoadWorkflowResult(
                true,
                frame.SourceBitmap,
                frame.BinaryBitmap,
                _imageRuntimeStateService.SourceWidth,
                _imageRuntimeStateService.SourceHeight,
                $"Loaded 2D image: {path}");
        }
        catch (Exception ex)
        {
            DiagnosticsLog.Write($"2D image load failed: {path} | {ex}");
            return new ImageLoadWorkflowResult(
                false,
                null,
                null,
                0,
                0,
                $"2D image load failed: {ex.Message}");
        }
    }
}
