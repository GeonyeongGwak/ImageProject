using WpfInspectionApp.Models;
using WpfInspectionApp.Services;
using WpfInspectionApp.Views;

namespace WpfInspectionApp;

public partial class MainWindow
{
    private void DrawRoiOverlays()
    {
        _roiOverlayCoordinator.RenderAll(CreateRoiOverlayState(), SourceOverlay, BinaryOverlay);
        UpdateRoiText();
    }

    private RoiOverlayState CreateRoiOverlayState()
    {
        return new RoiOverlayState(
            ViewModel.Model,
            ActiveAlgorithm?.Id,
            _roiCanvasViewModel.PreviewRoi,
            _roiCanvasViewModel.Target == RoiDrawTarget.Algorithm,
            _imageRuntimeStateService.SourceWidth,
            _imageRuntimeStateService.SourceHeight,
            _viewModel.CurrentImageZoom);
    }

    private void UpdateRoiText()
    {
        var text = _roiUiStateService.CreateRoiText(
            ViewModel.Model,
            SelectedAlgorithm(),
            _roiCanvasViewModel.PreviewRoi,
            _imageRuntimeStateService.SourceWidth,
            _imageRuntimeStateService.SourceHeight,
            FormatRoi);
        AlignPanel.SetRoiText(text);
    }

    private void ApplyRoiUiSyncState()
    {
        var state = _roiUiStateService.CreateSyncState(
            ViewModel.Model,
            SelectedAlgorithm(),
            _roiCanvasViewModel.PreviewRoi,
            _imageRuntimeStateService.SourceWidth,
            _imageRuntimeStateService.SourceHeight,
            FormatRoi);

        AlignPanel.SetActiveRoiText(state.ActiveRoiText);
        if (state.SearchSizeRoi.HasValue)
        {
            _syncingSearchSize = true;
            AlignPanel.SetSearchSize(state.SearchSizeRoi.Value.Width, state.SearchSizeRoi.Value.Height);
            _syncingSearchSize = false;
        }

        AlignPanel.SetRoiText(state.RoiText);
    }

    private string FormatRoi(RoiRect? roi)
    {
        if (!roi.HasValue)
        {
            return "none";
        }

        var value = roi.Value;
        var centerX = value.X + value.Width / 2;
        var centerY = value.Y + value.Height / 2;
        return $"X {value.X} Y {value.Y} W {value.Width} H {value.Height} | CX {centerX} CY {centerY} | ORG {_imageRuntimeStateService.SourceWidth}x{_imageRuntimeStateService.SourceHeight}";
    }
}
