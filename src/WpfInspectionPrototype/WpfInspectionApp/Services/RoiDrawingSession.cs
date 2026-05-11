using System.Windows;
using System.Windows.Controls;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class RoiDrawingSession
{
    private readonly IRoiGeometryService _geometryService;
    private Canvas? _surface;
    private Point? _startPixel;

    public RoiDrawingSession(IRoiGeometryService geometryService)
    {
        _geometryService = geometryService;
    }

    public bool IsDrawing => _surface != null && _startPixel.HasValue;

    public Canvas? Surface => _surface;

    public void Begin(Canvas surface, Point surfacePoint, int imageWidth, int imageHeight, double zoom)
    {
        _surface = surface;
        _startPixel = ToImagePixel(surfacePoint, surface, imageWidth, imageHeight, zoom);
        surface.CaptureMouse();
    }

    public RoiRect? Preview(Point surfacePoint, int imageWidth, int imageHeight, double zoom)
    {
        if (_surface == null || !_startPixel.HasValue)
        {
            return null;
        }

        var current = ToImagePixel(surfacePoint, _surface, imageWidth, imageHeight, zoom);
        return _geometryService.NormalizeRoi(_startPixel.Value, current);
    }

    public RoiRect? Commit(Point surfacePoint, int imageWidth, int imageHeight, double zoom)
    {
        if (_surface == null || !_startPixel.HasValue)
        {
            return null;
        }

        var current = ToImagePixel(surfacePoint, _surface, imageWidth, imageHeight, zoom);
        var roi = _geometryService.NormalizeRoi(_startPixel.Value, current);
        Reset();
        return roi;
    }

    public void Reset()
    {
        _surface?.ReleaseMouseCapture();
        _surface = null;
        _startPixel = null;
    }

    private Point ToImagePixel(Point surfacePoint, FrameworkElement surface, int imageWidth, int imageHeight, double zoom)
    {
        var display = _geometryService.GetImageDisplayRect(surface.ActualWidth, surface.ActualHeight, imageWidth, imageHeight, zoom);
        return _geometryService.ToImagePixel(surfacePoint, display, imageWidth, imageHeight);
    }
}
