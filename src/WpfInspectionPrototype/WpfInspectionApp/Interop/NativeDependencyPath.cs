using System;
using System.IO;
using System.Runtime.InteropServices;

namespace WpfInspectionApp.Interop;

internal static class NativeDependencyPath
{
    private const string PemtronSystemPath = @"C:\Program Files\Pemtron System";
    private static bool _initialized;

    public static void EnsureInitialized()
    {
        if (_initialized)
        {
            return;
        }

        if (Directory.Exists(PemtronSystemPath))
        {
            SetDllDirectory(PemtronSystemPath);
        }

        _initialized = true;
    }

    [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    private static extern bool SetDllDirectory(string lpPathName);
}
