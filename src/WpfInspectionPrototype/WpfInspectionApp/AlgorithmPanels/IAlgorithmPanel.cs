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
    public Action? RequestPreviewUpdate { get; set; }
    public Action? RequestTreeRefresh { get; set; }
    public Action? RequestWindowRoiDrawing { get; set; }
    public Action? RequestAlgorithmRoiDrawing { get; set; }
    public Action<string, string>? SetParameter { get; set; }
}

