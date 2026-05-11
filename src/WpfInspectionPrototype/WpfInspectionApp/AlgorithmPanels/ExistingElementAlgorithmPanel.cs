using System.Windows;

namespace WpfInspectionApp.AlgorithmPanels;

public sealed class ExistingElementAlgorithmPanel(string algorithmType, FrameworkElement view) : IAlgorithmPanel
{
    private AlgorithmPanelContext? _context;

    public string AlgorithmType { get; } = algorithmType;
    public FrameworkElement View { get; } = view;

    public void Bind(AlgorithmPanelContext context)
    {
        _context = context;
        View.DataContext = context;
    }

    public void Unbind()
    {
        _context = null;
        View.DataContext = null;
    }
}

