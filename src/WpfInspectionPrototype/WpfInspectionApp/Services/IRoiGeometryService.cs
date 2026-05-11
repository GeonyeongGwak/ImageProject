using System.Windows;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public interface IRoiGeometryService
{
    Rect GetImageDisplayRect(double surfaceWidth, double surfaceHeight, int imageWidth, int imageHeight, double zoom);

    Point ToImagePixel(Point surfacePoint, Rect displayRect, int imageWidth, int imageHeight);

    RoiRect NormalizeRoi(Point start, Point end);

    Rect ToDisplayRect(RoiRect roi, Rect displayRect, int imageWidth, int imageHeight);

    RoiRect ExpandRoi(RoiRect roi, int margin, int imageWidth, int imageHeight);
}
