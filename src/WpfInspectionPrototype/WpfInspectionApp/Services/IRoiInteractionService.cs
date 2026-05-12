using System.Windows;
using System.Windows.Controls;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public interface IRoiInteractionService
{
    bool IsEnabled { get; }
    RoiDrawTarget Target { get; }
    RoiRect? PreviewRoi { get; }
    Canvas? DrawingSurface { get; }
    bool IsDrawing { get; }

    void Enable(RoiDrawTarget target);
    void Disable();
    bool TryBegin(Canvas surface, Point surfacePoint, int imageWidth, int imageHeight, double zoom);
    RoiRect? Preview(Point surfacePoint, int imageWidth, int imageHeight, double zoom);
    RoiCommitResult Commit(Point surfacePoint, int imageWidth, int imageHeight, double zoom);
    void ResetDrawing();
}
