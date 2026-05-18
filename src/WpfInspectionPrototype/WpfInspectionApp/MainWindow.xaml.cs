using System.IO;
using System.Windows;
using System.Windows.Automation.Peers;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Threading;
using WpfInspectionApp.AlgorithmPanels;
using WpfInspectionApp.Diagnostics;
using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Interop;
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
    private System.Windows.Forms.Integration.WindowsFormsHost? _pttViewerHost;
    private System.Windows.Forms.Panel? _pttViewerPanel;
    private bool _uiReady;
    private bool _applyingModel;
    private bool _syncingSearchSize;

    public MainWindow()
    {
        InitializeComponent();
        ApplyNativeDebugStartupGuards();
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
            App.Services.InspectionFlow,
            App.Services.ThresholdPreviewWorkflow,
            App.Services.FlowAlgorithms);
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
        _viewModel.SetAlignSearchTabActive(AlignPanel.IsSearchTabActive);
        _uiReady = true;
    }

    protected override AutomationPeer? OnCreateAutomationPeer()
    {
        if (App.StartupStabilityGuardsEnabled)
        {
            FpExceptionGuard.Diag("MainWindow automation peer suppressed for native-debug startup");
            return null;
        }

        return base.OnCreateAutomationPeer();
    }

    protected override void OnSourceInitialized(EventArgs e)
    {
        base.OnSourceInitialized(e);
        if (!App.StartupStabilityGuardsEnabled)
        {
            return;
        }

        if (PresentationSource.FromVisual(this) is HwndSource source)
        {
            source.AddHook(NativeDebugWindowMessageHook);
            FpExceptionGuard.Diag("MainWindow native-debug HWND hook installed");
        }
    }

    private static IntPtr NativeDebugWindowMessageHook(
        IntPtr hwnd,
        int msg,
        IntPtr wParam,
        IntPtr lParam,
        ref bool handled)
    {
        FpExceptionGuard.TryMask();
        if (App.ShouldSuppressNativeDebugWindowMessage(msg))
        {
            handled = true;
            return IntPtr.Zero;
        }

        return IntPtr.Zero;
    }

    private bool _nativeDebugGuardsReleased;

    private void ApplyNativeDebugStartupGuards()
    {
        if (!App.StartupStabilityGuardsEnabled)
        {
            return;
        }

        ShowActivated = false;
        Focusable = false;
        IsEnabled = false;
        IsHitTestVisible = false;
        InputMethod.SetIsInputMethodEnabled(this, false);
        InputMethod.SetPreferredImeState(this, InputMethodState.Off);
        var guardedControls = GuardTextInputSubtree(this);
        FpExceptionGuard.Diag($"MainWindow native-debug startup guards applied textInputs={guardedControls}");

        // Release path. Use BOTH ContentRendered (preferred: fires after first
        // composition = past dangerous WPF native init) AND a dispatcher idle fallback
        // (in case ContentRendered doesn't fire because the window dies during first
        // render). Whichever fires first wins thanks to _nativeDebugGuardsReleased.
        ContentRendered += MainWindow_ReleaseGuards_Handler;
        Dispatcher.BeginInvoke(
            new Action(() => MainWindow_ReleaseGuards_Handler(this, EventArgs.Empty)),
            DispatcherPriority.ApplicationIdle);
        FpExceptionGuard.Diag("MainWindow native-debug guard release pending (ContentRendered + Idle)");
    }

    private void MainWindow_ReleaseGuards_Handler(object? sender, EventArgs e)
    {
        // Log entry FIRST — if we crash later we'll know we at least got here.
        FpExceptionGuard.Diag($"MainWindow guard release: entered (already-released={_nativeDebugGuardsReleased})");
        if (_nativeDebugGuardsReleased) return;
        _nativeDebugGuardsReleased = true;
        try { ContentRendered -= MainWindow_ReleaseGuards_Handler; } catch { /* event removal best-effort */ }

        // Re-mask FPU before any property write — restoring IsEnabled triggers a
        // layout pass which uses Matrix math (the original fp inexact source). VEH
        // already covers it, this is belt-and-suspenders.
        try { FpExceptionGuard.TryMask(); } catch { }

        // Per-step try/catch with logging so we know exactly which property assignment
        // takes the process down (if any). If the log shows "step N done" but no
        // "step N+1 done", step N+1 is the killer.
        SafeStep("IsEnabled=true",          () => IsEnabled = true);
        SafeStep("IsHitTestVisible=true",   () => IsHitTestVisible = true);
        SafeStep("Focusable=true",          () => Focusable = true);
        SafeStep("ImeEnabled=true",         () => InputMethod.SetIsInputMethodEnabled(this, true));
        SafeStep("RestoreTextInputSubtree", () => RestoreTextInputSubtree(this));
        // Activate() was the killer step under VS native debugging — it calls
        // SetForegroundWindow synchronously, which triggers WM_ACTIVATE / WM_GETOBJECT
        // (UIAutomation peer creation) and IMM32 fp ops despite our message filter.
        // Replaced with SetWindowPos(HWND_TOP, SWP_NOACTIVATE) which raises z-order
        // without grabbing focus or activating — no foreground-window side effects.
        SafeStep("RaiseZOrderNoActivate",   () => RaiseZOrderNoActivate());

        FpExceptionGuard.Diag("MainWindow native-debug guards RELEASED");
    }

    private void RaiseZOrderNoActivate()
    {
        var hwnd = new System.Windows.Interop.WindowInteropHelper(this).Handle;
        if (hwnd == IntPtr.Zero) return;
        // SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE = just z-order change, no activation.
        const uint SWP_NOSIZE = 0x0001;
        const uint SWP_NOMOVE = 0x0002;
        const uint SWP_NOACTIVATE = 0x0010;
        var HWND_TOP = IntPtr.Zero;
        SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
    }

    [System.Runtime.InteropServices.DllImport("user32.dll", SetLastError = true)]
    private static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter,
        int X, int Y, int cx, int cy, uint uFlags);

    private static void SafeStep(string tag, Action action)
    {
        try
        {
            action();
            FpExceptionGuard.Diag($"  release step ok: {tag}");
        }
        catch (Exception ex)
        {
            FpExceptionGuard.Diag($"  release step FAILED: {tag} :: {ex.GetType().Name}: {ex.Message}");
        }
    }

    private static void RestoreTextInputSubtree(DependencyObject root)
    {
        var visited = new HashSet<DependencyObject>();
        void Visit(DependencyObject current)
        {
            if (!visited.Add(current)) return;
            if (current is TextBoxBase textBox)
            {
                textBox.IsTabStop = true;
                textBox.Focusable = true;
                InputMethod.SetIsInputMethodEnabled(textBox, true);
            }
            else if (current is ComboBox comboBox)
            {
                comboBox.IsTabStop = true;
                comboBox.Focusable = true;
                InputMethod.SetIsInputMethodEnabled(comboBox, true);
            }
            foreach (var child in LogicalTreeHelper.GetChildren(current).OfType<DependencyObject>())
                Visit(child);
            if (current is Visual or System.Windows.Media.Media3D.Visual3D)
            {
                var visualChildren = VisualTreeHelper.GetChildrenCount(current);
                for (var index = 0; index < visualChildren; index++)
                    Visit(VisualTreeHelper.GetChild(current, index));
            }
        }
        Visit(root);
    }

    private static int GuardTextInputSubtree(DependencyObject root)
    {
        var guardedControls = 0;
        var visited = new HashSet<DependencyObject>();

        void Visit(DependencyObject current)
        {
            if (!visited.Add(current))
            {
                return;
            }

            if (current is TextBoxBase textBox)
            {
                GuardTextInputElement(textBox);
                textBox.IsTabStop = false;
                textBox.Focusable = false;
                guardedControls++;
            }
            else if (current is ComboBox comboBox)
            {
                GuardTextInputElement(comboBox);
                comboBox.IsTabStop = false;
                comboBox.Focusable = false;
                guardedControls++;
            }

            foreach (var child in LogicalTreeHelper.GetChildren(current).OfType<DependencyObject>())
            {
                Visit(child);
            }

            if (current is Visual or System.Windows.Media.Media3D.Visual3D)
            {
                var visualChildren = VisualTreeHelper.GetChildrenCount(current);
                for (var index = 0; index < visualChildren; index++)
                {
                    Visit(VisualTreeHelper.GetChild(current, index));
                }
            }
        }

        Visit(root);
        return guardedControls;
    }

    private static void GuardTextInputElement(DependencyObject element)
    {
        InputMethod.SetIsInputMethodEnabled(element, false);
        InputMethod.SetPreferredImeState(element, InputMethodState.Off);
    }

    private System.Windows.Forms.Panel EnsurePttViewerPanel()
    {
        if (_pttViewerPanel != null && _pttViewerHost != null)
        {
            return _pttViewerPanel;
        }

        _pttViewerPanel = new System.Windows.Forms.Panel
        {
            BackColor = System.Drawing.Color.FromArgb(2, 5, 10)
        };
        _pttViewerPanel.Resize += (_, _) => _pem3DViewerHostService.ResizeExternalViewer(_pttViewerPanel);

        _pttViewerHost = new System.Windows.Forms.Integration.WindowsFormsHost
        {
            Background = System.Windows.Media.Brushes.Transparent,
            Child = _pttViewerPanel,
            Visibility = Visibility.Collapsed
        };

        PttViewerSurface.Children.Insert(0, _pttViewerHost);
        return _pttViewerPanel;
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
        if (App.StartupStabilityGuardsEnabled)
        {
            FpExceptionGuard.Diag("MainWindow.Loaded entered; running startup work immediately for native-debug guard");
            FpExceptionGuard.TryMask();
            FpExceptionGuard.Diag("MainWindow native-debug input remains disabled to avoid UIAutomation/IME reentry");
            RunStartupLoadWork();
            return;
        }

        FpExceptionGuard.Diag("MainWindow.Loaded entered; deferring startup work");
        Dispatcher.BeginInvoke(new Action(RunStartupLoadWork), DispatcherPriority.ApplicationIdle);
    }

    private void RunStartupLoadWork()
    {
        FpExceptionGuard.TryMask();
        FpExceptionGuard.Diag("MainWindow.StartupLoadWork entered");
        try
        {
            var defaultImage = _applicationPathService.FindDefaultImagePath();
            if (defaultImage != null)
            {
                if (App.StartupStabilityGuardsEnabled)
                {
                    FpExceptionGuard.Diag("MainWindow.StartupLoadWork: default image auto-load skipped for native-debug guard");
                    ViewModel.StatusMessage = "Default image auto-load skipped while native debugging is attached.";
                }
                else
                {
                    FpExceptionGuard.Diag("MainWindow.StartupLoadWork: loading default image");
                    LoadImage(defaultImage);
                    FpExceptionGuard.Diag("MainWindow.StartupLoadWork: default image loaded");
                }
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
                FpExceptionGuard.Diag("MainWindow.StartupLoadWork: importing startup path");
                ImportPartFromPath(importPath);
                FpExceptionGuard.Diag("MainWindow.StartupLoadWork: startup path imported");
            }
        }
        catch (Exception ex)
        {
            FpExceptionGuard.Diag($"MainWindow.StartupLoadWork THREW {ex.GetType().FullName}: {ex.Message}");
            throw;
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
        if (e.Key == Key.F12 && Keyboard.Modifiers.HasFlag(ModifierKeys.Control) && Keyboard.Modifiers.HasFlag(ModifierKeys.Shift))
        {
            e.Handled = true;
            PreloadNativeBridgeFromUi(Keyboard.Modifiers.HasFlag(ModifierKeys.Alt));
            return;
        }
        if (e.Key == Key.F11)
        {
            PreloadNativeBridgeFromUi(true);   // �� �� ��° ������
        }
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

    private void PreloadNativeBridgeFromUi(bool breakIntoDebugger)
    {
        var result = MptiNativeBridge.DebugProbe(breakIntoDebugger);
        var status = result.Success
            ? $"Native bridge loaded: {result.Message}"
            : $"Native bridge load failed: {result.Message}";
        DiagnosticsLog.Write(
            $"Manual native preload: break={breakIntoDebugger}, available={result.Available}, success={result.Success}, code={result.Code}, message={result.Message}");
        ViewModel.StatusMessage = status;
    }

    protected override void OnClosed(EventArgs e)
    {
        _pem3DViewerHostService.Dispose();
        base.OnClosed(e);
    }

    private void InspectionTree_SelectedItemChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
    {
        if (e.NewValue is InspectionTreeNodeViewModel node)
        {
            ViewModel.SelectTreeNode(node);
        }
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

}





