using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using WpfInspectionApp.Models;
using WpfInspectionApp.Services;

namespace WpfInspectionApp;

public partial class MainWindow
{
    private void ImageOverlay_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
    {
        if (!_imageRuntimeStateService.HasSourceImage || sender is not Canvas canvas)
        {
            return;
        }

        if (_roiCanvasViewModel.TryBegin(canvas, e.GetPosition(canvas), _imageRuntimeStateService.SourceWidth, _imageRuntimeStateService.SourceHeight, _viewModel.CurrentImageZoom))
        {
            e.Handled = true;
            return;
        }

        e.Handled = TryBeginImagePan(canvas, e.GetPosition(this));
    }

    private void ImageOverlay_MouseMove(object sender, MouseEventArgs e)
    {
        if (_isImagePanning)
        {
            UpdateImagePan(e.GetPosition(this));
            return;
        }

        if (!_roiCanvasViewModel.IsDrawing || _roiCanvasViewModel.DrawingSurface == null)
        {
            return;
        }

        _roiCanvasViewModel.Preview(e.GetPosition(_roiCanvasViewModel.DrawingSurface), _imageRuntimeStateService.SourceWidth, _imageRuntimeStateService.SourceHeight, _viewModel.CurrentImageZoom);
        DrawRoiOverlays();
    }

    private void ImageOverlay_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
    {
        if (_isImagePanning)
        {
            EndImagePan();
            e.Handled = true;
            return;
        }

        if (!_roiCanvasViewModel.IsDrawing || _roiCanvasViewModel.DrawingSurface == null)
        {
            return;
        }

        CommitCurrentDrawingRoi(e.GetPosition(_roiCanvasViewModel.DrawingSurface));
    }

    private void Overlay_SizeChanged(object sender, SizeChangedEventArgs e)
    {
        ClampImagePanToSurface(sender as Canvas);
        DrawRoiOverlays();
    }

    private bool TryBeginImagePan(Canvas canvas, Point windowPoint)
    {
        if (_roiCanvasViewModel.IsEnabled || _viewModel.CurrentImageZoom <= 1.0001)
        {
            return false;
        }

        _isImagePanning = true;
        _imagePanSurface = canvas;
        _imagePanStartPoint = windowPoint;
        _imagePanStartX = _imagePanX;
        _imagePanStartY = _imagePanY;
        canvas.CaptureMouse();
        canvas.Cursor = Cursors.SizeAll;
        return true;
    }

    private void UpdateImagePan(Point windowPoint)
    {
        if (!_isImagePanning || _imagePanSurface == null)
        {
            return;
        }

        var delta = windowPoint - _imagePanStartPoint;
        SetImagePan(_imagePanStartX + delta.X, _imagePanStartY + delta.Y, _imagePanSurface);
    }

    private void EndImagePan()
    {
        _imagePanSurface?.ReleaseMouseCapture();
        if (_imagePanSurface != null)
        {
            _imagePanSurface.Cursor = null;
        }

        _imagePanSurface = null;
        _isImagePanning = false;
    }

    private void ClampImagePanToSurface(Canvas? surface)
    {
        if (surface == null)
        {
            return;
        }

        SetImagePan(_imagePanX, _imagePanY, surface);
    }

    private void SetImagePan(double x, double y, FrameworkElement? surface)
    {
        var clamped = ClampImagePan(x, y, surface);
        _imagePanX = clamped.X;
        _imagePanY = clamped.Y;
        ApplyImagePan();
    }

    private Point ClampImagePan(double x, double y, FrameworkElement? surface)
    {
        if (surface == null || _viewModel.CurrentImageZoom <= 1.0001)
        {
            return new Point(0, 0);
        }

        var display = _roiGeometryService.GetImageDisplayRect(
            surface.ActualWidth,
            surface.ActualHeight,
            _imageRuntimeStateService.SourceWidth,
            _imageRuntimeStateService.SourceHeight,
            _viewModel.CurrentImageZoom);

        if (display.IsEmpty)
        {
            return new Point(0, 0);
        }

        var minX = display.Width > surface.ActualWidth
            ? surface.ActualWidth - display.Right
            : 0;
        var maxX = display.Width > surface.ActualWidth
            ? -display.Left
            : 0;
        var minY = display.Height > surface.ActualHeight
            ? surface.ActualHeight - display.Bottom
            : 0;
        var maxY = display.Height > surface.ActualHeight
            ? -display.Top
            : 0;

        return new Point(
            Net48Compat.Clamp(x, minX, maxX),
            Net48Compat.Clamp(y, minY, maxY));
    }

    private void ApplyImagePan()
    {
        SourceImagePanTransform.X = _imagePanX;
        SourceImagePanTransform.Y = _imagePanY;
        SourceOverlayPanTransform.X = _imagePanX;
        SourceOverlayPanTransform.Y = _imagePanY;
        BinaryImagePanTransform.X = _imagePanX;
        BinaryImagePanTransform.Y = _imagePanY;
        BinaryOverlayPanTransform.X = _imagePanX;
        BinaryOverlayPanTransform.Y = _imagePanY;
    }

    private RoiRect? ActiveRoi
    {
        get
        {
            return _roiCanvasViewModel.GetActiveRoi(ViewModel.Model);
        }
        set
        {
            if (value.HasValue)
            {
                ApplyRoiModelResult(_roiCanvasViewModel.UpsertActiveWindow(ViewModel.Model, value.Value));
            }
        }
    }

    private RoiRect? ActiveInspectionRoi => _roiCanvasViewModel.GetActiveInspectionRoi(ViewModel.Model, SelectedAlgorithm());

    private InspectionWindowData? ActiveWindow => _roiCanvasViewModel.GetActiveWindow(ViewModel.Model);

    private InspectionAlgorithmData? ActiveAlgorithm => _roiCanvasViewModel.GetActiveAlgorithm(ViewModel.Model, SelectedAlgorithm());

    private void ApplyRoiModelResult(RoiModelOperationResult result)
    {
        if (!result.Changed)
        {
            return;
        }

        if (!string.IsNullOrWhiteSpace(result.SelectedId))
        {
            _viewModel.RefreshInspectionTree(result.SelectedId);
        }

        if (!string.IsNullOrWhiteSpace(result.StatusMessage))
        {
            ViewModel.StatusMessage = result.StatusMessage!;
        }
    }

    private void CommitCurrentDrawingRoi()
    {
        if (!_roiCanvasViewModel.IsDrawing || _roiCanvasViewModel.DrawingSurface == null)
        {
            SyncSearchSizeInputsFromActiveRoi();
            DrawRoiOverlays();
            return;
        }

        CommitCurrentDrawingRoi(Mouse.GetPosition(_roiCanvasViewModel.DrawingSurface));
    }

    private void CommitCurrentDrawingRoi(Point surfacePoint)
    {
        if (!_roiCanvasViewModel.IsDrawing)
        {
            return;
        }

        ApplyRoiModelResult(_roiCanvasViewModel.CommitToModel(
            ViewModel.Model,
            surfacePoint,
            _imageRuntimeStateService.SourceWidth,
            _imageRuntimeStateService.SourceHeight,
            _viewModel.CurrentImageZoom,
            SelectedAlgorithm(),
            FormatRoi));

        SyncSearchSizeInputsFromActiveRoi();
        RefreshRoiOverlaysAndThreshold();
    }
}
