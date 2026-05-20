using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class AlignConditionService : IAlignConditionService
{
    public int CalculateMaskDensity(InspectionModel model)
    {
        var min2d = Math.Min(model.Threshold2D, model.Threshold2DMax);
        var max2d = Math.Max(model.Threshold2D, model.Threshold2DMax);
        var min3d = Math.Min(model.Threshold3D, model.Threshold3DMax);
        var max3d = Math.Max(model.Threshold3D, model.Threshold3DMax);
        var normalized2d = model.Use2D ? (min2d + max2d) / 510.0 : 0;
        var normalized3d = model.Use3D ? (min3d + max3d) / 510.0 : 0;
        var normalizedEdge = model.UseEdge ? model.EdgeGain / 100.0 : 0;
        var sourceWeight = (model.Use2D ? 0.68 : 0) + (model.Use3D ? 0.24 : 0);
        var baseDensity = sourceWeight == 0 ? 0.24 : 1 - normalized2d * 0.68 - normalized3d * 0.24;
        return Net48Compat.Clamp((int)Math.Round((baseDensity + normalizedEdge * 0.18) * 100), 4, 98);
    }
}
