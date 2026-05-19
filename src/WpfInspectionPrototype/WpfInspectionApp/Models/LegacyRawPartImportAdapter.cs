using System.Globalization;
using System.IO;
using System.Text;
using System.Xml.Linq;

namespace WpfInspectionApp.Models;

public static class LegacyRawPartImportAdapter
{
    private static readonly Dictionary<string, string> AlgorithmNameMap = new(StringComparer.OrdinalIgnoreCase)
    {
        ["BW"] = "AlgoBW",
        ["BlackWhite"] = "AlgoBW",
        ["Blob"] = "AlgoBlob",
        ["Align"] = "AlgoAlign",
        ["Body_Blob"] = "AlgoBody_Blob",
        ["BodyBlob"] = "AlgoBody_Blob",
        ["Tilt"] = "AlgoTilt",
        ["OCR"] = "AlgoOCR",
        ["Pattern"] = "AlgoPattern",
        ["Color"] = "AlgoColor",
        ["Gray_Mean"] = "AlgoGray_Mean",
        ["GrayMean"] = "AlgoGray_Mean",
        ["Height_Mean"] = "AlgoHeight_Mean",
        ["HeightMean"] = "AlgoHeight_Mean",
        ["Gray_Diff"] = "AlgoGray_Diff",
        ["GrayDiff"] = "AlgoGray_Diff",
        ["Height_Diff"] = "AlgoHeight_Diff",
        ["HeightDiff"] = "AlgoHeight_Diff",
        ["Bridge"] = "AlgoBridge",
        ["Lead_Tip"] = "AlgoLead_Tip",
        ["LeadTip"] = "AlgoLead_Tip",
        ["Lead_Lift"] = "AlgoLead_Lift",
        ["LeadLift"] = "AlgoLead_Lift",
        ["Lead_Solder"] = "AlgoLead_Solder",
        ["LeadSolder"] = "AlgoLead_Solder",
        ["Lead_Color"] = "AlgoLead_Color",
        ["LeadColor"] = "AlgoLead_Color",
        ["Width"] = "AlgoWidth",
        ["Volume"] = "AlgoVolume",
        ["Lead_Search"] = "AlgoLead_Search",
        ["LeadSearch"] = "AlgoLead_Search",
        ["Lead_SideSolder"] = "AlgoLead_SideSolder",
        ["LeadSideSolder"] = "AlgoLead_SideSolder",
        ["Tab"] = "AlgoTab",
        ["Tab_Search"] = "AlgoTab_Search",
        ["TabSearch"] = "AlgoTab_Search",
        ["ColorBand_Search"] = "AlgoColorBand_Search",
        ["ColorBandSearch"] = "AlgoColorBand_Search",
        ["Grid"] = "AlgoGrid",
        ["Line"] = "AlgoLine",
        ["Edge"] = "AlgoEdge",
        ["SolderCone"] = "AlgoSolderCone",
        ["ColorXY"] = "AlgoColorXY",
        ["POCR"] = "AlgoPOCR",
        ["AlignEdge"] = "AlgoAlignEdge",
        ["PadAlign"] = "AlgoPadAlign",
        ["DisColor"] = "AlgoDisColor",
        ["Barcode"] = "AlgoBarcode",
        ["BarCode"] = "AlgoBarcode",
        ["Fillet"] = "AlgoFillet",
        ["BGA"] = "AlgoBGA",
        ["Bump"] = "AlgoBump",
        ["NGBlob"] = "AlgoNGBlob",
        ["BodyEdge"] = "AlgoBodyEdge",
        ["PackageThickness"] = "AlgoPackageThickness",
        ["Distance"] = "AlgoDistance",
        ["GWire"] = "AlgoGWire",
        ["Wire"] = "AlgoWire",
        ["Foot"] = "AlgoFoot",
        ["PatternDiff"] = "AlgoPatternDiff",
        ["PadBW"] = "AlgoPadBW",
        ["Padbw"] = "AlgoPadBW",
        ["ShapeX"] = "AlgoShapeX",
        ["PadArray"] = "AlgoPadArray",
        ["ForeignOCV"] = "AlgoForeignOCV",
        ["LQBGA"] = "AlgoLQBGA",
        ["EdgePoint"] = "AlgoEdgePoint"
    };

    // Must match pemtoFramework.MVPPattern.InspAlgorithm.InspAlgoBase.InspAlgoType
    // and MPTILib/PInsp_Algo/InspParamDef_Algo.h exactly.
    private static readonly string[] AlgorithmOrdinalMap =
    [
        "AlgoBW",
        "AlgoBlob",
        "AlgoAlign",
        "AlgoBody_Blob",
        "AlgoTilt",
        "AlgoOCR",
        "AlgoPattern",
        "AlgoColor",
        "AlgoGray_Mean",
        "AlgoHeight_Mean",
        "AlgoGray_Diff",
        "AlgoHeight_Diff",
        "AlgoBridge",
        "AlgoLead_Tip",
        "AlgoLead_Lift",
        "AlgoLead_Solder",
        "AlgoLead_Color",
        "AlgoWidth",
        "AlgoVolume",
        "AlgoLead_Search",
        "AlgoLead_SideSolder",
        "AlgoTab",
        "AlgoTab_Search",
        "AlgoColorBand_Search",
        "AlgoGrid",
        "AlgoLine",
        "AlgoEdge",
        "AlgoSolderCone",
        "AlgoColorXY",
        "AlgoAlignEdge",
        "AlgoPadAlign",
        "AlgoPOCR",
        "AlgoDisColor",
        "AlgoWire",
        "AlgoFoot",
        "AlgoBarcode",
        "AlgoFillet",
        "AlgoBGA",
        "AlgoBump",
        "AlgoNGBlob",
        "AlgoPadBW",
        "AlgoForeignOCV",
        "AlgoBodyEdge",
        "AlgoPackageThickness",
        "AlgoDistance",
        "AlgoGWire",
        "AlgoEdgePoint",
        "AlgoPatternDiff",
        "AlgoShapeX",
        "AlgoPadArray",
        "AlgoLQBGA"
    ];

    private static readonly string[] NormalFlagOrder =
    [
        "AlgoBW",
        "AlgoBlob",
        "AlgoAlign",
        "AlgoBody_Blob",
        "AlgoTilt",
        "AlgoOCR",
        "AlgoPattern",
        "AlgoColor",
        "AlgoGray_Mean",
        "AlgoHeight_Mean",
        "AlgoGray_Diff",
        "AlgoHeight_Diff",
        "AlgoBridge",
        "AlgoLead_Tip",
        "AlgoLead_Lift",
        "AlgoLead_Solder",
        "AlgoLead_Color",
        "AlgoWidth",
        "AlgoVolume",
        "AlgoLead_Search",
        "AlgoLead_SideSolder",
        "AlgoTab",
        "AlgoTab_Search",
        "AlgoColorBand_Search",
        "AlgoGrid",
        "AlgoLine",
        "AlgoEdge",
        "AlgoSolderCone",
        "AlgoColorXY",
        "AlgoPOCR"
    ];

    private static readonly string[] ExtendedFlagOrder =
    [
        "AlgoAlignEdge",
        "AlgoPadAlign",
        "AlgoDisColor",
        "AlgoBarcode",
        "AlgoFillet",
        "AlgoBGA",
        "AlgoBump",
        "AlgoNGBlob",
        "AlgoBodyEdge",
        "AlgoPackageThickness",
        "AlgoDistance",
        "AlgoGWire",
        "AlgoPatternDiff",
        "AlgoPadBW",
        "AlgoShapeX",
        "AlgoPadArray"
    ];

    public static bool TryParse(string xml, out PartInspectionData part, out string status)
    {
        try
        {
            return TryParseDocument(XDocument.Parse(xml, LoadOptions.PreserveWhitespace), out part, out status);
        }
        catch (Exception ex)
        {
            part = new PartInspectionData();
            status = $"Legacy RawData XML parse failed: {ex.Message}";
            return false;
        }
    }

    public static bool TryParse(Stream xmlStream, out PartInspectionData part, out string status)
    {
        return TryParse(xmlStream, initStream: null, out part, out status);
    }

    public static bool TryParse(Stream xmlStream, Stream? initStream, out PartInspectionData part, out string status)
    {
        try
        {
            var document = XDocument.Load(xmlStream, LoadOptions.PreserveWhitespace);
            var initDocument = initStream == null ? null : XDocument.Load(initStream, LoadOptions.PreserveWhitespace);
            return TryParseDocument(document, initDocument, out part, out status);
        }
        catch (Exception ex)
        {
            part = new PartInspectionData();
            status = $"Legacy RawData XML parse failed: {ex.Message}";
            return false;
        }
    }

    private static bool TryParseDocument(XDocument document, out PartInspectionData part, out string status)
    {
        return TryParseDocument(document, initDocument: null, out part, out status);
    }

    private static bool TryParseDocument(XDocument document, XDocument? initDocument, out PartInspectionData part, out string status)
    {
        part = new PartInspectionData();
        status = "";
        var root = document.Root;
        if (root == null || !IsRawPartRoot(root))
        {
            status = "Legacy RawData XML parse failed: RawDataContainer or JobContainer root was not found.";
            return false;
        }

        var partElement = root.Element("PartData") ?? root.Element("Part") ?? root.Descendants("PartData").FirstOrDefault();
        var partName = FirstValue(partElement, "Name", "PartCode", "PackageName", "ID", "RefID");
        part.Name = string.IsNullOrWhiteSpace(partName) ? "Imported Raw Part" : DecodeLegacyText(partName);
        var transform = CreateRoiTransform(root, initDocument?.Root, partElement);
        part.PixelResolutionX = transform.PixelResolutionX;
        part.PixelResolutionY = transform.PixelResolutionY;
        part.SourceWidth = transform.SourceWidth;
        part.SourceHeight = transform.SourceHeight;

        var windowElements = root
            .Descendants("WindowDataList")
            .Elements()
            .Where(element => element.Name.LocalName is "WindowData" or "Window")
            .ToList();

        if (windowElements.Count == 0)
        {
            windowElements = root
                .Descendants()
                .Where(element => element.Name.LocalName is "WindowData" or "Window")
                .ToList();
        }

        var index = 1;
        foreach (var windowElement in windowElements)
        {
            var window = ParseWindow(windowElement, index, transform);
            part.Windows.Add(window);
            index++;
        }

        if (part.Windows.Count == 0)
        {
            status = "Legacy RawData XML parse failed: no WindowData nodes were found.";
            return false;
        }

        status = $"Legacy RawData imported: {part.Windows.Count} Window(s), {part.Windows.Sum(window => window.Algorithms.Count)} Algorithm(s). Resolution {FormatResolution(transform)}.";
        return true;
    }

    private static InspectionWindowData ParseWindow(XElement element, int index, LegacyRoiTransform transform)
    {
        var id = FirstValue(element, "ID");
        var name = DecodeLegacyText(FirstValue(element, "Name", "Nk"));
        if (string.IsNullOrWhiteSpace(name))
        {
            name = $"Window ROI {index}";
        }

        var window = new InspectionWindowData
        {
            Id = string.IsNullOrWhiteSpace(id) ? InspectionWindowData.CreateId() : id,
            Name = name.StartsWith("Window ROI", StringComparison.OrdinalIgnoreCase) ? name : $"Window ROI {index} - {name}",
            Roi = ParseRoi(element.Element("RelRoi") ?? element.Element("Roi"), transform)
        };

        var algorithmElements = element
            .Descendants("AlgorithmDataList")
            .Elements()
            .Where(child => child.Name.LocalName is "AlgorithmData" or "Algo")
            .ToList();

        if (algorithmElements.Count == 0)
        {
            algorithmElements = element
                .Elements()
                .Where(child => child.Name.LocalName is "AlgorithmData" or "Algo")
                .ToList();
        }

        var algorithmIndex = 1;
        foreach (var algorithmElement in algorithmElements)
        {
            window.Algorithms.Add(ParseAlgorithm(algorithmElement, algorithmIndex, transform));
            algorithmIndex++;
        }

        if (window.Algorithms.Count == 0)
        {
            foreach (var type in ResolveWindowAlgorithmTypes(element))
            {
                window.Algorithms.Add(CreateFlagAlgorithm(type, algorithmIndex, element));
                algorithmIndex++;
            }
        }

        return window;
    }

    private static InspectionAlgorithmData ParseAlgorithm(XElement element, int index, LegacyRoiTransform transform)
    {
        var legacyId = FirstValue(element, "ID");
        var rawType = FirstValue(element, "Type", "TP", "Kind", "Kind2");
        var algorithm = new InspectionAlgorithmData
        {
            Id = string.IsNullOrWhiteSpace(legacyId) ? InspectionAlgorithmData.CreateId() : legacyId,
            Type = ResolveAlgorithmType(rawType, element)
        };
        algorithm.ApplyCatalogDefaults();
        algorithm.DisplayName = $"{algorithm.DisplayName} #{index}";
        algorithm.AlgorithmRoi = ParseOptionalAlgorithmRoi(element, transform);
        algorithm.Parameters["Legacy.ID"] = legacyId;
        algorithm.Parameters["Legacy.Type"] = rawType;
        algorithm.Parameters["Legacy.TypeMeaning"] = "InspAlgoType ordinal";
        algorithm.Parameters["Legacy.Source"] = "RawData XML";

        foreach (var pair in FlattenLeafValues(element))
        {
            algorithm.Parameters[pair.Key] = pair.Value;
        }

        ApplyAlgorithmParameterMapping(algorithm, element, transform);

        algorithm.Result = new InspectionResultData
        {
            Message = "Imported from legacy RawData"
        };
        algorithm.PanelData = AlgorithmPanelSchema.Create(algorithm);
        return algorithm;
    }

    private static void ApplyAlgorithmParameterMapping(InspectionAlgorithmData algorithm, XElement element, LegacyRoiTransform transform)
    {
        if (string.Equals(algorithm.Type, "AlgoAlign", StringComparison.OrdinalIgnoreCase))
        {
            ApplyAlignParameters(algorithm, element, transform);
        }
        else if (string.Equals(algorithm.Type, "AlgoBW", StringComparison.OrdinalIgnoreCase))
        {
            ApplyBlackWhiteParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoBlob", StringComparison.OrdinalIgnoreCase)
            || string.Equals(algorithm.Type, "AlgoBody_Blob", StringComparison.OrdinalIgnoreCase)
            || string.Equals(algorithm.Type, "AlgoBump", StringComparison.OrdinalIgnoreCase))
        {
            ApplyBlobParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoNGBlob", StringComparison.OrdinalIgnoreCase))
        {
            ApplyNgBlobParameters(algorithm, element, transform);
        }
        else if (string.Equals(algorithm.Type, "AlgoPadBW", StringComparison.OrdinalIgnoreCase))
        {
            ApplyPadBwParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoShapeX", StringComparison.OrdinalIgnoreCase))
        {
            ApplyShapeXParameters(algorithm, element, transform);
        }
        else if (string.Equals(algorithm.Type, "AlgoColor", StringComparison.OrdinalIgnoreCase))
        {
            ApplyColorParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoColorXY", StringComparison.OrdinalIgnoreCase))
        {
            ApplyColorXyParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoGray_Mean", StringComparison.OrdinalIgnoreCase))
        {
            ApplyGrayMeanParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoGray_Diff", StringComparison.OrdinalIgnoreCase))
        {
            ApplyGrayDiffParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoHeight_Mean", StringComparison.OrdinalIgnoreCase))
        {
            ApplyHeightMeanParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoHeight_Diff", StringComparison.OrdinalIgnoreCase))
        {
            ApplyHeightDiffParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoBridge", StringComparison.OrdinalIgnoreCase))
        {
            ApplyBridgeParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoTab", StringComparison.OrdinalIgnoreCase)
            || string.Equals(algorithm.Type, "AlgoTab_Search", StringComparison.OrdinalIgnoreCase))
        {
            ApplyTabParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoLead_Tip", StringComparison.OrdinalIgnoreCase))
        {
            ApplyLeadTipParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoLead_Lift", StringComparison.OrdinalIgnoreCase))
        {
            ApplyLeadLiftParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoLead_Solder", StringComparison.OrdinalIgnoreCase))
        {
            ApplyLeadSolderParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoLead_Color", StringComparison.OrdinalIgnoreCase))
        {
            ApplyLeadColorParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoLead_Search", StringComparison.OrdinalIgnoreCase))
        {
            ApplyLeadSearchParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoLead_SideSolder", StringComparison.OrdinalIgnoreCase))
        {
            ApplyLeadSideSolderParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoWidth", StringComparison.OrdinalIgnoreCase))
        {
            ApplyWidthParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoVolume", StringComparison.OrdinalIgnoreCase))
        {
            ApplyVolumeParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoFillet", StringComparison.OrdinalIgnoreCase))
        {
            ApplyFilletParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoFoot", StringComparison.OrdinalIgnoreCase))
        {
            ApplyFootParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoSolderCone", StringComparison.OrdinalIgnoreCase))
        {
            ApplySolderConeParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoGrid", StringComparison.OrdinalIgnoreCase))
        {
            ApplyGridParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoLine", StringComparison.OrdinalIgnoreCase))
        {
            ApplyLineParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoEdge", StringComparison.OrdinalIgnoreCase))
        {
            ApplyEdgeParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoBarcode", StringComparison.OrdinalIgnoreCase))
        {
            ApplyBarcodeParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoPatternDiff", StringComparison.OrdinalIgnoreCase))
        {
            ApplyPatternDiffParameters(algorithm, element);
        }
        else if (string.Equals(algorithm.Type, "AlgoPadArray", StringComparison.OrdinalIgnoreCase))
        {
            ApplyPadArrayParameters(algorithm, element);
        }
    }

    private static void ApplyAlignParameters(InspectionAlgorithmData algorithm, XElement element, LegacyRoiTransform transform)
    {
        if (TryReadIntLeaf(element, out var searchNum, "SearNum", "SearchNum"))
        {
            SetInt(algorithm, "Align.SearchNum", Net48Compat.Clamp(searchNum, 1, 4));
        }
        else
        {
            var pointCount = Enumerable.Range(1, 4).Count(index => TryReadNumberArrayLeaf(element, out _, $"SearPnt{index}", $"ArrSearchPoint{index}"));
            if (pointCount > 0)
            {
                SetInt(algorithm, "Align.SearchNum", pointCount);
            }
        }

        if (TryReadNumberArrayLeaf(element, out var binaryRange, "MnMx2D") && binaryRange.Length >= 2)
        {
            SetInt(algorithm, "Align.BinaryMin", Round(binaryRange[0]));
            SetInt(algorithm, "Align.BinaryMax", Round(binaryRange[1]));
            SetInt(algorithm, "Align.Threshold", Round(binaryRange[0]));
        }

        if (TryReadBoolLeaf(element, out var invertCheck, "InvChk", "InvertCheck", "bInvertCheck"))
        {
            SetBool(algorithm, "Align.InvertCheck", invertCheck);
        }

        if (TryReadBoolLeaf(element, out var use2D, "Use2D"))
        {
            SetBool(algorithm, "Align.Use2D", use2D);
        }

        if (TryReadBoolLeaf(element, out var use3D, "Use3D"))
        {
            SetBool(algorithm, "Align.Use3D", use3D);
        }

        if (TryReadIntLeaf(element, out var range2D, "TPR2D"))
        {
            SetInt(algorithm, "Align.Range2DType", range2D);
        }

        if (TryReadIntLeaf(element, out var range3D, "TPR3D", "Range3D", "nTypeRange3D"))
        {
            SetInt(algorithm, "Align.Range3DType", range3D);
        }

        if (TryReadNumberArrayLeaf(element, out var heightRange, "MnMxAvgHeiR") && heightRange.Length >= 3)
        {
            SetDouble(algorithm, "Align.HeightRateMin", heightRange[0]);
            SetDouble(algorithm, "Align.HeightRateMax", heightRange[1]);
            SetDouble(algorithm, "Align.HeightAvg", heightRange[2]);
        }

        if (TryReadBoolLeaf(element, out var useIpc, "UseIPC", "bUseIPC"))
        {
            SetBool(algorithm, "Align.UseIPC", useIpc);
        }

        if (TryReadIntLeaf(element, out var ipcClass, "CSIPC", "IPCClass", "byIPCClass"))
        {
            SetInt(algorithm, "Align.IPCClass", ipcClass);
        }

        if (TryReadBoolLeaf(element, out var useShift, "UseSft", "UseShift"))
        {
            SetBool(algorithm, "Align.UseShift", useShift);
        }

        if (TryReadNumberArrayLeaf(element, out var shift, "Sft") && shift.Length >= 2)
        {
            SetDouble(algorithm, "Align.ShiftX", shift[0]);
            SetDouble(algorithm, "Align.ShiftY", shift[1]);
        }

        if (TryReadBoolLeaf(element, out var useAngle, "UseAng", "UseAngle"))
        {
            SetBool(algorithm, "Align.UseAngle", useAngle);
        }

        if (TryReadDoubleLeaf(element, out var angle, "Ang", "Angle"))
        {
            SetDouble(algorithm, "Align.Angle", angle);
        }

        if (TryReadIntLeaf(element, out var searchMargin, "SearMargin", "SearchMargin"))
        {
            SetInt(algorithm, "Align.SearchMargin", Math.Max(0, searchMargin));
        }
        else if (TryReadDoubleLeaf(element, out var searchMarginMm, "SearMargin_mm") && transform.PixelResolutionX > 0)
        {
            SetInt(algorithm, "Align.SearchMargin", Math.Max(0, Round(searchMarginMm / transform.PixelResolutionX)));
        }

        var defaultSearchSize = TryReadNumberArrayLeaf(element, out var searchSize, "Searsz", "ArrSearchSize")
            ? searchSize
            : Array.Empty<double>();
        var searchSizeMm = TryReadNumberArrayLeaf(element, out var arrSearchSizeMm, "ArrSearchSize_mm")
            ? arrSearchSizeMm
            : Array.Empty<double>();

        var defaultWidth = defaultSearchSize.Length >= 1 ? Math.Max(1, Round(defaultSearchSize[0])) : 0;
        var defaultHeight = defaultSearchSize.Length >= 2 ? Math.Max(1, Round(defaultSearchSize[1])) : 0;
        if (defaultWidth > 0)
        {
            SetInt(algorithm, "Align.SearchSizeX", defaultWidth);
        }

        if (defaultHeight > 0)
        {
            SetInt(algorithm, "Align.SearchSizeY", defaultHeight);
        }

        for (var index = 1; index <= 4; index++)
        {
            if (TryReadNumberArrayLeaf(element, out var point, $"SearPnt{index}", $"ArrSearchPoint{index}") && point.Length >= 2)
            {
                var pixelPoint = ConvertLegacyMmPointToPixel(point[0], point[1], transform);
                SetInt(algorithm, $"Align.SearchPoint{index}.X", pixelPoint.X);
                SetInt(algorithm, $"Align.SearchPoint{index}.Y", pixelPoint.Y);
            }

            var width = defaultWidth;
            var height = defaultHeight;
            var sizeOffset = (index - 1) * 2;
            if (searchSizeMm.Length > sizeOffset + 1 && transform.PixelResolutionX > 0 && transform.PixelResolutionY > 0)
            {
                width = Math.Max(1, Round(searchSizeMm[sizeOffset] / transform.PixelResolutionX));
                height = Math.Max(1, Round(searchSizeMm[sizeOffset + 1] / transform.PixelResolutionY));
            }

            if (width > 0)
            {
                SetInt(algorithm, $"Align.SearchSize{index}.W", width);
            }

            if (height > 0)
            {
                SetInt(algorithm, $"Align.SearchSize{index}.H", height);
            }
        }

        if (TryReadIntLeaf(element, out var minBlob, "MinBlob"))
        {
            SetInt(algorithm, "Align.MinBlobArea", Math.Max(1, minBlob));
        }

        if (TryReadBoolLeaf(element, out var fillHole, "FH"))
        {
            SetBool(algorithm, "Align.FillHole", fillHole);
        }

        if (TryReadIntLeaf(element, out var inspOpt, "IOPT", "InspOPT", "byInspOPT"))
        {
            SetInt(algorithm, "Align.InspOPT", inspOpt);
        }

        if (TryReadDoubleLeaf(element, out var fiduAngle, "FiduAngle", "dFiduAngle"))
        {
            SetDouble(algorithm, "Align.FiduAngle", fiduAngle);
        }

        if (TryReadBoolLeaf(element, out var sameSize, "SSize", "SameSize", "bSameSize"))
        {
            SetBool(algorithm, "Align.SameSize", sameSize);
        }

        if (TryReadBoolLeaf(element, out var useFixedSize, "UseFixedSize"))
        {
            SetBool(algorithm, "Align.UseFixedSize", useFixedSize);
        }

        SetBool(algorithm, "Import.AlignMapped", true);
    }

    private static void ApplyBlackWhiteParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        SetBoolIfPresent(algorithm, $"{family}.InvertCheck", element, "InvChk", "InvertCheck", "bInvertCheck");
        SetBoolIfPresent(algorithm, $"{family}.Use2D", element, "Use2D", "b2DCheck");
        SetBoolIfPresent(algorithm, $"{family}.Use3D", element, "Use3D", "b3DCheck");
        SetBoolIfPresent(algorithm, $"{family}.ChipTracking", element, "ChipTrack", "ChipTracking");
        SetBoolIfPresent(algorithm, $"{family}.UseTeachingRate", element, "UseTCR", "UseTeachingRate");
        SetBoolIfPresent(algorithm, $"{family}.UseHeightMean", element, "UseHMean", "UseHeightMean");
        SetBoolIfPresent(algorithm, $"{family}.UseHeightMeanMin", element, "UseHOpt", "UseHeightMeanMin");

        SetIntIfPresent(algorithm, $"{family}.Range2DType", element, "R2D", "TPR2D", "Range");
        SetIntIfPresent(algorithm, $"{family}.Range3DType", element, "R3D", "TPR3D", "N3dRange", "n3dRange");
        SetDoubleIfPresent(algorithm, $"{family}.PercentOK", element, "PerOK", "PercentOK");
        SetDoubleIfPresent(algorithm, $"{family}.TeachingArea", element, "TCArea2", "TeachingArea");
        SetDoubleIfPresent(algorithm, $"{family}.TeachingAreaPix", element, "TeachingAreaPix");
        SetDoubleIfPresent(algorithm, $"{family}.AreaCurrent", element, "AreaCur", "AreaCurrent");
        SetDoubleIfPresent(algorithm, $"{family}.StdOKArea", element, "StdOKArea");
        SetDoubleIfPresent(algorithm, $"{family}.ChipTrackingGap", element, "ChipTrackGap", "ChipTrackingGap");
        SetDoubleIfPresent(algorithm, $"{family}.HeightMeanMinUm", element, "HMeanMin", "HeightMeanMinUm");
        SetDoubleIfPresent(algorithm, $"{family}.HeightMeanMax", element, "HMeanMax", "HeightMeanMax");

        if (TryReadNumberArrayLeaf(element, out var binaryRange, "MnMx2D") && binaryRange.Length >= 2)
        {
            SetBinaryRange(algorithm, family, Round(binaryRange[0]), Round(binaryRange[1]));
        }
        else if (TryReadIntLeaf(element, out var minValue, "MinValue")
            && TryReadIntLeaf(element, out var maxValue, "MaxValue"))
        {
            SetBinaryRange(algorithm, family, minValue, maxValue);
        }

        if (TryReadNumberArrayLeaf(element, out var heightRange, "MnMxAvg3D", "MnMxAvgHeiR") && heightRange.Length >= 3)
        {
            SetDouble(algorithm, $"{family}.HeightMin", heightRange[0]);
            SetDouble(algorithm, $"{family}.HeightMax", heightRange[1]);
            SetDouble(algorithm, $"{family}.HeightAvg", heightRange[2]);
        }
        else
        {
            SetDoubleIfPresent(algorithm, $"{family}.HeightMin", element, "d3dMinValue", "D3dHeightMin");
            SetDoubleIfPresent(algorithm, $"{family}.HeightMax", element, "d3dMaxValue", "D3dHeightMax");
            SetDoubleIfPresent(algorithm, $"{family}.HeightAvg", element, "d3dAvgHeight", "D3dHeightAvg");
        }

        SetBool(algorithm, "Import.BlackWhiteMapped", true);
    }

    private static void ApplyBlobParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        SetBoolIfPresent(algorithm, $"{family}.InvertCheck", element, "Inv", "InvChk", "InvertCheck");
        SetBoolIfPresent(algorithm, $"{family}.Invert", element, "Inv", "InvChk", "InvertCheck");
        SetBoolIfPresent(algorithm, $"{family}.UseIPC", element, "UseIPC", "bUseIPC");
        SetIntIfPresent(algorithm, $"{family}.IpcClass", element, "CSIPC", "IPCClass", "byIPCClass");
        SetIntIfPresent(algorithm, $"{family}.IPCClass", element, "CSIPC", "IPCClass", "byIPCClass");
        SetBoolIfPresent(algorithm, $"{family}.Use2D", element, "Use2D");
        SetBoolIfPresent(algorithm, $"{family}.Use3D", element, "Use3D");
        SetBoolIfPresent(algorithm, $"{family}.UseShift", element, "UseShift", "UseSft");
        SetBoolIfPresent(algorithm, $"{family}.FillHole", element, "FillHole", "FH");
        SetBoolIfPresent(algorithm, $"{family}.UseFillHole", element, "FillHole", "FH");
        SetIntIfPresent(algorithm, $"{family}.Range2DType", element, "TPR2D", "R2D", "Range");
        SetIntIfPresent(algorithm, $"{family}.Range3DType", element, "TPR3D", "R3D", "Range3D");

        if (TryReadNumberArrayLeaf(element, out var areaRange, "MnMxArea") && areaRange.Length >= 2)
        {
            SetBool(algorithm, $"{family}.UseArea", true);
            SetDouble(algorithm, $"{family}.AreaMin", areaRange[0]);
            SetDouble(algorithm, $"{family}.AreaMax", areaRange[1]);
        }

        if (TryReadNumberArrayLeaf(element, out var binaryRange, "MnMx2D") && binaryRange.Length >= 2)
        {
            SetBinaryRange(algorithm, family, Round(binaryRange[0]), Round(binaryRange[1]));
        }

        if (TryReadNumberArrayLeaf(element, out var heightRange, "MnMxAvgHeiR", "MnMxAvg3D") && heightRange.Length >= 3)
        {
            SetDouble(algorithm, $"{family}.HeightRateMin", heightRange[0]);
            SetDouble(algorithm, $"{family}.HeightRateMax", heightRange[1]);
            SetDouble(algorithm, $"{family}.HeightAvg", heightRange[2]);
            SetDouble(algorithm, $"{family}.MinHeightRate", heightRange[0]);
            SetDouble(algorithm, $"{family}.MaxHeightRate", heightRange[1]);
            SetDouble(algorithm, $"{family}.TargetHeight", heightRange[2]);
            SetDouble(algorithm, $"{family}.MinHeight", heightRange[0]);
            SetDouble(algorithm, $"{family}.MaxHeight", heightRange[1]);
        }

        if (TryReadNumberArrayLeaf(element, out var shift, "Shift", "Sft") && shift.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.ShiftX", shift[0]);
            SetDouble(algorithm, $"{family}.ShiftY", shift[1]);
        }

        if (TryReadNumberArrayLeaf(element, out var teachWidthRate, "MnMxTCWR") && teachWidthRate.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.TeachWidthRateMin", teachWidthRate[0]);
            SetDouble(algorithm, $"{family}.TeachWidthRateMax", teachWidthRate[1]);
        }

        if (TryReadNumberArrayLeaf(element, out var teachLengthRate, "MnMxTCLR") && teachLengthRate.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.TeachLengthRateMin", teachLengthRate[0]);
            SetDouble(algorithm, $"{family}.TeachLengthRateMax", teachLengthRate[1]);
        }

        SetBoolIfPresent(algorithm, $"{family}.UseWidth", element, "UseTCW");
        SetBoolIfPresent(algorithm, $"{family}.UseLength", element, "UseTCL");
        SetDoubleIfPresent(algorithm, $"{family}.BlobSizeWidth", element, "TCW");
        SetDoubleIfPresent(algorithm, $"{family}.BlobSizeLength", element, "TCL");

        ApplyBlobPatternParameters(algorithm, element, family);
        ApplyBlobSubLineParameters(algorithm, element, family);

        SetBool(algorithm, "Import.BlobMapped", true);
    }

    private static void ApplyBlobPatternParameters(InspectionAlgorithmData algorithm, XElement element, string family)
    {
        SetBoolIfPresent(algorithm, $"{family}.UseForeignPattern", element, "Use2D_pat");
        SetBoolIfPresent(algorithm, $"{family}.ForeignPatternBinary", element, "Use2D_pat");

        if (TryReadNumberArrayLeaf(element, out var pattern2D, "MnMx2D_pat") && pattern2D.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.ForeignPattern2DMin", pattern2D[0]);
            SetDouble(algorithm, $"{family}.ForeignPattern2DMax", pattern2D[1]);
        }

        SetBoolIfPresent(algorithm, $"{family}.UseForeignPattern3D", element, "Use3D_pat");
        if (TryReadNumberArrayLeaf(element, out var pattern3D, "MnMxHei_pat", "MnMx3D_pat") && pattern3D.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.ForeignPatternHeightMin", pattern3D[0]);
            SetDouble(algorithm, $"{family}.ForeignPatternHeightMax", pattern3D[1]);
        }
    }

    private static void ApplyBlobSubLineParameters(InspectionAlgorithmData algorithm, XElement element, string family)
    {
        SetBoolIfPresent(algorithm, $"{family}.UseSubLine2D", element, "Use2D_SB");
        if (TryReadNumberArrayLeaf(element, out var subLine2D, "MnMx2D_SB") && subLine2D.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.SubLine2DMin", subLine2D[0]);
            SetDouble(algorithm, $"{family}.SubLine2DMax", subLine2D[1]);
        }

        SetBoolIfPresent(algorithm, $"{family}.UseSubLine3D", element, "Use3D_SB");
        if (TryReadNumberArrayLeaf(element, out var subLine3D, "MnMxAvgHeiR_SB") && subLine3D.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.SubLine3DMin", subLine3D[0]);
            SetDouble(algorithm, $"{family}.SubLine3DMax", subLine3D[1]);
            if (subLine3D.Length >= 3)
            {
                SetDouble(algorithm, $"{family}.SubLine3DAvg", subLine3D[2]);
            }
        }
    }

    private static void ApplyNgBlobParameters(InspectionAlgorithmData algorithm, XElement element, LegacyRoiTransform transform)
    {
        var family = algorithm.ParameterFamily;
        var bData = TryReadNumberArrayLeaf(element, out var parsedBData, "BData")
            ? parsedBData
            : Array.Empty<double>();
        var fData = TryReadNumberArrayLeaf(element, out var parsedFData, "FData")
            ? parsedFData
            : Array.Empty<double>();

        SetInt(algorithm, $"{family}.RawBDataCount", bData.Length);
        SetInt(algorithm, $"{family}.RawFDataCount", fData.Length);

        var dataFlags = TryReadIntLeaf(element, out var explicitDataFlags, "UseData")
            ? explicitDataFlags
            : ReadArrayInt(bData, 0);
        var data2Flags = TryReadIntLeaf(element, out var explicitData2Flags, "UseData2")
            ? explicitData2Flags
            : 0;

        SetInt(algorithm, $"{family}.DataFlags", dataFlags);
        SetInt(algorithm, $"{family}.Data2Flags", data2Flags);
        SetFlagBooleans(algorithm, family, dataFlags,
            ("Use2D", 0x01),
            ("Use3D", 0x02),
            ("UseFilter", 0x04),
            ("UseWidth", 0x08),
            ("UseLength", 0x10),
            ("UseArea", 0x20),
            ("UseColor", 0x40),
            ("UseFillHole", 0x80),
            ("FillHole", 0x80),
            ("UseExtraBumpPercent", 0x100),
            ("UseExtraBumpWidth", 0x200),
            ("UseHeight", 0x400),
            ("UseCoplanarity", 0x800),
            ("UseWarpageDeviation", 0x1000),
            ("UseWarpageDeviationX", 0x2000),
            ("UseWarpageDeviationY", 0x4000),
            ("UseHistogram", 0x8000),
            ("UseAreaFilter", 0x10000),
            ("UseClustering", 0x20000),
            ("UseAnd", 0x40000),
            ("UseContrast", 0x80000),
            ("UseColorContrast", 0x100000),
            ("UseUnderHeight", 0x200000),
            ("UseChippingType", 0x400000),
            ("UseBumpNgToOk", 0x800000),
            ("UseScratchView", 0x1000000),
            ("UseScDistance", 0x2000000),
            ("UseScThickness", 0x4000000),
            ("UseBallDamage", 0x8000000),
            ("UseBallLand", 0x10000000),
            ("UseWidthC", 0x20000000),
            ("UseLengthC", 0x40000000));
        SetFlagBooleans(algorithm, family, data2Flags,
            ("UseUnderArea", 0x01),
            ("UseRelativeHeight", 0x02),
            ("UseSideWidth", 0x04),
            ("UseSideLength", 0x08),
            ("UseSideArea", 0x10),
            ("UseAI", 0x20),
            ("UseRectSize", 0x40),
            ("UseContrastExceptRange", 0x80),
            ("UseInsideBubbleWidth", 0x100),
            ("UseInsideBubbleLength", 0x200),
            ("UseInsideBubbleArea", 0x400));

        if (bData.Length >= 4)
        {
            SetBinaryRange(algorithm, family, ReadArrayInt(bData, 2), ReadArrayInt(bData, 3, 255));
        }

        SetInt(algorithm, $"{family}.Range2DType", ReadArrayInt(bData, 4));
        SetInt(algorithm, $"{family}.Range3DType", ReadArrayInt(bData, 5));
        SetInt(algorithm, $"{family}.InspType", ReadArrayInt(bData, 6));
        SetInt(algorithm, $"{family}.InspArea", ReadArrayInt(bData, 7));
        SetInt(algorithm, $"{family}.WarpageOption", ReadArrayInt(bData, 8));
        SetBool(algorithm, $"{family}.UseExceptPattern", ReadArrayInt(bData, 9) != 0);
        SetInt(algorithm, $"{family}.PatternScore", ReadArrayInt(bData, 10, 80));
        SetInt(algorithm, $"{family}.BallRangeMin", ReadArrayInt(bData, 11));
        SetInt(algorithm, $"{family}.BallRangeMax", ReadArrayInt(bData, 12));
        SetInt(algorithm, $"{family}.HistoMin2D", ReadArrayInt(bData, 13));
        SetInt(algorithm, $"{family}.HistoMax2D", ReadArrayInt(bData, 14));
        SetInt(algorithm, $"{family}.LimitMin", ReadArrayInt(bData, 15));
        SetInt(algorithm, $"{family}.LimitMax", ReadArrayInt(bData, 16, 255));
        SetInt(algorithm, $"{family}.RangeHistogramType", ReadArrayInt(bData, 17));
        SetInt(algorithm, $"{family}.MinAreaFilter", ReadArrayInt(bData, 18));
        SetInt(algorithm, $"{family}.GridX", ReadArrayInt(bData, 19));
        SetInt(algorithm, $"{family}.GridY", ReadArrayInt(bData, 20));
        SetInt(algorithm, $"{family}.HistogramLocalType", ReadArrayInt(bData, 21));
        SetInt(algorithm, $"{family}.MinContrast", ReadArrayInt(bData, 22));
        SetInt(algorithm, $"{family}.ColorContrast", ReadArrayInt(bData, 23));
        SetInt(algorithm, $"{family}.DefineNumber", ReadArrayInt(bData, 24));
        SetInt(algorithm, $"{family}.TargetType", ReadArrayInt(bData, 25));
        SetInt(algorithm, $"{family}.MinimumNG", ReadArrayInt(bData, 26));
        SetInt(algorithm, $"{family}.BlobType", ReadArrayInt(bData, 27));
        SetInt(algorithm, $"{family}.AIModelID", ReadArrayInt(bData, 28, -1));
        SetInt(algorithm, $"{family}.ContrastExceptRangeNum", ReadArrayInt(bData, 29));
        SetInt(algorithm, $"{family}.ContrastExceptRangeMode", ReadArrayInt(bData, 30));
        SetInt(algorithm, $"{family}.KernelSize", ReadArrayInt(bData, 31));
        SetInt(algorithm, $"{family}.DownSample", ReadArrayInt(bData, 32));
        SetInt(algorithm, $"{family}.InsideThreshold", ReadArrayInt(bData, 33));

        ApplyNgBlobFloatParameters(algorithm, family, fData);

        if (TryReadIntLeaf(element, out var selectValue, "SelV"))
        {
            SetInt(algorithm, $"{family}.SelectValue", selectValue);
        }

        for (var index = 1; index <= 4; index++)
        {
            ApplyNgBlobRoi(algorithm, element, family, transform, index);
        }

        SetBool(algorithm, "Import.NGBlobMapped", true);
    }

    private static void ApplyNgBlobFloatParameters(InspectionAlgorithmData algorithm, string family, IReadOnlyList<double> fData)
    {
        SetDouble(algorithm, $"{family}.HeightMin", ReadArrayDouble(fData, 0));
        SetDouble(algorithm, $"{family}.HeightMax", ReadArrayDouble(fData, 1));
        SetDouble(algorithm, $"{family}.MinHeight", ReadArrayDouble(fData, 0));
        SetDouble(algorithm, $"{family}.MaxHeight", ReadArrayDouble(fData, 1));
        SetDouble(algorithm, $"{family}.WidthLimit", ReadArrayDouble(fData, 2));
        SetDouble(algorithm, $"{family}.LengthLimit", ReadArrayDouble(fData, 3));
        SetDouble(algorithm, $"{family}.AreaLimit", ReadArrayDouble(fData, 4));
        SetDouble(algorithm, $"{family}.WidthMax", ReadArrayDouble(fData, 2));
        SetDouble(algorithm, $"{family}.LengthMax", ReadArrayDouble(fData, 3));
        SetDouble(algorithm, $"{family}.AreaMin", ReadArrayDouble(fData, 4));
        SetDouble(algorithm, $"{family}.ExtraBumpPercent", ReadArrayDouble(fData, 5));
        SetDouble(algorithm, $"{family}.ExtraBumpWidth", ReadArrayDouble(fData, 6));
        SetDouble(algorithm, $"{family}.OverHeightMax", ReadArrayDouble(fData, 7));
        SetDouble(algorithm, $"{family}.ScDistanceMax", ReadArrayDouble(fData, 8));
        SetDouble(algorithm, $"{family}.ScThicknessMax", ReadArrayDouble(fData, 9));
        SetDouble(algorithm, $"{family}.WarpageDeviation", ReadArrayDouble(fData, 10));
        SetDouble(algorithm, $"{family}.WarpageDeviationX", ReadArrayDouble(fData, 11));
        SetDouble(algorithm, $"{family}.WarpageDeviationY", ReadArrayDouble(fData, 12));
        SetDouble(algorithm, $"{family}.HistoMinValue", ReadArrayDouble(fData, 32));
        SetDouble(algorithm, $"{family}.HistoMaxValue", ReadArrayDouble(fData, 33));
        SetDouble(algorithm, $"{family}.HistoLocalAreaRate", ReadArrayDouble(fData, 34));
        SetDouble(algorithm, $"{family}.ClusteringPitch", ReadArrayDouble(fData, 35));
        SetDouble(algorithm, $"{family}.GroupingNumber", ReadArrayDouble(fData, 36));
        SetDouble(algorithm, $"{family}.Group", ReadArrayDouble(fData, 37));
        SetDouble(algorithm, $"{family}.ForeignMaskWidth", ReadArrayDouble(fData, 38));
        SetDouble(algorithm, $"{family}.ForeignMaskHeight", ReadArrayDouble(fData, 39));
        SetDouble(algorithm, $"{family}.WarpageDeviationMin", ReadArrayDouble(fData, 40));
        SetDouble(algorithm, $"{family}.WidthCLimit", ReadArrayDouble(fData, 41));
        SetDouble(algorithm, $"{family}.LengthCLimit", ReadArrayDouble(fData, 42));
        SetDouble(algorithm, $"{family}.SideWidthMin", ReadArrayDouble(fData, 43));
        SetDouble(algorithm, $"{family}.SideWidthMax", ReadArrayDouble(fData, 44));
        SetDouble(algorithm, $"{family}.SideLengthMin", ReadArrayDouble(fData, 45));
        SetDouble(algorithm, $"{family}.SideLengthMax", ReadArrayDouble(fData, 46));
        SetDouble(algorithm, $"{family}.SideAreaMin", ReadArrayDouble(fData, 47));
        SetDouble(algorithm, $"{family}.SideAreaMax", ReadArrayDouble(fData, 48));
        SetDouble(algorithm, $"{family}.WidthMin", ReadArrayDouble(fData, 49));
        SetDouble(algorithm, $"{family}.InsideBubbleWidth", ReadArrayDouble(fData, 50));
        SetDouble(algorithm, $"{family}.InsideBubbleLength", ReadArrayDouble(fData, 51));
        SetDouble(algorithm, $"{family}.InsideBubbleArea", ReadArrayDouble(fData, 52));
    }

    private static void ApplyNgBlobRoi(InspectionAlgorithmData algorithm, XElement element, string family, LegacyRoiTransform transform, int index)
    {
        if (!TryReadNumberArrayLeaf(element, out var roi, $"ROIF{index}") || roi.Length < 4)
        {
            return;
        }

        var prefix = $"{family}.Roi{index}";
        SetDouble(algorithm, $"{prefix}.Left", roi[0]);
        SetDouble(algorithm, $"{prefix}.Top", roi[1]);
        SetDouble(algorithm, $"{prefix}.Right", roi[2]);
        SetDouble(algorithm, $"{prefix}.Bottom", roi[3]);
        algorithm.Parameters[$"{prefix}.Raw"] = string.Join(",", roi.Select(value => value.ToString("0.########", CultureInfo.InvariantCulture)));

        var leftTop = ConvertLegacyMmPointToPixel(roi[0], roi[1], transform);
        var rightBottom = ConvertLegacyMmPointToPixel(roi[2], roi[3], transform);
        SetInt(algorithm, $"{prefix}.PixelLeft", leftTop.X);
        SetInt(algorithm, $"{prefix}.PixelTop", leftTop.Y);
        SetInt(algorithm, $"{prefix}.PixelRight", rightBottom.X);
        SetInt(algorithm, $"{prefix}.PixelBottom", rightBottom.Y);
    }

    private static void ApplyShapeXParameters(InspectionAlgorithmData algorithm, XElement element, LegacyRoiTransform transform)
    {
        var family = algorithm.ParameterFamily;

        SetIntIfPresent(algorithm, $"{family}.RoiCount", element, "ROICnt");
        SetIntIfPresent(algorithm, $"{family}.Direction", element, "Dir");
        SetIntIfPresent(algorithm, $"{family}.MatchScore", element, "MSc");
        SetIntIfPresent(algorithm, $"{family}.Histogram1", element, "Hist1");
        SetIntIfPresent(algorithm, $"{family}.Histogram2", element, "Hist2");
        SetDoubleIfPresent(algorithm, $"{family}.ScarAspectRatio", element, "Asp");
        SetDoubleIfPresent(algorithm, $"{family}.AspectRatio", element, "Asp");
        SetDoubleIfPresent(algorithm, $"{family}.MinScarThickness", element, "thick");
        SetDoubleIfPresent(algorithm, $"{family}.VerticalMaxLength", element, "VMxLeng");
        SetDoubleIfPresent(algorithm, $"{family}.HorizontalMaxLength", element, "HMxLeng");
        SetDoubleIfPresent(algorithm, $"{family}.MaxNGArea", element, "NGA");
        SetDoubleIfPresent(algorithm, $"{family}.CriticalArea", element, "CrtA");
        SetDoubleIfPresent(algorithm, $"{family}.ChippingMaxLength", element, "MxChLeng");
        SetDoubleIfPresent(algorithm, $"{family}.MaxChippingLength", element, "MxChLeng");
        SetDoubleIfPresent(algorithm, $"{family}.TiebarRate", element, "TBRate");
        SetDoubleIfPresent(algorithm, $"{family}.TieBarRate", element, "TBRate");
        SetIntIfPresent(algorithm, $"{family}.SelectBlob", element, "TPSelBlob");
        SetIntIfPresent(algorithm, $"{family}.SelectTarget", element, "TPSelTarget");
        SetDoubleIfPresent(algorithm, $"{family}.Check3DMax", element, "C3DMx");
        SetDoubleIfPresent(algorithm, $"{family}.Check3DMin", element, "C3DMn");
        SetIntIfPresent(algorithm, $"{family}.WarningForeignCount", element, "WrFrC");
        SetDoubleIfPresent(algorithm, $"{family}.WarningForeignWidth", element, "WrFrW");
        SetDoubleIfPresent(algorithm, $"{family}.WarningForeignLength", element, "WrFrL");
        SetDoubleIfPresent(algorithm, $"{family}.NGGroupingMaxSize", element, "NGGrM");
        SetDoubleIfPresent(algorithm, $"{family}.NGGroupingDistance", element, "NGGrD");

        var optionFlags = TryReadIntLeaf(element, out var parsedOptionFlags, "Opt") ? parsedOptionFlags : 0;
        SetInt(algorithm, $"{family}.OptionFlags", optionFlags);
        SetFlagBooleans(algorithm, family, optionFlags,
            ("UseMinScarThickness", 0x01),
            ("UseScarAspectRatio", 0x02),
            ("UseCrossLineDetect", 0x04),
            ("UseVerticalMaxLength", 0x08),
            ("UseHorizontalMaxLength", 0x10),
            ("UseMaxNGArea", 0x20),
            ("UseChippingCriticalLine", 0x40),
            ("TieBarROpt", 0x80),
            ("EraseScarArea", 0x100),
            ("UseExistInnerArea", 0x200),
            ("LineChipping", 0x400),
            ("NonMatchingMode", 0x800),
            ("IncludeSide", 0x1000),
            ("UseWarningForeignCount", 0x2000),
            ("UseWarningForeignWidth", 0x4000),
            ("UseWarningForeignLength", 0x8000),
            ("UseNGGrouping", 0x10000),
            ("UseContrast", 0x20000),
            ("UseSwapXY", 0x40000),
            ("UseNG1SubtractNG2", 0x80000));

        if (TryReadNumberArrayLeaf(element, out var shapeAreaBlob, "BW_ShA_"))
        {
            ApplyShapeXBlobBaseParameters(algorithm, family, "ShapeAreaBlob", shapeAreaBlob, promoteRuntimeKeys: true);
        }

        if (TryReadNumberArrayLeaf(element, out var ngBlob1, "BW_NG1_"))
        {
            ApplyShapeXBlobBaseParameters(algorithm, family, "NG1Blob", ngBlob1, promoteRuntimeKeys: false);
        }

        if (TryReadNumberArrayLeaf(element, out var ngBlob2, "BW_NG2_"))
        {
            ApplyShapeXBlobBaseParameters(algorithm, family, "NG2Blob", ngBlob2, promoteRuntimeKeys: false);
        }

        var roiCount = TryReadIntLeaf(element, out var parsedRoiCount, "ROICnt") ? parsedRoiCount : 0;
        var limit = Math.Min(Math.Max(roiCount, 0), 200);
        for (var index = 0; index < limit; index++)
        {
            ApplyShapeXRoi(algorithm, element, family, transform, index);
        }

        SetBool(algorithm, "Import.ShapeXMapped", true);
    }

    private static void ApplyColorParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        SetIntIfPresent(algorithm, $"{family}.ColorInspSetValue", element, "InspTypeColor", "InspTPClr");
        SetIntIfPresent(algorithm, $"{family}.InspTypeColor", element, "InspTypeColor", "InspTPClr");
        SetIntIfPresent(algorithm, $"{family}.Color", element, "ViewColor", "VwClr");
        SetIntIfPresent(algorithm, $"{family}.ViewColor", element, "ViewColor", "VwClr");
        SetIntIfPresent(algorithm, $"{family}.ArrayCopyCount", element, "CntPoint", "CntPt", "PolyCnt", "PolygonCnt");
        SetIntIfPresent(algorithm, $"{family}.PolygonCount", element, "PolyCnt", "PolygonCnt");
        SetBoolIfPresent(algorithm, $"{family}.AutoSearchROI", element, "AutoSearchROI");
        SetBoolIfPresent(algorithm, $"{family}.UseAngleColor", element, "UseMap2", "UseColorMap2");
        SetBoolIfPresent(algorithm, $"{family}.Invert", element, "Invert");
        SetIntIfPresent(algorithm, $"{family}.RoiLight", element, "ColorLightType");
        SetIntIfPresent(algorithm, $"{family}.ColorLightType", element, "ColorLightType");
        SetBoolIfPresent(algorithm, $"{family}.ColorJudge", element, "UseRange", "UseRangeBar");
        SetBoolIfPresent(algorithm, $"{family}.UseRangeBar", element, "UseRange", "UseRangeBar");
        SetBoolIfPresent(algorithm, $"{family}.StandardOK", element, "StandardOK", "StdOK");
        SetIntIfPresent(algorithm, $"{family}.RangeMode", element, "RangeMode", "RMode");

        if (TryReadNumberArrayLeaf(element, out var useRgb, "URGB", "UseRGB") && useRgb.Length >= 3)
        {
            SetBool(algorithm, $"{family}.UseR", Math.Abs(useRgb[0]) > double.Epsilon);
            SetBool(algorithm, $"{family}.UseG", Math.Abs(useRgb[1]) > double.Epsilon);
            SetBool(algorithm, $"{family}.UseB", Math.Abs(useRgb[2]) > double.Epsilon);
        }

        if (TryReadNumberArrayLeaf(element, out var colorRange, "CBRAG", "Range") && colorRange.Length >= 3)
        {
            SetInt(algorithm, $"{family}.RangeR", Round(colorRange[0]));
            SetInt(algorithm, $"{family}.RangeG", Round(colorRange[1]));
            SetInt(algorithm, $"{family}.RangeB", Round(colorRange[2]));
        }

        if (TryReadNumberArrayLeaf(element, out var colorMin, "CBMIN", "ColorMin") && colorMin.Length >= 3)
        {
            SetInt(algorithm, $"{family}.MinR", Round(colorMin[0]));
            SetInt(algorithm, $"{family}.MinG", Round(colorMin[1]));
            SetInt(algorithm, $"{family}.MinB", Round(colorMin[2]));
        }

        if (TryReadNumberArrayLeaf(element, out var colorMax, "CBMax", "CBMAX", "ColorMax") && colorMax.Length >= 3)
        {
            SetInt(algorithm, $"{family}.MaxR", Round(colorMax[0]));
            SetInt(algorithm, $"{family}.MaxG", Round(colorMax[1]));
            SetInt(algorithm, $"{family}.MaxB", Round(colorMax[2]));
        }

        if (TryReadNumberArrayLeaf(element, out var factor, "Fator") && factor.Length >= 3)
        {
            SetDouble(algorithm, $"{family}.FatorRed", factor[0]);
            SetDouble(algorithm, $"{family}.FatorGreen", factor[1]);
            SetDouble(algorithm, $"{family}.FatorBlue", factor[2]);
        }
        else
        {
            SetDoubleIfPresent(algorithm, $"{family}.FatorRed", element, "FatorRed");
            SetDoubleIfPresent(algorithm, $"{family}.FatorGreen", element, "FatorGreen");
            SetDoubleIfPresent(algorithm, $"{family}.FatorBlue", element, "FatorBlue");
        }

        if (TryReadNumberArrayLeaf(element, out var colorRangeMinMax, "MnMxR") && colorRangeMinMax.Length >= 2)
        {
            SetInt(algorithm, $"{family}.RangeMin", Round(colorRangeMinMax[0]));
            SetInt(algorithm, $"{family}.RangeMax", Round(colorRangeMinMax[1]));
        }
        else
        {
            SetIntIfPresent(algorithm, $"{family}.RangeMin", element, "RangeMin");
            SetIntIfPresent(algorithm, $"{family}.RangeMax", element, "RangeMax");
        }

        if (TryReadNumberArrayLeaf(element, out var cieSize, "SzCIE") && cieSize.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.SizeXCIE", cieSize[0]);
            SetDouble(algorithm, $"{family}.SizeYCIE", cieSize[1]);
        }
        else
        {
            SetDoubleIfPresent(algorithm, $"{family}.SizeXCIE", element, "SizeXCIE");
            SetDoubleIfPresent(algorithm, $"{family}.SizeYCIE", element, "SizeYCIE");
        }

        SetIntIfPresent(algorithm, $"{family}.CntHistoStd", element, "CntHistoStd", "CntHstStd");
        SetDoubleIfPresent(algorithm, $"{family}.RateStd", element, "RateStd", "RStd");
        SetIntIfPresent(algorithm, $"{family}.CntHistoResult", element, "CntHistoResult", "CntHstRst");
        SetDoubleIfPresent(algorithm, $"{family}.RateResult", element, "RateResult", "RRst");

        SetBool(algorithm, "Import.ColorMapped", true);
    }

    private static void ApplyColorXyParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        if (TryReadNumberArrayLeaf(element, out var avgX, "MnMxAvgX") && avgX.Length >= 3)
        {
            SetDouble(algorithm, $"{family}.MinX", avgX[0]);
            SetDouble(algorithm, $"{family}.MaxX", avgX[1]);
            SetDouble(algorithm, $"{family}.AvgX", avgX[2]);
        }
        else
        {
            SetDoubleIfPresent(algorithm, $"{family}.MinX", element, "ColorXYMinX");
            SetDoubleIfPresent(algorithm, $"{family}.MaxX", element, "ColorXYMaxX");
            SetDoubleIfPresent(algorithm, $"{family}.AvgX", element, "ColorXYAvgX");
        }

        if (TryReadNumberArrayLeaf(element, out var avgY, "MnMxAvgY") && avgY.Length >= 3)
        {
            SetDouble(algorithm, $"{family}.MinY", avgY[0]);
            SetDouble(algorithm, $"{family}.MaxY", avgY[1]);
            SetDouble(algorithm, $"{family}.AvgY", avgY[2]);
        }
        else
        {
            SetDoubleIfPresent(algorithm, $"{family}.MinY", element, "ColorXYMinY");
            SetDoubleIfPresent(algorithm, $"{family}.MaxY", element, "ColorXYMaxY");
            SetDoubleIfPresent(algorithm, $"{family}.AvgY", element, "ColorXYAvgY");
        }

        if (TryReadNumberArrayLeaf(element, out var factor, "Fator") && factor.Length >= 3)
        {
            SetDouble(algorithm, $"{family}.FatorRed", factor[0]);
            SetDouble(algorithm, $"{family}.FatorGreen", factor[1]);
            SetDouble(algorithm, $"{family}.FatorBlue", factor[2]);
        }
        else
        {
            SetDoubleIfPresent(algorithm, $"{family}.FatorRed", element, "FatorRed");
            SetDoubleIfPresent(algorithm, $"{family}.FatorGreen", element, "FatorGreen");
            SetDoubleIfPresent(algorithm, $"{family}.FatorBlue", element, "FatorBlue");
        }

        SetBool(algorithm, "Import.ColorXYMapped", true);
    }

    private static void ApplyGrayMeanParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        ApplyCommonRangeParameters(algorithm, element);
        if (TryReadNumberArrayLeaf(element, out var grayMean, "MnMxAvg_GM") && grayMean.Length >= 3)
        {
            SetDouble(algorithm, $"{family}.GrayRateMin", grayMean[0]);
            SetDouble(algorithm, $"{family}.GrayRateMax", grayMean[1]);
            SetDouble(algorithm, $"{family}.GrayAvg", grayMean[2]);
        }
        else
        {
            SetDoubleIfPresent(algorithm, $"{family}.GrayRateMin", element, "GrayRateMin");
            SetDoubleIfPresent(algorithm, $"{family}.GrayRateMax", element, "GrayRateMax");
            SetDoubleIfPresent(algorithm, $"{family}.GrayAvg", element, "GrayAvg");
        }

        SetBool(algorithm, "Import.GrayMeanMapped", true);
    }

    private static void ApplyGrayDiffParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        SetDoubleIfPresent(algorithm, $"{family}.StandardGray", element, "GrayDiff", "GD");
        SetBoolIfPresent(algorithm, $"{family}.Polarity", element, "Polarity", "Polar");
        SetBoolIfPresent(algorithm, $"{family}.SignInversion", element, "SignInversion", "SInv");
        SetIntIfPresent(algorithm, $"{family}.MinMaxFlag", element, "MinMaxflag", "MnMx");

        if (TryReadBoolArrayLeaf(element, out var diffUpperLower, "DiffUpLo") && diffUpperLower.Length >= 2)
        {
            SetBool(algorithm, $"{family}.UpperStandard", diffUpperLower[0]);
            SetBool(algorithm, $"{family}.LowerStandard", diffUpperLower[1]);
        }
        else
        {
            SetBoolIfPresent(algorithm, $"{family}.UpperStandard", element, "DiffUpper");
            SetBoolIfPresent(algorithm, $"{family}.LowerStandard", element, "DiffLower");
        }

        ApplyDiffRoiParameters(algorithm, element, family);
        SetBool(algorithm, "Import.GrayDiffMapped", true);
    }

    private static void ApplyHeightMeanParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        ApplyCommonRangeParameters(algorithm, element);
        if (TryReadNumberArrayLeaf(element, out var heightMean, "MnMxAvgHei") && heightMean.Length >= 3)
        {
            SetDouble(algorithm, $"{family}.MinHeight", heightMean[0]);
            SetDouble(algorithm, $"{family}.MaxHeight", heightMean[1]);
            SetDouble(algorithm, $"{family}.TargetHeight", heightMean[2]);
            SetDouble(algorithm, $"{family}.AvgHeight3D", heightMean[2]);
        }

        SetBoolIfPresent(algorithm, $"{family}.UseHeight", element, "HeightUse", "UseHei");
        SetBoolIfPresent(algorithm, $"{family}.UseHighest", element, "UseHighest", "UseHigh");
        SetBoolIfPresent(algorithm, $"{family}.UseLowest", element, "UseLowest", "UseLow");
        SetBoolIfPresent(algorithm, $"{family}.UseBW", element, "BWOption", "UseBW");
        SetBoolIfPresent(algorithm, $"{family}.UseHeightMin3D", element, "UseHeightMin3D", "UseBWMin3D");
        SetBoolIfPresent(algorithm, $"{family}.UseHeightMax3D", element, "UseHeightMax3D", "UseBWMax3D");

        if (TryReadNumberArrayLeaf(element, out var highest, "MnMxHigh") && highest.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.HighestMin3D", highest[0]);
            SetDouble(algorithm, $"{family}.HighestMax3D", highest[1]);
        }

        if (TryReadNumberArrayLeaf(element, out var lowest, "MnMxLow") && lowest.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.LowestMin3D", lowest[0]);
            SetDouble(algorithm, $"{family}.LowestMax3D", lowest[1]);
        }

        SetDoubleIfPresent(algorithm, $"{family}.HighestValue", element, "HighestValue", "HighV");
        SetDoubleIfPresent(algorithm, $"{family}.LowestValue", element, "LowestValue", "LowV");

        if (TryReadNumberArrayLeaf(element, out var addHeight, "AddH") && addHeight.Length > 0)
        {
            SetOptionalDouble(algorithm, $"{family}.CorrectionValue", addHeight, 0);
            SetOptionalDouble(algorithm, $"{family}.CorrectionUpper", addHeight, 1);
            SetOptionalDouble(algorithm, $"{family}.CorrectionLower", addHeight, 2);
            SetOptionalDouble(algorithm, $"{family}.AddHeight", addHeight, 0);
            SetOptionalDouble(algorithm, $"{family}.AddHighest", addHeight, 1);
            SetOptionalDouble(algorithm, $"{family}.AddLowest", addHeight, 2);
        }

        SetBool(algorithm, "Import.HeightMeanMapped", true);
    }

    private static void ApplyHeightDiffParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        SetBoolIfPresent(algorithm, $"{family}.Polarity", element, "Polarity", "Polar");
        SetBoolIfPresent(algorithm, $"{family}.SignInversion", element, "SignInversion", "SInv");
        SetIntIfPresent(algorithm, $"{family}.MinMaxFlag", element, "MinMaxflag", "MnMx");
        SetDoubleIfPresent(algorithm, $"{family}.AddHeight", element, "AddH");
        SetBoolIfPresent(algorithm, $"{family}.UseErodeFilter", element, "UsEF");
        SetIntIfPresent(algorithm, $"{family}.ErodeFilter", element, "EroF");
        SetBoolIfPresent(algorithm, $"{family}.UseBlobRoi2", element, "UBWR2");

        if (TryReadNumberArrayLeaf(element, out var heightDiff, "HDMM") && heightDiff.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.HeightDiff3D", heightDiff[0]);
            SetDouble(algorithm, $"{family}.HeightDiff3DMax", heightDiff[1]);
            SetDouble(algorithm, $"{family}.CurrentHeightDiff", heightDiff[0]);
        }
        else
        {
            SetDoubleIfPresent(algorithm, $"{family}.HeightDiff3D", element, "HeightDiff3D", "HD_3D");
            SetDoubleIfPresent(algorithm, $"{family}.CurrentHeightDiff", element, "HeightDiff3D", "HD_3D");
        }

        if (TryReadBoolArrayLeaf(element, out var diffUpperLower, "DiffUpLo") && diffUpperLower.Length >= 2)
        {
            SetBool(algorithm, $"{family}.DiffUpper", diffUpperLower[0]);
            SetBool(algorithm, $"{family}.DiffLower", diffUpperLower[1]);
        }
        else
        {
            SetBoolIfPresent(algorithm, $"{family}.DiffUpper", element, "DiffUpper");
            SetBoolIfPresent(algorithm, $"{family}.DiffLower", element, "DiffLower");
        }

        ApplyDiffRoiParameters(algorithm, element, family);
        SetBool(algorithm, "Import.HeightDiffMapped", true);
    }

    private static void ApplyBridgeParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        SetIntIfPresent(algorithm, $"{family}.GrayDiff", element, "GrayDiff", "GD");
        SetIntIfPresent(algorithm, $"{family}.GapCnt", element, "GapCnt");
        SetIntIfPresent(algorithm, $"{family}.GapCount", element, "GapCnt");
        SetIntIfPresent(algorithm, $"{family}.LeadTipDirection", element, "LeadTipDirection", "LT_Dir");
        SetBoolIfPresent(algorithm, $"{family}.Use3D", element, "Use3D");
        SetDoubleIfPresent(algorithm, $"{family}.HeightReferenceValue", element, "HeightReferenceValue", "HeiRefV");
        SetDoubleIfPresent(algorithm, $"{family}.HeightDiff2D3D", element, "HeightReferenceValue", "HeiRefV");
        SetBoolIfPresent(algorithm, $"{family}.Offset", element, "Offset", "os");
        SetIntIfPresent(algorithm, $"{family}.TypeBridge", element, "TypeBridge", "TPBrid");
        SetDoubleIfPresent(algorithm, $"{family}.PercentOK", element, "PercentOK", "PerOK");
        SetBoolIfPresent(algorithm, $"{family}.UseInsp2D", element, "UseInsp2D");
        SetBoolIfPresent(algorithm, $"{family}.Use2DInspection", element, "UseInsp2D");
        SetBoolIfPresent(algorithm, $"{family}.UseInsp3D", element, "UseInsp3D");
        SetBoolIfPresent(algorithm, $"{family}.Use3DInspection", element, "UseInsp3D");
        SetDoubleIfPresent(algorithm, $"{family}.HeightDiff", element, "HeightDiff", "HeiDiff");
        SetBoolIfPresent(algorithm, $"{family}.AutoSearchROI", element, "AutoSearchROI");
        SetBoolIfPresent(algorithm, $"{family}.UseSolderBall", element, "UseSolderBall", "SBUSE");
        SetDoubleIfPresent(algorithm, $"{family}.SolderBallArea", element, "SBArea");
        SetDoubleIfPresent(algorithm, $"{family}.SolderBallAreaPer", element, "SBAreaPer");
        SetBoolIfPresent(algorithm, $"{family}.Insp2DUpper", element, "Insp2DUp");
        SetDoubleIfPresent(algorithm, $"{family}.ThinBridge", element, "Thin");
        SetIntIfPresent(algorithm, $"{family}.Line2D3D", element, "Line23D");
        SetBoolIfPresent(algorithm, $"{family}.UseMode2", element, "UseMode2", "Mode2");
        SetIntIfPresent(algorithm, $"{family}.Sorting", element, "Sorting", "Sort");

        if (TryReadIntLeaf(element, out var optionFlags, "Option"))
        {
            SetInt(algorithm, $"{family}.OptionFlags", optionFlags);
            SetBool(algorithm, $"{family}.UseBinarize", HasFlag(optionFlags, 0x01));
            SetBool(algorithm, $"{family}.UseArea", HasFlag(optionFlags, 0x02));
            SetBool(algorithm, $"{family}.UseWidth", HasFlag(optionFlags, 0x04));
            SetBool(algorithm, $"{family}.UseLength", HasFlag(optionFlags, 0x08));
        }

        if (TryReadNumberArrayLeaf(element, out var arrValue, "ArrValue"))
        {
            SetOptionalDouble(algorithm, $"{family}.AreaMax", arrValue, 0);
            SetOptionalDouble(algorithm, $"{family}.WidthMax", arrValue, 1);
            SetOptionalDouble(algorithm, $"{family}.LengthMax", arrValue, 2);
            SetOptionalDouble(algorithm, $"{family}.TotalMax", arrValue, 3);
        }

        if (TryReadNumberArrayLeaf(element, out var solderBall2D, "SB2D") && solderBall2D.Length >= 4)
        {
            SetDouble(algorithm, $"{family}.SolderBallRange2D", solderBall2D[0]);
            SetDouble(algorithm, $"{family}.SolderBallRange3D", solderBall2D[1]);
            SetDouble(algorithm, $"{family}.SolderBall2DMin", solderBall2D[2]);
            SetDouble(algorithm, $"{family}.SolderBall2DMax", solderBall2D[3]);
        }

        if (TryReadNumberArrayLeaf(element, out var solderBall3D, "SB3D") && solderBall3D.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.SolderBall3DMin", solderBall3D[0]);
            SetDouble(algorithm, $"{family}.SolderBall3DMax", solderBall3D[1]);
        }

        if (TryReadNumberArrayLeaf(element, out var blobBaseNear, "BlobBN"))
        {
            ApplyShapeXBlobBaseParameters(algorithm, family, "NearBlob", blobBaseNear, promoteRuntimeKeys: false);
        }

        if (TryReadNumberArrayLeaf(element, out var blobBaseFar, "BlobBF"))
        {
            ApplyShapeXBlobBaseParameters(algorithm, family, "FarBlob", blobBaseFar, promoteRuntimeKeys: false);
        }

        var roiCount = 0;
        for (var index = 1; index <= 64; index++)
        {
            if (TryReadLeafValue(element, out var roi, $"ROI{index}", $"PtrrcGapRect{index}"))
            {
                algorithm.Parameters[$"{family}.GapRoi{index}.Raw"] = roi;
                roiCount++;
            }
        }

        if (roiCount > 0)
        {
            SetInt(algorithm, $"{family}.GapRoiCount", roiCount);
        }

        SetBool(algorithm, "Import.BridgeMapped", true);
    }

    private static void ApplyTabParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        ApplyColorParameters(algorithm, element);
        ApplyLeadPositionParameters(algorithm, element, family);
        SetBoolIfPresent(algorithm, $"{family}.AutoSearchROI", element, "AutoSearchROI");
        SetIntIfPresent(algorithm, $"{family}.TabCount", element, "GapCnt", "NTab");
        SetIntIfPresent(algorithm, $"{family}.TypeTab", element, "TypeTab", "TPTab");
        SetDoubleIfPresent(algorithm, $"{family}.TailArea", element, "Tail_", "Tail");
        SetDoubleIfPresent(algorithm, $"{family}.StandardMarginArea", element, "StdMgA_");
        SetDoubleIfPresent(algorithm, $"{family}.TabLength", element, "chipLg");
        SetDoubleIfPresent(algorithm, $"{family}.MaxChippingLength", element, "chipLg");
        SetBoolIfPresent(algorithm, $"{family}.UseChippingCriticalLine", element, "UChCriLn");
        SetBoolIfPresent(algorithm, $"{family}.UseTieBarRateOption", element, "UTBarROp");
        SetBoolIfPresent(algorithm, $"{family}.UseTieAreaOption", element, "UTAO");
        SetDoubleIfPresent(algorithm, $"{family}.StartIndex", element, "StI");
        SetDoubleIfPresent(algorithm, $"{family}.CriticalArea", element, "CriA", "CrtA");
        SetBoolIfPresent(algorithm, $"{family}.UseMinScarThickness", element, "UseMinScarThickness");
        SetDoubleIfPresent(algorithm, $"{family}.MinScarThickness", element, "MinScarThickness");
        SetBoolIfPresent(algorithm, $"{family}.UseScarAspectRatio", element, "UseScarAspectRatio");
        SetDoubleIfPresent(algorithm, $"{family}.ScarAspectRatio", element, "dScAspRat");
        SetBoolIfPresent(algorithm, $"{family}.UseDisableTabArea", element, "UseDisableTabArea");
        SetBoolIfPresent(algorithm, $"{family}.UseCrossLineDetect", element, "UseCrossLineDetect");
        SetBoolIfPresent(algorithm, $"{family}.UseVerticalMaxLength", element, "UseVerticalMaxLen");
        SetDoubleIfPresent(algorithm, $"{family}.VerticalMaxLength", element, "VerticalMaxLen");
        SetBoolIfPresent(algorithm, $"{family}.UseHorizontalMaxLength", element, "UseHorizontalMaxLen");
        SetDoubleIfPresent(algorithm, $"{family}.HorizontalMaxLength", element, "HorizontalMaxLen");
        SetBoolIfPresent(algorithm, $"{family}.UseMaxNGArea", element, "UseMaxNGArea");
        SetDoubleIfPresent(algorithm, $"{family}.MaxNGArea", element, "MaxNGArea");
        SetBoolIfPresent(algorithm, $"{family}.UseEraseScarArea", element, "UseEarseScarArea", "UseEraseScarArea");
        SetIntIfPresent(algorithm, $"{family}.WindowAngle", element, "WndAngle", "WndAng");

        if (TryReadNumberArrayLeaf(element, out var widths, "Ar_dWidth_"))
        {
            SetOptionalDouble(algorithm, $"{family}.TabWidth", widths, 0);
            SetIndexedDoubles(algorithm, family, "TabWidth", widths);
        }

        if (TryReadNumberArrayLeaf(element, out var lines, "Line_"))
        {
            SetOptionalDouble(algorithm, $"{family}.Pitch", lines, 0);
            SetIndexedDoubles(algorithm, family, "Line", lines);
        }

        CopyRawArrayIfPresent(algorithm, element, family, "LineUse", "Ar_bLine_");
        CopyRawArrayIfPresent(algorithm, element, family, "WidthUse", "Ar_bWidth_");
        CopyRawArrayIfPresent(algorithm, element, family, "TabUse", "Ar_bTab_");
        CopyRawArrayIfPresent(algorithm, element, family, "TailUse", "Ar_bTail_");

        if (TryReadNumberArrayLeaf(element, out var tabBlob, "BW_Tab_"))
        {
            ApplyShapeXBlobBaseParameters(algorithm, family, "TabBlob", tabBlob, promoteRuntimeKeys: true);
        }

        if (TryReadNumberArrayLeaf(element, out var ngBlob1, "BW_NG1_"))
        {
            ApplyShapeXBlobBaseParameters(algorithm, family, "NG1Blob", ngBlob1, promoteRuntimeKeys: false);
        }

        if (TryReadNumberArrayLeaf(element, out var ngBlob2, "BW_NG2_"))
        {
            ApplyShapeXBlobBaseParameters(algorithm, family, "NG2Blob", ngBlob2, promoteRuntimeKeys: false);
        }

        SetIntIfPresent(algorithm, $"{family}.Histogram1", element, "Hist1");
        SetIntIfPresent(algorithm, $"{family}.Histogram2", element, "nHist2", "Hist2");
        SetBool(algorithm, "Import.TabMapped", true);
    }

    private static void ApplyLeadTipParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        SetDoubleIfPresent(algorithm, $"{family}.LeadTipPosition", element, "LeadTipPos", "LT_Pos", "LT_Pos_mm");
        SetDoubleIfPresent(algorithm, $"{family}.SearchRange", element, "SearchRange", "SearR", "SearR_mm");
        SetDoubleIfPresent(algorithm, $"{family}.Gap", element, "SearchRange", "SearR", "SearR_mm");
        SetIntIfPresent(algorithm, $"{family}.LeadDirection", element, "LeadDirection", "L_Dir");
        SetBoolIfPresent(algorithm, $"{family}.InvertCheck", element, "InvertCheck", "InvChk");
        SetIntIfPresent(algorithm, $"{family}.PixelPercentValue", element, "PixelPercentValue", "PxPerV");
        SetIntIfPresent(algorithm, $"{family}.Range2DType", element, "N2dRange", "R2D");
        SetIntIfPresent(algorithm, $"{family}.Range3DType", element, "N3dRange", "R3D");
        SetBoolIfPresent(algorithm, $"{family}.Use2D", element, "B2dCheck", "Use2D");
        SetBoolIfPresent(algorithm, $"{family}.Use3D", element, "B3dCheck", "Use3D");
        SetBoolIfPresent(algorithm, $"{family}.NG", element, "TipFaultNG", "TipFNG");
        SetIntIfPresent(algorithm, $"{family}.LeadFindPercent", element, "LeadPercentValue", "L_PerV");
        SetIntIfPresent(algorithm, $"{family}.NGTipOption", element, "NGTipOption", "NGTipOpt");
        SetIntIfPresent(algorithm, $"{family}.SearchDirBody", element, "SearchDirBody", "SearDirBody");
        SetBoolIfPresent(algorithm, $"{family}.Use3DIn2D", element, "B3DIn2dCheck", "Use3DIn2D");
        SetIntIfPresent(algorithm, $"{family}.Range3DIn2D", element, "N3DIn2DRange", "R3DIn2D");
        SetBoolIfPresent(algorithm, $"{family}.UseSideTip", element, "UseSideTip", "UseST");
        SetBoolIfPresent(algorithm, $"{family}.UseLeadWindowSizeChange", element, "UseLeadWindowSizeChange", "UseLW_szC");
        SetBoolIfPresent(algorithm, $"{family}.UseSideTipShift", element, "UseSideTipShift", "UseST_Sft");
        SetDoubleIfPresent(algorithm, $"{family}.SideTipSearchArea", element, "SideTipSearchArea", "ST_SearArea");
        SetIntIfPresent(algorithm, $"{family}.FindOption", element, "FindOpt");
        SetDoubleIfPresent(algorithm, $"{family}.TipLength", element, "T_Length");
        SetBoolIfPresent(algorithm, $"{family}.UseTipLength", element, "UseTipLength", "T_Length");
        SetDoubleIfPresent(algorithm, $"{family}.CapGap", element, "T_CapGap");
        SetBoolIfPresent(algorithm, $"{family}.UseTipCap", element, "UseTipCap", "T_CapGap");
        SetDoubleIfPresent(algorithm, $"{family}.SideTipPosition", element, "SideTPos");
        SetDoubleIfPresent(algorithm, $"{family}.SideTipGap", element, "SideTGap");
        SetBoolIfPresent(algorithm, $"{family}.UseSideBin", element, "USideBin");
        SetBoolIfPresent(algorithm, $"{family}.Side2DCheck", element, "Side2dChk");
        SetIntIfPresent(algorithm, $"{family}.Side2DRange", element, "Side2dRg");
        SetBoolIfPresent(algorithm, $"{family}.Side3DCheck", element, "Side3dChk");
        SetIntIfPresent(algorithm, $"{family}.Side3DRange", element, "Side3dRg");
        SetDoubleIfPresent(algorithm, $"{family}.BaseMinArea", element, "BMinArea");
        SetDoubleIfPresent(algorithm, $"{family}.SideTipGapWidth", element, "SideGW");
        SetIntIfPresent(algorithm, $"{family}.TemplateMode", element, "TM");
        SetDoubleIfPresent(algorithm, $"{family}.RoiOffset", element, "ROFF", "ROFF_mm");
        SetDoubleIfPresent(algorithm, $"{family}.TemplateMatchAreaRate", element, "TMAR");
        SetIntIfPresent(algorithm, $"{family}.SideTipFindType", element, "ST_TF");
        SetBoolIfPresent(algorithm, $"{family}.DirectionInvert", element, "DirInv");
        SetBoolIfPresent(algorithm, $"{family}.UseSideTipShiftUnitPercent", element, "UseSideTipShiftUnitPer", "UseST_SftUnitPer");

        if (TryReadNumberArrayLeaf(element, out var binaryRange, "MnMx2D") && binaryRange.Length >= 2)
        {
            SetBinaryRange(algorithm, family, Round(binaryRange[0]), Round(binaryRange[1]));
        }
        else
        {
            SetIntIfPresent(algorithm, $"{family}.BinaryMin", element, "N2dBinaryMin");
            SetIntIfPresent(algorithm, $"{family}.BinaryMax", element, "N2dBinaryMax");
        }

        if (TryReadNumberArrayLeaf(element, out var heightRange, "MnMxAvg3D") && heightRange.Length >= 3)
        {
            SetDouble(algorithm, $"{family}.HeightMin3D", heightRange[0]);
            SetDouble(algorithm, $"{family}.HeightMax3D", heightRange[1]);
            SetDouble(algorithm, $"{family}.AvgHeight3D", heightRange[2]);
        }
        else
        {
            SetDoubleIfPresent(algorithm, $"{family}.HeightMin3D", element, "N3dHeightMin");
            SetDoubleIfPresent(algorithm, $"{family}.HeightMax3D", element, "N3dHeightMax");
            SetDoubleIfPresent(algorithm, $"{family}.AvgHeight3D", element, "D3dAvgHeight");
        }

        if (TryReadNumberArrayLeaf(element, out var heightIn2D, "MnMx3DIn2D") && heightIn2D.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.Height3DIn2DMin", heightIn2D[0]);
            SetDouble(algorithm, $"{family}.Height3DIn2DMax", heightIn2D[1]);
        }
        else
        {
            SetDoubleIfPresent(algorithm, $"{family}.Height3DIn2DMin", element, "N3DIn2DMinValue");
            SetDoubleIfPresent(algorithm, $"{family}.Height3DIn2DMax", element, "N3DIn2DMaxValue");
        }

        if (TryReadNumberArrayLeaf(element, out var sideTipShift, "ST_Sft") && sideTipShift.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.SideTipShiftX", sideTipShift[0]);
            SetDouble(algorithm, $"{family}.SideTipShiftY", sideTipShift[1]);
        }

        if (TryReadNumberArrayLeaf(element, out var sideTipShiftPercent, "ST_SftPer") && sideTipShiftPercent.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.SideTipShiftPercentX", sideTipShiftPercent[0]);
            SetDouble(algorithm, $"{family}.SideTipShiftPercentY", sideTipShiftPercent[1]);
        }

        SetBool(algorithm, "Import.LeadTipMapped", true);
    }

    private static void ApplyLeadLiftParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        ApplyLeadPositionParameters(algorithm, element, family);
        ApplySizeArrayParameters(algorithm, element, family, "Width", "Height", "ROIWH", "ROIWH_mm");
        SetDoubleIfPresent(algorithm, $"{family}.Interval", element, "LeadTipInterval", "LT_Itv", "LT_Itv_mm");
        SetIntIfPresent(algorithm, $"{family}.LeadTipDirection", element, "LeadTipDirection", "LT_Dir");
        SetDoubleIfPresent(algorithm, $"{family}.CurrentPosition", element, "LeadPosition", "L_Pos", "L_Pos_mm");
        SetDoubleIfPresent(algorithm, $"{family}.CurrentHeight3D", element, "CurrentHeight3D", "HeiCur3D");
        SetDoubleIfPresent(algorithm, $"{family}.ToleranceBand3D", element, "ToleranceBand3D", "TolerBand3D");
        SetDoubleIfPresent(algorithm, $"{family}.MinValue", element, "MinValue", "MinV");
        SetBoolIfPresent(algorithm, $"{family}.UseLeadLift", element, "UseLeadLift", "UseLL");
        SetBoolIfPresent(algorithm, $"{family}.UseLeadCoplanarity", element, "UseCoplanarity", "UseCopla");
        SetBoolIfPresent(algorithm, $"{family}.UseCoplanarity", element, "UseCoplanarity", "UseCopla");
        SetDoubleIfPresent(algorithm, $"{family}.HeightDiff", element, "HeightDiif", "HeightDiff", "HeiDiff");
        SetBoolIfPresent(algorithm, $"{family}.UseGradient", element, "UseGradient", "UseGrd");
        SetDoubleIfPresent(algorithm, $"{family}.Gradient", element, "Gradient", "Grd");
        SetBoolIfPresent(algorithm, $"{family}.UseFilletHeight", element, "UseCR");
        SetDoubleIfPresent(algorithm, $"{family}.FilletHeightMax", element, "CRMx");
        SetDoubleIfPresent(algorithm, $"{family}.FilletHeightMin", element, "CRMn");
        SetIntIfPresent(algorithm, $"{family}.IpcClass", element, "CSIPC");

        if (TryReadNumberArrayLeaf(element, out var heightRange, "MnMxAvgHei") && heightRange.Length >= 3)
        {
            SetDouble(algorithm, $"{family}.HeightMin3D", heightRange[0]);
            SetDouble(algorithm, $"{family}.HeightMax3D", heightRange[1]);
            SetDouble(algorithm, $"{family}.AvgHeight3D", heightRange[2]);
        }

        CopyRawArrayIfPresent(algorithm, element, family, "ArrN", "ArrN");
        CopyRawArrayIfPresent(algorithm, element, family, "ArrF", "ArrF");
        SetBool(algorithm, "Import.LeadLiftMapped", true);
    }

    private static void ApplyLeadSolderParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        ApplyLeadPositionParameters(algorithm, element, family);
        ApplySizeArrayParameters(algorithm, element, family, "Width", "Height", "ROIWH", "ROIWH_mm");
        SetDoubleIfPresent(algorithm, $"{family}.LeadInterval", element, "LeadInterval", "L_Itv", "L_Itv_mm");
        SetDoubleIfPresent(algorithm, $"{family}.SolderInterval", element, "SolderInterval", "SD_Itv", "SD_Itv_mm");
        SetIntIfPresent(algorithm, $"{family}.LeadTipDirection", element, "LeadTipDirection", "LT_Dir");
        SetDoubleIfPresent(algorithm, $"{family}.CurrentPosition", element, "LeadPosition", "L_Pos", "L_Pos_mm");
        SetDoubleIfPresent(algorithm, $"{family}.SolderLength", element, "SolderLength", "SD_Len", "SD_Len_mm");
        SetDoubleIfPresent(algorithm, $"{family}.SolderLength2", element, "SD_Len2");
        SetBoolIfPresent(algorithm, $"{family}.UseBW", element, "BWInspectionUse", "UseBW");
        SetBoolIfPresent(algorithm, $"{family}.UseMean", element, "HeightMeanUse", "UseHM");
        SetBoolIfPresent(algorithm, $"{family}.UseHeightDiff", element, "HeightDiffUse", "UseHD");
        SetDoubleIfPresent(algorithm, $"{family}.CurrentValueBW", element, "CurrentValueBW", "CurV_BW");
        SetDoubleIfPresent(algorithm, $"{family}.AvgHeight3D", element, "AvgHeight3D", "HeiAvg");
        SetDoubleIfPresent(algorithm, $"{family}.PermissibleRange", element, "ToleranceBand3D", "TolerBand3D");
        SetDoubleIfPresent(algorithm, $"{family}.HeightDiff", element, "HeightDiff");
        SetDoubleIfPresent(algorithm, $"{family}.OkStandard3DRate", element, "OKStandard3DRate", "OKStd3DR");
        SetBoolIfPresent(algorithm, $"{family}.UseBWCJ", element, "UseBWCJ");
        SetDoubleIfPresent(algorithm, $"{family}.CJInterval", element, "CJInterval");
        SetDoubleIfPresent(algorithm, $"{family}.CJWidth", element, "CJWidth");
        SetDoubleIfPresent(algorithm, $"{family}.CJHeight", element, "CJHeight");
        SetDoubleIfPresent(algorithm, $"{family}.BWCJ", element, "BWCJ");
        SetBoolIfPresent(algorithm, $"{family}.UseTotalArea3D", element, "UseTArea3D");
        SetDoubleIfPresent(algorithm, $"{family}.TotalArea3D", element, "TArea3D");
        SetDoubleIfPresent(algorithm, $"{family}.Gap", element, "GapW");
        SetBoolIfPresent(algorithm, $"{family}.UseExcept", element, "UseExc");
        SetBoolIfPresent(algorithm, $"{family}.UseContour", element, "UseCont");
        SetDoubleIfPresent(algorithm, $"{family}.IntervalContourX", element, "IntervalCont");
        SetDoubleIfPresent(algorithm, $"{family}.IntervalContourY", element, "IntervalContY");
        SetDoubleIfPresent(algorithm, $"{family}.LeadHeight", element, "HeiLead", "HLd");
        SetDoubleIfPresent(algorithm, $"{family}.LeadHeightMargin", element, "HLdMg");
        SetIntIfPresent(algorithm, $"{family}.HeightCalculationMethod", element, "HCalcMethod");

        if (TryReadNumberArrayLeaf(element, out var blackWhite, "BlackWhite", "BW"))
        {
            ApplyShapeXBlobBaseParameters(algorithm, family, "BlackWhite", blackWhite, promoteRuntimeKeys: true);
        }

        CopyRawArrayIfPresent(algorithm, element, family, "HeightMean", "HeightMean", "HM");
        CopyRawArrayIfPresent(algorithm, element, family, "Essential", "Ess");
        CopyRawArrayIfPresent(algorithm, element, family, "RoiContour", "ROICont");
        CopyRawArrayIfPresent(algorithm, element, family, "ThresholdContour", "T_Cont");
        SetBool(algorithm, "Import.LeadSolderMapped", true);
    }

    private static void ApplyLeadColorParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        ApplyColorParameters(algorithm, element);
        ApplyLeadPositionParameters(algorithm, element, family);
        SetDoubleIfPresent(algorithm, $"{family}.Gap", element, "SolderLength", "SD_Len", "SD_Len_mm");
        SetIntIfPresent(algorithm, $"{family}.LeadTipDirection", element, "LeadTipDirection", "LT_Dir");
        SetDoubleIfPresent(algorithm, $"{family}.CurrentPosition", element, "LeadPosition", "L_Pos", "L_Pos_mm");
        SetIntIfPresent(algorithm, $"{family}.ColorInspectionSetValue", element, "InspTypeColor", "InspTPClr");
        SetBoolIfPresent(algorithm, $"{family}.ColorJudge", element, "UseRange", "UseRangeBar");
        SetBool(algorithm, "Import.LeadColorMapped", true);
    }

    private static void ApplyLeadSearchParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        ApplyLeadPositionParameters(algorithm, element, family);
        ApplySizeArrayParameters(algorithm, element, family, "DisplayPanelSizeX", "DisplayPanelSizeY", "DPPansz");
        SetDoubleIfPresent(algorithm, $"{family}.SolderStartPos", element, "SolderStartPos", "SD_stPos");
        SetBoolIfPresent(algorithm, $"{family}.IsContainer", element, "IsContainer", "Ctn");
        SetBoolIfPresent(algorithm, $"{family}.Use3D", element, "Use3D");
        SetBoolIfPresent(algorithm, $"{family}.Use2D", element, "Use2D");
        SetIntIfPresent(algorithm, $"{family}.SolderThreshold", element, "ThresholdValue", "THV");
        SetBoolIfPresent(algorithm, $"{family}.UseHeight", element, "InspHeightFlag", "UseHei");
        SetBoolIfPresent(algorithm, $"{family}.UseCoplanarity", element, "InspCoplanrity", "UseCop");
        SetIntIfPresent(algorithm, $"{family}.RoiCount", element, "SolderCnt", "SDCnt");
        SetDoubleIfPresent(algorithm, $"{family}.RoiWidth", element, "LeadWidth", "L_W");
        SetDoubleIfPresent(algorithm, $"{family}.RoiPitch", element, "LeadPitch", "L_P");
        SetIntIfPresent(algorithm, $"{family}.StartLead", element, "StartLead");
        SetIntIfPresent(algorithm, $"{family}.EndLead", element, "EndLead");
        SetBoolIfPresent(algorithm, $"{family}.IsManualSearch", element, "IsManualSearch", "MSear");
        SetDoubleIfPresent(algorithm, $"{family}.LeadStartLeft", element, "LeadStartLeft", "L_stl");
        SetDoubleIfPresent(algorithm, $"{family}.LeadStartLeftDistanceX", element, "L_stlDstX");
        SetIntIfPresent(algorithm, $"{family}.ManualSearchType", element, "MSearType");
        SetBoolIfPresent(algorithm, $"{family}.SameWidthPitch", element, "SameWidthPitch", "SameWP");
        SetDoubleIfPresent(algorithm, $"{family}.RoiStartPos", element, "LeadPosition", "L_Pos", "L_Pos_mm");
        CopyRawArrayIfPresent(algorithm, element, family, "LeadStart", "L_st");
        CopyRawArrayIfPresent(algorithm, element, family, "LeadEnd", "L_ed");
        SetBool(algorithm, "Import.LeadSearchMapped", true);
    }

    private static void ApplyLeadSideSolderParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        ApplyLeadPositionParameters(algorithm, element, family);
        ApplySizeArrayParameters(algorithm, element, family, "Width", "Height", "RectWH");
        SetIntIfPresent(algorithm, $"{family}.LeadTipDirection", element, "LeadTipDirection", "LT_Dir");
        SetDoubleIfPresent(algorithm, $"{family}.CurrentPosition", element, "LeadPosition", "L_Pos", "L_Pos_mm");
        SetDoubleIfPresent(algorithm, $"{family}.LeadLiftSetValue", element, "LeadLiftSetValue", "LL_V");
        SetDoubleIfPresent(algorithm, $"{family}.SolderLength", element, "SolderLength", "SD_Len", "SD_Len_mm");
        SetDoubleIfPresent(algorithm, $"{family}.Gap", element, "SolderLength", "SD_Len", "SD_Len_mm");
        SetDoubleIfPresent(algorithm, $"{family}.GapWidth", element, "GapW");
        SetBoolIfPresent(algorithm, $"{family}.UseTeachingRate", element, "UseTeachA");
        SetDoubleIfPresent(algorithm, $"{family}.TeachingArea", element, "TeachA");
        SetDoubleIfPresent(algorithm, $"{family}.TeachingArea2", element, "TeachA2");
        SetIntIfPresent(algorithm, $"{family}.InspectionArea", element, "InspA");

        if (TryReadNumberArrayLeaf(element, out var blackWhite, "BlackWhite", "BW"))
        {
            ApplyShapeXBlobBaseParameters(algorithm, family, "BlackWhite", blackWhite, promoteRuntimeKeys: true);
        }

        SetBool(algorithm, "Import.LeadSideSolderMapped", true);
    }

    private static void ApplyWidthParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        ApplyCommonRangeParameters(algorithm, element);
        SetDoubleIfPresent(algorithm, $"{family}.StdWidth", element, "StdWidth", "StdW");
        SetDoubleIfPresent(algorithm, $"{family}.StdLength", element, "StdLength", "StdLen");
        SetBoolIfPresent(algorithm, $"{family}.UseWidth", element, "UseWidth", "UseW", "UseTCW");
        SetBoolIfPresent(algorithm, $"{family}.UseLength", element, "UseLength", "UseL", "UseTCL");
        SetDoubleIfPresent(algorithm, $"{family}.Width", element, "Width", "StdWidth", "StdW", "TCW");
        SetDoubleIfPresent(algorithm, $"{family}.Length", element, "Length", "StdLength", "StdLen", "TCL");

        if (TryReadNumberArrayLeaf(element, out var widthRange, "MnMxWidth", "MnMxW") && widthRange.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.WidthMin", widthRange[0]);
            SetDouble(algorithm, $"{family}.WidthMax", widthRange[1]);
        }

        if (TryReadNumberArrayLeaf(element, out var lengthRange, "MnMxLength", "MnMxLen", "MnMxL") && lengthRange.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.LengthMin", lengthRange[0]);
            SetDouble(algorithm, $"{family}.LengthMax", lengthRange[1]);
        }

        SetBool(algorithm, "Import.WidthMapped", true);
    }

    private static void ApplyVolumeParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        ApplyCommonRangeParameters(algorithm, element);
        SetIntIfPresent(algorithm, $"{family}.IpcClass", element, "ClassIPC", "CSIPC");
        SetIntIfPresent(algorithm, $"{family}.IPCClass", element, "ClassIPC", "CSIPC");
        SetDoubleIfPresent(algorithm, $"{family}.StandardBodyHeight", element, "StdBodyHeight", "StdBodyHei");
        SetDoubleIfPresent(algorithm, $"{family}.StdWidth", element, "StdWidth", "StdW");
        SetDoubleIfPresent(algorithm, $"{family}.StdLength", element, "StdLength", "StdLen");
        SetBoolIfPresent(algorithm, $"{family}.ChipTracking", element, "ChipTracking", "ChipTrack");
        SetBoolIfPresent(algorithm, $"{family}.UseBWOption", element, "UseBWOption", "UseBW");
        SetBoolIfPresent(algorithm, $"{family}.UseBW", element, "UseBWOption", "UseBW");
        SetBoolIfPresent(algorithm, $"{family}.UseStdMax", element, "UseStdMax");
        SetDoubleIfPresent(algorithm, $"{family}.LimitUpper", element, "LimitUpper", "LimitUp");
        SetIntIfPresent(algorithm, $"{family}.TypeSelectBlob", element, "TypeSelectBlob", "TPSelBlob");
        SetDoubleIfPresent(algorithm, $"{family}.StandardVolume", element, "StdVolume");
        SetDoubleIfPresent(algorithm, $"{family}.VolumeHeight", element, "StdVolume");
        SetDoubleIfPresent(algorithm, $"{family}.AddRefVolume", element, "ARefV");
        SetBoolIfPresent(algorithm, $"{family}.UseHeight", element, "UseHeight", "UseHei");
        SetBoolIfPresent(algorithm, $"{family}.UseVolume", element, "UseHeight", "UseHei");
        SetBoolIfPresent(algorithm, $"{family}.UseRelativeHeight", element, "HeightPer");
        SetDoubleIfPresent(algorithm, $"{family}.MinHeight", element, "MinHeight");
        SetDoubleIfPresent(algorithm, $"{family}.MaxHeight", element, "MaxHeight");
        SetDoubleIfPresent(algorithm, $"{family}.RelativeHeightMin", element, "MinHeight");
        SetDoubleIfPresent(algorithm, $"{family}.RelativeHeightMax", element, "MaxHeight");
        SetDoubleIfPresent(algorithm, $"{family}.StdArea", element, "StdArea");
        SetBoolIfPresent(algorithm, $"{family}.UseArea", element, "UseArea");
        SetBoolIfPresent(algorithm, $"{family}.UseAreaRate", element, "UseArea");
        SetBoolIfPresent(algorithm, $"{family}.UseColdJoint", element, "Use_CJ");
        SetDoubleIfPresent(algorithm, $"{family}.ColdJointArea", element, "Area_CJ");
        SetDoubleIfPresent(algorithm, $"{family}.ColdJointAreaPercent", element, "AreaP_CJ");
        SetBoolIfPresent(algorithm, $"{family}.UseInclination", element, "UseInc");
        SetDoubleIfPresent(algorithm, $"{family}.Inclination", element, "Inc");
        SetDoubleIfPresent(algorithm, $"{family}.InclinationMin", element, "IncMin");
        SetDoubleIfPresent(algorithm, $"{family}.InclinationMax", element, "IncMax");
        SetDoubleIfPresent(algorithm, $"{family}.RemovalHeight", element, "RH");
        SetBoolIfPresent(algorithm, $"{family}.BlackHeight", element, "BH");
        SetBoolIfPresent(algorithm, $"{family}.FillHole", element, "FH");
        SetIntIfPresent(algorithm, $"{family}.FillHoleSize", element, "FHS");
        SetBoolIfPresent(algorithm, $"{family}.UseFilter", element, "UseFlt", "UseFilter");
        SetIntIfPresent(algorithm, $"{family}.FilterStepNarrow", element, "FltStepNar");
        SetIntIfPresent(algorithm, $"{family}.ChipTrackingGap", element, "ChipTrackGap");

        if (TryReadNumberArrayLeaf(element, out var volumeHeightRange, "MnMxHei") && volumeHeightRange.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.VolumeMin", volumeHeightRange[0]);
            SetDouble(algorithm, $"{family}.VolumeMax", volumeHeightRange[1]);
            SetDouble(algorithm, $"{family}.MinHeight", volumeHeightRange[0]);
            SetDouble(algorithm, $"{family}.MaxHeight", volumeHeightRange[1]);
            SetDouble(algorithm, $"{family}.RelativeHeightMin", volumeHeightRange[0]);
            SetDouble(algorithm, $"{family}.RelativeHeightMax", volumeHeightRange[1]);
        }

        if (TryReadNumberArrayLeaf(element, out var areaRange, "MnMxArea") && areaRange.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.AreaMin", areaRange[0]);
            SetDouble(algorithm, $"{family}.AreaMax", areaRange[1]);
            SetDouble(algorithm, $"{family}.AreaRateMin", areaRange[0]);
            SetDouble(algorithm, $"{family}.AreaRateMax", areaRange[1]);
        }

        CopyRawArrayIfPresent(algorithm, element, family, "InclinationROI", "IncROI");
        CopyRawArrayIfPresent(algorithm, element, family, "RemovalHeightROI", "RHROI");

        if (TryReadNumberArrayLeaf(element, out var arrN, "ArrN"))
        {
            SetInt(algorithm, $"{family}.RawNDataCount", arrN.Length);
            SetIndexedDoubles(algorithm, family, "ArrN", arrN);
        }

        if (TryReadNumberArrayLeaf(element, out var arrF, "ArrF"))
        {
            SetInt(algorithm, $"{family}.RawFDataCount", arrF.Length);
            SetIndexedDoubles(algorithm, family, "ArrF", arrF);
        }

        SetBool(algorithm, "Import.VolumeMapped", true);
    }

    private static void ApplyFilletParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        ApplyCommonRangeParameters(algorithm, element);
        SetBoolIfPresent(algorithm, $"{family}.UseSolderFillet", element, "UseFilletInsp");
        SetBoolIfPresent(algorithm, $"{family}.UseFilletInsp", element, "UseFilletInsp");
        SetBoolIfPresent(algorithm, $"{family}.AngleRangeH", element, "UARH");
        SetBoolIfPresent(algorithm, $"{family}.ChipTracking", element, "ChipTracking", "ChipTrack");
        SetDoubleIfPresent(algorithm, $"{family}.FilletGap", element, "Gap");
        SetDoubleIfPresent(algorithm, $"{family}.FilletInterval", element, "Interval");
        SetIntIfPresent(algorithm, $"{family}.FilletDivCount", element, "DivCnt");
        SetIntIfPresent(algorithm, $"{family}.TipDirection", element, "TipDir");
        SetDoubleIfPresent(algorithm, $"{family}.MinAngleR", element, "MnA");
        SetDoubleIfPresent(algorithm, $"{family}.MaxAngleR", element, "MxA");
        SetDoubleIfPresent(algorithm, $"{family}.MinAngleSpec", element, "MnAS");
        SetDoubleIfPresent(algorithm, $"{family}.MaxAngleSpec", element, "MxAS");
        SetBoolIfPresent(algorithm, $"{family}.Direct", element, "UDir");
        SetBoolIfPresent(algorithm, $"{family}.UseLeadTipPosition", element, "ULTP");
        SetDoubleIfPresent(algorithm, $"{family}.LeadTipPosition", element, "LTP");

        CopyPrefixedLeafValues(algorithm, element, family, "HeightOption", "HgtOptVal");
        CopyPrefixedLeafValues(algorithm, element, family, "AngleOption", "AngOptVal");
        CopyPrefixedLeafValues(algorithm, element, family, "AngleRangeEnable", "ARHEnb");
        CopyPrefixedLeafValues(algorithm, element, family, "HeightOptionUse", "HOpt");
        CopyPrefixedLeafValues(algorithm, element, family, "AngleOptionUse", "AOpt");
        CopyContainerLeafValues(algorithm, element, family, "HeightOption", "HgtOptVal");
        CopyContainerLeafValues(algorithm, element, family, "AngleOption", "AngOptVal");
        CopyContainerLeafValues(algorithm, element, family, "AngleRangeEnable", "ARHEnb");
        CopyContainerLeafValues(algorithm, element, family, "HeightOptionUse", "HOpt");
        CopyContainerLeafValues(algorithm, element, family, "AngleOptionUse", "AOpt");

        SetBool(algorithm, "Import.FilletMapped", true);
    }

    private static void ApplyFootParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        ApplyCommonRangeParameters(algorithm, element);
        if (TryReadNumberArrayLeaf(element, out var useOptions, "UOpt"))
        {
            SetOptionalDouble(algorithm, $"{family}.UseOption", useOptions, 0);
            SetOptionalDouble(algorithm, $"{family}.UseOption2", useOptions, 1);
        }

        if (TryReadNumberArrayLeaf(element, out var refArea, "ROIF") && refArea.Length >= 4)
        {
            SetDouble(algorithm, $"{family}.FootRefLeft", refArea[0]);
            SetDouble(algorithm, $"{family}.FootRefRight", refArea[1]);
            SetDouble(algorithm, $"{family}.FootRefTop", refArea[2]);
            SetDouble(algorithm, $"{family}.FootRefBottom", refArea[3]);
        }

        if (TryReadNumberArrayLeaf(element, out var specRect, "SrchF", "SpcR") && specRect.Length >= 4)
        {
            SetDouble(algorithm, $"{family}.SpecLeft", specRect[0]);
            SetDouble(algorithm, $"{family}.SpecRight", specRect[1]);
            SetDouble(algorithm, $"{family}.SpecTop", specRect[2]);
            SetDouble(algorithm, $"{family}.SpecBottom", specRect[3]);
        }

        SetBoolIfPresent(algorithm, $"{family}.WindowOffset", element, "WndOffsetF");
        SetIntIfPresent(algorithm, $"{family}.FindOption", element, "FOpt");
        SetIntIfPresent(algorithm, $"{family}.FindOption2", element, "FOpt2");
        SetBoolIfPresent(algorithm, $"{family}.UsePatternAngle", element, "UPatAgl");
        SetBoolIfPresent(algorithm, $"{family}.Use2Foot", element, "U2Foot");
        SetBoolIfPresent(algorithm, $"{family}.UsePadAreaAutoTeach", element, "UPadAutoT");
        SetIntIfPresent(algorithm, $"{family}.FootType", element, "FType");
        SetIntIfPresent(algorithm, $"{family}.FootDirection", element, "FDir");
        SetIntIfPresent(algorithm, $"{family}.TipDirection", element, "FDir");
        SetIntIfPresent(algorithm, $"{family}.TeachFootDirection", element, "TchFDir");
        SetBoolIfPresent(algorithm, $"{family}.UseRemovedPadArea", element, "URmvPA");
        SetBoolIfPresent(algorithm, $"{family}.Use2DImageForInspWidth", element, "U2DImgFIW");
        SetIntIfPresent(algorithm, $"{family}.RemoveWireHeight", element, "RmvWireH");
        SetIntIfPresent(algorithm, $"{family}.InspCrackHeight", element, "CrkH");
        SetDoubleIfPresent(algorithm, $"{family}.WidthSubOffset", element, "WSOffset");
        SetDoubleIfPresent(algorithm, $"{family}.LengthSubOffset", element, "LSOffset");
        SetDoubleIfPresent(algorithm, $"{family}.HeightSubOffset", element, "HSOffset");
        SetIntIfPresent(algorithm, $"{family}.FootSobelMin", element, "FSbMn");
        SetIntIfPresent(algorithm, $"{family}.FootSobelMax", element, "FSbMx");
        SetIntIfPresent(algorithm, $"{family}.WireSpecDiameter", element, "FSPDIA");
        SetIntIfPresent(algorithm, $"{family}.FootPadSizeX", element, "PadX");
        SetIntIfPresent(algorithm, $"{family}.FootPadSizeY", element, "PadY");
        SetIntIfPresent(algorithm, $"{family}.PadEdgeMinRatio", element, "PEMR");
        SetBoolIfPresent(algorithm, $"{family}.UseDBCShapeTeaching", element, "UST");
        SetBoolIfPresent(algorithm, $"{family}.UseTeachRect", element, "UTchPRe");
        SetIntIfPresent(algorithm, $"{family}.PadWidth", element, "PadW");
        SetIntIfPresent(algorithm, $"{family}.PadHeight", element, "PadH");
        SetIntIfPresent(algorithm, $"{family}.Radius", element, "Rdi");

        CopyPrefixedLeafValues(algorithm, element, family, "OptionValue", "OptVal");
        CopyPrefixedLeafValues(algorithm, element, family, "FindOptionOrder", "FdOptOdr");
        CopyPrefixedLeafValues(algorithm, element, family, "FindOptionOrder2", "FdOptOdr2");
        CopyPrefixedLeafValues(algorithm, element, family, "Bin", "SetData_", "SetData2_", "LV_", "H_", "LtData_", "Set_", "Mop_");
        CopyContainerLeafValues(algorithm, element, family, "Point", "Pnt");
        CopyContainerLeafValues(algorithm, element, family, "OptionValue", "OptVal");
        CopyContainerLeafValues(algorithm, element, family, "FindOptionOrder", "FdOptOdr");
        CopyContainerLeafValues(algorithm, element, family, "FindOptionOrder2", "FdOptOdr2");
        CopyContainerLeafValues(algorithm, element, family, "Bin", "ArBin");

        SetBool(algorithm, "Import.FootMapped", true);
    }

    private static void ApplySolderConeParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        SetDoubleIfPresent(algorithm, $"{family}.TeachHeight", element, "TeachHeight", "TCHei");
        SetDoubleIfPresent(algorithm, $"{family}.Height1Step", element, "HeightLv1");
        SetDoubleIfPresent(algorithm, $"{family}.Height2Step", element, "HeightLv2");
        SetDoubleIfPresent(algorithm, $"{family}.Height3Step", element, "HeightLv3");
        SetBoolIfPresent(algorithm, $"{family}.UseSolderHighest", element, "UseSolderHighest");
        SetBoolIfPresent(algorithm, $"{family}.UseSolderHighestVolumeDiff", element, "UHstVD");
        SetBoolIfPresent(algorithm, $"{family}.UseInspVolumeDiff", element, "UIVD");
        SetBoolIfPresent(algorithm, $"{family}.UseInspVolumeMinLength", element, "UIVML");
        SetBoolIfPresent(algorithm, $"{family}.UseInspVolumePie", element, "UIVP");
        SetDoubleIfPresent(algorithm, $"{family}.HighestHeight", element, "HestH");
        SetDoubleIfPresent(algorithm, $"{family}.CurrentHeight", element, "HestH");
        SetDoubleIfPresent(algorithm, $"{family}.GapHeight", element, "GapH");
        SetIntIfPresent(algorithm, $"{family}.LevelCount", element, "LvlCnt");
        SetDoubleIfPresent(algorithm, $"{family}.PieHeightDiff", element, "PieHD");

        if (TryReadNumberArrayLeaf(element, out var heights, "Hei") && heights.Length >= 3)
        {
            SetDouble(algorithm, $"{family}.Height1Step", heights[0]);
            SetDouble(algorithm, $"{family}.Height2Step", heights[1]);
            SetDouble(algorithm, $"{family}.Height3Step", heights[2]);
            SetBool(algorithm, $"{family}.Use1Step", true);
            SetBool(algorithm, $"{family}.Use2Step", true);
            SetBool(algorithm, $"{family}.Use3Step", true);
        }
        else
        {
            SetBoolIfPresent(algorithm, $"{family}.Use1Step", element, "HeightLv1");
            SetBoolIfPresent(algorithm, $"{family}.Use2Step", element, "HeightLv2");
            SetBoolIfPresent(algorithm, $"{family}.Use3Step", element, "HeightLv3");
        }

        CopyPrefixedLeafValues(algorithm, element, family, "VolumeDiffOption", "DifOpt");
        CopyPrefixedLeafValues(algorithm, element, family, "VolumeDiffEnable", "DifEnb");
        CopyContainerLeafValues(algorithm, element, family, "VolumeDiffOption", "DifOpt");
        CopyContainerLeafValues(algorithm, element, family, "VolumeDiffEnable", "DifEnb");

        SetBool(algorithm, "Import.SolderConeMapped", true);
    }

    private static void ApplyGridParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;
        ApplyCommonRangeParameters(algorithm, element);

        SetIntIfPresent(algorithm, $"{family}.Column", element, "Column", "Col");
        SetIntIfPresent(algorithm, $"{family}.Row", element, "Row");
        SetIntIfPresent(algorithm, $"{family}.IgnoreColor", element, "IgnoreColor", "IgnClr");
        SetDoubleIfPresent(algorithm, $"{family}.StdDev", element, "StdDev");

        if (TryReadDoubleLeaf(element, out var detectWidth, "DetectWidth", "DetW"))
        {
            SetDouble(algorithm, $"{family}.DetectWidth", detectWidth);
            SetDouble(algorithm, $"{family}.PixelWidth", detectWidth);
        }

        if (TryReadDoubleLeaf(element, out var detectLength, "DetectLength", "DetLen"))
        {
            SetDouble(algorithm, $"{family}.DetectLength", detectLength);
            SetDouble(algorithm, $"{family}.PixelLength", detectLength);
        }

        if (TryReadBoolLeaf(element, out var ignoreUse, "IgnoreUse", "UseIgn"))
        {
            SetBool(algorithm, $"{family}.IgnoreUse", ignoreUse);
            SetBool(algorithm, $"{family}.ExceptInclude", ignoreUse);
        }

        if (TryReadBoolLeaf(element, out var stdUse, "StdUse", "UseStd"))
        {
            SetBool(algorithm, $"{family}.StdUse", stdUse);
            SetBool(algorithm, $"{family}.UseStandard", stdUse);
        }

        if (TryReadLeafValue(element, out var exceptArea, "EA"))
        {
            algorithm.Parameters[$"{family}.ExceptArea.Raw"] = exceptArea;
            SetInt(algorithm, $"{family}.ExceptAreaCount", CountDelimitedTokens(exceptArea));
        }

        CopyRawArrayIfPresent(algorithm, element, family, "Color", "Color", "Clr");
        SetBoolIfPresent(algorithm, $"{family}.UseImageMix", element, "UseImageMix");
        SetIntIfPresent(algorithm, $"{family}.ImageMixCount", element, "ImageMixCount");
        SetBool(algorithm, "Import.GridMapped", true);
    }

    private static void ApplyLineParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        ApplyLineEdgeCommonParameters(algorithm, element);
        SetBool(algorithm, "Import.LineMapped", true);
    }

    private static void ApplyEdgeParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;
        ApplyLineEdgeCommonParameters(algorithm, element);

        SetIntIfPresent(algorithm, $"{family}.SetLineCount", element, "SetLineCnt");
        SetBoolIfPresent(algorithm, $"{family}.Group", element, "Group", "Grp");
        SetIntIfPresent(algorithm, $"{family}.LineFindType", element, "LineFindType");
        SetDoubleIfPresent(algorithm, $"{family}.LineFindRate", element, "LineFindRate");
        SetIntIfPresent(algorithm, $"{family}.InspectionOption", element, "InspOption");
        SetBoolIfPresent(algorithm, $"{family}.FindCenter", element, "FC");
        SetBoolIfPresent(algorithm, $"{family}.UseFillHole", element, "FH");
        SetBoolIfPresent(algorithm, $"{family}.FillHole", element, "FH");

        if (TryReadNumberArrayLeaf(element, out var setInspCond, "SetInspCond"))
        {
            SetIndexedDoubles(algorithm, family, "SetInspCondition", setInspCond);
        }
        else
        {
            SetIntIfPresent(algorithm, $"{family}.SetInspCondition1", element, "ArrSetInspCondition1");
            SetIntIfPresent(algorithm, $"{family}.SetInspCondition2", element, "ArrSetInspCondition2");
            SetIntIfPresent(algorithm, $"{family}.SetInspCondition3", element, "ArrSetInspCondition3");
        }

        for (var index = 1; index <= 4; index++)
        {
            ApplyEdgeLineSlotParameters(algorithm, element, family, index);
        }

        if (TryReadNumberArrayLeaf(element, out var distanceX, "DisX") && distanceX.Length >= 2)
        {
            SetBool(algorithm, $"{family}.UseDistanceX", Math.Abs(distanceX[0]) > double.Epsilon);
            SetDouble(algorithm, $"{family}.DistanceX", distanceX[1]);
        }

        if (TryReadNumberArrayLeaf(element, out var distanceXRange, "DisXMnMx") && distanceXRange.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.DistanceXMin", distanceXRange[0]);
            SetDouble(algorithm, $"{family}.DistanceXMax", distanceXRange[1]);
        }

        if (TryReadNumberArrayLeaf(element, out var distanceY, "DisY") && distanceY.Length >= 2)
        {
            SetBool(algorithm, $"{family}.UseDistanceY", Math.Abs(distanceY[0]) > double.Epsilon);
            SetDouble(algorithm, $"{family}.DistanceY", distanceY[1]);
        }

        if (TryReadNumberArrayLeaf(element, out var distanceYRange, "DisYMnMx") && distanceYRange.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.DistanceYMin", distanceYRange[0]);
            SetDouble(algorithm, $"{family}.DistanceYMax", distanceYRange[1]);
        }

        if (TryReadNumberArrayLeaf(element, out var crossCenter, "CC"))
        {
            if (crossCenter.Length > 0)
            {
                SetBool(algorithm, $"{family}.UseCrossCenter", Math.Abs(crossCenter[0]) > double.Epsilon);
            }

            if (crossCenter.Length > 1)
            {
                SetDouble(algorithm, $"{family}.StdTeachRotate", crossCenter[1]);
            }
        }

        SetBool(algorithm, "Import.EdgeMapped", true);
    }

    private static void ApplyLineEdgeCommonParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;
        ApplyCommonRangeParameters(algorithm, element);

        if (TryReadBoolLeaf(element, out var invert, "Invert", "Inv"))
        {
            SetBool(algorithm, $"{family}.Invert", invert);
            SetBool(algorithm, $"{family}.InvertCheck", invert);
        }

        SetBoolIfPresent(algorithm, $"{family}.UseFilter", element, "FilterIsUse", "UseFlt");
        SetIntIfPresent(algorithm, $"{family}.FilterStepNarrow", element, "FilterStepNarrow", "FltStepNar");
        SetBoolIfPresent(algorithm, $"{family}.UseShift", element, "ShiftIsUse", "UseSft");
        SetBoolIfPresent(algorithm, $"{family}.UseArea", element, "AreaIsUse", "UseArea");
        SetDoubleIfPresent(algorithm, $"{family}.AreaCurrent", element, "AreaCurrent", "AreaCur");
        SetIntIfPresent(algorithm, $"{family}.TypeSelectBlob", element, "TypeSelectBlob", "TPSelBlob");
        SetBoolIfPresent(algorithm, $"{family}.Use2D", element, "Insp2D", "Use2D");
        SetBoolIfPresent(algorithm, $"{family}.Use3D", element, "Insp3D", "Use3D");
        SetIntIfPresent(algorithm, $"{family}.Range2DType", element, "TypeRange2D", "TPR2D");
        SetIntIfPresent(algorithm, $"{family}.Range3DType", element, "TypeRange3D", "TPR3D");

        if (TryReadNumberArrayLeaf(element, out var teachCenter, "TechC") && teachCenter.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.TeachCenterX", teachCenter[0]);
            SetDouble(algorithm, $"{family}.TeachCenterY", teachCenter[1]);
            SetDouble(algorithm, $"{family}.CenterX", teachCenter[0]);
            SetDouble(algorithm, $"{family}.CenterY", teachCenter[1]);
        }
        else
        {
            SetDoubleIfPresent(algorithm, $"{family}.TeachCenterX", element, "TechCenterX");
            SetDoubleIfPresent(algorithm, $"{family}.TeachCenterY", element, "TechCenterY");
        }

        SetBoolIfPresent(algorithm, $"{family}.UseIPC", element, "UseIPC");
        SetIntIfPresent(algorithm, $"{family}.IpcClass", element, "ClassIPC", "CSIPC");
        SetIntIfPresent(algorithm, $"{family}.TypeStdIPC", element, "TypeStdIPC", "TPStdIPC");
        SetBoolIfPresent(algorithm, $"{family}.TeachWidthUse", element, "TeachWidthUse", "TCWUse");
        SetDoubleIfPresent(algorithm, $"{family}.TeachWidth", element, "TeachWidth", "TCW");
        SetBoolIfPresent(algorithm, $"{family}.TeachLengthUse", element, "TeachLengthUse", "TCLenUse");
        SetDoubleIfPresent(algorithm, $"{family}.TeachLength", element, "TeachLength", "TCLen");

        if (TryReadNumberArrayLeaf(element, out var teachWidthRate, "MnMxTCWR") && teachWidthRate.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.TeachWidthRateMin", teachWidthRate[0]);
            SetDouble(algorithm, $"{family}.TeachWidthRateMax", teachWidthRate[1]);
        }

        if (TryReadNumberArrayLeaf(element, out var teachLengthRate, "MnMxTCLen") && teachLengthRate.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.TeachLengthRateMin", teachLengthRate[0]);
            SetDouble(algorithm, $"{family}.TeachLengthRateMax", teachLengthRate[1]);
        }

        SetBoolIfPresent(algorithm, $"{family}.ShiftXUse", element, "ShiftXUse", "UseSftX");
        SetBoolIfPresent(algorithm, $"{family}.ShiftYUse", element, "ShiftYUse", "UseSftY");
        SetBoolIfPresent(algorithm, $"{family}.IsHorizon", element, "IsHorizon");
        SetIntIfPresent(algorithm, $"{family}.MeasureDirection", element, "MeasureDirection", "MeasureDir");
        SetBoolIfPresent(algorithm, $"{family}.UseAngle", element, "UseAngle", "UseAng");
        SetDoubleIfPresent(algorithm, $"{family}.TeachRotate", element, "TeachRotate", "TCRot");
        SetDoubleIfPresent(algorithm, $"{family}.TeachAngle", element, "TeachRotate", "TCRot");
        SetBoolIfPresent(algorithm, $"{family}.UseCross", element, "Cross");
        SetBoolIfPresent(algorithm, $"{family}.UseFix", element, "C_Fix");
        SetIntIfPresent(algorithm, $"{family}.CrossOption", element, "C_Opt");
        SetBoolIfPresent(algorithm, $"{family}.UseFillHole", element, "FH");
        SetBoolIfPresent(algorithm, $"{family}.FillHole", element, "FH");
        SetBoolIfPresent(algorithm, $"{family}.UseEndPosition", element, "EP");
        SetBoolIfPresent(algorithm, $"{family}.UseROI", element, "UseROI");

        if (TryReadNumberArrayLeaf(element, out var lineFindOption, "LFO"))
        {
            SetIndexedDoubles(algorithm, family, "LineFindType", lineFindOption);
        }

        if (TryReadNumberArrayLeaf(element, out var data, "Data"))
        {
            algorithm.Parameters[$"{family}.Data.Raw"] = string.Join(",", data.Select(value => value.ToString("0.########", CultureInfo.InvariantCulture)));
            if (data.Length > 0)
            {
                SetInt(algorithm, $"{family}.LineData", Round(data[0]));
            }

            for (var index = 1; index < Math.Min(data.Length, 6); index++)
            {
                SetDouble(algorithm, $"{family}.Perpendicular{index}", data[index]);
            }
        }

        if (TryReadNumberArrayLeaf(element, out var roi, "ROI") && roi.Length >= 4)
        {
            SetDouble(algorithm, $"{family}.RoiLeft", roi[0]);
            SetDouble(algorithm, $"{family}.RoiRight", roi[1]);
            SetDouble(algorithm, $"{family}.RoiTop", roi[2]);
            SetDouble(algorithm, $"{family}.RoiBottom", roi[3]);
        }
    }

    private static void ApplyEdgeLineSlotParameters(InspectionAlgorithmData algorithm, XElement element, string family, int index)
    {
        if (TryReadNumberArrayLeaf(element, out var setLine, $"SetLine{index}") && setLine.Length >= 5)
        {
            SetBool(algorithm, $"{family}.Line{index}.IsHorizon", Math.Abs(setLine[0]) > double.Epsilon);
            SetInt(algorithm, $"{family}.Line{index}.MeasureDirection", Round(setLine[1]));
            SetDouble(algorithm, $"{family}.Line{index}.TeachLength", setLine[2]);
            SetDouble(algorithm, $"{family}.Line{index}.TeachCenterX", setLine[3]);
            SetDouble(algorithm, $"{family}.Line{index}.TeachCenterY", setLine[4]);
            return;
        }

        SetBoolIfPresent(algorithm, $"{family}.Line{index}.IsHorizon", element, $"ArrIsHorizon{index}");
        SetIntIfPresent(algorithm, $"{family}.Line{index}.MeasureDirection", element, $"ArrMeasureDirection{index}");
        SetDoubleIfPresent(algorithm, $"{family}.Line{index}.TeachLength", element, $"ArrTeachLength{index}");

        if (TryReadNumberArrayLeaf(element, out var teachCenter, $"ArrSetTeachCenter{index}") && teachCenter.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.Line{index}.TeachCenterX", teachCenter[0]);
            SetDouble(algorithm, $"{family}.Line{index}.TeachCenterY", teachCenter[1]);
        }
    }

    private static void ApplyBarcodeParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        if (TryReadNumberArrayLeaf(element, out var barData, "Bar_N"))
        {
            SetIndexedDoubles(algorithm, family, "BarData", barData);
            SetInt(algorithm, $"{family}.BarDataCount", barData.Length);
            SetInt(algorithm, $"{family}.BarcodeTypeFlags", ReadArrayInt(barData, 0));
            SetInt(algorithm, $"{family}.OptionFlags", ReadArrayInt(barData, 1));
            SetInt(algorithm, $"{family}.Retry", ReadArrayInt(barData, 2));
            SetInt(algorithm, $"{family}.EssentialPosition", ReadArrayInt(barData, 3));
            SetInt(algorithm, $"{family}.PartialFirst", ReadArrayInt(barData, 4));
            SetInt(algorithm, $"{family}.PartialSecond", ReadArrayInt(barData, 5));
            SetInt(algorithm, $"{family}.CharCountMin", ReadArrayInt(barData, 6));
            SetInt(algorithm, $"{family}.CharCountMax", ReadArrayInt(barData, 7));
            SetBool(algorithm, $"{family}.UseAngle", ReadArrayInt(barData, 8) != 0);
            SetInt(algorithm, $"{family}.Angle", ReadArrayInt(barData, 9));
            SetInt(algorithm, $"{family}.AngleTolerance", ReadArrayInt(barData, 10));
            SetInt(algorithm, $"{family}.KeyName", ReadArrayInt(barData, 11));
            SetInt(algorithm, $"{family}.KeyName2", ReadArrayInt(barData, 12));
            SetInt(algorithm, $"{family}.Quality1DOffsetX", ReadArrayInt(barData, 13));
            SetInt(algorithm, $"{family}.Quality1DOffsetY", ReadArrayInt(barData, 14));
            SetInt(algorithm, $"{family}.Quality1DTolerance", ReadArrayInt(barData, 15));
            SetInt(algorithm, $"{family}.Quality2DGrade", ReadArrayInt(barData, 16));
            SetInt(algorithm, $"{family}.Contrast2D", ReadArrayInt(barData, 17));
            SetInt(algorithm, $"{family}.DefectCount2D", ReadArrayInt(barData, 18));
            SetInt(algorithm, $"{family}.AlterAction", ReadArrayInt(barData, 19));

            var flags1 = ReadArrayInt(barData, 0);
            var flags2 = ReadArrayInt(barData, 1);
            SetBool(algorithm, $"{family}.Flip", HasFlag(flags1, 0x400000) || HasFlag(flags1, 0x800000));
            SetBool(algorithm, $"{family}.UseEssentialWords", HasFlag(flags2, 0x01));
            SetBool(algorithm, $"{family}.UseStandardWords", HasFlag(flags2, 0x02));
            SetBool(algorithm, $"{family}.UsePartialDisplay", HasFlag(flags2, 0x04));
            SetBool(algorithm, $"{family}.QualityCheck", HasFlag(flags2, 0x200));
            SetBool(algorithm, $"{family}.UseRecognitionWord", HasFlag(flags2, 0x10000));
            SetBool(algorithm, $"{family}.UseTargetWord", HasFlag(flags2, 0x20000));
            SetBool(algorithm, $"{family}.PNValidation", HasFlag(flags2, 0x40000));
            SetBool(algorithm, $"{family}.SplitImageSave", HasFlag(flags2, 0x80000));
        }

        if (TryReadLeafValue(element, out var barString, "Bar_S"))
        {
            var values = barString.Split(',').Select(value => value.Trim()).ToArray();
            SetInt(algorithm, $"{family}.BarStringCount", CountDelimitedTokens(barString));
            SetStringIfAvailable(algorithm, $"{family}.EssentialWords", values, 0);
            SetStringIfAvailable(algorithm, $"{family}.StandardWords", values, 1);
            SetStringIfAvailable(algorithm, $"{family}.MixWords", values, 2);
            SetStringIfAvailable(algorithm, $"{family}.SourceFilter", values, 4);
            SetStringIfAvailable(algorithm, $"{family}.TargetFilter", values, 5);
            SetStringIfAvailable(algorithm, $"{family}.SourceFilter2", values, 6);
            SetStringIfAvailable(algorithm, $"{family}.TargetFilter2", values, 7);
            SetStringIfAvailable(algorithm, $"{family}.CrossTarget", values, 8);
            SetStringIfAvailable(algorithm, $"{family}.CrossTarget2", values, 9);
            SetStringIfAvailable(algorithm, $"{family}.DefectConditionWidth", values, 10);
            SetStringIfAvailable(algorithm, $"{family}.DefectConditionLength", values, 11);
            SetStringIfAvailable(algorithm, $"{family}.DefectConditionCount", values, 12);
            SetStringIfAvailable(algorithm, $"{family}.DefectArea2D", values, 13);
            SetStringIfAvailable(algorithm, $"{family}.ModuleRecognizeTarget", values, 14);
            SetStringIfAvailable(algorithm, $"{family}.ModuleCharLength", values, 15);
        }

        if (TryReadNumberArrayLeaf(element, out var alignData, "Bar_A"))
        {
            SetBool(algorithm, $"{family}.UseAlign", alignData.Length > 0 && Math.Abs(alignData[0]) > double.Epsilon);
            if (alignData.Length > 1)
            {
                SetDouble(algorithm, $"{family}.TeachCenterX", alignData[1]);
            }

            if (alignData.Length > 2)
            {
                SetDouble(algorithm, $"{family}.TeachCenterY", alignData[2]);
            }

            if (alignData.Length > 3)
            {
                SetDouble(algorithm, $"{family}.TeachAngle", alignData[3]);
            }

            if (alignData.Length > 4)
            {
                SetBool(algorithm, $"{family}.QualityCheck", Math.Abs(alignData[4]) > double.Epsilon);
            }
        }

        SetBool(algorithm, "Import.BarcodeMapped", true);
    }

    private static void ApplyPatternDiffParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        SetIntIfPresent(algorithm, $"{family}.LayerCount", element, "LayerCnt");
        SetIntIfPresent(algorithm, $"{family}.ModelAddCount", element, "ModelAddCnt");
        SetBoolIfPresent(algorithm, $"{family}.UsePattern", element, "UsePattern");
        SetStringIfPresent(algorithm, $"{family}.ModelPath", element, "ModelPath");
        SetDoubleIfPresent(algorithm, $"{family}.Theta", element, "Theta");
        SetBoolIfPresent(algorithm, $"{family}.UseDarkForeign", element, "UseDarkForeign");
        SetBoolIfPresent(algorithm, $"{family}.UseExceptPattern", element, "UseExcPattern");
        SetIntIfPresent(algorithm, $"{family}.ExceptModelAddCount", element, "ExcModelAddCnt");
        SetIntIfPresent(algorithm, $"{family}.ExceptPatternScore", element, "ExcPatternScore");
        SetBoolIfPresent(algorithm, $"{family}.UseAlignArea", element, "UseArrAlignArea");
        SetIntIfPresent(algorithm, $"{family}.AlignAcceptScore", element, "AlignAcceptScore");
        SetBoolIfPresent(algorithm, $"{family}.UseAlignMatching", element, "bMth");
        SetBoolIfPresent(algorithm, $"{family}.UseMaskFilter", element, "UseF_Mask");
        SetIntIfPresent(algorithm, $"{family}.MaskFilterSize", element, "FS_Mask");

        ApplyPatternArray(algorithm, element, family, "LayerType", "LayerType_");
        ApplyPatternArray(algorithm, element, family, "PolygonAreaCount", "PolyAreaCnt_");
        ApplyPatternArray(algorithm, element, family, "PolygonPointCount", "PolyPtrCnt_");
        ApplyPatternArray(algorithm, element, family, "PolygonDirection", "PolyDir_");
        ApplyPatternArray(algorithm, element, family, "PolygonPointX", "PolyPtrX_");
        ApplyPatternArray(algorithm, element, family, "PolygonPointY", "PolyPtrY_");
        ApplyPatternArray(algorithm, element, family, "BWData", "BWData_");
        ApplyPatternArray(algorithm, element, family, "TeachLength", "TeachLength");
        ApplyPatternArray(algorithm, element, family, "TeachWidth", "TeachWidth");
        ApplyPatternArray(algorithm, element, family, "TeachArea", "TeachArea");
        ApplyPatternArray(algorithm, element, family, "Filter", "Filter");
        ApplyPatternArray(algorithm, element, family, "MatchingArea", "MatchingArea");
        ApplyPatternArray(algorithm, element, family, "NGBlobMode", "NGBlobMode");
        ApplyPatternArray(algorithm, element, family, "Histogram", "HistoData");
        ApplyPatternArray(algorithm, element, family, "SizeAnd", "SizeAnd");
        ApplyPatternArray(algorithm, element, family, "LightData", "LightData");
        ApplyPatternArray(algorithm, element, family, "MergeLayer", "MergeLayer");
        ApplyPatternArray(algorithm, element, family, "EdgeFilter", "EdgeFilter");
        ApplyPatternArray(algorithm, element, family, "FilterSize", "FilterSize");
        ApplyPatternArray(algorithm, element, family, "EdgeFilterLevel", "EdgeFilterLevel");
        ApplyPatternArray(algorithm, element, family, "AlignArea", "ArrAlignArea");
        ApplyPatternArray(algorithm, element, family, "UseMaskBW", "UseMaskBW");
        ApplyPatternArray(algorithm, element, family, "MaskBWData", "BWData_Mask");

        if (TryReadLeafValue(element, out var modelNames, "ArrModelName"))
        {
            SetInt(algorithm, $"{family}.ModelNameCount", CountDelimitedTokens(modelNames));
            algorithm.Parameters[$"{family}.ModelName.Raw"] = modelNames;
        }

        if (TryReadLeafValue(element, out var exceptModelNames, "ArrExcModelName"))
        {
            SetInt(algorithm, $"{family}.ExceptModelNameCount", CountDelimitedTokens(exceptModelNames));
            algorithm.Parameters[$"{family}.ExceptModelName.Raw"] = exceptModelNames;
        }

        SetBool(algorithm, "Import.PatternDiffMapped", true);
    }

    private static void ApplyPatternArray(InspectionAlgorithmData algorithm, XElement element, string family, string targetName, string legacyName)
    {
        if (TryReadNumberArrayLeaf(element, out var values, legacyName))
        {
            SetInt(algorithm, $"{family}.{targetName}Count", values.Length);
            SetIndexedDoubles(algorithm, family, targetName, values);
            return;
        }

        CopyRawArrayIfPresent(algorithm, element, family, targetName, legacyName);
    }

    private static void ApplyPadArrayParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        if (TryReadNumberArrayLeaf(element, out var nData, "NData"))
        {
            SetIndexedDoubles(algorithm, family, "NData", nData);
            SetInt(algorithm, $"{family}.NDataCount", nData.Length);
            SetInt(algorithm, $"{family}.DataFlags", ReadArrayInt(nData, 0));
            SetInt(algorithm, $"{family}.FilterIndex", ReadArrayInt(nData, 1));
            SetBinaryRange(algorithm, family, ReadArrayInt(nData, 2), ReadArrayInt(nData, 3, 255));
            SetInt(algorithm, $"{family}.Range2DType", ReadArrayInt(nData, 4));
            SetInt(algorithm, $"{family}.Range3DType", ReadArrayInt(nData, 5));
            SetInt(algorithm, $"{family}.Row", ReadArrayInt(nData, 6, 1));
            SetInt(algorithm, $"{family}.Column", ReadArrayInt(nData, 7, 1));

            var flags = ReadArrayInt(nData, 0);
            SetBool(algorithm, $"{family}.Use2D", HasFlag(flags, 1 << 0));
            SetBool(algorithm, $"{family}.Use3D", HasFlag(flags, 1 << 1));
            SetBool(algorithm, $"{family}.UseFilter", HasFlag(flags, 1 << 2));
            SetBool(algorithm, $"{family}.UseFillHole", HasFlag(flags, 1 << 3));
            SetBool(algorithm, $"{family}.UseShift", HasFlag(flags, 1 << 4));
            SetBool(algorithm, $"{family}.ShiftXUse", HasFlag(flags, 1 << 5));
            SetBool(algorithm, $"{family}.ShiftYUse", HasFlag(flags, 1 << 6));
            SetBool(algorithm, $"{family}.UseWidth", HasFlag(flags, 1 << 7));
            SetBool(algorithm, $"{family}.UseLength", HasFlag(flags, 1 << 8));
            SetBool(algorithm, $"{family}.UseDistanceX", HasFlag(flags, 1 << 9));
            SetBool(algorithm, $"{family}.UseDistanceY", HasFlag(flags, 1 << 10));
        }

        if (TryReadNumberArrayLeaf(element, out var fData, "FData"))
        {
            SetIndexedDoubles(algorithm, family, "FData", fData);
            SetInt(algorithm, $"{family}.FDataCount", fData.Length);
            SetDouble(algorithm, $"{family}.HeightMin", ReadArrayDouble(fData, 0));
            SetDouble(algorithm, $"{family}.HeightMax", ReadArrayDouble(fData, 1));
            SetDouble(algorithm, $"{family}.ShiftX", ReadArrayDouble(fData, 2));
            SetDouble(algorithm, $"{family}.ShiftY", ReadArrayDouble(fData, 3));
            SetDouble(algorithm, $"{family}.CenterX", ReadArrayDouble(fData, 4));
            SetDouble(algorithm, $"{family}.CenterY", ReadArrayDouble(fData, 5));
            SetDouble(algorithm, $"{family}.Width", ReadArrayDouble(fData, 6));
            SetDouble(algorithm, $"{family}.WidthMin", ReadArrayDouble(fData, 7));
            SetDouble(algorithm, $"{family}.WidthMax", ReadArrayDouble(fData, 8));
            SetDouble(algorithm, $"{family}.Length", ReadArrayDouble(fData, 9));
            SetDouble(algorithm, $"{family}.LengthMin", ReadArrayDouble(fData, 10));
            SetDouble(algorithm, $"{family}.LengthMax", ReadArrayDouble(fData, 11));
            SetDouble(algorithm, $"{family}.DistanceX", ReadArrayDouble(fData, 12));
            SetDouble(algorithm, $"{family}.DistanceXMin", ReadArrayDouble(fData, 13));
            SetDouble(algorithm, $"{family}.DistanceXMax", ReadArrayDouble(fData, 14));
            SetDouble(algorithm, $"{family}.DistanceY", ReadArrayDouble(fData, 15));
            SetDouble(algorithm, $"{family}.DistanceYMin", ReadArrayDouble(fData, 16));
            SetDouble(algorithm, $"{family}.DistanceYMax", ReadArrayDouble(fData, 17));
            SetDouble(algorithm, $"{family}.HeightAvg", ReadArrayDouble(fData, 18));
            SetDouble(algorithm, $"{family}.HeightDiffMin", ReadArrayDouble(fData, 19));
            SetDouble(algorithm, $"{family}.HeightDiffMax", ReadArrayDouble(fData, 20));
        }

        SetIntIfPresent(algorithm, $"{family}.UseData", element, "UseData");

        for (var index = 1; index <= 6; index++)
        {
            ApplyPadArrayRoiParameters(algorithm, element, family, $"FirstROI{index}", $"ROIF{index}");
            ApplyPadArrayRoiParameters(algorithm, element, family, $"SecondROI{index}", $"ROIS{index}");
        }

        SetBool(algorithm, "Import.PadArrayMapped", true);
    }

    private static void ApplyPadArrayRoiParameters(InspectionAlgorithmData algorithm, XElement element, string family, string targetName, string legacyName)
    {
        if (!TryReadNumberArrayLeaf(element, out var roi, legacyName) || roi.Length < 4)
        {
            return;
        }

        SetDouble(algorithm, $"{family}.{targetName}.Left", roi[0]);
        SetDouble(algorithm, $"{family}.{targetName}.Top", roi[1]);
        SetDouble(algorithm, $"{family}.{targetName}.Right", roi[2]);
        SetDouble(algorithm, $"{family}.{targetName}.Bottom", roi[3]);
    }

    private static void ApplyCommonRangeParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        var family = algorithm.ParameterFamily;

        SetBoolIfPresent(algorithm, $"{family}.InvertCheck", element, "InvChk", "InvertCheck", "bInvertCheck");
        SetBoolIfPresent(algorithm, $"{family}.Use2D", element, "Use2D", "B2dCheck", "b2DCheck");
        SetBoolIfPresent(algorithm, $"{family}.Use3D", element, "Use3D", "B3dCheck", "b3DCheck");
        SetBoolIfPresent(algorithm, $"{family}.UseFilter", element, "UseFilter");
        SetIntIfPresent(algorithm, $"{family}.Range2DType", element, "R2D", "TPR2D", "Range");
        SetIntIfPresent(algorithm, $"{family}.Range3DType", element, "R3D", "TPR3D", "N3dRange", "Range3D");
        SetIntIfPresent(algorithm, $"{family}.FilterIndex", element, "FilterIndex", "Filter");
        SetDoubleIfPresent(algorithm, $"{family}.PercentOK", element, "PerOK", "PercentOK");
        SetBoolIfPresent(algorithm, $"{family}.UseArea", element, "UseArea");

        if (TryReadNumberArrayLeaf(element, out var binaryRange, "MnMx2D") && binaryRange.Length >= 2)
        {
            SetBinaryRange(algorithm, family, Round(binaryRange[0]), Round(binaryRange[1]));
        }
        else if (TryReadIntLeaf(element, out var minValue, "MinValue")
            && TryReadIntLeaf(element, out var maxValue, "MaxValue"))
        {
            SetBinaryRange(algorithm, family, minValue, maxValue);
        }

        if (TryReadNumberArrayLeaf(element, out var heightRange, "MnMx3D") && heightRange.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.HeightMin", heightRange[0]);
            SetDouble(algorithm, $"{family}.HeightMax", heightRange[1]);
        }
        else
        {
            SetDoubleIfPresent(algorithm, $"{family}.HeightMin", element, "D3dHeightMin", "d3dMinValue");
            SetDoubleIfPresent(algorithm, $"{family}.HeightMax", element, "D3dHeightMax", "d3dMaxValue");
        }

        if (TryReadNumberArrayLeaf(element, out var averageHeightRange, "MnMxAvgHei", "MnMxAvgHeiR", "MnMxAvg3D") && averageHeightRange.Length >= 3)
        {
            SetDouble(algorithm, $"{family}.MinHeight", averageHeightRange[0]);
            SetDouble(algorithm, $"{family}.MaxHeight", averageHeightRange[1]);
            SetDouble(algorithm, $"{family}.TargetHeight", averageHeightRange[2]);
            SetDouble(algorithm, $"{family}.HeightAvg", averageHeightRange[2]);
        }

        if (TryReadNumberArrayLeaf(element, out var areaRange, "MnMxArea") && areaRange.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.AreaMin", areaRange[0]);
            SetDouble(algorithm, $"{family}.AreaMax", areaRange[1]);
        }
        else
        {
            SetDoubleIfPresent(algorithm, $"{family}.AreaMin", element, "AreaMin");
            SetDoubleIfPresent(algorithm, $"{family}.AreaMax", element, "AreaMax");
        }

        if (TryReadNumberArrayLeaf(element, out var shift, "Sft", "Shift") && shift.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.ShiftX", shift[0]);
            SetDouble(algorithm, $"{family}.ShiftY", shift[1]);
        }
    }

    private static void ApplyDiffRoiParameters(InspectionAlgorithmData algorithm, XElement element, string family)
    {
        if (TryReadLeafValue(element, out var roi1, "ROI1"))
        {
            algorithm.Parameters[$"{family}.Roi1.Raw"] = roi1;
        }

        if (TryReadLeafValue(element, out var roi2, "ROI2"))
        {
            algorithm.Parameters[$"{family}.Roi2.Raw"] = roi2;
        }

        if (TryReadLeafValue(element, out var roi1Mm, "ROI1_mm"))
        {
            algorithm.Parameters[$"{family}.Roi1.Mm"] = roi1Mm;
        }

        if (TryReadLeafValue(element, out var roi2Mm, "ROI2_mm"))
        {
            algorithm.Parameters[$"{family}.Roi2.Mm"] = roi2Mm;
        }
    }

    private static void ApplyLeadPositionParameters(InspectionAlgorithmData algorithm, XElement element, string family)
    {
        SetIntIfPresent(algorithm, $"{family}.LeadTipDirection", element, "LeadTipDirection", "LT_Dir", "LeadDirection", "L_Dir");
        SetDoubleIfPresent(algorithm, $"{family}.LeadPosition", element, "LeadPosition", "L_Pos", "L_Pos_mm");
        SetDoubleIfPresent(algorithm, $"{family}.SolderLength", element, "SolderLength", "SD_Len", "SD_Len_mm");
        SetDoubleIfPresent(algorithm, $"{family}.SolderStartPos", element, "SolderStartPos", "SD_stPos");
    }

    private static void ApplySizeArrayParameters(
        InspectionAlgorithmData algorithm,
        XElement element,
        string family,
        string widthKey,
        string heightKey,
        params string[] names)
    {
        if (!TryReadNumberArrayLeaf(element, out var size, names) || size.Length < 2)
        {
            return;
        }

        SetDouble(algorithm, $"{family}.{widthKey}", size[0]);
        SetDouble(algorithm, $"{family}.{heightKey}", size[1]);
        SetDouble(algorithm, $"{family}.RoiWidth", size[0]);
        SetDouble(algorithm, $"{family}.RoiHeight", size[1]);
    }

    private static void SetIndexedDoubles(InspectionAlgorithmData algorithm, string family, string name, IReadOnlyList<double> values)
    {
        var limit = Math.Min(values.Count, 32);
        for (var index = 0; index < limit; index++)
        {
            SetDouble(algorithm, $"{family}.{name}{index + 1}", values[index]);
        }
    }

    private static void CopyRawArrayIfPresent(
        InspectionAlgorithmData algorithm,
        XElement element,
        string family,
        string key,
        params string[] names)
    {
        if (TryReadLeafValue(element, out var value, names))
        {
            algorithm.Parameters[$"{family}.{key}.Raw"] = value;
        }
    }

    private static void CopyPrefixedLeafValues(
        InspectionAlgorithmData algorithm,
        XElement element,
        string family,
        string targetPrefix,
        params string[] legacyPrefixes)
    {
        var copied = 0;
        foreach (var leaf in element.Descendants().Where(candidate => !candidate.HasElements && !string.IsNullOrWhiteSpace(candidate.Value)))
        {
            foreach (var legacyPrefix in legacyPrefixes)
            {
                var name = leaf.Name.LocalName;
                if (!name.StartsWith(legacyPrefix, StringComparison.OrdinalIgnoreCase))
                {
                    continue;
                }

                var suffix = name.Substring(legacyPrefix.Length).TrimStart('_');
                var key = string.IsNullOrWhiteSpace(suffix)
                    ? $"{family}.{targetPrefix}"
                    : $"{family}.{targetPrefix}.{suffix}";
                algorithm.Parameters[key] = leaf.Value.Trim();
                copied++;
                break;
            }
        }

        if (copied > 0)
        {
            SetInt(algorithm, $"{family}.{targetPrefix}.Count", copied);
        }
    }

    private static void CopyContainerLeafValues(
        InspectionAlgorithmData algorithm,
        XElement element,
        string family,
        string targetPrefix,
        params string[] containerNames)
    {
        var copied = 0;
        var containers = element.Descendants()
            .Where(candidate => containerNames.Any(name => string.Equals(candidate.Name.LocalName, name, StringComparison.OrdinalIgnoreCase)));

        foreach (var container in containers)
        {
            foreach (var leaf in container.Descendants().Where(candidate => !candidate.HasElements && !string.IsNullOrWhiteSpace(candidate.Value)))
            {
                algorithm.Parameters[$"{family}.{targetPrefix}.{leaf.Name.LocalName}"] = leaf.Value.Trim();
                copied++;
            }
        }

        if (copied > 0)
        {
            SetInt(algorithm, $"{family}.{targetPrefix}.Count", copied);
        }
    }

    private static void ApplyShapeXBlobBaseParameters(
        InspectionAlgorithmData algorithm,
        string family,
        string suffix,
        IReadOnlyList<double> data,
        bool promoteRuntimeKeys)
    {
        var prefix = $"{family}.{suffix}";
        var dataFlags = ReadArrayInt(data, 0);
        var min2D = ReadArrayInt(data, 1);
        var max2D = ReadArrayInt(data, 2, 255);
        SetInt(algorithm, $"{prefix}.DataFlags", dataFlags);
        SetBool(algorithm, $"{prefix}.Use", HasFlag(dataFlags, 0x01));
        SetBool(algorithm, $"{prefix}.Use2D", HasFlag(dataFlags, 0x02));
        SetBool(algorithm, $"{prefix}.Invert2D", HasFlag(dataFlags, 0x20));
        SetBool(algorithm, $"{prefix}.Use3D", HasFlag(dataFlags, 0x40));
        SetBool(algorithm, $"{prefix}.Invert", HasFlag(dataFlags, 0x400));
        SetBool(algorithm, $"{prefix}.FillHole", HasFlag(dataFlags, 0x800));
        SetBool(algorithm, $"{prefix}.UseFilter", HasFlag(dataFlags, 0x1000));
        SetInt(algorithm, $"{prefix}.Range2DType", ResolveBlobBaseRangeType(dataFlags));
        SetInt(algorithm, $"{prefix}.BinaryMin", min2D);
        SetInt(algorithm, $"{prefix}.BinaryMax", max2D);
        SetInt(algorithm, $"{prefix}.FilterIndex", ReadArrayInt(data, 3));
        SetDouble(algorithm, $"{prefix}.MinArea", ReadArrayDouble(data, 4));
        SetDouble(algorithm, $"{prefix}.HeightMin", ReadArrayDouble(data, 5));
        SetDouble(algorithm, $"{prefix}.HeightMax", ReadArrayDouble(data, 6));

        if (!promoteRuntimeKeys)
        {
            return;
        }

        SetBinaryRange(algorithm, family, min2D, max2D);
        SetBool(algorithm, $"{family}.Use2D", HasFlag(dataFlags, 0x02));
        SetBool(algorithm, $"{family}.UseFilter", HasFlag(dataFlags, 0x1000));
        SetInt(algorithm, $"{family}.Range2DType", ResolveBlobBaseRangeType(dataFlags));
        SetInt(algorithm, $"{family}.FilterIndex", ReadArrayInt(data, 3));
        SetInt(algorithm, $"{family}.MinBlobArea", Math.Max(1, Round(ReadArrayDouble(data, 4, 1))));
        SetDouble(algorithm, $"{family}.HeightMin", ReadArrayDouble(data, 5));
        SetDouble(algorithm, $"{family}.HeightMax", ReadArrayDouble(data, 6));
    }

    private static void ApplyShapeXRoi(InspectionAlgorithmData algorithm, XElement element, string family, LegacyRoiTransform transform, int zeroBasedIndex)
    {
        var oneBasedIndex = zeroBasedIndex + 1;
        var prefix = $"{family}.Roi{oneBasedIndex}";

        if (TryReadNumberArrayLeaf(element, out var roi, $"ROI{zeroBasedIndex}") && roi.Length >= 4)
        {
            SetDouble(algorithm, $"{prefix}.Left", roi[0]);
            SetDouble(algorithm, $"{prefix}.Top", roi[1]);
            SetDouble(algorithm, $"{prefix}.Right", roi[2]);
            SetDouble(algorithm, $"{prefix}.Bottom", roi[3]);
            algorithm.Parameters[$"{prefix}.Raw"] = string.Join(",", roi.Select(value => value.ToString("0.########", CultureInfo.InvariantCulture)));

            var leftTop = ConvertLegacyMmPointToPixel(roi[0], roi[1], transform);
            var rightBottom = ConvertLegacyMmPointToPixel(roi[2], roi[3], transform);
            SetInt(algorithm, $"{prefix}.PixelLeft", leftTop.X);
            SetInt(algorithm, $"{prefix}.PixelTop", leftTop.Y);
            SetInt(algorithm, $"{prefix}.PixelRight", rightBottom.X);
            SetInt(algorithm, $"{prefix}.PixelBottom", rightBottom.Y);
        }

        if (TryReadNumberArrayLeaf(element, out var center, $"pCt{zeroBasedIndex}") && center.Length >= 2)
        {
            SetDouble(algorithm, $"{prefix}.CenterX", center[0]);
            SetDouble(algorithm, $"{prefix}.CenterY", center[1]);
        }

        SetBoolIfPresent(algorithm, $"{prefix}.UseShape", element, $"UShp{zeroBasedIndex}");
        SetBoolIfPresent(algorithm, $"{prefix}.UseExistShape", element, $"UExShp{zeroBasedIndex}");
        SetBoolIfPresent(algorithm, $"{prefix}.UseInner", element, $"UIn{zeroBasedIndex}");
        SetBoolIfPresent(algorithm, $"{prefix}.UseExist", element, $"UExi{zeroBasedIndex}");
        SetBoolIfPresent(algorithm, $"{prefix}.UseShift", element, $"UShift{zeroBasedIndex}");
        SetBoolIfPresent(algorithm, $"{prefix}.UseLocalLength", element, $"ULoL{zeroBasedIndex}");
        SetBoolIfPresent(algorithm, $"{prefix}.UseDent", element, $"UDnt{zeroBasedIndex}");
        SetBoolIfPresent(algorithm, $"{prefix}.UseForeignWidth", element, $"UFrW{zeroBasedIndex}");
        SetBoolIfPresent(algorithm, $"{prefix}.UseForeignLength", element, $"UFrL{zeroBasedIndex}");
        SetDoubleIfPresent(algorithm, $"{prefix}.ShapeArea", element, $"ShpA{zeroBasedIndex}");
        SetDoubleIfPresent(algorithm, $"{prefix}.ExistShapeArea", element, $"ExShpA{zeroBasedIndex}");
        SetDoubleIfPresent(algorithm, $"{prefix}.InnerArea", element, $"InA{zeroBasedIndex}");
        SetDoubleIfPresent(algorithm, $"{prefix}.ExistArea", element, $"ExiA{zeroBasedIndex}");
        SetDoubleIfPresent(algorithm, $"{prefix}.LocalLengthMin", element, $"LolMn{zeroBasedIndex}");
        SetDoubleIfPresent(algorithm, $"{prefix}.LocalLengthMax", element, $"LolMx{zeroBasedIndex}");
        SetDoubleIfPresent(algorithm, $"{prefix}.Dent", element, $"Dnt{zeroBasedIndex}");
        SetDoubleIfPresent(algorithm, $"{prefix}.DentMin", element, $"DntMn{zeroBasedIndex}");
        SetDoubleIfPresent(algorithm, $"{prefix}.DentMax", element, $"DntMx{zeroBasedIndex}");
        SetDoubleIfPresent(algorithm, $"{prefix}.ForeignWidth", element, $"FrW{zeroBasedIndex}");
        SetDoubleIfPresent(algorithm, $"{prefix}.ForeignLength", element, $"FrL{zeroBasedIndex}");
        SetIntIfPresent(algorithm, $"{prefix}.ModelCount", element, $"MoCnt{zeroBasedIndex}");

        if (TryReadNumberArrayLeaf(element, out var shift, $"Shft{zeroBasedIndex}") && shift.Length >= 2)
        {
            SetDouble(algorithm, $"{prefix}.ShiftX", shift[0]);
            SetDouble(algorithm, $"{prefix}.ShiftY", shift[1]);
        }

        if (TryReadLeafValue(element, out var models, $"ArrMo{zeroBasedIndex}"))
        {
            algorithm.Parameters[$"{prefix}.Models"] = models;
        }

        if (zeroBasedIndex == 0)
        {
            PromoteFirstShapeXRoi(algorithm, element, family);
        }
    }

    private static void PromoteFirstShapeXRoi(InspectionAlgorithmData algorithm, XElement element, string family)
    {
        SetBoolIfPresent(algorithm, $"{family}.UseShapeXArea", element, "UShp0");
        SetBoolIfPresent(algorithm, $"{family}.ExistShape", element, "UExShp0");
        SetBoolIfPresent(algorithm, $"{family}.UseExistArea", element, "UExi0");
        SetBoolIfPresent(algorithm, $"{family}.UseLocalLength", element, "ULoL0");
        SetBoolIfPresent(algorithm, $"{family}.UseDent", element, "UDnt0");
        SetDoubleIfPresent(algorithm, $"{family}.ShapeArea", element, "ShpA0");
        SetDoubleIfPresent(algorithm, $"{family}.InnerArea", element, "InA0");
        SetDoubleIfPresent(algorithm, $"{family}.ExistArea", element, "ExiA0");
        SetDoubleIfPresent(algorithm, $"{family}.LocalLengthMin", element, "LolMn0");
        SetDoubleIfPresent(algorithm, $"{family}.LocalLengthMax", element, "LolMx0");
        SetDoubleIfPresent(algorithm, $"{family}.DentMin", element, "DntMn0");
        SetDoubleIfPresent(algorithm, $"{family}.DentMax", element, "DntMx0");

        if (TryReadDoubleLeaf(element, out var shapeArea, "ShpA0"))
        {
            SetDouble(algorithm, $"{family}.AreaMin", shapeArea);
        }

        if (TryReadNumberArrayLeaf(element, out var shift, "Shft0") && shift.Length >= 2)
        {
            SetDouble(algorithm, $"{family}.ShiftXTol", shift[0]);
            SetDouble(algorithm, $"{family}.ShiftYTol", shift[1]);
        }
    }

    private static void ApplyPadBwParameters(InspectionAlgorithmData algorithm, XElement element)
    {
        if (!TryReadNumberArrayLeaf(element, out var bData, "BData"))
        {
            return;
        }

        var fData = TryReadNumberArrayLeaf(element, out var parsedFData, "FData")
            ? parsedFData
            : Array.Empty<double>();

        SetInt(algorithm, "PadBW.RawBDataCount", bData.Length);
        SetInt(algorithm, "PadBW.RawFDataCount", fData.Length);

        var dataFlags = ReadArrayInt(bData, 0);
        var modeFlags = ReadArrayInt(bData, 1);
        var data2Flags = ReadArrayInt(bData, 20);
        var data3Flags = ReadArrayInt(bData, 23);
        var useHistogram = HasFlag(modeFlags, 0x04);

        var binaryMin = useHistogram
            ? ReadArrayInt(fData, 18, ReadArrayInt(bData, 8, ReadArrayInt(bData, 2)))
            : ReadArrayInt(bData, 2);
        var binaryMax = useHistogram
            ? ReadArrayInt(fData, 19, ReadArrayInt(bData, 9, ReadArrayInt(bData, 3, 255)))
            : ReadArrayInt(bData, 3, 255);

        SetInt(algorithm, "PadBW.BinaryMin", Net48Compat.Clamp(binaryMin, 0, 255));
        SetInt(algorithm, "PadBW.BinaryMax", Net48Compat.Clamp(binaryMax, 0, 255));
        SetBool(algorithm, "PadBW.Use2D", HasFlag(modeFlags, 0x01));
        SetBool(algorithm, "PadBW.Use3D", HasFlag(modeFlags, 0x02));
        SetBool(algorithm, "PadBW.UseHistogram", useHistogram);
        SetBool(algorithm, "PadBW.UseFillHole", HasFlag(modeFlags, 0x08));
        SetInt(algorithm, "PadBW.Range2DType", ReadArrayInt(bData, 4));
        SetInt(algorithm, "PadBW.Range3DType", ReadArrayInt(bData, 5));
        SetInt(algorithm, "PadBW.LightCount", ReadArrayInt(bData, 6));
        SetInt(algorithm, "PadBW.HistogramRangeType", ReadArrayInt(bData, 7));
        SetInt(algorithm, "PadBW.HistogramMin", ReadArrayInt(bData, 8));
        SetInt(algorithm, "PadBW.HistogramMax", ReadArrayInt(bData, 9));
        SetInt(algorithm, "PadBW.HistogramMaxFreq", ReadArrayInt(bData, 10));
        SetInt(algorithm, "PadBW.Mask", ReadArrayInt(bData, 11));
        ApplyPadBwMaskFlags(algorithm, "PadBW", ReadArrayInt(bData, 11));
        SetInt(algorithm, "PadBW.HistogramLimitMin", ReadArrayInt(bData, 12));
        SetInt(algorithm, "PadBW.HistogramLimitMax", ReadArrayInt(bData, 13));
        SetInt(algorithm, "PadBW.MaskShape", ReadArrayInt(bData, 14));
        SetInt(algorithm, "PadBW.EdgeFilterLevel", ReadArrayInt(bData, 15));
        SetInt(algorithm, "PadBW.Correct3DOrder", ReadArrayInt(bData, 16));
        SetInt(algorithm, "PadBW.FilterLevel", ReadArrayInt(bData, 17));
        SetInt(algorithm, "PadBW.SelectBlobType", ReadArrayInt(bData, 18));
        SetInt(algorithm, "PadBW.SelectBlobNum", ReadArrayInt(bData, 19));
        SetInt(algorithm, "PadBW.ContrastValue", ReadArrayInt(bData, 21));
        SetInt(algorithm, "PadBW.Direction", ReadArrayInt(bData, 22));
        SetInt(algorithm, "PadBW.AIModelID", ReadArrayInt(bData, 24, -1));

        SetBool(algorithm, "PadBW.UseFilter", HasFlag(dataFlags, 0x01));
        SetBool(algorithm, "PadBW.UseTeachArea", HasFlag(dataFlags, 0x02));
        SetBool(algorithm, "PadBW.UseShift", HasFlag(dataFlags, 0x04));
        SetBool(algorithm, "PadBW.UseBlobWidth", HasFlag(dataFlags, 0x08));
        SetBool(algorithm, "PadBW.UseBlobLength", HasFlag(dataFlags, 0x10));
        SetBool(algorithm, "PadBW.UseBlobArea", HasFlag(dataFlags, 0x20));
        SetBool(algorithm, "PadBW.UseOption3DMinMax", HasFlag(dataFlags, 0x40));
        SetBool(algorithm, "PadBW.UseOption3DRange", HasFlag(dataFlags, 0x80));
        SetBool(algorithm, "PadBW.UseRelativeHeight", HasFlag(data2Flags, 0x01));
        SetBool(algorithm, "PadBW.UseShadeFix", HasFlag(data2Flags, 0x02));
        SetBool(algorithm, "PadBW.UseBlobAnd", HasFlag(data2Flags, 0x04));
        SetBool(algorithm, "PadBW.UseContrastGV", HasFlag(data2Flags, 0x08));
        SetBool(algorithm, "PadBW.UseHoleAlign", HasFlag(data2Flags, 0x10));
        SetBool(algorithm, "PadBW.UseNGGrouping", HasFlag(data2Flags, 0x20));
        SetBool(algorithm, "PadBW.UseDirection", HasFlag(data2Flags, 0x40));
        SetBool(algorithm, "PadBW.UseWarningArea", HasFlag(data3Flags, 0x01));
        SetBool(algorithm, "PadBW.WarningAnd", HasFlag(data3Flags, 0x02));
        SetBool(algorithm, "PadBW.UseImageAnd", HasFlag(data3Flags, 0x04));
        SetBool(algorithm, "PadBW.UseAI", HasFlag(data3Flags, 0x08));

        SetDouble(algorithm, "PadBW.HeightMin", ReadArrayDouble(fData, 0));
        SetDouble(algorithm, "PadBW.HeightMax", ReadArrayDouble(fData, 1));
        SetDouble(algorithm, "PadBW.TeachArea", ReadArrayDouble(fData, 2));
        SetDouble(algorithm, "PadBW.AreaRateMin", ReadArrayDouble(fData, 3, 80));
        SetDouble(algorithm, "PadBW.AreaRateMax", ReadArrayDouble(fData, 4, 120));
        SetDouble(algorithm, "PadBW.ShiftX", ReadArrayDouble(fData, 5));
        SetDouble(algorithm, "PadBW.ShiftY", ReadArrayDouble(fData, 6));
        SetDouble(algorithm, "PadBW.BlobWidth", ReadArrayDouble(fData, 7));
        SetDouble(algorithm, "PadBW.BlobLength", ReadArrayDouble(fData, 8));
        SetDouble(algorithm, "PadBW.BlobAreaMin", ReadArrayDouble(fData, 9));
        SetDouble(algorithm, "PadBW.Option3DMin", ReadArrayDouble(fData, 10));
        SetDouble(algorithm, "PadBW.Option3DMax", ReadArrayDouble(fData, 11));
        SetDouble(algorithm, "PadBW.Option3DRange", ReadArrayDouble(fData, 12));
        SetDouble(algorithm, "PadBW.Option3DThickMin", ReadArrayDouble(fData, 13));
        SetDouble(algorithm, "PadBW.RelativeHeightMin", ReadArrayDouble(fData, 14));
        SetDouble(algorithm, "PadBW.RelativeHeightMax", ReadArrayDouble(fData, 15));
        SetDouble(algorithm, "PadBW.NGGroupingMaxSize", ReadArrayDouble(fData, 16));
        SetDouble(algorithm, "PadBW.NGGroupingDistance", ReadArrayDouble(fData, 17));
        SetDouble(algorithm, "PadBW.HistogramMinValue", ReadArrayDouble(fData, 18, ReadArrayDouble(bData, 8)));
        SetDouble(algorithm, "PadBW.HistogramMaxValue", ReadArrayDouble(fData, 19, ReadArrayDouble(bData, 9)));
        SetDouble(algorithm, "PadBW.DirectionLength", ReadArrayDouble(fData, 20));
        SetInt(algorithm, "PadBW.MinBlobArea", Math.Max(1, Round(ReadArrayDouble(fData, 9, 1))));

        if (TryReadLeafValue(element, out var amRoi, "AMROI"))
        {
            algorithm.Parameters["PadBW.MaskRoi"] = amRoi;
        }

        if (TryReadIntLeaf(element, out var blobInfoCount, "BlobInfoCnt_"))
        {
            SetInt(algorithm, "PadBW.BlobInfoCount", Math.Max(0, blobInfoCount));
            ApplyPadBwBlobLabelInfo(algorithm, element, Math.Max(0, blobInfoCount));
        }

        for (var index = 1; index <= 4; index++)
        {
            ApplyPadBwSubParameters(algorithm, element, index);
        }

        SetBool(algorithm, "Import.PadBWMapped", true);
    }

    private static void ApplyPadBwSubParameters(InspectionAlgorithmData algorithm, XElement element, int index)
    {
        if (!TryReadNumberArrayLeaf(element, out var bData, $"SubBData_{index}"))
        {
            return;
        }

        var fData = TryReadNumberArrayLeaf(element, out var parsedFData, $"SubFData_{index}")
            ? parsedFData
            : Array.Empty<double>();
        var modeFlags = ReadArrayInt(bData, 1);
        var useHistogram = HasFlag(modeFlags, 0x04);
        var prefix = $"PadBW.Sub{index}";
        var subBinaryMin = useHistogram
            ? ReadArrayInt(fData, 2, ReadArrayInt(bData, 5, ReadArrayInt(bData, 2)))
            : ReadArrayInt(bData, 2);
        var subBinaryMax = useHistogram
            ? ReadArrayInt(fData, 3, ReadArrayInt(bData, 6, ReadArrayInt(bData, 3, 255)))
            : ReadArrayInt(bData, 3, 255);

        SetInt(algorithm, $"{prefix}.DataFlags", ReadArrayInt(bData, 0));
        SetBool(algorithm, $"{prefix}.UseFilter", HasFlag(ReadArrayInt(bData, 0), 0x01));
        SetBool(algorithm, $"{prefix}.Use2D", HasFlag(modeFlags, 0x01));
        SetBool(algorithm, $"{prefix}.Use3D", HasFlag(modeFlags, 0x02));
        SetBool(algorithm, $"{prefix}.UseHistogram", useHistogram);
        SetBool(algorithm, $"{prefix}.UseFillHole", HasFlag(modeFlags, 0x08));
        SetInt(algorithm, $"{prefix}.BinaryMin", Net48Compat.Clamp(subBinaryMin, 0, 255));
        SetInt(algorithm, $"{prefix}.BinaryMax", Net48Compat.Clamp(subBinaryMax, 0, 255));
        SetInt(algorithm, $"{prefix}.Range2DType", ReadArrayInt(bData, 4));
        SetInt(algorithm, $"{prefix}.HistogramMinByte", ReadArrayInt(bData, 5));
        SetInt(algorithm, $"{prefix}.HistogramMaxByte", ReadArrayInt(bData, 6));
        SetInt(algorithm, $"{prefix}.Range3DType", ReadArrayInt(bData, 7));
        SetInt(algorithm, $"{prefix}.RangeHistogramType", ReadArrayInt(bData, 8));
        SetInt(algorithm, $"{prefix}.HistogramMaxFreq", ReadArrayInt(bData, 9));
        SetInt(algorithm, $"{prefix}.Mask", ReadArrayInt(bData, 10));
        ApplyPadBwMaskFlags(algorithm, prefix, ReadArrayInt(bData, 10));
        SetInt(algorithm, $"{prefix}.HistogramLimitMin", ReadArrayInt(bData, 11));
        SetInt(algorithm, $"{prefix}.HistogramLimitMax", ReadArrayInt(bData, 12));
        SetInt(algorithm, $"{prefix}.MaskShape", ReadArrayInt(bData, 13));
        SetInt(algorithm, $"{prefix}.EdgeFilterLevel", ReadArrayInt(bData, 14));
        SetInt(algorithm, $"{prefix}.FilterSize", ReadArrayInt(bData, 15));
        SetDouble(algorithm, $"{prefix}.HeightMin", ReadArrayDouble(fData, 0));
        SetDouble(algorithm, $"{prefix}.HeightMax", ReadArrayDouble(fData, 1));
        SetDouble(algorithm, $"{prefix}.HistogramMin", ReadArrayDouble(fData, 2, ReadArrayDouble(bData, 5)));
        SetDouble(algorithm, $"{prefix}.HistogramMax", ReadArrayDouble(fData, 3, ReadArrayDouble(bData, 6)));
        SetDouble(algorithm, $"{prefix}.HistogramMinValue", ReadArrayDouble(fData, 2, ReadArrayDouble(bData, 5)));
        SetDouble(algorithm, $"{prefix}.HistogramMaxValue", ReadArrayDouble(fData, 3, ReadArrayDouble(bData, 6)));

        if (TryReadNumberArrayLeaf(element, out var subLight, $"SubLight_{index}"))
        {
            SetInt(algorithm, $"{prefix}.LightCount", ReadArrayInt(subLight, 0));
        }
    }

    private static void ApplyPadBwMaskFlags(InspectionAlgorithmData algorithm, string prefix, int maskFlags)
    {
        SetBool(algorithm, $"{prefix}.MaskFilter", HasFlag(maskFlags, 0x01));
        SetBool(algorithm, $"{prefix}.MaskBright", HasFlag(maskFlags, 0x02));
        SetBool(algorithm, $"{prefix}.MaskIn", HasFlag(maskFlags, 0x04));
        SetBool(algorithm, $"{prefix}.MaskOut", HasFlag(maskFlags, 0x08));
    }

    private static void ApplyPadBwBlobLabelInfo(InspectionAlgorithmData algorithm, XElement element, int blobInfoCount)
    {
        var limit = Math.Min(blobInfoCount, 32);
        if (limit <= 0)
        {
            return;
        }

        TryReadNumberArrayLeaf(element, out var ids, "BlobID_");
        TryReadNumberArrayLeaf(element, out var centerXs, "BlobCX_");
        TryReadNumberArrayLeaf(element, out var centerYs, "BlobCY_");
        TryReadNumberArrayLeaf(element, out var areas, "BlobArea_");
        TryReadNumberArrayLeaf(element, out var widths, "BlobWidth_");
        TryReadNumberArrayLeaf(element, out var lengths, "BlobLength_");
        TryReadNumberArrayLeaf(element, out var startXs, "BlobStX_");
        TryReadNumberArrayLeaf(element, out var startYs, "BlobStY_");

        for (var offset = 0; offset < limit; offset++)
        {
            var prefix = $"PadBW.BlobInfo{offset + 1}";
            SetOptionalInt(algorithm, $"{prefix}.Id", ids, offset);
            SetOptionalDouble(algorithm, $"{prefix}.CenterX", centerXs, offset);
            SetOptionalDouble(algorithm, $"{prefix}.CenterY", centerYs, offset);
            SetOptionalDouble(algorithm, $"{prefix}.Area", areas, offset);
            SetOptionalDouble(algorithm, $"{prefix}.Width", widths, offset);
            SetOptionalDouble(algorithm, $"{prefix}.Length", lengths, offset);
            SetOptionalDouble(algorithm, $"{prefix}.StartX", startXs, offset);
            SetOptionalDouble(algorithm, $"{prefix}.StartY", startYs, offset);
        }
    }

    private static void SetOptionalInt(InspectionAlgorithmData algorithm, string key, IReadOnlyList<double> values, int index)
    {
        if (index >= 0 && index < values.Count)
        {
            SetInt(algorithm, key, Round(values[index]));
        }
    }

    private static void SetOptionalDouble(InspectionAlgorithmData algorithm, string key, IReadOnlyList<double> values, int index)
    {
        if (index >= 0 && index < values.Count)
        {
            SetDouble(algorithm, key, values[index]);
        }
    }

    private static void SetBoolIfPresent(InspectionAlgorithmData algorithm, string key, XElement element, params string[] names)
    {
        if (TryReadBoolLeaf(element, out var value, names))
        {
            SetBool(algorithm, key, value);
        }
    }

    private static void SetIntIfPresent(InspectionAlgorithmData algorithm, string key, XElement element, params string[] names)
    {
        if (TryReadIntLeaf(element, out var value, names))
        {
            SetInt(algorithm, key, value);
        }
    }

    private static void SetDoubleIfPresent(InspectionAlgorithmData algorithm, string key, XElement element, params string[] names)
    {
        if (TryReadDoubleLeaf(element, out var value, names))
        {
            SetDouble(algorithm, key, value);
        }
    }

    private static void SetStringIfPresent(InspectionAlgorithmData algorithm, string key, XElement element, params string[] names)
    {
        if (TryReadLeafValue(element, out var value, names))
        {
            algorithm.Parameters[key] = value;
        }
    }

    private static void SetStringIfAvailable(InspectionAlgorithmData algorithm, string key, IReadOnlyList<string> values, int index)
    {
        if (index >= 0 && index < values.Count && !string.IsNullOrWhiteSpace(values[index]))
        {
            algorithm.Parameters[key] = values[index];
        }
    }

    private static int CountDelimitedTokens(string value)
    {
        return value.Split(new[] { ',' }, StringSplitOptions.RemoveEmptyEntries)
            .Count(token => !string.IsNullOrWhiteSpace(token));
    }

    private static void SetBinaryRange(InspectionAlgorithmData algorithm, string family, int minValue, int maxValue)
    {
        var min = Net48Compat.Clamp(minValue, 0, 255);
        var max = Net48Compat.Clamp(maxValue, 0, 255);
        SetInt(algorithm, $"{family}.MinValue", min);
        SetInt(algorithm, $"{family}.MaxValue", max);
        SetInt(algorithm, $"{family}.BinaryMin", min);
        SetInt(algorithm, $"{family}.BinaryMax", max);
        SetInt(algorithm, $"{family}.Threshold", min);
    }

    private static void SetFlagBooleans(InspectionAlgorithmData algorithm, string family, int flags, params (string Key, int Flag)[] flagMap)
    {
        foreach (var (key, flag) in flagMap)
        {
            SetBool(algorithm, $"{family}.{key}", HasFlag(flags, flag));
        }
    }

    private static int ResolveBlobBaseRangeType(int flags)
    {
        if (HasFlag(flags, 0x10))
        {
            return 0;
        }

        if (HasFlag(flags, 0x04))
        {
            return 2;
        }

        if (HasFlag(flags, 0x08))
        {
            return 3;
        }

        return 0;
    }

    private static (int X, int Y) ConvertLegacyMmPointToPixel(double xMm, double yMm, LegacyRoiTransform transform)
    {
        if (!transform.HasResolution)
        {
            return (Round(xMm), Round(yMm));
        }

        return (
            Round(transform.OriginX + xMm / transform.PixelResolutionX),
            Round(transform.OriginY + yMm / transform.PixelResolutionY));
    }

    private static bool TryReadNumberArrayLeaf(XElement element, out double[] values, params string[] names)
    {
        values = Array.Empty<double>();
        if (!TryReadLeafValue(element, out var raw, names))
        {
            return false;
        }

        var parsed = new List<double>();
        foreach (var token in raw.Split(new[] { ',' }, StringSplitOptions.RemoveEmptyEntries))
        {
            if (!TryParseDouble(token.Trim(), out var number))
            {
                continue;
            }

            parsed.Add(number);
        }

        values = parsed.ToArray();
        return values.Length > 0;
    }

    private static bool TryReadBoolArrayLeaf(XElement element, out bool[] values, params string[] names)
    {
        values = Array.Empty<bool>();
        if (!TryReadLeafValue(element, out var raw, names))
        {
            return false;
        }

        var parsed = new List<bool>();
        foreach (var token in raw.Split(new[] { ',' }, StringSplitOptions.RemoveEmptyEntries))
        {
            var trimmed = token.Trim();
            if (bool.TryParse(trimmed, out var boolValue))
            {
                parsed.Add(boolValue);
                continue;
            }

            if (TryParseDouble(trimmed, out var number))
            {
                parsed.Add(Math.Abs(number) > double.Epsilon);
            }
        }

        values = parsed.ToArray();
        return values.Length > 0;
    }

    private static bool TryReadIntLeaf(XElement element, out int value, params string[] names)
    {
        value = 0;
        if (!TryReadDoubleLeaf(element, out var parsed, names))
        {
            return false;
        }

        value = Round(parsed);
        return true;
    }

    private static bool TryReadDoubleLeaf(XElement element, out double value, params string[] names)
    {
        value = 0;
        if (!TryReadLeafValue(element, out var raw, names))
        {
            return false;
        }

        return TryParseDouble(raw, out value);
    }

    private static bool TryReadBoolLeaf(XElement element, out bool value, params string[] names)
    {
        value = false;
        if (!TryReadLeafValue(element, out var raw, names))
        {
            return false;
        }

        if (bool.TryParse(raw, out value))
        {
            return true;
        }

        if (TryParseDouble(raw, out var number))
        {
            value = Math.Abs(number) > double.Epsilon;
            return true;
        }

        return false;
    }

    private static bool TryReadLeafValue(XElement element, out string value, params string[] names)
    {
        foreach (var name in names)
        {
            var direct = element.Elements()
                .FirstOrDefault(candidate => string.Equals(candidate.Name.LocalName, name, StringComparison.OrdinalIgnoreCase));
            if (direct != null && !direct.HasElements && !string.IsNullOrWhiteSpace(direct.Value))
            {
                value = direct.Value.Trim();
                return true;
            }
        }

        foreach (var name in names)
        {
            var descendant = element.Descendants()
                .FirstOrDefault(candidate => !candidate.HasElements
                    && string.Equals(candidate.Name.LocalName, name, StringComparison.OrdinalIgnoreCase)
                    && !string.IsNullOrWhiteSpace(candidate.Value));
            if (descendant != null)
            {
                value = descendant.Value.Trim();
                return true;
            }
        }

        value = "";
        return false;
    }

    private static int ReadArrayInt(IReadOnlyList<double> values, int index, int fallback = 0)
    {
        return index >= 0 && index < values.Count ? Round(values[index]) : fallback;
    }

    private static double ReadArrayDouble(IReadOnlyList<double> values, int index, double fallback = 0)
    {
        return index >= 0 && index < values.Count ? values[index] : fallback;
    }

    private static bool HasFlag(int value, int flag)
    {
        return (value & flag) == flag;
    }

    private static void SetInt(InspectionAlgorithmData algorithm, string key, int value)
    {
        algorithm.Parameters[key] = value.ToString(CultureInfo.InvariantCulture);
    }

    private static void SetDouble(InspectionAlgorithmData algorithm, string key, double value)
    {
        algorithm.Parameters[key] = value.ToString("0.########", CultureInfo.InvariantCulture);
    }

    private static void SetBool(InspectionAlgorithmData algorithm, string key, bool value)
    {
        algorithm.Parameters[key] = value ? "true" : "false";
    }

    private static bool TryParseDouble(string value, out double parsed)
    {
        return double.TryParse(value, NumberStyles.Float, CultureInfo.InvariantCulture, out parsed)
            || double.TryParse(value, NumberStyles.Float, CultureInfo.CurrentCulture, out parsed);
    }

    private static InspectionAlgorithmData CreateFlagAlgorithm(string type, int index, XElement windowElement)
    {
        var algorithm = new InspectionAlgorithmData
        {
            Id = $"Algo{index}",
            Type = type
        };
        algorithm.ApplyCatalogDefaults();
        algorithm.DisplayName = $"{algorithm.DisplayName} #{index}";
        algorithm.Parameters["Legacy.Source"] = "RawData XML window algorithm flag";
        algorithm.Parameters["Legacy.WindowID"] = FirstValue(windowElement, "ID");
        algorithm.Result = new InspectionResultData
        {
            Message = "Imported from legacy RawData window algorithm flag"
        };
        algorithm.PanelData = AlgorithmPanelSchema.Create(algorithm);
        return algorithm;
    }

    private static RoiRect ParseRoi(XElement? roiElement, LegacyRoiTransform transform)
    {
        if (roiElement == null)
        {
            return new RoiRect(0, 0, 1, 1);
        }

        var rawWidth = ReadDouble(roiElement, "w", "Width", "SizeX");
        var rawHeight = ReadDouble(roiElement, "h", "Height", "SizeY");
        var cx = ReadDouble(roiElement, "cx", "CenterX", "X");
        var cy = ReadDouble(roiElement, "cy", "CenterY", "Y");
        if (transform.HasResolution && LooksLikeMillimeterRoi(rawWidth, rawHeight))
        {
            var widthPixels = Math.Max(1, Round(rawWidth / transform.PixelResolutionX));
            var heightPixels = Math.Max(1, Round(rawHeight / transform.PixelResolutionY));
            var centerX = transform.OriginX + cx / transform.PixelResolutionX;
            var centerY = transform.OriginY + cy / transform.PixelResolutionY;
            return new RoiRect(
                Math.Max(0, Round(centerX - widthPixels / 2.0)),
                Math.Max(0, Round(centerY - heightPixels / 2.0)),
                widthPixels,
                heightPixels);
        }

        var width = Math.Max(1, Round(rawWidth));
        var height = Math.Max(1, Round(rawHeight));
        var x = Round(cx - width / 2.0);
        var y = Round(cy - height / 2.0);
        return new RoiRect(Math.Max(0, x), Math.Max(0, y), width, height);
    }

    private static RoiRect? ParseOptionalAlgorithmRoi(XElement algorithmElement, LegacyRoiTransform transform)
    {
        var roi = algorithmElement.Elements()
            .FirstOrDefault(element => element.Name.LocalName.Contains("Roi", StringComparison.OrdinalIgnoreCase)
                && element.Elements().Any());
        return roi == null ? null : ParseRoi(roi, transform);
    }

    private static LegacyRoiTransform CreateRoiTransform(XElement root, XElement? initRoot, XElement? partElement)
    {
        var resolutionX = FirstPositive(
            ReadOptionalDouble(initRoot, "PixelResolutionX", "ResolutionX", "ResX"),
            ReadDouble(root, "PixelResolutionX", "ResolutionX", "ResX"));
        var resolutionY = FirstPositive(
            ReadOptionalDouble(initRoot, "PixelResolutionY", "ResolutionY", "ResY"),
            ReadDouble(root, "PixelResolutionY", "ResolutionY", "ResY"));

        var partRoi = partElement?.Element("Roi") ?? root.Element("Roi");
        var sourceWidth = ReadOptionalInt(initRoot, "ImageWidth", "SourceWidth", "Width");
        var sourceHeight = ReadOptionalInt(initRoot, "ImageHeight", "SourceHeight", "Height");
        if (partRoi != null && resolutionX > 0 && resolutionY > 0)
        {
            sourceWidth = sourceWidth > 0 ? sourceWidth : Math.Max(1, Round(ReadDouble(partRoi, "w", "Width", "SizeX") / resolutionX));
            sourceHeight = sourceHeight > 0 ? sourceHeight : Math.Max(1, Round(ReadDouble(partRoi, "h", "Height", "SizeY") / resolutionY));
        }

        return new LegacyRoiTransform(resolutionX, resolutionY, sourceWidth, sourceHeight);
    }

    private static bool LooksLikeMillimeterRoi(double width, double height)
    {
        return width > 0
            && height > 0
            && width < 500
            && height < 500;
    }

    private static string FormatResolution(LegacyRoiTransform transform)
    {
        return transform.HasResolution
            ? $"{transform.PixelResolutionX.ToString("0.########", CultureInfo.InvariantCulture)} x {transform.PixelResolutionY.ToString("0.########", CultureInfo.InvariantCulture)} mm/px, FOV {transform.SourceWidth}x{transform.SourceHeight}"
            : "not found";
    }

    private static double FirstPositive(params double[] values)
    {
        return values.FirstOrDefault(value => value > 0);
    }

    private static string ResolveAlgorithmType(string rawType, XElement algorithmElement)
    {
        if (AlgorithmNameMap.TryGetValue(rawType, out var mapped))
        {
            return mapped;
        }

        if (int.TryParse(rawType, NumberStyles.Integer, CultureInfo.InvariantCulture, out var ordinal)
            && ordinal >= 0
            && ordinal < AlgorithmOrdinalMap.Length)
        {
            return AlgorithmOrdinalMap[ordinal];
        }

        foreach (var value in algorithmElement.Descendants().Select(child => child.Value.Trim()).Where(value => value.Length > 0))
        {
            if (AlgorithmNameMap.TryGetValue(value, out mapped))
            {
                return mapped;
            }
        }

        return "AlgoAlign";
    }

    private static IEnumerable<string> ResolveWindowAlgorithmTypes(XElement windowElement)
    {
        var normalFlag = ReadInt(windowElement, "AlgoKind", "AlgorithmKind", "AlgorithmFlag", "AlgorithmNo", "AlgoNo", "Kind");
        var extendedFlag = ReadInt(windowElement, "AlgoKind2", "AlgorithmKind2", "AlgorithmFlag2", "AlgorithmNo2", "AlgoNo2", "Kind2");

        foreach (var type in ExpandLegacyFlags(normalFlag, NormalFlagOrder, firstFlagBit: 1))
        {
            yield return type;
        }

        foreach (var type in ExpandLegacyFlags(extendedFlag, ExtendedFlagOrder, firstFlagBit: 0))
        {
            yield return type;
        }
    }

    private static IEnumerable<string> ExpandLegacyFlags(int total, IReadOnlyList<string> order, int firstFlagBit)
    {
        if (total <= 0)
        {
            yield break;
        }

        for (var index = 0; index < order.Count; index++)
        {
            var flag = 1 << (index + firstFlagBit);
            if ((total & flag) == flag)
            {
                yield return order[index];
            }
        }
    }

    private static IEnumerable<KeyValuePair<string, string>> FlattenLeafValues(XElement root)
    {
        foreach (var leaf in root.Descendants().Where(element => !element.HasElements))
        {
            var value = leaf.Value.Trim();
            if (string.IsNullOrWhiteSpace(value))
            {
                continue;
            }

            yield return new KeyValuePair<string, string>(BuildPath(root, leaf), value);
        }
    }

    private static string BuildPath(XElement root, XElement leaf)
    {
        var parts = new Stack<string>();
        var current = leaf;
        while (current != root && current.Parent != null)
        {
            parts.Push(current.Name.LocalName);
            current = current.Parent;
        }

        return "Legacy." + string.Join(".", parts);
    }

    private static bool IsRawPartRoot(XElement root)
    {
        return root.Name.LocalName is "RawDataContainer" or "JobContainer"
            || root.Descendants("WindowDataList").Any();
    }

    private static string FirstValue(XElement? element, params string[] names)
    {
        if (element == null)
        {
            return "";
        }

        foreach (var name in names)
        {
            var child = element.Elements().FirstOrDefault(candidate => string.Equals(candidate.Name.LocalName, name, StringComparison.OrdinalIgnoreCase));
            if (child != null)
            {
                return child.Value.Trim();
            }
        }

        return "";
    }

    private static double ReadDouble(XElement element, params string[] names)
    {
        var value = FirstValue(element, names);
        if (double.TryParse(value, NumberStyles.Float, CultureInfo.InvariantCulture, out var parsed))
        {
            return parsed;
        }

        if (double.TryParse(value, NumberStyles.Float, CultureInfo.CurrentCulture, out parsed))
        {
            return parsed;
        }

        return 0;
    }

    private static double ReadOptionalDouble(XElement? element, params string[] names)
    {
        return element == null ? 0 : ReadDouble(element, names);
    }

    private static int ReadInt(XElement element, params string[] names)
    {
        var value = FirstValue(element, names);
        if (int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out var parsed))
        {
            return parsed;
        }

        return 0;
    }

    private static int ReadOptionalInt(XElement? element, params string[] names)
    {
        return element == null ? 0 : ReadInt(element, names);
    }

    private static int Round(double value)
    {
        return (int)Math.Round(value, MidpointRounding.AwayFromZero);
    }

    private static string DecodeLegacyText(string value)
    {
        if (string.IsNullOrWhiteSpace(value))
        {
            return "";
        }

        var current = value.Trim();
        for (var i = 0; i < 3; i++)
        {
            try
            {
                var bytes = Convert.FromBase64String(current);
                var decoded = Encoding.Unicode.GetString(bytes).TrimEnd('\0').Trim();
                if (string.IsNullOrWhiteSpace(decoded) || string.Equals(decoded, current, StringComparison.Ordinal))
                {
                    break;
                }

                current = decoded;
            }
            catch (FormatException)
            {
                break;
            }
        }

        return current;
    }

    private readonly record struct LegacyRoiTransform(double PixelResolutionX, double PixelResolutionY, int SourceWidth, int SourceHeight)
    {
        public bool HasResolution => PixelResolutionX > 0 && PixelResolutionY > 0;
        public double OriginX => SourceWidth > 0 ? SourceWidth / 2.0 : 0;
        public double OriginY => SourceHeight > 0 ? SourceHeight / 2.0 : 0;
    }
}

