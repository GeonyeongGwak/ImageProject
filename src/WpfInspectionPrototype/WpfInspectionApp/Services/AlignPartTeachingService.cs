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
        Set(algorithm, "PartTeaching.Enabled", "true");
        Set(algorithm, "PartTeaching.Mode", useGerber ? "Gerber" : "FullMap");
        Set(algorithm, "PartTeaching.UseCommonLibrary", model.PartTeachingUseCommonLibrary.ToString());
        Set(algorithm, "PartTeaching.UseLibraryPart", model.PartTeachingUseLibraryPart.ToString());
        Set(algorithm, "PartTeaching.UseAutoTeaching", model.PartTeachingUseAutoTeaching.ToString());
        Set(algorithm, "PartTeaching.UseCadMatching", model.PartTeachingUseCadMatching.ToString());
        Set(algorithm, "PartTeaching.LibraryMatchMode", model.PartTeachingLibraryMatchMode);
        Set(algorithm, "PartTeaching.WindowIndex", (index + 1).ToString());
        Set(algorithm, "PartTeaching.WindowName", window.Name);
        Set(algorithm, "PartTeaching.WindowRoi", formatRoi(window.Roi));
        Set(algorithm, "PartTeaching.SearchMargin", model.AlignSearchMargin.ToString());
        Set(algorithm, "PartTeaching.SearchSizeX", model.AlignSearchSizeX.ToString());
        Set(algorithm, "PartTeaching.SearchSizeY", model.AlignSearchSizeY.ToString());
        Set(algorithm, "Align.MinBinary", Math.Min(model.Threshold2D, model.Threshold2DMax).ToString());
        Set(algorithm, "Align.MaxBinary", Math.Max(model.Threshold2D, model.Threshold2DMax).ToString());
        Set(algorithm, "Align.Range2DType", model.AlignRange2DType.ToString());
        Set(algorithm, "Align.InvertCheck", model.AlignInvertCheck.ToString());
        Set(algorithm, "Align.Use3D", model.Use3D.ToString());
        Set(algorithm, "Align.HeightMin", Math.Min(model.Threshold3D, model.Threshold3DMax).ToString());
        Set(algorithm, "Align.HeightMax", Math.Max(model.Threshold3D, model.Threshold3DMax).ToString());
        Set(algorithm, "Align.HeightAverage", model.AlignHeightAverage.ToString("0.##"));
        Set(algorithm, "Align.Range3DType", model.AlignRange3DType.ToString());
        Set(algorithm, "Align.FillHole", model.AlignFillHole.ToString());
        Set(algorithm, "Align.InspOption", (model.AlignInspectionAreaCount ? 0x01 : 0).ToString());
        Set(algorithm, "PartTeaching.TeachTime", DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"));
        algorithm.PanelData = AlgorithmPanelSchema.Create(algorithm);
        algorithm.Result = new InspectionResultData
        {
            Message = $"Align Part Teaching prepared ({AlgorithmParameterStore.GetValue(algorithm.Parameters, "PartTeaching.Mode")})",
            Bounds = window.Roi
        };
    }

    private static void Set(InspectionAlgorithmData algorithm, string key, string value)
    {
        AlgorithmParameterStore.Set(algorithm.Parameters, key, value);
    }
}
