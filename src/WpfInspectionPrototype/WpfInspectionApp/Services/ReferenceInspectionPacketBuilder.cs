using System.Globalization;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class ReferenceInspectionPacket
{
    public string ModelName { get; set; } = "";
    public string PartName { get; set; } = "";
    public List<ReferenceWindowPacket> Windows { get; set; } = [];
    public int AlgorithmCount => Windows.Sum(window => window.Algorithms.Count);
}

public sealed class ReferenceWindowPacket
{
    public int Index { get; set; }
    public string Id { get; set; } = "";
    public string Name { get; set; } = "";
    public RoiRect Roi { get; set; }
    public List<ReferenceAlgorithmPacket> Algorithms { get; set; } = [];
}

public sealed class ReferenceAlgorithmPacket
{
    public int WindowIndex { get; set; }
    public int AlgorithmIndex { get; set; }
    public string Id { get; set; } = "";
    public string Type { get; set; } = "";
    public string DisplayName { get; set; } = "";
    public string ReferenceName { get; set; } = "";
    public int InspAlgoTypeOrdinal { get; set; } = -1;
    public string ResultBucket { get; set; } = "";
    public RoiRect WindowRoi { get; set; }
    public RoiRect InspectionRoi { get; set; }
    public bool UsesAlgorithmRoi { get; set; }
    public Dictionary<string, string> RawFields { get; set; } = new(StringComparer.OrdinalIgnoreCase);
    public Dictionary<string, string> ScalarFields { get; set; } = new(StringComparer.OrdinalIgnoreCase);
    public Dictionary<string, double[]> NumericArrays { get; set; } = new(StringComparer.OrdinalIgnoreCase);

    public string ToSummary()
    {
        var roiKind = UsesAlgorithmRoi ? "Algorithm ROI" : "Window ROI";
        return $"{ReferenceName} ({InspAlgoTypeOrdinal}) -> {ResultBucket}, {roiKind}, raw {RawFields.Count}, arrays {NumericArrays.Count}";
    }
}

public static class ReferenceInspectionPacketBuilder
{
    private static readonly string[] InspAlgoTypes =
    [
        "BW", "Blob", "Align", "Body_Blob", "Tilt", "OCR", "Pattern", "Color",
        "Gray_Mean", "Height_Mean", "Gray_Diff", "Height_Diff",
        "Lead_Search", "Bridge", "Lead_Tip", "Lead_Lift", "Lead_Solder", "Lead_Color",
        "Width", "Volume", "Lead_SideSolder", "Tab_Search", "Tab", "ColorBand_Search",
        "Grid", "Line", "Edge", "SolderCone", "ColorXY", "AlignEdge", "PadAlign", "POCR",
        "DisColor", "Wire", "Foot", "Barcode", "Fillet", "BGA", "Bump", "NGBlob", "PadBW",
        "ForeignOCV", "BodyEdge", "PackageThickness", "Distance", "GWire", "EdgePoint",
        "PatternDiff", "ShapeX", "PadArray", "LQBGA"
    ];

    private static readonly Dictionary<string, int> Ordinals = BuildOrdinals();

    public static ReferenceInspectionPacket Build(PartRuntimePacket packet)
    {
        var referencePacket = new ReferenceInspectionPacket
        {
            ModelName = packet.ModelName,
            PartName = packet.PartName
        };

        foreach (var window in packet.Windows)
        {
            var referenceWindow = new ReferenceWindowPacket
            {
                Index = window.Index,
                Id = window.Id,
                Name = window.Name,
                Roi = window.Roi
            };

            foreach (var algorithm in window.Algorithms)
            {
                referenceWindow.Algorithms.Add(BuildAlgorithm(window, algorithm));
            }

            referencePacket.Windows.Add(referenceWindow);
        }

        return referencePacket;
    }

    private static ReferenceAlgorithmPacket BuildAlgorithm(WindowRuntimePacket window, AlgorithmRuntimePacket algorithm)
    {
        var catalog = AlgorithmCatalog.Find(algorithm.Type);
        var referenceName = NormalizeReferenceName(catalog.LegacyName, catalog.Type);
        var referenceAlgorithm = new ReferenceAlgorithmPacket
        {
            WindowIndex = window.Index,
            AlgorithmIndex = algorithm.Index,
            Id = algorithm.Id,
            Type = catalog.Type,
            DisplayName = catalog.DisplayName,
            ReferenceName = referenceName,
            InspAlgoTypeOrdinal = TryGetOrdinal(referenceName),
            ResultBucket = CreateResultBucket(referenceName),
            WindowRoi = window.Roi,
            InspectionRoi = algorithm.InspectionRoi,
            UsesAlgorithmRoi = algorithm.AlgorithmRoi.HasValue
        };

        CopyLegacyFields(referenceAlgorithm, algorithm);
        PromoteImportantFields(referenceAlgorithm, referenceName);
        return referenceAlgorithm;
    }

    public static string FormatOrdinal(string type)
    {
        var catalog = AlgorithmCatalog.Find(type);
        var referenceName = NormalizeReferenceName(catalog.LegacyName, catalog.Type);
        return TryGetOrdinal(referenceName).ToString(CultureInfo.InvariantCulture);
    }

    private static void CopyLegacyFields(ReferenceAlgorithmPacket referenceAlgorithm, AlgorithmRuntimePacket algorithm)
    {
        foreach (var pair in algorithm.Parameters.OrderBy(pair => pair.Key, StringComparer.OrdinalIgnoreCase))
        {
            if (!pair.Key.StartsWith("Legacy.", StringComparison.OrdinalIgnoreCase))
            {
                continue;
            }

            var key = pair.Key.Substring("Legacy.".Length);
            if (IsImporterMetadata(key))
            {
                continue;
            }

            referenceAlgorithm.RawFields[key] = pair.Value;
            var values = ParseNumberArray(pair.Value);
            if (values.Length > 1)
            {
                referenceAlgorithm.NumericArrays[key] = values;
            }
            else
            {
                referenceAlgorithm.ScalarFields[key] = pair.Value;
            }
        }
    }

    private static void PromoteImportantFields(ReferenceAlgorithmPacket referenceAlgorithm, string referenceName)
    {
        var prefixes = GetImportantPrefixes(referenceName);
        foreach (var pair in referenceAlgorithm.RawFields)
        {
            if (prefixes.Any(prefix => pair.Key.StartsWith(prefix, StringComparison.OrdinalIgnoreCase)))
            {
                referenceAlgorithm.ScalarFields[pair.Key] = pair.Value;
            }
        }
    }

    private static IReadOnlyList<string> GetImportantPrefixes(string referenceName)
    {
        return referenceName switch
        {
            "Align" => ["LTInfo", "SearMargin_mm", "Use2D", "TPR2D", "MnMx2D", "Use3D", "UseSft", "Sft", "UseAng", "Ang", "SearPnt", "Searsz", "SearMargin", "FH"],
            "AlignEdge" => ["InspDirection", "UsePeak", "MinEdge", "MaxEdge", "Search", "Edge", "Angle", "Offset", "ROI"],
            "PadBW" => ["BData", "FData", "SubBData", "SubFData", "SubLight", "SubC", "AMROI", "ROI"],
            "NGBlob" => ["BData", "FData", "UseData", "SelV", "ROIF", "AMROI", "ROI"],
            "ShapeX" => ["OCRMxCnt", "ImgCBLTData", "ROICnt", "Dir", "MSc", "Hist", "Opt", "Asp", "thick", "VMxLeng", "HMxLeng", "NGA", "CrtA", "MxChLeng", "TBRate", "TPSel", "ROI", "UShp", "UExShp"],
            _ => ["BData", "FData", "UseData", "ROI", "AMROI", "Sub", "Threshold", "Tolerance"]
        };
    }

    private static double[] ParseNumberArray(string value)
    {
        if (string.IsNullOrWhiteSpace(value) || !value.Contains(','))
        {
            return [];
        }

        var numbers = new List<double>();
        foreach (var token in value.Split(','))
        {
            if (!double.TryParse(token.Trim(), NumberStyles.Float, CultureInfo.InvariantCulture, out var number))
            {
                return [];
            }

            numbers.Add(number);
        }

        return numbers.ToArray();
    }

    private static bool IsImporterMetadata(string key)
    {
        return string.Equals(key, "Source", StringComparison.OrdinalIgnoreCase)
            || string.Equals(key, "ID", StringComparison.OrdinalIgnoreCase)
            || string.Equals(key, "Type", StringComparison.OrdinalIgnoreCase)
            || string.Equals(key, "TypeMeaning", StringComparison.OrdinalIgnoreCase)
            || string.Equals(key, "WindowIndex", StringComparison.OrdinalIgnoreCase);
    }

    private static string NormalizeReferenceName(string legacyName, string type)
    {
        var name = string.IsNullOrWhiteSpace(legacyName)
            ? StripAlgorithmPrefix(type)
            : legacyName;

        return name switch
        {
            "BlackWhite" => "BW",
            "BodyBlob" => "Body_Blob",
            "GrayMean" => "Gray_Mean",
            "HeightMean" => "Height_Mean",
            "GrayDiff" => "Gray_Diff",
            "HeightDiff" => "Height_Diff",
            "LeadSearch" => "Lead_Search",
            "LeadTip" => "Lead_Tip",
            "LeadLift" => "Lead_Lift",
            "LeadSolder" => "Lead_Solder",
            "LeadColor" => "Lead_Color",
            "LeadSideSolder" => "Lead_SideSolder",
            "TabSearch" => "Tab_Search",
            "ColorBandSearch" => "ColorBand_Search",
            "SolderCone" => "SolderCone",
            "BarCode" => "Barcode",
            "Padbw" => "PadBW",
            _ => name
        };
    }

    private static int TryGetOrdinal(string referenceName)
    {
        return Ordinals.TryGetValue(referenceName, out var ordinal) ? ordinal : -1;
    }

    private static string CreateResultBucket(string referenceName)
    {
        return $"vINSPALGO_{referenceName.Replace("_", "").ToUpperInvariant()}";
    }

    private static string StripAlgorithmPrefix(string type)
    {
        return type.StartsWith("Algo", StringComparison.OrdinalIgnoreCase)
            ? type.Substring("Algo".Length)
            : type;
    }

    private static Dictionary<string, int> BuildOrdinals()
    {
        var ordinals = new Dictionary<string, int>(StringComparer.OrdinalIgnoreCase);
        for (var index = 0; index < InspAlgoTypes.Length; index++)
        {
            ordinals[InspAlgoTypes[index]] = index;
        }

        return ordinals;
    }
}
