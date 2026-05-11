using System.IO;

namespace WpfInspectionApp;

internal static class DiagnosticsLog
{
    public static string Path => System.IO.Path.Combine(System.IO.Path.GetTempPath(), "WpfInspectionApp.log");

    public static void Write(string message)
    {
        try
        {
            File.AppendAllText(Path, $"[{DateTime.Now:yyyy-MM-dd HH:mm:ss.fff}] {message}{Environment.NewLine}");
        }
        catch
        {
            // Logging must never break inspection UI startup.
        }
    }
}
