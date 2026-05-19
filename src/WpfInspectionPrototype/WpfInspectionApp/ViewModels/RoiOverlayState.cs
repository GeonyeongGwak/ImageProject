using WpfInspectionApp.Models;

namespace WpfInspectionApp.ViewModels;

public sealed record RoiOverlayState(
    InspectionModel Model,
    string? ActiveAlgorithmId,
    RoiRect? PreviewRoi,
    bool PreviewIsAlgorithmRoi,
    int SourceWidth,
    int SourceHeight,
    double ImageZoom);
