namespace WpfInspectionApp.Models;

public enum LegacyAlgorithmGroup
{
    Normal,
    Extended
}

// NativeAlgoType: native MptiBridge flow path 가 알아야 하는 InspAlgoType 값.
// 0 (NativeAlgoTypeIds.Unknown) 은 flow path 미지원 — RuntimeFlowAlgorithmAdapter 가
// fallback (per-algo bridge) 로 떨어뜨림.
public sealed record AlgorithmCatalogItem(
    string Type,
    string DisplayName,
    LegacyAlgorithmGroup Group,
    int LegacyFlag,
    string LegacyName,
    string ParameterFamily,
    int NativeAlgoType = NativeAlgoTypeIds.Unknown);

public static class AlgorithmCatalog
{
    // NOTE: nativeAlgoType 가 0(Unknown) 인 항목은 아직 flow path 미연결. 추가 작업 시
    // native InspParamDef_Algo.h 에서 해당 eAlgo* 값 찾아 NativeAlgoTypeIds 에 상수 추가 +
    // 본 entry 에 인수 넣고 MptiBridgeFlow.cpp 의 switch 도 확장하면 됨.
    public static IReadOnlyList<AlgorithmCatalogItem> All { get; } =
    [
        Normal("AlgoBW", "BlackWhite", 2, "BlackWhite", "BlackWhite"),
        Normal("AlgoBlob", "Blob", 4, "Blob", "Blob", NativeAlgoTypeIds.Blob),
        Normal("AlgoAlign", "Align", 8, "Align", "Align", NativeAlgoTypeIds.Align),
        Normal("AlgoBody_Blob", "Body Blob", 16, "BodyBlob", "BodyBlob", NativeAlgoTypeIds.BodyBlob),
        Normal("AlgoTilt", "Tilt", 32, "Tilt", "Tilt"),
        Normal("AlgoOCR", "OCR", 64, "OCR", "OCR", NativeAlgoTypeIds.Ocr),
        Normal("AlgoPattern", "Pattern", 128, "Pattern", "Pattern", NativeAlgoTypeIds.Pattern),
        Normal("AlgoColor", "Color", 256, "Color", "Color"),
        Normal("AlgoGray_Mean", "Gray Mean", 512, "GrayMean", "Graymean", NativeAlgoTypeIds.GrayMean),
        Normal("AlgoHeight_Mean", "Height Mean", 1024, "HeightMean", "Heightmean", NativeAlgoTypeIds.HeightMean),
        Normal("AlgoGray_Diff", "Gray Diff", 2048, "GrayDiff", "Graydiff", NativeAlgoTypeIds.GrayDiff),
        Normal("AlgoHeight_Diff", "Height Diff", 4096, "HeightDiff", "Heightdiff", NativeAlgoTypeIds.HeightDiff),
        Normal("AlgoBridge", "Bridge", 8192, "Bridge", "Bridge"),
        Normal("AlgoLead_Tip", "Lead Tip", 16384, "LeadTip", "Leadtip"),
        Normal("AlgoLead_Lift", "Lead Lift", 32768, "LeadLift", "Leadlift"),
        Normal("AlgoLead_Solder", "Lead Solder", 65536, "LeadSolder", "Leadsolder"),
        Normal("AlgoLead_Color", "Lead Color", 131072, "LeadColor", "LeadColor"),
        Normal("AlgoWidth", "Width", 262144, "Width", "Length"),
        Normal("AlgoVolume", "Volume", 524288, "Volume", "Volume", NativeAlgoTypeIds.Volume),
        Normal("AlgoLead_Search", "Lead Search", 1048576, "LeadSearch", "Leadsearch"),
        Normal("AlgoLead_SideSolder", "Lead Side Solder", 2097152, "LeadSideSolder", "Leadsidesolder"),
        Normal("AlgoTab", "Tab", 4194304, "Tab", "Tab"),
        Normal("AlgoTab_Search", "Tab Search", 8388608, "TabSearch", "Tabsearch"),
        Normal("AlgoColorBand_Search", "Color Band Search", 16777216, "ColorBandSearch", "ColorBandSearch"),
        Normal("AlgoGrid", "Grid", 33554432, "Grid", "Grid"),
        Normal("AlgoLine", "Line", 67108864, "Line", "Line", NativeAlgoTypeIds.Line),
        Normal("AlgoEdge", "Edge", 134217728, "Edge", "Edge", NativeAlgoTypeIds.Edge),
        Normal("AlgoSolderCone", "Solder Cone", 268435456, "SolderCone", "Soldercone"),
        Normal("AlgoColorXY", "Color XY", 536870912, "ColorXY", "ColorXY"),
        Normal("AlgoPOCR", "POCR", 1073741824, "POCR", "POCR", NativeAlgoTypeIds.Pocr),
        Extended("AlgoAlignEdge", "Align Edge", 1, "AlignEdge", "AlignEdge"),
        Extended("AlgoPadAlign", "Pad Align", 1 << 1, "PadAlign", "PadAlign"),
        Extended("AlgoDisColor", "DisColor", 1 << 2, "DisColor", "DisColor"),
        Extended("AlgoBarcode", "Barcode", 1 << 3, "BarCode", "BarCode"),
        Extended("AlgoFillet", "Fillet", 1 << 4, "Fillet", "Fillet"),
        Extended("AlgoBGA", "BGA", 1 << 5, "BGA", "BGA", NativeAlgoTypeIds.Bga),
        Extended("AlgoBump", "Bump", 1 << 6, "Bump", "Bump", NativeAlgoTypeIds.Bump),
        Extended("AlgoNGBlob", "NG Blob", 1 << 7, "NGBlob", "NGBlob", NativeAlgoTypeIds.NgBlob),
        Extended("AlgoBodyEdge", "Body Edge", 1 << 8, "BodyEdge", "BodyEdge", NativeAlgoTypeIds.BodyEdge),
        Extended("AlgoPackageThickness", "Package Thickness", 1 << 9, "PackageThickness", "PackageThickness", NativeAlgoTypeIds.PackageThickness),
        Extended("AlgoDistance", "Distance", 1 << 10, "Distance", "Distance", NativeAlgoTypeIds.Distance),
        Extended("AlgoGWire", "G Wire", 1 << 11, "GWire", "GWire"),
        Extended("AlgoPatternDiff", "Pattern Diff", 1 << 12, "PatternDiff", "PatternDiff", NativeAlgoTypeIds.PatternDiff),
        Extended("AlgoPadBW", "Pad BW", 1 << 13, "Padbw", "PadBW", NativeAlgoTypeIds.PadBw),
        Extended("AlgoShapeX", "Shape X", 1 << 14, "ShapeX", "ShapeX", NativeAlgoTypeIds.ShapeX),
        Extended("AlgoPadArray", "Pad Array", 1 << 15, "PadArray", "PadArray"),
        Extended("AlgoWire", "Wire", 1 << 11, "GWire", "GWire"),
        Extended("AlgoFoot", "Foot", 1 << 4, "Foot", "Foot"),
        Extended("AlgoForeignOCV", "Foreign OCV", 1 << 12, "PatternDiff", "PatternDiff", NativeAlgoTypeIds.ForeignOcv),
        Extended("AlgoEdgePoint", "Edge Point", 1 << 10, "Distance", "Distance", NativeAlgoTypeIds.EdgePoint),
        Extended("AlgoLQBGA", "LQ BGA", 1 << 5, "BGA", "BGA", NativeAlgoTypeIds.LqBga)
    ];

    public static AlgorithmCatalogItem Find(string? type)
    {
        return All.FirstOrDefault(item => string.Equals(item.Type, type, StringComparison.OrdinalIgnoreCase))
            ?? All.First(item => item.Type == "AlgoAlign");
    }

    private static AlgorithmCatalogItem Normal(string type, string displayName, int legacyFlag, string legacyName, string parameterFamily, int nativeAlgoType = NativeAlgoTypeIds.Unknown)
    {
        return new AlgorithmCatalogItem(type, displayName, LegacyAlgorithmGroup.Normal, legacyFlag, legacyName, parameterFamily, nativeAlgoType);
    }

    private static AlgorithmCatalogItem Extended(string type, string displayName, int legacyFlag, string legacyName, string parameterFamily, int nativeAlgoType = NativeAlgoTypeIds.Unknown)
    {
        return new AlgorithmCatalogItem(type, displayName, LegacyAlgorithmGroup.Extended, legacyFlag, legacyName, parameterFamily, nativeAlgoType);
    }
}

