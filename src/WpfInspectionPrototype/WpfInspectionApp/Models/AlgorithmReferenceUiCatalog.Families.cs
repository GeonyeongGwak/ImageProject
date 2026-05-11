namespace WpfInspectionApp.Models;

public static partial class AlgorithmReferenceUiCatalog
{
    private static void AddBinary(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("2D/3D", "Use 2D", $"{family}.Use2D", "true"));
        profile.Controls.Add(Combo("2D/3D", "2D Range", $"{family}.Range2DType", "0", ["Inside", "Outside", "Upper", "Lower"]));
        profile.Controls.Add(Slider("2D/3D", "2D Min", $"{family}.MinValue", "0", 0, 255));
        profile.Controls.Add(Slider("2D/3D", "2D Max", $"{family}.MaxValue", "255", 0, 255));
        profile.Controls.Add(Check("2D/3D", "Use 3D", $"{family}.Use3D", "false"));
        profile.Controls.Add(Combo("2D/3D", "3D Range", $"{family}.Range3DType", "0", ["Inside", "Outside", "Upper", "Lower"]));
        profile.Controls.Add(Number("2D/3D", "3D Min", $"{family}.HeightMin", "0"));
        profile.Controls.Add(Number("2D/3D", "3D Max", $"{family}.HeightMax", "100"));
        profile.Controls.Add(Check("2D/3D", "Use Filter", $"{family}.UseFilter", "false"));
        profile.Controls.Add(Combo("2D/3D", "Filter", $"{family}.FilterIndex", "0", ["None", "Sobel", "Median", "Morphology"]));
    }

    private static void AddBlob(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("Blob", "Use Area", $"{family}.UseArea", "true"));
        profile.Controls.Add(Number("Blob", "Area Min", $"{family}.AreaMin", "10"));
        profile.Controls.Add(Number("Blob", "Area Max", $"{family}.AreaMax", "999999"));
        profile.Controls.Add(Check("Blob", "Use Width", $"{family}.UseWidth", "false"));
        profile.Controls.Add(Number("Blob", "Width Min", $"{family}.WidthMin", "0"));
        profile.Controls.Add(Number("Blob", "Width Max", $"{family}.WidthMax", "9999"));
        profile.Controls.Add(Check("Blob", "Use Length", $"{family}.UseLength", "false"));
        profile.Controls.Add(Number("Blob", "Length Min", $"{family}.LengthMin", "0"));
        profile.Controls.Add(Number("Blob", "Length Max", $"{family}.LengthMax", "9999"));
        profile.Controls.Add(Check("Blob", "Fill Hole", $"{family}.FillHole", "false"));
        profile.Controls.Add(Check("Blob", "Use Outline", $"{family}.UseOutline", "false"));
        profile.Controls.Add(Check("Blob", "Search Blob Size", $"{family}.SearchBlobSize", "false"));
        profile.Controls.Add(Number("Blob", "Blob Size Width", $"{family}.BlobSizeWidth", "0"));
        profile.Controls.Add(Number("Blob", "Blob Size Length", $"{family}.BlobSizeLength", "0"));
        profile.Controls.Add(Number("Blob", "Blob Size Distance", $"{family}.BlobSizeDistance", "0"));
    }

    private static void AddHeight(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("Height", "Use Height", $"{family}.UseHeight", "false"));
        profile.Controls.Add(Number("Height", "Height Min", $"{family}.MinHeight", "0"));
        profile.Controls.Add(Number("Height", "Height Max", $"{family}.MaxHeight", "100"));
        profile.Controls.Add(Number("Height", "Target Height", $"{family}.TargetHeight", "0"));
        profile.Controls.Add(Number("Height", "Height Min Rate", $"{family}.MinHeightRate", "0"));
        profile.Controls.Add(Number("Height", "Height Max Rate", $"{family}.MaxHeightRate", "100"));
        profile.Controls.Add(Check("Height", "Calc Height Avg Mode", $"{family}.CalcHeightAvgMode", "false"));
    }

    private static void AddArea(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("Area", "Use Area", $"{family}.UseArea", "false"));
        profile.Controls.Add(Number("Area", "Area Min", $"{family}.AreaMin", "0"));
        profile.Controls.Add(Number("Area", "Area Max", $"{family}.AreaMax", "999999"));
        profile.Controls.Add(Number("Area", "Percent OK", $"{family}.PercentOK", "50"));
    }

    private static void AddEdge(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Combo("Edge", "Select Area", $"{family}.SelectedArea", "0", ["Area 1", "Area 2", "Area 3"]));
        profile.Controls.Add(Combo("Edge", "Select Type", $"{family}.SelectedType", "0", ["Edge", "Center Gravity", "ROI"]));
        profile.Controls.Add(Combo("Edge", "Select Line", $"{family}.SelectedLine", "0", ["Line 1", "Line 2", "Line 3", "Line 4"]));
        profile.Controls.Add(Number("Edge", "Line Count", $"{family}.LineCount", "1"));
        profile.Controls.Add(Number("Edge", "Area Size X", $"{family}.AreaSizeX", "40"));
        profile.Controls.Add(Number("Edge", "Area Size Y", $"{family}.AreaSizeY", "40"));
        profile.Controls.Add(Check("Edge", "Line All View", $"{family}.LineAllView", "true"));
        profile.Controls.Add(Combo("Edge", "Direction", $"{family}.Direction", "0", ["Horizontal", "Vertical"]));
        profile.Controls.Add(Combo("Edge", "Measure Direction", $"{family}.MeasureDirection", "1", ["Left", "Center", "Right"]));
    }

    private static void AddMeasure(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("Measure", "Use Angle", $"{family}.UseAngle", "false"));
        profile.Controls.Add(Number("Measure", "Teach Angle", $"{family}.TeachAngle", "0"));
        profile.Controls.Add(Number("Measure", "Angle Condition", $"{family}.AngleCondition", "0"));
        profile.Controls.Add(Check("Measure", "Use Distance", $"{family}.UseDistance", "false"));
        profile.Controls.Add(Number("Measure", "Distance", $"{family}.Distance", "0"));
        profile.Controls.Add(Check("Measure", "Distance Min", $"{family}.UseDistanceMin", "false"));
        profile.Controls.Add(Number("Measure", "Distance Min Value", $"{family}.DistanceMin", "0"));
        profile.Controls.Add(Check("Measure", "Distance Max", $"{family}.UseDistanceMax", "false"));
        profile.Controls.Add(Number("Measure", "Distance Max Value", $"{family}.DistanceMax", "0"));
        profile.Controls.Add(Check("Measure", "Use Distance X", $"{family}.UseDistanceX", "false"));
        profile.Controls.Add(Number("Measure", "Distance X", $"{family}.DistanceX", "0"));
        profile.Controls.Add(Check("Measure", "Use Distance Y", $"{family}.UseDistanceY", "false"));
        profile.Controls.Add(Number("Measure", "Distance Y", $"{family}.DistanceY", "0"));
    }

    private static void AddShift(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("Shift", "Use Shift", $"{family}.UseShift", "false"));
        profile.Controls.Add(Check("Shift", "Use Shift X", $"{family}.UseShiftX", "true"));
        profile.Controls.Add(Number("Shift", "Shift X", $"{family}.ShiftX", "0"));
        profile.Controls.Add(Check("Shift", "Use Shift Y", $"{family}.UseShiftY", "true"));
        profile.Controls.Add(Number("Shift", "Shift Y", $"{family}.ShiftY", "0"));
        profile.Controls.Add(Check("Shift", "Use Offset Distance", $"{family}.UseOffsetDistance", "false"));
    }

    private static void AddColor(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("Color", "Use Color", $"{family}.UseColor", "true"));
        profile.Controls.Add(Slider("Color", "R Min", $"{family}.MinR", "0", 0, 255));
        profile.Controls.Add(Slider("Color", "R Max", $"{family}.MaxR", "255", 0, 255));
        profile.Controls.Add(Slider("Color", "G Min", $"{family}.MinG", "0", 0, 255));
        profile.Controls.Add(Slider("Color", "G Max", $"{family}.MaxG", "255", 0, 255));
        profile.Controls.Add(Slider("Color", "B Min", $"{family}.MinB", "0", 0, 255));
        profile.Controls.Add(Slider("Color", "B Max", $"{family}.MaxB", "255", 0, 255));
        profile.Controls.Add(Check("Color", "Angle Color Option", $"{family}.UseAngleColor", "false"));
    }

    private static void AddPattern(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Number("Pattern", "Match Score", $"{family}.MatchScore", "80"));
        profile.Controls.Add(Number("Pattern", "Search Margin", $"{family}.SearchMargin", "20"));
        profile.Controls.Add(Check("Pattern", "Use Rotation", $"{family}.UseRotation", "false"));
        profile.Controls.Add(Check("Pattern", "Save Pattern", $"{family}.SavePattern", "false"));
        profile.Controls.Add(Check("Pattern", "Use OCV/OCR DB", $"{family}.UseDatabase", "false"));
        profile.Controls.Add(Number("Pattern", "Model Count", $"{family}.ModelCount", "0"));
    }

    private static void AddAnchor(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("Anchor", "Use Anchor", $"{family}.UseAnchor", "false"));
        profile.Controls.Add(Number("Anchor", "Align Window Index", $"{family}.AlignWindowIndex", "0"));
        profile.Controls.Add(Check("Anchor", "Use Two Anchor", $"{family}.UseTwoAnchor", "false"));
        profile.Controls.Add(Check("Anchor", "3 Point Distance", $"{family}.Use3PtDistance", "false"));
        profile.Controls.Add(Number("Anchor", "1-3 Distance", $"{family}.Teach1_3PtDis", "0"));
        profile.Controls.Add(Number("Anchor", "1-3 Distance Min", $"{family}.Teach1_3PtDisMin", "0"));
        profile.Controls.Add(Number("Anchor", "1-3 Distance Max", $"{family}.Teach1_3PtDisMax", "0"));
    }

    private static void AddIpc(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("IPC", "Use IPC", $"{family}.UseIPC", "false"));
        profile.Controls.Add(Combo("IPC", "IPC Class", $"{family}.IpcClass", "1", ["Class1", "Class2", "Class3"]));
        profile.Controls.Add(Number("IPC", "IPC Percent", $"{family}.IpcPercent", "50"));
    }
}
