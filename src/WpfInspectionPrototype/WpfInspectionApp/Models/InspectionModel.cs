namespace WpfInspectionApp.Models;

public sealed class InspectionModel
{
    public string ModelName { get; set; } = "PCB-2026-0406-A1";
    public string Algorithm { get; set; } = "AlgoAlign";
    public PartInspectionData Part { get; set; } = new();
    public string? SelectedWindowId { get; set; }

    public int Threshold2D { get; set; } = 128;
    public int Threshold3D { get; set; } = 96;
    public int EdgeGain { get; set; } = 42;
    public bool Use2D { get; set; } = true;
    public bool Use3D { get; set; } = true;
    public bool UseEdge { get; set; } = true;
    public bool WheelZoomEnabled { get; set; } = true;
    public double WheelZoomStep { get; set; } = 0.1;
    public double WheelZoomMax { get; set; } = 6.0;
    public double ImageZoom { get; set; } = 1.0;

    public int AlignSearchNum { get; set; } = 4;
    public int AlignActiveRoiIndex { get; set; } = 0;
    public int AlignSearchMargin { get; set; } = 50;
    public int AlignSearchSizeX { get; set; } = 103;
    public int AlignSearchSizeY { get; set; } = 102;
    public bool AlignSameSize { get; set; } = true;
    public RoiRect?[] AlignSearchRois { get; set; } = new RoiRect?[4];

    public bool AlignShiftEnabled { get; set; } = true;
    public double AlignShiftX { get; set; } = 1.0;
    public double AlignShiftY { get; set; } = 1.0;
    public bool AlignAngleEnabled { get; set; } = true;
    public double AlignAngle { get; set; } = 10.0;
    public bool AlignFillHole { get; set; }
    public int AlignFilter { get; set; } = 5;
    public bool AlignInspectionAreaCount { get; set; }

    public bool IpcUse { get; set; }
    public string IpcClass { get; set; } = "Class2";
    public double IpcPercent { get; set; } = 50.0;

    public bool PartTeachingUseCommonLibrary { get; set; } = true;
    public bool PartTeachingUseLibraryPart { get; set; }
    public bool PartTeachingUseAutoTeaching { get; set; } = true;
    public bool PartTeachingUseCadMatching { get; set; } = true;
    public string PartTeachingLibraryMatchMode { get; set; } = "None";
    public bool PartTeachingStopRequested { get; set; }
    public string PartTeachingLastMode { get; set; } = "Gerber";

    public void EnsureRoiSlots()
    {
        EnsureStructure();
    }

    public void EnsureStructure()
    {
        Part ??= new PartInspectionData();
        Part.Name = string.IsNullOrWhiteSpace(Part.Name) ? ModelName : Part.Name;
        Part.Windows ??= [];

        foreach (var window in Part.Windows)
        {
            window.Algorithms ??= [];
            foreach (var algorithm in window.Algorithms)
            {
                algorithm.ApplyCatalogDefaults();
                algorithm.Result ??= new InspectionResultData();
                algorithm.PanelData ??= AlgorithmPanelSchema.Create(algorithm);
            }
        }

        if (AlignSearchRois is { Length: 4 })
        {
            return;
        }

        var next = new RoiRect?[4];
        if (AlignSearchRois != null)
        {
            Array.Copy(AlignSearchRois, next, Math.Min(AlignSearchRois.Length, next.Length));
        }
        AlignSearchRois = next;
    }
}

public readonly record struct RoiRect(int X, int Y, int Width, int Height)
{
    public bool IsValid => Width > 0 && Height > 0;
}

public sealed class PartInspectionData
{
    public string Name { get; set; } = "Part-01";
    public double PixelResolutionX { get; set; }
    public double PixelResolutionY { get; set; }
    public int SourceWidth { get; set; }
    public int SourceHeight { get; set; }
    public List<InspectionWindowData> Windows { get; set; } = [];
}

public sealed class InspectionWindowData
{
    public string Id { get; set; } = CreateId();
    public string Name { get; set; } = "Window 1";
    public string TypeName { get; set; } = "Mount";
    public bool IsEnabled { get; set; } = true;
    public bool IsGroup { get; set; }
    public string GroupId { get; set; } = "";
    public RoiRect Roi { get; set; }
    public List<InspectionAlgorithmData> Algorithms { get; set; } = [];

    public static string CreateId() => $"WIN-{Guid.NewGuid():N}";
}

public sealed class InspectionAlgorithmData
{
    public string Id { get; set; } = CreateId();
    public string Type { get; set; } = "AlgoAlign";
    public string DisplayName { get; set; } = "Align";
    public LegacyAlgorithmGroup LegacyGroup { get; set; } = LegacyAlgorithmGroup.Normal;
    public int LegacyFlag { get; set; } = 8;
    public string LegacyName { get; set; } = "Align";
    public string ParameterFamily { get; set; } = "Align";
    public RoiRect? AlgorithmRoi { get; set; }
    public Dictionary<string, string> Parameters { get; set; } = [];
    public AlgorithmPanelData PanelData { get; set; } = new();
    public InspectionResultData Result { get; set; } = new();

    // Optional bridge result snapshot for Align. Used by the ROI overlay to draw
    // detected fiducial centers; null when bridge has not produced a result yet.
    public int[]? AlignDetectedCentersX { get; set; }
    public int[]? AlignDetectedCentersY { get; set; }
    public double? AlignOffsetX { get; set; }
    public double? AlignOffsetY { get; set; }
    public double? AlignTheta { get; set; }

    public static string CreateId() => $"ALG-{Guid.NewGuid():N}";

    public void ApplyCatalogDefaults()
    {
        var catalog = AlgorithmCatalog.Find(Type);
        Type = catalog.Type;
        DisplayName = catalog.DisplayName;
        LegacyGroup = catalog.Group;
        LegacyFlag = catalog.LegacyFlag;
        LegacyName = catalog.LegacyName;
        ParameterFamily = catalog.ParameterFamily;
        Parameters ??= [];
        PanelData ??= AlgorithmPanelSchema.Create(this);
        if (string.IsNullOrWhiteSpace(PanelData.AlgorithmType) || !string.Equals(PanelData.AlgorithmType, Type, StringComparison.OrdinalIgnoreCase))
        {
            PanelData = AlgorithmPanelSchema.Create(this);
        }
    }
}

public sealed class AlgorithmPanelData
{
    public string AlgorithmType { get; set; } = "AlgoAlign";
    public string PanelKind { get; set; } = "Dedicated";
    public string ActiveTab { get; set; } = "Common";
    public List<AlgorithmPanelTabData> Tabs { get; set; } = [];
}

public sealed class AlgorithmPanelTabData
{
    public string Id { get; set; } = "";
    public string Title { get; set; } = "";
    public List<string> ParameterKeys { get; set; } = [];
}

public sealed class InspectionResultData
{
    public string Message { get; set; } = "Not inspected";
    public double ElapsedMs { get; set; }
    public int ForegroundPixels { get; set; }
    public int BlobCount { get; set; }
    public RoiRect? Bounds { get; set; }
}

