namespace WpfInspectionApp.Services;

public enum AlignPanelInspectionToggle
{
    Use2D,
    Use3D,
    UseEdge
}

public enum AlignPanelCorrectionToggle
{
    Shift,
    Angle
}

public readonly record struct AlignPanelInspectionState(bool Use2D, bool Use3D, bool UseEdge);

public readonly record struct AlignPanelCorrectionState(bool ShiftEnabled, bool AngleEnabled);

public interface IAlignPanelStateService
{
    AlignPanelInspectionState KeepInspectionSourceEnabled(bool use2D, bool use3D, bool useEdge, AlignPanelInspectionToggle changedToggle);

    AlignPanelCorrectionState KeepCorrectionOptionEnabled(bool shiftEnabled, bool angleEnabled, AlignPanelCorrectionToggle changedToggle);

    string NormalizeIntegerText(string? text, string fallback, int min, int max);

    string NormalizeDoubleText(string? text, string fallback, double min, double max, string format);

    string NormalizeIpcClass(string? value);
}
