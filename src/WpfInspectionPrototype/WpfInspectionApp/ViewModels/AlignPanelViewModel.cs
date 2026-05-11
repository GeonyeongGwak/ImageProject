using System.Windows;
using System.Windows.Media;
using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.ViewModels;

public sealed class AlignPanelViewModel : ViewModelBase
{
    private int _threshold2D = 128;
    private int _threshold3D = 96;
    private int _edgeGain = 42;
    private bool _use2D = true;
    private bool _use3D = true;
    private bool _useEdge = true;
    private string _searchNum = "4";
    private string _searchMargin = "50";
    private string _searchSizeX = "103";
    private string _searchSizeY = "102";
    private bool _sameSize = true;
    private bool _shiftEnabled = true;
    private string _shiftX = "1.00";
    private string _shiftY = "1.00";
    private bool _angleEnabled = true;
    private string _angle = "10";
    private bool _fillHole;
    private string _filter = "5";
    private bool _inspectionAreaCount;
    private bool _ipcUse;
    private string _ipcClass = "Class2";
    private string _ipcPercent = "50.00";
    private string _partTeachingLibraryMatchMode = "None";
    private bool _partTeachingUseCommonLibrary = true;
    private bool _partTeachingUseLibraryPart;
    private bool _partTeachingUseAutoTeaching = true;
    private bool _partTeachingUseCadMatching = true;
    private string _maskDensityText = "62%";
    private string _activeRoiText = "ROI - 1";
    private string _roiText = "ROI: none";
    private string _drawWindowRoiText = "Draw Window ROI (A)";
    private Brush _drawWindowRoiBrush = new SolidColorBrush(Color.FromRgb(16, 36, 61));
    private string _partTeachingStatus = "Part Teaching ready.";
    private Visibility _partTeachingIcVisibility = Visibility.Visible;
    private Visibility _partTeachingOkVisibility = Visibility.Collapsed;

    public int Threshold2D
    {
        get => _threshold2D;
        set
        {
            var next = Net48Compat.Clamp(value, 0, 255);
            if (SetProperty(ref _threshold2D, next))
            {
                OnPropertyChanged(nameof(Threshold2DText));
            }
        }
    }

    public string Threshold2DText => Threshold2D.ToString();

    public int Threshold3D
    {
        get => _threshold3D;
        set
        {
            var next = Net48Compat.Clamp(value, 0, 255);
            if (SetProperty(ref _threshold3D, next))
            {
                OnPropertyChanged(nameof(Threshold3DText));
            }
        }
    }

    public string Threshold3DText => Threshold3D.ToString();

    public int EdgeGain
    {
        get => _edgeGain;
        set
        {
            var next = Net48Compat.Clamp(value, 0, 100);
            if (SetProperty(ref _edgeGain, next))
            {
                OnPropertyChanged(nameof(EdgeGainText));
            }
        }
    }

    public string EdgeGainText => $"{EdgeGain}%";

    public bool Use2D
    {
        get => _use2D;
        set => SetProperty(ref _use2D, value);
    }

    public bool Use3D
    {
        get => _use3D;
        set => SetProperty(ref _use3D, value);
    }

    public bool UseEdge
    {
        get => _useEdge;
        set => SetProperty(ref _useEdge, value);
    }

    public string SearchNum
    {
        get => _searchNum;
        set => SetProperty(ref _searchNum, string.IsNullOrWhiteSpace(value) ? "1" : value);
    }

    public string SearchMargin
    {
        get => _searchMargin;
        set => SetProperty(ref _searchMargin, value);
    }

    public string SearchSizeX
    {
        get => _searchSizeX;
        set => SetProperty(ref _searchSizeX, value);
    }

    public string SearchSizeY
    {
        get => _searchSizeY;
        set => SetProperty(ref _searchSizeY, value);
    }

    public bool SameSize
    {
        get => _sameSize;
        set => SetProperty(ref _sameSize, value);
    }

    public bool ShiftEnabled
    {
        get => _shiftEnabled;
        set => SetProperty(ref _shiftEnabled, value);
    }

    public string ShiftX
    {
        get => _shiftX;
        set => SetProperty(ref _shiftX, value);
    }

    public string ShiftY
    {
        get => _shiftY;
        set => SetProperty(ref _shiftY, value);
    }

    public bool AngleEnabled
    {
        get => _angleEnabled;
        set => SetProperty(ref _angleEnabled, value);
    }

    public string Angle
    {
        get => _angle;
        set => SetProperty(ref _angle, value);
    }

    public bool FillHole
    {
        get => _fillHole;
        set => SetProperty(ref _fillHole, value);
    }

    public string Filter
    {
        get => _filter;
        set => SetProperty(ref _filter, value);
    }

    public bool InspectionAreaCount
    {
        get => _inspectionAreaCount;
        set => SetProperty(ref _inspectionAreaCount, value);
    }

    public bool IpcUse
    {
        get => _ipcUse;
        set => SetProperty(ref _ipcUse, value);
    }

    public string IpcClass
    {
        get => _ipcClass;
        set => SetProperty(ref _ipcClass, string.IsNullOrWhiteSpace(value) ? "Class2" : value);
    }

    public string IpcPercent
    {
        get => _ipcPercent;
        set => SetProperty(ref _ipcPercent, value);
    }

    public string PartTeachingLibraryMatchMode
    {
        get => _partTeachingLibraryMatchMode;
        set => SetProperty(ref _partTeachingLibraryMatchMode, string.IsNullOrWhiteSpace(value) ? "None" : value);
    }

    public bool PartTeachingUseCommonLibrary
    {
        get => _partTeachingUseCommonLibrary;
        set => SetProperty(ref _partTeachingUseCommonLibrary, value);
    }

    public bool PartTeachingUseLibraryPart
    {
        get => _partTeachingUseLibraryPart;
        set => SetProperty(ref _partTeachingUseLibraryPart, value);
    }

    public bool PartTeachingUseAutoTeaching
    {
        get => _partTeachingUseAutoTeaching;
        set => SetProperty(ref _partTeachingUseAutoTeaching, value);
    }

    public bool PartTeachingUseCadMatching
    {
        get => _partTeachingUseCadMatching;
        set => SetProperty(ref _partTeachingUseCadMatching, value);
    }

    public string MaskDensityText
    {
        get => _maskDensityText;
        set => SetProperty(ref _maskDensityText, value);
    }

    public string ActiveRoiText
    {
        get => _activeRoiText;
        set => SetProperty(ref _activeRoiText, value);
    }

    public string RoiText
    {
        get => _roiText;
        set => SetProperty(ref _roiText, value);
    }

    public string DrawWindowRoiText
    {
        get => _drawWindowRoiText;
        set => SetProperty(ref _drawWindowRoiText, value);
    }

    public Brush DrawWindowRoiBrush
    {
        get => _drawWindowRoiBrush;
        set => SetProperty(ref _drawWindowRoiBrush, value);
    }

    public string PartTeachingStatus
    {
        get => _partTeachingStatus;
        set => SetProperty(ref _partTeachingStatus, value);
    }

    public Visibility PartTeachingIcVisibility
    {
        get => _partTeachingIcVisibility;
        set => SetProperty(ref _partTeachingIcVisibility, value);
    }

    public Visibility PartTeachingOkVisibility
    {
        get => _partTeachingOkVisibility;
        set => SetProperty(ref _partTeachingOkVisibility, value);
    }

    public void LoadFromModel(InspectionModel model)
    {
        Threshold2D = model.Threshold2D;
        Threshold3D = model.Threshold3D;
        EdgeGain = model.EdgeGain;
        Use2D = model.Use2D;
        Use3D = model.Use3D;
        UseEdge = model.UseEdge;
        SearchNum = Net48Compat.Clamp(model.AlignSearchNum, 1, 4).ToString();
        SearchMargin = model.AlignSearchMargin.ToString();
        SearchSizeX = model.AlignSearchSizeX.ToString();
        SearchSizeY = model.AlignSearchSizeY.ToString();
        SameSize = model.AlignSameSize;
        ShiftEnabled = model.AlignShiftEnabled;
        ShiftX = model.AlignShiftX.ToString("0.00");
        ShiftY = model.AlignShiftY.ToString("0.00");
        AngleEnabled = model.AlignAngleEnabled;
        Angle = model.AlignAngle.ToString("0.##");
        FillHole = model.AlignFillHole;
        Filter = model.AlignFilter.ToString();
        InspectionAreaCount = model.AlignInspectionAreaCount;
        IpcUse = model.IpcUse;
        IpcClass = model.IpcClass;
        IpcPercent = model.IpcPercent.ToString("0.00");
        PartTeachingUseCommonLibrary = model.PartTeachingUseCommonLibrary;
        PartTeachingUseLibraryPart = model.PartTeachingUseLibraryPart;
        PartTeachingUseAutoTeaching = model.PartTeachingUseAutoTeaching;
        PartTeachingUseCadMatching = model.PartTeachingUseCadMatching;
        PartTeachingLibraryMatchMode = model.PartTeachingLibraryMatchMode;
    }

    public void ApplyToModel(InspectionModel model, int sourceWidth, int sourceHeight)
    {
        model.Threshold2D = Threshold2D;
        model.Threshold3D = Threshold3D;
        model.EdgeGain = EdgeGain;
        model.Use2D = Use2D;
        model.Use3D = Use3D;
        model.UseEdge = UseEdge;
        model.AlignSearchNum = Net48Compat.Clamp(ReadInt(SearchNum, model.AlignSearchNum, 1, 4), 1, 4);
        model.AlignSearchMargin = ReadInt(SearchMargin, model.AlignSearchMargin, 0, 100000);
        model.AlignSearchSizeX = ReadInt(SearchSizeX, model.AlignSearchSizeX, 1, Math.Max(1, sourceWidth));
        model.AlignSearchSizeY = ReadInt(SearchSizeY, model.AlignSearchSizeY, 1, Math.Max(1, sourceHeight));
        model.AlignSameSize = SameSize;
        model.AlignShiftEnabled = ShiftEnabled;
        model.AlignShiftX = ReadDouble(ShiftX, model.AlignShiftX);
        model.AlignShiftY = ReadDouble(ShiftY, model.AlignShiftY);
        model.AlignAngleEnabled = AngleEnabled;
        model.AlignAngle = ReadDouble(Angle, model.AlignAngle);
        model.AlignFillHole = FillHole;
        model.AlignFilter = ReadInt(Filter, model.AlignFilter, 0, 100000);
        model.AlignInspectionAreaCount = InspectionAreaCount;
        model.IpcUse = IpcUse;
        model.IpcClass = IpcClass;
        model.IpcPercent = ReadDouble(IpcPercent, model.IpcPercent);
        model.PartTeachingUseCommonLibrary = PartTeachingUseCommonLibrary;
        model.PartTeachingUseLibraryPart = PartTeachingUseLibraryPart;
        model.PartTeachingUseAutoTeaching = PartTeachingUseAutoTeaching;
        model.PartTeachingUseCadMatching = PartTeachingUseCadMatching;
        model.PartTeachingLibraryMatchMode = PartTeachingLibraryMatchMode;
    }

    public void SetWindowRoiDrawingState(bool isDrawingWindow)
    {
        DrawWindowRoiBrush = isDrawingWindow
            ? new SolidColorBrush(Color.FromRgb(255, 176, 32))
            : new SolidColorBrush(Color.FromRgb(16, 36, 61));
        DrawWindowRoiText = isDrawingWindow
            ? "Drawing Window... (A)"
            : "Draw Window ROI (A)";
    }

    public void UpdatePartTeachingUi()
    {
        if (!PartTeachingUseCommonLibrary && !PartTeachingUseLibraryPart)
        {
            PartTeachingUseCommonLibrary = true;
        }

        var useIcMode = PartTeachingUseAutoTeaching && !PartTeachingUseLibraryPart;
        PartTeachingIcVisibility = useIcMode ? Visibility.Visible : Visibility.Collapsed;
        PartTeachingOkVisibility = useIcMode ? Visibility.Collapsed : Visibility.Visible;
        PartTeachingStatus =
            $"Mode: {(useIcMode ? "FullMap/IC" : "Gerber/Part")} | " +
            $"Library: {(PartTeachingUseCommonLibrary ? "Common" : "")}" +
            $"{(PartTeachingUseLibraryPart ? " Part" : "")} | " +
            $"Match: {PartTeachingLibraryMatchMode}";
    }

    public void SetSearchNum(int value)
    {
        SearchNum = Net48Compat.Clamp(value, 1, 4).ToString();
    }

    public void SetSearchSize(int width, int height)
    {
        SearchSizeX = width.ToString();
        SearchSizeY = height.ToString();
    }

    public void MirrorSearchSizeFromX()
    {
        if (SameSize)
        {
            SearchSizeY = SearchSizeX;
        }
    }

    public void MirrorSearchSizeFromY()
    {
        if (SameSize)
        {
            SearchSizeX = SearchSizeY;
        }
    }

    public void ClosePartTeaching()
    {
        PartTeachingStatus = "Part Teaching closed. Stop requested.";
    }

    private static int ReadInt(string text, int fallback, int min, int max)
    {
        return int.TryParse(text, out var value) ? Net48Compat.Clamp(value, min, max) : fallback;
    }

    private static double ReadDouble(string text, double fallback)
    {
        return double.TryParse(text, out var value) ? value : fallback;
    }
}
