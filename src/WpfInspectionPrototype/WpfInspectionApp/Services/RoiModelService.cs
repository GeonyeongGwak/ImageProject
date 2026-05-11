using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class RoiModelService : IRoiModelService
{
    public RoiRect? GetActiveRoi(InspectionModel model, RoiRect? previewRoi)
    {
        return previewRoi ?? GetActiveWindow(model)?.Roi;
    }

    public RoiRect? GetActiveInspectionRoi(InspectionModel model, string selectedAlgorithm, RoiRect? previewRoi)
    {
        return previewRoi
            ?? GetActiveAlgorithm(model, selectedAlgorithm)?.AlgorithmRoi
            ?? GetActiveWindow(model)?.Roi;
    }

    public InspectionWindowData? GetActiveWindow(InspectionModel model)
    {
        model.EnsureStructure();
        return model.Part.Windows.FirstOrDefault(window => window.Id == model.SelectedWindowId)
            ?? model.Part.Windows.FirstOrDefault();
    }

    public InspectionAlgorithmData? GetActiveAlgorithm(InspectionModel model, string selectedAlgorithm)
    {
        var window = GetActiveWindow(model);
        if (window == null)
        {
            return null;
        }

        return window.Algorithms.LastOrDefault(item => string.Equals(item.Type, selectedAlgorithm, StringComparison.OrdinalIgnoreCase))
            ?? window.Algorithms.LastOrDefault();
    }

    public RoiModelOperationResult UpsertActiveWindow(InspectionModel model, RoiRect roi)
    {
        if (!roi.IsValid)
        {
            return NoChange();
        }

        model.EnsureStructure();
        var window = GetActiveWindow(model);
        if (window == null || model.SelectedWindowId == null)
        {
            window = new InspectionWindowData
            {
                Id = InspectionWindowData.CreateId(),
                Name = $"Window ROI {model.Part.Windows.Count + 1}",
                Roi = roi
            };
            model.Part.Windows.Add(window);
            model.SelectedWindowId = window.Id;
        }
        else
        {
            window.Roi = roi;
        }

        SyncAlignWindowSelection(model, window);
        return new RoiModelOperationResult(roi, window.Id, null, Changed: true);
    }

    public RoiModelOperationResult CreateWindowFromRoi(InspectionModel model, RoiRect roi)
    {
        if (!roi.IsValid)
        {
            return NoChange();
        }

        model.EnsureStructure();
        var window = new InspectionWindowData
        {
            Id = InspectionWindowData.CreateId(),
            Name = $"Window ROI {model.Part.Windows.Count + 1}",
            Roi = roi
        };
        model.Part.Windows.Add(window);
        model.SelectedWindowId = window.Id;
        SyncAlignWindowSelection(model, window);
        return new RoiModelOperationResult(roi, window.Id, $"{window.Name} created. Select it and press Algorithm Add.", Changed: true);
    }

    public RoiModelOperationResult SetActiveAlgorithmRoi(InspectionModel model, string selectedAlgorithm, RoiRect roi, Func<RoiRect?, string> formatRoi)
    {
        if (!roi.IsValid)
        {
            return NoChange();
        }

        var algorithm = GetActiveAlgorithm(model, selectedAlgorithm);
        if (algorithm == null)
        {
            return new RoiModelOperationResult(null, null, "Select or add an Algorithm before drawing Algorithm ROI.", Changed: false);
        }

        algorithm.AlgorithmRoi = roi;
        return new RoiModelOperationResult(roi, algorithm.Id, $"{algorithm.Type} Algorithm ROI assigned: {formatRoi(roi)}", Changed: true);
    }

    public RoiModelOperationResult SelectNextWindow(InspectionModel model)
    {
        model.EnsureStructure();
        if (model.Part.Windows.Count == 0)
        {
            return NoChange();
        }

        var currentIndex = Math.Max(0, model.Part.Windows.FindIndex(window => window.Id == model.SelectedWindowId));
        var nextIndex = (currentIndex + 1) % model.Part.Windows.Count;
        model.SelectedWindowId = model.Part.Windows[nextIndex].Id;
        model.AlignActiveRoiIndex = nextIndex;
        return new RoiModelOperationResult(model.Part.Windows[nextIndex].Roi, model.SelectedWindowId, null, Changed: true);
    }

    public RoiModelOperationResult DeleteActiveWindow(InspectionModel model)
    {
        var window = GetActiveWindow(model);
        if (window == null)
        {
            return NoChange();
        }

        model.Part.Windows.Remove(window);
        model.SelectedWindowId = model.Part.Windows.FirstOrDefault()?.Id;
        model.AlignSearchNum = Net48Compat.Clamp(model.Part.Windows.Count, 1, 4);
        model.AlignActiveRoiIndex = Math.Max(0, model.Part.Windows.FindIndex(candidate => candidate.Id == model.SelectedWindowId));
        return new RoiModelOperationResult(GetActiveWindow(model)?.Roi, model.SelectedWindowId, null, Changed: true);
    }

    public RoiModelOperationResult ResizeActiveRoiFromSearchInputs(InspectionModel model, RoiRect? previewRoi, int sourceWidth, int sourceHeight)
    {
        var activeRoi = GetActiveRoi(model, previewRoi);
        if (activeRoi == null || sourceWidth <= 0 || sourceHeight <= 0)
        {
            return NoChange();
        }

        var roi = activeRoi.Value;
        var centerX = roi.X + roi.Width / 2.0;
        var centerY = roi.Y + roi.Height / 2.0;
        var width = Net48Compat.Clamp(model.AlignSearchSizeX, 1, sourceWidth);
        var height = Net48Compat.Clamp(model.AlignSearchSizeY, 1, sourceHeight);
        var x = (int)Math.Round(centerX - width / 2.0);
        var y = (int)Math.Round(centerY - height / 2.0);
        x = Net48Compat.Clamp(x, 0, Math.Max(0, sourceWidth - width));
        y = Net48Compat.Clamp(y, 0, Math.Max(0, sourceHeight - height));
        return UpsertActiveWindow(model, new RoiRect(x, y, width, height));
    }

    public RoiRect? SyncSearchSizeFromActiveRoi(InspectionModel model, RoiRect? previewRoi)
    {
        var activeRoi = GetActiveRoi(model, previewRoi);
        if (activeRoi.HasValue)
        {
            model.AlignSearchSizeX = activeRoi.Value.Width;
            model.AlignSearchSizeY = activeRoi.Value.Height;
        }

        return activeRoi;
    }

    public string FormatActiveRoiText(
        InspectionModel model,
        string selectedAlgorithm,
        RoiRect? previewRoi,
        int sourceWidth,
        int sourceHeight,
        Func<RoiRect?, string> formatRoi)
    {
        var window = GetActiveWindow(model);
        var algorithm = GetActiveAlgorithm(model, selectedAlgorithm);
        return window != null
            ? $"{window.Name}: {formatRoi(window.Roi)}{Environment.NewLine}Algorithm ROI: {formatRoi(algorithm?.AlgorithmRoi)}"
            : sourceWidth > 0
                ? $"Window: No ROI | ORG {sourceWidth}x{sourceHeight}"
                : "Window: No ROI";
    }

    private static void SyncAlignWindowSelection(InspectionModel model, InspectionWindowData window)
    {
        model.AlignSearchNum = Net48Compat.Clamp(model.Part.Windows.Count, 1, 4);
        model.AlignActiveRoiIndex = Math.Max(0, model.Part.Windows.FindIndex(candidate => candidate.Id == window.Id));
    }

    private static RoiModelOperationResult NoChange()
    {
        return new RoiModelOperationResult(null, null, null, Changed: false);
    }
}
