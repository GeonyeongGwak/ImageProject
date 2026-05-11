using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class AlignPartTeachingService : IAlignPartTeachingService
{
    public AlignPartTeachingResult Apply(InspectionModel model, bool useGerber, Func<RoiRect?, string> formatRoi)
    {
        model.EnsureStructure();
        model.Algorithm = "AlgoAlign";
        model.PartTeachingStopRequested = false;
        model.PartTeachingLastMode = useGerber ? "Gerber" : "FullMap";

        var windows = model.Part.Windows.Where(window => window.Roi.IsValid).ToList();
        if (windows.Count == 0)
        {
            return new AlignPartTeachingResult(
                false,
                "Draw or import Window ROI before Part Teaching.",
                "",
                null,
                0,
                0);
        }

        model.AlignSearchNum = Net48Compat.Clamp(windows.Count, 1, 4);
        model.AlignActiveRoiIndex = 0;
        for (var index = 0; index < model.AlignSearchRois.Length; index++)
        {
            model.AlignSearchRois[index] = index < windows.Count ? windows[index].Roi : null;
        }

        var taughtCount = 0;
        for (var index = 0; index < windows.Count; index++)
        {
            var window = windows[index];
            var algorithm = EnsureAlignAlgorithm(window);
            ApplyParameters(model, algorithm, window, index, useGerber, formatRoi);
            taughtCount++;
        }

        model.SelectedWindowId = windows[0].Id;
        var summary =
            $"Align Part Teaching{Environment.NewLine}" +
            $"Mode: {model.PartTeachingLastMode}{Environment.NewLine}" +
            $"Use Common Library: {model.PartTeachingUseCommonLibrary}{Environment.NewLine}" +
            $"Use Library Part: {model.PartTeachingUseLibraryPart}{Environment.NewLine}" +
            $"Use Auto Teaching: {model.PartTeachingUseAutoTeaching}{Environment.NewLine}" +
            $"Use CAD Matching: {model.PartTeachingUseCadMatching}{Environment.NewLine}" +
            $"Library Match: {model.PartTeachingLibraryMatchMode}{Environment.NewLine}" +
            $"Window count: {windows.Count}{Environment.NewLine}" +
            $"Align ROI count: {model.AlignSearchNum}";

        return new AlignPartTeachingResult(
            true,
            $"{model.PartTeachingLastMode} teaching applied to {taughtCount} Window(s).",
            summary,
            model.SelectedWindowId,
            windows.Count,
            taughtCount);
    }

    private static InspectionAlgorithmData EnsureAlignAlgorithm(InspectionWindowData window)
    {
        var algorithm = window.Algorithms.LastOrDefault(item => string.Equals(item.Type, "AlgoAlign", StringComparison.OrdinalIgnoreCase));
        if (algorithm != null)
        {
            return algorithm;
        }

        algorithm = new InspectionAlgorithmData { Type = "AlgoAlign" };
        algorithm.ApplyCatalogDefaults();
        window.Algorithms.Add(algorithm);
        return algorithm;
    }

    private static void ApplyParameters(
        InspectionModel model,
        InspectionAlgorithmData algorithm,
        InspectionWindowData window,
        int index,
        bool useGerber,
        Func<RoiRect?, string> formatRoi)
    {
        algorithm.ApplyCatalogDefaults();
        algorithm.Parameters["PartTeaching.Enabled"] = "true";
        algorithm.Parameters["PartTeaching.Mode"] = useGerber ? "Gerber" : "FullMap";
        algorithm.Parameters["PartTeaching.UseCommonLibrary"] = model.PartTeachingUseCommonLibrary.ToString();
        algorithm.Parameters["PartTeaching.UseLibraryPart"] = model.PartTeachingUseLibraryPart.ToString();
        algorithm.Parameters["PartTeaching.UseAutoTeaching"] = model.PartTeachingUseAutoTeaching.ToString();
        algorithm.Parameters["PartTeaching.UseCadMatching"] = model.PartTeachingUseCadMatching.ToString();
        algorithm.Parameters["PartTeaching.LibraryMatchMode"] = model.PartTeachingLibraryMatchMode;
        algorithm.Parameters["PartTeaching.WindowIndex"] = (index + 1).ToString();
        algorithm.Parameters["PartTeaching.WindowName"] = window.Name;
        algorithm.Parameters["PartTeaching.WindowRoi"] = formatRoi(window.Roi);
        algorithm.Parameters["PartTeaching.SearchMargin"] = model.AlignSearchMargin.ToString();
        algorithm.Parameters["PartTeaching.SearchSizeX"] = model.AlignSearchSizeX.ToString();
        algorithm.Parameters["PartTeaching.SearchSizeY"] = model.AlignSearchSizeY.ToString();
        algorithm.Parameters["PartTeaching.TeachTime"] = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss");
        algorithm.PanelData = AlgorithmPanelSchema.Create(algorithm);
        algorithm.Result = new InspectionResultData
        {
            Message = $"Align Part Teaching prepared ({algorithm.Parameters["PartTeaching.Mode"]})",
            Bounds = window.Roi
        };
    }
}
