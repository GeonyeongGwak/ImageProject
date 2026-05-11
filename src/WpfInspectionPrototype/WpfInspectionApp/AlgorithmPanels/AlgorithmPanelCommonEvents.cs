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
        context.Algorithm.Parameters["ROI.DrawTarget"] = "Algorithm";
        context.Algorithm.Parameters["ROI.DrawRequestedAt"] = stamp;
        CommitTeachingChange(context, "ROI", "DrawAlgorithmRoi", "Algorithm ROI drawing requested", stamp);
        context.RequestAlgorithmRoiDrawing();
    }

    public static void WriteParameter(AlgorithmPanelContext? context, string key, string value)
    {
        if (context == null)
        {
            return;
        }

        context.Algorithm.Parameters[key] = value;
        var stamp = Now();
        context.Algorithm.Parameters[$"Event.{key}.ChangedAt"] = stamp;
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
        context.Algorithm.Parameters[key] = stamp;
        context.Algorithm.Parameters[$"Event.{key}.Click"] = stamp;
        context.Algorithm.Parameters["Common.LastCommandKey"] = key;
        context.Algorithm.Parameters["Common.LastCommandAt"] = stamp;
        context.SetParameter(key, stamp);

        if (IsTeachCommand(key))
        {
            var roi = context.Algorithm.AlgorithmRoi ?? context.Window.Roi;
            var formattedRoi = FormatRoi(roi);
            context.Algorithm.Parameters[$"{catalog.ParameterFamily}.TeachRoi"] = formattedRoi;
            context.Algorithm.Parameters["Common.TeachRoi"] = formattedRoi;
            context.Algorithm.Parameters["Common.SaveAlgoPropertyRequestedAt"] = stamp;
            context.Algorithm.Parameters["Common.SaveAlgoPropertyReason"] = "Teach";
        }

        if (IsApplyAllCommand(key))
        {
            context.Algorithm.Parameters["Common.ApplyAllRequestedAt"] = stamp;
            context.Algorithm.Parameters["Common.ApplyAllScope"] = "Part";
            context.Algorithm.Parameters["Common.ApplyAllSourceType"] = catalog.Type;
        }

        if (IsSearchCommand(catalog, key))
        {
            context.Algorithm.Parameters["Common.SearchRequestedAt"] = stamp;
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
        context.Algorithm.Parameters["Common.IsDirty"] = "true";
        context.Algorithm.Parameters["Common.LastEvent"] = eventName;
        context.Algorithm.Parameters["Common.LastEventSource"] = sourceKey;
        context.Algorithm.Parameters["Common.LastEventValue"] = value;
        context.Algorithm.Parameters["Common.LastChangedAt"] = changedAt;
        context.Algorithm.Parameters["Common.PartRefId"] = context.Model.Part.Name;
        context.Algorithm.Parameters["Common.WindowId"] = context.Window.Id;
        context.Algorithm.Parameters["Common.WindowName"] = context.Window.Name;
        context.Algorithm.Parameters["Common.AlgorithmId"] = context.Algorithm.Id;

        context.RequestPreviewUpdate();
        context.RequestTreeRefresh();
    }

    private static void StampClick(AlgorithmPanelContext context, string key, string stamp)
    {
        context.Algorithm.Parameters[$"Event.{key}.Click"] = stamp;
        context.SetParameter($"Event.{key}.Click", stamp);
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
