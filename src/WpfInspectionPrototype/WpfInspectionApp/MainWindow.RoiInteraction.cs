using System.Linq;
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

        // Drawing 모드가 켜져 있으면 기존처럼 ROI 그리기 시작.
        if (_roiCanvasViewModel.TryBegin(canvas, e.GetPosition(canvas), _imageRuntimeStateService.SourceWidth, _imageRuntimeStateService.SourceHeight, _viewModel.CurrentImageZoom))
        {
            e.Handled = true;
            return;
        }

        // Drawing 모드가 아니면 클릭 좌표를 source-space 로 환원해서 어느 Window ROI 안인지
        // hit-test. 매칭되는 게 있으면 오른쪽 트리에서 그 Window 를 선택 상태로 만든다.
        if (TrySelectWindowAtPoint(canvas, e.GetPosition(canvas)))
        {
            e.Handled = true;
            return;
        }

        e.Handled = TryBeginImagePan(canvas, e.GetPosition(this));
    }

    private bool TrySelectWindowAtPoint(Canvas canvas, Point canvasPoint)
    {
        var width = _imageRuntimeStateService.SourceWidth;
        var height = _imageRuntimeStateService.SourceHeight;
        if (width <= 0 || height <= 0)
        {
            return false;
        }

        // 캔버스 좌표 → 이미지 픽셀 좌표 (zoom + letterbox 포함) 변환.
        var display = _roiGeometryService.GetImageDisplayRect(
            canvas.ActualWidth,
            canvas.ActualHeight,
            width,
            height,
            _viewModel.CurrentImageZoom);
        if (display.IsEmpty)
        {
            return false;
        }

        var imagePoint = _roiGeometryService.ToImagePixel(canvasPoint, display, width, height);
        var sx = (int)imagePoint.X;
        var sy = (int)imagePoint.Y;

        // 클릭 지점이 들어가는 가장 작은 Window ROI 를 우선 선택 (중첩된 ROI 가 있을 경우
        // 안쪽 것이 우선되도록). 매칭이 없으면 false 반환해서 호출자가 pan 으로 fallback.
        var hit = _viewModel.Model.Part.Windows
            .Where(w => w.Roi.IsValid
                        && sx >= w.Roi.X
                        && sx < w.Roi.X + w.Roi.Width
                        && sy >= w.Roi.Y
                        && sy < w.Roi.Y + w.Roi.Height)
            .OrderBy(w => w.Roi.Width * w.Roi.Height)
            .FirstOrDefault();

        return hit != null && _viewModel.TrySelectWindowById(hit.Id);
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

    private InspectionWindowData? ActiveWindow => ViewModel.ActiveWindow;

    private InspectionAlgorithmData? ActiveAlgorithm => ViewModel.ActiveAlgorithm;

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

        _viewModel.CommitDrawingRoi(
            surfacePoint,
            _imageRuntimeStateService.SourceWidth,
            _imageRuntimeStateService.SourceHeight,
            _viewModel.CurrentImageZoom,
            FormatRoi);

        SyncSearchSizeInputsFromActiveRoi();
        RefreshRoiOverlaysAndThreshold();
    }
}
