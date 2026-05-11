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
        var frame = _imageRuntimeStateService.LoadImage(path);
        return new ImageLoadWorkflowResult(
            frame.SourceBitmap,
            frame.BinaryBitmap,
            _imageRuntimeStateService.SourceWidth,
            _imageRuntimeStateService.SourceHeight,
            $"Loaded 2D image: {path}");
    }
}
