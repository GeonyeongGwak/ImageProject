using System.Configuration;
using System.Data;
using System.Diagnostics;
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
    public static AppServices Services { get; } = new();

    protected override void OnStartup(StartupEventArgs e)
    {
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

        base.OnStartup(e);
    }

    private static void ApplyDebuggerRenderGuard(string[] cmdArgs)
    {
        if (!Debugger.IsAttached
            && !cmdArgs.Any(arg => string.Equals(arg, "--software-rendering", StringComparison.OrdinalIgnoreCase)))
        {
            return;
        }

        System.Windows.Media.RenderOptions.ProcessRenderMode = System.Windows.Interop.RenderMode.SoftwareOnly;
        DiagnosticsLog.Write("WPF software rendering enabled for debugger stability.");
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
