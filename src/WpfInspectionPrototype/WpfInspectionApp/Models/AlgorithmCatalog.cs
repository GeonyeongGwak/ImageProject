namespace WpfInspectionApp.Models;

public enum LegacyAlgorithmGroup
{
    Normal,
    Extended
}

public sealed record AlgorithmCatalogItem(
    string Type,
    string DisplayName,
    LegacyAlgorithmGroup Group,
    int LegacyFlag,
    string LegacyName,
    string ParameterFamily);

public static class AlgorithmCatalog
{
    public static IReadOnlyList<AlgorithmCatalogItem> All { get; } =
    [
        Normal("AlgoBW", "BlackWhite", 2, "BlackWhite", "BlackWhite"),
        Normal("AlgoBlob", "Blob", 4, "Blob", "Blob"),
        Normal("AlgoAlign", "Align", 8, "Align", "Align"),
        Normal("AlgoBody_Blob", "Body Blob", 16, "BodyBlob", "BodyBlob"),
        Normal("AlgoTilt", "Tilt", 32, "Tilt", "Tilt"),
        Normal("AlgoOCR", "OCR", 64, "OCR", "OCR"),
        Normal("AlgoPattern", "Pattern", 128, "Pattern", "Pattern"),
        Normal("AlgoColor", "Color", 256, "Color", "Color"),
        Normal("AlgoGray_Mean", "Gray Mean", 512, "GrayMean", "Graymean"),
        Normal("AlgoHeight_Mean", "Height Mean", 1024, "HeightMean", "Heightmean"),
        Normal("AlgoGray_Diff", "Gray Diff", 2048, "GrayDiff", "Graydiff"),
        Normal("AlgoHeight_Diff", "Height Diff", 4096, "HeightDiff", "Heightdiff"),
        Normal("AlgoBridge", "Bridge", 8192, "Bridge", "Bridge"),
        Normal("AlgoLead_Tip", "Lead Tip", 16384, "LeadTip", "Leadtip"),
        Normal("AlgoLead_Lift", "Lead Lift", 32768, "LeadLift", "Leadlift"),
        Normal("AlgoLead_Solder", "Lead Solder", 65536, "LeadSolder", "Leadsolder"),
        Normal("AlgoLead_Color", "Lead Color", 131072, "LeadColor", "LeadColor"),
        Normal("AlgoWidth", "Width", 262144, "Width", "Length"),
        Normal("AlgoVolume", "Volume", 524288, "Volume", "Volume"),
        Normal("AlgoLead_Search", "Lead Search", 1048576, "LeadSearch", "Leadsearch"),
        Normal("AlgoLead_SideSolder", "Lead Side Solder", 2097152, "LeadSideSolder", "Leadsidesolder"),
        Normal("AlgoTab", "Tab", 4194304, "Tab", "Tab"),
        Normal("AlgoTab_Search", "Tab Search", 8388608, "TabSearch", "Tabsearch"),
        Normal("AlgoColorBand_Search", "Color Band Search", 16777216, "ColorBandSearch", "ColorBandSearch"),
        Normal("AlgoGrid", "Grid", 33554432, "Grid", "Grid"),
        Normal("AlgoLine", "Line", 67108864, "Line", "Line"),
        Normal("AlgoEdge", "Edge", 134217728, "Edge", "Edge"),
        Normal("AlgoSolderCone", "Solder Cone", 268435456, "SolderCone", "Soldercone"),
        Normal("AlgoColorXY", "Color XY", 536870912, "ColorXY", "ColorXY"),
        Normal("AlgoPOCR", "POCR", 1073741824, "POCR", "POCR"),
        Extended("AlgoAlignEdge", "Align Edge", 1, "AlignEdge", "AlignEdge"),
        Extended("AlgoPadAlign", "Pad Align", 1 << 1, "PadAlign", "PadAlign"),
        Extended("AlgoDisColor", "DisColor", 1 << 2, "DisColor", "DisColor"),
        Extended("AlgoBarcode", "Barcode", 1 << 3, "BarCode", "BarCode"),
        Extended("AlgoFillet", "Fillet", 1 << 4, "Fillet", "Fillet"),
        Extended("AlgoBGA", "BGA", 1 << 5, "BGA", "BGA"),
        Extended("AlgoBump", "Bump", 1 << 6, "Bump", "Bump"),
        Extended("AlgoNGBlob", "NG Blob", 1 << 7, "NGBlob", "NGBlob"),
        Extended("AlgoBodyEdge", "Body Edge", 1 << 8, "BodyEdge", "BodyEdge"),
        Extended("AlgoPackageThickness", "Package Thickness", 1 << 9, "PackageThickness", "PackageThickness"),
        Extended("AlgoDistance", "Distance", 1 << 10, "Distance", "Distance"),
        Extended("AlgoGWire", "G Wire", 1 << 11, "GWire", "GWire"),
        Extended("AlgoPatternDiff", "Pattern Diff", 1 << 12, "PatternDiff", "PatternDiff"),
        Extended("AlgoPadBW", "Pad BW", 1 << 13, "Padbw", "PadBW"),
        Extended("AlgoShapeX", "Shape X", 1 << 14, "ShapeX", "ShapeX"),
        Extended("AlgoPadArray", "Pad Array", 1 << 15, "PadArray", "PadArray"),
        Extended("AlgoWire", "Wire", 1 << 11, "GWire", "GWire"),
        Extended("AlgoFoot", "Foot", 1 << 4, "Fillet", "Fillet"),
        Extended("AlgoForeignOCV", "Foreign OCV", 1 << 12, "PatternDiff", "PatternDiff"),
        Extended("AlgoEdgePoint", "Edge Point", 1 << 10, "Distance", "Distance"),
        Extended("AlgoLQBGA", "LQ BGA", 1 << 5, "BGA", "BGA")
    ];

    public static AlgorithmCatalogItem Find(string? type)
    {
        return All.FirstOrDefault(item => string.Equals(item.Type, type, StringComparison.OrdinalIgnoreCase))
            ?? All.First(item => item.Type == "AlgoAlign");
    }

    private static AlgorithmCatalogItem Normal(string type, string displayName, int legacyFlag, string legacyName, string parameterFamily)
    {
        return new AlgorithmCatalogItem(type, displayName, LegacyAlgorithmGroup.Normal, legacyFlag, legacyName, parameterFamily);
    }

    private static AlgorithmCatalogItem Extended(string type, string displayName, int legacyFlag, string legacyName, string parameterFamily)
    {
        return new AlgorithmCatalogItem(type, displayName, LegacyAlgorithmGroup.Extended, legacyFlag, legacyName, parameterFamily);
    }
}

