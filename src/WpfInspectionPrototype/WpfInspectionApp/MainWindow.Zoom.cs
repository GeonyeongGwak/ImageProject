using System.Windows;
using System.Windows.Input;

namespace WpfInspectionApp;

public partial class MainWindow
{
    private void ImageOverlay_MouseWheel(object sender, MouseWheelEventArgs e)
    {
        if (!ViewModel.Model.WheelZoomEnabled || !_imageRuntimeStateService.HasSourceImage)
        {
            return;
        }

        var direction = e.Delta > 0 ? 1 : -1;
        var step = Net48Compat.Clamp(ViewModel.Model.WheelZoomStep, 0.01, 1.0);
        var next = ViewModel.Model.ImageZoom + direction * step;
        SetImageZoom(next);
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
        var nextZoom = Net48Compat.Clamp(zoom, 1.0, Math.Max(1.0, ViewModel.Model.WheelZoomMax));
        _applyingModel = true;
        ViewModel.ImageZoom = nextZoom;
        _applyingModel = false;
        DrawRoiOverlays();
    }
}
