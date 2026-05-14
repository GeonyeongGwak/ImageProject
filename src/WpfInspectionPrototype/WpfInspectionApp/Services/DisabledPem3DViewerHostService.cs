using WpfInspectionApp.Infrastructure;

namespace WpfInspectionApp.Services;

// No-op replacement for Pem3DViewerHostService. Wired in AppServices instead of the
// real implementation to keep PEM3DControl OCX / ActiveX wrappers from being
// instantiated. The PEM3DControl COM/OCX init is suspected of unmasking the x87 FPU
// exception bits (or triggering native D3D probes that do) and causing 0xC000008F
// during WPF Application init under VS native debugging.
//
// PEM3DControl* assemblies remain referenced in csproj (so PTT file metadata classes
// that may use them stay compilable), but no PEM3D type is ever touched at runtime
// from this service - the JIT never resolves PEM3DControlWrapper for this class.
internal sealed class DisabledPem3DViewerHostService : IPem3DViewerHostService
{
    public void LoadIntoControl(string path, System.Windows.Forms.Panel hostPanel)
    {
        DiagnosticsLog.Write($"Pem3D viewer DISABLED - skipping LoadIntoControl({path})");
    }

    public void LoadExternalViewer(string path, System.Windows.Forms.Panel hostPanel)
    {
        DiagnosticsLog.Write($"Pem3D viewer DISABLED - skipping LoadExternalViewer({path})");
    }

    public void ResizeExternalViewer(System.Windows.Forms.Panel hostPanel)
    {
        // no-op
    }

    public void CloseControl(System.Windows.Forms.Panel hostPanel)
    {
        // no-op
    }

    public void CloseExternalViewer()
    {
        // no-op
    }

    public void Dispose()
    {
        // no-op
    }
}
