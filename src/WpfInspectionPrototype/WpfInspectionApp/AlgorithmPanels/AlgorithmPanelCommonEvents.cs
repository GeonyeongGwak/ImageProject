using WpfInspectionApp.Models;

namespace WpfInspectionApp.AlgorithmPanels;

public static class AlgorithmPanelCommonEvents
{
    public static void RequestAlgorithmRoi(AlgorithmPanelContext? context)
    {
        if (context == null)
        {
            return;
        }

        var stamp = Now();
        StampClick(context, "Common.DrawAlgorithmRoi", stamp);
        Set(context, "ROI.DrawTarget", "Algorithm");
        Set(context, "ROI.DrawRequestedAt", stamp);
        CommitTeachingChange(context, "ROI", "DrawAlgorithmRoi", "Algorithm ROI drawing requested", stamp);
        context.RequestAlgorithmRoiDrawing();
    }

    public static void WriteParameter(AlgorithmPanelContext? context, string key, string value)
    {
        if (context == null)
        {
            return;
        }

        Set(context, key, value);
        var stamp = Now();
        Set(context, $"Event.{key}.ChangedAt", stamp);
        context.SetParameter(key, value);
        CommitTeachingChange(context, key, "ParameterChanged", value, stamp);
    }

    public static void TriggerCommand(AlgorithmPanelContext? context, AlgorithmCatalogItem catalog, string key)
    {
        if (context == null)
        {
            return;
        }

        var stamp = Now();
        Set(context, key, stamp);
        Set(context, $"Event.{key}.Click", stamp);
        Set(context, "Common.LastCommandKey", key);
        Set(context, "Common.LastCommandAt", stamp);
        context.SetParameter(key, stamp);

        if (IsTeachCommand(key))
        {
            var roi = context.Algorithm.AlgorithmRoi ?? context.Window.Roi;
            var formattedRoi = FormatRoi(roi);
            Set(context, $"{catalog.ParameterFamily}.TeachRoi", formattedRoi);
            Set(context, "Common.TeachRoi", formattedRoi);
            Set(context, "Common.SaveAlgoPropertyRequestedAt", stamp);
            Set(context, "Common.SaveAlgoPropertyReason", "Teach");
        }

        if (IsApplyAllCommand(key))
        {
            Set(context, "Common.ApplyAllRequestedAt", stamp);
            Set(context, "Common.ApplyAllScope", "Part");
            Set(context, "Common.ApplyAllSourceType", catalog.Type);
        }

        if (IsSearchCommand(catalog, key))
        {
            Set(context, "Common.SearchRequestedAt", stamp);
        }

        CommitTeachingChange(context, key, "CommandClicked", stamp, stamp);
    }

    public static bool IsCommonCommand(AlgorithmCatalogItem catalog, string key)
    {
        return IsApplyAllCommand(key)
            || string.Equals(key, $"{catalog.ParameterFamily}.TeachRequested", StringComparison.OrdinalIgnoreCase)
            || IsSearchCommand(catalog, key)
            || IsTeachCommand(key);
    }

    public static void StampClick(AlgorithmPanelContext context, string key)
    {
        StampClick(context, key, Now());
    }

    public static void CommitTeachingChange(
        AlgorithmPanelContext context,
        string sourceKey,
        string eventName,
        string value,
        string? stamp = null)
    {
        var changedAt = stamp ?? Now();
        Set(context, "Common.IsDirty", "true");
        Set(context, "Common.LastEvent", eventName);
        Set(context, "Common.LastEventSource", sourceKey);
        Set(context, "Common.LastEventValue", value);
        Set(context, "Common.LastChangedAt", changedAt);
        Set(context, "Common.PartRefId", context.Model.Part.Name);
        Set(context, "Common.WindowId", context.Window.Id);
        Set(context, "Common.WindowName", context.Window.Name);
        Set(context, "Common.AlgorithmId", context.Algorithm.Id);

        context.RequestPreviewUpdate();
        context.RequestTreeRefresh();
    }

    private static void StampClick(AlgorithmPanelContext context, string key, string stamp)
    {
        Set(context, $"Event.{key}.Click", stamp);
        context.SetParameter($"Event.{key}.Click", stamp);
    }

    private static void Set(AlgorithmPanelContext context, string key, string value)
    {
        AlgorithmParameterStore.Set(context.Algorithm.Parameters, key, value);
    }

    public static string FormatRoi(RoiRect roi)
    {
        return $"X {roi.X} Y {roi.Y} W {roi.Width} H {roi.Height}";
    }

    private static bool IsTeachCommand(string key)
    {
        return key.EndsWith(".TeachRequested", StringComparison.OrdinalIgnoreCase);
    }

    private static bool IsApplyAllCommand(string key)
    {
        return string.Equals(key, "Command.ApplyAllTarget", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".ApplyAllRequested", StringComparison.OrdinalIgnoreCase);
    }

    private static bool IsSearchCommand(AlgorithmCatalogItem catalog, string key)
    {
        return string.Equals(key, $"{catalog.ParameterFamily}.SearchRequested", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".SearchRequested", StringComparison.OrdinalIgnoreCase);
    }

    private static string Now()
    {
        return DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss");
    }
}
