using System.Windows.Media.Imaging;

namespace WpfInspectionApp.Services;

public sealed class ImageRuntimeStateService : IImageRuntimeStateService
{
    private readonly IImageFrameService _imageFrameService;

    public ImageRuntimeStateService(IImageFrameService imageFrameService)
    {
        _imageFrameService = imageFrameService;
    }

    public byte[]? SourcePixels { get; private set; }

    public WriteableBitmap? BinaryBitmap { get; set; }

    public int SourceWidth { get; private set; }

    public int SourceHeight { get; private set; }

    public int SourceStride { get; private set; }

    public bool HasSourceImage => SourcePixels != null && SourceWidth > 0 && SourceHeight > 0;

    public LoadedImageFrame LoadImage(string path)
    {
        var frame = _imageFrameService.LoadBgraImage(path);
        SetFrame(frame);
        return frame;
    }

    public WriteableBitmap UpdateBinaryBitmap(byte[] pixels, int width, int height, int stride)
    {
        if (BinaryBitmap == null || BinaryBitmap.PixelWidth != width || BinaryBitmap.PixelHeight != height)
        {
            BinaryBitmap = _imageFrameService.CreateGray8Bitmap(width, height);
        }

        _imageFrameService.WriteGray8Pixels(BinaryBitmap, pixels, width, height, stride);
        return BinaryBitmap;
    }

    private void SetFrame(LoadedImageFrame frame)
    {
        SourcePixels = frame.SourcePixels;
        BinaryBitmap = frame.BinaryBitmap;
        SourceWidth = frame.Width;
        SourceHeight = frame.Height;
        SourceStride = frame.SourceStride;
    }

    public PartRuntimeImage? CreatePartRuntimeImage(int threshold2D)
    {
        return !HasSourceImage || SourcePixels == null
            ? null
            : new PartRuntimeImage(SourcePixels, SourceWidth, SourceHeight, SourceStride, threshold2D);
    }
}
