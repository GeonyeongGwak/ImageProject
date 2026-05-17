using System.Configuration;
using System.Data;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Automation.Peers;
using WpfInspectionApp.Diagnostics;
using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Interop;

namespace WpfInspectionApp;

/// <summary>
/// Interaction logic for App.xaml
/// </summary>
public partial class App : Application
{
    // Lazy-init so AppServices construction (which touches 50+ service classes + the
    // FlowAlgorithm plugin registry) only runs when something actually needs a service.
    // Eager-init via a property auto-initializer (`= new()`) runs the chain during
    // App's class-init which can fault under VS native debugging on some setups.
    private static readonly Lazy<AppServices> s_servicesLazy = new(() =>
    {
        FpExceptionGuard.Diag("App.Services factory invoked");
        try
        {
            var svc = new AppServices();
            FpExceptionGuard.Diag("App.Services factory returned ok");
            return svc;
        }
        catch (Exception ex)
        {
            FpExceptionGuard.Diag($"App.Services factory THREW {ex.GetType().Name}: {ex.Message}");
            throw;
        }
    });
    public static AppServices Services => s_servicesLazy.Value;
    private static bool s_renderGuardApplied;
    private static bool s_imeGuardApplied;
    private static bool s_visualDiagnosticsGuardApplied;
    private static bool s_messagePumpGuardApplied;
    private static bool s_startupStabilityGuardsEnabled;
    private Window? _startupGuardWindow;
    internal static bool StartupStabilityGuardsEnabled => s_startupStabilityGuardsEnabled;

    [DllImport("kernel32.dll", ExactSpelling = true)]
    private static extern bool IsDebuggerPresent();

    [DllImport("imm32.dll", ExactSpelling = true)]
    private static extern bool ImmDisableIME(uint idThread);

    static App()
    {
        // Native debugging can re-enter WPF after changing the FPU exception mask.
        // Re-mask immediately before System.Windows.Application's base constructor runs.
        FpExceptionGuard.TryMask();
        FpExceptionGuard.Diag("App.cctor entered");
    }

    public App()
    {
        FpExceptionGuard.TryMask();
        FpExceptionGuard.Diag("App ctor entered");
    }

    protected override void OnStartup(StartupEventArgs e)
    {
        FpExceptionGuard.Diag("App.OnStartup entered");
        // FpExceptionGuard's ModuleInitializer already ran before Main. Re-mask defensively
        // in case PresentationFramework/PresentationCore static ctors (or VS WpfTap injection)
        // ran code that unmasked fp exceptions between module init and OnStartup.
        FpExceptionGuard.TryMask();

        var cmdArgs = Environment.GetCommandLineArgs();
        ApplyDebuggerRenderGuard(cmdArgs);

        DispatcherUnhandledException += (_, args) =>
        {
            DiagnosticsLog.Write($"DispatcherUnhandledException: {args.Exception}");
            if (IsRecoverableWpfRenderException(args.Exception))
            {
                DiagnosticsLog.Write("DispatcherUnhandledException handled: recoverable WPF render overflow.");
                args.Handled = true;
            }
        };

        AppDomain.CurrentDomain.UnhandledException += (_, args) =>
        {
            DiagnosticsLog.Write($"UnhandledException: {args.ExceptionObject}");
        };

        DiagnosticsLog.Write("Application startup.");

        DiagnosticsLog.Write($"Command line args count: {cmdArgs.Length}");
        PreloadNativeBridgeForDebugger(cmdArgs);
        FpExceptionGuard.TryMask();

        var runSmokeTest = cmdArgs.Any(arg => string.Equals(arg, "--smoke-test", StringComparison.OrdinalIgnoreCase));
        if (runSmokeTest)
        {
            ShutdownMode = ShutdownMode.OnExplicitShutdown;
        }

        if (StartupStabilityGuardsEnabled)
        {
            ShutdownMode = ShutdownMode.OnExplicitShutdown;
            base.OnStartup(e);
            FpExceptionGuard.Diag("App.OnStartup: base.OnStartup done before delayed MainWindow show");
            ShowNativeDebugParkingWindow();
            _ = ShowMainWindowAfterNativeDebugDelayAsync(runSmokeTest);
            return;
        }

        CreateShowMainWindowAndMaybeRunSmokeTest(runSmokeTest);
        base.OnStartup(e);
        FpExceptionGuard.Diag("App.OnStartup: base.OnStartup done");
    }

    private async Task ShowMainWindowAfterNativeDebugDelayAsync(bool runSmokeTest)
    {
        FpExceptionGuard.Diag("App.OnStartup: delaying MainWindow creation for native-debug guard");
        await Task.Delay(1500).ConfigureAwait(false);
        await Dispatcher.InvokeAsync(() =>
        {
            FpExceptionGuard.TryMask();
            FpExceptionGuard.Diag("App.OnStartup: delayed MainWindow creation dispatching");
            CreateShowMainWindowAndMaybeRunSmokeTest(runSmokeTest);
        }, System.Windows.Threading.DispatcherPriority.Background);
    }

    private void CreateShowMainWindowAndMaybeRunSmokeTest(bool runSmokeTest)
    {
        FpExceptionGuard.Diag("App.MainWindow: creating MainWindow");
        MainWindow = new MainWindow();
        FpExceptionGuard.Diag("App.MainWindow: MainWindow created, showing");
        MainWindow.Show();
        FpExceptionGuard.Diag("App.MainWindow: MainWindow.Show() returned");
        CloseNativeDebugParkingWindow();
        ShutdownMode = runSmokeTest ? ShutdownMode.OnExplicitShutdown : ShutdownMode.OnMainWindowClose;

        if (runSmokeTest)
        {
            Dispatcher.BeginInvoke(new Action(() =>
            {
                var exitCode = SmokeTestRunner.Run(Services);
                DiagnosticsLog.Write($"Smoke test exit code: {exitCode}");
                if (MainWindow != null)
                {
                    try { MainWindow.Close(); } catch { /* ignore */ }
                }

                Shutdown(exitCode);
            }), System.Windows.Threading.DispatcherPriority.ApplicationIdle);
        }
    }

    private void ShowNativeDebugParkingWindow()
    {
        try
        {
            _startupGuardWindow = new NativeDebugParkingWindow();
            _startupGuardWindow.Show();
            FpExceptionGuard.Diag("App.NativeDebugParkingWindow shown");
        }
        catch (Exception ex)
        {
            FpExceptionGuard.Diag($"App.NativeDebugParkingWindow show FAILED {ex.GetType().FullName}: {ex.Message}");
        }
    }

    private void CloseNativeDebugParkingWindow()
    {
        var parkingWindow = _startupGuardWindow;
        if (parkingWindow == null)
        {
            return;
        }

        _startupGuardWindow = null;
        try
        {
            parkingWindow.Close();
            FpExceptionGuard.Diag("App.NativeDebugParkingWindow closed");
        }
        catch (Exception ex)
        {
            FpExceptionGuard.Diag($"App.NativeDebugParkingWindow close FAILED {ex.GetType().FullName}: {ex.Message}");
        }
    }

    private sealed class NativeDebugParkingWindow : Window
    {
        public NativeDebugParkingWindow()
        {
            Width = 1;
            Height = 1;
            Left = -32000;
            Top = -32000;
            ShowActivated = false;
            ShowInTaskbar = false;
            WindowStyle = WindowStyle.None;
            ResizeMode = ResizeMode.NoResize;
            Focusable = false;
        }

        protected override AutomationPeer? OnCreateAutomationPeer()
        {
            return null;
        }
    }

    internal static void ApplyDebuggerRenderGuard(string[] cmdArgs)
    {
        var managedDebuggerAttached = Debugger.IsAttached;
        var nativeDebuggerAttached = IsNativeDebuggerAttached();
        var requestedByArg = cmdArgs.Any(arg => string.Equals(arg, "--software-rendering", StringComparison.OrdinalIgnoreCase));
        s_startupStabilityGuardsEnabled = managedDebuggerAttached || nativeDebuggerAttached || requestedByArg;
        if (!s_startupStabilityGuardsEnabled)
        {
            return;
        }

        ApplyProcessImeGuard();
        DisableVisualDiagnosticsForDebugger();
        InstallMessagePumpGuardForDebugger();

        if (s_renderGuardApplied)
        {
            return;
        }

        s_renderGuardApplied = true;
        System.Windows.Media.RenderOptions.ProcessRenderMode = System.Windows.Interop.RenderMode.SoftwareOnly;
        DiagnosticsLog.Write("WPF software rendering enabled for debugger stability.");
        FpExceptionGuard.Diag(
            $"WPF software rendering enabled managedDebugger={managedDebuggerAttached} nativeDebugger={nativeDebuggerAttached} arg={requestedByArg}");
    }

    private static void ApplyProcessImeGuard()
    {
        if (s_imeGuardApplied)
        {
            return;
        }

        s_imeGuardApplied = true;
        try
        {
            var disabled = ImmDisableIME(uint.MaxValue);
            FpExceptionGuard.Diag($"Process IME disabled for native-debug startup: {disabled}");
        }
        catch (Exception ex)
        {
            FpExceptionGuard.Diag($"Process IME disable FAILED {ex.GetType().Name}: {ex.Message}");
        }
    }

    private static void InstallMessagePumpGuardForDebugger()
    {
        if (s_messagePumpGuardApplied)
        {
            return;
        }

        s_messagePumpGuardApplied = true;
        try
        {
            System.Windows.Interop.ComponentDispatcher.ThreadFilterMessage +=
                (ref System.Windows.Interop.MSG msg, ref bool handled) => FpExceptionGuard.TryMask();
            System.Windows.Interop.ComponentDispatcher.ThreadPreprocessMessage +=
                (ref System.Windows.Interop.MSG msg, ref bool handled) => FpExceptionGuard.TryMask();
            FpExceptionGuard.Diag("WPF message-pump FPU guard installed for native-debug startup");
        }
        catch (Exception ex)
        {
            FpExceptionGuard.Diag($"WPF message-pump FPU guard install FAILED {ex.GetType().FullName}: {ex.Message}");
        }
    }

    private static void DisableVisualDiagnosticsForDebugger()
    {
        if (s_visualDiagnosticsGuardApplied)
        {
            return;
        }

        s_visualDiagnosticsGuardApplied = true;
        try
        {
            System.Windows.Diagnostics.VisualDiagnostics.DisableVisualTreeChanged();
            FpExceptionGuard.Diag("WPF VisualDiagnostics disabled for native-debug startup");
        }
        catch (Exception ex)
        {
            FpExceptionGuard.Diag($"WPF VisualDiagnostics disable FAILED {ex.GetType().Name}: {ex.Message}");
        }
    }

    private static bool IsNativeDebuggerAttached()
    {
        try
        {
            return IsDebuggerPresent();
        }
        catch
        {
            return false;
        }
    }

    private static void PreloadNativeBridgeForDebugger(string[] cmdArgs)
    {
        var preloadNative = cmdArgs.Any(arg => string.Equals(arg, "--preload-native", StringComparison.OrdinalIgnoreCase));
        var breakIntoDebugger = cmdArgs.Any(arg => string.Equals(arg, "--native-debug-break", StringComparison.OrdinalIgnoreCase));
        if (!preloadNative && !breakIntoDebugger)
        {
            return;
        }

        var result = MptiNativeBridge.DebugProbe(breakIntoDebugger);
        DiagnosticsLog.Write(
            $"Native preload: available={result.Available} success={result.Success} code={result.Code} message={result.Message}");
    }

    private static bool IsRecoverableWpfRenderException(Exception exception)
    {
        if (exception is not ArithmeticException)
        {
            return false;
        }

        var stack = exception.StackTrace ?? string.Empty;
        return stack.Contains("System.Windows.Media.Matrix.CreateRotationRadians", StringComparison.Ordinal)
            || stack.Contains("System.Windows.Media.RotateTransform.get_Value", StringComparison.Ordinal)
            || stack.Contains("System.Windows.Media.TransformGroup.get_Value", StringComparison.Ordinal);
    }
}
