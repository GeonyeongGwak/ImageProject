namespace WpfInspectionApp;

public partial class MainWindow
{
    private void TeachActiveRoiSize()
    {
        SyncSearchSizeInputsFromActiveRoi();
        ViewModel.StatusMessage = $"Teach active ROI size: {FormatRoi(_viewModel.GetActiveRoi())}";
    }

    private void CloseAlignPartTeaching()
    {
        UpdateModelFromUi();
        ViewModel.Model.PartTeachingStopRequested = true;
        AlignPanel.ClosePartTeaching();
        ViewModel.StatusMessage = "Align Part Teaching closed.";
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
        try
        {
            UpdatePartTeachingUi();
            UpdateModelFromUi();
            var teaching = _alignPartTeachingService.Apply(ViewModel.Model, useGerber, FormatRoi);
            if (!teaching.Success)
            {
                AlignPanel.SetPartTeachingStatus(teaching.Status);
                ViewModel.StatusMessage = "Align Part Teaching failed: Window ROI is required.";
                return;
            }

            ApplyModelAndRefreshView(teaching.SelectedWindowId, scheduleThreshold: false);

            AlignPanel.SetPartTeachingStatus(teaching.Status);
            ViewModel.StatusMessage = $"Align Part Teaching completed: {teaching.TaughtCount} Window(s).";
            ViewModel.InspectionResultText = teaching.Summary;

            await _viewModel.RunThresholdAsync(refreshTreeOnAlgorithmUpdate: true);
        }
        catch (Exception ex)
        {
            DiagnosticsLog.Write($"Align Part Teaching failed: {ex}");
            AlignPanel.SetPartTeachingStatus($"Part Teaching failed: {ex.Message}");
            ViewModel.StatusMessage = $"Align Part Teaching failed: {ex.Message}";
        }
    }
}
