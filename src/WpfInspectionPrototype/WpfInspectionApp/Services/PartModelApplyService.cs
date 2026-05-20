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
        model.AlignSearchNum = Net48Compat.Clamp(ReadInt(parameters, model.AlignSearchNum, "Align.SearchNum"), 1, 4);
        model.AlignSearchMargin = Math.Max(0, ReadInt(parameters, model.AlignSearchMargin, "Align.SearchMargin"));
        model.AlignSearchSizeX = Math.Max(1, ReadInt(parameters, model.AlignSearchSizeX, "Align.SearchSizeX", "Align.SearchSize1.W"));
        model.AlignSearchSizeY = Math.Max(1, ReadInt(parameters, model.AlignSearchSizeY, "Align.SearchSizeY", "Align.SearchSize1.H"));
        model.AlignSameSize = ReadBool(parameters, model.AlignSameSize, "Align.SameSize");
        model.AlignShiftEnabled = ReadBool(parameters, model.AlignShiftEnabled, "Align.UseShift");
        model.AlignShiftX = ReadDouble(parameters, model.AlignShiftX, "Align.ShiftX");
        model.AlignShiftY = ReadDouble(parameters, model.AlignShiftY, "Align.ShiftY");
        model.AlignAngleEnabled = ReadBool(parameters, model.AlignAngleEnabled, "Align.UseAngle");
        model.AlignAngle = ReadDouble(parameters, model.AlignAngle, "Align.Angle");
        model.AlignFillHole = ReadBool(parameters, model.AlignFillHole, "Align.FillHole");
        model.AlignFilter = Math.Max(1, ReadInt(parameters, model.AlignFilter, "Align.MinBlobArea"));
        model.Threshold2D = Net48Compat.Clamp(ReadInt(parameters, model.Threshold2D, "Align.BinaryMin", "Align.MinBinary", "Align.Threshold"), 0, 255);
        model.Threshold2DMax = Net48Compat.Clamp(ReadInt(parameters, model.Threshold2DMax, "Align.BinaryMax", "Align.MaxBinary"), 0, 255);
        model.AlignRange2DType = Net48Compat.Clamp(ReadInt(parameters, model.AlignRange2DType, "Align.Range2DType", "Align.TypeRange2D"), 0, 3);
        model.AlignInvertCheck = ReadBool(parameters, model.AlignInvertCheck, "Align.InvertCheck");
        model.Use2D = ReadBool(parameters, model.Use2D, "Align.Use2D");

        model.Use3D = ReadBool(parameters, model.Use3D, "Align.Use3D");
        model.AlignRange3DType = Net48Compat.Clamp(ReadInt(parameters, model.AlignRange3DType, "Align.Range3DType", "Align.TypeRange3D"), 0, 3);
        model.Threshold3D = ReadInt(parameters, model.Threshold3D, "Align.HeightRateMin", "Align.HeightMin");
        model.Threshold3DMax = ReadInt(parameters, model.Threshold3DMax, "Align.HeightRateMax", "Align.HeightMax");
        model.AlignHeightAverage = ReadDouble(parameters, model.AlignHeightAverage, "Align.HeightAvg", "Align.HeightAverage");
        model.AlignInspectionAreaCount = ReadInspOption(parameters, model.AlignInspectionAreaCount);
        model.IpcUse = ReadBool(parameters, model.IpcUse, "Align.UseIPC");
        model.IpcClass = ReadIpcClass(parameters, model.IpcClass);
        model.IpcPercent = ReadDouble(parameters, model.IpcPercent, "Align.IpcPercent", "Align.IPCPercent");

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
        if (!AlgorithmParameterStore.TryGetValue(parameters, $"Align.SearchPoint{index}.X", out var rawX)
            || !AlgorithmParameterStore.TryGetValue(parameters, $"Align.SearchPoint{index}.Y", out var rawY))
        {
            return false;
        }

        var width = Math.Max(1, ReadInt(parameters, defaultWidth, $"Align.SearchSize{index}.W"));
        var height = Math.Max(1, ReadInt(parameters, defaultHeight, $"Align.SearchSize{index}.H"));
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
            TypeName = window.TypeName,
            IsEnabled = window.IsEnabled,
            IsGroup = window.IsGroup,
            GroupId = window.GroupId,
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
                Parameters = AlgorithmParameterStore.CloneCaseInsensitive(algorithm.Parameters),
                PanelData = ClonePanelData(algorithm.PanelData),
                Result = CloneInspectionResult(algorithm.Result)
            }).ToList()
        }).ToList();

        foreach (var window in clone)
        {
            window.Id = string.IsNullOrWhiteSpace(window.Id) ? InspectionWindowData.CreateId() : window.Id;
            window.Name = string.IsNullOrWhiteSpace(window.Name) ? "Window ROI" : window.Name;
            window.TypeName = string.IsNullOrWhiteSpace(window.TypeName) ? "Mount" : window.TypeName;
            window.GroupId = string.IsNullOrWhiteSpace(window.GroupId) ? (clone.IndexOf(window) + 1).ToString() : window.GroupId;
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

    private static int ReadInt(Dictionary<string, string> parameters, int fallback, params string[] keys)
    {
        if (!TryGetValue(parameters, out var raw, keys))
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

    private static double ReadDouble(Dictionary<string, string> parameters, double fallback, params string[] keys)
    {
        if (TryGetValue(parameters, out var raw, keys)
            && double.TryParse(raw, System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out var parsed))
        {
            return parsed;
        }

        return fallback;
    }

    private static bool ReadBool(Dictionary<string, string> parameters, bool fallback, params string[] keys)
    {
        if (!TryGetValue(parameters, out var raw, keys))
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

    private static bool ReadInspOption(Dictionary<string, string> parameters, bool fallback)
    {
        if (!TryGetValue(parameters, out var raw, "Align.InspOPT", "Align.InspOption"))
        {
            return fallback;
        }

        if (int.TryParse(raw, System.Globalization.NumberStyles.Integer, System.Globalization.CultureInfo.InvariantCulture, out var flags))
        {
            return (flags & 0x01) == 0x01;
        }

        return ReadBoolValue(raw, fallback);
    }

    private static string ReadIpcClass(Dictionary<string, string> parameters, string fallback)
    {
        if (!TryGetValue(parameters, out var raw, "Align.IPCClass", "Align.IpcClass"))
        {
            return fallback;
        }

        var trimmed = raw.Trim();
        if (int.TryParse(trimmed, System.Globalization.NumberStyles.Integer, System.Globalization.CultureInfo.InvariantCulture, out var index))
        {
            return Net48Compat.Clamp(index, 0, 2) switch
            {
                0 => "Class1",
                2 => "Class3",
                _ => "Class2"
            };
        }

        return trimmed.Equals("Class1", StringComparison.OrdinalIgnoreCase)
            || trimmed.Equals("Class2", StringComparison.OrdinalIgnoreCase)
            || trimmed.Equals("Class3", StringComparison.OrdinalIgnoreCase)
            ? trimmed
            : fallback;
    }

    private static bool ReadBoolValue(string raw, bool fallback)
    {
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

    private static bool TryGetValue(Dictionary<string, string> parameters, out string value, params string[] keys)
    {
        foreach (var key in keys)
        {
            if (AlgorithmParameterStore.TryGetValue(parameters, key, out value!))
            {
                return true;
            }
        }

        value = "";
        return false;
    }
}
