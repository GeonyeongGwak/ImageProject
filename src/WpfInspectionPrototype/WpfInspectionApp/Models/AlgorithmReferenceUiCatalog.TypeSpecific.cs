namespace WpfInspectionApp.Models;

public static partial class AlgorithmReferenceUiCatalog
{
    private static void AddBga(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("BGA", "Use Model", $"{family}.UseModel", "false"));
        profile.Controls.Add(Check("BGA", "Use Model Matching", $"{family}.UseModelMatching", "false"));
        profile.Controls.Add(Check("BGA", "Model Visible", $"{family}.ModelVisible", "true"));
        profile.Controls.Add(Command("BGA", "Create Pad Mask", $"{family}.CreatePadMaskRequested"));
        profile.Controls.Add(Command("BGA", "Save BGA Pad Mask", $"{family}.SavePadMaskRequested"));
        profile.Controls.Add(Command("BGA", "Bump Search", $"{family}.BumpSearchRequested"));
        profile.Controls.Add(Check("BGA", "Use Pitch", $"{family}.UsePitch", "false"));
        profile.Controls.Add(Number("BGA", "Pitch", $"{family}.Pitch", "0"));
        profile.Controls.Add(Number("BGA", "Min Pitch", $"{family}.MinPitch", "0"));
        profile.Controls.Add(Check("BGA", "Use Circle", $"{family}.UseCircle", "false"));
        profile.Controls.Add(Number("BGA", "Circle Rate", $"{family}.CircleRate", "0"));
        profile.Controls.Add(Check("BGA", "Use Bridge", $"{family}.UseBridge", "false"));
        profile.Controls.Add(Check("BGA", "Use Volume", $"{family}.UseVolume", "false"));
        profile.Controls.Add(Number("BGA", "Volume Min", $"{family}.VolumeMin", "0"));
        profile.Controls.Add(Number("BGA", "Volume Max", $"{family}.VolumeMax", "0"));
        profile.Controls.Add(Check("BGA", "Use Coplanarity", $"{family}.UseCoplanarity", "false"));
        profile.Controls.Add(Number("BGA", "Coplanarity Min", $"{family}.CoplanarityMin", "0"));
        profile.Controls.Add(Number("BGA", "Coplanarity Max", $"{family}.CoplanarityMax", "0"));
        profile.Controls.Add(Check("BGA", "Use Flux", $"{family}.UseFlux", "false"));
        profile.Controls.Add(Command("BGA", "Teach Flux Input", $"{family}.TeachFluxInputRequested"));
        profile.Controls.Add(Combo("BGA", "Inspection Type", $"{family}.InspType", "0", ["Bump", "Pad", "Bridge", "Flux"]));
        profile.Controls.Add(Combo("BGA", "View Mode", $"{family}.ViewMode", "0", ["All", "NG", "OK"]));
        profile.Controls.Add(Command("BGA", "Create Bump Model", $"{family}.CreateModelRequested"));
        profile.Controls.Add(Command("BGA", "Delete Bump Model", $"{family}.DeleteModelRequested"));
        profile.Controls.Add(Command("BGA", "Bump Size Apply", $"{family}.BumpSizeApplyRequested"));
        profile.Controls.Add(Command("BGA", "Sort Bump", $"{family}.SortBumpRequested"));
        profile.Controls.Add(Command("BGA", "Gerber", $"{family}.GerberRequested"));
        profile.Controls.Add(Command("BGA Base ROI", "Add Base ROI", $"{family}.AddBaseRoiRequested"));
        profile.Controls.Add(Command("BGA Base ROI", "Delete Base ROI", $"{family}.DeleteBaseRoiRequested"));
        profile.Controls.Add(Combo("BGA Base ROI", "Base ROI", $"{family}.BaseRoi", "0", ["ROI 1", "ROI 2", "ROI 3"]));
        profile.Controls.Add(Combo("BGA Base ROI", "Target 1", $"{family}.Target1", "0", ["Body", "Pad", "Ball"]));
        profile.Controls.Add(Combo("BGA Base ROI", "Target 2", $"{family}.Target2", "0", ["Body", "Pad", "Ball"]));
        profile.Controls.Add(Check("BGA Advanced", "Use Absolute Height", $"{family}.UseAbsoluteHeight", "false"));
        profile.Controls.Add(Check("BGA Advanced", "Use Zone Based Area", $"{family}.UseZoneBasedArea", "false"));
        profile.Controls.Add(Check("BGA Advanced", "Use Anomaly Filter", $"{family}.UseAnomalyFilter", "false"));
        profile.Controls.Add(Check("BGA Advanced", "Anomaly Width", $"{family}.AnomalyWidth", "false"));
        profile.Controls.Add(Check("BGA Advanced", "Anomaly Length", $"{family}.AnomalyLength", "false"));
        profile.Controls.Add(Number("BGA Advanced", "Anomaly Area", $"{family}.AnomalyArea", "0"));
    }

    private static void AddAlignEdge(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Combo("AlignEdge", "Direction", $"{family}.Direction", "0", ["LeftToRight", "RightToLeft", "TopToBottom", "BottomToTop"]));
        profile.Controls.Add(Slider("AlignEdge", "Threshold", $"{family}.Threshold", "128", 0, 255));
        profile.Controls.Add(Number("AlignEdge", "Search Width", $"{family}.SearchWidth", "40"));
        profile.Controls.Add(Number("AlignEdge", "Search Height", $"{family}.SearchHeight", "12"));
        profile.Controls.Add(Check("AlignEdge", "Use Peak Edge", $"{family}.UsePeak", "true"));
        profile.Controls.Add(Check("AlignEdge", "Use Sub Pixel", $"{family}.UseSubPixel", "true"));

        profile.Controls.Add(Check("AlignEdge Range", "Use 2D Range", $"{family}.Use2D", "true"));
        profile.Controls.Add(Number("AlignEdge Range", "2D Min", $"{family}.Min2D", "0"));
        profile.Controls.Add(Number("AlignEdge Range", "2D Max", $"{family}.Max2D", "255"));
        profile.Controls.Add(Check("AlignEdge Range", "Use 3D Range", $"{family}.Use3D", "false"));
        profile.Controls.Add(Number("AlignEdge Range", "3D Min", $"{family}.Min3D", "0"));
        profile.Controls.Add(Number("AlignEdge Range", "3D Max", $"{family}.Max3D", "1000"));

        profile.Controls.Add(Check("AlignEdge Measure", "Use Distance", $"{family}.UseDistance", "true"));
        profile.Controls.Add(Number("AlignEdge Measure", "Distance Spec", $"{family}.DistanceSpec", "0"));
        profile.Controls.Add(Number("AlignEdge Measure", "Distance Tol", $"{family}.DistanceTolerance", "10"));
        profile.Controls.Add(Check("AlignEdge Measure", "Use Angle", $"{family}.UseAngle", "true"));
        profile.Controls.Add(Number("AlignEdge Measure", "Angle Spec", $"{family}.AngleSpec", "0"));
        profile.Controls.Add(Number("AlignEdge Measure", "Angle Tol", $"{family}.AngleTolerance", "5"));
        profile.Controls.Add(Check("AlignEdge Measure", "Use Shift", $"{family}.UseShift", "true"));
        profile.Controls.Add(Number("AlignEdge Measure", "Shift X", $"{family}.ShiftX", "0"));
        profile.Controls.Add(Number("AlignEdge Measure", "Shift Y", $"{family}.ShiftY", "0"));

        profile.Controls.Add(Check("AlignEdge Anchor", "Use Anchor", $"{family}.UseAnchor", "false"));
        profile.Controls.Add(Combo("AlignEdge Anchor", "Anchor Mode", $"{family}.AnchorMode", "0", ["Window ROI", "Algorithm ROI", "Last Align"]));
        profile.Controls.Add(Number("AlignEdge Anchor", "Anchor X", $"{family}.AnchorX", "0"));
        profile.Controls.Add(Number("AlignEdge Anchor", "Anchor Y", $"{family}.AnchorY", "0"));
        profile.Controls.Add(Command("AlignEdge Anchor", "Capture Anchor", $"{family}.CaptureAnchorRequested"));
    }

    private static void AddNgBlob(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Combo("NG Blob", "Inspection Type", $"{family}.InspType", "0", ["NG", "Scratch", "Warpage", "Pattern"]));
        profile.Controls.Add(Combo("NG Blob", "Target Type", $"{family}.TargetType", "0", ["Body", "Lead", "Pad", "Package"]));
        profile.Controls.Add(Check("NG Blob", "Use Except Pattern", $"{family}.UseExceptPattern", "false"));
        profile.Controls.Add(Command("NG Blob", "Create Pattern Model", $"{family}.PatternCreateModelRequested"));
        profile.Controls.Add(Command("NG Blob", "Pattern Previous", $"{family}.PatternPrevRequested"));
        profile.Controls.Add(Command("NG Blob", "Pattern Next", $"{family}.PatternNextRequested"));
        profile.Controls.Add(Number("NG Blob", "Pattern Score", $"{family}.PatternScore", "80"));
        profile.Controls.Add(Check("NG Blob", "Coplanarity", $"{family}.UseCoplanarity", "false"));
        profile.Controls.Add(Command("NG Blob", "Coplanarity", $"{family}.CoplanarityRequested"));
        profile.Controls.Add(Check("NG Blob", "Warpage Deviation", $"{family}.UseWarpageDeviation", "false"));
        profile.Controls.Add(Number("NG Blob", "Warpage Deviation", $"{family}.WarpageDeviation", "0"));
        profile.Controls.Add(Check("NG Blob", "Detail Warpage", $"{family}.UseDetailWarpageDeviation", "false"));
        profile.Controls.Add(Number("NG Blob", "Detail Warpage", $"{family}.DetailWarpageDeviation", "0"));
        profile.Controls.Add(Check("NG Blob", "Use Histogram", $"{family}.UseHistogram", "false"));
        profile.Controls.Add(Number("NG Blob", "Limit Min", $"{family}.LimitMin", "0"));
        profile.Controls.Add(Number("NG Blob", "Limit Max", $"{family}.LimitMax", "255"));
        profile.Controls.Add(Check("NG Blob", "Area Filter", $"{family}.UseAreaFilter", "false"));
        profile.Controls.Add(Number("NG Blob", "Min Contrast", $"{family}.MinContrast", "0"));
        profile.Controls.Add(Check("NG Blob", "Clustering", $"{family}.UseClustering", "false"));
        profile.Controls.Add(Number("NG Blob", "Clustering Pitch", $"{family}.ClusteringPitch", "0"));
    }

    private static void AddShapeX(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("ShapeX", "Use ShapeX Area", $"{family}.UseShapeXArea", "false"));
        profile.Controls.Add(Number("ShapeX", "Shape Area", $"{family}.ShapeArea", "0"));
        profile.Controls.Add(Number("ShapeX", "Inner Area", $"{family}.InnerArea", "0"));
        profile.Controls.Add(Check("ShapeX", "Exist Area", $"{family}.UseExistArea", "false"));
        profile.Controls.Add(Number("ShapeX", "Exist Area", $"{family}.ExistArea", "0"));
        profile.Controls.Add(Command("ShapeX", "Shape Area All Set", $"{family}.ShapeAreaAllSetRequested"));
        profile.Controls.Add(Command("ShapeX", "Inner Area All Set", $"{family}.InnerAreaAllSetRequested"));
        profile.Controls.Add(Command("ShapeX", "Exist Area All Set", $"{family}.ExistAreaAllSetRequested"));
        profile.Controls.Add(Check("ShapeX", "Scar Aspect Ratio", $"{family}.UseScarAspectRatio", "false"));
        profile.Controls.Add(Number("ShapeX", "Scar Aspect Ratio", $"{family}.ScarAspectRatio", "0"));
        profile.Controls.Add(Check("ShapeX", "Min Scar Thickness", $"{family}.UseMinScarThickness", "false"));
        profile.Controls.Add(Number("ShapeX", "Min Scar Thickness", $"{family}.MinScarThickness", "0"));
        profile.Controls.Add(Check("ShapeX", "Cross Line Detect", $"{family}.UseCrossLineDetect", "false"));
        profile.Controls.Add(Check("ShapeX", "Chipping Critical Line", $"{family}.UseChippingCriticalLine", "false"));
        profile.Controls.Add(Number("ShapeX", "Chipping Max Length", $"{family}.ChippingMaxLength", "0"));
        profile.Controls.Add(Check("ShapeX", "Tie Bar R Opt", $"{family}.TieBarROpt", "false"));
        profile.Controls.Add(Number("ShapeX", "Tiebar Rate", $"{family}.TiebarRate", "0"));
        profile.Controls.Add(Command("ShapeX", "Create Model", $"{family}.CreateModelRequested"));
        profile.Controls.Add(Command("ShapeX", "Clone ROI", $"{family}.CloneRoiRequested"));
        profile.Controls.Add(Command("ShapeX", "Delete All Model", $"{family}.DeleteAllModelRequested"));
        profile.Controls.Add(Combo("ShapeX", "Select Target", $"{family}.SelectTarget", "0", ["All", "Shape", "NG", "Foreign"]));
        profile.Controls.Add(Combo("ShapeX", "Select Blob", $"{family}.SelectBlob", "0", ["Largest", "Nearest", "All"]));
        profile.Controls.Add(Command("ShapeX", "Select All", $"{family}.SelectAllRequested"));
        profile.Controls.Add(Command("ShapeX", "Apply Same", $"{family}.ApplySameRequested"));
        profile.Controls.Add(Command("ShapeX", "Apply Spec", $"{family}.ApplySpecRequested"));
        profile.Controls.Add(Command("ShapeX", "Sorting Run", $"{family}.SortingRunRequested"));
        profile.Controls.Add(Combo("ShapeX", "Sorting Order", $"{family}.SortingOrder", "0", ["None", "Area", "Length", "Width"]));
        profile.Controls.Add(Check("ShapeX Detail", "Disable ShapeX Area", $"{family}.DisableShapeXArea", "false"));
        profile.Controls.Add(Check("ShapeX Detail", "Erase Scar Area", $"{family}.EraseScarArea", "false"));
        profile.Controls.Add(Check("ShapeX Detail", "Exist Shape", $"{family}.ExistShape", "false"));
        profile.Controls.Add(Check("ShapeX Detail", "Line Chipping", $"{family}.LineChipping", "false"));
        profile.Controls.Add(Check("ShapeX Detail", "Include Side", $"{family}.IncludeSide", "false"));
        profile.Controls.Add(Check("ShapeX Detail", "Local Length", $"{family}.UseLocalLength", "false"));
        profile.Controls.Add(Number("ShapeX Detail", "Local Length Min", $"{family}.LocalLengthMin", "0"));
        profile.Controls.Add(Number("ShapeX Detail", "Local Length Max", $"{family}.LocalLengthMax", "0"));
        profile.Controls.Add(Check("ShapeX Detail", "Dent", $"{family}.UseDent", "false"));
        profile.Controls.Add(Number("ShapeX Detail", "Dent Min", $"{family}.DentMin", "0"));
        profile.Controls.Add(Number("ShapeX Detail", "Dent Max", $"{family}.DentMax", "0"));
    }

    private static void AddLeadSolder(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Command("Lead Solder", "Set Position", $"{family}.SetPositionRequested"));
        profile.Controls.Add(Number("Lead Solder", "Current Position", $"{family}.CurrentPosition", "0"));
        profile.Controls.Add(Number("Lead Solder", "Lead Interval", $"{family}.LeadInterval", "0"));
        profile.Controls.Add(Number("Lead Solder", "Solder Interval", $"{family}.SolderInterval", "0"));
        profile.Controls.Add(Number("Lead Solder", "Solder Length", $"{family}.SolderLength", "0"));
        profile.Controls.Add(Check("Lead Solder", "Use BW", $"{family}.UseBW", "false"));
        profile.Controls.Add(Check("Lead Solder", "Use Height Diff", $"{family}.UseHeightDiff", "false"));
        profile.Controls.Add(Number("Lead Solder", "Current BW", $"{family}.CurrentValueBW", "0"));
        profile.Controls.Add(Check("Lead Solder", "Use Rect", $"{family}.UseRect", "false"));
        profile.Controls.Add(Number("Lead Solder", "Gap", $"{family}.Gap", "0"));
        profile.Controls.Add(Check("Lead Solder", "Use Mean", $"{family}.UseMean", "false"));
        profile.Controls.Add(Command("Lead Solder", "Avg Height 3D Teach", $"{family}.AvgHeight3DTeachRequested"));
        profile.Controls.Add(Number("Lead Solder", "Height Diff", $"{family}.HeightDiff", "0"));
        profile.Controls.Add(Number("Lead Solder", "Permissible Range", $"{family}.PermissibleRange", "0"));
        profile.Controls.Add(Check("Cold Joint", "Cold Joint", $"{family}.UseColdJoint", "false"));
        profile.Controls.Add(Number("Cold Joint", "CJ Area", $"{family}.ColdJointArea", "0"));
        profile.Controls.Add(Number("Cold Joint", "CJ Area Percent", $"{family}.ColdJointAreaPercent", "0"));
        profile.Controls.Add(Number("Cold Joint", "CJ Gap", $"{family}.ColdJointGap", "0"));
        profile.Controls.Add(Number("Cold Joint", "CJ Width", $"{family}.ColdJointWidth", "0"));
        profile.Controls.Add(Number("Cold Joint", "CJ Height", $"{family}.ColdJointHeight", "0"));
        profile.Controls.Add(Check("Angle Color", "Use Angle Color", $"{family}.UseAngleColor", "false"));
        profile.Controls.Add(Number("Angle Color", "AC Count", $"{family}.AngleColorCount", "0"));
        profile.Controls.Add(Number("Angle Color", "AC Pixel", $"{family}.AngleColorPixel", "0"));
        profile.Controls.Add(Number("Angle Color", "AC Gap", $"{family}.AngleColorGap", "0"));
        profile.Controls.Add(Combo("Angle Color", "AC Option 1", $"{family}.AngleColorOption1", "0", ["R", "G", "B", "Gray"]));
        profile.Controls.Add(Combo("Angle Color", "AC Option 2", $"{family}.AngleColorOption2", "0", ["R", "G", "B", "Gray"]));
        profile.Controls.Add(Command("Angle Color", "Angle Color Inspect", $"{family}.AngleColorInspectRequested"));
    }

    private static void AddPadBw(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Combo("Pad BW", "Select Light", $"{family}.SelectLight", "0", ["Main", "Sub", "Free"]));
        profile.Controls.Add(Command("Pad BW", "Add Light", $"{family}.AddLightRequested"));
        profile.Controls.Add(Command("Pad BW", "Sub Light", $"{family}.SubLightRequested"));
        profile.Controls.Add(Check("Pad BW", "Use Histogram", $"{family}.UseHistogram", "false"));
        profile.Controls.Add(Number("Pad BW", "Calc Green", $"{family}.CalcGreen", "0"));
        profile.Controls.Add(Number("Pad BW", "Calc Yellow", $"{family}.CalcYellow", "0"));
        profile.Controls.Add(Number("Pad BW", "Frequency", $"{family}.Frequency", "0"));
        profile.Controls.Add(Check("Pad BW", "NG Grouping", $"{family}.UseNgGrouping", "false"));
        profile.Controls.Add(Number("Pad BW", "NG Group Max Size", $"{family}.NgGroupingMaxSize", "0"));
        profile.Controls.Add(Number("Pad BW", "NG Group Distance", $"{family}.NgGroupingDistance", "0"));
        profile.Controls.Add(Check("Pad BW", "Use AI", $"{family}.UseAI", "false"));
        profile.Controls.Add(Combo("Pad BW", "AI Model", $"{family}.AIModel", "0", ["Model 1", "Model 2", "Model 3"]));
        profile.Controls.Add(Check("Pad BW", "Hole Align", $"{family}.HoleAlign", "false"));
        profile.Controls.Add(Check("Pad BW", "Shade Fix", $"{family}.ShadeFix", "false"));
    }

    private static void AddBlackWhite(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Number("Teaching Rate", "Teaching Area", $"{family}.TeachingArea", "0"));
        profile.Controls.Add(Number("Teaching Rate", "Teaching Area Pix", $"{family}.TeachingAreaPix", "0"));
        profile.Controls.Add(Check("Teaching Rate", "Use Teaching Rate", $"{family}.UseTeachingRate", "false"));
        profile.Controls.Add(Command("Teaching Rate", "Teaching Rate", $"{family}.TeachingRateRequested"));
        profile.Controls.Add(Check("BW Height", "Use Height Mean", $"{family}.UseHeightMean", "false"));
        profile.Controls.Add(Check("BW Height", "Height Mean Min", $"{family}.UseHeightMeanMin", "false"));
        profile.Controls.Add(Number("BW Height", "Height Mean Min um", $"{family}.HeightMeanMinUm", "0"));
        profile.Controls.Add(Check("BW Height", "Height Mean Max", $"{family}.UseHeightMeanMax", "false"));
        profile.Controls.Add(Number("BW Height", "Height Mean Max", $"{family}.HeightMeanMax", "0"));
        profile.Controls.Add(Check("BW AC", "Use Angle Color", $"{family}.UseAngleColor", "false"));
        profile.Controls.Add(Number("BW AC", "AC Min", $"{family}.AngleColorMin", "0"));
        profile.Controls.Add(Number("BW AC", "AC Max", $"{family}.AngleColorMax", "0"));
        profile.Controls.Add(Check("BW AC", "Cold Joint", $"{family}.UseColdJoint", "false"));
        profile.Controls.Add(Check("BW AI", "View AI", $"{family}.ViewAI", "false"));
        profile.Controls.Add(Check("BW AI", "Include Black Area", $"{family}.IncludeBlackArea", "false"));
    }

    private static void AddBarcode(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Combo("Barcode", "Barcode Type", $"{family}.BarcodeType", "0", ["Code128", "QR", "DataMatrix", "EAN"]));
        profile.Controls.Add(Combo("Barcode", "Inspection Type", $"{family}.InspBarcodeType", "0", ["Read", "Compare", "Quality"]));
        profile.Controls.Add(Check("Barcode", "Essential Words", $"{family}.UseEssentialWords", "false"));
        profile.Controls.Add(Number("Barcode", "Essential Position", $"{family}.EssentialPosition", "0"));
        profile.Controls.Add(Check("Barcode", "Partial Display", $"{family}.UsePartialDisplay", "false"));
        profile.Controls.Add(Number("Barcode", "Partial First", $"{family}.PartialFirst", "0"));
        profile.Controls.Add(Number("Barcode", "Partial Second", $"{family}.PartialSecond", "0"));
        profile.Controls.Add(Check("Barcode", "Flip", $"{family}.Flip", "false"));
        profile.Controls.Add(Combo("Barcode", "Checksum", $"{family}.Checksum", "0", ["None", "Auto", "Required"]));
        profile.Controls.Add(Combo("Barcode", "Retry", $"{family}.Retry", "0", ["0", "1", "2", "3"]));
        profile.Controls.Add(Command("Barcode", "Barcode Test", $"{family}.BarcodeTestRequested"));
        profile.Controls.Add(Check("Barcode", "Quality Check", $"{family}.QualityCheck", "false"));
        profile.Controls.Add(Check("Barcode", "Use Align", $"{family}.UseAlign", "false"));
        profile.Controls.Add(Command("Barcode", "Teach Align", $"{family}.TeachAlignRequested"));
        profile.Controls.Add(Check("Barcode", "PN Validation", $"{family}.PNValidation", "false"));
        profile.Controls.Add(Check("Barcode", "Split Image Save", $"{family}.SplitImageSave", "false"));
        profile.Controls.Add(Number("Barcode Legacy", "Barcode Type Flags", $"{family}.BarcodeTypeFlags", "0"));
        profile.Controls.Add(Number("Barcode Legacy", "Option Flags", $"{family}.OptionFlags", "0"));
        profile.Controls.Add(Number("Barcode Legacy", "Char Count Min", $"{family}.CharCountMin", "0"));
        profile.Controls.Add(Number("Barcode Legacy", "Char Count Max", $"{family}.CharCountMax", "0"));
        profile.Controls.Add(Number("Barcode Legacy", "Angle Tolerance", $"{family}.AngleTolerance", "0"));
        profile.Controls.Add(Number("Barcode Legacy", "Data Count", $"{family}.BarDataCount", "0"));
        profile.Controls.Add(Number("Barcode Legacy", "String Count", $"{family}.BarStringCount", "0"));
        profile.Controls.Add(Command("Barcode Items", "Save Item", $"{family}.SaveItemRequested"));
        profile.Controls.Add(Command("Barcode Items", "Clear Item", $"{family}.ClearItemRequested"));
        profile.Controls.Add(Command("Barcode Items", "Add Label Item", $"{family}.AddLabelItemRequested"));
        profile.Controls.Add(Check("Barcode Items", "Standard Words", $"{family}.UseStandardWords", "false"));
        profile.Controls.Add(Check("Barcode Items", "Use Recognition Word", $"{family}.UseRecognitionWord", "false"));
        profile.Controls.Add(Check("Barcode Items", "Use Target Word", $"{family}.UseTargetWord", "false"));
        profile.Controls.Add(Number("Barcode Quality", "2D Contrast", $"{family}.Contrast2D", "0"));
        profile.Controls.Add(Number("Barcode Quality", "2D Defect Area", $"{family}.DefectArea2D", "0"));
        profile.Controls.Add(Number("Barcode Quality", "2D Defect Count", $"{family}.DefectCount2D", "0"));
        profile.Controls.Add(Combo("Barcode Alter", "Alter Action", $"{family}.AlterAction", "0", ["None", "Alarm", "Edit Barcode", "PCB Number", "Inspection Input"]));
    }

    private static void AddOcr(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Slider("OCR", "Binarize", $"{family}.BinarizeValue", "128", 0, 255));
        profile.Controls.Add(Check("OCR", "Auto Threshold", $"{family}.AutoThreshold", "false"));
        profile.Controls.Add(Combo("OCR", "Polarity", $"{family}.Polarity", "0", ["Black", "White", "Mix"]));
        profile.Controls.Add(Check("OCR", "Use Char Score", $"{family}.UseCharScore", "false"));
        profile.Controls.Add(Number("OCR", "Char Score", $"{family}.CharScore", "80"));
        profile.Controls.Add(Check("OCR", "Remove Terminator", $"{family}.RemoveTerminator", "false"));
        profile.Controls.Add(Command("OCR", "Add Font", $"{family}.AddFontRequested"));
        profile.Controls.Add(Command("OCR", "Delete Font", $"{family}.DeleteFontRequested"));
        profile.Controls.Add(Command("OCR", "Read", $"{family}.ReadRequested"));
        profile.Controls.Add(Number("OCR", "Angle", $"{family}.Angle", "0"));
        profile.Controls.Add(Check("OCR Processing", "Use Stretching", $"{family}.UseStretching", "false"));
        profile.Controls.Add(Check("OCR Processing", "Use Erode", $"{family}.UseErode", "false"));
        profile.Controls.Add(Number("OCR Processing", "Erode Size", $"{family}.ErodeSize", "0"));
        profile.Controls.Add(Check("OCR Processing", "Use Dilate", $"{family}.UseDilate", "false"));
        profile.Controls.Add(Number("OCR Processing", "Dilate Size", $"{family}.DilateSize", "0"));
        profile.Controls.Add(Check("OCR String", "Constraint", $"{family}.Constraint", "false"));
        profile.Controls.Add(Number("OCR String", "Consecutive Space", $"{family}.ConsecutiveSpace", "0"));
        profile.Controls.Add(Command("OCR Navigate", "Previous Font", $"{family}.BackRequested"));
        profile.Controls.Add(Command("OCR Navigate", "Next Font", $"{family}.NextRequested"));
        profile.Controls.Add(Command("OCR Navigate", "Load OCR Image 1", $"{family}.LoadImage1Requested"));
        profile.Controls.Add(Command("OCR Navigate", "Load OCR Image 2", $"{family}.LoadImage2Requested"));
        profile.Controls.Add(Command("OCR Navigate", "Load OCR Image 3", $"{family}.LoadImage3Requested"));
        profile.Controls.Add(Command("OCR Navigate", "Select Next Image", $"{family}.SelectNextImageRequested"));
        profile.Controls.Add(Check("OCR Paint", "Pen", $"{family}.PaintPen", "false"));
        profile.Controls.Add(Check("OCR Paint", "Eraser", $"{family}.PaintEraser", "false"));
        profile.Controls.Add(Check("OCR Paint", "Move/Pan", $"{family}.PaintMovePan", "true"));
        profile.Controls.Add(Number("OCR Paint", "Pen Size", $"{family}.PenSize", "1"));
        profile.Controls.Add(Check("OCR Font", "Use Pass Font", $"{family}.UsePassFont", "false"));
        profile.Controls.Add(Number("OCR Font", "Pass Font Count", $"{family}.PassFontCount", "0"));
        profile.Controls.Add(Check("OCR Font", "OK String", $"{family}.OKString", "true"));
        profile.Controls.Add(Check("OCR Font", "NG String", $"{family}.NGString", "false"));
        profile.Controls.Add(Command("OCR Font", "Add Character", $"{family}.AddCharRequested"));
        profile.Controls.Add(Command("OCR Font", "Add Index", $"{family}.AddIndexRequested"));
    }

    private static void AddPatternAdvanced(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Number("Pattern", "Angle Range", $"{family}.AngleRange", "0"));
        profile.Controls.Add(Number("Pattern", "Sampling Angle", $"{family}.SamplingAngle", "0"));
        profile.Controls.Add(Check("Pattern", "Pattern Polarity", $"{family}.PatternPolarity", "false"));
        profile.Controls.Add(Check("Pattern", "Use NG Option", $"{family}.UseNgOption", "false"));
        profile.Controls.Add(Number("Pattern Array", "Column", $"{family}.Column", "1"));
        profile.Controls.Add(Number("Pattern Array", "Row", $"{family}.Row", "1"));
        profile.Controls.Add(Number("Pattern Array", "Score All", $"{family}.ScoreAll", "80"));
        profile.Controls.Add(Command("Pattern Model", "Create Model", $"{family}.PatternCreateModelRequested"));
        profile.Controls.Add(Command("Pattern Model", "Delete", $"{family}.PatternDeleteRequested"));
        profile.Controls.Add(Command("Pattern Model", "Rotate", $"{family}.PatternRotateRequested"));
        profile.Controls.Add(Command("Pattern Model", "Save Pattern", $"{family}.SavePatternRequested"));
        profile.Controls.Add(Command("Pattern Model", "Update Pattern", $"{family}.UpdatePatternRequested"));
        profile.Controls.Add(Command("Pattern Model", "Delete Pattern List", $"{family}.DeletePatternListRequested"));
        profile.Controls.Add(Command("Pattern Model", "All Model", $"{family}.AllModelRequested"));
        profile.Controls.Add(Command("Pattern Model", "Similar Edit", $"{family}.SimilarEditRequested"));
        profile.Controls.Add(Command("Pattern Model", "Inspection", $"{family}.InspectionRequested"));
        profile.Controls.Add(Command("Pattern Model", "Custom OK Image", $"{family}.CustomOkImageRequested"));
        profile.Controls.Add(Command("Pattern Model", "Delete Custom OK Image", $"{family}.DeleteCustomOkImageRequested"));
        profile.Controls.Add(Check("Pattern Paint", "Move/Pan", $"{family}.PaintMovePan", "true"));
        profile.Controls.Add(Check("Pattern Paint", "Mask", $"{family}.PaintMask", "false"));
        profile.Controls.Add(Check("Pattern Paint", "Edge", $"{family}.PaintEdge", "false"));
        profile.Controls.Add(Number("Pattern Paint", "Pen Size", $"{family}.PenSize", "1"));
        profile.Controls.Add(Check("Pattern Search", "Detail Search", $"{family}.DetailSearch", "false"));
        profile.Controls.Add(Check("Pattern Search", "Character", $"{family}.Character", "false"));
        profile.Controls.Add(Combo("Pattern Search", "Algorithm", $"{family}.Algorithm", "0", ["Algo 1", "Algo 2", "Algo 3", "Algo 4", "Algo 5"]));
    }

    private static void AddPatternDiff(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Number("Pattern Diff", "Layer Count", $"{family}.LayerCount", "0"));
        profile.Controls.Add(Number("Pattern Diff", "Model Count", $"{family}.ModelAddCount", "0"));
        profile.Controls.Add(Check("Pattern Diff", "Use Pattern", $"{family}.UsePattern", "false"));
        profile.Controls.Add(Check("Pattern Diff", "Use Dark Foreign", $"{family}.UseDarkForeign", "false"));
        profile.Controls.Add(Number("Pattern Diff", "Theta", $"{family}.Theta", "0"));
        profile.Controls.Add(Number("Pattern Diff", "Model Name Count", $"{family}.ModelNameCount", "0"));
        profile.Controls.Add(Check("Pattern Diff Except", "Use Except Pattern", $"{family}.UseExceptPattern", "false"));
        profile.Controls.Add(Number("Pattern Diff Except", "Except Model Count", $"{family}.ExceptModelAddCount", "0"));
        profile.Controls.Add(Number("Pattern Diff Except", "Except Score", $"{family}.ExceptPatternScore", "0"));
        profile.Controls.Add(Number("Pattern Diff Except", "Except Name Count", $"{family}.ExceptModelNameCount", "0"));
        profile.Controls.Add(Check("Pattern Diff Align", "Use Align Area", $"{family}.UseAlignArea", "false"));
        profile.Controls.Add(Number("Pattern Diff Align", "Align Accept Score", $"{family}.AlignAcceptScore", "0"));
        profile.Controls.Add(Check("Pattern Diff Align", "Use Align Matching", $"{family}.UseAlignMatching", "false"));
        profile.Controls.Add(Number("Pattern Diff Data", "Layer Type Count", $"{family}.LayerTypeCount", "0"));
        profile.Controls.Add(Number("Pattern Diff Data", "BW Data Count", $"{family}.BWDataCount", "0"));
        profile.Controls.Add(Number("Pattern Diff Data", "Teach Area Count", $"{family}.TeachAreaCount", "0"));
        profile.Controls.Add(Number("Pattern Diff Data", "Histogram Count", $"{family}.HistogramCount", "0"));
        profile.Controls.Add(Number("Pattern Diff Data", "Light Data Count", $"{family}.LightDataCount", "0"));
        profile.Controls.Add(Check("Pattern Diff Mask", "Use Mask Filter", $"{family}.UseMaskFilter", "false"));
        profile.Controls.Add(Number("Pattern Diff Mask", "Mask Filter Size", $"{family}.MaskFilterSize", "0"));
        profile.Controls.Add(Number("Pattern Diff Mask", "Mask BW Count", $"{family}.MaskBWDataCount", "0"));
    }

    private static void AddColorAdvanced(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Number("Color Teach", "Inspection Set Value", $"{family}.ColorInspSetValue", "0"));
        profile.Controls.Add(Command("Color Teach", "Color Teach", $"{family}.ColorTeachRequested"));
        profile.Controls.Add(Command("Color Teach", "Color View", $"{family}.ColorViewRequested"));
        profile.Controls.Add(Combo("Color Teach", "Color", $"{family}.Color", "0", ["Red", "Green", "Blue", "Brown", "Gray"]));
        profile.Controls.Add(Check("Color ROI", "Auto Search ROI", $"{family}.AutoSearchROI", "false"));
        profile.Controls.Add(Command("Color ROI", "Add ROI", $"{family}.ColorRoiAddRequested"));
        profile.Controls.Add(Command("Color ROI", "Delete ROI", $"{family}.ColorRoiDeleteRequested"));
        profile.Controls.Add(Command("Color ROI", "Save ROI", $"{family}.ColorRoiSaveRequested"));
        profile.Controls.Add(Check("Color Judge", "Color Judge", $"{family}.ColorJudge", "false"));
        profile.Controls.Add(Check("Color Judge", "Invert", $"{family}.Invert", "false"));
        profile.Controls.Add(Check("Color Judge", "Use R", $"{family}.UseR", "true"));
        profile.Controls.Add(Check("Color Judge", "Use G", $"{family}.UseG", "true"));
        profile.Controls.Add(Check("Color Judge", "Use B", $"{family}.UseB", "true"));
        profile.Controls.Add(Number("Color Array", "Array Copy Count", $"{family}.ArrayCopyCount", "0"));
        profile.Controls.Add(Command("Color ROI", "Load ROI", $"{family}.ColorRoiLoadRequested"));
        profile.Controls.Add(Command("Color ROI", "Select ROI", $"{family}.SelectRoiRequested"));
        profile.Controls.Add(Command("Color ROI", "Select All", $"{family}.SelectAllRequested"));
        profile.Controls.Add(Command("Color ROI", "Delete", $"{family}.DeleteRequested"));
        profile.Controls.Add(Check("Color ROI", "Copy", $"{family}.Copy", "false"));
        profile.Controls.Add(Command("Color Histogram", "Histogram View", $"{family}.ColorHistogramViewRequested"));
        profile.Controls.Add(Command("Color Histogram", "Gray Save", $"{family}.GraySaveRequested"));
        profile.Controls.Add(Combo("Color Histogram", "Gray Value", $"{family}.GrayValue", "0", ["Average", "Min", "Max"]));
        profile.Controls.Add(Combo("Color Light", "ROI Light", $"{family}.RoiLight", "0", ["Bottom", "Middle", "Top"]));
        profile.Controls.Add(Check("Color Light", "Mid Light", $"{family}.MidLight", "false"));
    }

    private static void AddColorXy(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Number("Color XY", "X Min", $"{family}.MinX", "0"));
        profile.Controls.Add(Number("Color XY", "X Max", $"{family}.MaxX", "0"));
        profile.Controls.Add(Number("Color XY", "X Avg", $"{family}.AvgX", "0"));
        profile.Controls.Add(Number("Color XY", "Y Min", $"{family}.MinY", "0"));
        profile.Controls.Add(Number("Color XY", "Y Max", $"{family}.MaxY", "0"));
        profile.Controls.Add(Number("Color XY", "Y Avg", $"{family}.AvgY", "0"));
        profile.Controls.Add(Number("Color XY", "Factor Red", $"{family}.FatorRed", "0"));
        profile.Controls.Add(Number("Color XY", "Factor Green", $"{family}.FatorGreen", "0"));
        profile.Controls.Add(Number("Color XY", "Factor Blue", $"{family}.FatorBlue", "0"));
    }

    private static void AddDistanceAdvanced(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Combo("Distance Link", "Anchor Mode", $"{family}.AnchorMode", "0", ["Window", "Algorithm", "CAD"]));
        profile.Controls.Add(Combo("Distance Link", "Anchor Window", $"{family}.AnchorWindow", "0", ["Current", "Window 1", "Window 2"]));
        profile.Controls.Add(Combo("Distance Link", "Target Window", $"{family}.TargetWindow", "0", ["Current", "Window 1", "Window 2"]));
        profile.Controls.Add(Combo("Distance Link", "Anchor Algorithm", $"{family}.AnchorAlgorithm", "0", ["None", "Align", "Edge"]));
        profile.Controls.Add(Combo("Distance Link", "Target Algorithm", $"{family}.TargetAlgorithm", "0", ["None", "Align", "Edge"]));
        profile.Controls.Add(Check("Distance Target", "Use Target Point", $"{family}.UseTargetPoint", "false"));
        profile.Controls.Add(Check("Distance Target", "Use Target Axes", $"{family}.UseTargetAxes", "false"));
        profile.Controls.Add(Command("Distance Target", "Center To Target", $"{family}.CenterToTargetRequested"));
        profile.Controls.Add(Command("Distance Target", "CAD To Target", $"{family}.CadToTargetRequested"));
        profile.Controls.Add(Check("Distance Correction", "Use Correction Limit", $"{family}.UseCorrectionLimit", "false"));
        profile.Controls.Add(Number("Distance Correction", "Correction Limit Min", $"{family}.CorrectionLimitMin", "0"));
        profile.Controls.Add(Number("Distance Correction", "Correction Limit Max", $"{family}.CorrectionLimitMax", "0"));
        profile.Controls.Add(Check("Distance Log", "Coordinate Log", $"{family}.CoordinateLog", "false"));
        profile.Controls.Add(Command("Distance Link", "Add", $"{family}.AddRequested"));
        profile.Controls.Add(Command("Distance Link", "Delete", $"{family}.DeleteRequested"));
    }

    private static void AddBlobAdvanced(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("Blob Model", "Use Foreign Pattern", $"{family}.UseForeignPattern", "false"));
        profile.Controls.Add(Check("Blob Model", "Foreign Pattern Binary", $"{family}.ForeignPatternBinary", "false"));
        profile.Controls.Add(Check("Blob Model", "Add Model MinMax", $"{family}.AddModelMinMax", "false"));
        profile.Controls.Add(Number("Blob Model", "Foreign Pattern Margin", $"{family}.FPMargin", "0"));
        profile.Controls.Add(Command("Blob Model", "All Blob", $"{family}.AllBlobRequested"));
        profile.Controls.Add(Check("Blob Sub Line", "Use Sub Line 2D", $"{family}.UseSubLine2D", "false"));
        profile.Controls.Add(Number("Blob Sub Line", "Sub Line 2D Min", $"{family}.SubLine2DMin", "0"));
        profile.Controls.Add(Number("Blob Sub Line", "Sub Line 2D Max", $"{family}.SubLine2DMax", "255"));
        profile.Controls.Add(Check("Blob Sub Line", "Use Sub Line 3D", $"{family}.UseSubLine3D", "false"));
        profile.Controls.Add(Number("Blob Sub Line", "Sub Line 3D Min", $"{family}.SubLine3DMin", "0"));
        profile.Controls.Add(Number("Blob Sub Line", "Sub Line 3D Max", $"{family}.SubLine3DMax", "100"));
        profile.Controls.Add(Combo("Blob Sub Line", "Except Area Calc", $"{family}.ExceptAreaCalc", "0", ["None", "Area", "Width", "Length"]));
        profile.Controls.Add(Check("Blob Coil", "Show Coil Body", $"{family}.ShowCoilBody", "false"));
        profile.Controls.Add(Number("Blob Coil", "Coil Min", $"{family}.CoilMin", "0"));
        profile.Controls.Add(Number("Blob Coil", "Coil Max", $"{family}.CoilMax", "0"));
        profile.Controls.Add(Combo("Blob Coil", "Distance Type", $"{family}.DistanceType", "0", ["Center", "Edge", "Pitch"]));
        profile.Controls.Add(Command("Blob Coil", "Coil Filter Config", $"{family}.CoilFilterConfigRequested"));
        profile.Controls.Add(Check("Blob Pin", "Use Pin Angle", $"{family}.UsePinAngle", "false"));
        profile.Controls.Add(Number("Blob Pin", "Pin Angle", $"{family}.PinAngle", "0"));
        profile.Controls.Add(Number("Blob Pin", "Pin Height", $"{family}.PinHeight", "0"));
        profile.Controls.Add(Number("Blob Pin", "Pin Angle Range", $"{family}.PinAngleRange", "0"));
        profile.Controls.Add(Check("Blob PAD", "Use Teach Threshold", $"{family}.UseTeachThreshold", "false"));
        profile.Controls.Add(Number("Blob PAD", "PAD G", $"{family}.PADG", "0"));
        profile.Controls.Add(Number("Blob PAD", "PAD W", $"{family}.PADW", "0"));
        profile.Controls.Add(Number("Blob PAD", "PAD WM", $"{family}.PADWM", "0"));
    }

    private static void AddGrid(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Number("Grid", "Row", $"{family}.Row", "1"));
        profile.Controls.Add(Number("Grid", "Column", $"{family}.Column", "1"));
        profile.Controls.Add(Check("Grid", "Use Pixel", $"{family}.UsePixel", "true"));
        profile.Controls.Add(Check("Grid", "Use MM", $"{family}.UseMM", "false"));
        profile.Controls.Add(Number("Grid", "Pixel Width", $"{family}.PixelWidth", "0"));
        profile.Controls.Add(Number("Grid", "Pixel Length", $"{family}.PixelLength", "0"));
        profile.Controls.Add(Number("Grid", "MM Width", $"{family}.MMWidth", "0"));
        profile.Controls.Add(Number("Grid", "MM Length", $"{family}.MMLength", "0"));
        profile.Controls.Add(Number("Grid", "Ignore Color", $"{family}.IgnoreColor", "3"));
        profile.Controls.Add(Number("Grid", "Std Dev", $"{family}.StdDev", "0"));
        profile.Controls.Add(Check("Grid", "Use Standard", $"{family}.UseStandard", "true"));
        profile.Controls.Add(Check("Grid", "Use Image Mix", $"{family}.UseImageMix", "false"));
        profile.Controls.Add(Number("Grid", "Image Mix Count", $"{family}.ImageMixCount", "0"));
        profile.Controls.Add(Command("Grid", "Convert", $"{family}.ConvertRequested"));
        profile.Controls.Add(Check("Grid Except", "Include", $"{family}.ExceptInclude", "true"));
        profile.Controls.Add(Check("Grid Except", "Gray", $"{family}.UseGray", "false"));
        profile.Controls.Add(Check("Grid Except", "Color", $"{family}.UseColor", "false"));
        profile.Controls.Add(Number("Grid Except", "Except Area", $"{family}.ExceptAreaCount", "0"));
        profile.Controls.Add(Command("Grid Except", "Except Area Save", $"{family}.ExceptAreaSaveRequested"));
    }

    private static void AddLine(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("Line", "Invert", $"{family}.Invert", "false"));
        profile.Controls.Add(Check("Line", "Use Filter", $"{family}.UseFilter", "false"));
        profile.Controls.Add(Number("Line", "Filter Step Narrow", $"{family}.FilterStepNarrow", "4"));
        profile.Controls.Add(Check("Line", "Use Shift", $"{family}.UseShift", "true"));
        profile.Controls.Add(Check("Line", "Use ROI", $"{family}.UseROI", "false"));
        profile.Controls.Add(Check("Line Measure", "Is Horizon", $"{family}.IsHorizon", "false"));
        profile.Controls.Add(Number("Line Measure", "Measure Direction", $"{family}.MeasureDirection", "0"));
        profile.Controls.Add(Check("Line Measure", "Use Angle", $"{family}.UseAngle", "false"));
        profile.Controls.Add(Number("Line Measure", "Teach Rotate", $"{family}.TeachRotate", "0"));
        profile.Controls.Add(Number("Line Measure", "Teach Center X", $"{family}.TeachCenterX", "0"));
        profile.Controls.Add(Number("Line Measure", "Teach Center Y", $"{family}.TeachCenterY", "0"));
        profile.Controls.Add(Check("Line Cross", "Use Cross", $"{family}.UseCross", "false"));
        profile.Controls.Add(Check("Line Cross", "Use Fix", $"{family}.UseFix", "false"));
        profile.Controls.Add(Number("Line Cross", "Cross Option", $"{family}.CrossOption", "0"));
        profile.Controls.Add(Check("Line Cross", "Use End Position", $"{family}.UseEndPosition", "false"));
        profile.Controls.Add(Number("Line Data", "Line Data", $"{family}.LineData", "0"));
        profile.Controls.Add(Number("Line Data", "Perpendicular 1", $"{family}.Perpendicular1", "0"));
        profile.Controls.Add(Number("Line Data", "Perpendicular 2", $"{family}.Perpendicular2", "0"));
    }

    private static void AddEdgeAdvanced(AlgorithmReferenceUiProfile profile, string family)
    {
        AddLine(profile, family);
        profile.Controls.Add(Number("Edge", "Set Line Count", $"{family}.SetLineCount", "0"));
        profile.Controls.Add(Check("Edge", "Group", $"{family}.Group", "false"));
        profile.Controls.Add(Number("Edge", "Line Find Type", $"{family}.LineFindType", "0"));
        profile.Controls.Add(Number("Edge", "Line Find Rate", $"{family}.LineFindRate", "100"));
        profile.Controls.Add(Number("Edge", "Inspection Option", $"{family}.InspectionOption", "0"));
        profile.Controls.Add(Check("Edge", "Find Center", $"{family}.FindCenter", "false"));
        profile.Controls.Add(Check("Edge Cross", "Use Cross Center", $"{family}.UseCrossCenter", "false"));
        profile.Controls.Add(Number("Edge Cross", "Std Teach Rotate", $"{family}.StdTeachRotate", "0"));
        profile.Controls.Add(Check("Edge Distance", "Use Distance X", $"{family}.UseDistanceX", "false"));
        profile.Controls.Add(Number("Edge Distance", "Distance X", $"{family}.DistanceX", "0"));
        profile.Controls.Add(Number("Edge Distance", "Distance X Min", $"{family}.DistanceXMin", "0"));
        profile.Controls.Add(Number("Edge Distance", "Distance X Max", $"{family}.DistanceXMax", "0"));
        profile.Controls.Add(Check("Edge Distance", "Use Distance Y", $"{family}.UseDistanceY", "false"));
        profile.Controls.Add(Number("Edge Distance", "Distance Y", $"{family}.DistanceY", "0"));
        profile.Controls.Add(Number("Edge Distance", "Distance Y Min", $"{family}.DistanceYMin", "0"));
        profile.Controls.Add(Number("Edge Distance", "Distance Y Max", $"{family}.DistanceYMax", "0"));
        profile.Controls.Add(Number("Edge Lines", "Line 1 Length", $"{family}.Line1.TeachLength", "0"));
        profile.Controls.Add(Number("Edge Lines", "Line 2 Length", $"{family}.Line2.TeachLength", "0"));
        profile.Controls.Add(Number("Edge Lines", "Line 3 Length", $"{family}.Line3.TeachLength", "0"));
        profile.Controls.Add(Number("Edge Lines", "Line 4 Length", $"{family}.Line4.TeachLength", "0"));
    }

    private static void AddTab(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("Tab", "Use Align", $"{family}.UseAlign", "false"));
        profile.Controls.Add(Check("Tab", "Use Body", $"{family}.UseBody", "false"));
        profile.Controls.Add(Check("Tab", "Use Lead", $"{family}.UseLead", "false"));
        profile.Controls.Add(Number("Tab", "Tab Count", $"{family}.TabCount", "0"));
        profile.Controls.Add(Number("Tab", "Tab Width", $"{family}.TabWidth", "0"));
        profile.Controls.Add(Number("Tab", "Tab Length", $"{family}.TabLength", "0"));
        profile.Controls.Add(Number("Tab", "Lead Position", $"{family}.LeadPosition", "0"));
        profile.Controls.Add(Number("Tab", "Solder Length", $"{family}.SolderLength", "0"));
        profile.Controls.Add(Number("Tab", "Tail Area", $"{family}.TailArea", "0"));
        profile.Controls.Add(Number("Tab", "Critical Area", $"{family}.CriticalArea", "0"));
        profile.Controls.Add(Check("Tab", "Use Pitch", $"{family}.UsePitch", "false"));
        profile.Controls.Add(Number("Tab", "Pitch", $"{family}.Pitch", "0"));
        profile.Controls.Add(Check("Tab", "Auto Search ROI", $"{family}.AutoSearchROI", "false"));
        profile.Controls.Add(Check("Tab", "Use Max NG Area", $"{family}.UseMaxNGArea", "false"));
        profile.Controls.Add(Number("Tab", "Max NG Area", $"{family}.MaxNGArea", "0"));
        profile.Controls.Add(Command("Tab", "Search", $"{family}.TabSearchRequested"));
        profile.Controls.Add(Command("Tab", "Apply All", $"{family}.TabApplyAllRequested"));
    }

    private static void AddVolume(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("Volume", "Use Volume", $"{family}.UseVolume", "true"));
        profile.Controls.Add(Number("Volume", "IPC Class", $"{family}.IpcClass", "1"));
        profile.Controls.Add(Number("Volume", "Std Body Height", $"{family}.StandardBodyHeight", "0"));
        profile.Controls.Add(Number("Volume", "Std Width", $"{family}.StdWidth", "0"));
        profile.Controls.Add(Number("Volume", "Std Length", $"{family}.StdLength", "0"));
        profile.Controls.Add(Number("Volume", "Std Volume", $"{family}.StandardVolume", "0"));
        profile.Controls.Add(Number("Volume", "Add Ref Volume", $"{family}.AddRefVolume", "0"));
        profile.Controls.Add(Number("Volume", "Volume Min", $"{family}.VolumeMin", "0"));
        profile.Controls.Add(Number("Volume", "Volume Max", $"{family}.VolumeMax", "0"));
        profile.Controls.Add(Number("Volume", "Volume Height", $"{family}.VolumeHeight", "0"));
        profile.Controls.Add(Check("Volume", "Use Relative Height", $"{family}.UseRelativeHeight", "false"));
        profile.Controls.Add(Number("Volume", "Relative Height Min", $"{family}.RelativeHeightMin", "0"));
        profile.Controls.Add(Number("Volume", "Relative Height Max", $"{family}.RelativeHeightMax", "0"));
        profile.Controls.Add(Check("Volume", "Use Area Rate", $"{family}.UseAreaRate", "false"));
        profile.Controls.Add(Number("Volume", "Area Rate", $"{family}.AreaRate", "0"));
        profile.Controls.Add(Number("Volume", "Area Rate Min", $"{family}.AreaRateMin", "0"));
        profile.Controls.Add(Number("Volume", "Area Rate Max", $"{family}.AreaRateMax", "0"));
        profile.Controls.Add(Check("Volume", "Use Zone Based Area", $"{family}.UseZoneBasedArea", "false"));
        profile.Controls.Add(Check("Volume", "Use BW", $"{family}.UseBW", "true"));
        profile.Controls.Add(Check("Volume", "Use Std Max", $"{family}.UseStdMax", "false"));
        profile.Controls.Add(Number("Volume", "Limit Upper", $"{family}.LimitUpper", "500"));
        profile.Controls.Add(Check("Volume", "Use Cold Joint", $"{family}.UseColdJoint", "false"));
        profile.Controls.Add(Number("Volume", "Cold Joint Area", $"{family}.ColdJointArea", "0"));
        profile.Controls.Add(Number("Volume", "Cold Joint Area %", $"{family}.ColdJointAreaPercent", "0"));
        profile.Controls.Add(Check("Volume", "Use Inclination", $"{family}.UseInclination", "false"));
        profile.Controls.Add(Number("Volume", "Inclination", $"{family}.Inclination", "0"));
        profile.Controls.Add(Number("Volume", "Removal Height", $"{family}.RemovalHeight", "0"));
        profile.Controls.Add(Check("Volume", "Black Height", $"{family}.BlackHeight", "false"));
        profile.Controls.Add(Check("Volume", "Fill Hole", $"{family}.FillHole", "false"));
        profile.Controls.Add(Number("Volume", "Fill Hole Size", $"{family}.FillHoleSize", "0"));
        profile.Controls.Add(Number("Volume", "Chip Tracking Gap", $"{family}.ChipTrackingGap", "-1"));
    }

    private static void AddWidth(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("Width", "Use Width", $"{family}.UseWidth", "false"));
        profile.Controls.Add(Number("Width", "Std Width", $"{family}.StdWidth", "0"));
        profile.Controls.Add(Number("Width", "Width", $"{family}.Width", "0"));
        profile.Controls.Add(Number("Width", "Width Min", $"{family}.WidthMin", "0"));
        profile.Controls.Add(Number("Width", "Width Max", $"{family}.WidthMax", "0"));
        profile.Controls.Add(Check("Width", "Use Length", $"{family}.UseLength", "false"));
        profile.Controls.Add(Number("Width", "Std Length", $"{family}.StdLength", "0"));
        profile.Controls.Add(Number("Width", "Length", $"{family}.Length", "0"));
        profile.Controls.Add(Number("Width", "Length Min", $"{family}.LengthMin", "0"));
        profile.Controls.Add(Number("Width", "Length Max", $"{family}.LengthMax", "0"));
    }

    private static void AddHeightMean(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Command("Height Mean", "Avg Height 3D Teach", $"{family}.AvgHeight3DTeachRequested"));
        profile.Controls.Add(Number("Height Mean", "Avg Height 3D", $"{family}.AvgHeight3D", "0"));
        profile.Controls.Add(Check("Height Mean", "Use Highest", $"{family}.UseHighest", "false"));
        profile.Controls.Add(Number("Height Mean", "Highest Min 3D", $"{family}.HighestMin3D", "0"));
        profile.Controls.Add(Number("Height Mean", "Highest Max 3D", $"{family}.HighestMax3D", "0"));
        profile.Controls.Add(Check("Height Mean", "Use Lowest", $"{family}.UseLowest", "false"));
        profile.Controls.Add(Number("Height Mean", "Lowest Min 3D", $"{family}.LowestMin3D", "0"));
        profile.Controls.Add(Number("Height Mean", "Lowest Max 3D", $"{family}.LowestMax3D", "0"));
        profile.Controls.Add(Check("Height Mean", "Use BW", $"{family}.UseBW", "false"));
        profile.Controls.Add(Command("Height Mean", "BW Option", $"{family}.BWOptionRequested"));
        profile.Controls.Add(Check("Height Mean", "Use Blob", $"{family}.UseBlob", "false"));
        profile.Controls.Add(Command("Height Mean", "Use Blob", $"{family}.UseBlobRequested"));
        profile.Controls.Add(Number("Height Mean", "Correction Value", $"{family}.CorrectionValue", "0"));
        profile.Controls.Add(Number("Height Mean", "Correction Upper", $"{family}.CorrectionUpper", "0"));
        profile.Controls.Add(Number("Height Mean", "Correction Lower", $"{family}.CorrectionLower", "0"));
    }

    private static void AddHeightDiff(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Command("Height Diff", "Set ROI 1", $"{family}.SetRoi1Requested"));
        profile.Controls.Add(Command("Height Diff", "Set ROI 2", $"{family}.SetRoi2Requested"));
        profile.Controls.Add(Command("Height Diff", "Set Window ROI", $"{family}.SetWindowRoiRequested"));
        profile.Controls.Add(Number("Height Diff", "Current 1", $"{family}.Current1", "0"));
        profile.Controls.Add(Number("Height Diff", "Current 2", $"{family}.Current2", "0"));
        profile.Controls.Add(Number("Height Diff", "Current Height Diff", $"{family}.CurrentHeightDiff", "0"));
        profile.Controls.Add(Check("Height Diff", "Polarity", $"{family}.Polarity", "false"));
        profile.Controls.Add(Check("Height Diff", "Sign Inversion", $"{family}.SignInversion", "false"));
        profile.Controls.Add(Combo("Height Diff", "ROI 1 Type", $"{family}.Roi1Type", "0", ["Window", "Algorithm", "Manual"]));
        profile.Controls.Add(Combo("Height Diff", "ROI 2 Type", $"{family}.Roi2Type", "0", ["Window", "Algorithm", "Manual"]));
        profile.Controls.Add(Check("Height Diff", "Use Blob ROI 1", $"{family}.UseBlobRoi1", "false"));
        profile.Controls.Add(Check("Height Diff", "Use Blob ROI 2", $"{family}.UseBlobRoi2", "false"));
        profile.Controls.Add(Command("Height Diff", "Apply", $"{family}.ApplyRequested"));
        profile.Controls.Add(Command("Height Diff", "Apply All", $"{family}.ApplyAllRequested"));
    }

    private static void AddGrayMean(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Command("Gray Mean", "Gray Avg Teach", $"{family}.GrayAvgTeachRequested"));
        profile.Controls.Add(Command("Gray Mean", "Gray Avg Teach All", $"{family}.GrayAvgTeachAllRequested"));
        profile.Controls.Add(Number("Gray Mean", "Gray Avg", $"{family}.GrayAvg", "0"));
        profile.Controls.Add(Number("Gray Mean", "Gray Rate Min", $"{family}.GrayRateMin", "0"));
        profile.Controls.Add(Number("Gray Mean", "Gray Rate Max", $"{family}.GrayRateMax", "100"));
        profile.Controls.Add(Check("Gray Mean", "Include Rate Min", $"{family}.IncludeRateMin", "false"));
        profile.Controls.Add(Check("Gray Mean", "Include Rate Max", $"{family}.IncludeRateMax", "false"));
        profile.Controls.Add(Check("Gray Mean", "Substandard Delete", $"{family}.SubstandardDelete", "false"));
        profile.Controls.Add(Check("Gray Mean", "Substandard Ignore", $"{family}.SubstandardIgnore", "false"));
        profile.Controls.Add(Number("Gray Mean", "Limit Value", $"{family}.LimitValue", "0"));
        profile.Controls.Add(Check("Gray Mean", "Use Blob", $"{family}.UseBlob", "false"));
    }

    private static void AddGrayDiff(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Command("Gray Diff", "Set ROI 1", $"{family}.SetRoi1Requested"));
        profile.Controls.Add(Command("Gray Diff", "Set ROI 2", $"{family}.SetRoi2Requested"));
        profile.Controls.Add(Command("Gray Diff", "Set Window ROI", $"{family}.SetWindowRoiRequested"));
        profile.Controls.Add(Number("Gray Diff", "Current 1", $"{family}.Current1", "0"));
        profile.Controls.Add(Number("Gray Diff", "Current 2", $"{family}.Current2", "0"));
        profile.Controls.Add(Number("Gray Diff", "Current Gray", $"{family}.CurrentGray", "0"));
        profile.Controls.Add(Number("Gray Diff", "Standard Gray", $"{family}.StandardGray", "0"));
        profile.Controls.Add(Check("Gray Diff", "Upper Standard", $"{family}.UpperStandard", "true"));
        profile.Controls.Add(Check("Gray Diff", "Lower Standard", $"{family}.LowerStandard", "false"));
        profile.Controls.Add(Check("Gray Diff", "Polarity", $"{family}.Polarity", "false"));
        profile.Controls.Add(Check("Gray Diff", "Sign Inversion", $"{family}.SignInversion", "false"));
        profile.Controls.Add(Command("Gray Diff", "Measure", $"{family}.MeasureRequested"));
        profile.Controls.Add(Command("Gray Diff", "Apply", $"{family}.ApplyRequested"));
    }

    private static void AddBridge(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Command("Bridge", "Teach", $"{family}.TeachRequested"));
        profile.Controls.Add(Number("Bridge", "Gray Diff", $"{family}.GrayDiff", "0"));
        profile.Controls.Add(Number("Bridge", "Gap Count", $"{family}.GapCount", "0"));
        profile.Controls.Add(Number("Bridge", "Lead Direction", $"{family}.LeadTipDirection", "0"));
        profile.Controls.Add(Number("Bridge", "Current Gray Min", $"{family}.CurrentGrayMin", "0"));
        profile.Controls.Add(Check("Bridge", "2D Inspection Use", $"{family}.Use2DInspection", "true"));
        profile.Controls.Add(Check("Bridge", "2D Range", $"{family}.Use2DRange", "false"));
        profile.Controls.Add(Check("Bridge", "3D Inspection Use", $"{family}.Use3DInspection", "false"));
        profile.Controls.Add(Number("Bridge", "2D/3D Height Diff", $"{family}.HeightDiff2D3D", "0"));
        profile.Controls.Add(Number("Bridge", "Height Diff", $"{family}.HeightDiff", "0"));
        profile.Controls.Add(Number("Bridge", "Percent OK", $"{family}.PercentOK", "0"));
        profile.Controls.Add(Check("Bridge", "Auto Search ROI", $"{family}.AutoSearchROI", "false"));
        profile.Controls.Add(Check("Bridge", "Use Solder Ball", $"{family}.UseSolderBall", "false"));
        profile.Controls.Add(Number("Bridge", "Solder Ball Area", $"{family}.SolderBallArea", "0"));
        profile.Controls.Add(Number("Bridge", "Solder Ball Area %", $"{family}.SolderBallAreaPer", "0"));
        profile.Controls.Add(Check("Bridge", "Use Binarize", $"{family}.UseBinarize", "false"));
        profile.Controls.Add(Check("Bridge", "Use Area", $"{family}.UseArea", "false"));
        profile.Controls.Add(Number("Bridge", "Area Max", $"{family}.AreaMax", "0"));
        profile.Controls.Add(Check("Bridge", "Use Width", $"{family}.UseWidth", "false"));
        profile.Controls.Add(Number("Bridge", "Width Max", $"{family}.WidthMax", "0"));
        profile.Controls.Add(Check("Bridge", "Use Length", $"{family}.UseLength", "false"));
        profile.Controls.Add(Number("Bridge", "Length Max", $"{family}.LengthMax", "0"));
        profile.Controls.Add(Check("Bridge", "Use Mode2", $"{family}.UseMode2", "false"));
        profile.Controls.Add(Combo("Bridge", "Sorting", $"{family}.Sorting", "0", ["None", "Left", "Right", "Area"]));
        profile.Controls.Add(Command("Bridge", "Sorting Run", $"{family}.SortingRunRequested"));
    }

    private static void AddFilletFoot(AlgorithmReferenceUiProfile profile, string family, bool includeFootFields)
    {
        var tab = includeFootFields ? "Foot" : "Fillet";
        profile.Controls.Add(Check(tab, "Use Solder Fillet", $"{family}.UseSolderFillet", "false"));
        profile.Controls.Add(Number(tab, "Fillet Interval", $"{family}.FilletInterval", "0"));
        profile.Controls.Add(Number(tab, "Fillet Gap", $"{family}.FilletGap", "0"));
        profile.Controls.Add(Command(tab, "Set Fillet Range", $"{family}.SetFilletRangeRequested"));
        profile.Controls.Add(Command(tab, "Set Fillet Tolerance", $"{family}.SetFilletToleranceRequested"));
        profile.Controls.Add(Number(tab, "Fillet Div Count", $"{family}.FilletDivCount", "0"));
        profile.Controls.Add(Check(tab, "Angle Range H", $"{family}.AngleRangeH", "false"));
        profile.Controls.Add(Number(tab, "Min Angle R", $"{family}.MinAngleR", "0"));
        profile.Controls.Add(Number(tab, "Max Angle R", $"{family}.MaxAngleR", "0"));
        profile.Controls.Add(Check(tab, "Direct", $"{family}.Direct", "false"));
        profile.Controls.Add(Number(tab, "Tip Direction", $"{family}.TipDirection", "0"));
        profile.Controls.Add(Check(tab, "Use Lead Tip Position", $"{family}.UseLeadTipPosition", "false"));
        profile.Controls.Add(Number(tab, "Lead Tip Position", $"{family}.LeadTipPosition", "0"));
        profile.Controls.Add(Command(tab, "Apply Fillet", $"{family}.ApplyFilletRequested"));

        if (!includeFootFields)
        {
            return;
        }

        profile.Controls.Add(Number("Foot", "Foot Type", $"{family}.FootType", "0"));
        profile.Controls.Add(Number("Foot", "Foot Direction", $"{family}.FootDirection", "0"));
        profile.Controls.Add(Number("Foot", "Teach Foot Direction", $"{family}.TeachFootDirection", "0"));
        profile.Controls.Add(Number("Foot", "Find Option", $"{family}.FindOption", "0"));
        profile.Controls.Add(Number("Foot", "Find Option 2", $"{family}.FindOption2", "0"));
        profile.Controls.Add(Check("Foot", "Use Pattern Angle", $"{family}.UsePatternAngle", "false"));
        profile.Controls.Add(Check("Foot", "Use 2 Foot", $"{family}.Use2Foot", "false"));
        profile.Controls.Add(Check("Foot", "Use Pad Area Auto Teach", $"{family}.UsePadAreaAutoTeach", "false"));
        profile.Controls.Add(Number("Foot", "Pad Width", $"{family}.PadWidth", "0"));
        profile.Controls.Add(Number("Foot", "Pad Height", $"{family}.PadHeight", "0"));
        profile.Controls.Add(Number("Foot", "Radius", $"{family}.Radius", "0"));
        profile.Controls.Add(Number("Foot", "Remove Wire Height", $"{family}.RemoveWireHeight", "0"));
        profile.Controls.Add(Number("Foot", "Crack Height", $"{family}.InspCrackHeight", "0"));
        profile.Controls.Add(Number("Foot", "Width Sub Offset", $"{family}.WidthSubOffset", "0"));
        profile.Controls.Add(Number("Foot", "Length Sub Offset", $"{family}.LengthSubOffset", "0"));
        profile.Controls.Add(Number("Foot", "Height Sub Offset", $"{family}.HeightSubOffset", "0"));
    }

    private static void AddLeadTip(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Command("Lead Tip", "Set Position", $"{family}.SetPositionRequested"));
        profile.Controls.Add(Number("Lead Tip", "Lead Tip Position", $"{family}.LeadTipPosition", "0"));
        profile.Controls.Add(Number("Lead Tip", "Search Range", $"{family}.SearchRange", "0"));
        profile.Controls.Add(Number("Lead Tip", "Gap", $"{family}.Gap", "0"));
        profile.Controls.Add(Check("Lead Tip", "NG", $"{family}.NG", "false"));
        profile.Controls.Add(Check("Lead Tip", "Use 2D", $"{family}.Use2D", "false"));
        profile.Controls.Add(Check("Lead Tip", "Use 3D", $"{family}.Use3D", "false"));
        profile.Controls.Add(Check("Lead Tip", "Use Side Tip", $"{family}.UseSideTip", "false"));
        profile.Controls.Add(Number("Lead Tip", "Side Tip Position", $"{family}.SideTipPosition", "0"));
        profile.Controls.Add(Number("Lead Tip", "Side Tip Gap", $"{family}.SideTipGap", "0"));
        profile.Controls.Add(Number("Lead Tip", "Side Tip Gap Width", $"{family}.SideTipGapWidth", "0"));
        profile.Controls.Add(Check("Lead Tip", "Use Tip Cap", $"{family}.UseTipCap", "false"));
        profile.Controls.Add(Number("Lead Tip", "Cap Gap", $"{family}.CapGap", "0"));
        profile.Controls.Add(Check("Lead Tip", "Use Tip Length", $"{family}.UseTipLength", "false"));
        profile.Controls.Add(Number("Lead Tip", "Tip Length", $"{family}.TipLength", "0"));
        profile.Controls.Add(Number("Lead Tip", "Lead Find Percent", $"{family}.LeadFindPercent", "0"));
        profile.Controls.Add(Check("Lead Tip", "Direction Invert", $"{family}.DirectionInvert", "false"));
    }

    private static void AddLeadLift(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Command("Lead Lift", "Set Position", $"{family}.SetPositionRequested"));
        profile.Controls.Add(Number("Lead Lift", "Lead Position", $"{family}.LeadPosition", "0"));
        profile.Controls.Add(Number("Lead Lift", "Direction", $"{family}.LeadTipDirection", "0"));
        profile.Controls.Add(Number("Lead Lift", "Interval", $"{family}.Interval", "0"));
        profile.Controls.Add(Number("Lead Lift", "Width", $"{family}.Width", "0"));
        profile.Controls.Add(Number("Lead Lift", "Height", $"{family}.Height", "0"));
        profile.Controls.Add(Number("Lead Lift", "Current Height 3D", $"{family}.CurrentHeight3D", "0"));
        profile.Controls.Add(Number("Lead Lift", "Avg Height 3D", $"{family}.AvgHeight3D", "0"));
        profile.Controls.Add(Number("Lead Lift", "Height Diff", $"{family}.HeightDiff", "0"));
        profile.Controls.Add(Check("Lead Lift", "Use Lead Coplanarity", $"{family}.UseLeadCoplanarity", "false"));
        profile.Controls.Add(Check("Lead Lift", "Use Gradient", $"{family}.UseGradient", "false"));
        profile.Controls.Add(Number("Lead Lift", "Gradient", $"{family}.Gradient", "0"));
        profile.Controls.Add(Check("Lead Lift", "Use Fillet Height", $"{family}.UseFilletHeight", "false"));
        profile.Controls.Add(Number("Lead Lift", "Fillet Height Min", $"{family}.FilletHeightMin", "0"));
        profile.Controls.Add(Number("Lead Lift", "Fillet Height Max", $"{family}.FilletHeightMax", "0"));
        profile.Controls.Add(Combo("Lead Lift", "Fillet Height Find Type", $"{family}.FilletHeightFindType", "0", ["Min", "Max", "Average"]));
    }

    private static void AddLeadColor(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Command("Lead Color", "Set Position", $"{family}.SetPositionRequested"));
        profile.Controls.Add(Number("Lead Color", "Lead Position", $"{family}.LeadPosition", "0"));
        profile.Controls.Add(Number("Lead Color", "Solder Length", $"{family}.SolderLength", "0"));
        profile.Controls.Add(Number("Lead Color", "Gap", $"{family}.Gap", "0"));
        profile.Controls.Add(Check("Lead Color", "Color Judge", $"{family}.ColorJudge", "false"));
        profile.Controls.Add(Number("Lead Color", "Color Inspection Set Value", $"{family}.ColorInspectionSetValue", "0"));
        profile.Controls.Add(Command("Lead Color", "Color Teach", $"{family}.ColorTeachRequested"));
        profile.Controls.Add(Command("Lead Color", "Copper Default", $"{family}.CopperDefaultRequested"));
        profile.Controls.Add(Check("Lead Color", "Use Range Bar", $"{family}.UseRangeBar", "false"));
        profile.Controls.Add(Check("Lead Color", "Copa Edit", $"{family}.CopaEdit", "false"));
    }

    private static void AddLeadSideSolder(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Command("Lead Side", "Set Position", $"{family}.SetPositionRequested"));
        profile.Controls.Add(Number("Lead Side", "Lead Position", $"{family}.LeadPosition", "0"));
        profile.Controls.Add(Number("Lead Side", "Solder Length", $"{family}.SolderLength", "0"));
        profile.Controls.Add(Number("Lead Side", "Height", $"{family}.Height", "0"));
        profile.Controls.Add(Number("Lead Side", "Width", $"{family}.Width", "0"));
        profile.Controls.Add(Number("Lead Side", "Lead Lift Set Value", $"{family}.LeadLiftSetValue", "0"));
        profile.Controls.Add(Number("Lead Side", "Gap", $"{family}.Gap", "0"));
        profile.Controls.Add(Number("Lead Side", "Gap Width", $"{family}.GapWidth", "0"));
        profile.Controls.Add(Check("Lead Side", "Use Teaching Rate", $"{family}.UseTeachingRate", "false"));
        profile.Controls.Add(Number("Lead Side", "Teaching Area", $"{family}.TeachingArea", "0"));
        profile.Controls.Add(Command("Lead Side", "Teach Area", $"{family}.TeachAreaRequested"));
        profile.Controls.Add(Combo("Lead Side", "Inspection Area", $"{family}.InspectionArea", "0", ["Left", "Top", "Right", "Bottom"]));
        profile.Controls.Add(Check("Lead Side", "Angle Color Option", $"{family}.AngleColorOption", "false"));
    }

    private static void AddLeadSearch(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("Lead Search", "Use 3D", $"{family}.Use3D", "true"));
        profile.Controls.Add(Check("Lead Search", "Use 2D", $"{family}.Use2D", "true"));
        profile.Controls.Add(Number("Lead Search", "Lead Position", $"{family}.LeadPosition", "0"));
        profile.Controls.Add(Number("Lead Search", "Solder Start", $"{family}.SolderStartPos", "0"));
        profile.Controls.Add(Slider("Lead Search", "Solder Threshold", $"{family}.SolderThreshold", "20", 0, 255));
        profile.Controls.Add(Check("Lead Search", "Same Width / Pitch", $"{family}.SameWidthPitch", "false"));
        profile.Controls.Add(Check("Lead Search", "Use Gerber", $"{family}.UseGerber", "false"));
        profile.Controls.Add(Number("Lead Search ROI", "ROI Width", $"{family}.RoiWidth", "0"));
        profile.Controls.Add(Number("Lead Search ROI", "ROI Pitch", $"{family}.RoiPitch", "0"));
        profile.Controls.Add(Number("Lead Search ROI", "ROI Count", $"{family}.RoiCount", "0"));
        profile.Controls.Add(Number("Lead Search ROI", "ROI Start Position", $"{family}.RoiStartPos", "0"));
        profile.Controls.Add(Command("Lead Search", "Create", $"{family}.CreateRequested"));
        profile.Controls.Add(Command("Lead Search", "Delete", $"{family}.DeleteRequested"));
        profile.Controls.Add(Command("Lead Search", "Manual", $"{family}.ManualRequested"));
        profile.Controls.Add(Command("Lead Search", "Top", $"{family}.TopRequested"));
        profile.Controls.Add(Command("Lead Search", "Bottom", $"{family}.BottomRequested"));
        profile.Controls.Add(Command("Lead Search", "Previous", $"{family}.PrevRequested"));
    }

    private static void AddWire(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Command("Wire", "Teach", $"{family}.TeachRequested"));
        profile.Controls.Add(Combo("Wire", "Select Type", $"{family}.SelectType", "0", ["Wire", "GWire", "Cross"]));
        profile.Controls.Add(Number("Wire", "Wire Count", $"{family}.WireCount", "0"));
        profile.Controls.Add(Number("Wire", "Wire Thickness", $"{family}.WireThickness", "0"));
        profile.Controls.Add(Check("Wire", "Fill Hole", $"{family}.UseFillHole", "false"));
        profile.Controls.Add(Check("Wire", "Cross Filter", $"{family}.UseCrossFilter", "false"));
        profile.Controls.Add(Check("Wire", "Use Center Margin", $"{family}.UseCenterMargin", "false"));
        profile.Controls.Add(Number("Wire", "Center Margin", $"{family}.CenterMargin", "0"));
    }

    private static void AddPadArray(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Number("Pad Array", "Row", $"{family}.Row", "1"));
        profile.Controls.Add(Number("Pad Array", "Column", $"{family}.Column", "1"));
        profile.Controls.Add(Number("Pad Array", "Center X", $"{family}.CenterX", "0"));
        profile.Controls.Add(Number("Pad Array", "Center Y", $"{family}.CenterY", "0"));
        profile.Controls.Add(Check("Pad Array", "Use Distance", $"{family}.UseDistance", "false"));
        profile.Controls.Add(Number("Pad Array", "Distance X", $"{family}.DistanceX", "0"));
        profile.Controls.Add(Number("Pad Array", "Distance Y", $"{family}.DistanceY", "0"));
        profile.Controls.Add(Number("Pad Array", "Data Flags", $"{family}.DataFlags", "0"));
        profile.Controls.Add(Number("Pad Array", "NData Count", $"{family}.NDataCount", "0"));
        profile.Controls.Add(Number("Pad Array", "FData Count", $"{family}.FDataCount", "0"));
        profile.Controls.Add(Check("Pad Array", "Use Width", $"{family}.UseWidth", "false"));
        profile.Controls.Add(Check("Pad Array", "Use Length", $"{family}.UseLength", "false"));
        profile.Controls.Add(Number("Pad Array Size", "Width", $"{family}.Width", "0"));
        profile.Controls.Add(Number("Pad Array Size", "Width Min", $"{family}.WidthMin", "0"));
        profile.Controls.Add(Number("Pad Array Size", "Width Max", $"{family}.WidthMax", "0"));
        profile.Controls.Add(Number("Pad Array Size", "Length", $"{family}.Length", "0"));
        profile.Controls.Add(Number("Pad Array Size", "Length Min", $"{family}.LengthMin", "0"));
        profile.Controls.Add(Number("Pad Array Size", "Length Max", $"{family}.LengthMax", "0"));
        profile.Controls.Add(Number("Pad Array Height", "Height Min", $"{family}.HeightMin", "0"));
        profile.Controls.Add(Number("Pad Array Height", "Height Max", $"{family}.HeightMax", "0"));
        profile.Controls.Add(Number("Pad Array Height", "Height Avg", $"{family}.HeightAvg", "0"));
        profile.Controls.Add(Command("Pad Array", "Set ROI 1", $"{family}.SetRoi1Requested"));
        profile.Controls.Add(Command("Pad Array", "Set ROI 2", $"{family}.SetRoi2Requested"));
        profile.Controls.Add(Combo("Pad Array", "Pad Select", $"{family}.PadSelect", "0", ["All", "Pad 1", "Pad 2"]));
        profile.Controls.Add(Command("Pad Array", "Gerber", $"{family}.GerberRequested"));
    }

    private static void AddPadAlign(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Check("Pad Align", "Use Angle", $"{family}.UseAngle", "false"));
        profile.Controls.Add(Number("Pad Align", "Angle", $"{family}.Angle", "90"));
        profile.Controls.Add(Check("Pad Align", "Use Shift", $"{family}.UseShift", "false"));
        profile.Controls.Add(Number("Pad Align", "Shift X", $"{family}.ShiftX", "0"));
        profile.Controls.Add(Number("Pad Align", "Shift Y", $"{family}.ShiftY", "0"));
        profile.Controls.Add(Check("Pad Align", "Use Distance", $"{family}.UseDistance", "false"));
        profile.Controls.Add(Number("Pad Align", "Distance X", $"{family}.DistanceX", "0"));
        profile.Controls.Add(Number("Pad Align", "Distance Y", $"{family}.DistanceY", "0"));
        profile.Controls.Add(Command("Pad Align", "Teach", $"{family}.TeachRequested"));
        profile.Controls.Add(Command("Pad Align", "Apply", $"{family}.ApplyRequested"));
    }

    private static void AddColorBandSearch(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Command("Color Band", "Select All", $"{family}.SelectAllRequested"));
        profile.Controls.Add(Check("Color Band", "Copy", $"{family}.Copy", "false"));
        profile.Controls.Add(Check("Color Band", "Draw Mode", $"{family}.DrawMode", "false"));
        profile.Controls.Add(Command("Color Band", "Group", $"{family}.GroupCreateRequested"));
        profile.Controls.Add(Command("Color Band", "Group Off", $"{family}.GroupOffRequested"));
        profile.Controls.Add(Number("Color Band ROI", "ROI Width", $"{family}.RoiWidth", "0"));
        profile.Controls.Add(Number("Color Band ROI", "ROI Pitch", $"{family}.RoiPitch", "0"));
        profile.Controls.Add(Number("Color Band ROI", "ROI Count", $"{family}.RoiCount", "0"));
        profile.Controls.Add(Number("Color Band ROI", "ROI Start Position", $"{family}.RoiStartPos", "0"));
    }

    private static void AddOcv(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Slider("OCV", "Binarize", $"{family}.BinarizeValue", "128", 0, 255));
        profile.Controls.Add(Check("OCV", "Auto Threshold", $"{family}.AutoThreshold", "false"));
        profile.Controls.Add(Number("OCV", "Angle", $"{family}.Angle", "0"));
        profile.Controls.Add(Number("OCV", "Char Score", $"{family}.CharScore", "80"));
        profile.Controls.Add(Check("OCV", "Polarity", $"{family}.Polarity", "false"));
        profile.Controls.Add(Number("OCV", "Allocated Font", $"{family}.AllocFont", "0"));
        profile.Controls.Add(Number("OCV", "Variable", $"{family}.Variable", "0"));
        profile.Controls.Add(Combo("OCV", "Font Score", $"{family}.FontScore", "0", ["Low", "Normal", "High"]));
        profile.Controls.Add(Command("OCV", "Read", $"{family}.ReadRequested"));
        profile.Controls.Add(Command("OCV", "Add Font", $"{family}.AddFontRequested"));
        profile.Controls.Add(Command("OCV", "Delete Font", $"{family}.DeleteFontRequested"));
        profile.Controls.Add(Command("OCV", "Back", $"{family}.BackRequested"));
        profile.Controls.Add(Command("OCV", "Next", $"{family}.NextRequested"));
        profile.Controls.Add(Command("OCV", "Font Apply", $"{family}.FontApplyRequested"));
        profile.Controls.Add(Command("OCV Pattern", "Save Pattern", $"{family}.SavePatternRequested"));
        profile.Controls.Add(Command("OCV Pattern", "Update Pattern", $"{family}.UpdatePatternRequested"));
        profile.Controls.Add(Number("OCV Blob Filter", "Pixel Filter", $"{family}.PixelFilter", "0"));
        profile.Controls.Add(Number("OCV Blob Filter", "Blob Size", $"{family}.BlobSize", "0"));
        profile.Controls.Add(Number("OCV Blob Filter", "Blob Count", $"{family}.BlobCount", "0"));
        profile.Controls.Add(Number("OCV Blob Filter", "Contrast", $"{family}.Contrast", "0"));
        profile.Controls.Add(Check("OCV String", "Use Lower Special", $"{family}.UseLowerSpecial", "false"));
        profile.Controls.Add(Check("OCV String", "Use Char Space", $"{family}.UseCharSpace", "false"));
        profile.Controls.Add(Number("OCV String", "Font Count Spec", $"{family}.FontCountSpec", "0"));
        profile.Controls.Add(Command("OCV Model", "Train Model", $"{family}.TrainModelRequested"));
        profile.Controls.Add(Command("OCV Model", "Create Model", $"{family}.CreateModelRequested"));
        profile.Controls.Add(Number("OCV Model", "OCV Score", $"{family}.OcvScore", "80"));
        profile.Controls.Add(Check("OCV Font", "Use Font Angle", $"{family}.UseFontAngle", "false"));
        profile.Controls.Add(Number("OCV Font", "Standard Font Angle", $"{family}.StdFontAngle", "0"));
        profile.Controls.Add(Combo("OCV Font", "Font Style", $"{family}.FontStyle", "0", ["Default", "Bold", "Narrow"]));
        profile.Controls.Add(Command("OCV Font", "Font Style Add", $"{family}.FontStyleAddRequested"));
        profile.Controls.Add(Command("OCV Font", "Font Style Visible", $"{family}.FontStyleVisibleRequested"));
        profile.Controls.Add(Number("OCV Font", "Font Style Text", $"{family}.FontStyleText", "0"));
        profile.Controls.Add(Number("OCV Locking", "Locking Letter", $"{family}.LockingLetter", "0"));
        profile.Controls.Add(Command("OCV Locking", "Locking Reset", $"{family}.LockingResetRequested"));
    }

    private static void AddBodyEdge(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Command("Body Edge", "Teach", $"{family}.TeachRequested"));
        profile.Controls.Add(Command("Body Edge", "Add Edge ROI", $"{family}.AddEdgeRoiRequested"));
        profile.Controls.Add(Command("Body Edge", "Delete Edge ROI", $"{family}.DeleteEdgeRoiRequested"));
        profile.Controls.Add(Combo("Body Edge", "Select Edge ROI", $"{family}.SelectEdgeROI", "0", ["ROI 1", "ROI 2", "ROI 3"]));
        profile.Controls.Add(Number("Body Edge", "ROI Total Count", $"{family}.RoiTotalCount", "0"));
        profile.Controls.Add(Check("Body Edge", "Use Height", $"{family}.UseHeight", "false"));
        profile.Controls.Add(Number("Body Edge", "Height Min", $"{family}.HeightMin", "0"));
        profile.Controls.Add(Number("Body Edge", "Height Max", $"{family}.HeightMax", "0"));
        profile.Controls.Add(Check("Body Edge", "Use Width", $"{family}.UseWidth", "false"));
        profile.Controls.Add(Number("Body Edge", "Width Min", $"{family}.WidthMin", "0"));
        profile.Controls.Add(Number("Body Edge", "Width Max", $"{family}.WidthMax", "0"));
        profile.Controls.Add(Combo("Body Edge", "Anchor Mode", $"{family}.AnchorMode", "0", ["None", "Window", "Algorithm"]));
        profile.Controls.Add(Combo("Body Edge", "Target Type", $"{family}.TargetType", "0", ["Body", "Lead", "Pad"]));
        profile.Controls.Add(Check("Body Edge", "Use Sub Line", $"{family}.UseSubLine", "false"));
        profile.Controls.Add(Number("Body Edge", "Max Sub Line", $"{family}.MaxSubLine", "0"));
    }

    private static void AddTilt(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Number("Tilt", "Angle", $"{family}.Angle", "0"));
        profile.Controls.Add(Command("Tilt", "Calc Height", $"{family}.CalcHeightRequested"));
        profile.Controls.Add(Check("Tilt", "One More", $"{family}.OneMore", "false"));
        profile.Controls.Add(Check("Tilt", "Use Color Range", $"{family}.UseColorRange", "false"));
        profile.Controls.Add(Number("Tilt", "Color Min", $"{family}.ColorMin", "0"));
        profile.Controls.Add(Number("Tilt", "Color Max", $"{family}.ColorMax", "0"));
        profile.Controls.Add(Command("Tilt", "Set Teaching Range", $"{family}.SetTeachingRangeRequested"));
        profile.Controls.Add(Command("Tilt", "Set Standard ROI", $"{family}.SetStandardRoiRequested"));
        profile.Controls.Add(Check("Tilt", "Use Angle", $"{family}.UseAngle", "false"));
        profile.Controls.Add(Number("Tilt", "Angle H Min", $"{family}.AngleHMin", "0"));
        profile.Controls.Add(Number("Tilt", "Angle H Max", $"{family}.AngleHMax", "0"));
        profile.Controls.Add(Check("Tilt", "ROI Value Type", $"{family}.RoiValueType", "false"));
        profile.Controls.Add(Check("Tilt", "Use Part", $"{family}.UsePart", "false"));
        profile.Controls.Add(Check("Tilt", "Align Shape", $"{family}.AlignShape", "false"));
        profile.Controls.Add(Combo("Tilt", "Select Angle Value", $"{family}.SelectAngleValue", "0", ["Average", "Max", "Min"]));
    }

    private static void AddSolderCone(AlgorithmReferenceUiProfile profile, string family)
    {
        profile.Controls.Add(Command("Solder Cone", "Teach", $"{family}.TeachRequested"));
        profile.Controls.Add(Number("Solder Cone", "Teach Height", $"{family}.TeachHeight", "0"));
        profile.Controls.Add(Number("Solder Cone", "Current Height", $"{family}.CurrentHeight", "0"));
        profile.Controls.Add(Number("Solder Cone", "Current Area", $"{family}.CurrentArea", "0"));
        profile.Controls.Add(Check("Solder Cone", "Use Highest", $"{family}.UseSolderHighest", "false"));
        profile.Controls.Add(Check("Solder Cone", "Highest Volume Diff", $"{family}.UseSolderHighestVolumeDiff", "false"));
        profile.Controls.Add(Number("Solder Cone", "Highest Height", $"{family}.HighestHeight", "0"));
        profile.Controls.Add(Check("Solder Cone", "Inspect Volume Diff", $"{family}.UseInspVolumeDiff", "false"));
        profile.Controls.Add(Check("Solder Cone", "Inspect Volume Min Length", $"{family}.UseInspVolumeMinLength", "false"));
        profile.Controls.Add(Check("Solder Cone", "Inspect Volume Pie", $"{family}.UseInspVolumePie", "false"));
        profile.Controls.Add(Check("Solder Cone Step", "Use 1 Step", $"{family}.Use1Step", "false"));
        profile.Controls.Add(Number("Solder Cone Step", "1 Step Height", $"{family}.Height1Step", "0"));
        profile.Controls.Add(Number("Solder Cone Step", "1 Step Percent", $"{family}.Step1Percent", "0"));
        profile.Controls.Add(Check("Solder Cone Step", "Use 2 Step", $"{family}.Use2Step", "false"));
        profile.Controls.Add(Number("Solder Cone Step", "2 Step Height", $"{family}.Height2Step", "0"));
        profile.Controls.Add(Number("Solder Cone Step", "2 Step Percent", $"{family}.Step2Percent", "0"));
        profile.Controls.Add(Check("Solder Cone Step", "Use 3 Step", $"{family}.Use3Step", "false"));
        profile.Controls.Add(Number("Solder Cone Step", "3 Step Height", $"{family}.Height3Step", "0"));
        profile.Controls.Add(Number("Solder Cone Step", "3 Step Percent", $"{family}.Step3Percent", "0"));
        profile.Controls.Add(Number("Solder Level", "Gap Height", $"{family}.GapHeight", "0"));
        profile.Controls.Add(Number("Solder Level", "Level Count", $"{family}.LevelCount", "0"));
        profile.Controls.Add(Number("Solder Level", "Volume Diff", $"{family}.VolumeDiff", "0"));
        profile.Controls.Add(Number("Solder Level", "Pie Height Diff", $"{family}.PieHeightDiff", "0"));
        profile.Controls.Add(Command("Solder Level", "Measure Solder Level", $"{family}.MeasureSolderLevelRequested"));
    }
}
