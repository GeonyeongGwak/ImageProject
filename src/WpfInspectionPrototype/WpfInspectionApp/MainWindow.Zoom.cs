using System.Windows;
using System.Windows.Input;

namespace WpfInspectionApp;

public partial class MainWindow
{
    private void ImageOverlay_MouseWheel(object sender, MouseWheelEventArgs e)
    {
        if (!ViewModel.CanApplyWheelZoom)
        {
            return;
        }

        var next = ViewModel.CreateWheelZoom(e.Delta);
        if (sender is FrameworkElement surface)
        {
            SetImageZoomAtPoint(next, surface, e.GetPosition(surface));
        }
        else
        {
            SetImageZoom(next);
        }

        e.Handled = true;
    }

    private void ImageZoomSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
    {
        if (!_uiReady || _applyingModel)
        {
            return;
        }

        SetImageZoom(e.NewValue);
    }

    private void ZoomOne()
    {
        SetImageZoom(1.0);
    }

    private void ZoomFit()
    {
        SetImageZoom(1.0);
    }

    private void SetImageZoom(double zoom)
    {
        var nextZoom = ViewModel.ClampImageZoom(zoom);
        ApplyImageZoomValue(nextZoom);
        SetImagePan(
            nextZoom <= 1.0001 ? 0 : _imagePanX,
            nextZoom <= 1.0001 ? 0 : _imagePanY,
            SourceOverlay);
        DrawRoiOverlays();
    }

    private void SetImageZoomAtPoint(double zoom, FrameworkElement surface, Point anchorPoint)
    {
        var previousZoom = _viewModel.CurrentImageZoom;
        var nextZoom = ViewModel.ClampImageZoom(zoom);
        if (Math.Abs(previousZoom - nextZoom) < 0.0001)
        {
            return;
        }

        var previousDisplay = _roiGeometryService.GetImageDisplayRect(
            surface.ActualWidth,
            surface.ActualHeight,
            _imageRuntimeStateService.SourceWidth,
            _imageRuntimeStateService.SourceHeight,
            previousZoom);

        if (previousDisplay.IsEmpty)
        {
            SetImageZoom(nextZoom);
            return;
        }

        var imageRatioX = Net48Compat.Clamp((anchorPoint.X - previousDisplay.Left) / previousDisplay.Width, 0, 1);
        var imageRatioY = Net48Compat.Clamp((anchorPoint.Y - previousDisplay.Top) / previousDisplay.Height, 0, 1);
        var anchorSurfacePoint = new Point(anchorPoint.X + _imagePanX, anchorPoint.Y + _imagePanY);

        ApplyImageZoomValue(nextZoom);

        if (nextZoom <= 1.0001)
        {
            SetImagePan(0, 0, surface);
            DrawRoiOverlays();
            return;
        }

        var nextDisplay = _roiGeometryService.GetImageDisplayRect(
            surface.ActualWidth,
            surface.ActualHeight,
            _imageRuntimeStateService.SourceWidth,
            _imageRuntimeStateService.SourceHeight,
            nextZoom);

        if (nextDisplay.IsEmpty)
        {
            SetImagePan(0, 0, surface);
            DrawRoiOverlays();
            return;
        }

        var nextPanX = anchorSurfacePoint.X - nextDisplay.Left - imageRatioX * nextDisplay.Width;
        var nextPanY = anchorSurfacePoint.Y - nextDisplay.Top - imageRatioY * nextDisplay.Height;
        SetImagePan(nextPanX, nextPanY, surface);
        DrawRoiOverlays();
    }

    private void ApplyImageZoomValue(double zoom)
    {
        _applyingModel = true;
        ViewModel.ImageZoom = zoom;
        _applyingModel = false;
    }
}
