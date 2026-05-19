using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using WpfInspectionApp.Commands;
using WpfInspectionApp.Infrastructure;

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

    public AlignPanelViewModel()
    {
        ActiveRoiCommand = ActionCommand(AlignPanelActionKind.ActiveRoi);
        DrawWindowRoiCommand = ActionCommand(AlignPanelActionKind.DrawWindowRoi);
        DrawAlgorithmRoiCommand = ActionCommand(AlignPanelActionKind.DrawAlgorithmRoi);
        TeachCommand = ActionCommand(AlignPanelActionKind.Teach);
        PartTeachingIcCommand = ActionCommand(AlignPanelActionKind.PartTeachingIc);
        PartTeachingOkCommand = ActionCommand(AlignPanelActionKind.PartTeachingOk);
        PartTeachingCloseCommand = ActionCommand(AlignPanelActionKind.PartTeachingClose);
    }

    public event EventHandler<AlignPanelActionRequestedEventArgs>? ActionRequested;

    public ICommand ActiveRoiCommand { get; }

    public ICommand DrawWindowRoiCommand { get; }

    public ICommand DrawAlgorithmRoiCommand { get; }

    public ICommand TeachCommand { get; }

    public ICommand PartTeachingIcCommand { get; }

    public ICommand PartTeachingOkCommand { get; }

    public ICommand PartTeachingCloseCommand { get; }

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

    public void LoadState(AlignPanelModelState state)
    {
        Threshold2D = state.Threshold2D;
        Threshold3D = state.Threshold3D;
        EdgeGain = state.EdgeGain;
        Use2D = state.Use2D;
        Use3D = state.Use3D;
        UseEdge = state.UseEdge;
        SearchNum = state.SearchNum;
        SearchMargin = state.SearchMargin;
        SearchSizeX = state.SearchSizeX;
        SearchSizeY = state.SearchSizeY;
        SameSize = state.SameSize;
        ShiftEnabled = state.ShiftEnabled;
        ShiftX = state.ShiftX;
        ShiftY = state.ShiftY;
        AngleEnabled = state.AngleEnabled;
        Angle = state.Angle;
        FillHole = state.FillHole;
        Filter = state.Filter;
        InspectionAreaCount = state.InspectionAreaCount;
        IpcUse = state.IpcUse;
        IpcClass = state.IpcClass;
        IpcPercent = state.IpcPercent;
        PartTeachingUseCommonLibrary = state.PartTeachingUseCommonLibrary;
        PartTeachingUseLibraryPart = state.PartTeachingUseLibraryPart;
        PartTeachingUseAutoTeaching = state.PartTeachingUseAutoTeaching;
        PartTeachingUseCadMatching = state.PartTeachingUseCadMatching;
        PartTeachingLibraryMatchMode = state.PartTeachingLibraryMatchMode;
    }

    public AlignPanelModelState CaptureModelState(string selectedAlgorithm)
    {
        return new AlignPanelModelState(
            selectedAlgorithm,
            Threshold2D,
            Threshold3D,
            EdgeGain,
            Use2D,
            Use3D,
            UseEdge,
            SearchNum,
            SearchMargin,
            SearchSizeX,
            SearchSizeY,
            SameSize,
            ShiftEnabled,
            ShiftX,
            ShiftY,
            AngleEnabled,
            Angle,
            FillHole,
            Filter,
            InspectionAreaCount,
            IpcUse,
            IpcClass,
            IpcPercent,
            PartTeachingUseCommonLibrary,
            PartTeachingUseLibraryPart,
            PartTeachingUseAutoTeaching,
            PartTeachingUseCadMatching,
            PartTeachingLibraryMatchMode);
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

    private RelayCommand ActionCommand(AlignPanelActionKind kind)
    {
        return new RelayCommand(() => ActionRequested?.Invoke(this, new AlignPanelActionRequestedEventArgs(kind)));
    }

}
