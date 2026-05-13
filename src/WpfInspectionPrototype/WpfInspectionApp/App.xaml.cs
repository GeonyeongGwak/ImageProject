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
    public static AppServices Services { get; } = new();

    protected override void OnStartup(StartupEventArgs e)
    {
        // MFC/IPP/MIL/PEM3D ActiveX, and VS native debugger hooks can unmask x87 FPU
        // exceptions. WPF transform math (RotateTransform / Matrix.CreateRotationRadians)
        // produces inexact fp results as normal output - if exceptions are unmasked the
        // CLR sees 0xC000008F (FE_INEXACT) and surfaces ArithmeticException. Re-mask
        // every fp exception so WPF behaves the same with or without native debugging.
        MaskAllFloatingPointExceptions();

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
        // Native preload (MptiBridge.dll + MFC + dependencies) is the most likely
        // place for the FPU control word to get clobbered. Re-mask after it loads.
        MaskAllFloatingPointExceptions();

        // Catch-all: re-mask immediately before each WPF render frame so any later
        // P/Invoke that unmasks fp exceptions cannot leak into Matrix/RotateTransform
        // math. _controlfp_s is ~1us, so the per-frame cost is negligible.
        System.Windows.Media.CompositionTarget.Rendering += (_, _) => MaskAllFloatingPointExceptions();
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

    // _MCW_EM (mask of all 6 fp exception bits) - see crtdefs.h:
    //   _EM_INVALID 0x10, _EM_DENORMAL 0x80000, _EM_ZERODIVIDE 0x08,
    //   _EM_OVERFLOW 0x04, _EM_UNDERFLOW 0x02, _EM_INEXACT 0x01.
    private const uint MCW_EM = 0x0008001F;

    [DllImport("ucrtbase.dll", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
    private static extern int _controlfp_s(out uint currentControl, uint newControl, uint mask);

    private static void MaskAllFloatingPointExceptions()
    {
        try
        {
            _controlfp_s(out _, MCW_EM, MCW_EM);
        }
        catch
        {
            // best effort; not all runtimes expose _controlfp_s identically.
        }
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
