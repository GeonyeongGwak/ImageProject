using System.Configuration;
using System.Data;
using System.Windows;
using WpfInspectionApp.Diagnostics;
using WpfInspectionApp.Infrastructure;

namespace WpfInspectionApp;

/// <summary>
/// Interaction logic for App.xaml
/// </summary>
public partial class App : Application
{
    public static AppServices Services { get; } = new();

    protected override void OnStartup(StartupEventArgs e)
    {
        DispatcherUnhandledException += (_, args) =>
        {
            DiagnosticsLog.Write($"DispatcherUnhandledException: {args.Exception}");
        };

        AppDomain.CurrentDomain.UnhandledException += (_, args) =>
        {
            DiagnosticsLog.Write($"UnhandledException: {args.ExceptionObject}");
        };

        DiagnosticsLog.Write("Application startup.");

        var cmdArgs = Environment.GetCommandLineArgs();
        DiagnosticsLog.Write($"Command line args count: {cmdArgs.Length}");
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
}



