using System.Windows;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.AlgorithmPanels;

public interface IAlgorithmPanel
{
    string AlgorithmType { get; }
    FrameworkElement View { get; }
    void Bind(AlgorithmPanelContext context);
    void Unbind();
}

public sealed class AlgorithmPanelContext
{
    public InspectionModel Model { get; set; } = null!;
    public InspectionWindowData Window { get; set; } = null!;
    public InspectionAlgorithmData Algorithm { get; set; } = null!;
    public Action<AlgorithmPanelRequest>? Request { get; set; }

    public void RequestPreviewUpdate()
    {
        Request?.Invoke(AlgorithmPanelRequest.PreviewUpdate());
    }

    public void RequestTreeRefresh()
    {
        Request?.Invoke(AlgorithmPanelRequest.TreeRefresh());
    }

    public void RequestWindowRoiDrawing()
    {
        Request?.Invoke(AlgorithmPanelRequest.WindowRoiDrawing());
    }

    public void RequestAlgorithmRoiDrawing()
    {
        Request?.Invoke(AlgorithmPanelRequest.AlgorithmRoiDrawing());
    }

    public void SetParameter(string name, string value)
    {
        Request?.Invoke(AlgorithmPanelRequest.SetParameter(name, value));
    }
}

