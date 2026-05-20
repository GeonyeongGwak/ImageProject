using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.AlgorithmPanels;

public class DynamicAlgorithmPanel : IAlgorithmPanel
{
    private readonly AlgorithmCatalogItem _catalog;
    private readonly AlgorithmReferenceUiProfile _profile;
    private readonly Border _root;
    private readonly StackPanel _content;
    private readonly AlgorithmPanelInteraction _interaction;
    private readonly AlgorithmPanelControlStatePolicy _controlStatePolicy;
    private AlgorithmPanelContext? _context;
    private bool _binding;

    protected DynamicAlgorithmPanel(string algorithmType)
        : this(AlgorithmCatalog.Find(algorithmType))
    {
    }

    public DynamicAlgorithmPanel(AlgorithmCatalogItem catalog)
    {
        _catalog = catalog;
        _profile = AlgorithmReferenceUiCatalog.Create(catalog);
        _content = new StackPanel();
        _root = AlgorithmPanelUi.RootBorder(_content);
        _interaction = new AlgorithmPanelInteraction(
            _catalog,
            () => _binding,
            () => _context,
            Rebuild,
            OnAlgorithmSpecificCommand,
            OnAlgorithmSpecificParameterChanged);
        _controlStatePolicy = new AlgorithmPanelControlStatePolicy(_catalog, _interaction.Read);
    }

    public string AlgorithmType => _catalog.Type;
    public FrameworkElement View => _root;
    protected AlgorithmCatalogItem Catalog => _catalog;
    protected AlgorithmPanelContext? Context => _context;
    protected AlgorithmPanelInteraction Interaction => _interaction;

    public void Bind(AlgorithmPanelContext context)
    {
        _context = context;
        _context.Algorithm.ApplyCatalogDefaults();
        ApplyDefaults(context);
        Rebuild();
    }

    public void Unbind()
    {
        _context = null;
        _content.Children.Clear();
        _content.Children.Add(Text("Select or add an Algorithm.", 12, "#8BA5C4", FontWeights.SemiBold));
    }

    private void Rebuild()
    {
        if (_context == null)
        {
            Unbind();
            return;
        }

        _binding = true;
        _content.Children.Clear();
        _content.Children.Add(Text($"{_catalog.DisplayName.ToUpperInvariant()} ALGORITHM", 13, "#27A6FF", FontWeights.Bold));
        _content.Children.Add(Text($"{_profile.SourceControl} | {_context.Window.Name}", 12, "#D6ECFF", FontWeights.SemiBold));

        var commandGrid = new UniformGrid { Columns = 4, Margin = new Thickness(0, 6, 0, 6) };
        commandGrid.Children.Add(Button("Draw Algorithm ROI", () => AlgorithmPanelCommonEvents.RequestAlgorithmRoi(_context)));
        commandGrid.Children.Add(Button("Teach", () => _interaction.ExecuteCommand($"{_catalog.ParameterFamily}.TeachRequested", rebuild: false)));
        commandGrid.Children.Add(Button("Search", () => _interaction.ExecuteCommand($"{_catalog.ParameterFamily}.SearchRequested", rebuild: false)));
        commandGrid.Children.Add(Button("Apply All", () => _interaction.ExecuteCommand("Command.ApplyAllTarget", rebuild: false)));
        _content.Children.Add(commandGrid);

        var tabs = new TabControl
        {
            Margin = new Thickness(0, 4, 0, 0)
        };
        foreach (var group in _profile.Controls.GroupBy(control => control.Tab))
        {
            tabs.Items.Add(Tab(group.Key, BuildReferenceTab(group.Key, group.ToList())));
        }
        AddCustomTabs(tabs);
        _content.Children.Add(tabs);
        _binding = false;
    }

    protected void RequestRebuild()
    {
        Rebuild();
    }

    protected virtual void ApplyDefaults(AlgorithmPanelContext context)
    {
    }

    protected virtual void AddCustomTabs(TabControl tabs)
    {
    }

    private FrameworkElement BuildReferenceTab(string tab, List<AlgorithmReferenceControl> controls)
    {
        var panel = TabPanel();
        var editGrid = AlgorithmPanelUi.EditGrid();

        if (tab == "ROI / Mask")
        {
            var roiText = _context?.Algorithm.AlgorithmRoi.HasValue == true
                ? FormatRoi(_context.Algorithm.AlgorithmRoi.Value)
                : "none - uses Window ROI";
            panel.Children.Add(Text($"Algorithm ROI: {roiText}", 12, "#D6ECFF", FontWeights.Bold));
            panel.Children.Add(Button("Draw Algorithm ROI", () => AlgorithmPanelCommonEvents.RequestAlgorithmRoi(_context)));
        }

        if (tab == "Events")
        {
            panel.Children.Add(Text(_profile.EventSummary, 12, "#8BA5C4", FontWeights.SemiBold));
        }

        foreach (var control in controls)
        {
            editGrid.Children.Add(BuildReferenceControl(control));
        }

        panel.Children.Add(editGrid);
        return panel;
    }

    private FrameworkElement BuildReferenceControl(AlgorithmReferenceControl control)
    {
        var element = control.Kind switch
        {
            AlgorithmReferenceControlKind.Check => Check(control.Label, control.Key, control.DefaultValue.Equals("true", StringComparison.OrdinalIgnoreCase)),
            AlgorithmReferenceControlKind.Number => Number(control.Label, control.Key, control.DefaultValue),
            AlgorithmReferenceControlKind.Slider => Slider(control.Label, control.Key, control.Minimum, control.Maximum, control.DefaultValue),
            AlgorithmReferenceControlKind.Combo => Combo(control.Label, control.Key, control.Options ?? [], _interaction.ReadIndex(control.Key, control.DefaultValue)),
            AlgorithmReferenceControlKind.Command => Button(control.Label, () => _interaction.ExecuteCommand(control.Key, rebuild: false)),
            _ => Text(control.Label, 12, "#8BA5C4", FontWeights.SemiBold)
        };
        element.IsEnabled = _controlStatePolicy.IsControlEnabled(control);
        return element;
    }

    private CheckBox Check(string label, string key, bool fallback)
    {
        return AlgorithmPanelUi.Check(label, _interaction.Read(key, fallback ? "true" : "false"), fallback, value => _interaction.Write(key, value));
    }

    private FrameworkElement Number(string label, string key, string fallback)
    {
        return AlgorithmPanelUi.Number(label, _interaction.Read(key, fallback), value => _interaction.Write(key, value));
    }

    private FrameworkElement Slider(string label, string key, int min, int max, string fallback)
    {
        return AlgorithmPanelUi.Slider(label, _interaction.Read(key, fallback), min, max, fallback, value => _interaction.Write(key, value));
    }

    private FrameworkElement Combo(string label, string key, string[] values, int selectedIndex)
    {
        return AlgorithmPanelUi.Combo(label, values, selectedIndex, value => _interaction.Write(key, value.ToString()));
    }

    private FrameworkElement Row(string label, FrameworkElement editor)
    {
        return AlgorithmPanelUi.Row(label, editor);
    }

    private static StackPanel TabPanel()
    {
        return AlgorithmPanelUi.TabPanel();
    }

    private static TabItem Tab(string header, FrameworkElement content)
    {
        return AlgorithmPanelUi.Tab(header, content);
    }

    private Button Button(string content, Action action)
    {
        return AlgorithmPanelUi.Button(content, action);
    }

    protected virtual bool OnAlgorithmSpecificCommand(string key)
    {
        return false;
    }

    protected virtual void OnAlgorithmSpecificParameterChanged(string key, string value)
    {
    }

    private static TextBlock Text(string text, double size, string color, FontWeight weight)
    {
        return AlgorithmPanelUi.Text(text, size, color, weight);
    }

    private static string FormatRoi(RoiRect roi)
    {
        return AlgorithmPanelCommonEvents.FormatRoi(roi);
    }
}

