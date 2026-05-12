using WpfInspectionApp.Models;
using WpfInspectionApp.Services;

namespace WpfInspectionApp;

public partial class MainWindow
{
    private void OnModelLoaded(InspectionModel loadedModel, string statusMessage)
    {
        ViewModel.Model = loadedModel;
        ApplyModelAndRefreshView();
        ViewModel.StatusMessage = statusMessage;
    }

    private void ImportPartFromPath(string path)
    {
        try
        {
            UpdateModelFromUi();
            var result = _partImportWorkflowService.ImportIntoModel(ViewModel.Model, path);
            if (!result.Success)
            {
                ViewModel.StatusMessage = result.StatusMessage;
                return;
            }

            ApplyImportedPart(result);
            var loadedImportImage = false;
            if (!string.IsNullOrWhiteSpace(result.PttPath))
            {
                LoadPtt(result.PttPath!, prepareMpti: false);
            }

            if (!string.IsNullOrWhiteSpace(result.ImagePath))
            {
                LoadImage(result.ImagePath!);
                loadedImportImage = true;
            }

            if (!loadedImportImage)
            {
                RefreshRoiOverlaysAndThreshold();
            }

            ViewModel.StatusMessage = result.StatusMessage;
        }
        catch (Exception ex)
        {
            DiagnosticsLog.Write($"Part import failed: {ex}");
            ViewModel.StatusMessage = $"Part import failed: {ex.Message}";
        }
    }

    private void ApplyImportedPart(PartImportWorkflowResult result)
    {
        _roiCanvasViewModel.ResetDrawing();
        _viewModel.DisableRoiDrawing();
        ApplyModelAndRefreshView(result.SelectedWindowId, scheduleThreshold: false);
        if (!string.IsNullOrWhiteSpace(result.Summary))
        {
            ViewModel.InspectionResultText = result.Summary!;
        }
    }

    private void LoadImage(string path)
    {
        var result = _imageLoadWorkflowService.Load(path);
        if (!result.Success || result.SourceImage == null || result.BinaryImage == null)
        {
            ViewModel.StatusMessage = result.StatusMessage;
            return;
        }

        ViewModel.ApplyImageLoad(result.SourceImage, result.BinaryImage, result.Width, result.Height, result.StatusMessage);
        RefreshRoiOverlaysAndThreshold();
    }

    private void LoadPtt(string path, bool prepareMpti = true)
    {
        var result = _pttViewerWorkflowService.LoadIntoControl(path, PttViewerPanel, prepareMpti);
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
        RefreshInspectionTree(selectedId);
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
