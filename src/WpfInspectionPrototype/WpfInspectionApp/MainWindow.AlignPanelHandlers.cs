using System.Windows.Controls;

namespace WpfInspectionApp;

public partial class MainWindow
{
    private void AlignTabControl_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (sender != AlignPanel.AlignTabControl)
        {
            return;
        }

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
            ViewModel.Model.AlignSearchNum = Net48Compat.Clamp(ViewModel.Model.AlignSearchNum, 1, 4);
            ViewModel.Model.AlignActiveRoiIndex = Math.Min(ViewModel.Model.AlignActiveRoiIndex, ViewModel.Model.AlignSearchNum - 1);
            UpdateActiveRoiUi();
            DrawRoiOverlays();
        });
    }

    private void SearchSizeBox_TextChanged(object? source)
    {
        if (_syncingSearchSize)
        {
            return;
        }

        HandleUiChange(() =>
        {
            UpdateModelFromUi();

            _syncingSearchSize = true;
            AlignPanel.MirrorSearchSizeInput(source, ViewModel.Model);
            _syncingSearchSize = false;

            ResizeActiveRoiFromSearchInputs();
        });
    }

    private void UpdateMaskDensity()
    {
        var maskDensity = _alignConditionService.CalculateMaskDensity(ViewModel.Model);
        AlignPanel.SetMaskDensity(maskDensity);
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
