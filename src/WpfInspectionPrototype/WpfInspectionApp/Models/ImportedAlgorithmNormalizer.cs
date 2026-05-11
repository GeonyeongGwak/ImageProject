namespace WpfInspectionApp.Models;

public static class ImportedAlgorithmNormalizer
{
    private static readonly Dictionary<string, string> AliasMap = BuildAliasMap();

    public static void NormalizePart(PartInspectionData part)
    {
        part.Windows ??= [];
        for (var windowIndex = 0; windowIndex < part.Windows.Count; windowIndex++)
        {
            var window = part.Windows[windowIndex];
            window.Id = string.IsNullOrWhiteSpace(window.Id) ? InspectionWindowData.CreateId() : window.Id;
            window.Name = string.IsNullOrWhiteSpace(window.Name) ? $"Window ROI {windowIndex + 1}" : window.Name;
            window.Algorithms ??= [];

            for (var algorithmIndex = 0; algorithmIndex < window.Algorithms.Count; algorithmIndex++)
            {
                NormalizeAlgorithm(window.Algorithms[algorithmIndex], algorithmIndex + 1);
            }
        }
    }

    public static void NormalizeAlgorithm(InspectionAlgorithmData algorithm, int index)
    {
        algorithm.Id = string.IsNullOrWhiteSpace(algorithm.Id) ? InspectionAlgorithmData.CreateId() : algorithm.Id;
        algorithm.Parameters ??= [];
        algorithm.Type = ResolveType(algorithm);
        algorithm.ApplyCatalogDefaults();

        if (string.IsNullOrWhiteSpace(algorithm.DisplayName) || algorithm.DisplayName.StartsWith("Align", StringComparison.OrdinalIgnoreCase))
        {
            algorithm.DisplayName = $"{AlgorithmCatalog.Find(algorithm.Type).DisplayName} #{index}";
        }

        SeedReferenceUiDefaults(algorithm);
        algorithm.Result ??= new InspectionResultData { Message = "Imported and normalized" };
        algorithm.PanelData = AlgorithmPanelSchema.Create(algorithm);
        algorithm.Parameters["Import.NormalizedType"] = algorithm.Type;
        algorithm.Parameters["Import.ReferenceUi"] = AlgorithmReferenceUiCatalog.Create(AlgorithmCatalog.Find(algorithm.Type)).SourceControl;
    }

    private static string ResolveType(InspectionAlgorithmData algorithm)
    {
        foreach (var candidate in CandidateValues(algorithm))
        {
            if (string.IsNullOrWhiteSpace(candidate))
            {
                continue;
            }

            if (AliasMap.TryGetValue(NormalizeKey(candidate), out var mapped))
            {
                return mapped;
            }
        }

        return AlgorithmCatalog.Find(algorithm.Type).Type;
    }

    private static IEnumerable<string> CandidateValues(InspectionAlgorithmData algorithm)
    {
        yield return algorithm.Type;
        yield return algorithm.LegacyName;
        yield return algorithm.DisplayName;
        yield return algorithm.ParameterFamily;

        if (algorithm.Parameters == null)
        {
            yield break;
        }

        foreach (var key in new[]
        {
            "Legacy.Type",
            "Legacy.AlgoType",
            "Legacy.Algorithm",
            "Legacy.Name",
            "Type",
            "AlgoType",
            "AlgorithmType",
            "Name"
        })
        {
            if (algorithm.Parameters.TryGetValue(key, out var value))
            {
                yield return value;
            }
        }

        foreach (var pair in algorithm.Parameters.Where(pair =>
            pair.Key.Contains("Algo", StringComparison.OrdinalIgnoreCase) ||
            pair.Key.Contains("Type", StringComparison.OrdinalIgnoreCase) ||
            pair.Key.Contains("Name", StringComparison.OrdinalIgnoreCase)))
        {
            yield return pair.Value;
        }
    }

    private static void SeedReferenceUiDefaults(InspectionAlgorithmData algorithm)
    {
        var catalog = AlgorithmCatalog.Find(algorithm.Type);
        var profile = AlgorithmReferenceUiCatalog.Create(catalog);
        foreach (var control in profile.Controls)
        {
            if (control.Kind == AlgorithmReferenceControlKind.Command)
            {
                continue;
            }

            if (!algorithm.Parameters.ContainsKey(control.Key))
            {
                algorithm.Parameters[control.Key] = control.DefaultValue;
            }
        }

        AddDefault(algorithm.Parameters, "Common.bAlgoEnable", "true");
        AddDefault(algorithm.Parameters, "ROI.UseAlgorithmRoi", algorithm.AlgorithmRoi.HasValue ? "true" : "false");
    }

    private static Dictionary<string, string> BuildAliasMap()
    {
        var aliases = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
        foreach (var item in AlgorithmCatalog.All)
        {
            Add(aliases, item.Type, item.Type);
            Add(aliases, item.Type.Replace("Algo", ""), item.Type);
            Add(aliases, item.DisplayName, item.Type);
            Add(aliases, item.LegacyName, item.Type);
            Add(aliases, item.ParameterFamily, item.Type);
        }

        Add(aliases, "BW", "AlgoBW");
        Add(aliases, "BlackWhite", "AlgoBW");
        Add(aliases, "BodyBlob", "AlgoBody_Blob");
        Add(aliases, "Body_Blob", "AlgoBody_Blob");
        Add(aliases, "GrayMean", "AlgoGray_Mean");
        Add(aliases, "Gray_Mean", "AlgoGray_Mean");
        Add(aliases, "HeightMean", "AlgoHeight_Mean");
        Add(aliases, "Height_Mean", "AlgoHeight_Mean");
        Add(aliases, "GrayDiff", "AlgoGray_Diff");
        Add(aliases, "Gray_Diff", "AlgoGray_Diff");
        Add(aliases, "HeightDiff", "AlgoHeight_Diff");
        Add(aliases, "Height_Diff", "AlgoHeight_Diff");
        Add(aliases, "LeadTip", "AlgoLead_Tip");
        Add(aliases, "Lead_Tip", "AlgoLead_Tip");
        Add(aliases, "LeadLift", "AlgoLead_Lift");
        Add(aliases, "Lead_Lift", "AlgoLead_Lift");
        Add(aliases, "LeadSolder", "AlgoLead_Solder");
        Add(aliases, "Lead_Solder", "AlgoLead_Solder");
        Add(aliases, "LeadColor", "AlgoLead_Color");
        Add(aliases, "Lead_Color", "AlgoLead_Color");
        Add(aliases, "LeadSearch", "AlgoLead_Search");
        Add(aliases, "Lead_Search", "AlgoLead_Search");
        Add(aliases, "LeadSideSolder", "AlgoLead_SideSolder");
        Add(aliases, "Lead_SideSolder", "AlgoLead_SideSolder");
        Add(aliases, "TabSearch", "AlgoTab_Search");
        Add(aliases, "Tab_Search", "AlgoTab_Search");
        Add(aliases, "ColorBandSearch", "AlgoColorBand_Search");
        Add(aliases, "ColorBand_Search", "AlgoColorBand_Search");
        Add(aliases, "DisColor", "AlgoDisColor");
        Add(aliases, "ForeignOCV", "AlgoForeignOCV");
        Add(aliases, "OCV", "AlgoForeignOCV");
        Add(aliases, "EdgePoint", "AlgoEdgePoint");
        Add(aliases, "LQBGA", "AlgoLQBGA");
        Add(aliases, "BarCode", "AlgoBarcode");
        Add(aliases, "Padbw", "AlgoPadBW");
        return aliases;
    }

    private static void Add(Dictionary<string, string> aliases, string key, string value)
    {
        if (!string.IsNullOrWhiteSpace(key))
        {
            aliases[NormalizeKey(key)] = value;
        }
    }

    private static void AddDefault(Dictionary<string, string> parameters, string key, string value)
    {
        if (!parameters.ContainsKey(key))
        {
            parameters[key] = value;
        }
    }

    private static string NormalizeKey(string value)
    {
        var chars = value
            .Where(char.IsLetterOrDigit)
            .Select(char.ToUpperInvariant)
            .ToArray();
        return new string(chars);
    }
}
