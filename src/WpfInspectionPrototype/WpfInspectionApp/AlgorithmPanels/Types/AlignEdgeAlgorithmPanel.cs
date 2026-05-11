using System.Globalization;
using System.Text.Json;
using System.Windows;
using System.Windows.Controls;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.AlgorithmPanels.Types;

public sealed class AlignEdgeAlgorithmPanel : DynamicAlgorithmPanel
{
    public AlignEdgeAlgorithmPanel()
        : base("AlgoAlignEdge")
    {
    }

    protected override void ApplyDefaults(AlgorithmPanelContext context)
    {
        WriteDefault(context, "AlignEdge.Direction", "0");
        WriteDefault(context, "AlignEdge.Threshold", "128");
        WriteDefault(context, "AlignEdge.SearchWidth", "40");
        WriteDefault(context, "AlignEdge.SearchHeight", "12");
        WriteDefault(context, "AlignEdge.UsePeak", "true");
        WriteDefault(context, "AlignEdge.UseSubPixel", "true");
        WriteDefault(context, "AlignEdge.Use2D", "true");
        WriteDefault(context, "AlignEdge.Use3D", "false");
        WriteDefault(context, "AlignEdge.UseDistance", "true");
        WriteDefault(context, "AlignEdge.UseAngle", "true");
        WriteDefault(context, "AlignEdge.UseShift", "true");
    }

    protected override bool OnAlgorithmSpecificCommand(string key)
    {
        if (!key.Equals("AlignEdge.CaptureAnchorRequested", StringComparison.OrdinalIgnoreCase))
        {
            return false;
        }

        CaptureAnchor();
        return true;
    }

    protected override void AddCustomTabs(TabControl tabs)
    {
        tabs.Items.Add(AlgorithmPanelUi.Tab("Serialize", BuildSerializeTab()));
    }

    private void CaptureAnchor()
    {
        if (Context == null)
        {
            return;
        }

        var roi = Context.Algorithm.AlgorithmRoi ?? Context.Window.Roi;
        Interaction.Write("AlignEdge.AnchorX", (roi.X + roi.Width / 2).ToString(CultureInfo.InvariantCulture));
        Interaction.Write("AlignEdge.AnchorY", (roi.Y + roi.Height / 2).ToString(CultureInfo.InvariantCulture));
        RequestRebuild();
    }

    private FrameworkElement BuildSerializeTab()
    {
        var panel = AlgorithmPanelUi.TabPanel();
        var json = JsonSerializer.Serialize(Context?.Algorithm.Parameters ?? [], new JsonSerializerOptions { WriteIndented = true });
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

    private static void WriteDefault(AlgorithmPanelContext context, string key, string value)
    {
        if (!context.Algorithm.Parameters.ContainsKey(key))
        {
            context.Algorithm.Parameters[key] = value;
        }
    }
}
