using System.Windows;
using System.Reflection;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.AlgorithmPanels;

public sealed class AlgorithmPanelFactory
{
    private readonly Dictionary<string, IAlgorithmPanel> _panels = new(StringComparer.OrdinalIgnoreCase);
    private IAlgorithmPanel? _fallbackPanel;

    public void Register(string algorithmType, FrameworkElement view)
    {
        _panels[algorithmType] = new ExistingElementAlgorithmPanel(algorithmType, view);
    }

    public void Register(IAlgorithmPanel panel)
    {
        _panels[panel.AlgorithmType] = panel;
    }

    public void RegisterDiscoveredPanels()
    {
        var panelTypes = Assembly.GetExecutingAssembly()
            .GetTypes()
            .Where(type =>
                !type.IsAbstract &&
                typeof(IAlgorithmPanel).IsAssignableFrom(type) &&
                type.GetConstructor(Type.EmptyTypes) != null &&
                type != typeof(ExistingElementAlgorithmPanel))
            .OrderBy(type => type.Name, StringComparer.OrdinalIgnoreCase);

        foreach (var panelType in panelTypes)
        {
            if (Activator.CreateInstance(panelType) is IAlgorithmPanel panel)
            {
                if (!_panels.ContainsKey(panel.AlgorithmType))
                {
                    Register(panel);
                }
            }
        }
    }

    public void RegisterFallback(FrameworkElement view)
    {
        _fallbackPanel = new ExistingElementAlgorithmPanel("*", view);
    }

    public IAlgorithmPanel Resolve(string algorithmType)
    {
        var catalog = AlgorithmCatalog.Find(algorithmType);
        if (_panels.TryGetValue(catalog.Type, out var exactPanel))
        {
            return exactPanel;
        }

        if (_fallbackPanel == null)
        {
            throw new InvalidOperationException("Algorithm panel fallback is not registered.");
        }

        return _fallbackPanel;
    }

    public bool IsRegistered(string algorithmType)
    {
        var catalog = AlgorithmCatalog.Find(algorithmType);
        return _panels.ContainsKey(catalog.Type);
    }

    public IReadOnlyCollection<string> RegisteredAlgorithmTypes => _panels.Keys.ToArray();
}

