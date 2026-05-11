namespace WpfInspectionApp.ViewModels;

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
