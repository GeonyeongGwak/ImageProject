using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace WpfInspectionApp.Services;

public sealed class ImageFrameService : IImageFrameService
{
    public LoadedImageFrame LoadBgraImage(string path)
    {
        var bitmap = new BitmapImage();
        bitmap.BeginInit();
        bitmap.UriSource = new Uri(path, UriKind.Absolute);
        bitmap.CacheOption = BitmapCacheOption.OnLoad;
        bitmap.CreateOptions = BitmapCreateOptions.PreservePixelFormat;
        bitmap.EndInit();
        bitmap.Freeze();

        var converted = new FormatConvertedBitmap(bitmap, PixelFormats.Bgra32, null, 0);
        converted.Freeze();

        var width = converted.PixelWidth;
        var height = converted.PixelHeight;
        var stride = CalculateStride(width, converted.Format.BitsPerPixel);
        var pixels = new byte[stride * height];
        converted.CopyPixels(pixels, stride, 0);

        var binaryBitmap = CreateGray8Bitmap(width, height, converted.DpiX, converted.DpiY);
        return new LoadedImageFrame(converted, pixels, binaryBitmap, width, height, stride);
    }

    public WriteableBitmap CreateGray8Bitmap(int width, int height, double dpiX = 96.0, double dpiY = 96.0)
    {
        return new WriteableBitmap(width, height, dpiX, dpiY, PixelFormats.Gray8, null);
    }

    public void WriteGray8Pixels(WriteableBitmap bitmap, byte[] pixels, int width, int height, int stride)
    {
        bitmap.WritePixels(new Int32Rect(0, 0, width, height), pixels, stride, 0);
    }

    private static int CalculateStride(int width, int bitsPerPixel)
    {
        return ((width * bitsPerPixel + 31) / 32) * 4;
    }
}
