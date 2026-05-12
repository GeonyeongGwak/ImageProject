using System.Windows;
using System.Windows.Controls;
using WpfInspectionApp.Models;
using WpfInspectionApp.Services;

namespace WpfInspectionApp.ViewModels;

public sealed class RoiCanvasViewModel
{
    private readonly IRoiInteractionService _roiInteractionService;
    private readonly IRoiModelService _roiModelService;

    public RoiCanvasViewModel(IRoiInteractionService roiInteractionService, IRoiModelService roiModelService)
    {
        _roiInteractionService = roiInteractionService;
        _roiModelService = roiModelService;
    }

    public RoiRect? PreviewRoi => _roiInteractionService.PreviewRoi;

    public RoiDrawTarget Target => _roiInteractionService.Target;

    public bool IsEnabled => _roiInteractionService.IsEnabled;

    public bool IsDrawing => _roiInteractionService.IsDrawing;

    public Canvas? DrawingSurface => _roiInteractionService.DrawingSurface;

    public void Enable(RoiDrawTarget target)
    {
        _roiInteractionService.Enable(target);
    }

    public void Disable()
    {
        _roiInteractionService.Disable();
    }

    public void ResetDrawing()
    {
        _roiInteractionService.ResetDrawing();
    }

    public bool TryBegin(Canvas surface, Point surfacePoint, int imageWidth, int imageHeight, double zoom)
    {
        return _roiInteractionService.TryBegin(surface, surfacePoint, imageWidth, imageHeight, zoom);
    }

    public RoiRect? Preview(Point surfacePoint, int imageWidth, int imageHeight, double zoom)
    {
        return _roiInteractionService.Preview(surfacePoint, imageWidth, imageHeight, zoom);
    }

    public RoiModelOperationResult CommitToModel(
        InspectionModel model,
        Point surfacePoint,
        int imageWidth,
        int imageHeight,
        double zoom,
        string selectedAlgorithm,
        Func<RoiRect?, string> formatRoi)
    {
        if (!_roiInteractionService.IsDrawing)
        {
            return NoChange();
        }

        var commit = _roiInteractionService.Commit(surfacePoint, imageWidth, imageHeight, zoom);
        if (!commit.Roi.HasValue)
        {
            return NoChange();
        }

        return commit.Target == RoiDrawTarget.Algorithm
            ? _roiModelService.SetActiveAlgorithmRoi(model, selectedAlgorithm, commit.Roi.Value, formatRoi)
            : _roiModelService.CreateWindowFromRoi(model, commit.Roi.Value);
    }

    public RoiRect? GetActiveRoi(InspectionModel model)
    {
        return _roiModelService.GetActiveRoi(model, PreviewRoi);
    }

    public RoiRect? GetActiveInspectionRoi(InspectionModel model, string selectedAlgorithm)
    {
        return _roiModelService.GetActiveInspectionRoi(model, selectedAlgorithm, PreviewRoi);
    }

    public InspectionWindowData? GetActiveWindow(InspectionModel model)
    {
        return _roiModelService.GetActiveWindow(model);
    }

    public InspectionAlgorithmData? GetActiveAlgorithm(InspectionModel model, string selectedAlgorithm)
    {
        return _roiModelService.GetActiveAlgorithm(model, selectedAlgorithm);
    }

    public RoiModelOperationResult UpsertActiveWindow(InspectionModel model, RoiRect roi)
    {
        return _roiModelService.UpsertActiveWindow(model, roi);
    }

    public RoiModelOperationResult SelectNextWindow(InspectionModel model)
    {
        return _roiModelService.SelectNextWindow(model);
    }

    public RoiModelOperationResult DeleteActiveWindow(InspectionModel model)
    {
        return _roiModelService.DeleteActiveWindow(model);
    }

    public RoiModelOperationResult ResizeActiveRoiFromSearchInputs(InspectionModel model, int sourceWidth, int sourceHeight)
    {
        return _roiModelService.ResizeActiveRoiFromSearchInputs(model, PreviewRoi, sourceWidth, sourceHeight);
    }

    public RoiRect? SyncSearchSizeFromActiveRoi(InspectionModel model)
    {
        return _roiModelService.SyncSearchSizeFromActiveRoi(model, PreviewRoi);
    }

    public string FormatActiveRoiText(
        InspectionModel model,
        string selectedAlgorithm,
        int sourceWidth,
        int sourceHeight,
        Func<RoiRect?, string> formatRoi)
    {
        return _roiModelService.FormatActiveRoiText(model, selectedAlgorithm, PreviewRoi, sourceWidth, sourceHeight, formatRoi);
    }

    private static RoiModelOperationResult NoChange()
    {
        return new RoiModelOperationResult(null, null, null, Changed: false);
    }
}
