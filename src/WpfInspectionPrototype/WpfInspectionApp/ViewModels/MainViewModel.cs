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

    private readonly IDialogOwner _dialogOwner;
    private readonly IFileDialogService _fileDialogService;
    private readonly IModelWorkflowService _modelWorkflowService;
    private readonly IApplicationPathService _applicationPathService;
    private readonly RoiCanvasViewModel _roi;
    private readonly IImageRuntimeStateService _imageRuntimeStateService;
    private readonly IInspectionWorkflowService _inspectionWorkflowService;
    private readonly IInspectionFlowService _inspectionFlowService;
    private readonly IThresholdPreviewWorkflowService _thresholdPreviewWorkflowService;
    private string? _lastFlowPttPath;
    private AlgorithmPanelFactory? _algorithmPanelFactory;
    private IAlgorithmPanel? _activeAlgorithmPanel;
    private bool _alignSearchTabActive;
    private bool _refreshingTree;

    public MainViewModel(
        InspectionModel model,
        IDialogOwner dialogOwner,
        IFileDialogService fileDialogService,
        IModelWorkflowService modelWorkflowService,
        IApplicationPathService applicationPathService,
        RoiCanvasViewModel roi,
        IImageRuntimeStateService imageRuntimeStateService,
        IInspectionWorkflowService inspectionWorkflowService,
        IInspectionFlowService inspectionFlowService,
        IThresholdPreviewWorkflowService thresholdPreviewWorkflowService)
    {
        _model = model;
        _model.EnsureStructure();
        _selectedAlgorithm = _model.Algorithm;
        _dialogOwner = dialogOwner;
        _fileDialogService = fileDialogService;
        _modelWorkflowService = modelWorkflowService;
        _applicationPathService = applicationPathService;
        _roi = roi;
        _imageRuntimeStateService = imageRuntimeStateService;
        _inspectionWorkflowService = inspectionWorkflowService;
        _inspectionFlowService = inspectionFlowService;
        _thresholdPreviewWorkflowService = thresholdPreviewWorkflowService;
        AlgorithmTypes = new ObservableCollection<string>(AlgorithmCatalog.All.Select(item => item.Type));
        InspectionTreeNodes = [];

        LoadImageCommand = new RelayCommand(BrowseAndLoadImage);
        LoadPttCommand = new RelayCommand(BrowseAndLoadPtt);
        SaveModelCommand = new RelayCommand(SaveModel);
        LoadModelCommand = new RelayCommand(BrowseAndLoadModel);
        ImportPartCommand = new RelayCommand(BrowseAndImportPart);
        AddAlgorithmCommand = new RelayCommand(AddAlgorithm);
        RunInspectionCommand = new AsyncRelayCommand(RunInspectionAsync, () => CanRunInspection);
        RunFlowCommand = new AsyncRelayCommand(RunFlowAsync, () => !IsInspectionRunning);
        ZoomOneCommand = DisabledCommand();
        ZoomFitCommand = DisabledCommand();
    }

    // Runs the new MPTI_SetInspParam -> MPTI_InspProc -> MPTI_GetInspectionResult flow.
    // Currently scoped to one Align window with one Align algorithm; uses the last-loaded
    // PTT path (or prompts) so the result is visible in InspectionResultText without
    // touching the existing single-shot RunInspectionCommand pipeline.
    private async Task RunFlowAsync()
    {
        var pttPath = _lastFlowPttPath;
        if (string.IsNullOrWhiteSpace(pttPath) || !System.IO.File.Exists(pttPath))
        {
            pttPath = _fileDialogService.BrowsePtt(_dialogOwner.GetDialogOwner());
            if (string.IsNullOrWhiteSpace(pttPath)) return;
            _lastFlowPttPath = pttPath;
        }

        BeginInspectionRun();
        StatusMessage = "MPTI flow running...";

        try
        {
            // Pull resolution from Part Import metadata if the model has it. The service
            // falls back to reading the .pot file when these are 0, and finally to 1.0
            // mm/pixel inside the native bridge.
            var resX = _model.Part?.PixelResolutionX ?? 0.0;
            var resY = _model.Part?.PixelResolutionY ?? 0.0;
            var result = await _inspectionFlowService.RunAlignAsync(new AlignFlowRequest(
                PttPath: pttPath!,
                WindowWidth: 0,
                WindowHeight: 0,
                PixelResolutionX: resX,
                PixelResolutionY: resY));
            ApplyFlowResult(result);
        }
        catch (Exception ex)
        {
            DiagnosticsLog.Write($"MPTI flow failed: {ex}");
            InspectionResultText = $"FLOW FAILED: {ex.GetType().Name}: {ex.Message}";
            StatusMessage = "MPTI flow failed (see InspectionResult).";
        }
        finally
        {
            IsInspectionRunning = false;
        }
    }

    private void ApplyFlowResult(AlignFlowResult result)
    {
        if (!result.Available)
        {
            StatusMessage = "MPTI bridge unavailable.";
            InspectionResultText = result.StatusMessage;
            return;
        }

        var centers = string.Join(";",
            Enumerable.Range(0, 4)
                .Select(i => $"({result.CentersX[i]},{result.CentersY[i]})"));
        InspectionResultText =
            $"PART={result.PartWidth}x{result.PartHeight}  " +
            $"isInsp={result.IsInsp} isOk={result.IsOk} defect={result.DefectCode} " +
            $"okCount={result.OkCount}\n" +
            $"offset=({result.OffsetX:F2},{result.OffsetY:F2})  theta={result.Theta:F3}°  " +
            $"okShift=({result.OkShiftX},{result.OkShiftY}) okAngle={result.OkAngle}\n" +
            $"centers=[{centers}]\n" +
            $"elapsed={result.ElapsedMs:F1} ms\n" +
            $"---\n{result.StatusMessage}";
        StatusMessage = result.Success
            ? $"MPTI flow OK ({result.ElapsedMs:F1} ms)"
            : $"MPTI flow ran (isInsp={result.IsInsp})";
        TimingText = $"{result.ElapsedMs:F2} ms";
    }

    private async Task RunInspectionAsync()
    {
        ModelSyncFromUiRequested?.Invoke();
        BeginInspectionRun();

        try
        {
            var result = await _inspectionWorkflowService.RunPartAsync(
                _model,
                _imageRuntimeStateService.CreatePartRuntimeImage(_model.Threshold2D),
                ActiveAlgorithm?.Id);
            if (result.RefreshSelectedId != null)
            {
                TreeRefreshRequested?.Invoke(result.RefreshSelectedId);
            }

            ApplyInspectionRun(result);
        }
        catch (Exception ex)
        {
            DiagnosticsLog.Write($"Part inspection failed: {ex}");
            ApplyInspectionFailure(ex);
        }
        finally
        {
            IsInspectionRunning = false;
        }
    }

    public event Action<string>? ImageLoadRequested;
    public event Action<string>? PttLoadRequested;
    public event Action<string>? PartImportRequested;
    public event Action<InspectionModel, string>? ModelLoaded;
    public event Action? ModelSyncFromUiRequested;
    public event Action? ThresholdScheduleRequested;
    public event Action? AlignSearchTabActivationRequested;
    public event Action<bool>? AlignRoiDrawButtonStateRequested;
    public event Action<int>? AlignSearchNumSyncRequested;
    public event Action? AlignActiveRoiUiRefreshRequested;
    public event Action? OverlayRefreshRequested;

    public void SelectNextAlignRoi()
    {
        var result = _roi.SelectNextWindow(_model);
        AlignSearchNumSyncRequested?.Invoke(_model.AlignSearchNum);
        AlignActiveRoiUiRefreshRequested?.Invoke();
        TreeRefreshRequested?.Invoke(result.SelectedId);
        OverlayRefreshRequested?.Invoke();
    }

    public void DeleteActiveAlignRoi()
    {
        var result = _roi.DeleteActiveWindow(_model);
        AlignActiveRoiUiRefreshRequested?.Invoke();
        TreeRefreshRequested?.Invoke(result.SelectedId);
        OverlayRefreshRequested?.Invoke();
        ThresholdScheduleRequested?.Invoke();
    }

    public double CurrentImageZoom => Net48Compat.Clamp(_model.ImageZoom, 1.0, Math.Max(1.0, _model.WheelZoomMax));

    public bool IsAlignSelected => string.Equals(_model.Algorithm, "AlgoAlign", StringComparison.OrdinalIgnoreCase);

    public bool IsAlignSearchActive => IsAlignSelected && _alignSearchTabActive;

    public void SetAlignSearchTabActive(bool active)
    {
        _alignSearchTabActive = active;
    }

    public void EnableWindowRoiDrawing()
    {
        EnableRoiDrawing(RoiDrawTarget.Window);
    }

    public void EnableAlgorithmRoiDrawing()
    {
        if (ActiveAlgorithm == null)
        {
            StatusMessage = "Select or add an Algorithm before drawing Algorithm ROI.";
            return;
        }

        EnableRoiDrawing(RoiDrawTarget.Algorithm);
    }

    public void DisableRoiDrawing()
    {
        _roi.Disable();
        UpdateRoiDrawButtonState();
    }

    private void EnableRoiDrawing(RoiDrawTarget target)
    {
        if (target == RoiDrawTarget.Window && !IsAlignSearchActive)
        {
            AlignSearchTabActivationRequested?.Invoke();
        }

        _roi.Enable(target);
        UpdateRoiDrawButtonState();
        StatusMessage = target == RoiDrawTarget.Window
            ? "Window ROI draw mode: drag on CAM-03 Binary or CAM-01 2D. Press S to move to the next Window ROI."
            : $"Algorithm ROI draw mode: drag on CAM-03 Binary or CAM-01 2D for {ActiveAlgorithm?.Type}.";
    }

    public void UpdateRoiDrawButtonState()
    {
        AlignRoiDrawButtonStateRequested?.Invoke(_roi.IsEnabled && _roi.Target == RoiDrawTarget.Window);
    }

    private RoiRect? ActiveInspectionRoi => _roi.GetActiveInspectionRoi(_model, SelectedAlgorithm);

    public async Task RunThresholdAsync(bool refreshTreeOnAlgorithmUpdate = false)
    {
        if (!_imageRuntimeStateService.HasSourceImage)
        {
            return;
        }

        ModelSyncFromUiRequested?.Invoke();
        StatusMessage = "Threshold running...";

        var result = await _thresholdPreviewWorkflowService.RunAsync(
            _model,
            ActiveInspectionRoi,
            ActiveWindow,
            ActiveAlgorithm,
            FormatRoi);

        if (!result.RanPreview || result.Canceled)
        {
            return;
        }

        ApplyThresholdPreview(
            result.BinaryImage,
            result.UsedNative,
            result.TimingText,
            result.StatusMessage,
            result.ResultText);
        if (refreshTreeOnAlgorithmUpdate && !string.IsNullOrWhiteSpace(result.UpdatedAlgorithmId))
        {
            TreeRefreshRequested?.Invoke(result.UpdatedAlgorithmId);
        }
        OverlayRefreshRequested?.Invoke();
    }

    public void SetAlgorithmPanelFactory(AlgorithmPanelFactory factory)
    {
        _algorithmPanelFactory = factory;
    }

    public void UpdateAlgorithmPanels()
    {
        if (_algorithmPanelFactory == null)
        {
            return;
        }

        var panel = _algorithmPanelFactory.Resolve(SelectedAlgorithm);
        if (!ReferenceEquals(_activeAlgorithmPanel, panel))
        {
            _activeAlgorithmPanel?.Unbind();
            _activeAlgorithmPanel = panel;
            ActiveAlgorithmPanelContent = panel.View;
        }

        var window = ActiveWindow;
        var algorithm = ActiveAlgorithm;
        if (window != null && algorithm != null)
        {
            panel.Bind(new AlgorithmPanelContext
            {
                Model = _model,
                Window = window,
                Algorithm = algorithm,
                Request = request => HandleAlgorithmPanelRequest(algorithm, request)
            });
        }
        else
        {
            panel.Unbind();
        }

        if (!string.Equals(panel.AlgorithmType, "AlgoAlign", StringComparison.OrdinalIgnoreCase))
        {
            DisableRoiDrawing();
        }
    }

    private void HandleAlgorithmPanelRequest(InspectionAlgorithmData algorithm, AlgorithmPanelRequest request)
    {
        switch (request.Kind)
        {
            case AlgorithmPanelRequestKind.PreviewUpdate:
                ThresholdScheduleRequested?.Invoke();
                break;
            case AlgorithmPanelRequestKind.TreeRefresh:
                TreeRefreshRequested?.Invoke(algorithm.Id);
                break;
            case AlgorithmPanelRequestKind.WindowRoiDrawing:
                EnableWindowRoiDrawing();
                break;
            case AlgorithmPanelRequestKind.AlgorithmRoiDrawing:
                EnableAlgorithmRoiDrawing();
                break;
            case AlgorithmPanelRequestKind.SetParameter:
                if (request.ParameterName != null && request.ParameterValue != null)
                {
                    algorithm.ApplyCatalogDefaults();
                    algorithm.Parameters[request.ParameterName] = request.ParameterValue;
                    ThresholdScheduleRequested?.Invoke();
                }
                break;
        }
    }

    private void BrowseAndLoadImage()
    {
        var path = _fileDialogService.BrowseImage(_dialogOwner.GetDialogOwner());
        if (!string.IsNullOrWhiteSpace(path))
        {
            ImageLoadRequested?.Invoke(path!);
        }
    }

    private void BrowseAndLoadPtt()
    {
        var path = _fileDialogService.BrowsePtt(_dialogOwner.GetDialogOwner());
        if (!string.IsNullOrWhiteSpace(path))
        {
            PttLoadRequested?.Invoke(path!);
        }
    }

    private void SaveModel()
    {
        ModelSyncFromUiRequested?.Invoke();
        var path = _modelWorkflowService.Save(_model, _applicationPathService.GetModelDirectory());
        StatusMessage = $"Model saved: {path}";
    }

    private void BrowseAndLoadModel()
    {
        var path = _fileDialogService.BrowseModel(_dialogOwner.GetDialogOwner(), _applicationPathService.GetModelDirectory());
        if (string.IsNullOrWhiteSpace(path))
        {
            return;
        }

        var result = _modelWorkflowService.Load(path!);
        if (!result.Success || result.Model == null)
        {
            StatusMessage = result.StatusMessage;
            return;
        }

        ModelLoaded?.Invoke(result.Model, result.StatusMessage);
    }

    private void BrowseAndImportPart()
    {
        var path = _fileDialogService.BrowsePart(_dialogOwner.GetDialogOwner(), _applicationPathService.GetModelDirectory());
        if (!string.IsNullOrWhiteSpace(path))
        {
            PartImportRequested?.Invoke(path!);
        }
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
    public ICommand RunFlowCommand { get; private set; }
    public ICommand ZoomOneCommand { get; private set; }
    public ICommand ZoomFitCommand { get; private set; }

    public object? ActiveAlgorithmPanelContent
    {
        get => _activeAlgorithmPanelContent;
        set => SetProperty(ref _activeAlgorithmPanelContent, value);
    }

    public event Action<string?>? TreeRefreshRequested;
    public event Action? SelectionChanged;

    public void ConfigureCommands(
        Action zoomOne,
        Action zoomFit)
    {
        ZoomOneCommand = new RelayCommand(zoomOne);
        ZoomFitCommand = new RelayCommand(zoomFit);

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
            TimingText = result.TimingText!;
        }

        StatusMessage = result.StatusMessage;
        InspectionResultText = result.ResultText;
    }

    public void ApplyInspectionFailure(Exception exception)
    {
        StatusMessage = $"Part inspection failed: {exception.Message}";
        InspectionResultText = exception.ToString();
    }

    public void RefreshInspectionTree(string? selectedId = null)
    {
        RefreshInspectionTree(selectedId, _imageRuntimeStateService.SourceWidth, _imageRuntimeStateService.SourceHeight);
    }

    public void RefreshInspectionTree(string? selectedId, int sourceWidth, int sourceHeight)
    {
        _refreshingTree = true;
        try
        {
            RebuildInspectionTree(selectedId, sourceWidth, sourceHeight);
        }
        finally
        {
            _refreshingTree = false;
        }
    }

    private void RebuildInspectionTree(string? selectedId, int sourceWidth, int sourceHeight)
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

                foreach (var summary in FormatBridgeSummaries(algorithm))
                {
                    algorithmNode.Children.Add(new InspectionTreeNodeViewModel
                    {
                        Header = summary,
                        Kind = InspectionTreeNodeKind.InspectionResult,
                        Payload = algorithm,
                        Foreground = new SolidColorBrush(Color.FromRgb(180, 240, 200)),
                        IsEnabled = false
                    });
                }
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
        if (_refreshingTree)
        {
            return false;
        }

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
        UpdateAlgorithmPanels();
        RefreshModelBindings();
    }

    private static IEnumerable<string> FormatBridgeSummaries(InspectionAlgorithmData algorithm)
    {
        var alignBridge = Lookup(algorithm.Parameters, "Runtime.AlignBridge");
        if (!string.IsNullOrEmpty(alignBridge))
        {
            if (string.Equals(alignBridge, "native", StringComparison.OrdinalIgnoreCase))
            {
                var okCount = Lookup(algorithm.Parameters, "Runtime.AlignOkCount", "0");
                var alignRes = Lookup(algorithm.Parameters, "Runtime.AlignResult", "-");
                var okShiftX = Lookup(algorithm.Parameters, "Runtime.AlignOkShiftX", "?");
                var okShiftY = Lookup(algorithm.Parameters, "Runtime.AlignOkShiftY", "?");
                var okAngle = Lookup(algorithm.Parameters, "Runtime.AlignOkAngle", "?");
                yield return $"Align bridge: native | OK {okCount} | ShiftX={okShiftX} ShiftY={okShiftY} Angle={okAngle}";
                yield return $"Align result: {alignRes}";
            }
            else
            {
                yield return $"Align bridge: fallback ({Lookup(algorithm.Parameters, "Runtime.AlignBridgeMessage", "unavailable")})";
            }
        }

        var shapeBridge = Lookup(algorithm.Parameters, "Runtime.ShapeXBridge");
        if (!string.IsNullOrEmpty(shapeBridge))
        {
            if (string.Equals(shapeBridge, "native", StringComparison.OrdinalIgnoreCase))
            {
                var ok = Lookup(algorithm.Parameters, "Runtime.ShapeXIsOK", "?");
                var areaRatio = Lookup(algorithm.Parameters, "Runtime.ShapeXAreaRatio", "?");
                var shift = Lookup(algorithm.Parameters, "Runtime.ShapeXShift", "?");
                yield return $"ShapeX bridge: native | OK={ok} | Area ratio={areaRatio} | Shift {shift}";
            }
            else
            {
                yield return $"ShapeX bridge: fallback ({Lookup(algorithm.Parameters, "Runtime.ShapeXBridgeMessage", "unavailable")})";
            }
        }

        var genericBridge = Lookup(algorithm.Parameters, "Runtime.GenericBridge");
        if (!string.IsNullOrEmpty(genericBridge))
        {
            if (string.Equals(genericBridge, "native", StringComparison.OrdinalIgnoreCase))
            {
                var kind = Lookup(algorithm.Parameters, "Runtime.GenericAlgoKind", "?");
                var ok = Lookup(algorithm.Parameters, "Runtime.GenericIsOK", "?");
                var areaRatio = Lookup(algorithm.Parameters, "Runtime.GenericAreaRatio", "?");
                var shift = Lookup(algorithm.Parameters, "Runtime.GenericShift", "?");
                yield return $"{kind} bridge: native | OK={ok} | Area ratio={areaRatio} | Shift {shift}";
            }
            else
            {
                yield return $"Generic bridge: fallback ({Lookup(algorithm.Parameters, "Runtime.GenericBridgeMessage", "unavailable")})";
            }
        }

        var padBridge = Lookup(algorithm.Parameters, "Runtime.PadBWBridge");
        if (!string.IsNullOrEmpty(padBridge))
        {
            if (string.Equals(padBridge, "native", StringComparison.OrdinalIgnoreCase))
            {
                var ok = Lookup(algorithm.Parameters, "Runtime.PadBWIsOK", "?");
                var rate = Lookup(algorithm.Parameters, "Runtime.PadBWAreaRate", "?");
                var shift = Lookup(algorithm.Parameters, "Runtime.PadBWShift", "?");
                yield return $"PadBW bridge: native | OK={ok} | Area rate={rate}% | Shift {shift}";
            }
            else
            {
                yield return $"PadBW bridge: fallback ({Lookup(algorithm.Parameters, "Runtime.PadBWBridgeMessage", "unavailable")})";
            }
        }
    }

    private static string Lookup(Dictionary<string, string> parameters, string key, string fallback = "")
    {
        return parameters.TryGetValue(key, out var value) ? value : fallback;
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
