using System.Windows;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class RoiGeometryService : IRoiGeometryService
{
    public Rect GetImageDisplayRect(double surfaceWidth, double surfaceHeight, int imageWidth, int imageHeight, double zoom)
    {
        if (imageWidth <= 0 || imageHeight <= 0 || surfaceWidth <= 0 || surfaceHeight <= 0)
        {
            return Rect.Empty;
        }

        var normalizedZoom = Net48Compat.Clamp(zoom, 1.0, Math.Max(1.0, zoom));
        var scale = Math.Min(surfaceWidth / imageWidth, surfaceHeight / imageHeight);
        var width = imageWidth * scale * normalizedZoom;
        var height = imageHeight * scale * normalizedZoom;
        return new Rect((surfaceWidth - width) / 2, (surfaceHeight - height) / 2, width, height);
    }

    public Point ToImagePixel(Point surfacePoint, Rect displayRect, int imageWidth, int imageHeight)
    {
        if (displayRect.IsEmpty || imageWidth <= 0 || imageHeight <= 0)
        {
            return new Point();
        }

        var x = (surfacePoint.X - displayRect.Left) / displayRect.Width * imageWidth;
        var y = (surfacePoint.Y - displayRect.Top) / displayRect.Height * imageHeight;
        return new Point(
            Net48Compat.Clamp(x, 0, Math.Max(0, imageWidth - 1)),
            Net48Compat.Clamp(y, 0, Math.Max(0, imageHeight - 1)));
    }

    public RoiRect NormalizeRoi(Point start, Point end)
    {
        var x = (int)Math.Round(Math.Min(start.X, end.X));
        var y = (int)Math.Round(Math.Min(start.Y, end.Y));
        var right = (int)Math.Round(Math.Max(start.X, end.X));
        var bottom = (int)Math.Round(Math.Max(start.Y, end.Y));
        return new RoiRect(x, y, Math.Max(1, right - x), Math.Max(1, bottom - y));
    }

    public Rect ToDisplayRect(RoiRect roi, Rect displayRect, int imageWidth, int imageHeight)
    {
        if (displayRect.IsEmpty || imageWidth <= 0 || imageHeight <= 0)
        {
            return Rect.Empty;
        }

        var left = displayRect.Left + roi.X / (double)imageWidth * displayRect.Width;
        var top = displayRect.Top + roi.Y / (double)imageHeight * displayRect.Height;
        var width = roi.Width / (double)imageWidth * displayRect.Width;
        var height = roi.Height / (double)imageHeight * displayRect.Height;
        return new Rect(left, top, width, height);
    }

    public RoiRect ExpandRoi(RoiRect roi, int margin, int imageWidth, int imageHeight)
    {
        var normalizedMargin = Math.Max(0, margin);
        var x = Math.Max(0, roi.X - normalizedMargin);
        var y = Math.Max(0, roi.Y - normalizedMargin);
        var right = Math.Min(imageWidth, roi.X + roi.Width + normalizedMargin);
        var bottom = Math.Min(imageHeight, roi.Y + roi.Height + normalizedMargin);
        return new RoiRect(x, y, Math.Max(1, right - x), Math.Max(1, bottom - y));
    }
}
