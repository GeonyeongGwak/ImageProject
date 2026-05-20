namespace WpfInspectionApp;

public partial class MainWindow
{
    private void AlignPanel_SearchTabSelectionChanged(object? sender, EventArgs e)
    {
        HandleUiChange(() =>
        {
            _viewModel.SetAlignSearchTabActive(AlignPanel.IsSearchTabActive);
            if (!_viewModel.IsAlignSearchActive)
            {
                _viewModel.DisableRoiDrawing();
            }
        });
    }

    private void SearchNumCombo_SelectionChanged()
    {
        HandleUiChange(() =>
        {
            UpdateModelFromUi();
            _viewModel.NormalizeAlignSearchSelection();
            UpdateActiveRoiUi();
            DrawRoiOverlays();
        });
    }

    private void SearchSizeBox_TextChanged()
    {
        HandleUiChange(() =>
        {
            UpdateModelFromUi();
            ResizeActiveRoiFromSearchInputs();
        });
    }

    private void UpdateMaskDensity()
    {
        AlignPanel.SetMaskDensity(_viewModel.CalculateMaskDensity());
    }

    private void SyncAlignSearchNumFromViewModel(int searchNum)
    {
        _applyingModel = true;
        AlignPanel.SetSearchNum(searchNum);
        _applyingModel = false;
    }

    private void UpdateActiveRoiUi()
    {
        ViewModel.RefreshModelBindings();
        ApplyRoiUiSyncState();
    }

    private void ResizeActiveRoiFromSearchInputs()
    {
        if (!_viewModel.ResizeActiveRoiFromSearchInputs(_imageRuntimeStateService.SourceWidth, _imageRuntimeStateService.SourceHeight))
        {
            DrawRoiOverlays();
            return;
        }

        RefreshRoiOverlaysAndThreshold();
    }

    private void SyncSearchSizeInputsFromActiveRoi()
    {
        ApplyRoiUiSyncState();
    }
}
