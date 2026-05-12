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

        e.Handled = _roiCanvasViewModel.TryBegin(canvas, e.GetPosition(canvas), _imageRuntimeStateService.SourceWidth, _imageRuntimeStateService.SourceHeight, _viewModel.CurrentImageZoom);
    }

    private void ImageOverlay_MouseMove(object sender, MouseEventArgs e)
    {
        if (!_roiCanvasViewModel.IsDrawing || _roiCanvasViewModel.DrawingSurface == null)
        {
            return;
        }

        _roiCanvasViewModel.Preview(e.GetPosition(_roiCanvasViewModel.DrawingSurface), _imageRuntimeStateService.SourceWidth, _imageRuntimeStateService.SourceHeight, _viewModel.CurrentImageZoom);
        DrawRoiOverlays();
    }

    private void ImageOverlay_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
    {
        if (!_roiCanvasViewModel.IsDrawing || _roiCanvasViewModel.DrawingSurface == null)
        {
            return;
        }

        CommitCurrentDrawingRoi(e.GetPosition(_roiCanvasViewModel.DrawingSurface));
    }

    private void Overlay_SizeChanged(object sender, SizeChangedEventArgs e)
    {
        DrawRoiOverlays();
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
