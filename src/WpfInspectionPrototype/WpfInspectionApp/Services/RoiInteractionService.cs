using System.Windows;
using System.Windows.Controls;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public enum RoiDrawTarget
{
    Window,
    Algorithm
}

public sealed record RoiCommitResult(RoiDrawTarget Target, RoiRect? Roi);

public sealed class RoiInteractionService : IRoiInteractionService
{
    private readonly RoiDrawingSession _drawingSession;

    public RoiInteractionService(IRoiGeometryService geometryService)
    {
        _drawingSession = new RoiDrawingSession(geometryService);
    }

    public bool IsEnabled { get; private set; }

    public RoiDrawTarget Target { get; private set; } = RoiDrawTarget.Window;

    public RoiRect? PreviewRoi { get; private set; }

    public Canvas? DrawingSurface => _drawingSession.Surface;

    public bool IsDrawing => _drawingSession.IsDrawing;

    public void Enable(RoiDrawTarget target)
    {
        Target = target;
        IsEnabled = true;
    }

    public void Disable()
    {
        IsEnabled = false;
        ResetDrawing();
    }

    public bool TryBegin(Canvas surface, Point surfacePoint, int imageWidth, int imageHeight, double zoom)
    {
        if (!IsEnabled || imageWidth <= 0 || imageHeight <= 0)
        {
            return false;
        }

        _drawingSession.Begin(surface, surfacePoint, imageWidth, imageHeight, zoom);
        return true;
    }

    public RoiRect? Preview(Point surfacePoint, int imageWidth, int imageHeight, double zoom)
    {
        PreviewRoi = _drawingSession.Preview(surfacePoint, imageWidth, imageHeight, zoom);
        return PreviewRoi;
    }

    public RoiCommitResult Commit(Point surfacePoint, int imageWidth, int imageHeight, double zoom)
    {
        var target = Target;
        var roi = _drawingSession.Commit(surfacePoint, imageWidth, imageHeight, zoom);
        PreviewRoi = null;
        return new RoiCommitResult(target, roi);
    }

    public void ResetDrawing()
    {
        _drawingSession.Reset();
        PreviewRoi = null;
    }
}
