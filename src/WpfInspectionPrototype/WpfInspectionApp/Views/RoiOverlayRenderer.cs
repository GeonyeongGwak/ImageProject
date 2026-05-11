using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shapes;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Views;

public sealed class RoiOverlayRenderContext
{
    public InspectionModel Model { get; set; } = new();
    public Rect Display { get; set; }
    public Func<RoiRect, Rect> ToDisplayRect { get; set; } = _ => Rect.Empty;
    public Func<RoiRect, RoiRect> CreateMarginRoi { get; set; } = roi => roi;
    public RoiRect? PreviewRoi { get; set; }
    public bool PreviewIsAlgorithmRoi { get; set; }
    public string? ActiveAlgorithmId { get; set; }
}

public static class RoiOverlayRenderer
{
    public static void Render(Canvas overlay, RoiOverlayRenderContext context)
    {
        overlay.Children.Clear();

        context.Model.EnsureStructure();
        for (var index = 0; index < context.Model.Part.Windows.Count; index++)
        {
            var window = context.Model.Part.Windows[index];
            var roi = window.Roi;
            if (!roi.IsValid)
            {
                continue;
            }

            DrawSingleRoi(
                overlay,
                context,
                roi,
                $"Window ROI {index + 1}",
                window.Id == context.Model.SelectedWindowId,
                isWindowRoi: true);

            foreach (var algorithm in window.Algorithms.Where(item => item.AlgorithmRoi.HasValue))
            {
                DrawSingleRoi(
                    overlay,
                    context,
                    algorithm.AlgorithmRoi!.Value,
                    $"{algorithm.Type} ROI",
                    context.ActiveAlgorithmId == algorithm.Id,
                    isWindowRoi: false);
            }
        }

        if (context.PreviewRoi.HasValue && context.PreviewRoi.Value.IsValid)
        {
            var label = context.PreviewIsAlgorithmRoi
                ? "Algorithm ROI preview"
                : $"Window ROI {context.Model.Part.Windows.Count + 1}";
            DrawSingleRoi(
                overlay,
                context,
                context.PreviewRoi.Value,
                label,
                isActive: true,
                isWindowRoi: !context.PreviewIsAlgorithmRoi);
        }
    }

    private static void DrawSingleRoi(
        Canvas overlay,
        RoiOverlayRenderContext context,
        RoiRect roi,
        string labelText,
        bool isActive,
        bool isWindowRoi)
    {
        var marginRoi = context.CreateMarginRoi(roi);
        var marginRect = context.ToDisplayRect(marginRoi);
        var roiRect = context.ToDisplayRect(roi);
        var activeColor = isWindowRoi ? Color.FromRgb(255, 210, 41) : Color.FromRgb(128, 223, 255);
        var inactiveColor = isWindowRoi ? Color.FromRgb(160, 129, 28) : Color.FromRgb(54, 137, 168);

        if (isWindowRoi)
        {
            var margin = new Rectangle
            {
                Width = marginRect.Width,
                Height = marginRect.Height,
                Stroke = new SolidColorBrush(Color.FromRgb(24, 224, 123)),
                StrokeDashArray = new DoubleCollection { 5, 4 },
                StrokeThickness = isActive ? 2 : 1,
                Fill = new SolidColorBrush(Color.FromArgb(34, 24, 224, 123))
            };
            Canvas.SetLeft(margin, marginRect.Left);
            Canvas.SetTop(margin, marginRect.Top);
            overlay.Children.Add(margin);
        }

        var rectangle = new Rectangle
        {
            Width = roiRect.Width,
            Height = roiRect.Height,
            Stroke = new SolidColorBrush(isActive ? activeColor : inactiveColor),
            StrokeDashArray = isWindowRoi ? null : new DoubleCollection { 2, 2 },
            StrokeThickness = isActive ? 2.2 : 1.4,
            Fill = new SolidColorBrush(isWindowRoi
                ? (isActive ? Color.FromArgb(34, 255, 210, 41) : Color.FromArgb(18, 255, 210, 41))
                : (isActive ? Color.FromArgb(42, 128, 223, 255) : Color.FromArgb(20, 128, 223, 255)))
        };
        Canvas.SetLeft(rectangle, roiRect.Left);
        Canvas.SetTop(rectangle, roiRect.Top);
        overlay.Children.Add(rectangle);

        var label = new TextBlock
        {
            Text = $"{labelText} {roi.Width}x{roi.Height}px",
            Foreground = Brushes.Black,
            Background = new SolidColorBrush(isActive ? activeColor : inactiveColor),
            FontFamily = new FontFamily("Consolas"),
            FontWeight = FontWeights.Bold,
            Padding = new Thickness(4, 1, 4, 1)
        };
        Canvas.SetLeft(label, roiRect.Left);
        Canvas.SetTop(label, Math.Max(0, roiRect.Top - 21));
        overlay.Children.Add(label);
    }
}
