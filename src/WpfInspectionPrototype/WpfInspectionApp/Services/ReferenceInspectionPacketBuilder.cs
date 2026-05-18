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
            "BW" => ["InvChk", "InvertCheck", "PerOK", "Use2D", "Use3D", "R2D", "R3D", "Range", "MnMx2D", "MnMxAvg3D", "MinValue", "MaxValue", "ChipTrack", "UseTCR", "TCArea", "AreaCur", "StdOKArea", "HMean", "UseH"],
            "Blob" or "Body_Blob" or "Bump" => ["Inv", "UseIPC", "CSIPC", "MnMxArea", "Use2D", "TPR2D", "MnMx2D", "Use3D", "TPR3D", "MnMxAvgHeiR", "FillHole", "UseShift", "Shift", "MnMxTC", "UseTC", "TCW", "TCL", "Use2D_pat", "MnMx2D_pat", "Use3D_pat", "MnMxHei_pat", "Use2D_SB", "MnMx2D_SB", "Use3D_SB", "MnMxAvgHeiR_SB", "ROI"],
            "Align" => ["LTInfo", "SearMargin_mm", "InvChk", "Use2D", "TPR2D", "TPR3D", "MnMx2D", "MnMxAvgHeiR", "Use3D", "UseIPC", "CSIPC", "UseSft", "Sft", "UseAng", "Ang", "SearPnt", "Searsz", "SearMargin", "FH", "IOPT", "SSize", "UseFixedSize"],
            "AlignEdge" => ["InspDirection", "UsePeak", "MinEdge", "MaxEdge", "Search", "Edge", "Angle", "Offset", "ROI"],
            "Color" => ["InspTPClr", "InspTypeColor", "VwClr", "ViewColor", "CntPt", "CntPoint", "PolyCnt", "AutoSearchROI", "UseMap2", "Invert", "ColorLightType", "UseRange", "URGB", "CBRAG", "CBMIN", "CBMax", "Fator", "MnMxR", "SzCIE", "CntHst", "RStd", "RRst", "StdOK"],
            "ColorXY" => ["MnMxAvgX", "MnMxAvgY", "ColorXY", "Fator"],
            "Gray_Mean" => ["MnMxAvg_GM", "GrayRate", "GrayAvg", "Use2D", "Use3D", "R2D", "R3D", "MnMx2D", "MnMx3D", "PerOK", "MnMxArea"],
            "Gray_Diff" => ["ROI", "GD", "GrayDiff", "DiffUpLo", "DiffUpper", "DiffLower", "SInv", "MnMx", "Polar"],
            "Height_Mean" => ["InvChk", "PerOK", "Use2D", "Use3D", "R2D", "R3D", "MnMx2D", "MnMx3D", "UseHei", "MnMxAvgHei", "UseHigh", "MnMxHigh", "HighV", "UseLow", "MnMxLow", "LowV", "UseBW", "UseBWMin3D", "UseBWMax3D", "AddH", "MnMxArea"],
            "Height_Diff" => ["ROI", "HDMM", "HD_3D", "HeightDiff3D", "DiffUpLo", "DiffUpper", "DiffLower", "SInv", "MnMx", "Polar", "AddH", "UsEF", "EroF", "UBWR2"],
            "PadBW" => ["BData", "FData", "SubBData", "SubFData", "SubLight", "SubC", "BlobInfo", "BlobID", "BlobCX", "BlobCY", "BlobArea", "BlobWidth", "BlobLength", "BlobStX", "BlobStY", "AMROI", "ROI"],
            "NGBlob" => ["BData", "FData", "UseData", "UseData2", "SelV", "ROIF", "AMROI", "ROI"],
            "ShapeX" => ["OCRMxCnt", "ImgCBLTData", "ROICnt", "Dir", "MSc", "Hist", "Opt", "Asp", "thick", "VMxLeng", "HMxLeng", "NGA", "CrtA", "MxChLeng", "TBRate", "TPSel", "C3D", "WrFr", "NGGr", "ROI", "pCt", "UShp", "UExShp", "UIn", "UExi", "UShift", "ShpA", "ExShpA", "InA", "ExiA", "Shft", "ULoL", "UDnt", "Lol", "Dnt", "UFr", "Fr", "MoCnt", "ArrMo", "BW_ShA", "BW_NG"],
            "Bridge" => ["GD", "GrayDiff", "GapCnt", "LT_Dir", "LeadTipDirection", "Use3D", "HeiRefV", "HeightReferenceValue", "os", "Offset", "TPBrid", "TypeBridge", "PerOK", "PercentOK", "UseInsp2D", "UseInsp3D", "HeiDiff", "HeightDiff", "AutoSearchROI", "SBUSE", "SBArea", "SBAreaPer", "Insp2DUp", "Thin", "Line23D", "Use2DSB", "Use3DSB", "SB2D", "SB3D", "Mode2", "Option", "ArrValue", "BlobBN", "BlobBF", "ROI", "PtrrcGapRect"],
            "Tab" or "Tab_Search" => ["L_Dir", "LT_Dir", "L_Pos", "SD_Len", "GapCnt", "AutoSearchROI", "Tail_", "StdMgA_", "Ar_dWidth_", "Line_", "Ar_bLine_", "Ar_bWidth_", "Ar_bTab_", "Ar_bTail_", "BW_Tab_", "BW_NG1_", "BW_NG2_", "Hist1", "nHist2", "UseMinScarThickness", "MinScarThickness", "UseScarAspectRatio", "UseDisableTabArea", "dScAspRat", "UseCrossLineDetect", "UseCont", "UseVerticalMaxLen", "VerticalMaxLen", "UseHorizontalMaxLen", "HorizontalMaxLen", "UseMaxNGArea", "MaxNGArea", "UseEarseScarArea", "StI", "CriA", "NTab", "chipLg", "UChCriLn", "UTBarROp", "UTAO", "TPTab", "TypeTab", "WndAng", "WndAngle", "PolyCnt", "UseMap2", "Invert", "ColorLightType", "UseRange", "URGB", "CBRAG", "CBMIN", "CBMax"],
            "Lead_Tip" => ["LT_Pos", "LeadTipPos", "SearR", "SearchRange", "L_Dir", "LeadDirection", "InvChk", "InvertCheck", "PxPerV", "PixelPercentValue", "R2D", "N2dRange", "MnMx2D", "N2dBinaryMin", "N2dBinaryMax", "B3dCheck", "R3D", "N3dRange", "MnMxAvg3D", "N3dHeightMin", "N3dHeightMax", "D3dAvgHeight", "TipFNG", "TipFaultNG", "Use2D", "L_PerV", "LeadPercentValue", "NGTipOpt", "NGTipOption", "SearDirBody", "Use3DIn2D", "R3DIn2D", "MnMx3DIn2D", "UseST", "UseSideTip", "UseLW_szC", "UseST_Sft", "ST_SearArea", "ST_Sft", "FindOpt", "T_Length", "T_CapGap", "SideTPos", "SideTGap", "USideBin", "Side2dChk", "Side2dRg", "MnMxSide2D", "Side3dChk", "Side3dRg", "MnMxSide3D", "BMinArea", "SideGW", "TM", "ROFF", "TMAR", "ST_TF", "DirInv", "ST_SftPer", "UseST_SftUnitPer"],
            "Lead_Lift" => ["ROIWH", "LT_Itv", "LeadTipInterval", "LT_Dir", "L_Pos", "HeiCur3D", "CurrentHeight3D", "MnMxAvgHei", "TolerBand3D", "ToleranceBand3D", "MinV", "MinValue", "UseLL", "UseLeadLift", "UseCopla", "UseCoplanarity", "HeiDiff", "HeightDiif", "UseGrd", "UseGradient", "Grd", "Gradient", "UseCR", "CRMx", "CRMn", "CSIPC", "ArrN", "ArrF"],
            "Lead_Solder" => ["ROIWH", "L_Itv", "LeadInterval", "SD_Itv", "SolderInterval", "LT_Dir", "L_Pos", "SD_Len", "SD_Len2", "UseBW", "BWInspectionUse", "UseHM", "HeightMeanUse", "CurV_BW", "CurrentValueBW", "UseHD", "HeightDiffUse", "HeiAvg", "AvgHeight3D", "TolerBand3D", "BW", "BlackWhite", "HM", "HeightMean", "OKStd3DR", "OKStandard3DRate", "Ess", "UseBWCJ", "CJInterval", "CJWidth", "CJHeight", "BWCJ", "UseTArea3D", "TArea3D", "GapW", "UseExc", "UseCont", "IntervalCont", "IntervalContY", "ROICont", "T_Cont", "HLd", "HeiLead", "HLdMg", "HCalcMethod"],
            "Lead_Color" => ["LT_Dir", "L_Pos", "SD_Len", "InspTPClr", "InspTypeColor", "VwClr", "ViewColor", "CntPt", "PolyCnt", "AutoSearchROI", "UseMap2", "Invert", "ColorLightType", "UseRange", "URGB", "CBRAG", "CBMIN", "CBMax", "Fator", "MnMxR", "SzCIE", "CntHst", "RStd", "RRst", "StdOK"],
            "Lead_Search" => ["DPPansz", "DisplayPannelSize", "L_Pos", "LeadPosition", "SD_stPos", "SolderStartPos", "Ctn", "IsContainer", "Use3D", "THV", "ThresholdValue", "UseHei", "InspHeightFlag", "UseCop", "InspCoplanrity", "SDCnt", "SolderCnt", "Use2D", "L_W", "LeadWidth", "L_P", "LeadPitch", "StartLead", "EndLead", "L_st", "L_ed", "MSear", "IsManualSearch", "L_stl", "LeadStartLeft", "L_stlDstX", "MSearType", "SameWP", "SameWidthPitch"],
            "Lead_SideSolder" => ["LT_Dir", "LeadTipDirection", "L_Pos", "LeadPosition", "LL_V", "LeadLiftSetValue", "SD_Len", "SolderLength", "RectWH", "BW", "BlackWhite", "UseTeachA", "TeachA", "TeachA2", "GapW", "InspA"],
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
