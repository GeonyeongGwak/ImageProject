using System.Configuration;
using System.Data;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Windows;
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

    [DllImport("kernel32.dll", ExactSpelling = true)]
    private static extern bool IsDebuggerPresent();

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

        // Catch-all #1: re-mask immediately before each WPF render frame.
        System.Windows.Media.CompositionTarget.Rendering += (_, _) => FpExceptionGuard.TryMask();
        // Catch-all #2: re-mask before EVERY dispatcher operation runs. Measure/Arrange/
        // input/etc. all go through the dispatcher - this catches anything that the
        // per-frame hook misses (e.g. layout passes happening between frames).
        Dispatcher.Hooks.OperationStarted += (_, _) => FpExceptionGuard.TryMask();
        Dispatcher.Hooks.OperationPosted  += (_, _) => FpExceptionGuard.TryMask();
        if (cmdArgs.Any(arg => string.Equals(arg, "--smoke-test", StringComparison.OrdinalIgnoreCase)))
        {
            // MainWindow is intentionally allowed to be created so that MptiBridge.dll
            // can load with full MFC/WPF context. The smoke test runs once the window
            // is loaded, then explicitly shuts the application down.
            ShutdownMode = ShutdownMode.OnExplicitShutdown;
            Startup += (_, _) =>
            {
                Dispatcher.BeginInvoke(new Action(() =>
                {
                    var exitCode = SmokeTestRunner.Run(Services);
                    DiagnosticsLog.Write($"Smoke test exit code: {exitCode}");
                    // Close MainWindow if present, then exit
                    if (MainWindow != null)
                    {
                        try { MainWindow.Close(); } catch { /* ignore */ }
                    }
                    Shutdown(exitCode);
                }), System.Windows.Threading.DispatcherPriority.ApplicationIdle);
            };
        }
        FpExceptionGuard.Diag("App.OnStartup: creating MainWindow");
        MainWindow = new MainWindow();
        FpExceptionGuard.Diag("App.OnStartup: MainWindow created, showing");
        MainWindow.Show();
        FpExceptionGuard.Diag("App.OnStartup: MainWindow.Show() returned");
        base.OnStartup(e);
        FpExceptionGuard.Diag("App.OnStartup: base.OnStartup done");
    }

    internal static void ApplyDebuggerRenderGuard(string[] cmdArgs)
    {
        var managedDebuggerAttached = Debugger.IsAttached;
        var nativeDebuggerAttached = IsNativeDebuggerAttached();
        var requestedByArg = cmdArgs.Any(arg => string.Equals(arg, "--software-rendering", StringComparison.OrdinalIgnoreCase));
        if (!managedDebuggerAttached && !nativeDebuggerAttached && !requestedByArg)
        {
            return;
        }

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
