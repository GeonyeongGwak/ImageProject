using System;
using WpfInspectionApp.Diagnostics;

namespace WpfInspectionApp;

internal static class Program
{
    [STAThread]
    public static int Main(string[] args)
    {
        FpExceptionGuard.TryMask();
        FpExceptionGuard.Diag("Program.Main entered");

        try
        {
            FpExceptionGuard.Diag("Program.Main: constructing App");
            var app = new App();
            FpExceptionGuard.Diag("Program.Main: App constructed");

            app.InitializeComponent();
            FpExceptionGuard.Diag("Program.Main: App.InitializeComponent done");

            app.Run();
            FpExceptionGuard.Diag("Program.Main: App.Run returned");
            return 0;
        }
        catch (Exception ex)
        {
            FpExceptionGuard.Diag($"Program.Main THREW {ex.GetType().FullName}: {ex.Message}");
            throw;
        }
    }
}
