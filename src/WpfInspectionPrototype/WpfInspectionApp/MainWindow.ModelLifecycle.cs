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
        ViewModel.Model.EnsureStructure();
        ViewModel.Model.Part.Name = ViewModel.Model.ModelName;
        AlignPanel.ApplyToModel(ViewModel.Model, SelectedAlgorithm(), _imageRuntimeStateService.SourceWidth, _imageRuntimeStateService.SourceHeight);
    }

    private void ApplyModelToUi()
    {
        _applyingModel = true;
        ViewModel.Model.EnsureStructure();
        ViewModel.Model.AlignSearchNum = Net48Compat.Clamp(ViewModel.Model.AlignSearchNum, 1, 4);
        ViewModel.Model.AlignActiveRoiIndex = Net48Compat.Clamp(ViewModel.Model.AlignActiveRoiIndex, 0, ViewModel.Model.AlignSearchNum - 1);

        ViewModel.RefreshModelBindings();
        ViewModel.SelectedAlgorithm = ViewModel.Model.Algorithm;
        AlignPanel.LoadFromModel(ViewModel.Model);

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
