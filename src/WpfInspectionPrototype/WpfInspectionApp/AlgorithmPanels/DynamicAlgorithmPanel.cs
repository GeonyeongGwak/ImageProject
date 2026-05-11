using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Media;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.AlgorithmPanels;

public class DynamicAlgorithmPanel : IAlgorithmPanel
{
    private readonly AlgorithmCatalogItem _catalog;
    private readonly AlgorithmReferenceUiProfile _profile;
    private readonly Border _root;
    private readonly StackPanel _content;
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
        _root = new Border
        {
            Padding = new Thickness(10),
            Margin = new Thickness(0, 0, 0, 10),
            Background = Brush("#0B1320"),
            BorderBrush = Brush("#155FA4"),
            BorderThickness = new Thickness(1),
            CornerRadius = new CornerRadius(4),
            Child = _content
        };
    }

    public string AlgorithmType => _catalog.Type;
    public FrameworkElement View => _root;
    protected AlgorithmCatalogItem Catalog => _catalog;
    protected AlgorithmPanelContext? Context => _context;

    public void Bind(AlgorithmPanelContext context)
    {
        _context = context;
        _context.Algorithm.ApplyCatalogDefaults();
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
        _content.Children.Add(Text($"{_catalog.Type} | {_catalog.Group}:{_catalog.LegacyName} ({_catalog.LegacyFlag}) | Family {_catalog.ParameterFamily}", 12, "#D6ECFF", FontWeights.SemiBold));
        _content.Children.Add(Text($"Reference UI: {_profile.SourceControl}", 12, "#80DFFF", FontWeights.Bold));
        _content.Children.Add(Text($"Window: {_context.Window.Name}", 12, "#8BA5C4", FontWeights.SemiBold));

        var commandGrid = new UniformGrid { Columns = 2, Margin = new Thickness(0, 8, 0, 8) };
        commandGrid.Children.Add(Button("Draw Algorithm ROI", () => AlgorithmPanelCommonEvents.RequestAlgorithmRoi(_context)));
        commandGrid.Children.Add(Button("Teach", () => ExecuteCommand($"{_catalog.ParameterFamily}.TeachRequested", rebuild: false)));
        commandGrid.Children.Add(Button("Search", () => ExecuteCommand($"{_catalog.ParameterFamily}.SearchRequested", rebuild: false)));
        commandGrid.Children.Add(Button("Apply All", () => ExecuteCommand("Command.ApplyAllTarget", rebuild: false)));
        _content.Children.Add(commandGrid);

        var tabs = new TabControl
        {
            Margin = new Thickness(0, 4, 0, 0)
        };
        foreach (var group in _profile.Controls.GroupBy(control => control.Tab))
        {
            tabs.Items.Add(Tab(group.Key, BuildReferenceTab(group.Key, group.ToList())));
        }
        _content.Children.Add(tabs);
        _binding = false;
    }

    protected void RequestRebuild()
    {
        Rebuild();
    }

    private FrameworkElement BuildReferenceTab(string tab, List<AlgorithmReferenceControl> controls)
    {
        var panel = TabPanel();

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
            panel.Children.Add(BuildReferenceControl(control));
        }

        return panel;
    }

    private FrameworkElement BuildReferenceControl(AlgorithmReferenceControl control)
    {
        var element = control.Kind switch
        {
            AlgorithmReferenceControlKind.Check => Check(control.Label, control.Key, control.DefaultValue.Equals("true", StringComparison.OrdinalIgnoreCase)),
            AlgorithmReferenceControlKind.Number => Number(control.Label, control.Key, control.DefaultValue),
            AlgorithmReferenceControlKind.Slider => Slider(control.Label, control.Key, control.Minimum, control.Maximum, control.DefaultValue),
            AlgorithmReferenceControlKind.Combo => Combo(control.Label, control.Key, control.Options ?? [], ReadIndex(control.Key, control.DefaultValue)),
            AlgorithmReferenceControlKind.Command => Button(control.Label, () => ExecuteCommand(control.Key, rebuild: false)),
            _ => Text(control.Label, 12, "#8BA5C4", FontWeights.SemiBold)
        };
        element.IsEnabled = IsControlEnabled(control);
        return element;
    }

    private CheckBox Check(string label, string key, bool fallback)
    {
        var check = new CheckBox
        {
            Content = label,
            IsChecked = Read(key, fallback ? "true" : "false").Equals("true", StringComparison.OrdinalIgnoreCase),
            Foreground = Brush("#D6E8FF"),
            FontWeight = FontWeights.SemiBold,
            Margin = new Thickness(0, 4, 0, 4)
        };
        check.Checked += (_, _) => Write(key, "true");
        check.Unchecked += (_, _) => Write(key, "false");
        return check;
    }

    private FrameworkElement Number(string label, string key, string fallback)
    {
        var box = new TextBox
        {
            Text = Read(key, fallback),
            Height = 28,
            Padding = new Thickness(7, 3, 7, 3),
            Foreground = Brush("#E4F3FF"),
            Background = Brush("#07101C"),
            BorderBrush = Brush("#284A72")
        };
        box.TextChanged += (_, _) => Write(key, box.Text);
        return Row(label, box);
    }

    private FrameworkElement Slider(string label, string key, int min, int max, string fallback)
    {
        var initial = int.TryParse(Read(key, fallback), out var value) ? value : int.Parse(fallback);
        var valueText = Text(initial.ToString(), 12, "#FFB020", FontWeights.Bold);
        var slider = new Slider
        {
            Minimum = min,
            Maximum = max,
            Value = Net48Compat.Clamp(initial, min, max),
            TickFrequency = 1
        };
        slider.ValueChanged += (_, args) =>
        {
            var next = ((int)Math.Round(args.NewValue)).ToString();
            valueText.Text = next;
            Write(key, next);
        };

        var grid = new Grid { Margin = new Thickness(0, 4, 0, 8) };
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(90) });
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(46) });
        grid.Children.Add(Text(label, 12, "#9DB4D0", FontWeights.Normal));
        Grid.SetColumn(slider, 1);
        grid.Children.Add(slider);
        Grid.SetColumn(valueText, 2);
        grid.Children.Add(valueText);
        return grid;
    }

    private FrameworkElement Combo(string label, string key, string[] values, int selectedIndex)
    {
        var combo = new ComboBox
        {
            Height = 28,
            Foreground = Brush("#E4F3FF"),
            Background = Brush("#07101C"),
            BorderBrush = Brush("#284A72"),
            ItemsSource = values,
            SelectedIndex = Net48Compat.Clamp(selectedIndex, 0, Math.Max(0, values.Length - 1))
        };
        combo.SelectionChanged += (_, _) => Write(key, Math.Max(0, combo.SelectedIndex).ToString());
        return Row(label, combo);
    }

    private FrameworkElement Row(string label, FrameworkElement editor)
    {
        var grid = new Grid { Margin = new Thickness(0, 4, 0, 8) };
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(110) });
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
        grid.Children.Add(Text(label, 12, "#9DB4D0", FontWeights.Normal));
        Grid.SetColumn(editor, 1);
        grid.Children.Add(editor);
        return grid;
    }

    private static StackPanel TabPanel()
    {
        return new StackPanel { Margin = new Thickness(0, 8, 0, 0) };
    }

    private static TabItem Tab(string header, FrameworkElement content)
    {
        return new TabItem { Header = header, Content = content };
    }

    private Button Button(string content, Action action)
    {
        var button = new Button
        {
            Content = content,
            MinHeight = 30,
            Margin = new Thickness(0, 0, 6, 6),
            Padding = new Thickness(10, 4, 10, 4),
            Foreground = Brush("#E4F3FF"),
            Background = Brush("#10243D"),
            BorderBrush = Brush("#236EA8"),
            FontWeight = FontWeights.SemiBold
        };
        button.Click += (_, _) => action();
        return button;
    }

    private string Read(string key, string fallback)
    {
        return _context?.Algorithm.Parameters.TryGetValue(key, out var value) == true ? value : fallback;
    }

    private void Write(string key, string value)
    {
        if (_binding || _context == null)
        {
            return;
        }

        AlgorithmPanelCommonEvents.WriteParameter(_context, key, value);
        OnAlgorithmSpecificParameterChanged(key, value);

        if (ShouldRebuildForDependency(key))
        {
            Rebuild();
        }
    }

    private void ExecuteCommand(string key, bool rebuild)
    {
        if (_context == null)
        {
            return;
        }

        if (AlgorithmPanelCommonEvents.IsCommonCommand(_catalog, key))
        {
            AlgorithmPanelCommonEvents.TriggerCommand(_context, _catalog, key);
        }
        else if (!OnAlgorithmSpecificCommand(key))
        {
            AlgorithmPanelCommonEvents.TriggerCommand(_context, _catalog, key);
        }

        if (rebuild)
        {
            Rebuild();
        }
    }

    protected virtual bool OnAlgorithmSpecificCommand(string key)
    {
        return false;
    }

    protected virtual void OnAlgorithmSpecificParameterChanged(string key, string value)
    {
    }

    private bool IsControlEnabled(AlgorithmReferenceControl control)
    {
        if (control.Kind == AlgorithmReferenceControlKind.Check || control.Kind == AlgorithmReferenceControlKind.Command)
        {
            return true;
        }

        var key = control.Key;
        var prefix = $"{_catalog.ParameterFamily}.";
        if (!key.StartsWith(prefix, StringComparison.OrdinalIgnoreCase))
        {
            return true;
        }

        return IsFamilyControlEnabled(key);
    }

    private bool IsFamilyControlEnabled(string key)
    {
        var family = _catalog.ParameterFamily;

        if (ContainsAny(key, "MinValue", "MaxValue", "Range2D", "SubLine2D", "Binary"))
        {
            return ReadBool($"{family}.Use2D", true) && ReadBool($"{family}.b2DCheck", true);
        }

        if (ContainsAny(key, "Height", "Range3D", "SubLine3D", "Volume", "Coplanarity", "Flux"))
        {
            return ReadBool($"{family}.Use3D", false) || ReadBool($"{family}.UseHeight", false) || ReadBool($"{family}.UseVolume", false);
        }

        if (ContainsAny(key, "Color", "MinR", "MaxR", "MinG", "MaxG", "MinB", "MaxB"))
        {
            return ReadBool($"{family}.UseColor", true);
        }

        if (ContainsAny(key, "Filter"))
        {
            return ReadBool($"{family}.UseFilter", false);
        }

        if (ContainsAny(key, "ShiftX"))
        {
            return ReadBool($"{family}.UseShift", false) && ReadBool($"{family}.UseShiftX", true);
        }

        if (ContainsAny(key, "ShiftY"))
        {
            return ReadBool($"{family}.UseShift", false) && ReadBool($"{family}.UseShiftY", true);
        }

        if (ContainsAny(key, "Shift"))
        {
            return ReadBool($"{family}.UseShift", false);
        }

        if (ContainsAny(key, "DistanceX"))
        {
            return ReadBool($"{family}.UseDistanceX", false);
        }

        if (ContainsAny(key, "DistanceY"))
        {
            return ReadBool($"{family}.UseDistanceY", false);
        }

        if (ContainsAny(key, "Distance", "Toler_Dist", "Offset_Dist"))
        {
            return ReadBool($"{family}.UseDistance", false) || ReadBool($"{family}.UseDist", false);
        }

        if (ContainsAny(key, "Angle"))
        {
            return ReadBool($"{family}.UseAngle", false);
        }

        if (ContainsAny(key, "Area"))
        {
            return ReadBool($"{family}.UseArea", true);
        }

        if (ContainsAny(key, "Width"))
        {
            return ReadBool($"{family}.UseWidth", false);
        }

        if (ContainsAny(key, "Length"))
        {
            return ReadBool($"{family}.UseLength", false);
        }

        if (ContainsAny(key, "Anchor"))
        {
            return ReadBool($"{family}.UseAnchor", false);
        }

        return true;
    }

    private bool ShouldRebuildForDependency(string key)
    {
        return key.EndsWith(".Use2D", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".b2DCheck", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".Use3D", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseHeight", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseVolume", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseColor", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseFilter", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseShift", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseShiftX", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseShiftY", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseDistance", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseDist", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseDistanceX", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseDistanceY", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseAngle", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseArea", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseWidth", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseLength", StringComparison.OrdinalIgnoreCase)
            || key.EndsWith(".UseAnchor", StringComparison.OrdinalIgnoreCase);
    }

    private bool ReadBool(string key, bool fallback)
    {
        var value = Read(key, fallback ? "true" : "false");
        if (bool.TryParse(value, out var parsed))
        {
            return parsed;
        }

        return int.TryParse(value, out var number) ? number != 0 : fallback;
    }

    private static bool ContainsAny(string value, params string[] needles)
    {
        return needles.Any(needle => value.Contains(needle, StringComparison.OrdinalIgnoreCase));
    }

    private int ReadIndex(string key, string fallback)
    {
        return int.TryParse(Read(key, fallback), out var value) ? value : 0;
    }

    private static TextBlock Text(string text, double size, string color, FontWeight weight)
    {
        return new TextBlock
        {
            Text = text,
            FontSize = size,
            Foreground = Brush(color),
            FontWeight = weight,
            TextWrapping = TextWrapping.Wrap,
            Margin = new Thickness(0, 0, 0, 6)
        };
    }

    private static string FormatRoi(RoiRect roi)
    {
        return AlgorithmPanelCommonEvents.FormatRoi(roi);
    }

    private static SolidColorBrush Brush(string color)
    {
        return new SolidColorBrush((Color)ColorConverter.ConvertFromString(color));
    }
}

