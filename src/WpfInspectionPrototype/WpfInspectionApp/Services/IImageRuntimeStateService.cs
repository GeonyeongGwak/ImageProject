using System.Windows.Media.Imaging;

namespace WpfInspectionApp.Services;

public interface IImageRuntimeStateService
{
    byte[]? SourcePixels { get; }

    WriteableBitmap? BinaryBitmap { get; set; }

    int SourceWidth { get; }

    int SourceHeight { get; }

    int SourceStride { get; }

    bool HasSourceImage { get; }

    LoadedImageFrame LoadImage(string path);

    WriteableBitmap UpdateBinaryBitmap(byte[] pixels, int width, int height, int stride);

    PartRuntimeImage? CreatePartRuntimeImage(int threshold2D);
}
