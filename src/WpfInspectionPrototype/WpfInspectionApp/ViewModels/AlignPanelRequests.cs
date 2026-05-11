namespace WpfInspectionApp.ViewModels;

[Flags]
public enum AlignPanelUpdateEffect
{
    None = 0,
    Model = 1,
    MaskDensity = 2,
    Threshold = 4,
    RoiText = 8,
    AlgorithmPanels = 16,
    RoiDrawButton = 32,
    RoiOverlays = 64,
    ActiveRoiUi = 128,
    PartTeachingUi = 256
}

public enum AlignPanelUpdateKind
{
    SearchNum,
    SearchParameter,
    SearchSize,
    Parameter,
    AlignParameter,
    Threshold2D,
    Threshold3D,
    EdgeGain,
    IpcClass,
    PartTeachingOption
}

public enum AlignPanelActionKind
{
    ActiveRoi,
    DrawWindowRoi,
    DrawAlgorithmRoi,
    Teach,
    PartTeachingIc,
    PartTeachingOk,
    PartTeachingClose
}

public sealed record AlignPanelUpdateRequestedEventArgs(AlignPanelUpdateKind Kind, object? Source);

public sealed record AlignPanelActionRequestedEventArgs(AlignPanelActionKind Kind);

public static class AlignPanelUpdateKindExtensions
{
    public static AlignPanelUpdateEffect ToEffect(this AlignPanelUpdateKind kind)
    {
        return kind switch
        {
            AlignPanelUpdateKind.SearchParameter => AlignPanelUpdateEffect.Model | AlignPanelUpdateEffect.RoiOverlays,
            AlignPanelUpdateKind.Parameter => AlignPanelUpdateEffect.Model | AlignPanelUpdateEffect.MaskDensity | AlignPanelUpdateEffect.Threshold,
            AlignPanelUpdateKind.AlignParameter => AlignPanelUpdateEffect.Model | AlignPanelUpdateEffect.RoiText,
            AlignPanelUpdateKind.Threshold2D => AlignPanelUpdateEffect.Model | AlignPanelUpdateEffect.Threshold,
            AlignPanelUpdateKind.Threshold3D => AlignPanelUpdateEffect.Model | AlignPanelUpdateEffect.MaskDensity,
            AlignPanelUpdateKind.EdgeGain => AlignPanelUpdateEffect.Model | AlignPanelUpdateEffect.MaskDensity,
            AlignPanelUpdateKind.IpcClass => AlignPanelUpdateEffect.Model,
            AlignPanelUpdateKind.PartTeachingOption => AlignPanelUpdateEffect.PartTeachingUi | AlignPanelUpdateEffect.Model,
            _ => AlignPanelUpdateEffect.None
        };
    }
}
