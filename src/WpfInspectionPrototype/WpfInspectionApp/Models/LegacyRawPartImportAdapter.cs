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

        algorithm.Result = new InspectionResultData
        {
            Message = "Imported from legacy RawData"
        };
        algorithm.PanelData = AlgorithmPanelSchema.Create(algorithm);
        return algorithm;
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

