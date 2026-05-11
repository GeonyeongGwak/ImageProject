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
}
