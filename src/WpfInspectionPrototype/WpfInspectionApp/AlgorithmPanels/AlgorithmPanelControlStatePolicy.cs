using WpfInspectionApp.Models;

namespace WpfInspectionApp.AlgorithmPanels;

public sealed class AlgorithmPanelControlStatePolicy
{
    private readonly AlgorithmCatalogItem _catalog;
    private readonly Func<string, string, string> _read;

    public AlgorithmPanelControlStatePolicy(AlgorithmCatalogItem catalog, Func<string, string, string> read)
    {
        _catalog = catalog;
        _read = read;
    }

    public bool IsControlEnabled(AlgorithmReferenceControl control)
    {
        if (control.Kind == AlgorithmReferenceControlKind.Check || control.Kind == AlgorithmReferenceControlKind.Command)
        {
            return true;
        }

        var key = control.Key;
        var prefix = $"{_catalog.ParameterFamily}.";
        if (!key.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
        {
            return true;
        }

        return IsFamilyControlEnabled(key);
    }

    public static bool ShouldRebuildForDependency(string key)
    {
        return key.EndsWith(".Use2D", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".b2DCheck", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".Use3D", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseHeight", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseVolume", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseColor", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseFilter", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseShift", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseShiftX", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseShiftY", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseDistance", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseDist", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseDistanceX", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseDistanceY", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseAngle", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseArea", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseWidth", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseLength", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseAnchor", StringComparison.OrdinalIgnoreCase);
    }

    private bool IsFamilyControlEnabled(string key)
    {
        var family = _catalog.ParameterFamily;

        if (ContainsAny(key, "MinValue", "MaxValue", "Range2D", "SubLine2D", "Binary"))
        {
            return ReadBool($"{family}.Use2D", true) && ReadBool($"{family}.b2DCheck", true);
        }

        if (ContainsAny(key, "Height", "Range3D", "SubLine3D", "Volume", "Coplanarity", "Flux"))
        {
            return ReadBool($"{family}.Use3D", false) || ReadBool($"{family}.UseHeight", false) || ReadBool($"{family}.UseVolume", false);
        }

        if (ContainsAny(key, "Color", "MinR", "MaxR", "MinG", "MaxG", "MinB", "MaxB"))
        {
            return ReadBool($"{family}.UseColor", true);
        }

        if (ContainsAny(key, "Filter"))
        {
            return ReadBool($"{family}.UseFilter", false);
        }

        if (ContainsAny(key, "ShiftX"))
        {
            return ReadBool($"{family}.UseShift", false) && ReadBool($"{family}.UseShiftX", true);
        }

        if (ContainsAny(key, "ShiftY"))
        {
            return ReadBool($"{family}.UseShift", false) && ReadBool($"{family}.UseShiftY", true);
        }

        if (ContainsAny(key, "Shift"))
        {
            return ReadBool($"{family}.UseShift", false);
        }

        if (ContainsAny(key, "DistanceX"))
        {
            return ReadBool($"{family}.UseDistanceX", false);
        }

        if (ContainsAny(key, "DistanceY"))
        {
            return ReadBool($"{family}.UseDistanceY", false);
        }

        if (ContainsAny(key, "Distance", "Toler_Dist", "Offset_Dist"))
        {
            return ReadBool($"{family}.UseDistance", false) || ReadBool($"{family}.UseDist", false);
        }

        if (ContainsAny(key, "Angle"))
        {
            return ReadBool($"{family}.UseAngle", false);
        }

        if (ContainsAny(key, "Area"))
        {
            return ReadBool($"{family}.UseArea", true);
        }

        if (ContainsAny(key, "Width"))
        {
            return ReadBool($"{family}.UseWidth", false);
        }

        if (ContainsAny(key, "Length"))
        {
            return ReadBool($"{family}.UseLength", false);
        }

        if (ContainsAny(key, "Anchor"))
        {
            return ReadBool($"{family}.UseAnchor", false);
        }

        return true;
    }

    private bool ReadBool(string key, bool fallback)
    {
        var value = _read(key, fallback ? "true" : "false");
        if (bool.TryParse(value, out var parsed))
        {
            return parsed;
        }

        return int.TryParse(value, out var number) ? number != 0 : fallback;
    }

    private static bool ContainsAny(string value, params string[] needles)
    {
        return needles.Any(needle => value.Contains(needle, StringComparison.OrdinalIgnoreCase));
    }
}
