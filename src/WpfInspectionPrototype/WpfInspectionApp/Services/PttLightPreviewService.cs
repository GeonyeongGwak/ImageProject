using System.Windows.Media;
using System.Windows.Media.Imaging;
using WpfInspectionApp.Interop;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class PttLightPreviewService : IPttLightPreviewService
{
    private readonly IImageFrameService _imageFrameService;

    public PttLightPreviewService(IImageFrameService imageFrameService)
    {
        _imageFrameService = imageFrameService;
    }

    public PttLightPreviewResult Render(AlgorithmLightState state, int width, int height)
    {
        var native = MptiNativeBridge.RenderLightPreview(state, width, height);
        if (!native.Success)
        {
            DiagnosticsLog.Write($"PTT light preview failed: available={native.Available}, code={native.Code}, message={native.Message}");
            return new PttLightPreviewResult(false, native.Available, native.Message, null);
        }

        var frame = CreateFrame(native.GrayPixels, native.Width, native.Height);
        return new PttLightPreviewResult(true, native.Available, native.Message, frame);
    }

    // CAM-01 컬러 미리보기. native 가 BGRA32 인터리브 버퍼를 채워주고, 여기서 frozen
    // BitmapSource 로 감싸기만 한다. Light 슬라이더와 무관하므로 PTT 로드 시 1 회만 호출됨.
    public PttColorPreviewResult RenderColor(int width, int height)
    {
        var native = MptiNativeBridge.RenderColorPreview(width, height);
        if (!native.Success)
        {
            DiagnosticsLog.Write($"PTT color preview failed: available={native.Available}, code={native.Code}, message={native.Message}");
            return new PttColorPreviewResult(false, native.Available, native.Message, null, 0, 0);
        }

        var stride = width * 4;
        var bitmap = BitmapSource.Create(
            width,
            height,
            96.0,
            96.0,
            PixelFormats.Bgra32,
            null,
            native.BgraPixels,
            stride);
        bitmap.Freeze();

        return new PttColorPreviewResult(true, native.Available, native.Message, bitmap, width, height);
    }

    private LoadedImageFrame CreateFrame(byte[] grayPixels, int width, int height)
    {
        var sourceStride = width * 4;
        var sourcePixels = new byte[sourceStride * height];
        for (var y = 0; y < height; y++)
        {
            var grayRow = y * width;
            var sourceRow = y * sourceStride;
            for (var x = 0; x < width; x++)
            {
                var gray = grayPixels[grayRow + x];
                var offset = sourceRow + x * 4;
                sourcePixels[offset] = gray;
                sourcePixels[offset + 1] = gray;
                sourcePixels[offset + 2] = gray;
                sourcePixels[offset + 3] = 255;
            }
        }

        var sourceBitmap = BitmapSource.Create(
            width,
            height,
            96.0,
            96.0,
            PixelFormats.Bgra32,
            null,
            sourcePixels,
            sourceStride);
        sourceBitmap.Freeze();

        var binaryBitmap = _imageFrameService.CreateGray8Bitmap(width, height);
        return new LoadedImageFrame(sourceBitmap, sourcePixels, binaryBitmap, width, height, sourceStride);
    }
}
