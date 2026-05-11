using System.Windows.Media.Imaging;

namespace WpfInspectionApp.Services;

public sealed record LoadedImageFrame(
    BitmapSource SourceBitmap,
    byte[] SourcePixels,
    WriteableBitmap BinaryBitmap,
    int Width,
    int Height,
    int SourceStride);

public interface IImageFrameService
{
    LoadedImageFrame LoadBgraImage(string path);

    WriteableBitmap CreateGray8Bitmap(int width, int height, double dpiX = 96.0, double dpiY = 96.0);

    void WriteGray8Pixels(WriteableBitmap bitmap, byte[] pixels, int width, int height, int stride);
}
