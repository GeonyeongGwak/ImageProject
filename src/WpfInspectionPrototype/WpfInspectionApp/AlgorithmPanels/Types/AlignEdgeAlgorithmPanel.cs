using System.Globalization;
using System.Text.Json;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using WpfInspectionApp.AlgorithmPanels;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.AlgorithmPanels.Types;

public sealed class AlignEdgeAlgorithmPanel : IAlgorithmPanel
{
    private readonly AlgorithmCatalogItem _catalog;
    private readonly Border _root;
    private readonly StackPanel _content;
    private readonly AlgorithmPanelInteraction _interaction;
    private AlgorithmPanelContext? _context;
    private bool _binding;

    public AlignEdgeAlgorithmPanel()
    {
        _catalog = AlgorithmCatalog.Find("AlgoAlignEdge");
        _content = new StackPanel();
        _root = AlgorithmPanelUi.RootBorder(_content);
        _interaction = new AlgorithmPanelInteraction(
            _catalog,
            () => _binding,
            () => _context,
            Rebuild,
            _ => false,
            (_, _) => { });
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
        panel.Children.Add(Combo("Direction", "AlignEdge.Direction", ["LeftToRight", "RightToLeft", "TopToBottom", "BottomToTop"], _interaction.ReadIndex("AlignEdge.Direction", "0")));
        panel.Children.Add(Slider("Threshold", "AlignEdge.Threshold", 0, 255, _interaction.Read("AlignEdge.Threshold", "128")));
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
        panel.Children.Add(Combo("Anchor Mode", "AlignEdge.AnchorMode", ["Window ROI", "Algorithm ROI", "Last Align"], _interaction.ReadIndex("AlignEdge.AnchorMode", "0")));
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
            Foreground = AlgorithmPanelUi.Brush("#E4F3FF"),
            Background = AlgorithmPanelUi.Brush("#07101C"),
            BorderBrush = AlgorithmPanelUi.Brush("#284A72")
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
        _interaction.Write("AlignEdge.TeachRoi", FormatRoi(roi));
        _interaction.ExecuteCommand("AlignEdge.TeachRequested", rebuild: true);
    }

    private void CaptureAnchor()
    {
        if (_context == null)
        {
            return;
        }

        var roi = _context.Algorithm.AlgorithmRoi ?? _context.Window.Roi;
        _interaction.Write("AlignEdge.AnchorX", (roi.X + roi.Width / 2).ToString(CultureInfo.InvariantCulture));
        _interaction.Write("AlignEdge.AnchorY", (roi.Y + roi.Height / 2).ToString(CultureInfo.InvariantCulture));
        Rebuild();
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
        return AlgorithmPanelUi.Slider(label, _interaction.Read(key, fallback), min, max, fallback, value => _interaction.Write(key, value), labelWidth: 100);
    }

    private FrameworkElement Combo(string label, string key, string[] values, int selectedIndex)
    {
        return AlgorithmPanelUi.Combo(label, values, selectedIndex, value => _interaction.Write(key, value.ToString(CultureInfo.InvariantCulture)));
    }

    private FrameworkElement Row(string label, FrameworkElement editor)
    {
        return AlgorithmPanelUi.Row(label, editor);
    }

    private Button Button(string content, Action action)
    {
        return AlgorithmPanelUi.Button(content, action);
    }

    private static StackPanel TabPanel()
    {
        return AlgorithmPanelUi.TabPanel();
    }

    private static TabItem Tab(string header, FrameworkElement content)
    {
        return AlgorithmPanelUi.Tab(header, content);
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
