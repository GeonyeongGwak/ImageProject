using System.Globalization;
using System.Text.Json;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Media;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.AlgorithmPanels.Types;

public sealed class AlignEdgeAlgorithmPanel : IAlgorithmPanel
{
    private readonly Border _root;
    private readonly StackPanel _content;
    private AlgorithmPanelContext? _context;
    private bool _binding;

    public AlignEdgeAlgorithmPanel()
    {
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

    public string AlgorithmType => "AlgoAlignEdge";
    public FrameworkElement View => _root;

    public void Bind(AlgorithmPanelContext context)
    {
        _context = context;
        _context.Algorithm.ApplyCatalogDefaults();
        EnsureDefaults();
        Rebuild();
    }

    public void Unbind()
    {
        _context = null;
        _content.Children.Clear();
        _content.Children.Add(Text("Select an AlignEdge algorithm.", 12, "#8BA5C4", FontWeights.SemiBold));
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
        _content.Children.Add(Text("ALIGN EDGE ALGORITHM", 13, "#27A6FF", FontWeights.Bold));
        _content.Children.Add(Text("Area / Line edge teaching, 2D/3D range, angle, shift, distance and anchor options", 12, "#D6ECFF", FontWeights.SemiBold));

        var commandGrid = new UniformGrid { Columns = 2, Margin = new Thickness(0, 6, 0, 8) };
        commandGrid.Children.Add(Button("Draw Algorithm ROI", () => AlgorithmPanelCommonEvents.RequestAlgorithmRoi(_context)));
        commandGrid.Children.Add(Button("Teach", TeachCurrentValues));
        _content.Children.Add(commandGrid);

        var tabs = new TabControl();
        tabs.Items.Add(Tab("Edge", BuildEdgeTab()));
        tabs.Items.Add(Tab("Range", BuildRangeTab()));
        tabs.Items.Add(Tab("Measure", BuildMeasureTab()));
        tabs.Items.Add(Tab("Anchor", BuildAnchorTab()));
        tabs.Items.Add(Tab("Serialize", BuildSerializeTab()));
        _content.Children.Add(tabs);
        _binding = false;
    }

    private FrameworkElement BuildEdgeTab()
    {
        var panel = TabPanel();
        panel.Children.Add(Combo("Direction", "AlignEdge.Direction", ["LeftToRight", "RightToLeft", "TopToBottom", "BottomToTop"], ReadIndex("AlignEdge.Direction", "0")));
        panel.Children.Add(Slider("Threshold", "AlignEdge.Threshold", 0, 255, Read("AlignEdge.Threshold", "128")));
        panel.Children.Add(Number("Search Width", "AlignEdge.SearchWidth", "40"));
        panel.Children.Add(Number("Search Height", "AlignEdge.SearchHeight", "12"));
        panel.Children.Add(Check("Use Peak Edge", "AlignEdge.UsePeak", true));
        panel.Children.Add(Check("Use Sub Pixel", "AlignEdge.UseSubPixel", true));
        return panel;
    }

    private FrameworkElement BuildRangeTab()
    {
        var panel = TabPanel();
        panel.Children.Add(Check("Use 2D Range", "AlignEdge.Use2D", true));
        panel.Children.Add(Number("2D Min", "AlignEdge.Min2D", "0"));
        panel.Children.Add(Number("2D Max", "AlignEdge.Max2D", "255"));
        panel.Children.Add(Check("Use 3D Range", "AlignEdge.Use3D", false));
        panel.Children.Add(Number("3D Min", "AlignEdge.Min3D", "0"));
        panel.Children.Add(Number("3D Max", "AlignEdge.Max3D", "1000"));
        return panel;
    }

    private FrameworkElement BuildMeasureTab()
    {
        var panel = TabPanel();
        panel.Children.Add(Check("Use Distance", "AlignEdge.UseDistance", true));
        panel.Children.Add(Number("Distance Spec", "AlignEdge.DistanceSpec", "0"));
        panel.Children.Add(Number("Distance Tol", "AlignEdge.DistanceTolerance", "10"));
        panel.Children.Add(Check("Use Angle", "AlignEdge.UseAngle", true));
        panel.Children.Add(Number("Angle Spec", "AlignEdge.AngleSpec", "0"));
        panel.Children.Add(Number("Angle Tol", "AlignEdge.AngleTolerance", "5"));
        panel.Children.Add(Check("Use Shift", "AlignEdge.UseShift", true));
        panel.Children.Add(Number("Shift X", "AlignEdge.ShiftX", "0"));
        panel.Children.Add(Number("Shift Y", "AlignEdge.ShiftY", "0"));
        return panel;
    }

    private FrameworkElement BuildAnchorTab()
    {
        var panel = TabPanel();
        panel.Children.Add(Check("Use Anchor", "AlignEdge.UseAnchor", false));
        panel.Children.Add(Combo("Anchor Mode", "AlignEdge.AnchorMode", ["Window ROI", "Algorithm ROI", "Last Align"], ReadIndex("AlignEdge.AnchorMode", "0")));
        panel.Children.Add(Number("Anchor X", "AlignEdge.AnchorX", "0"));
        panel.Children.Add(Number("Anchor Y", "AlignEdge.AnchorY", "0"));
        panel.Children.Add(Button("Capture Anchor", CaptureAnchor));
        return panel;
    }

    private FrameworkElement BuildSerializeTab()
    {
        var panel = TabPanel();
        var json = JsonSerializer.Serialize(_context?.Algorithm.Parameters ?? [], new JsonSerializerOptions { WriteIndented = true });
        var box = new TextBox
        {
            Text = json,
            MinHeight = 120,
            AcceptsReturn = true,
            TextWrapping = TextWrapping.Wrap,
            VerticalScrollBarVisibility = ScrollBarVisibility.Auto,
            Foreground = Brush("#E4F3FF"),
            Background = Brush("#07101C"),
            BorderBrush = Brush("#284A72")
        };
        panel.Children.Add(box);
        return panel;
    }

    private void EnsureDefaults()
    {
        WriteDefault("AlignEdge.Direction", "0");
        WriteDefault("AlignEdge.Threshold", "128");
        WriteDefault("AlignEdge.SearchWidth", "40");
        WriteDefault("AlignEdge.SearchHeight", "12");
        WriteDefault("AlignEdge.UsePeak", "true");
        WriteDefault("AlignEdge.UseSubPixel", "true");
        WriteDefault("AlignEdge.Use2D", "true");
        WriteDefault("AlignEdge.Use3D", "false");
        WriteDefault("AlignEdge.UseDistance", "true");
        WriteDefault("AlignEdge.UseAngle", "true");
        WriteDefault("AlignEdge.UseShift", "true");
    }

    private void WriteDefault(string key, string value)
    {
        if (_context != null && !_context.Algorithm.Parameters.ContainsKey(key))
        {
            _context.Algorithm.Parameters[key] = value;
        }
    }

    private void TeachCurrentValues()
    {
        if (_context == null)
        {
            return;
        }

        var roi = _context.Algorithm.AlgorithmRoi ?? _context.Window.Roi;
        Write("AlignEdge.TeachRoi", FormatRoi(roi));
        AlgorithmPanelCommonEvents.TriggerCommand(_context, AlgorithmCatalog.Find("AlgoAlignEdge"), "AlignEdge.TeachRequested");
        Rebuild();
    }

    private void CaptureAnchor()
    {
        if (_context == null)
        {
            return;
        }

        var roi = _context.Algorithm.AlgorithmRoi ?? _context.Window.Roi;
        Write("AlignEdge.AnchorX", (roi.X + roi.Width / 2).ToString(CultureInfo.InvariantCulture));
        Write("AlignEdge.AnchorY", (roi.Y + roi.Height / 2).ToString(CultureInfo.InvariantCulture));
        Rebuild();
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
        var initial = int.TryParse(Read(key, fallback), out var value) ? value : int.Parse(fallback, CultureInfo.InvariantCulture);
        var valueText = Text(initial.ToString(CultureInfo.InvariantCulture), 12, "#FFB020", FontWeights.Bold);
        var slider = new Slider
        {
            Minimum = min,
            Maximum = max,
            Value = Net48Compat.Clamp(initial, min, max),
            TickFrequency = 1
        };
        slider.ValueChanged += (_, args) =>
        {
            var next = ((int)Math.Round(args.NewValue)).ToString(CultureInfo.InvariantCulture);
            valueText.Text = next;
            Write(key, next);
        };

        var grid = new Grid { Margin = new Thickness(0, 4, 0, 8) };
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(100) });
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
        combo.SelectionChanged += (_, _) => Write(key, Math.Max(0, combo.SelectedIndex).ToString(CultureInfo.InvariantCulture));
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
    }

    private int ReadIndex(string key, string fallback)
    {
        return int.TryParse(Read(key, fallback), out var value) ? value : 0;
    }

    private static StackPanel TabPanel()
    {
        return new StackPanel { Margin = new Thickness(0, 8, 0, 0) };
    }

    private static TabItem Tab(string header, FrameworkElement content)
    {
        return new TabItem { Header = header, Content = content };
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
