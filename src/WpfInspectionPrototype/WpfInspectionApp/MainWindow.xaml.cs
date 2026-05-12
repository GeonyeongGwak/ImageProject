using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Threading;
using WpfInspectionApp.AlgorithmPanels;
using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Models;
using WpfInspectionApp.Services;
using WpfInspectionApp.ViewModels;
using WpfInspectionApp.Views;

namespace WpfInspectionApp;

public partial class MainWindow : Window, IDialogOwner
{
    Window IDialogOwner.GetDialogOwner() => this;

    private readonly DispatcherTimer _thresholdTimer;
    private readonly AlgorithmPanelFactory _algorithmPanelFactory = new();
    private readonly RoiOverlayCoordinator _roiOverlayCoordinator;
    private readonly IApplicationPathService _applicationPathService;
    private readonly IPartImportWorkflowService _partImportWorkflowService;
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
    private bool _uiReady;
    private bool _applyingModel;
    private bool _syncingSearchSize;
    private bool _refreshingInspectionTree;

    public MainWindow()
    {
        InitializeComponent();
        SubscribeAlignPanelEvents();
        _roiOverlayCoordinator = new RoiOverlayCoordinator(App.Services.RoiGeometry);
        _applicationPathService = App.Services.ApplicationPath;
        _partImportWorkflowService = App.Services.PartImportWorkflow;
        _imageLoadWorkflowService = App.Services.ImageLoadWorkflow;
        _imageRuntimeStateService = App.Services.ImageRuntimeState;
        _roiGeometryService = App.Services.RoiGeometry;
        _roiCanvasViewModel = new RoiCanvasViewModel(App.Services.RoiInteraction, App.Services.RoiModel);
        _roiUiStateService = App.Services.RoiUiState;
        _pem3DViewerHostService = App.Services.Pem3DViewerHost;
        _pttViewerWorkflowService = App.Services.PttViewerWorkflow;
        _alignPartTeachingService = App.Services.AlignPartTeaching;
        _alignConditionService = App.Services.AlignCondition;
        _viewModel = new MainViewModel(
            new InspectionModel(),
            this,
            App.Services.FileDialog,
            App.Services.ModelWorkflow,
            _applicationPathService,
            _roiCanvasViewModel,
            _imageRuntimeStateService,
            App.Services.InspectionWorkflow,
            App.Services.ThresholdPreviewWorkflow);
        DataContext = _viewModel;
        _viewModel.ConfigureCommands(ZoomOne, ZoomFit);
        _viewModel.TreeRefreshRequested += RefreshInspectionView;
        _viewModel.SelectionChanged += OnViewModelSelectionChanged;
        _viewModel.ImageLoadRequested += LoadImage;
        _viewModel.PttLoadRequested += path => LoadPtt(path);
        _viewModel.PartImportRequested += ImportPartFromPath;
        _viewModel.ModelLoaded += OnModelLoaded;
        _viewModel.ModelSyncFromUiRequested += UpdateModelFromUi;
        _viewModel.ThresholdScheduleRequested += ScheduleThreshold;
        _viewModel.AlignSearchTabActivationRequested += AlignPanel.ActivateSearchTab;
        _viewModel.AlignRoiDrawButtonStateRequested += AlignPanel.SetWindowRoiDrawingState;
        _viewModel.AlignSearchNumSyncRequested += SyncAlignSearchNumFromViewModel;
        _viewModel.AlignActiveRoiUiRefreshRequested += UpdateActiveRoiUi;
        _viewModel.OverlayRefreshRequested += DrawRoiOverlays;
        InitializeAlgorithmPanels();

        _thresholdTimer = new DispatcherTimer
        {
            Interval = TimeSpan.FromMilliseconds(35)
        };
        _thresholdTimer.Tick += async (_, _) =>
        {
            _thresholdTimer.Stop();
            await _viewModel.RunThresholdAsync();
        };

        ApplyModelAndRefreshView(scheduleThreshold: false);
        PttViewerPanel.Resize += (_, _) => _pem3DViewerHostService.ResizeExternalViewer(PttViewerPanel);
        _viewModel.SetAlignSearchTabActive(AlignPanel.IsSearchTabActive);
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


    private void OnViewModelSelectionChanged()
    {
        UpdateActiveRoiUi();
        _viewModel.UpdateAlgorithmPanels();
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
        _viewModel.SetAlgorithmPanelFactory(_algorithmPanelFactory);
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
        if (!_viewModel.IsAlignSearchActive || Keyboard.Modifiers.HasFlag(ModifierKeys.Control) || Keyboard.Modifiers.HasFlag(ModifierKeys.Alt))
        {
            return;
        }

        if (e.Key == Key.A)
        {
            e.Handled = true;
            _viewModel.EnableWindowRoiDrawing();
            return;
        }

        if (e.Key == Key.S)
        {
            e.Handled = true;
            CommitCurrentDrawingRoi();
            _viewModel.SelectNextAlignRoi();
            _viewModel.EnableWindowRoiDrawing();
            return;
        }

        if (e.Key == Key.Delete)
        {
            e.Handled = true;
            _viewModel.DeleteActiveAlignRoi();
        }
    }

    protected override void OnClosed(EventArgs e)
    {
        _pem3DViewerHostService.Dispose();
        base.OnClosed(e);
    }

    private void OnModelLoaded(InspectionModel loadedModel, string statusMessage)
    {
        ViewModel.Model = loadedModel;
        ApplyModelAndRefreshView();
        ViewModel.StatusMessage = statusMessage;
    }

    private void ImportPartFromPath(string path)
    {
        try
        {
            UpdateModelFromUi();
            var result = _partImportWorkflowService.ImportIntoModel(ViewModel.Model, path);
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
        _viewModel.DisableRoiDrawing();
        ApplyModelAndRefreshView(result.SelectedWindowId, scheduleThreshold: false);
        if (!string.IsNullOrWhiteSpace(result.Summary))
        {
            ViewModel.InspectionResultText = result.Summary!;
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
        _viewModel.EnableAlgorithmRoiDrawing();
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
            _viewModel.SetAlignSearchTabActive(AlignPanel.IsSearchTabActive);
            if (!_viewModel.IsAlignSearchActive)
            {
                _viewModel.DisableRoiDrawing();
            }
        });
    }

    private void SearchNumCombo_SelectionChanged()
    {
        HandleUiChange(() =>
        {
            UpdateModelFromUi();
            ViewModel.Model.AlignSearchNum = Net48Compat.Clamp(ViewModel.Model.AlignSearchNum, 1, 4);
            ViewModel.Model.AlignActiveRoiIndex = Math.Min(ViewModel.Model.AlignActiveRoiIndex, ViewModel.Model.AlignSearchNum - 1);
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
            AlignPanel.MirrorSearchSizeInput(source, ViewModel.Model);
            _syncingSearchSize = false;

            ResizeActiveRoiFromSearchInputs();
        });
    }

    private void ImageOverlay_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
    {
        if (!_imageRuntimeStateService.HasSourceImage || sender is not Canvas canvas)
        {
            return;
        }

        e.Handled = _roiCanvasViewModel.TryBegin(canvas, e.GetPosition(canvas), _imageRuntimeStateService.SourceWidth, _imageRuntimeStateService.SourceHeight, _viewModel.CurrentImageZoom);
    }

    private void ImageOverlay_MouseMove(object sender, MouseEventArgs e)
    {
        if (!_roiCanvasViewModel.IsDrawing || _roiCanvasViewModel.DrawingSurface == null)
        {
            return;
        }

        _roiCanvasViewModel.Preview(e.GetPosition(_roiCanvasViewModel.DrawingSurface), _imageRuntimeStateService.SourceWidth, _imageRuntimeStateService.SourceHeight, _viewModel.CurrentImageZoom);
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
        if (!ViewModel.Model.WheelZoomEnabled || !_imageRuntimeStateService.HasSourceImage)
        {
            return;
        }

        var direction = e.Delta > 0 ? 1 : -1;
        var step = Net48Compat.Clamp(ViewModel.Model.WheelZoomStep, 0.01, 1.0);
        var next = ViewModel.Model.ImageZoom + direction * step;
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

    private void UpdateModelFromUi()
    {
        ViewModel.Model.EnsureStructure();
        ViewModel.Model.Part.Name = ViewModel.Model.ModelName;
        AlignPanel.ApplyToModel(ViewModel.Model, SelectedAlgorithm(), _imageRuntimeStateService.SourceWidth, _imageRuntimeStateService.SourceHeight);
    }

    private void ApplyModelToUi()
    {
        _applyingModel = true;
        ViewModel.Model.EnsureStructure();
        ViewModel.Model.AlignSearchNum = Net48Compat.Clamp(ViewModel.Model.AlignSearchNum, 1, 4);
        ViewModel.Model.AlignActiveRoiIndex = Net48Compat.Clamp(ViewModel.Model.AlignActiveRoiIndex, 0, ViewModel.Model.AlignSearchNum - 1);

        ViewModel.RefreshModelBindings();
        ViewModel.SelectedAlgorithm = ViewModel.Model.Algorithm;
        AlignPanel.LoadFromModel(ViewModel.Model);
        ViewModel.ImageZoomMaximum = Math.Max(1.0, ViewModel.Model.WheelZoomMax);
        ViewModel.ImageZoom = ViewModel.Model.ImageZoom;

        _applyingModel = false;
        _viewModel.UpdateAlgorithmPanels();
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

    private RoiRect? ActiveRoi
    {
        get
        {
            return _roiCanvasViewModel.GetActiveRoi(ViewModel.Model);
        }
        set
        {
            if (value.HasValue)
            {
                ApplyRoiModelResult(_roiCanvasViewModel.UpsertActiveWindow(ViewModel.Model, value.Value));
            }
        }
    }

    private RoiRect? ActiveInspectionRoi => _roiCanvasViewModel.GetActiveInspectionRoi(ViewModel.Model, SelectedAlgorithm());

    private InspectionWindowData? ActiveWindow => _roiCanvasViewModel.GetActiveWindow(ViewModel.Model);

    private InspectionAlgorithmData? ActiveAlgorithm => _roiCanvasViewModel.GetActiveAlgorithm(ViewModel.Model, SelectedAlgorithm());

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
            ViewModel.StatusMessage = result.StatusMessage!;
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

    private void SetImageZoom(double zoom)
    {
        var nextZoom = Net48Compat.Clamp(zoom, 1.0, Math.Max(1.0, ViewModel.Model.WheelZoomMax));
        _applyingModel = true;
        ViewModel.ImageZoom = nextZoom;
        _applyingModel = false;
        DrawRoiOverlays();
    }

    private void UpdateMaskDensity()
    {
        var maskDensity = _alignConditionService.CalculateMaskDensity(ViewModel.Model);
        AlignPanel.SetMaskDensity(maskDensity);
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
            ViewModel.Model,
            surfacePoint,
            _imageRuntimeStateService.SourceWidth,
            _imageRuntimeStateService.SourceHeight,
            _viewModel.CurrentImageZoom,
            SelectedAlgorithm(),
            FormatRoi));

        SyncSearchSizeInputsFromActiveRoi();
        RefreshRoiOverlaysAndThreshold();
    }


    private void SyncAlignSearchNumFromViewModel(int searchNum)
    {
        _applyingModel = true;
        AlignPanel.SetSearchNum(searchNum);
        _applyingModel = false;
    }

    private void UpdateActiveRoiUi()
    {
        ViewModel.RefreshModelBindings();
        ApplyRoiUiSyncState();
    }

    private void ResizeActiveRoiFromSearchInputs()
    {
        var result = _roiCanvasViewModel.ResizeActiveRoiFromSearchInputs(ViewModel.Model, _imageRuntimeStateService.SourceWidth, _imageRuntimeStateService.SourceHeight);
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
            ViewModel.Model,
            ActiveAlgorithm?.Id,
            _roiCanvasViewModel.PreviewRoi,
            _roiCanvasViewModel.Target == RoiDrawTarget.Algorithm,
            _imageRuntimeStateService.SourceWidth,
            _imageRuntimeStateService.SourceHeight,
            _viewModel.CurrentImageZoom);
    }

    private void UpdateRoiText()
    {
        var text = _roiUiStateService.CreateRoiText(
            ViewModel.Model,
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
            ViewModel.Model,
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





