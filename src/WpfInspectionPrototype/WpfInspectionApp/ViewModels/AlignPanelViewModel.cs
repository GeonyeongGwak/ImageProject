using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using WpfInspectionApp.Commands;
using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Services;

namespace WpfInspectionApp.ViewModels;

public sealed class AlignPanelViewModel : ViewModelBase
{
    private readonly IAlignPanelStateService _stateService;
    private int _threshold2D = 128;
    private int _threshold2DMax = 255;
    private int _threshold3D = 96;
    private int _threshold3DMax = 120;
    private int _edgeGain = 42;
    private bool _use2D = true;
    private bool _use3D = true;
    private bool _useEdge = true;
    private int _range2DType = 2;
    private int _range3DType = 2;
    private bool _invertCheck;
    private string _heightAverage = "0.00";
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
        : this(new AlignPanelStateService())
    {
    }

    internal AlignPanelViewModel(IAlignPanelStateService stateService)
    {
        _stateService = stateService ?? throw new ArgumentNullException(nameof(stateService));
        ActiveRoiCommand = ActionCommand(AlignPanelActionKind.ActiveRoi);
        DrawWindowRoiCommand = ActionCommand(AlignPanelActionKind.DrawWindowRoi);
        DrawAlgorithmRoiCommand = ActionCommand(AlignPanelActionKind.DrawAlgorithmRoi);
        TeachCommand = ActionCommand(AlignPanelActionKind.Teach);
        PartTeachingIcCommand = ActionCommand(AlignPanelActionKind.PartTeachingIc);
        PartTeachingOkCommand = ActionCommand(AlignPanelActionKind.PartTeachingOk);
        PartTeachingCloseCommand = ActionCommand(AlignPanelActionKind.PartTeachingClose);
        NotifyAllControlStateChanged();
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
            next = Math.Min(next, _threshold2DMax);
            if (SetProperty(ref _threshold2D, next))
            {
                OnPropertyChanged(nameof(Threshold2DText));
            }
        }
    }

    public string Threshold2DText => Threshold2D.ToString();

    public int Threshold2DMax
    {
        get => _threshold2DMax;
        set
        {
            var next = Net48Compat.Clamp(value, 0, 255);
            next = Math.Max(next, _threshold2D);
            if (SetProperty(ref _threshold2DMax, next))
            {
                OnPropertyChanged(nameof(Threshold2DMaxText));
            }
        }
    }

    public string Threshold2DMaxText => Threshold2DMax.ToString();

    public int Threshold3D
    {
        get => _threshold3D;
        set
        {
            var next = Net48Compat.Clamp(value, 0, 255);
            next = Math.Min(next, _threshold3DMax);
            if (SetProperty(ref _threshold3D, next))
            {
                OnPropertyChanged(nameof(Threshold3DText));
            }
        }
    }

    public string Threshold3DText => Threshold3D.ToString();

    public int Threshold3DMax
    {
        get => _threshold3DMax;
        set
        {
            var next = Net48Compat.Clamp(value, 0, 255);
            next = Math.Max(next, _threshold3D);
            if (SetProperty(ref _threshold3DMax, next))
            {
                OnPropertyChanged(nameof(Threshold3DMaxText));
            }
        }
    }

    public string Threshold3DMaxText => Threshold3DMax.ToString();

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
        set
        {
            if (SetProperty(ref _use2D, value))
            {
                KeepInspectionSourceEnabled(AlignPanelInspectionToggle.Use2D);
            }
        }
    }

    public bool Use3D
    {
        get => _use3D;
        set
        {
            if (SetProperty(ref _use3D, value))
            {
                KeepInspectionSourceEnabled(AlignPanelInspectionToggle.Use3D);
            }
        }
    }

    public bool UseEdge
    {
        get => _useEdge;
        set
        {
            if (SetProperty(ref _useEdge, value))
            {
                KeepInspectionSourceEnabled(AlignPanelInspectionToggle.UseEdge);
            }
        }
    }

    public bool Is2DConditionEnabled => Use2D;

    public bool Is3DConditionEnabled => Use3D;

    public bool IsEdgeConditionEnabled => UseEdge;

    public int Range2DType
    {
        get => _range2DType;
        set => SetProperty(ref _range2DType, Net48Compat.Clamp(value, 0, 3));
    }

    public int Range3DType
    {
        get => _range3DType;
        set => SetProperty(ref _range3DType, Net48Compat.Clamp(value, 0, 3));
    }

    public bool InvertCheck
    {
        get => _invertCheck;
        set => SetProperty(ref _invertCheck, value);
    }

    public string HeightAverage
    {
        get => _heightAverage;
        set => SetProperty(ref _heightAverage, value ?? "0.00");
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
        set
        {
            if (SetProperty(ref _searchSizeX, value) && SameSize)
            {
                SearchSizeY = _searchSizeX;
            }
        }
    }

    public string SearchSizeY
    {
        get => _searchSizeY;
        set => SetProperty(ref _searchSizeY, value);
    }

    public bool SameSize
    {
        get => _sameSize;
        set
        {
            if (SetProperty(ref _sameSize, value))
            {
                if (_sameSize)
                {
                    SearchSizeY = SearchSizeX;
                }

                OnPropertyChanged(nameof(IsSearchSizeYEnabled));
            }
        }
    }

    public bool IsSearchSizeYEnabled => !SameSize;

    public bool ShiftEnabled
    {
        get => _shiftEnabled;
        set
        {
            if (SetProperty(ref _shiftEnabled, value))
            {
                KeepCorrectionOptionEnabled(AlignPanelCorrectionToggle.Shift);
            }
        }
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
        set
        {
            if (SetProperty(ref _angleEnabled, value))
            {
                KeepCorrectionOptionEnabled(AlignPanelCorrectionToggle.Angle);
            }
        }
    }

    public bool IsShiftInputEnabled => ShiftEnabled && !IpcUse;

    public bool IsAngleInputEnabled => AngleEnabled;

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
        set
        {
            if (SetProperty(ref _ipcUse, value))
            {
                OnPropertyChanged(nameof(IsIpcClassEnabled));
                OnPropertyChanged(nameof(IsIpcPercentEnabled));
                OnPropertyChanged(nameof(IsShiftInputEnabled));
            }
        }
    }

    public bool IsIpcClassEnabled => IpcUse;

    public bool IsIpcPercentEnabled => IpcUse;

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
        Threshold2DMax = Math.Max(state.Threshold2DMax, state.Threshold2D);
        Threshold2D = state.Threshold2D;
        Threshold3DMax = Math.Max(state.Threshold3DMax, state.Threshold3D);
        Threshold3D = state.Threshold3D;
        EdgeGain = state.EdgeGain;
        Use2D = state.Use2D;
        Use3D = state.Use3D;
        UseEdge = state.UseEdge;
        Range2DType = state.Range2DType;
        Range3DType = state.Range3DType;
        InvertCheck = state.InvertCheck;
        HeightAverage = state.HeightAverage;
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
        NormalizeEditableValues();
        NotifyAllControlStateChanged();
    }

    public AlignPanelModelState CaptureModelState(string selectedAlgorithm)
    {
        NormalizeEditableValues();
        NotifyAllControlStateChanged();

        return new AlignPanelModelState(
            selectedAlgorithm,
            Threshold2D,
            Threshold2DMax,
            Threshold3D,
            Threshold3DMax,
            EdgeGain,
            Use2D,
            Use3D,
            UseEdge,
            Range2DType,
            Range3DType,
            InvertCheck,
            HeightAverage,
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
        SearchSizeY = SameSize ? SearchSizeX : height.ToString();
    }

    public void SetMaskDensity(int maskDensity)
    {
        MaskDensityText = $"{maskDensity}%";
    }

    public void ClosePartTeaching()
    {
        PartTeachingStatus = "Part Teaching closed. Stop requested.";
    }

    private RelayCommand ActionCommand(AlignPanelActionKind kind)
    {
        return new RelayCommand(() => ActionRequested?.Invoke(this, new AlignPanelActionRequestedEventArgs(kind)));
    }

    private void KeepInspectionSourceEnabled(AlignPanelInspectionToggle changedToggle)
    {
        var state = _stateService.KeepInspectionSourceEnabled(_use2D, _use3D, _useEdge, changedToggle);
        SetProperty(ref _use2D, state.Use2D, nameof(Use2D));
        SetProperty(ref _use3D, state.Use3D, nameof(Use3D));
        SetProperty(ref _useEdge, state.UseEdge, nameof(UseEdge));
        NotifyInspectionControlStateChanged();
    }

    private void KeepCorrectionOptionEnabled(AlignPanelCorrectionToggle changedToggle)
    {
        var state = _stateService.KeepCorrectionOptionEnabled(_shiftEnabled, _angleEnabled, changedToggle);
        SetProperty(ref _shiftEnabled, state.ShiftEnabled, nameof(ShiftEnabled));
        SetProperty(ref _angleEnabled, state.AngleEnabled, nameof(AngleEnabled));
        NotifyCorrectionControlStateChanged();
    }

    private void NormalizeEditableValues()
    {
        SearchNum = _stateService.NormalizeIntegerText(SearchNum, "4", 1, 4);
        SearchMargin = _stateService.NormalizeIntegerText(SearchMargin, "50", 0, 100000);
        SearchSizeX = _stateService.NormalizeIntegerText(SearchSizeX, "1", 1, 100000);
        SearchSizeY = SameSize
            ? SearchSizeX
            : _stateService.NormalizeIntegerText(SearchSizeY, "1", 1, 100000);
        HeightAverage = _stateService.NormalizeDoubleText(HeightAverage, "0.00", 0, 1000000, "0.00");
        ShiftX = _stateService.NormalizeDoubleText(ShiftX, "1.00", 0, 100000, "0.00");
        ShiftY = _stateService.NormalizeDoubleText(ShiftY, "1.00", 0, 100000, "0.00");
        Angle = _stateService.NormalizeDoubleText(Angle, "10", 0, 360, "0.##");
        Filter = _stateService.NormalizeIntegerText(Filter, "5", 1, 100000);
        IpcClass = _stateService.NormalizeIpcClass(IpcClass);
        IpcPercent = _stateService.NormalizeDoubleText(IpcPercent, "50.00", 0, 100, "0.00");
    }

    private void NotifyAllControlStateChanged()
    {
        NotifyInspectionControlStateChanged();
        OnPropertyChanged(nameof(IsSearchSizeYEnabled));
        NotifyCorrectionControlStateChanged();
        OnPropertyChanged(nameof(IsIpcClassEnabled));
        OnPropertyChanged(nameof(IsIpcPercentEnabled));
    }

    private void NotifyInspectionControlStateChanged()
    {
        OnPropertyChanged(nameof(Is2DConditionEnabled));
        OnPropertyChanged(nameof(Is3DConditionEnabled));
        OnPropertyChanged(nameof(IsEdgeConditionEnabled));
    }

    private void NotifyCorrectionControlStateChanged()
    {
        OnPropertyChanged(nameof(IsShiftInputEnabled));
        OnPropertyChanged(nameof(IsAngleInputEnabled));
    }

}
