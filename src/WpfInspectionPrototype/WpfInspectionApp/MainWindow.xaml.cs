using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Threading;
using WpfInspectionApp.AlgorithmPanels;
using WpfInspectionApp.Models;
using WpfInspectionApp.Services;
using WpfInspectionApp.ViewModels;
using WpfInspectionApp.Views;

namespace WpfInspectionApp;

public partial class MainWindow : Window
{
    private readonly DispatcherTimer _thresholdTimer;
    private readonly AlgorithmPanelFactory _algorithmPanelFactory = new();
    private readonly RoiOverlayCoordinator _roiOverlayCoordinator;
    private readonly IApplicationPathService _applicationPathService;
    private readonly IFileDialogService _fileDialogService;
    private readonly IModelWorkflowService _modelWorkflowService;
    private readonly IPartImportWorkflowService _partImportWorkflowService;
    private readonly IInspectionWorkflowService _inspectionWorkflowService;
    private readonly IThresholdPreviewWorkflowService _thresholdPreviewWorkflowService;
    private readonly IImageLoadWorkflowService _imageLoadWorkflowService;
    private readonly IImageRuntimeStateService _imageRuntimeStateService;
    private readonly IRoiGeometryService _roiGeometryService;
    private readonly RoiCanvasViewModel _roiCanvasViewModel;
    private readonly IRoiUiStateService _roiUiStateService;
    private readonly IPem3DViewerHostService _pem3DViewerHostService;
    private readonly IPttViewerWorkflowService _pttViewerWorkflowService;
    private readonly IAlignPartTeachingService _alignPartTeachingService;
    private readonly IAlignConditionService _alignConditionService;
    private readonly MainViewModel _viewModel;
    private IAlgorithmPanel? _activeAlgorithmPanel;
    private InspectionModel _model = new();
    private bool _uiReady;
    private bool _applyingModel;
    private bool _syncingSearchSize;
    private bool _refreshingInspectionTree;
    private bool _inspectionRunRequested;

    public MainWindow()
    {
        InitializeComponent();
        SubscribeAlignPanelEvents();
        _roiOverlayCoordinator = new RoiOverlayCoordinator(App.Services.RoiGeometry);
        _applicationPathService = App.Services.ApplicationPath;
        _fileDialogService = App.Services.FileDialog;
        _modelWorkflowService = App.Services.ModelWorkflow;
        _partImportWorkflowService = App.Services.PartImportWorkflow;
        _inspectionWorkflowService = App.Services.InspectionWorkflow;
        _thresholdPreviewWorkflowService = App.Services.ThresholdPreviewWorkflow;
        _imageLoadWorkflowService = App.Services.ImageLoadWorkflow;
        _imageRuntimeStateService = App.Services.ImageRuntimeState;
        _roiGeometryService = App.Services.RoiGeometry;
        _roiCanvasViewModel = new RoiCanvasViewModel(App.Services.RoiInteraction, App.Services.RoiModel);
        _roiUiStateService = App.Services.RoiUiState;
        _pem3DViewerHostService = App.Services.Pem3DViewerHost;
        _pttViewerWorkflowService = App.Services.PttViewerWorkflow;
        _alignPartTeachingService = App.Services.AlignPartTeaching;
        _alignConditionService = App.Services.AlignCondition;
        _viewModel = new MainViewModel(_model);
        DataContext = _viewModel;
        _viewModel.ConfigureCommands(
            BrowseAndLoadImage,
            BrowseAndLoadPtt,
            SaveModel,
            BrowseAndLoadModel,
            BrowseAndImportPart,
            RunInspectionAsync,
            ZoomOne,
            ZoomFit);
        _viewModel.TreeRefreshRequested += RefreshInspectionView;
        _viewModel.AlgorithmPanelRefreshRequested += UpdateAlgorithmPanels;
        _viewModel.SelectionChanged += OnViewModelSelectionChanged;
        InitializeAlgorithmPanels();

        _thresholdTimer = new DispatcherTimer
        {
            Interval = TimeSpan.FromMilliseconds(35)
        };
        _thresholdTimer.Tick += async (_, _) =>
        {
            _thresholdTimer.Stop();
            await RunThresholdAsync();
        };

        _model.EnsureStructure();
        ApplyModelAndRefreshView(scheduleThreshold: false);
        PttViewerPanel.Resize += (_, _) => _pem3DViewerHostService.ResizeExternalViewer(PttViewerPanel);
        _uiReady = true;
    }

    private void SubscribeAlignPanelEvents()
    {
        AlignPanel.AlignTabSelectionChanged += AlignTabControl_SelectionChanged;
        AlignPanel.UpdateRequested += AlignPanel_UpdateRequested;
        AlignPanel.ActionRequested += AlignPanel_ActionRequested;
    }

    private MainViewModel ViewModel => _viewModel;

    private bool CanHandleUiEvent => _uiReady && !_applyingModel;

    private void HandleUiChange(Action action)
    {
        if (!CanHandleUiEvent)
        {
            return;
        }

        action();
    }

    private void HandleAlignPanelUpdate(AlignPanelUpdateEffect update)
    {
        HandleUiChange(() => ApplyAlignPanelUpdate(update));
    }

    private void ApplyAlignPanelUpdate(AlignPanelUpdateEffect update)
    {
        if (update.HasFlag(AlignPanelUpdateEffect.PartTeachingUi))
        {
            UpdatePartTeachingUi();
        }

        if (update.HasFlag(AlignPanelUpdateEffect.Model))
        {
            UpdateModelFromUi();
        }

        if (update.HasFlag(AlignPanelUpdateEffect.AlgorithmPanels))
        {
            UpdateAlgorithmPanels();
        }

        if (update.HasFlag(AlignPanelUpdateEffect.RoiDrawButton))
        {
            UpdateRoiDrawButtonState();
        }

        if (update.HasFlag(AlignPanelUpdateEffect.ActiveRoiUi))
        {
            UpdateActiveRoiUi();
        }

        if (update.HasFlag(AlignPanelUpdateEffect.MaskDensity))
        {
            UpdateMaskDensity();
        }

        if (update.HasFlag(AlignPanelUpdateEffect.RoiText))
        {
            UpdateRoiText();
        }

        if (update.HasFlag(AlignPanelUpdateEffect.RoiOverlays))
        {
            DrawRoiOverlays();
        }

        if (update.HasFlag(AlignPanelUpdateEffect.Threshold))
        {
            ScheduleThreshold();
        }
    }

    private void AlignPanel_UpdateRequested(object? sender, AlignPanelUpdateRequestedEventArgs e)
    {
        switch (e.Kind)
        {
            case AlignPanelUpdateKind.SearchNum:
                SearchNumCombo_SelectionChanged();
                break;
            case AlignPanelUpdateKind.SearchSize:
                SearchSizeBox_TextChanged(e.Source);
                break;
            default:
                HandleAlignPanelUpdate(e.Kind.ToEffect());
                break;
        }
    }

    private void AlignPanel_ActionRequested(object? sender, AlignPanelActionRequestedEventArgs e)
    {
        switch (e.Kind)
        {
            case AlignPanelActionKind.ActiveRoi:
                SelectNextAlignRoi();
                break;
            case AlignPanelActionKind.DrawWindowRoi:
                EnableWindowRoiDrawing();
                break;
            case AlignPanelActionKind.DrawAlgorithmRoi:
                EnableAlgorithmRoiDrawing();
                break;
            case AlignPanelActionKind.Teach:
                TeachActiveRoiSize();
                break;
            case AlignPanelActionKind.PartTeachingIc:
                RunAlignPartTeaching(useGerber: false);
                break;
            case AlignPanelActionKind.PartTeachingOk:
                RunAlignPartTeaching(useGerber: true);
                break;
            case AlignPanelActionKind.PartTeachingClose:
                CloseAlignPartTeaching();
                break;
        }
    }

    private void OnViewModelSelectionChanged()
    {
        UpdateActiveRoiUi();
        UpdateAlgorithmPanels();
        RefreshRoiOverlaysAndThreshold();
    }

    private void InitializeAlgorithmPanels()
    {
        AlgorithmPanelStaging.Children.Remove(AlignPanel);
        AlgorithmPanelStaging.Children.Remove(GenericAlgorithmPanel);

        AlignPanel.Visibility = Visibility.Visible;
        GenericAlgorithmPanel.Visibility = Visibility.Visible;

        _algorithmPanelFactory.Register("AlgoAlign", AlignPanel);
        _algorithmPanelFactory.RegisterDiscoveredPanels();
        _algorithmPanelFactory.RegisterFallback(GenericAlgorithmPanel);

        foreach (var catalog in AlgorithmCatalog.All.Where(item => !_algorithmPanelFactory.IsRegistered(item.Type)))
        {
            _algorithmPanelFactory.Register(new DynamicAlgorithmPanel(catalog));
        }

        DiagnosticsLog.Write($"Algorithm panels registered: {string.Join(", ", _algorithmPanelFactory.RegisteredAlgorithmTypes.OrderBy(type => type, StringComparer.OrdinalIgnoreCase))}");
    }

    private void Window_Loaded(object sender, RoutedEventArgs e)
    {
        var defaultImage = _applicationPathService.FindDefaultImagePath();
        if (defaultImage != null)
        {
            LoadImage(defaultImage);
        }
        else
        {
            ViewModel.StatusMessage = "Default Image/2D.jpg was not found.";
        }

        var importPath = ResolveStartupImportPath(Environment.GetCommandLineArgs().Skip(1));
        DiagnosticsLog.Write($"Startup args: {string.Join(" | ", Environment.GetCommandLineArgs().Skip(1))}");
        DiagnosticsLog.Write($"Startup import path: {importPath ?? "<none>"}");
        if (importPath != null)
        {
            ImportPartFromPath(importPath);
        }
    }

    private static string? ResolveStartupImportPath(IEnumerable<string> arguments)
    {
        var normalizedArguments = arguments
            .Select(argument => argument.Trim().Trim('"'))
            .Where(argument => !string.IsNullOrWhiteSpace(argument))
            .ToList();
        var directPath = normalizedArguments.FirstOrDefault(File.Exists);
        if (!string.IsNullOrWhiteSpace(directPath))
        {
            return directPath;
        }

        var joinedPath = string.Join(" ", normalizedArguments).Trim().Trim('"');
        return File.Exists(joinedPath) ? joinedPath : null;
    }

    private void Window_PreviewKeyDown(object sender, KeyEventArgs e)
    {
        if (!IsAlignSearchActive() || Keyboard.Modifiers.HasFlag(ModifierKeys.Control) || Keyboard.Modifiers.HasFlag(ModifierKeys.Alt))
        {
            return;
        }

        if (e.Key == Key.A)
        {
            e.Handled = true;
            EnableWindowRoiDrawing();
            return;
        }

        if (e.Key == Key.S)
        {
            e.Handled = true;
            CommitCurrentDrawingRoi();
            SelectNextAlignRoi();
            EnableWindowRoiDrawing();
            return;
        }

        if (e.Key == Key.Delete)
        {
            e.Handled = true;
            DeleteActiveAlignRoi();
        }
    }

    protected override void OnClosed(EventArgs e)
    {
        _pem3DViewerHostService.Dispose();
        base.OnClosed(e);
    }

    private void BrowseAndLoadImage()
    {
        var path = _fileDialogService.BrowseImage(this);
        if (!string.IsNullOrWhiteSpace(path))
        {
            LoadImage(path);
        }
    }

    private void BrowseAndLoadPtt()
    {
        var path = _fileDialogService.BrowsePtt(this);
        if (!string.IsNullOrWhiteSpace(path))
        {
            LoadPtt(path);
        }
    }

    private void SaveModel()
    {
        UpdateModelFromUi();
        var path = _modelWorkflowService.Save(_model, _applicationPathService.GetModelDirectory());
        ViewModel.StatusMessage = $"Model saved: {path}";
    }

    private void BrowseAndLoadModel()
    {
        var path = _fileDialogService.BrowseModel(this, _applicationPathService.GetModelDirectory());
        if (string.IsNullOrWhiteSpace(path))
        {
            return;
        }

        var result = _modelWorkflowService.Load(path);
        if (!result.Success || result.Model == null)
        {
            ViewModel.StatusMessage = result.StatusMessage;
            return;
        }

        _model = result.Model;
        ViewModel.Model = _model;
        ApplyModelAndRefreshView();
        ViewModel.StatusMessage = result.StatusMessage;
    }

    private void BrowseAndImportPart()
    {
        var path = _fileDialogService.BrowsePart(this, _applicationPathService.GetModelDirectory());
        if (string.IsNullOrWhiteSpace(path))
        {
            return;
        }

        ImportPartFromPath(path);
    }

    private void ImportPartFromPath(string path)
    {
        try
        {
            UpdateModelFromUi();
            var result = _partImportWorkflowService.ImportIntoModel(_model, path);
            if (!result.Success)
            {
                ViewModel.StatusMessage = result.StatusMessage;
                return;
            }

            ApplyImportedPart(result);
            var loadedImportImage = false;
            if (!string.IsNullOrWhiteSpace(result.PttPath))
            {
                LoadPtt(result.PttPath!, prepareMpti: false);
            }

            if (!string.IsNullOrWhiteSpace(result.ImagePath))
            {
                LoadImage(result.ImagePath!);
                loadedImportImage = true;
            }

            if (!loadedImportImage)
            {
                RefreshRoiOverlaysAndThreshold();
            }

            ViewModel.StatusMessage = result.StatusMessage;
        }
        catch (Exception ex)
        {
            DiagnosticsLog.Write($"Part import failed: {ex}");
            ViewModel.StatusMessage = $"Part import failed: {ex.Message}";
        }
    }

    private void ApplyImportedPart(PartImportWorkflowResult result)
    {
        _roiCanvasViewModel.ResetDrawing();
        DisableRoiDrawing();
        ApplyModelAndRefreshView(result.SelectedWindowId, scheduleThreshold: false);
        if (!string.IsNullOrWhiteSpace(result.Summary))
        {
            ViewModel.InspectionResultText = result.Summary;
        }
    }

    private void InspectionTree_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
    {
        if (_refreshingInspectionTree)
        {
            return;
        }

        if (e.NewValue is not InspectionTreeNodeViewModel node)
        {
            return;
        }

        ViewModel.SelectTreeNode(node);
    }

    private void DrawAlgorithmRoiButton_Click(object sender, RoutedEventArgs e)
    {
        EnableAlgorithmRoiDrawing();
    }

    private async Task RunInspectionAsync()
    {
        UpdateModelFromUi();

        _inspectionRunRequested = true;
        ViewModel.BeginInspectionRun();

        try
        {
            var result = await _inspectionWorkflowService.RunPartAsync(
                _model,
                CreatePartRuntimeImage(),
                ActiveAlgorithm?.Id);
            if (result.RefreshSelectedId != null)
            {
                RefreshInspectionTree(result.RefreshSelectedId);
            }

            ViewModel.ApplyInspectionRun(result);
        }
        catch (Exception ex)
        {
            DiagnosticsLog.Write($"Part inspection failed: {ex}");
            ViewModel.ApplyInspectionFailure(ex);
        }
        finally
        {
            _inspectionRunRequested = false;
            ViewModel.IsInspectionRunning = false;
        }
    }

    private PartRuntimeImage? CreatePartRuntimeImage()
    {
        return _imageRuntimeStateService.CreatePartRuntimeImage(_model.Threshold2D);
    }

    private void AlgorithmCombo_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        HandleAlignPanelUpdate(AlignPanelUpdateEffect.Model | AlignPanelUpdateEffect.AlgorithmPanels | AlignPanelUpdateEffect.RoiDrawButton | AlignPanelUpdateEffect.Threshold);
    }

    private void AlignTabControl_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        if (sender != AlignPanel.AlignTabControl)
        {
            return;
        }

        HandleUiChange(() =>
        {
            if (!IsAlignSearchActive())
            {
                DisableRoiDrawing();
            }
        });
    }

    private void SearchNumCombo_SelectionChanged()
    {
        HandleUiChange(() =>
        {
            UpdateModelFromUi();
            _model.AlignSearchNum = Net48Compat.Clamp(_model.AlignSearchNum, 1, 4);
            _model.AlignActiveRoiIndex = Math.Min(_model.AlignActiveRoiIndex, _model.AlignSearchNum - 1);
            UpdateActiveRoiUi();
            DrawRoiOverlays();
        });
    }

    private void SearchSizeBox_TextChanged(object? source)
    {
        if (_syncingSearchSize)
        {
            return;
        }

        HandleUiChange(() =>
        {
            UpdateModelFromUi();

            _syncingSearchSize = true;
            AlignPanel.MirrorSearchSizeInput(source, _model);
            _syncingSearchSize = false;

            ResizeActiveRoiFromSearchInputs();
        });
    }

    private void TeachActiveRoiSize()
    {
        SyncSearchSizeInputsFromActiveRoi();
        ViewModel.StatusMessage = $"Teach active ROI size: {FormatRoi(ActiveRoi)}";
    }

    private void CloseAlignPartTeaching()
    {
        UpdateModelFromUi();
        _model.PartTeachingStopRequested = true;
        AlignPanel.ClosePartTeaching();
        ViewModel.StatusMessage = "Align Part Teaching closed.";
    }

    private void ImageOverlay_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
    {
        if (!_imageRuntimeStateService.HasSourceImage || sender is not Canvas canvas)
        {
            return;
        }

        e.Handled = _roiCanvasViewModel.TryBegin(canvas, e.GetPosition(canvas), _imageRuntimeStateService.SourceWidth, _imageRuntimeStateService.SourceHeight, CurrentImageZoom());
    }

    private void ImageOverlay_MouseMove(object sender, MouseEventArgs e)
    {
        if (!_roiCanvasViewModel.IsDrawing || _roiCanvasViewModel.DrawingSurface == null)
        {
            return;
        }

        _roiCanvasViewModel.Preview(e.GetPosition(_roiCanvasViewModel.DrawingSurface), _imageRuntimeStateService.SourceWidth, _imageRuntimeStateService.SourceHeight, CurrentImageZoom());
        DrawRoiOverlays();
    }

    private void ImageOverlay_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
    {
        if (!_roiCanvasViewModel.IsDrawing || _roiCanvasViewModel.DrawingSurface == null)
        {
            return;
        }

        CommitCurrentDrawingRoi(e.GetPosition(_roiCanvasViewModel.DrawingSurface));
    }

    private void Overlay_SizeChanged(object sender, SizeChangedEventArgs e)
    {
        DrawRoiOverlays();
    }

    private void ImageOverlay_MouseWheel(object sender, MouseWheelEventArgs e)
    {
        if (!_model.WheelZoomEnabled || !_imageRuntimeStateService.HasSourceImage)
        {
            return;
        }

        var direction = e.Delta > 0 ? 1 : -1;
        var step = Net48Compat.Clamp(_model.WheelZoomStep, 0.01, 1.0);
        var next = _model.ImageZoom + direction * step;
        SetImageZoom(next);
        e.Handled = true;
    }

    private void ViewerOption_Changed(object sender, RoutedEventArgs e)
    {
        if (!_uiReady || _applyingModel)
        {
            return;
        }

        UpdateModelFromUi();
    }

    private void ImageZoomSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
    {
        if (!_uiReady || _applyingModel)
        {
            return;
        }

        SetImageZoom(e.NewValue);
    }

    private void ZoomOne()
    {
        SetImageZoom(1.0);
    }

    private void ZoomFit()
    {
        SetImageZoom(1.0);
    }

    private void LoadImage(string path)
    {
        var result = _imageLoadWorkflowService.Load(path);
        if (!result.Success || result.SourceImage == null || result.BinaryImage == null)
        {
            ViewModel.StatusMessage = result.StatusMessage;
            return;
        }

        ViewModel.ApplyImageLoad(result.SourceImage, result.BinaryImage, result.Width, result.Height, result.StatusMessage);
        RefreshRoiOverlaysAndThreshold();
    }

    private void LoadPtt(string path, bool prepareMpti = true)
    {
        var result = _pttViewerWorkflowService.LoadIntoControl(path, PttViewerPanel, prepareMpti);
        ViewModel.ApplyPttLoad(result.Success, result.Path, result.StatusMessage);
    }

    private void ScheduleThreshold()
    {
        if (!_uiReady)
        {
            return;
        }

        UpdateModelFromUi();
        UpdateMaskDensity();

        if (!_imageRuntimeStateService.HasSourceImage)
        {
            return;
        }

        _thresholdTimer.Stop();
        _thresholdTimer.Start();
    }

    private async Task RunThresholdAsync()
    {
        if (!_imageRuntimeStateService.HasSourceImage)
        {
            return;
        }

        UpdateModelFromUi();
        ViewModel.StatusMessage = "Threshold running...";

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

        ViewModel.ApplyThresholdPreview(
            result.BinaryImage,
            result.UsedNative,
            result.TimingText,
            result.StatusMessage,
            result.ResultText);
        if (_inspectionRunRequested && !string.IsNullOrWhiteSpace(result.UpdatedAlgorithmId))
        {
            RefreshInspectionTree(result.UpdatedAlgorithmId);
        }
        _inspectionRunRequested = false;
        DrawRoiOverlays();
    }

    private void UpdateModelFromUi()
    {
        _model.EnsureStructure();
        _model.ModelName = string.IsNullOrWhiteSpace(ViewModel.ModelName) ? "UnnamedModel" : ViewModel.ModelName.Trim();
        _model.Part.Name = _model.ModelName;
        AlignPanel.ApplyToModel(_model, SelectedAlgorithm(), _imageRuntimeStateService.SourceWidth, _imageRuntimeStateService.SourceHeight);
        _model.WheelZoomEnabled = ViewModel.WheelZoomEnabled;
        _model.ImageZoom = ViewModel.ImageZoom;
    }

    private void ApplyModelToUi()
    {
        _applyingModel = true;
        _model.EnsureStructure();
        _model.AlignSearchNum = Net48Compat.Clamp(_model.AlignSearchNum, 1, 4);
        _model.AlignActiveRoiIndex = Net48Compat.Clamp(_model.AlignActiveRoiIndex, 0, _model.AlignSearchNum - 1);

        ViewModel.RefreshModelBindings();
        ViewModel.SelectedAlgorithm = _model.Algorithm;
        AlignPanel.LoadFromModel(_model);
        ViewModel.ImageZoomMaximum = Math.Max(1.0, _model.WheelZoomMax);
        ViewModel.ImageZoom = _model.ImageZoom;

        _applyingModel = false;
        UpdateAlgorithmPanels();
        UpdateMaskDensity();
        UpdateActiveRoiUi();
        UpdateRoiText();
        UpdatePartTeachingUi();
    }

    private void ApplyModelAndRefreshView(string? selectedId = null, bool scheduleThreshold = true)
    {
        ApplyModelToUi();
        if (scheduleThreshold)
        {
            RefreshInspectionViewAndThreshold(selectedId);
            return;
        }

        RefreshInspectionView(selectedId);
    }

    private void RefreshInspectionView(string? selectedId = null)
    {
        RefreshInspectionTree(selectedId);
        DrawRoiOverlays();
    }

    private void RefreshInspectionViewAndThreshold(string? selectedId = null)
    {
        RefreshInspectionView(selectedId);
        ScheduleThreshold();
    }

    private void RefreshRoiOverlaysAndThreshold()
    {
        DrawRoiOverlays();
        ScheduleThreshold();
    }

    private string SelectedAlgorithm()
    {
        return ViewModel.SelectedAlgorithm;
    }

    private bool IsAlignSelected()
    {
        return string.Equals(_model.Algorithm, "AlgoAlign", StringComparison.OrdinalIgnoreCase);
    }

    private bool IsAlignSearchActive()
    {
        return IsAlignSelected() && AlignPanel.IsSearchTabActive;
    }

    private void UpdatePartTeachingUi()
    {
        var wasApplying = _applyingModel;
        _applyingModel = true;
        try
        {
            AlignPanel.UpdatePartTeachingUi();
        }
        finally
        {
            _applyingModel = wasApplying;
        }
    }

    private async void RunAlignPartTeaching(bool useGerber)
    {
        try
        {
            UpdatePartTeachingUi();
            UpdateModelFromUi();
            var teaching = _alignPartTeachingService.Apply(_model, useGerber, FormatRoi);
            if (!teaching.Success)
            {
                AlignPanel.SetPartTeachingStatus(teaching.Status);
                ViewModel.StatusMessage = "Align Part Teaching failed: Window ROI is required.";
                return;
            }

            ApplyModelAndRefreshView(teaching.SelectedWindowId, scheduleThreshold: false);

            AlignPanel.SetPartTeachingStatus(teaching.Status);
            ViewModel.StatusMessage = $"Align Part Teaching completed: {teaching.TaughtCount} Window(s).";
            ViewModel.InspectionResultText = teaching.Summary;

            _inspectionRunRequested = true;
            await RunThresholdAsync();
        }
        catch (Exception ex)
        {
            DiagnosticsLog.Write($"Align Part Teaching failed: {ex}");
            AlignPanel.SetPartTeachingStatus($"Part Teaching failed: {ex.Message}");
            ViewModel.StatusMessage = $"Align Part Teaching failed: {ex.Message}";
        }
    }

    private RoiRect? ActiveRoi
    {
        get
        {
            return _roiCanvasViewModel.GetActiveRoi(_model);
        }
        set
        {
            if (value.HasValue)
            {
                ApplyRoiModelResult(_roiCanvasViewModel.UpsertActiveWindow(_model, value.Value));
            }
        }
    }

    private RoiRect? ActiveInspectionRoi => _roiCanvasViewModel.GetActiveInspectionRoi(_model, SelectedAlgorithm());

    private InspectionWindowData? ActiveWindow => _roiCanvasViewModel.GetActiveWindow(_model);

    private InspectionAlgorithmData? ActiveAlgorithm => _roiCanvasViewModel.GetActiveAlgorithm(_model, SelectedAlgorithm());

    private void ApplyRoiModelResult(RoiModelOperationResult result)
    {
        if (!result.Changed)
        {
            return;
        }

        if (!string.IsNullOrWhiteSpace(result.SelectedId))
        {
            RefreshInspectionTree(result.SelectedId);
        }

        if (!string.IsNullOrWhiteSpace(result.StatusMessage))
        {
            ViewModel.StatusMessage = result.StatusMessage;
        }
    }


    private void RefreshInspectionTree(string? selectedId = null)
    {
        _refreshingInspectionTree = true;
        try
        {
            ViewModel.RefreshInspectionTree(selectedId, _imageRuntimeStateService.SourceWidth, _imageRuntimeStateService.SourceHeight);
        }
        finally
        {
            _refreshingInspectionTree = false;
        }
    }

    private void UpdateAlgorithmPanels()
    {
        var panel = ResolveAlgorithmPanel(SelectedAlgorithm());
        if (!ReferenceEquals(_activeAlgorithmPanel, panel))
        {
            _activeAlgorithmPanel?.Unbind();
            _activeAlgorithmPanel = panel;
            ViewModel.ActiveAlgorithmPanelContent = panel.View;
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

    private IAlgorithmPanel ResolveAlgorithmPanel(string algorithmType)
    {
        return _algorithmPanelFactory.Resolve(algorithmType);
    }

    private void HandleAlgorithmPanelRequest(InspectionAlgorithmData algorithm, AlgorithmPanelRequest request)
    {
        switch (request.Kind)
        {
            case AlgorithmPanelRequestKind.PreviewUpdate:
                ScheduleThreshold();
                break;
            case AlgorithmPanelRequestKind.TreeRefresh:
                RefreshInspectionTree(algorithm.Id);
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
                    SetAlgorithmParameter(algorithm, request.ParameterName, request.ParameterValue);
                }
                break;
        }
    }

    private void SetAlgorithmParameter(InspectionAlgorithmData algorithm, string name, string value)
    {
        algorithm.ApplyCatalogDefaults();
        algorithm.Parameters[name] = value;
        ScheduleThreshold();
    }

    private void SetImageZoom(double zoom)
    {
        var nextZoom = Net48Compat.Clamp(zoom, 1.0, Math.Max(1.0, _model.WheelZoomMax));
        _applyingModel = true;
        ViewModel.ImageZoom = nextZoom;
        _applyingModel = false;
        DrawRoiOverlays();
    }

    private void UpdateMaskDensity()
    {
        var maskDensity = _alignConditionService.CalculateMaskDensity(_model);
        AlignPanel.SetMaskDensity(maskDensity);
    }

    private void EnableWindowRoiDrawing()
    {
        EnableRoiDrawing(RoiDrawTarget.Window);
    }

    private void EnableAlgorithmRoiDrawing()
    {
        if (ActiveAlgorithm == null)
        {
            ViewModel.StatusMessage = "Select or add an Algorithm before drawing Algorithm ROI.";
            return;
        }

        EnableRoiDrawing(RoiDrawTarget.Algorithm);
    }

    private void EnableRoiDrawing(RoiDrawTarget target)
    {
        if (target == RoiDrawTarget.Window && !IsAlignSearchActive())
        {
            AlignPanel.ActivateSearchTab();
        }

        _roiCanvasViewModel.Enable(target);
        UpdateRoiDrawButtonState();
        ViewModel.StatusMessage = target == RoiDrawTarget.Window
            ? "Window ROI draw mode: drag on CAM-03 Binary or CAM-01 2D. Press S to move to the next Window ROI."
            : $"Algorithm ROI draw mode: drag on CAM-03 Binary or CAM-01 2D for {ActiveAlgorithm?.Type}.";
    }

    private void CommitCurrentDrawingRoi()
    {
        if (!_roiCanvasViewModel.IsDrawing || _roiCanvasViewModel.DrawingSurface == null)
        {
            SyncSearchSizeInputsFromActiveRoi();
            DrawRoiOverlays();
            return;
        }

        CommitCurrentDrawingRoi(Mouse.GetPosition(_roiCanvasViewModel.DrawingSurface));
    }

    private void CommitCurrentDrawingRoi(Point surfacePoint)
    {
        if (!_roiCanvasViewModel.IsDrawing)
        {
            return;
        }

        ApplyRoiModelResult(_roiCanvasViewModel.CommitToModel(
            _model,
            surfacePoint,
            _imageRuntimeStateService.SourceWidth,
            _imageRuntimeStateService.SourceHeight,
            CurrentImageZoom(),
            SelectedAlgorithm(),
            FormatRoi));

        SyncSearchSizeInputsFromActiveRoi();
        RefreshRoiOverlaysAndThreshold();
    }

    private void DisableRoiDrawing()
    {
        _roiCanvasViewModel.Disable();
        UpdateRoiDrawButtonState();
    }

    private void UpdateRoiDrawButtonState()
    {
        AlignPanel.SetWindowRoiDrawingState(_roiCanvasViewModel.IsEnabled && _roiCanvasViewModel.Target == RoiDrawTarget.Window);
    }

    private void SelectNextAlignRoi()
    {
        var result = _roiCanvasViewModel.SelectNextWindow(_model);
        SyncSearchNumCombo();
        UpdateActiveRoiUi();
        RefreshInspectionView(result.SelectedId);
    }

    private void SyncSearchNumCombo()
    {
        _applyingModel = true;
        AlignPanel.SetSearchNum(_model.AlignSearchNum);
        _applyingModel = false;
    }

    private void DeleteActiveAlignRoi()
    {
        var result = _roiCanvasViewModel.DeleteActiveWindow(_model);
        UpdateActiveRoiUi();
        RefreshInspectionViewAndThreshold(result.SelectedId);
    }

    private void UpdateActiveRoiUi()
    {
        ViewModel.RefreshModelBindings();
        ApplyRoiUiSyncState();
    }

    private void ResizeActiveRoiFromSearchInputs()
    {
        var result = _roiCanvasViewModel.ResizeActiveRoiFromSearchInputs(_model, _imageRuntimeStateService.SourceWidth, _imageRuntimeStateService.SourceHeight);
        if (!result.Changed)
        {
            DrawRoiOverlays();
            return;
        }

        ApplyRoiModelResult(result);
        RefreshRoiOverlaysAndThreshold();
    }

    private void SyncSearchSizeInputsFromActiveRoi()
    {
        ApplyRoiUiSyncState();
    }

    private void DrawRoiOverlays()
    {
        _roiOverlayCoordinator.RenderAll(CreateRoiOverlayState(), SourceOverlay, BinaryOverlay);
        UpdateRoiText();
    }

    private RoiOverlayState CreateRoiOverlayState()
    {
        return new RoiOverlayState(
            _model,
            ActiveAlgorithm?.Id,
            _roiCanvasViewModel.PreviewRoi,
            _roiCanvasViewModel.Target == RoiDrawTarget.Algorithm,
            _imageRuntimeStateService.SourceWidth,
            _imageRuntimeStateService.SourceHeight,
            CurrentImageZoom());
    }

    private void UpdateRoiText()
    {
        var text = _roiUiStateService.CreateRoiText(
            _model,
            SelectedAlgorithm(),
            _roiCanvasViewModel.PreviewRoi,
            _imageRuntimeStateService.SourceWidth,
            _imageRuntimeStateService.SourceHeight,
            FormatRoi);
        AlignPanel.SetRoiText(text);
    }

    private void ApplyRoiUiSyncState()
    {
        var state = _roiUiStateService.CreateSyncState(
            _model,
            SelectedAlgorithm(),
            _roiCanvasViewModel.PreviewRoi,
            _imageRuntimeStateService.SourceWidth,
            _imageRuntimeStateService.SourceHeight,
            FormatRoi);

        AlignPanel.SetActiveRoiText(state.ActiveRoiText);
        if (state.SearchSizeRoi.HasValue)
        {
            _syncingSearchSize = true;
            AlignPanel.SetSearchSize(state.SearchSizeRoi.Value.Width, state.SearchSizeRoi.Value.Height);
            _syncingSearchSize = false;
        }

        AlignPanel.SetRoiText(state.RoiText);
    }

    private Rect GetImageDisplayRect(FrameworkElement surface)
    {
        return _roiOverlayCoordinator.GetImageDisplayRect(surface, CreateRoiOverlayState());
    }

    private double CurrentImageZoom()
    {
        return Net48Compat.Clamp(_model.ImageZoom, 1.0, Math.Max(1.0, _model.WheelZoomMax));
    }

    private string FormatRoi(RoiRect? roi)
    {
        if (!roi.HasValue)
        {
            return "none";
        }

        var value = roi.Value;
        var centerX = value.X + value.Width / 2;
        var centerY = value.Y + value.Height / 2;
        return $"X {value.X} Y {value.Y} W {value.Width} H {value.Height} | CX {centerX} CY {centerY} | ORG {_imageRuntimeStateService.SourceWidth}x{_imageRuntimeStateService.SourceHeight}";
    }

}





