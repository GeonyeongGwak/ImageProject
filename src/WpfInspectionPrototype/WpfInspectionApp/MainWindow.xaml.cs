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

    private void ViewerOption_Changed(object sender, RoutedEventArgs e)
    {
        if (!_uiReady || _applyingModel)
        {
            return;
        }

        UpdateModelFromUi();
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

    private string SelectedAlgorithm()
    {
        return ViewModel.SelectedAlgorithm;
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

}





