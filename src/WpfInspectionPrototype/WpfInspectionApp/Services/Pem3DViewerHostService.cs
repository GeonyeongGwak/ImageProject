using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using PEM3DControlWrapper;

namespace WpfInspectionApp.Services;

public sealed class Pem3DViewerHostService : IPem3DViewerHostService
{
    private PEM3DControl_Winform? _pem3DControl;
    private System.Windows.Forms.Form? _pem3DHostForm;
    private Process? _viewerProcess;
    private IntPtr _viewerHandle;

    public void LoadIntoControl(string path, System.Windows.Forms.Panel hostPanel)
    {
        if (!File.Exists(path))
        {
            throw new FileNotFoundException("PTT file was not found.", path);
        }

        EnsureControl(hostPanel);
        _pem3DControl!.Helper.Clear();
        if (!_pem3DControl.Helper.Load(path))
        {
            throw new InvalidDataException("PEM3DControl failed to load the PTT file.");
        }

        _pem3DControl.ToolSetting.IsLight = false;
        _pem3DControl.Helper.Refresh();
    }

    public void LoadExternalViewer(string path, System.Windows.Forms.Panel hostPanel)
    {
        CloseControl(hostPanel);
        CloseExternalViewer();

        var viewerPath = FindPem3DViewerPath();
        var startInfo = new ProcessStartInfo
        {
            FileName = viewerPath,
            Arguments = QuoteProcessArgument(path),
            WorkingDirectory = Path.GetDirectoryName(viewerPath) ?? AppDomain.CurrentDomain.BaseDirectory,
            UseShellExecute = false
        };

        _viewerProcess = Process.Start(startInfo) ?? throw new InvalidOperationException("PEM3DViewer process could not be started.");
        _viewerHandle = WaitForMainWindowHandle(_viewerProcess, TimeSpan.FromSeconds(15));
        if (_viewerHandle == IntPtr.Zero)
        {
            throw new InvalidOperationException("PEM3DViewer main window was not created.");
        }

        var style = GetWindowLong(_viewerHandle, GWL_STYLE);
        style |= WS_CHILD | WS_VISIBLE;
        style &= ~(WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
        SetWindowLong(_viewerHandle, GWL_STYLE, style);
        SetParent(_viewerHandle, hostPanel.Handle);
        ResizeExternalViewer(hostPanel);
    }

    public void ResizeExternalViewer(System.Windows.Forms.Panel hostPanel)
    {
        if (_viewerHandle == IntPtr.Zero)
        {
            return;
        }

        MoveWindow(_viewerHandle, 0, 0, Math.Max(1, hostPanel.Width), Math.Max(1, hostPanel.Height), true);
    }

    public void CloseControl(System.Windows.Forms.Panel hostPanel)
    {
        if (_pem3DControl != null)
        {
            try
            {
                _pem3DControl.Helper.Clear();
                _pem3DControl.FormDispose();
                hostPanel.Controls.Remove(_pem3DControl);
                _pem3DControl.Dispose();
            }
            catch
            {
                // The PEM3D ActiveX wrapper owns native resources; shutdown failures are non-fatal.
            }
        }

        _pem3DControl = null;
        _pem3DHostForm?.Dispose();
        _pem3DHostForm = null;
    }

    public void CloseExternalViewer()
    {
        _viewerHandle = IntPtr.Zero;
        if (_viewerProcess == null)
        {
            return;
        }

        try
        {
            if (!_viewerProcess.HasExited)
            {
                _viewerProcess.Kill();
            }
        }
        catch
        {
            // External viewer shutdown is best-effort; a new import can start a fresh instance.
        }
        finally
        {
            _viewerProcess.Dispose();
            _viewerProcess = null;
        }
    }

    public void Dispose()
    {
        CloseExternalViewer();
        if (_pem3DControl != null)
        {
            try
            {
                _pem3DControl.Helper.Clear();
                _pem3DControl.FormDispose();
                _pem3DControl.Dispose();
            }
            catch
            {
            }
        }

        _pem3DControl = null;
        _pem3DHostForm?.Dispose();
        _pem3DHostForm = null;
    }

    private void EnsureControl(System.Windows.Forms.Panel hostPanel)
    {
        if (_pem3DControl != null)
        {
            return;
        }

        _pem3DHostForm = new System.Windows.Forms.Form();
        _pem3DControl = new PEM3DControl_Winform(_pem3DHostForm, true)
        {
            Dock = System.Windows.Forms.DockStyle.Fill
        };
        _pem3DControl.ToolSetting.IsLight = false;
        hostPanel.Controls.Clear();
        hostPanel.Controls.Add(_pem3DControl);
    }

    private static string FindPem3DViewerPath()
    {
        const string referencePath = @"D:\Work\GGY\pemtoFrameworkAll_R_4.0.0.7\bin\x64\Release\PEM3DViewer.exe";
        if (File.Exists(referencePath))
        {
            return referencePath;
        }

        var localPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "PEM3DViewer.exe");
        if (File.Exists(localPath))
        {
            return localPath;
        }

        throw new FileNotFoundException("PEM3DViewer.exe was not found.", localPath);
    }

    private static string QuoteProcessArgument(string value)
    {
        return "\"" + value.Replace("\"", "\\\"") + "\"";
    }

    private static IntPtr WaitForMainWindowHandle(Process process, TimeSpan timeout)
    {
        var stopwatch = Stopwatch.StartNew();
        while (!process.HasExited && stopwatch.Elapsed < timeout)
        {
            process.Refresh();
            if (process.MainWindowHandle != IntPtr.Zero)
            {
                return process.MainWindowHandle;
            }

            Thread.Sleep(100);
        }

        process.Refresh();
        return process.MainWindowHandle;
    }

    private const int GWL_STYLE = -16;
    private const int WS_CHILD = 0x40000000;
    private const int WS_VISIBLE = 0x10000000;
    private const int WS_CAPTION = 0x00C00000;
    private const int WS_THICKFRAME = 0x00040000;
    private const int WS_SYSMENU = 0x00080000;
    private const int WS_MINIMIZEBOX = 0x00020000;
    private const int WS_MAXIMIZEBOX = 0x00010000;

    [DllImport("user32.dll")]
    private static extern IntPtr SetParent(IntPtr hWndChild, IntPtr hWndNewParent);

    [DllImport("user32.dll")]
    private static extern bool MoveWindow(IntPtr hWnd, int x, int y, int nWidth, int nHeight, bool bRepaint);

    [DllImport("user32.dll", SetLastError = true)]
    private static extern int GetWindowLong(IntPtr hWnd, int nIndex);

    [DllImport("user32.dll", SetLastError = true)]
    private static extern int SetWindowLong(IntPtr hWnd, int nIndex, int dwNewLong);
}
