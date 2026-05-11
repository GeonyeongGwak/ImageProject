using System.Configuration;
using System.Data;
using System.Windows;
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
        base.OnStartup(e);
    }
}


