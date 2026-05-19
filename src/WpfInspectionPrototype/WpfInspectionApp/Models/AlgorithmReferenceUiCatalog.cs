namespace WpfInspectionApp.Models;

public enum AlgorithmReferenceControlKind
{
    Check,
    Number,
    Slider,
    Combo,
    Command
}

public sealed record AlgorithmReferenceControl(
    string Tab,
    AlgorithmReferenceControlKind Kind,
    string Label,
    string Key,
    string DefaultValue,
    int Minimum = 0,
    int Maximum = 255,
    string[]? Options = null);

public sealed class AlgorithmReferenceUiProfile
{
    public string SourceControl { get; set; } = "";
    public string EventSummary { get; set; } = "";
    public List<AlgorithmReferenceControl> Controls { get; set; } = [];
}

public static partial class AlgorithmReferenceUiCatalog
{
    public static AlgorithmReferenceUiProfile Create(AlgorithmCatalogItem catalog)
    {
        var profile = new AlgorithmReferenceUiProfile
        {
            SourceControl = ResolveSourceControl(catalog),
            EventSummary = "TextChanged / CheckedChanged / SelectedIndexChanged / Click / MouseUp / KeyPress values are persisted into Algorithm.Parameters."
        };

        AddCommon(profile);
        AddByFamily(profile, catalog);
        AddTypeSpecific(profile, catalog);
        AddRoiMask(profile);
        AddCommands(profile, catalog);
        return profile;
    }

    private static void AddCommon(AlgorithmReferenceUiProfile profile)
    {
        profile.Controls.Add(Check("Common", "Enable", "Common.bAlgoEnable", "true"));
        profile.Controls.Add(Check("Common", "Required", "Common.IsRequired", "false"));
        profile.Controls.Add(Check("Common", "Algorithm Group", "Common.bAlgoGroup", "false"));
        profile.Controls.Add(Combo("Common", "Light Type", "Common.LightTypeNum", "0", ["None", "Red", "Green", "Blue", "White", "Free"]));
        profile.Controls.Add(Number("Common", "Red Light", "Common.RedValue", "0"));
        profile.Controls.Add(Number("Common", "Green Light", "Common.GreenValue", "0"));
        profile.Controls.Add(Number("Common", "Blue Light", "Common.BlueValue", "0"));
        profile.Controls.Add(Number("Common", "White Light", "Common.WhiteValue", "0"));
        profile.Controls.Add(Number("Common", "Manual Defect Code", "Common.ManualDefectCode", "0"));
        profile.Controls.Add(Number("Common", "Manual Sub Defect Code", "Common.ManualSubDefectCode", "0"));
    }

    private static void AddByFamily(AlgorithmReferenceUiProfile profile, AlgorithmCatalogItem catalog)
    {
        var family = catalog.ParameterFamily;
        var type = catalog.Type;

        if (IsBinary(type, family))
        {
            AddBinary(profile, family);
            AddIpc(profile, family);
            return;
        }

        if (IsBlob(type, family))
        {
            AddBinary(profile, family);
            AddBlob(profile, family);
            AddHeight(profile, family);
            AddShift(profile, family);
            AddAnchor(profile, family);
            AddIpc(profile, family);
            return;
        }

        if (IsEdgeMeasure(type, family))
        {
            AddBinary(profile, family);
            AddEdge(profile, family);
            AddMeasure(profile, family);
            AddAnchor(profile, family);
            return;
        }

        if (IsColor(type, family))
        {
            AddColor(profile, family);
            AddArea(profile, family);
            return;
        }

        if (IsHeight(type, family))
        {
            AddHeight(profile, family);
            AddArea(profile, family);
            AddShift(profile, family);
            return;
        }

        if (IsPattern(type, family))
        {
            AddPattern(profile, family);
            AddBinary(profile, family);
            AddAnchor(profile, family);
            return;
        }

        if (IsLeadOrSolder(type, family))
        {
            AddBinary(profile, family);
            AddHeight(profile, family);
            AddMeasure(profile, family);
            AddShift(profile, family);
            AddIpc(profile, family);
            return;
        }

        AddBinary(profile, family);
        AddArea(profile, family);
        AddMeasure(profile, family);
    }


    private static void AddTypeSpecific(AlgorithmReferenceUiProfile profile, AlgorithmCatalogItem catalog)
    {
        var family = catalog.ParameterFamily;
        switch (catalog.Type)
        {
            case "AlgoBGA":
            case "AlgoLQBGA":
                AddBga(profile, family);
                break;
            case "AlgoNGBlob":
                AddNgBlob(profile, family);
                break;
            case "AlgoShapeX":
                AddShapeX(profile, family);
                break;
            case "AlgoLead_Solder":
                AddLeadSolder(profile, family);
                break;
            case "AlgoPadBW":
                AddPadBw(profile, family);
                break;
            case "AlgoBW":
                AddBlackWhite(profile, family);
                break;
            case "AlgoAlignEdge":
                AddAlignEdge(profile, family);
                break;
            case "AlgoBarcode":
                AddBarcode(profile, family);
                break;
            case "AlgoOCR":
            case "AlgoPOCR":
                AddOcr(profile, family);
                break;
            case "AlgoPattern":
            case "AlgoPatternDiff":
                AddPatternAdvanced(profile, family);
                break;
            case "AlgoColor":
                AddColorAdvanced(profile, family);
                break;
            case "AlgoColorXY":
                AddColorAdvanced(profile, family);
                AddColorXy(profile, family);
                break;
            case "AlgoDistance":
            case "AlgoEdgePoint":
                AddDistanceAdvanced(profile, family);
                break;
            case "AlgoBlob":
            case "AlgoBody_Blob":
            case "AlgoBump":
                AddBlobAdvanced(profile, family);
                break;
            case "AlgoGrid":
                AddGrid(profile, family);
                break;
            case "AlgoTab":
            case "AlgoTab_Search":
                AddTab(profile, family);
                break;
            case "AlgoWidth":
                AddWidth(profile, family);
                break;
            case "AlgoVolume":
                AddVolume(profile, family);
                break;
            case "AlgoHeight_Mean":
                AddHeightMean(profile, family);
                break;
            case "AlgoHeight_Diff":
                AddHeightDiff(profile, family);
                break;
            case "AlgoGray_Mean":
                AddGrayMean(profile, family);
                break;
            case "AlgoGray_Diff":
                AddGrayDiff(profile, family);
                break;
            case "AlgoBridge":
                AddBridge(profile, family);
                break;
            case "AlgoFillet":
                AddFilletFoot(profile, family, includeFootFields: false);
                break;
            case "AlgoFoot":
                AddFilletFoot(profile, family, includeFootFields: true);
                break;
            case "AlgoLead_Tip":
                AddLeadTip(profile, family);
                break;
            case "AlgoLead_Lift":
                AddLeadLift(profile, family);
                break;
            case "AlgoLead_Color":
                AddLeadColor(profile, family);
                break;
            case "AlgoLead_SideSolder":
                AddLeadSideSolder(profile, family);
                break;
            case "AlgoLead_Search":
                AddLeadSearch(profile, family);
                break;
            case "AlgoGWire":
            case "AlgoWire":
                AddWire(profile, family);
                break;
            case "AlgoPadArray":
                AddPadArray(profile, family);
                break;
            case "AlgoPadAlign":
                AddPadAlign(profile, family);
                break;
            case "AlgoColorBand_Search":
                AddColorBandSearch(profile, family);
                break;
            case "AlgoForeignOCV":
                AddOcv(profile, family);
                break;
            case "AlgoBodyEdge":
                AddBodyEdge(profile, family);
                break;
            case "AlgoTilt":
                AddTilt(profile, family);
                break;
            case "AlgoSolderCone":
                AddSolderCone(profile, family);
                break;
        }
    }


    private static void AddRoiMask(AlgorithmReferenceUiProfile profile)
    {
        profile.Controls.Add(Check("ROI / Mask", "Use Algorithm ROI", "ROI.UseAlgorithmRoi", "false"));
        profile.Controls.Add(Check("ROI / Mask", "Use Mask", "Mask.UseMask", "false"));
        profile.Controls.Add(Check("ROI / Mask", "Use Polygon", "Mask.UsePolygon", "false"));
        profile.Controls.Add(Number("ROI / Mask", "Except ROI Count", "Mask.ExceptRoiCount", "0"));
    }

    private static void AddCommands(AlgorithmReferenceUiProfile profile, AlgorithmCatalogItem catalog)
    {
        var family = catalog.ParameterFamily;
        profile.Controls.Add(Command("Events", "Teach", $"{family}.TeachRequested"));
        profile.Controls.Add(Command("Events", "Search", $"{family}.SearchRequested"));
        profile.Controls.Add(Command("Events", "Apply All", "Command.ApplyAllTarget"));
        profile.Controls.Add(Command("Events", "Apply Select", "Command.ApplySelectTarget"));
        profile.Controls.Add(Command("Events", "Color Option", $"{family}.ColorOptionRequested"));
        profile.Controls.Add(Command("Events", "Fill Hole Option", $"{family}.FillHoleOptionRequested"));
        profile.Controls.Add(Command("Events", "New Model", $"{family}.NewModelRequested"));
        profile.Controls.Add(Command("Events", "Add Model", $"{family}.AddModelRequested"));
    }

    private static string ResolveSourceControl(AlgorithmCatalogItem catalog)
    {
        return catalog.Type switch
        {
            "AlgoBW" => "ucBlackAndWhite",
            "AlgoBlob" => "ucBlob",
            "AlgoBody_Blob" => "ucBodyBlob",
            "AlgoAlignEdge" => "ucAlignEdge",
            "AlgoDistance" or "AlgoEdgePoint" => "ucDistance",
            "AlgoColor" => "ucColor",
            "AlgoColorXY" => "ucColorXY",
            "AlgoColorBand_Search" => "ucColorBandSearch",
            "AlgoOCR" or "AlgoPOCR" => "ucOCR",
            "AlgoBarcode" => "ucBarcode",
            "AlgoPattern" => "ucPattern",
            "AlgoPatternDiff" => "ucPatternDiff",
            "AlgoForeignOCV" => "ucOCV",
            "AlgoBGA" or "AlgoLQBGA" => "ucBGA",
            "AlgoBump" => "ucBump",
            "AlgoNGBlob" => "ucNGBlob",
            "AlgoPadBW" => "ucPadBW",
            "AlgoPadAlign" => "ucPadAlign",
            "AlgoPadArray" => "ucPadArray",
            "AlgoGrid" => "ucGrid",
            "AlgoLine" => "ucLine",
            "AlgoEdge" => "ucEdge",
            "AlgoTilt" => "ucTilt",
            "AlgoWidth" => "ucLength",
            "AlgoVolume" => "ucVolume",
            "AlgoBridge" => "ucBridge",
            "AlgoSolderCone" => "ucSolderCone",
            "AlgoTab" => "ucTab",
            "AlgoTab_Search" => "ucTabSearch",
            "AlgoFillet" => "ucFillet",
            "AlgoFoot" => "ucFoot",
            "AlgoGWire" or "AlgoWire" => "ucGWire",
            "AlgoBodyEdge" => "ucBodyEdge",
            _ => $"uc{catalog.DisplayName.Replace(" ", "")}"
        };
    }

    private static bool IsBinary(string type, string family) => family.Contains("BlackWhite", StringComparison.OrdinalIgnoreCase) || family.Contains("PadBW", StringComparison.OrdinalIgnoreCase) || type is "AlgoBW" or "AlgoPadBW";
    private static bool IsBlob(string type, string family) => family.Contains("Blob", StringComparison.OrdinalIgnoreCase) || type.Contains("Blob", StringComparison.OrdinalIgnoreCase) || type is "AlgoBump" or "AlgoBGA" or "AlgoLQBGA";
    private static bool IsEdgeMeasure(string type, string family) => family.Contains("Edge", StringComparison.OrdinalIgnoreCase) || family.Contains("Distance", StringComparison.OrdinalIgnoreCase) || family.Contains("Length", StringComparison.OrdinalIgnoreCase) || type is "AlgoLine" or "AlgoGrid" or "AlgoWidth" or "AlgoPadAlign";
    private static bool IsColor(string type, string family) => family.Contains("Color", StringComparison.OrdinalIgnoreCase) || type.Contains("Color", StringComparison.OrdinalIgnoreCase);
    private static bool IsHeight(string type, string family) => family.Contains("Height", StringComparison.OrdinalIgnoreCase) || type is "AlgoVolume" or "AlgoTilt";
    private static bool IsPattern(string type, string family) => family.Contains("Pattern", StringComparison.OrdinalIgnoreCase) || family.Contains("OCR", StringComparison.OrdinalIgnoreCase) || family.Contains("BarCode", StringComparison.OrdinalIgnoreCase) || type is "AlgoPOCR";
    private static bool IsLeadOrSolder(string type, string family) => family.Contains("Lead", StringComparison.OrdinalIgnoreCase) || family.Contains("Solder", StringComparison.OrdinalIgnoreCase) || type is "AlgoBridge" or "AlgoFillet" or "AlgoFoot";

    private static AlgorithmReferenceControl Check(string tab, string label, string key, string defaultValue) => new(tab, AlgorithmReferenceControlKind.Check, label, key, defaultValue);
    private static AlgorithmReferenceControl Number(string tab, string label, string key, string defaultValue) => new(tab, AlgorithmReferenceControlKind.Number, label, key, defaultValue);
    private static AlgorithmReferenceControl Slider(string tab, string label, string key, string defaultValue, int min, int max) => new(tab, AlgorithmReferenceControlKind.Slider, label, key, defaultValue, min, max);
    private static AlgorithmReferenceControl Combo(string tab, string label, string key, string defaultValue, string[] options) => new(tab, AlgorithmReferenceControlKind.Combo, label, key, defaultValue, Options: options);
    private static AlgorithmReferenceControl Command(string tab, string label, string key) => new(tab, AlgorithmReferenceControlKind.Command, label, key, "");
}
