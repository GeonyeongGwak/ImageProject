using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.ViewModels;

public sealed record AlignPanelModelState(
    string SelectedAlgorithm,
    int Threshold2D,
    int Threshold3D,
    int EdgeGain,
    bool Use2D,
    bool Use3D,
    bool UseEdge,
    string SearchNum,
    string SearchMargin,
    string SearchSizeX,
    string SearchSizeY,
    bool SameSize,
    bool ShiftEnabled,
    string ShiftX,
    string ShiftY,
    bool AngleEnabled,
    string Angle,
    bool FillHole,
    string Filter,
    bool InspectionAreaCount,
    bool IpcUse,
    string IpcClass,
    string IpcPercent,
    bool PartTeachingUseCommonLibrary,
    bool PartTeachingUseLibraryPart,
    bool PartTeachingUseAutoTeaching,
    bool PartTeachingUseCadMatching,
    string PartTeachingLibraryMatchMode)
{
    public static AlignPanelModelState FromModel(InspectionModel model, string selectedAlgorithm)
    {
        return new AlignPanelModelState(
            selectedAlgorithm,
            model.Threshold2D,
            model.Threshold3D,
            model.EdgeGain,
            model.Use2D,
            model.Use3D,
            model.UseEdge,
            Net48Compat.Clamp(model.AlignSearchNum, 1, 4).ToString(),
            model.AlignSearchMargin.ToString(),
            model.AlignSearchSizeX.ToString(),
            model.AlignSearchSizeY.ToString(),
            model.AlignSameSize,
            model.AlignShiftEnabled,
            model.AlignShiftX.ToString("0.00"),
            model.AlignShiftY.ToString("0.00"),
            model.AlignAngleEnabled,
            model.AlignAngle.ToString("0.##"),
            model.AlignFillHole,
            model.AlignFilter.ToString(),
            model.AlignInspectionAreaCount,
            model.IpcUse,
            model.IpcClass,
            model.IpcPercent.ToString("0.00"),
            model.PartTeachingUseCommonLibrary,
            model.PartTeachingUseLibraryPart,
            model.PartTeachingUseAutoTeaching,
            model.PartTeachingUseCadMatching,
            model.PartTeachingLibraryMatchMode);
    }
}
