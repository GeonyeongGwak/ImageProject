namespace WpfInspectionApp.Services;

public interface IPem3DViewerHostService : IDisposable
{
    void LoadIntoControl(string path, System.Windows.Forms.Panel hostPanel);

    void LoadExternalViewer(string path, System.Windows.Forms.Panel hostPanel);

    void ResizeExternalViewer(System.Windows.Forms.Panel hostPanel);

    void CloseControl(System.Windows.Forms.Panel hostPanel);

    void CloseExternalViewer();
}
