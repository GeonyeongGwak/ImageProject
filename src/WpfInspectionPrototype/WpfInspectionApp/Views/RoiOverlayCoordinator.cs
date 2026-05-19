using System.Windows;
using System.Windows.Controls;
using WpfInspectionApp.Services;
using WpfInspectionApp.ViewModels;

namespace WpfInspectionApp.Views;

public sealed class RoiOverlayCoordinator
{
    private readonly IRoiGeometryService _geometryService;

    public RoiOverlayCoordinator(IRoiGeometryService geometryService)
    {
        _geometryService = geometryService;
    }

    public void RenderAll(RoiOverlayState state, params Canvas[] overlays)
    {
        foreach (var overlay in overlays)
        {
            Render(overlay, state);
        }
    }

    public Rect GetImageDisplayRect(FrameworkElement surface, RoiOverlayState state)
    {
        return _geometryService.GetImageDisplayRect(
            surface.ActualWidth,
            surface.ActualHeight,
            state.SourceWidth,
            state.SourceHeight,
            state.ImageZoom);
    }

    private void Render(Canvas overlay, RoiOverlayState state)
    {
        overlay.Children.Clear();

        if (state.SourceWidth <= 0 || state.SourceHeight <= 0)
        {
            return;
        }

        var display = GetImageDisplayRect(overlay, state);
        if (display.IsEmpty)
        {
            return;
        }

        RoiOverlayRenderer.Render(overlay, new RoiOverlayRenderContext
        {
            Model = state.Model,
            Display = display,
            ActiveAlgorithmId = state.ActiveAlgorithmId,
            PreviewRoi = state.PreviewRoi,
            PreviewIsAlgorithmRoi = state.PreviewIsAlgorithmRoi,
            CreateMarginRoi = roi => _geometryService.ExpandRoi(roi, state.Model.AlignSearchMargin, state.SourceWidth, state.SourceHeight),
            ToDisplayRect = roi => _geometryService.ToDisplayRect(roi, display, state.SourceWidth, state.SourceHeight)
        });
    }
}
