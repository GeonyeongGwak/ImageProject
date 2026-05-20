using System.Collections.ObjectModel;
using System.IO;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using WpfInspectionApp.Commands;
using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Models;
using WpfInspectionApp.Services;
using WpfInspectionApp.Services.FlowAlgorithms;

namespace WpfInspectionApp.ViewModels;

// CAM-03 (오른쪽 카메라) 가 어떤 화면을 보여줄지 결정하는 모드.
// Origin = PTT + Light 합성 그레이 원본, Binary = threshold preview 결과.
// 라벨은 "전환 후 모드" 를 표시 (현재 Origin 이면 버튼 라벨 "Binary").
public enum Cam03PreviewMode
{
    Origin,
    Binary
}

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
    private string _selectedThemeKey = "Dark";
    private double _imageZoomPercent = 100;
    private ImageSource? _sourceImage;
    private ImageSource? _binaryImage;
    // CAM-01 전용 컬러(BGRA32) 이미지. PTT 로드 시 1 회만 갱신되며 Light 슬라이더에는 영향 없음.
    private ImageSource? _cam01ColorImage;
    private object? _activeAlgorithmPanelContent;
    private string _selectedWindowTypeFilter = AllWindowTypesFilter;
    // CAM-03 가 Light 합성 원본을 보여줄지(Origin), 이진화 결과를 보여줄지(Binary).
    // 기본값은 기존 동작 유지(Binary).
    private Cam03PreviewMode _cam03Mode = Cam03PreviewMode.Binary;
    private const string AllWindowTypesFilter = "All";

    private readonly IDialogOwner _dialogOwner;
    private readonly IFileDialogService _fileDialogService;
    private readonly IModelWorkflowService _modelWorkflowService;
    private readonly IApplicationPathService _applicationPathService;
    private readonly IPartImportWorkflowService _partImportWorkflowService;
    private readonly IImageLoadWorkflowService _imageLoadWorkflowService;
    private readonly IAlignPartTeachingService _alignPartTeachingService;
    private readonly IAlignConditionService _alignConditionService;
    private readonly RoiCanvasViewModel _roi;
    private readonly IRoiUiStateService _roiUiStateService;
    private readonly IImageRuntimeStateService _imageRuntimeStateService;
    private readonly IInspectionWorkflowService _inspectionWorkflowService;
    private readonly IInspectionFlowService _inspectionFlowService;
    private readonly IAlignFlowRequestFactory _alignFlowRequestFactory;
    private readonly IThresholdPreviewWorkflowService _thresholdPreviewWorkflowService;
    private readonly IAlgorithmLightService _algorithmLightService;
    private readonly IPttLightPreviewService _pttLightPreviewService;
    private readonly IPttViewerWorkflowService _pttViewerWorkflowService;
    private bool _isImporting;
    private string _importProgressText = "";
    private string? _lastFlowPttPath;
    private int _lastPttWidth;
    private int _lastPttHeight;
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
        IPartImportWorkflowService partImportWorkflowService,
        IImageLoadWorkflowService imageLoadWorkflowService,
        IAlignPartTeachingService alignPartTeachingService,
        IAlignConditionService alignConditionService,
        RoiCanvasViewModel roi,
        IRoiUiStateService roiUiStateService,
        IImageRuntimeStateService imageRuntimeStateService,
        IInspectionWorkflowService inspectionWorkflowService,
        IInspectionFlowService inspectionFlowService,
        IAlignFlowRequestFactory alignFlowRequestFactory,
        IThresholdPreviewWorkflowService thresholdPreviewWorkflowService,
        IAlgorithmLightService algorithmLightService,
        IPttLightPreviewService pttLightPreviewService,
        IPttViewerWorkflowService pttViewerWorkflowService,
        FlowAlgorithmRegistry flowAlgorithmRegistry)
    {
        _model = model;
        _model.EnsureStructure();
        _selectedAlgorithm = _model.Algorithm;
        _dialogOwner = dialogOwner;
        _fileDialogService = fileDialogService;
        _modelWorkflowService = modelWorkflowService;
        _applicationPathService = applicationPathService;
        _partImportWorkflowService = partImportWorkflowService;
        _imageLoadWorkflowService = imageLoadWorkflowService;
        _alignPartTeachingService = alignPartTeachingService;
        _alignConditionService = alignConditionService;
        _roi = roi;
        _roiUiStateService = roiUiStateService;
        _imageRuntimeStateService = imageRuntimeStateService;
        _inspectionWorkflowService = inspectionWorkflowService;
        _inspectionFlowService = inspectionFlowService;
        _alignFlowRequestFactory = alignFlowRequestFactory;
        _thresholdPreviewWorkflowService = thresholdPreviewWorkflowService;
        _algorithmLightService = algorithmLightService;
        _pttLightPreviewService = pttLightPreviewService;
        _pttViewerWorkflowService = pttViewerWorkflowService;
        LightControl = new LightControlViewModel(_algorithmLightService, HandleLightPanelChanged);
        AlgorithmTypes = new ObservableCollection<string>(AlgorithmCatalog.All.Select(item => item.Type));
        InspectionTreeNodes = [];

        // One FlowAlgorithmRunner per registered IFlowAlgorithm. Each runner owns its
        // own Run command + result display state. XAML ItemsControl binds to this
        // collection — adding a new algorithm to AppServices.RegisterFlowAlgorithms
        // automatically grows the UI without ViewModel/XAML changes.
        FlowAlgorithms = new ObservableCollection<FlowAlgorithmRunner>(
            flowAlgorithmRegistry.All.Select(algo => new FlowAlgorithmRunner(
                algo,
                resolvePttPath: () => ResolveLastFlowPttPath(),
                resolveResolution: () => ResolveModelResolution())));

        ThemeOptions = new ObservableCollection<ThemeOptionViewModel>
        {
            new("Dark", "Dark", Color.FromRgb(39, 166, 255)),
            new("Light", "Light", Color.FromRgb(48, 128, 208)),
            new("Pink", "Pink", Color.FromRgb(255, 112, 189))
        };
        SetSelectedTheme("Dark", raiseEvent: false);

        LoadImageCommand = new RelayCommand(BrowseAndLoadImage);
        LoadPttCommand = new RelayCommand(BrowseAndLoadPtt);
        SaveModelCommand = new RelayCommand(SaveModel);
        LoadModelCommand = new RelayCommand(BrowseAndLoadModel);
        ImportPartCommand = new AsyncRelayCommand(BrowseAndImportPartAsync, () => !IsImporting);
        AddAlgorithmCommand = new RelayCommand(AddAlgorithm);
        DrawAlgorithmRoiCommand = new RelayCommand(EnableAlgorithmRoiDrawing);
        RunInspectionCommand = new AsyncRelayCommand(RunInspectionAsync, () => CanRunInspection);
        RunFlowCommand = new AsyncRelayCommand(RunFlowAsync, () => !IsInspectionRunning);
        SelectThemeCommand = new RelayCommand(parameter => SetSelectedTheme(parameter?.ToString() ?? "Dark"));
        ExpandAllTreeNodesCommand = new RelayCommand(() => SetAllTreeNodesExpanded(true));
        CollapseAllTreeNodesCommand = new RelayCommand(() => SetAllTreeNodesExpanded(false));
        MoveSelectedWindowDownCommand = new RelayCommand(() => MoveSelectedWindow(+1));
        MoveSelectedWindowUpCommand = new RelayCommand(() => MoveSelectedWindow(-1));
        MoveSelectedWindowToBottomCommand = new RelayCommand(() => MoveSelectedWindow(int.MaxValue));
        MoveSelectedWindowToTopCommand = new RelayCommand(() => MoveSelectedWindow(int.MinValue));
        WindowTypeFilters = new ObservableCollection<string> { AllWindowTypesFilter };
        Cam03ToggleCommand = new RelayCommand(ToggleCam03PreviewMode);
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
            ModelSyncFromUiRequested?.Invoke();
            var request = _alignFlowRequestFactory.Create(
                _model,
                pttPath!,
                _imageRuntimeStateService.SourceWidth,
                _imageRuntimeStateService.SourceHeight);
            var result = await _inspectionFlowService.RunAlignAsync(request);
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

    public event Action<string, bool>? PttLoadRequested;
    public event Action<string?, bool>? ModelViewRefreshRequested;
    public event Action? ModelSyncFromUiRequested;
    public event Action<string>? PartTeachingStatusRequested;
    public event Action? ThresholdScheduleRequested;
    public event Action? AlignSearchTabActivationRequested;
    public event Action<bool>? AlignRoiDrawButtonStateRequested;
    public event Action<int>? AlignSearchNumSyncRequested;
    public event Action? AlignActiveRoiUiRefreshRequested;
    public event Action? OverlayRefreshRequested;
    public event Action<string>? ThemeChanged;

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

    public bool CanApplyWheelZoom => WheelZoomEnabled && _imageRuntimeStateService.HasSourceImage;

    public double CreateWheelZoom(int wheelDelta)
    {
        var direction = wheelDelta > 0 ? 1 : -1;
        var step = Net48Compat.Clamp(_model.WheelZoomStep, 0.01, 1.0);
        return CurrentImageZoom + direction * step;
    }

    public double ClampImageZoom(double zoom)
    {
        return Net48Compat.Clamp(zoom, 1.0, ImageZoomMaximum);
    }

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

    public RoiRect? GetActiveRoi()
    {
        return _roi.GetActiveRoi(_model);
    }

    public RoiOverlayState CreateRoiOverlayState(int sourceWidth, int sourceHeight)
    {
        return new RoiOverlayState(
            _model,
            ActiveAlgorithm?.Id,
            _roi.PreviewRoi,
            _roi.Target == RoiDrawTarget.Algorithm,
            sourceWidth,
            sourceHeight,
            CurrentImageZoom);
    }

    public string CreateRoiText(int sourceWidth, int sourceHeight, Func<RoiRect?, string> formatRoi)
    {
        return _roiUiStateService.CreateRoiText(
            _model,
            SelectedAlgorithm,
            _roi.PreviewRoi,
            sourceWidth,
            sourceHeight,
            formatRoi);
    }

    public RoiUiSyncState CreateRoiUiSyncState(int sourceWidth, int sourceHeight, Func<RoiRect?, string> formatRoi)
    {
        return _roiUiStateService.CreateSyncState(
            _model,
            SelectedAlgorithm,
            _roi.PreviewRoi,
            sourceWidth,
            sourceHeight,
            formatRoi);
    }

    public bool UpsertActiveWindowRoi(RoiRect roi)
    {
        return ApplyRoiModelResult(_roi.UpsertActiveWindow(_model, roi));
    }

    public bool CommitDrawingRoi(
        Point surfacePoint,
        int imageWidth,
        int imageHeight,
        double zoom,
        Func<RoiRect?, string> formatRoi)
    {
        return ApplyRoiModelResult(_roi.CommitToModel(
            _model,
            surfacePoint,
            imageWidth,
            imageHeight,
            zoom,
            SelectedAlgorithm,
            formatRoi));
    }

    public bool ResizeActiveRoiFromSearchInputs(int sourceWidth, int sourceHeight)
    {
        return ApplyRoiModelResult(_roi.ResizeActiveRoiFromSearchInputs(_model, sourceWidth, sourceHeight));
    }

    public void NormalizeAlignSearchSelection()
    {
        _model.EnsureStructure();
        _model.AlignSearchNum = Net48Compat.Clamp(_model.AlignSearchNum, 1, 4);
        _model.AlignActiveRoiIndex = Net48Compat.Clamp(_model.AlignActiveRoiIndex, 0, _model.AlignSearchNum - 1);
        RefreshModelBindings();
    }

    public void NormalizeModelForView()
    {
        _model.EnsureStructure();
        _model.AlignSearchNum = Net48Compat.Clamp(_model.AlignSearchNum, 1, 4);
        _model.AlignActiveRoiIndex = Net48Compat.Clamp(_model.AlignActiveRoiIndex, 0, _model.AlignSearchNum - 1);
        SelectedAlgorithm = _model.Algorithm;
        RefreshModelBindings();
    }

    public AlignPanelModelState CreateAlignPanelModelState()
    {
        return AlignPanelModelState.FromModel(_model, SelectedAlgorithm);
    }

    public void ApplyAlignPanelState(AlignPanelModelState state, int sourceWidth, int sourceHeight)
    {
        _model.EnsureStructure();
        SelectedAlgorithm = state.SelectedAlgorithm;
        _model.Part.Name = _model.ModelName;
        _model.Threshold2D = state.Threshold2D;
        _model.Threshold2DMax = Net48Compat.Clamp(state.Threshold2DMax, 0, 255);
        _model.Threshold3D = state.Threshold3D;
        _model.Threshold3DMax = Net48Compat.Clamp(state.Threshold3DMax, 0, 255);
        _model.EdgeGain = state.EdgeGain;
        _model.Use2D = state.Use2D;
        _model.Use3D = state.Use3D;
        _model.UseEdge = state.UseEdge;
        _model.AlignRange2DType = Net48Compat.Clamp(state.Range2DType, 0, 3);
        _model.AlignRange3DType = Net48Compat.Clamp(state.Range3DType, 0, 3);
        _model.AlignInvertCheck = state.InvertCheck;
        _model.AlignHeightAverage = ReadDouble(state.HeightAverage, _model.AlignHeightAverage);
        _model.AlignSearchNum = Net48Compat.Clamp(ReadInt(state.SearchNum, _model.AlignSearchNum, 1, 4), 1, 4);
        _model.AlignSearchMargin = ReadInt(state.SearchMargin, _model.AlignSearchMargin, 0, 100000);
        _model.AlignSearchSizeX = ReadInt(state.SearchSizeX, _model.AlignSearchSizeX, 1, Math.Max(1, sourceWidth));
        _model.AlignSearchSizeY = ReadInt(state.SearchSizeY, _model.AlignSearchSizeY, 1, Math.Max(1, sourceHeight));
        _model.AlignSameSize = state.SameSize;
        _model.AlignShiftEnabled = state.ShiftEnabled;
        _model.AlignShiftX = ReadDouble(state.ShiftX, _model.AlignShiftX);
        _model.AlignShiftY = ReadDouble(state.ShiftY, _model.AlignShiftY);
        _model.AlignAngleEnabled = state.AngleEnabled;
        _model.AlignAngle = ReadDouble(state.Angle, _model.AlignAngle);
        _model.AlignFillHole = state.FillHole;
        _model.AlignFilter = ReadInt(state.Filter, _model.AlignFilter, 0, 100000);
        _model.AlignInspectionAreaCount = state.InspectionAreaCount;
        _model.IpcUse = state.IpcUse;
        _model.IpcClass = state.IpcClass;
        _model.IpcPercent = ReadDouble(state.IpcPercent, _model.IpcPercent);
        _model.PartTeachingUseCommonLibrary = state.PartTeachingUseCommonLibrary;
        _model.PartTeachingUseLibraryPart = state.PartTeachingUseLibraryPart;
        _model.PartTeachingUseAutoTeaching = state.PartTeachingUseAutoTeaching;
        _model.PartTeachingUseCadMatching = state.PartTeachingUseCadMatching;
        _model.PartTeachingLibraryMatchMode = state.PartTeachingLibraryMatchMode;

        RefreshModelBindings();
    }

    public int CalculateMaskDensity()
    {
        return _alignConditionService.CalculateMaskDensity(_model);
    }

    public void TeachActiveRoiSize(Func<RoiRect?, string> formatRoi)
    {
        StatusMessage = $"Teach active ROI size: {formatRoi(GetActiveRoi())}";
    }

    public void CloseAlignPartTeaching()
    {
        ModelSyncFromUiRequested?.Invoke();
        _model.PartTeachingStopRequested = true;
        PartTeachingStatusRequested?.Invoke("Part Teaching closed. Stop requested.");
        StatusMessage = "Align Part Teaching closed.";
    }

    public async Task RunAlignPartTeachingAsync(bool useGerber, Func<RoiRect?, string> formatRoi)
    {
        try
        {
            ModelSyncFromUiRequested?.Invoke();
            var teaching = _alignPartTeachingService.Apply(_model, useGerber, formatRoi);
            PartTeachingStatusRequested?.Invoke(teaching.Status);

            if (!teaching.Success)
            {
                StatusMessage = "Align Part Teaching failed: Window ROI is required.";
                return;
            }

            ModelViewRefreshRequested?.Invoke(teaching.SelectedWindowId, false);
            StatusMessage = $"Align Part Teaching completed: {teaching.TaughtCount} Window(s).";
            InspectionResultText = teaching.Summary;

            await RunThresholdAsync(refreshTreeOnAlgorithmUpdate: true);
        }
        catch (Exception ex)
        {
            DiagnosticsLog.Write($"Align Part Teaching failed: {ex}");
            PartTeachingStatusRequested?.Invoke($"Part Teaching failed: {ex.Message}");
            StatusMessage = $"Align Part Teaching failed: {ex.Message}";
        }
    }

    private bool ApplyRoiModelResult(RoiModelOperationResult result)
    {
        if (!result.Changed)
        {
            return false;
        }

        if (!string.IsNullOrWhiteSpace(result.SelectedId))
        {
            RefreshInspectionTree(result.SelectedId);
        }

        if (!string.IsNullOrWhiteSpace(result.StatusMessage))
        {
            StatusMessage = result.StatusMessage!;
        }

        RefreshModelBindings();
        return true;
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
                    AlgorithmParameterStore.Set(algorithm.Parameters, request.ParameterName, request.ParameterValue);
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
            LoadImageFromPath(path!);
        }
    }

    private void BrowseAndLoadPtt()
    {
        var path = _fileDialogService.BrowsePtt(_dialogOwner.GetDialogOwner());
        if (!string.IsNullOrWhiteSpace(path))
        {
            PttLoadRequested?.Invoke(path!, true);
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

        Model = result.Model;
        StatusMessage = result.StatusMessage;
        ModelViewRefreshRequested?.Invoke(null, true);
    }

    private async Task BrowseAndImportPartAsync()
    {
        var path = _fileDialogService.BrowsePart(_dialogOwner.GetDialogOwner(), _applicationPathService.GetModelDirectory());
        if (!string.IsNullOrWhiteSpace(path))
        {
            await ImportPartFromPathAsync(path!);
        }
    }

    // 외부 호출자 (예: 테스트, drag-drop) 호환을 위한 sync 래퍼. 내부적으로 async 본체를 동기
    // 대기. UI 가 freezing 되어도 알림 띄울 필요가 없는 코드 경로 (smoke test 등) 에서 사용.
    public bool ImportPartFromPath(string path)
    {
        return ImportPartFromPathAsync(path).ConfigureAwait(false).GetAwaiter().GetResult();
    }

    public bool LoadImageFromPath(string path)
    {
        var result = _imageLoadWorkflowService.Load(path);
        if (!result.Success || result.SourceImage == null || result.BinaryImage == null)
        {
            StatusMessage = result.StatusMessage;
            return false;
        }

        ApplyImageLoad(result.SourceImage, result.BinaryImage, result.Width, result.Height, result.StatusMessage);
        RequestOverlayAndThresholdRefresh();
        return true;
    }

    // Part Import 본체. UI freezing 을 막기 위해 무거운 단계 (XML 파싱 + 모델 변환,
    // PTT 네이티브 로드, 이미지 디코드) 는 Task.Run 으로 백그라운드에서 수행하고, 단계
    // 사이마다 ImportProgressText 를 갱신해서 사용자가 어떤 작업이 진행 중인지 알 수 있게 한다.
    public async Task<bool> ImportPartFromPathAsync(string path)
    {
        IsImporting = true;
        try
        {
            ImportProgressText = "Reading part file...";
            ModelSyncFromUiRequested?.Invoke();
            // UI 가 오버레이를 paint 할 시간을 주고 무거운 XML 파싱 + 모델 적용은 background.
            await Task.Yield();

            ImportProgressText = "Parsing XML and normalizing algorithms...";
            var result = await Task.Run(() => _partImportWorkflowService.ImportIntoModel(_model, path));
            if (!result.Success)
            {
                StatusMessage = result.StatusMessage;
                return false;
            }

            ImportProgressText = "Applying model to UI...";
            await Task.Yield();
            _roi.ResetDrawing();
            DisableRoiDrawing();
            ModelViewRefreshRequested?.Invoke(result.SelectedWindowId, false);

            if (!string.IsNullOrWhiteSpace(result.Summary))
            {
                InspectionResultText = result.Summary!;
            }

            if (!string.IsNullOrWhiteSpace(result.PttPath))
            {
                ImportProgressText = "Loading PTT (large file, may take a few seconds)...";
                await Task.Yield();
                await LoadPttAsync(result.PttPath!, prepareMpti: false);
            }

            var loadedImportImage = false;
            if (!string.IsNullOrWhiteSpace(result.ImagePath))
            {
                ImportProgressText = "Loading source image...";
                await Task.Yield();
                loadedImportImage = await Task.Run(() => LoadImageFromPath(result.ImagePath!));
            }

            if (!loadedImportImage)
            {
                RequestOverlayAndThresholdRefresh();
            }

            ImportProgressText = "Finalizing...";
            StatusMessage = result.StatusMessage;
            return true;
        }
        catch (Exception ex)
        {
            DiagnosticsLog.Write($"Part import failed: {ex}");
            StatusMessage = $"Part import failed: {ex.Message}";
            return false;
        }
        finally
        {
            IsImporting = false;
            ImportProgressText = "";
        }
    }

    // PTT 네이티브 로드는 무겁다 (50MB 이상). Task.Run 으로 백그라운드에서 수행하고 결과만
    // UI thread 에서 ApplyPttLoad 로 반영한다. PttLoadRequested 이벤트 경로는 다른 sync
    // 호출 (LoadPttCommand 등) 호환을 위해 그대로 유지.
    private async Task LoadPttAsync(string path, bool prepareMpti)
    {
        var result = await Task.Run(() => _pttViewerWorkflowService.Load(path, prepareMpti));
        ApplyPttLoad(result.Success, result.Path, result.StatusMessage, result.Width, result.Height);
    }

    private void RequestOverlayAndThresholdRefresh()
    {
        OverlayRefreshRequested?.Invoke();
        ThresholdScheduleRequested?.Invoke();
    }

    private void HandleLightPanelChanged()
    {
        // SourceImage 는 항상 갱신 (CAM-01 메인 뷰에서도 쓰임).
        ApplyPttLightPreviewForActiveAlgorithm();

        // CAM-03 가 Origin 모드면 binary 는 굳이 재계산하지 않는다 — 사용자가 Binary 로
        // 토글할 때 ToggleCam03PreviewMode 가 lazy 재계산을 트리거.
        OverlayRefreshRequested?.Invoke();
        if (Cam03Mode == Cam03PreviewMode.Binary)
        {
            ThresholdScheduleRequested?.Invoke();
        }
    }

    private bool ApplyPttLightPreviewForActiveAlgorithm()
    {
        if (_lastPttWidth <= 0 || _lastPttHeight <= 0 || ActiveAlgorithm == null)
        {
            return false;
        }

        var state = LightControl.CreatePreviewState(_algorithmLightService.ReadState(ActiveAlgorithm));
        var preview = _pttLightPreviewService.Render(state, _lastPttWidth, _lastPttHeight);
        if (!preview.Success || preview.Frame == null)
        {
            StatusMessage = $"PTT light preview failed: {preview.StatusMessage}";
            return false;
        }

        _imageRuntimeStateService.SetSourceFrame(preview.Frame);
        SourceImage = preview.Frame.SourceBitmap;
        BinaryImage = preview.Frame.BinaryBitmap;
        MarkImageLoaded(preview.Frame.Width, preview.Frame.Height);
        StatusMessage = $"PTT light preview updated: {LightControl.LightTypeLabel}";
        return true;
    }

    private static int ReadInt(string text, int fallback, int min, int max)
    {
        return int.TryParse(text, out var value) ? Net48Compat.Clamp(value, min, max) : fallback;
    }

    private static double ReadDouble(string text, double fallback)
    {
        return double.TryParse(text, out var value) ? value : fallback;
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

    // One runner per registered IFlowAlgorithm. XAML ItemsControl renders them.
    public ObservableCollection<FlowAlgorithmRunner> FlowAlgorithms { get; }

    public ObservableCollection<ThemeOptionViewModel> ThemeOptions { get; }

    public LightControlViewModel LightControl { get; }

    public ObservableCollection<InspectionTreeNodeViewModel> InspectionTreeNodes { get; }

    // PTT path / resolution lookups used by each runner. Centralized here so that all
    // runners share the same "last-loaded PTT" and the same Part Import resolution.
    // Returning null prompts the user via the dialog.
    private string? ResolveLastFlowPttPath()
    {
        if (!string.IsNullOrWhiteSpace(_lastFlowPttPath) && System.IO.File.Exists(_lastFlowPttPath))
            return _lastFlowPttPath;
        var path = _fileDialogService.BrowsePtt(_dialogOwner.GetDialogOwner());
        if (!string.IsNullOrWhiteSpace(path))
            _lastFlowPttPath = path;
        return _lastFlowPttPath;
    }

    private (double X, double Y)? ResolveModelResolution()
    {
        var rx = _model.Part?.PixelResolutionX ?? 0;
        var ry = _model.Part?.PixelResolutionY ?? 0;
        if (rx > 0 && ry > 0) return (rx, ry);
        return null;
    }

    public string StatusMessage
    {
        get => _statusMessage;
        set => SetProperty(ref _statusMessage, value);
    }

    // Part Import 진행 중 여부. 변경되면 오버레이 Visibility 도 갱신된다.
    public bool IsImporting
    {
        get => _isImporting;
        private set
        {
            if (SetProperty(ref _isImporting, value))
            {
                OnPropertyChanged(nameof(ImportOverlayVisibility));
                if (ImportPartCommand is AsyncRelayCommand asyncCommand)
                {
                    asyncCommand.RaiseCanExecuteChanged();
                }
            }
        }
    }

    // 현재 진행 중인 import 단계를 사용자에게 표시하기 위한 문자열.
    public string ImportProgressText
    {
        get => _importProgressText;
        private set => SetProperty(ref _importProgressText, value);
    }

    // 오버레이 UI 가 IsImporting 에 직접 바인딩하지 않고 Visibility 로 가져갈 수 있게 변환.
    public Visibility ImportOverlayVisibility => IsImporting ? Visibility.Visible : Visibility.Collapsed;

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
            var windowIndex = window == null ? -1 : Model.Part.Windows.IndexOf(window);
            return window == null
                ? "Selected Window: none"
                : $"Selected Window: {FormatWindowDisplayName(windowIndex)} | Algorithms {window.Algorithms.Count}";
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
            NotifyActiveAlgorithmStateChanged();
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

    public bool HasActiveAlgorithm => ActiveAlgorithm != null;

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
        set
        {
            if (SetProperty(ref _sourceImage, value) && Cam03Mode == Cam03PreviewMode.Origin)
            {
                // CAM-03 가 Origin 모드일 때만 SourceImage 변경이 화면 갱신으로 이어진다.
                OnPropertyChanged(nameof(Cam03Image));
            }
        }
    }

    public ImageSource? BinaryImage
    {
        get => _binaryImage;
        set
        {
            if (SetProperty(ref _binaryImage, value) && Cam03Mode == Cam03PreviewMode.Binary)
            {
                // CAM-03 가 Binary 모드일 때만 BinaryImage 변경이 화면 갱신으로 이어진다.
                OnPropertyChanged(nameof(Cam03Image));
            }
        }
    }

    // CAM-01 전용 컬러 이미지. PTT 의 TR/TG/TB 채널을 R=G=B=100 baseline 으로 BGRA32 합성한
    // 결과. Light 슬라이더에는 반응하지 않는다 (의도적 — 사용자가 색 자체를 보는 용도).
    public ImageSource? Cam01ColorImage
    {
        get => _cam01ColorImage;
        private set => SetProperty(ref _cam01ColorImage, value);
    }

    // CAM-03 가 어떤 그림을 보여줄지 결정. Origin = 2D Light 합성, Binary = 이진화 결과.
    // 사용자가 헤더의 토글 버튼으로 직접 전환한다.
    public Cam03PreviewMode Cam03Mode
    {
        get => _cam03Mode;
        private set
        {
            if (!SetProperty(ref _cam03Mode, value))
            {
                return;
            }

            OnPropertyChanged(nameof(Cam03Image));
            OnPropertyChanged(nameof(Cam03ToggleLabel));
            OnPropertyChanged(nameof(Cam03Title));
        }
    }

    // CAM-03 가 실제로 보여줄 비트맵. 모드 + SourceImage/BinaryImage 변경 모두에 반응.
    public ImageSource? Cam03Image => Cam03Mode == Cam03PreviewMode.Binary ? _binaryImage : _sourceImage;

    // 토글 버튼 라벨은 "전환 후 모드" 를 표시한다 (현재 Origin 이면 클릭 시 Binary 가 되므로 "Binary").
    public string Cam03ToggleLabel => Cam03Mode == Cam03PreviewMode.Binary ? "Origin" : "Binary";

    // 카메라 헤더 타이틀. 현재 모드 + 라벨.
    public string Cam03Title => Cam03Mode == Cam03PreviewMode.Binary
        ? "CAM-03 | BINARY RESULT"
        : "CAM-03 | ORIGIN";

    private void ToggleCam03PreviewMode()
    {
        var next = Cam03Mode == Cam03PreviewMode.Binary ? Cam03PreviewMode.Origin : Cam03PreviewMode.Binary;
        Cam03Mode = next;

        // Origin 모드 동안에는 Light 변경 시 binary 재계산을 skip 했으므로,
        // 다시 Binary 모드로 들어올 때 한 번 lazy 하게 재계산해서 최신 상태로 보여준다.
        if (next == Cam03PreviewMode.Binary)
        {
            RequestOverlayAndThresholdRefresh();
        }
        else
        {
            // Origin 으로 돌아갈 때는 overlay 만 재그리기 (binary 는 안 건드림).
            OverlayRefreshRequested?.Invoke();
        }
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
    public ICommand DrawAlgorithmRoiCommand { get; private set; }
    public ICommand ExpandAllTreeNodesCommand { get; private set; } = null!;
    public ICommand CollapseAllTreeNodesCommand { get; private set; } = null!;
    public ICommand MoveSelectedWindowUpCommand { get; private set; } = null!;
    public ICommand MoveSelectedWindowDownCommand { get; private set; } = null!;
    public ICommand MoveSelectedWindowToTopCommand { get; private set; } = null!;
    public ICommand MoveSelectedWindowToBottomCommand { get; private set; } = null!;
    public ICommand Cam03ToggleCommand { get; private set; } = null!;

    // 인스펙션 트리 툴바: Window 타입 필터 ComboBox 의 항목들 ("All" + 현재 모델에
    // 존재하는 모든 distinct TypeName). RebuildInspectionTree 가 매번 갱신.
    public ObservableCollection<string> WindowTypeFilters { get; }

    public string SelectedWindowTypeFilter
    {
        get => _selectedWindowTypeFilter;
        set
        {
            var next = string.IsNullOrWhiteSpace(value) ? AllWindowTypesFilter : value;
            if (SetProperty(ref _selectedWindowTypeFilter, next))
            {
                // 필터 변경 시 트리 재구성 (재구성 안에서 필터링 로직 적용).
                TreeRefreshRequested?.Invoke(Model.SelectedWindowId);
            }
        }
    }

    // 트리 헤더 Expand/Collapse: 재귀로 모든 노드의 IsExpanded 일괄 변경.
    // InspectionTreeNodeViewModel.IsExpanded 가 INPC 를 raise 하므로 TreeViewItem
    // 의 IsExpanded TwoWay 바인딩이 자동으로 UI 에 반영됨.
    private void SetAllTreeNodesExpanded(bool expanded)
    {
        foreach (var node in InspectionTreeNodes)
        {
            SetTreeNodeExpandedRecursive(node, expanded);
        }
    }

    private static void SetTreeNodeExpandedRecursive(InspectionTreeNodeViewModel node, bool expanded)
    {
        node.IsExpanded = expanded;
        foreach (var child in node.Children)
        {
            SetTreeNodeExpandedRecursive(child, expanded);
        }
    }

    // 선택된 Window 를 delta 칸만큼 이동. delta=±1 은 인접 위치, int.MaxValue/
    // MinValue 는 맨 끝으로 이동. Model.Part.Windows 순서를 바꾸고 TreeRefresh.
    private void MoveSelectedWindow(int delta)
    {
        Model.EnsureStructure();
        var windows = Model.Part.Windows;
        if (windows.Count <= 1) return;

        var idx = windows.FindIndex(w => w.Id == Model.SelectedWindowId);
        if (idx < 0) return;

        int newIdx;
        if (delta == int.MinValue)      newIdx = 0;
        else if (delta == int.MaxValue) newIdx = windows.Count - 1;
        else                            newIdx = Math.Max(0, Math.Min(windows.Count - 1, idx + delta));
        if (newIdx == idx) return;

        var item = windows[idx];
        windows.RemoveAt(idx);
        windows.Insert(newIdx, item);
        TreeRefreshRequested?.Invoke(item.Id);
    }
    public ICommand RunInspectionCommand { get; private set; }
    public ICommand RunFlowCommand { get; private set; }
    public ICommand SelectThemeCommand { get; private set; }
    public ICommand ZoomOneCommand { get; private set; }
    public ICommand ZoomFitCommand { get; private set; }

    // ComboBox 의 SelectedValue TwoWay 바인딩이 들어올 수 있게 public setter.
    // setter 가 호출되면 SetSelectedTheme 로 위임해서 ThemeOptions.IsSelected /
    // ThemeChanged 이벤트가 같이 처리되도록 한다. 같은 키가 들어오면 무시 (재진입 방지).
    public string SelectedThemeKey
    {
        get => _selectedThemeKey;
        set
        {
            if (string.IsNullOrWhiteSpace(value)) return;
            if (string.Equals(_selectedThemeKey, value, StringComparison.OrdinalIgnoreCase)) return;
            SetSelectedTheme(value);
        }
    }

    private void SetSelectedTheme(string key, bool raiseEvent = true)
    {
        var next = ThemeOptions.FirstOrDefault(item => string.Equals(item.Key, key, StringComparison.OrdinalIgnoreCase))
            ?? ThemeOptions.First(item => item.Key == "Dark");

        // 백킹 필드에 직접 써서 INPC 만 발화 — public setter 를 재호출하지 않아 재진입 없음.
        SetProperty(ref _selectedThemeKey, next.Key, nameof(SelectedThemeKey));
        foreach (var option in ThemeOptions)
        {
            option.IsSelected = ReferenceEquals(option, next);
        }

        if (raiseEvent)
        {
            ThemeChanged?.Invoke(next.Key);
        }
    }

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
        NotifyActiveAlgorithmStateChanged();
    }

    private void NotifyActiveAlgorithmStateChanged()
    {
        OnPropertyChanged(nameof(ActiveAlgorithm));
        OnPropertyChanged(nameof(HasActiveAlgorithm));
        LightControl.Load(ActiveAlgorithm);
        ApplyPttLightPreviewForActiveAlgorithm();
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

    public void ApplyPttLoad(bool success, string path, string statusMessage, int width, int height)
    {
        StatusMessage = statusMessage;
        if (success)
        {
            MarkPttLoaded(path);
            _lastFlowPttPath = path;
            _lastPttWidth = width;
            _lastPttHeight = height;
            // CAM-01 컬러 이미지는 PTT 로드 시점에 1 회만 native 가 TR/TG/TB 채널을 BGRA32 로
            // 합성해서 반환한다. 이후 Light 변경은 이 값에 영향 없음.
            ApplyPttColorPreview();
            if (ApplyPttLightPreviewForActiveAlgorithm())
            {
                RequestOverlayAndThresholdRefresh();
            }
        }
        else
        {
            MarkPttLoadFailed();
            _lastPttWidth = 0;
            _lastPttHeight = 0;
            Cam01ColorImage = null;
        }
    }

    private void ApplyPttColorPreview()
    {
        if (_lastPttWidth <= 0 || _lastPttHeight <= 0)
        {
            Cam01ColorImage = null;
            return;
        }

        var color = _pttLightPreviewService.RenderColor(_lastPttWidth, _lastPttHeight);
        if (!color.Success || color.ColorImage == null)
        {
            DiagnosticsLog.Write($"PTT color preview failed: {color.StatusMessage}");
            Cam01ColorImage = null;
            return;
        }

        Cam01ColorImage = color.ColorImage;
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
        RefreshWindowTypeFilters();

        // 활성 필터. "All" 또는 빈 문자열이면 모든 Window 노출.
        var activeFilter = string.IsNullOrWhiteSpace(_selectedWindowTypeFilter) ? null : _selectedWindowTypeFilter;
        if (string.Equals(activeFilter, AllWindowTypesFilter, StringComparison.OrdinalIgnoreCase))
        {
            activeFilter = null;
        }

        for (var index = 0; index < Model.Part.Windows.Count; index++)
        {
            var window = Model.Part.Windows[index];
            window.TypeName = string.IsNullOrWhiteSpace(window.TypeName) ? "Mount" : window.TypeName;
            window.GroupId = string.IsNullOrWhiteSpace(window.GroupId) ? (index + 1).ToString() : window.GroupId;

            // 필터: 선택된 타입과 일치하지 않는 윈도우는 트리에 노출 안 함 (데이터는
            // 모델에 그대로 — 필터 해제 시 다시 보임).
            if (activeFilter != null && !string.Equals(window.TypeName, activeFilter, StringComparison.OrdinalIgnoreCase))
            {
                continue;
            }

            var windowNode = new InspectionTreeNodeViewModel
            {
                Header = FormatWindowGridName(window, index),
                TypeText = window.TypeName,
                IsInspectionEnabled = window.IsEnabled,
                IsEssential = window.Algorithms.Count > 0 && window.Algorithms.All(algorithm => InspectionTreeNodeViewModel.ReadBool(algorithm, "Common.IsRequired", false)),
                IsGrouped = window.IsGroup,
                GroupId = window.GroupId,
                Kind = InspectionTreeNodeKind.Window,
                Payload = window,
                IsSelected = selectedId == window.Id
            };
            foreach (var catalog in AlgorithmCatalog.All)
            {
                windowNode.AlgorithmMenuItems.Add(new AlgorithmMenuItemViewModel(
                    catalog.Type,
                    catalog.DisplayName,
                    new RelayCommand(() => AddAlgorithmToWindow(window.Id, catalog.Type))));
            }

            InspectionTreeNodes.Add(windowNode);

            for (var algorithmIndex = 0; algorithmIndex < window.Algorithms.Count; algorithmIndex++)
            {
                var algorithm = window.Algorithms[algorithmIndex];
                algorithm.ApplyCatalogDefaults();
                var algorithmNode = new InspectionTreeNodeViewModel
                {
                    Header = $"Algorithm{algorithmIndex + 1}",
                    TypeText = FormatAlgorithmDisplayName(algorithm),
                    IsInspectionEnabled = InspectionTreeNodeViewModel.ReadBool(algorithm, "Common.bAlgoEnable", true),
                    IsEssential = InspectionTreeNodeViewModel.ReadBool(algorithm, "Common.IsRequired", false),
                    IsGrouped = InspectionTreeNodeViewModel.ReadBool(algorithm, "Common.bAlgoGroup", false),
                    Kind = InspectionTreeNodeKind.Algorithm,
                    Payload = algorithm,
                    IsSelected = selectedId == algorithm.Id
                };
                windowNode.Children.Add(algorithmNode);
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

    private static string FormatWindowDisplayName(int zeroBasedIndex)
    {
        return $"Window ROI {Math.Max(0, zeroBasedIndex) + 1}";
    }

    // 트리 툴바의 Window List ComboBox 항목 갱신. 현재 모델의 Window TypeName 들을
    // distinct 로 모은 뒤 항상 "All" 을 맨 앞에 둠. 이전 선택값이 새 목록에 남아
    // 있으면 유지, 아니면 "All" 로 재설정.
    private void RefreshWindowTypeFilters()
    {
        var types = Model.Part.Windows
            .Select(w => string.IsNullOrWhiteSpace(w.TypeName) ? "Mount" : w.TypeName)
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .OrderBy(t => t, StringComparer.OrdinalIgnoreCase)
            .ToList();

        var desired = new List<string> { AllWindowTypesFilter };
        desired.AddRange(types);

        // 변경 없을 때 컬렉션 noop. WPF ComboBox 가 이상 동작 안 하도록 in-place 갱신.
        var changed = WindowTypeFilters.Count != desired.Count
            || !WindowTypeFilters.SequenceEqual(desired, StringComparer.OrdinalIgnoreCase);
        if (changed)
        {
            WindowTypeFilters.Clear();
            foreach (var t in desired) WindowTypeFilters.Add(t);
        }

        // 현재 선택값이 새 리스트에 없으면 "All" 로.
        if (!WindowTypeFilters.Contains(_selectedWindowTypeFilter, StringComparer.OrdinalIgnoreCase))
        {
            _selectedWindowTypeFilter = AllWindowTypesFilter;
            OnPropertyChanged(nameof(SelectedWindowTypeFilter));
        }
    }

    private static string FormatWindowGridName(InspectionWindowData window, int zeroBasedIndex)
    {
        if (!string.IsNullOrWhiteSpace(window.Name))
        {
            var name = window.Name.Trim();
            var delimiterIndex = name.LastIndexOf(" - ", StringComparison.Ordinal);
            if (delimiterIndex >= 0 && delimiterIndex + 3 < name.Length)
            {
                name = name.Substring(delimiterIndex + 3).Trim();
            }

            if (name.StartsWith("Window ROI ", StringComparison.OrdinalIgnoreCase))
            {
                name = "Window" + name.Substring("Window ROI ".Length);
            }

            if (name.StartsWith("Window ", StringComparison.OrdinalIgnoreCase))
            {
                name = "Window" + name.Substring("Window ".Length);
            }

            return name;
        }

        return $"Window{Math.Max(0, zeroBasedIndex) + 1}";
    }

    private static string FormatAlgorithmDisplayName(InspectionAlgorithmData algorithm)
    {
        return !string.IsNullOrWhiteSpace(algorithm.DisplayName)
            ? algorithm.DisplayName
            : algorithm.Type;
    }

    // 이미지 오버레이에서 ROI 사각형을 클릭했을 때 호출. 해당 Window 를 선택 상태로
    // 만들고 트리 highlight + ROI overlay active 상태 + 알고리즘 패널까지 동기화.
    // SelectTreeNode 와 동일한 후처리를 따르되, 인자가 윈도우 노드가 아니라 ID 기반.
    public bool TrySelectWindowById(string windowId)
    {
        if (_refreshingTree || string.IsNullOrWhiteSpace(windowId))
        {
            return false;
        }

        var window = Model.Part.Windows.FirstOrDefault(candidate => candidate.Id == windowId);
        if (window == null)
        {
            return false;
        }

        Model.SelectedWindowId = window.Id;
        var firstAlgorithm = window.Algorithms.FirstOrDefault();
        if (firstAlgorithm != null && AlgorithmTypes.Contains(firstAlgorithm.Type))
        {
            SelectedAlgorithm = firstAlgorithm.Type;
        }

        RefreshModelBindings();
        // 트리 노드의 IsSelected highlight 갱신을 위해 명시적 rebuild 요청.
        // (SelectTreeNode 경로는 WPF TreeView 가 자체적으로 highlight 처리해서 rebuild 불필요했지만
        // 외부 트리거인 ROI 클릭은 TreeView 가 모르기 때문에 수동 갱신.)
        TreeRefreshRequested?.Invoke(window.Id);
        SelectionChanged?.Invoke();
        return true;
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

            // Window 선택 시 그 윈도우의 첫 algorithm 의 Type 으로 SelectedAlgorithm 을
            // 동기화. 그러면 UpdateAlgorithmPanels (OnViewModelSelectionChanged 가
            // 호출) 가 그 algorithm 의 패널을 띄움. 이전 SelectedAlgorithm 이 다른
            // 타입이었으면 ActiveAlgorithm 의 LastOrDefault fallback 으로 표면적으로는
            // 그 윈도우의 마지막 algorithm 이 보이지만 패널 종류는 stale 상태로 남아
            // 있었기에 명시적 sync 가 필요.
            var firstAlgorithm = window.Algorithms.FirstOrDefault();
            if (firstAlgorithm != null && AlgorithmTypes.Contains(firstAlgorithm.Type))
            {
                SelectedAlgorithm = firstAlgorithm.Type;
            }

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
        AddAlgorithmToWindow(Model.SelectedWindowId, SelectedAlgorithm);
    }

    private void AddAlgorithmToWindow(string? windowId, string? algorithmType)
    {
        Model.EnsureStructure();
        var window = string.IsNullOrWhiteSpace(windowId)
            ? null
            : Model.Part.Windows.FirstOrDefault(candidate => candidate.Id == windowId);
        window ??= Model.Part.Windows.FirstOrDefault(candidate => candidate.Id == Model.SelectedWindowId)
            ?? Model.Part.Windows.FirstOrDefault();
        if (window == null)
        {
            StatusMessage = "Draw and select a Window ROI before adding an Algorithm.";
            return;
        }

        var catalog = AlgorithmCatalog.Find(algorithmType);
        SelectedAlgorithm = catalog.Type;
        var algorithm = CreateAlgorithm(catalog);
        algorithm.ApplyCatalogDefaults();
        window.Algorithms.Add(algorithm);
        Model.SelectedWindowId = window.Id;

        StatusMessage = $"{catalog.DisplayName} added to {window.Name}.";
        TreeRefreshRequested?.Invoke(algorithm.Id);
        UpdateAlgorithmPanels();
        RefreshModelBindings();
    }

    private static InspectionAlgorithmData CreateAlgorithm(AlgorithmCatalogItem catalog)
    {
        return new InspectionAlgorithmData
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

    private string ReadActiveAlgorithmParameter(string key, string fallback)
    {
        var algorithm = ActiveAlgorithm;
        if (algorithm == null)
        {
            return fallback;
        }

        algorithm.ApplyCatalogDefaults();
        return AlgorithmParameterStore.GetValue(algorithm.Parameters, key, fallback);
    }

    private int ReadActiveAlgorithmInt(string key, int fallback, int min, int max)
    {
        var text = ReadActiveAlgorithmParameter(key, fallback.ToString());
        return int.TryParse(text, out var value)
            ? Net48Compat.Clamp(value, min, max)
            : Net48Compat.Clamp(fallback, min, max);
    }

    private void SetActiveAlgorithmIntParameter(string key, string? value, int fallback, int min, int max, string propertyName)
    {
        var raw = value?.Trim() ?? "";
        var nextValue = int.TryParse(raw, out var parsed) ? parsed : fallback;
        nextValue = Net48Compat.Clamp(nextValue, min, max);
        var next = nextValue.ToString();
        SetActiveAlgorithmParameter(key, next, propertyName, !string.Equals(raw, next, StringComparison.Ordinal));
    }

    private void SetActiveAlgorithmParameter(string key, string? value, string propertyName, bool forceNotify = false)
    {
        var algorithm = ActiveAlgorithm;
        if (algorithm == null)
        {
            return;
        }

        algorithm.ApplyCatalogDefaults();
        var next = string.IsNullOrWhiteSpace(value) ? "0" : value!;
        var current = AlgorithmParameterStore.GetValue(algorithm.Parameters, key, "");
        if (string.Equals(current, next, StringComparison.Ordinal))
        {
            if (forceNotify)
            {
                OnPropertyChanged(propertyName);
            }

            return;
        }

        AlgorithmParameterStore.Set(algorithm.Parameters, key, next);
        OnPropertyChanged(propertyName);
        ThresholdScheduleRequested?.Invoke();
    }

    private static string Lookup(Dictionary<string, string> parameters, string key, string fallback = "")
    {
        return AlgorithmParameterStore.GetValue(parameters, key, fallback);
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
