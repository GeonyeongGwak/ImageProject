using System.Globalization;

namespace WpfInspectionApp.Services;

public sealed class AlignPanelStateService : IAlignPanelStateService
{
    public AlignPanelInspectionState KeepInspectionSourceEnabled(bool use2D, bool use3D, bool useEdge, AlignPanelInspectionToggle changedToggle)
    {
        if (use2D || use3D || useEdge)
        {
            return new AlignPanelInspectionState(use2D, use3D, useEdge);
        }

        return changedToggle switch
        {
            AlignPanelInspectionToggle.Use3D => new AlignPanelInspectionState(false, true, false),
            AlignPanelInspectionToggle.UseEdge => new AlignPanelInspectionState(false, false, true),
            _ => new AlignPanelInspectionState(true, false, false)
        };
    }

    public AlignPanelCorrectionState KeepCorrectionOptionEnabled(bool shiftEnabled, bool angleEnabled, AlignPanelCorrectionToggle changedToggle)
    {
        if (shiftEnabled || angleEnabled)
        {
            return new AlignPanelCorrectionState(shiftEnabled, angleEnabled);
        }

        return changedToggle == AlignPanelCorrectionToggle.Angle
            ? new AlignPanelCorrectionState(false, true)
            : new AlignPanelCorrectionState(true, false);
    }

    public string NormalizeIntegerText(string? text, string fallback, int min, int max)
    {
        if (max < min)
        {
            max = min;
        }

        var value = TryParseInteger(text, out var parsed)
            ? parsed
            : TryParseInteger(fallback, out var fallbackValue)
                ? fallbackValue
                : min;

        value = Math.Max(min, Math.Min(max, value));
        return value.ToString(CultureInfo.InvariantCulture);
    }

    public string NormalizeDoubleText(string? text, string fallback, double min, double max, string format)
    {
        if (max < min)
        {
            max = min;
        }

        var value = TryParseDouble(text, out var parsed)
            ? parsed
            : TryParseDouble(fallback, out var fallbackValue)
                ? fallbackValue
                : min;

        value = Math.Max(min, Math.Min(max, value));
        return value.ToString(format, CultureInfo.InvariantCulture);
    }

    public string NormalizeIpcClass(string? value)
    {
        return value switch
        {
            "Class1" => "Class1",
            "Class3" => "Class3",
            _ => "Class2"
        };
    }

    private static bool TryParseInteger(string? text, out int value)
    {
        if (int.TryParse(text, NumberStyles.Integer, CultureInfo.InvariantCulture, out value))
        {
            return true;
        }

        if (int.TryParse(text, NumberStyles.Integer, CultureInfo.CurrentCulture, out value))
        {
            return true;
        }

        value = 0;
        return false;
    }

    private static bool TryParseDouble(string? text, out double value)
    {
        if (double.TryParse(text, NumberStyles.Float, CultureInfo.InvariantCulture, out value))
        {
            return true;
        }

        if (double.TryParse(text, NumberStyles.Float, CultureInfo.CurrentCulture, out value))
        {
            return true;
        }

        value = 0;
        return false;
    }
}
