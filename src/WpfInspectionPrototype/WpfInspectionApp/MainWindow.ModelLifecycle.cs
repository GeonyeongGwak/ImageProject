namespace WpfInspectionApp;

public partial class MainWindow
{
    private void LoadPtt(string path, bool prepareMpti = true)
    {
        var pttViewerPanel = EnsurePttViewerPanel();
        var result = _pttViewerWorkflowService.LoadIntoControl(path, pttViewerPanel, prepareMpti);
        if (_pttViewerHost != null)
        {
            _pttViewerHost.Visibility = result.Success ? System.Windows.Visibility.Visible : System.Windows.Visibility.Collapsed;
        }

        ViewModel.ApplyPttLoad(result.Success, result.Path, result.StatusMessage);
    }

    private void UpdateModelFromUi()
    {
        _viewModel.ApplyAlignPanelState(
            AlignPanel.CaptureModelState(ViewModel.SelectedAlgorithm),
            _imageRuntimeStateService.SourceWidth,
            _imageRuntimeStateService.SourceHeight);
        AlignPanel.LoadState(_viewModel.CreateAlignPanelModelState());
    }

    private void ApplyModelToUi()
    {
        _applyingModel = true;
        _viewModel.NormalizeModelForView();
        AlignPanel.LoadState(_viewModel.CreateAlignPanelModelState());

        _applyingModel = false;
        _viewModel.UpdateAlgorithmPanels();
        UpdateMaskDensity();
        UpdateActiveRoiUi();
        UpdateRoiText();
        UpdatePartTeachingUi();
    }

    private void ApplyModelAndRefreshView(string? selectedId = null, bool scheduleThreshold = true)
    {
        ApplyModelToUi();
        if (scheduleThreshold)
        {
            RefreshInspectionViewAndThreshold(selectedId);
            return;
        }

        RefreshInspectionView(selectedId);
    }

    private void RefreshInspectionView(string? selectedId = null)
    {
        _viewModel.RefreshInspectionTree(selectedId);
        DrawRoiOverlays();
    }

    private void RefreshInspectionViewAndThreshold(string? selectedId = null)
    {
        RefreshInspectionView(selectedId);
        ScheduleThreshold();
    }

    private void RefreshRoiOverlaysAndThreshold()
    {
        DrawRoiOverlays();
        ScheduleThreshold();
    }
}
