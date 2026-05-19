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
            _viewModel.NormalizeAlignSearchSelection();
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
            _syncingSearchSize = true;
            AlignPanel.MirrorSearchSizeInput(source);
            _syncingSearchSize = false;

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
