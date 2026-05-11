using System.Collections.ObjectModel;
using System.IO;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using WpfInspectionApp.Commands;
using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Models;
using WpfInspectionApp.Services;

namespace WpfInspectionApp.ViewModels;

public sealed class MainViewModel : ViewModelBase
{
    private InspectionModel _model;
    private string _statusMessage = "Ready.";
    private string _bridgeState = "C++ BRIDGE ACTIVE";
    private Brush _bridgeStateBrush = new SolidColorBrush(Color.FromRgb(24, 224, 123));
    private string _sourceInfoText = "NO IMAGE";
    private Brush _sourceInfoBrush = new SolidColorBrush(Color.FromRgb(24, 224, 123));
    private string _pttInfoText = "NO IMAGE";
    private Brush _pttInfoBrush = new SolidColorBrush(Color.FromRgb(139, 165, 196));
    private Visibility _pttEmptyVisibility = Visibility.Visible;
    private string _timingText = "0.00 ms";
    private string _inspectionResultText = "Waiting for inspection...";
    private bool _isInspectionRunning;
    private string _selectedAlgorithm = "AlgoAlign";
    private double _imageZoomPercent = 100;
    private ImageSource? _sourceImage;
    private ImageSource? _binaryImage;
    private object? _activeAlgorithmPanelContent;

    public MainViewModel(InspectionModel model)
    {
        _model = model;
        _model.EnsureStructure();
        _selectedAlgorithm = _model.Algorithm;
        AlgorithmTypes = new ObservableCollection<string>(AlgorithmCatalog.All.Select(item => item.Type));
        InspectionTreeNodes = [];

        LoadImageCommand = DisabledCommand();
        LoadPttCommand = DisabledCommand();
        SaveModelCommand = DisabledCommand();
        LoadModelCommand = DisabledCommand();
        ImportPartCommand = DisabledCommand();
        AddAlgorithmCommand = new RelayCommand(AddAlgorithm);
        RunInspectionCommand = DisabledCommand();
        ZoomOneCommand = DisabledCommand();
        ZoomFitCommand = DisabledCommand();
    }

    public InspectionModel Model
    {
        get => _model;
        set
        {
            if (SetProperty(ref _model, value))
            {
                _model.EnsureStructure();
                SelectedAlgorithm = _model.Algorithm;
                RefreshModelBindings();
            }
        }
    }

    public ObservableCollection<string> AlgorithmTypes { get; }

    public ObservableCollection<InspectionTreeNodeViewModel> InspectionTreeNodes { get; }

    public string StatusMessage
    {
        get => _statusMessage;
        set => SetProperty(ref _statusMessage, value);
    }

    public string BridgeState
    {
        get => _bridgeState;
        set => SetProperty(ref _bridgeState, value);
    }

    public Brush BridgeStateBrush
    {
        get => _bridgeStateBrush;
        set => SetProperty(ref _bridgeStateBrush, value);
    }

    public string SourceInfoText
    {
        get => _sourceInfoText;
        set => SetProperty(ref _sourceInfoText, value);
    }

    public Brush SourceInfoBrush
    {
        get => _sourceInfoBrush;
        set => SetProperty(ref _sourceInfoBrush, value);
    }

    public string PttInfoText
    {
        get => _pttInfoText;
        set => SetProperty(ref _pttInfoText, value);
    }

    public Brush PttInfoBrush
    {
        get => _pttInfoBrush;
        set => SetProperty(ref _pttInfoBrush, value);
    }

    public Visibility PttEmptyVisibility
    {
        get => _pttEmptyVisibility;
        set => SetProperty(ref _pttEmptyVisibility, value);
    }

    public string TimingText
    {
        get => _timingText;
        set => SetProperty(ref _timingText, value);
    }

    public string InspectionResultText
    {
        get => _inspectionResultText;
        set => SetProperty(ref _inspectionResultText, value);
    }

    public bool IsInspectionRunning
    {
        get => _isInspectionRunning;
        set
        {
            if (SetProperty(ref _isInspectionRunning, value))
            {
                OnPropertyChanged(nameof(CanRunInspection));
                if (RunInspectionCommand is AsyncRelayCommand command)
                {
                    command.RaiseCanExecuteChanged();
                }
            }
        }
    }

    public bool CanRunInspection => !IsInspectionRunning;

    public string ModelName
    {
        get => Model.ModelName;
        set
        {
            var next = string.IsNullOrWhiteSpace(value) ? "UnnamedModel" : value;
            if (Model.ModelName == next)
            {
                return;
            }

            Model.ModelName = next;
            Model.Part.Name = next;
            OnPropertyChanged();
            OnPropertyChanged(nameof(PartTitle));
        }
    }

    public string PartTitle => $"PART: {Model.Part.Name}";

    public string SelectedWindowText
    {
        get
        {
            var window = ActiveWindow;
            return window == null
                ? "Selected Window: none"
                : $"Selected Window: {window.Name} | {FormatRoi(window.Roi)} | Algorithms {window.Algorithms.Count}";
        }
    }

    public InspectionWindowData? ActiveWindow
    {
        get
        {
            Model.EnsureStructure();
            return Model.Part.Windows.FirstOrDefault(window => window.Id == Model.SelectedWindowId)
                ?? Model.Part.Windows.FirstOrDefault();
        }
    }

    public InspectionAlgorithmData? ActiveAlgorithm
    {
        get
        {
            var window = ActiveWindow;
            if (window == null)
            {
                return null;
            }

            return window.Algorithms.LastOrDefault(item => string.Equals(item.Type, SelectedAlgorithm, StringComparison.OrdinalIgnoreCase))
                ?? window.Algorithms.LastOrDefault();
        }
    }

    public string SelectedAlgorithm
    {
        get => _selectedAlgorithm;
        set
        {
            var next = AlgorithmCatalog.Find(value).Type;
            if (!SetProperty(ref _selectedAlgorithm, next))
            {
                return;
            }

            Model.Algorithm = next;
            OnPropertyChanged(nameof(SelectedAlgorithmText));
            OnPropertyChanged(nameof(ActiveAlgorithm));
        }
    }

    public string SelectedAlgorithmText
    {
        get
        {
            var catalog = AlgorithmCatalog.Find(SelectedAlgorithm);
            var profile = AlgorithmReferenceUiCatalog.Create(catalog);
            return $"Selected: {catalog.Type} | {catalog.DisplayName} | {catalog.Group}:{catalog.LegacyName} ({catalog.LegacyFlag}) | UI: {profile.SourceControl}";
        }
    }

    public bool WheelZoomEnabled
    {
        get => Model.WheelZoomEnabled;
        set
        {
            if (Model.WheelZoomEnabled == value)
            {
                return;
            }

            Model.WheelZoomEnabled = value;
            OnPropertyChanged();
        }
    }

    public double ImageZoom
    {
        get => Model.ImageZoom;
        set
        {
            var next = Net48Compat.Clamp(value, 1.0, ImageZoomMaximum);
            if (Math.Abs(Model.ImageZoom - next) < 0.0001)
            {
                return;
            }

            Model.ImageZoom = next;
            OnPropertyChanged();
            ImageZoomPercent = next * 100.0;
        }
    }

    public double ImageZoomPercent
    {
        get => _imageZoomPercent;
        set
        {
            if (SetProperty(ref _imageZoomPercent, value))
            {
                OnPropertyChanged(nameof(ImageZoomText));
            }
        }
    }

    public string ImageZoomText => $"{ImageZoomPercent:0}%";

    public ImageSource? SourceImage
    {
        get => _sourceImage;
        set => SetProperty(ref _sourceImage, value);
    }

    public ImageSource? BinaryImage
    {
        get => _binaryImage;
        set => SetProperty(ref _binaryImage, value);
    }

    public double ImageZoomMaximum
    {
        get => Math.Max(1.0, Model.WheelZoomMax);
        set
        {
            var next = Math.Max(1.0, value);
            if (Math.Abs(Model.WheelZoomMax - next) < 0.0001)
            {
                return;
            }

            Model.WheelZoomMax = next;
            OnPropertyChanged();
            ImageZoom = Model.ImageZoom;
        }
    }

    public ICommand LoadImageCommand { get; private set; }
    public ICommand LoadPttCommand { get; private set; }
    public ICommand SaveModelCommand { get; private set; }
    public ICommand LoadModelCommand { get; private set; }
    public ICommand ImportPartCommand { get; private set; }
    public ICommand AddAlgorithmCommand { get; private set; }
    public ICommand RunInspectionCommand { get; private set; }
    public ICommand ZoomOneCommand { get; private set; }
    public ICommand ZoomFitCommand { get; private set; }

    public object? ActiveAlgorithmPanelContent
    {
        get => _activeAlgorithmPanelContent;
        set => SetProperty(ref _activeAlgorithmPanelContent, value);
    }

    public event Action<string?>? TreeRefreshRequested;
    public event Action? AlgorithmPanelRefreshRequested;
    public event Action? SelectionChanged;

    public void ConfigureCommands(
        Action loadImage,
        Action loadPtt,
        Action saveModel,
        Action loadModel,
        Action importPart,
        Func<Task> runInspection,
        Action zoomOne,
        Action zoomFit)
    {
        LoadImageCommand = new RelayCommand(loadImage);
        LoadPttCommand = new RelayCommand(loadPtt);
        SaveModelCommand = new RelayCommand(saveModel);
        LoadModelCommand = new RelayCommand(loadModel);
        ImportPartCommand = new RelayCommand(importPart);
        RunInspectionCommand = new AsyncRelayCommand(runInspection, () => CanRunInspection);
        ZoomOneCommand = new RelayCommand(zoomOne);
        ZoomFitCommand = new RelayCommand(zoomFit);

        OnPropertyChanged(nameof(LoadImageCommand));
        OnPropertyChanged(nameof(LoadPttCommand));
        OnPropertyChanged(nameof(SaveModelCommand));
        OnPropertyChanged(nameof(LoadModelCommand));
        OnPropertyChanged(nameof(ImportPartCommand));
        OnPropertyChanged(nameof(RunInspectionCommand));
        OnPropertyChanged(nameof(ZoomOneCommand));
        OnPropertyChanged(nameof(ZoomFitCommand));
    }

    public void RefreshModelBindings()
    {
        OnPropertyChanged(nameof(Model));
        OnPropertyChanged(nameof(ModelName));
        OnPropertyChanged(nameof(PartTitle));
        OnPropertyChanged(nameof(SelectedWindowText));
        OnPropertyChanged(nameof(WheelZoomEnabled));
        OnPropertyChanged(nameof(ImageZoom));
        OnPropertyChanged(nameof(ImageZoomMaximum));
        ImageZoomPercent = Model.ImageZoom * 100.0;
        OnPropertyChanged(nameof(ImageZoomText));
        OnPropertyChanged(nameof(SelectedAlgorithmText));
        OnPropertyChanged(nameof(ActiveWindow));
        OnPropertyChanged(nameof(ActiveAlgorithm));
    }

    public void MarkImageLoaded(int width, int height)
    {
        SourceInfoText = $"{width} x {height}";
        SourceInfoBrush = new SolidColorBrush(Color.FromRgb(24, 224, 123));
    }

    public void ApplyImageLoad(ImageSource sourceImage, ImageSource binaryImage, int width, int height, string statusMessage)
    {
        SourceImage = sourceImage;
        BinaryImage = binaryImage;
        MarkImageLoaded(width, height);
        StatusMessage = statusMessage;
    }

    public void MarkPttLoaded(string path)
    {
        PttEmptyVisibility = Visibility.Collapsed;
        PttInfoText = Path.GetFileName(path);
        PttInfoBrush = new SolidColorBrush(Color.FromRgb(24, 224, 123));
    }

    public void MarkPttLoadFailed()
    {
        PttInfoText = "LOAD FAILED";
        PttInfoBrush = new SolidColorBrush(Color.FromRgb(255, 176, 32));
    }

    public void ApplyPttLoad(bool success, string path, string statusMessage)
    {
        if (success)
        {
            MarkPttLoaded(path);
        }
        else
        {
            MarkPttLoadFailed();
        }

        StatusMessage = statusMessage;
    }

    public void MarkBridgeState(bool usedNative)
    {
        BridgeState = usedNative ? "C++ BRIDGE ACTIVE" : "C++ BRIDGE FALLBACK";
        BridgeStateBrush = usedNative
            ? new SolidColorBrush(Color.FromRgb(24, 224, 123))
            : new SolidColorBrush(Color.FromRgb(255, 176, 32));
    }

    public void ApplyThresholdPreview(ImageSource? binaryImage, bool usedNative, string timingText, string statusMessage, string resultText)
    {
        BinaryImage = binaryImage;
        MarkBridgeState(usedNative);
        TimingText = timingText;
        StatusMessage = statusMessage;
        InspectionResultText = resultText;
    }

    public void BeginInspectionRun()
    {
        StatusMessage = "Part inspection running...";
        IsInspectionRunning = true;
    }

    public void ApplyInspectionRun(PartInspectionWorkflowResult result)
    {
        if (!string.IsNullOrWhiteSpace(result.TimingText))
        {
            TimingText = result.TimingText;
        }

        StatusMessage = result.StatusMessage;
        InspectionResultText = result.ResultText;
    }

    public void ApplyInspectionFailure(Exception exception)
    {
        StatusMessage = $"Part inspection failed: {exception.Message}";
        InspectionResultText = exception.ToString();
    }

    public void RefreshInspectionTree(string? selectedId, int sourceWidth, int sourceHeight)
    {
        Model.EnsureStructure();
        selectedId ??= Model.SelectedWindowId;
        InspectionTreeNodes.Clear();

        for (var index = 0; index < Model.Part.Windows.Count; index++)
        {
            var window = Model.Part.Windows[index];
            var windowNode = new InspectionTreeNodeViewModel
            {
                Header = window.Name,
                Kind = InspectionTreeNodeKind.Window,
                Payload = window,
                IsSelected = selectedId == window.Id
            };
            InspectionTreeNodes.Add(windowNode);

            windowNode.Children.Add(new InspectionTreeNodeViewModel
            {
                Header = $"{window.Name}: {FormatRoi(window.Roi, sourceWidth, sourceHeight)}",
                Kind = InspectionTreeNodeKind.WindowInfo,
                Payload = window.Roi,
                Foreground = new SolidColorBrush(Color.FromRgb(255, 210, 41)),
                IsEnabled = false
            });

            foreach (var algorithm in window.Algorithms)
            {
                algorithm.ApplyCatalogDefaults();
                var result = algorithm.Result ?? new InspectionResultData();
                var algorithmRoi = algorithm.AlgorithmRoi.HasValue
                    ? FormatRoi(algorithm.AlgorithmRoi, sourceWidth, sourceHeight)
                    : "none - uses Window ROI unless assigned";
                var algorithmNode = new InspectionTreeNodeViewModel
                {
                    Header = $"{algorithm.Type} | {algorithm.DisplayName} | {algorithm.LegacyGroup}:{algorithm.LegacyName} ({algorithm.LegacyFlag})",
                    Kind = InspectionTreeNodeKind.Algorithm,
                    Payload = algorithm,
                    IsSelected = selectedId == algorithm.Id
                };
                windowNode.Children.Add(algorithmNode);
                algorithmNode.Children.Add(new InspectionTreeNodeViewModel
                {
                    Header = $"Algorithm ROI: {algorithmRoi}",
                    Kind = InspectionTreeNodeKind.AlgorithmRoi,
                    Payload = algorithm.AlgorithmRoi,
                    Foreground = new SolidColorBrush(Color.FromRgb(128, 223, 255)),
                    IsEnabled = false
                });
                algorithmNode.Children.Add(new InspectionTreeNodeViewModel
                {
                    Header = $"Inspection Result: {result.Message} | FG {result.ForegroundPixels:N0} | {result.ElapsedMs:F3} ms",
                    Kind = InspectionTreeNodeKind.InspectionResult,
                    Payload = result,
                    IsEnabled = false
                });
            }
        }

        if (Model.Part.Windows.Count == 0)
        {
            InspectionTreeNodes.Add(new InspectionTreeNodeViewModel
            {
                Header = "Window: none - draw ROI to create Window",
                Kind = InspectionTreeNodeKind.Empty,
                IsEnabled = false
            });
        }

        RefreshModelBindings();
    }

    public bool SelectTreeNode(InspectionTreeNodeViewModel? node)
    {
        if (node?.Payload is InspectionWindowData window)
        {
            Model.SelectedWindowId = window.Id;
            RefreshModelBindings();
            SelectionChanged?.Invoke();
            return true;
        }

        if (node?.Payload is InspectionAlgorithmData algorithm)
        {
            var owner = Model.Part.Windows.FirstOrDefault(candidate => candidate.Algorithms.Any(child => child.Id == algorithm.Id));
            if (owner == null)
            {
                return false;
            }

            Model.SelectedWindowId = owner.Id;
            if (AlgorithmTypes.Contains(algorithm.Type))
            {
                SelectedAlgorithm = algorithm.Type;
            }

            RefreshModelBindings();
            SelectionChanged?.Invoke();
            return true;
        }

        return false;
    }

    private void AddAlgorithm()
    {
        Model.EnsureStructure();
        var window = Model.Part.Windows.FirstOrDefault(candidate => candidate.Id == Model.SelectedWindowId)
            ?? Model.Part.Windows.FirstOrDefault();
        if (window == null)
        {
            StatusMessage = "Draw and select a Window ROI before adding an Algorithm.";
            return;
        }

        var catalog = AlgorithmCatalog.Find(SelectedAlgorithm);
        var algorithm = new InspectionAlgorithmData
        {
            Type = catalog.Type,
            DisplayName = catalog.DisplayName,
            LegacyGroup = catalog.Group,
            LegacyFlag = catalog.LegacyFlag,
            LegacyName = catalog.LegacyName,
            ParameterFamily = catalog.ParameterFamily,
            PanelData = AlgorithmPanelSchema.Create(new InspectionAlgorithmData { Type = catalog.Type }),
            Result = new InspectionResultData()
        };
        algorithm.ApplyCatalogDefaults();
        window.Algorithms.Add(algorithm);
        Model.SelectedWindowId = window.Id;

        StatusMessage = $"{algorithm.Type} added to {window.Name}.";
        TreeRefreshRequested?.Invoke(algorithm.Id);
        AlgorithmPanelRefreshRequested?.Invoke();
        RefreshModelBindings();
    }

    private string FormatRoi(RoiRect? roi)
    {
        return FormatRoi(roi, Model.Part.SourceWidth, Model.Part.SourceHeight);
    }

    private static string FormatRoi(RoiRect? roi, int sourceWidth, int sourceHeight)
    {
        if (!roi.HasValue)
        {
            return "none";
        }

        var value = roi.Value;
        var centerX = value.X + value.Width / 2;
        var centerY = value.Y + value.Height / 2;
        return $"X {value.X} Y {value.Y} W {value.Width} H {value.Height} | CX {centerX} CY {centerY} | ORG {sourceWidth}x{sourceHeight}";
    }

    private static ICommand DisabledCommand()
    {
        return new RelayCommand(() => { }, () => false);
    }
}
