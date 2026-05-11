using System.Collections.ObjectModel;
using System.Windows.Media;
using WpfInspectionApp.Infrastructure;

namespace WpfInspectionApp.ViewModels;

public enum InspectionTreeNodeKind
{
    Empty,
    Window,
    WindowInfo,
    Algorithm,
    AlgorithmRoi,
    InspectionResult
}

public sealed class InspectionTreeNodeViewModel : ViewModelBase
{
    private bool _isSelected;

    public string Header { get; init; } = "";

    public InspectionTreeNodeKind Kind { get; init; }

    public object? Payload { get; init; }

    public bool IsEnabled { get; init; } = true;

    public bool IsExpanded { get; set; } = true;

    public Brush? Foreground { get; init; }

    public ObservableCollection<InspectionTreeNodeViewModel> Children { get; } = [];

    public bool IsSelected
    {
        get => _isSelected;
        set => SetProperty(ref _isSelected, value);
    }
}
