namespace WpfInspectionApp;

public partial class MainWindow
{
    private void TeachActiveRoiSize()
    {
        SyncSearchSizeInputsFromActiveRoi();
        _viewModel.TeachActiveRoiSize(FormatRoi);
    }

    private void CloseAlignPartTeaching()
    {
        _viewModel.CloseAlignPartTeaching();
        AlignPanel.ClosePartTeaching();
    }

    private void UpdatePartTeachingUi()
    {
        var wasApplying = _applyingModel;
        _applyingModel = true;
        try
        {
            AlignPanel.UpdatePartTeachingUi();
        }
        finally
        {
            _applyingModel = wasApplying;
        }
    }

    private async void RunAlignPartTeaching(bool useGerber)
    {
        UpdatePartTeachingUi();
        await _viewModel.RunAlignPartTeachingAsync(useGerber, FormatRoi);
    }
}
