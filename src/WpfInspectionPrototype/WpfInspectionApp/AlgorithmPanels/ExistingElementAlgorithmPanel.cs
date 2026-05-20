using System.Windows;

namespace WpfInspectionApp.AlgorithmPanels;

public sealed class ExistingElementAlgorithmPanel(string algorithmType, FrameworkElement view) : IAlgorithmPanel
{
    private AlgorithmPanelContext? _context;
    private bool _assignedContext;

    public string AlgorithmType { get; } = algorithmType;
    public FrameworkElement View { get; } = view;

    public void Bind(AlgorithmPanelContext context)
    {
        _context = context;
        if (View.ReadLocalValue(FrameworkElement.DataContextProperty) == DependencyProperty.UnsetValue)
        {
            View.DataContext = context;
            _assignedContext = true;
        }
    }

    public void Unbind()
    {
        _context = null;
        if (_assignedContext)
        {
            View.ClearValue(FrameworkElement.DataContextProperty);
            _assignedContext = false;
        }
    }
}

