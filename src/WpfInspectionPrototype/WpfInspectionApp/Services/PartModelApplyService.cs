using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class PartModelApplyService : IPartModelApplyService
{
    public PartModelApplyResult Apply(InspectionModel model, PartInspectionData importedPart, string source)
    {
        model.EnsureStructure();
        ImportedAlgorithmNormalizer.NormalizePart(importedPart);
        foreach (var window in importedPart.Windows)
        {
            window.Name = string.IsNullOrWhiteSpace(window.Name) ? $"Window ROI {model.Part.Windows.Count + 1}" : window.Name;
        }

        model.Part.Windows = CloneWindows(importedPart.Windows);
        model.Part.Name = model.ModelName;
        model.Part.PixelResolutionX = importedPart.PixelResolutionX;
        model.Part.PixelResolutionY = importedPart.PixelResolutionY;
        model.Part.SourceWidth = importedPart.SourceWidth;
        model.Part.SourceHeight = importedPart.SourceHeight;
        model.SelectedWindowId = model.Part.Windows.FirstOrDefault()?.Id;
        model.AlignSearchNum = Net48Compat.Clamp(model.Part.Windows.Count, 1, 4);
        model.AlignActiveRoiIndex = 0;
        ApplyImportedAlignDefaults(model);

        var summary =
            $"Part imported from: {source}{Environment.NewLine}" +
            $"Resolution: {FormatImportedResolution(model.Part)}{Environment.NewLine}" +
            $"Window count: {model.Part.Windows.Count}{Environment.NewLine}" +
            $"Algorithm count: {model.Part.Windows.Sum(window => window.Algorithms.Count)}{Environment.NewLine}" +
            $"Algorithm types: {string.Join(", ", model.Part.Windows.SelectMany(window => window.Algorithms).Select(algorithm => algorithm.Type).Distinct().OrderBy(type => type))}";

        return new PartModelApplyResult(summary, model.SelectedWindowId);
    }

    private static string FormatImportedResolution(PartInspectionData part)
    {
        return part.PixelResolutionX > 0 && part.PixelResolutionY > 0
            ? $"{part.PixelResolutionX:0.########} x {part.PixelResolutionY:0.########} mm/px | FOV {part.SourceWidth}x{part.SourceHeight}"
            : "not found";
    }

    private static void ApplyImportedAlignDefaults(InspectionModel model)
    {
        var align = model.Part.Windows
            .SelectMany(window => window.Algorithms ?? [])
            .FirstOrDefault(algorithm => string.Equals(algorithm.Type, "AlgoAlign", StringComparison.OrdinalIgnoreCase));
        if (align?.Parameters == null)
        {
            return;
        }

        var parameters = align.Parameters;
        model.AlignSearchNum = Net48Compat.Clamp(ReadInt(parameters, "Align.SearchNum", model.AlignSearchNum), 1, 4);
        model.AlignSearchMargin = Math.Max(0, ReadInt(parameters, "Align.SearchMargin", model.AlignSearchMargin));
        model.AlignSearchSizeX = Math.Max(1, ReadInt(parameters, "Align.SearchSizeX", model.AlignSearchSizeX));
        model.AlignSearchSizeY = Math.Max(1, ReadInt(parameters, "Align.SearchSizeY", model.AlignSearchSizeY));
        model.AlignSameSize = ReadBool(parameters, "Align.SameSize", model.AlignSameSize);
        model.AlignShiftEnabled = ReadBool(parameters, "Align.UseShift", model.AlignShiftEnabled);
        model.AlignShiftX = ReadDouble(parameters, "Align.ShiftX", model.AlignShiftX);
        model.AlignShiftY = ReadDouble(parameters, "Align.ShiftY", model.AlignShiftY);
        model.AlignAngleEnabled = ReadBool(parameters, "Align.UseAngle", model.AlignAngleEnabled);
        model.AlignAngle = ReadDouble(parameters, "Align.Angle", model.AlignAngle);
        model.AlignFillHole = ReadBool(parameters, "Align.FillHole", model.AlignFillHole);
        model.AlignFilter = Math.Max(1, ReadInt(parameters, "Align.MinBlobArea", model.AlignFilter));
        model.Threshold2D = Net48Compat.Clamp(ReadInt(parameters, "Align.BinaryMin", model.Threshold2D), 0, 255);

        var rois = new RoiRect?[4];
        for (var index = 1; index <= 4; index++)
        {
            if (TryReadSearchRoi(parameters, index, model.AlignSearchSizeX, model.AlignSearchSizeY, out var roi))
            {
                rois[index - 1] = roi;
            }
        }

        model.AlignSearchRois = rois;
    }

    private static bool TryReadSearchRoi(Dictionary<string, string> parameters, int index, int defaultWidth, int defaultHeight, out RoiRect roi)
    {
        roi = default;
        if (!parameters.TryGetValue($"Align.SearchPoint{index}.X", out var rawX)
            || !parameters.TryGetValue($"Align.SearchPoint{index}.Y", out var rawY))
        {
            return false;
        }

        var width = Math.Max(1, ReadInt(parameters, $"Align.SearchSize{index}.W", defaultWidth));
        var height = Math.Max(1, ReadInt(parameters, $"Align.SearchSize{index}.H", defaultHeight));
        var centerX = ReadIntValue(rawX, 0);
        var centerY = ReadIntValue(rawY, 0);
        roi = new RoiRect(centerX - width / 2, centerY - height / 2, width, height);
        return true;
    }

    private static List<InspectionWindowData> CloneWindows(List<InspectionWindowData> windows)
    {
        var clone = windows.Select(window => new InspectionWindowData
        {
            Id = window.Id,
            Name = window.Name,
            Roi = window.Roi,
            Algorithms = (window.Algorithms ?? []).Select(algorithm => new InspectionAlgorithmData
            {
                Id = algorithm.Id,
                Type = algorithm.Type,
                DisplayName = algorithm.DisplayName,
                LegacyGroup = algorithm.LegacyGroup,
                LegacyFlag = algorithm.LegacyFlag,
                LegacyName = algorithm.LegacyName,
                ParameterFamily = algorithm.ParameterFamily,
                AlgorithmRoi = algorithm.AlgorithmRoi,
                Parameters = new Dictionary<string, string>(algorithm.Parameters ?? []),
                PanelData = ClonePanelData(algorithm.PanelData),
                Result = CloneInspectionResult(algorithm.Result)
            }).ToList()
        }).ToList();

        foreach (var window in clone)
        {
            window.Id = string.IsNullOrWhiteSpace(window.Id) ? InspectionWindowData.CreateId() : window.Id;
            window.Name = string.IsNullOrWhiteSpace(window.Name) ? "Window ROI" : window.Name;
            window.Algorithms ??= [];
            for (var index = 0; index < window.Algorithms.Count; index++)
            {
                ImportedAlgorithmNormalizer.NormalizeAlgorithm(window.Algorithms[index], index + 1);
            }
        }

        return clone;
    }

    private static AlgorithmPanelData ClonePanelData(AlgorithmPanelData? panelData)
    {
        if (panelData == null)
        {
            return new AlgorithmPanelData();
        }

        return new AlgorithmPanelData
        {
            AlgorithmType = panelData.AlgorithmType,
            PanelKind = panelData.PanelKind,
            ActiveTab = panelData.ActiveTab,
            Tabs = (panelData.Tabs ?? []).Select(tab => new AlgorithmPanelTabData
            {
                Id = tab.Id,
                Title = tab.Title,
                ParameterKeys = (tab.ParameterKeys ?? []).ToList()
            }).ToList()
        };
    }

    private static InspectionResultData CloneInspectionResult(InspectionResultData? result)
    {
        if (result == null)
        {
            return new InspectionResultData();
        }

        return new InspectionResultData
        {
            Message = result.Message,
            ElapsedMs = result.ElapsedMs,
            ForegroundPixels = result.ForegroundPixels,
            BlobCount = result.BlobCount,
            Bounds = result.Bounds
        };
    }

    private static int ReadInt(Dictionary<string, string> parameters, string key, int fallback)
    {
        if (!parameters.TryGetValue(key, out var raw))
        {
            return fallback;
        }

        return ReadIntValue(raw, fallback);
    }

    private static int ReadIntValue(string raw, int fallback)
    {
        if (int.TryParse(raw, System.Globalization.NumberStyles.Integer, System.Globalization.CultureInfo.InvariantCulture, out var parsed))
        {
            return parsed;
        }

        if (double.TryParse(raw, System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out var number))
        {
            return (int)Math.Round(number, MidpointRounding.AwayFromZero);
        }

        return fallback;
    }

    private static double ReadDouble(Dictionary<string, string> parameters, string key, double fallback)
    {
        if (parameters.TryGetValue(key, out var raw)
            && double.TryParse(raw, System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out var parsed))
        {
            return parsed;
        }

        return fallback;
    }

    private static bool ReadBool(Dictionary<string, string> parameters, string key, bool fallback)
    {
        if (!parameters.TryGetValue(key, out var raw))
        {
            return fallback;
        }

        if (bool.TryParse(raw, out var parsed))
        {
            return parsed;
        }

        if (double.TryParse(raw, System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out var number))
        {
            return Math.Abs(number) > double.Epsilon;
        }

        return fallback;
    }
}
