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

            DrawAlignSearchOverlay(overlay, context, window);
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

    private static void DrawAlignSearchOverlay(Canvas overlay, RoiOverlayRenderContext context, InspectionWindowData window)
    {
        // Only draw Align search visualisation when an AlgoAlign algorithm exists in this window
        var alignAlgo = window.Algorithms.FirstOrDefault(a =>
            string.Equals(a.Type, "AlgoAlign", StringComparison.OrdinalIgnoreCase));
        if (alignAlgo == null)
        {
            return;
        }

        var rois = context.Model.AlignSearchRois;
        if (rois == null)
        {
            return;
        }

        for (var i = 0; i < rois.Length; i++)
        {
            var roi = rois[i];
            if (!roi.HasValue || !roi.Value.IsValid)
            {
                continue;
            }

            var rect = context.ToDisplayRect(roi.Value);
            // Dashed cyan rectangle for the search area
            var searchRect = new Rectangle
            {
                Width = rect.Width,
                Height = rect.Height,
                Stroke = new SolidColorBrush(Color.FromRgb(0, 200, 230)),
                StrokeDashArray = new DoubleCollection { 3, 3 },
                StrokeThickness = 1.2,
                Fill = new SolidColorBrush(Color.FromArgb(20, 0, 200, 230))
            };
            Canvas.SetLeft(searchRect, rect.Left);
            Canvas.SetTop(searchRect, rect.Top);
            overlay.Children.Add(searchRect);

            // Expected center marker (small circle)
            var cx = roi.Value.X + roi.Value.Width / 2;
            var cy = roi.Value.Y + roi.Value.Height / 2;
            DrawMarker(overlay, context, cx, cy, Color.FromRgb(0, 200, 230), $"S{i + 1}");

            // Detected center if available
            if (alignAlgo.AlignDetectedCentersX != null && i < alignAlgo.AlignDetectedCentersX.Length &&
                alignAlgo.AlignDetectedCentersY != null && i < alignAlgo.AlignDetectedCentersY.Length)
            {
                var dx = alignAlgo.AlignDetectedCentersX[i];
                var dy = alignAlgo.AlignDetectedCentersY[i];
                // If detected differs from expected, draw a yellow crosshair + offset arrow
                if (dx != cx || dy != cy)
                {
                    DrawMarker(overlay, context, dx, dy, Color.FromRgb(255, 220, 60), null);
                    DrawArrow(overlay, context, cx, cy, dx, dy);
                }
            }
        }

        // Summary label near the window's top-left if offset/theta available
        if (alignAlgo.AlignOffsetX.HasValue || alignAlgo.AlignTheta.HasValue)
        {
            var winRect = context.ToDisplayRect(window.Roi);
            var summary = new TextBlock
            {
                Text = $"Align ΔX={alignAlgo.AlignOffsetX:F2} ΔY={alignAlgo.AlignOffsetY:F2} θ={alignAlgo.AlignTheta:F2}°",
                Foreground = Brushes.Black,
                Background = new SolidColorBrush(Color.FromRgb(255, 220, 60)),
                FontFamily = new FontFamily("Consolas"),
                FontWeight = FontWeights.Bold,
                Padding = new Thickness(4, 1, 4, 1)
            };
            Canvas.SetLeft(summary, winRect.Left);
            Canvas.SetTop(summary, winRect.Bottom + 2);
            overlay.Children.Add(summary);
        }
    }

    private static void DrawMarker(Canvas overlay, RoiOverlayRenderContext context, int sourceX, int sourceY, Color color, string? label)
    {
        // Convert source coords to display coords by mapping a 1x1 ROI
        var pixelRect = context.ToDisplayRect(new RoiRect(sourceX, sourceY, 1, 1));
        var cx = pixelRect.Left;
        var cy = pixelRect.Top;
        const double radius = 5;

        var brush = new SolidColorBrush(color);
        var hLine = new Line { X1 = cx - radius, X2 = cx + radius, Y1 = cy, Y2 = cy, Stroke = brush, StrokeThickness = 1.5 };
        var vLine = new Line { X1 = cx, X2 = cx, Y1 = cy - radius, Y2 = cy + radius, Stroke = brush, StrokeThickness = 1.5 };
        var circle = new Ellipse { Width = radius * 2, Height = radius * 2, Stroke = brush, StrokeThickness = 1.2 };
        Canvas.SetLeft(circle, cx - radius);
        Canvas.SetTop(circle, cy - radius);
        overlay.Children.Add(hLine);
        overlay.Children.Add(vLine);
        overlay.Children.Add(circle);

        if (!string.IsNullOrEmpty(label))
        {
            var tb = new TextBlock
            {
                Text = label,
                Foreground = brush,
                FontFamily = new FontFamily("Consolas"),
                FontWeight = FontWeights.Bold,
                FontSize = 10
            };
            Canvas.SetLeft(tb, cx + radius + 2);
            Canvas.SetTop(tb, cy - 8);
            overlay.Children.Add(tb);
        }
    }

    private static void DrawArrow(Canvas overlay, RoiOverlayRenderContext context, int fromX, int fromY, int toX, int toY)
    {
        var fromRect = context.ToDisplayRect(new RoiRect(fromX, fromY, 1, 1));
        var toRect = context.ToDisplayRect(new RoiRect(toX, toY, 1, 1));
        var brush = new SolidColorBrush(Color.FromRgb(255, 220, 60));
        var line = new Line
        {
            X1 = fromRect.Left, Y1 = fromRect.Top,
            X2 = toRect.Left, Y2 = toRect.Top,
            Stroke = brush,
            StrokeThickness = 1.2,
            StrokeDashArray = new DoubleCollection { 2, 2 }
        };
        overlay.Children.Add(line);
    }

    private static void DrawSingleRoi(
        Canvas overlay,
        RoiOverlayRenderContext context,
        RoiRect roi,
        string labelText,
        bool isActive,
        bool isWindowRoi)
    {
        var roiRect = context.ToDisplayRect(roi);

        // Window ROI 는 옐로우 통일, Algorithm ROI 는 기존 시안 톤 유지.
        // active/inactive 차이는 stroke 두께로만 표현 (색은 동일) — Window 의 경우
        // 어두운 음영 inactive 가 잘 안 보인다는 의견 반영.
        var activeColor = isWindowRoi ? Color.FromRgb(255, 210, 41) : Color.FromRgb(128, 223, 255);
        var inactiveColor = isWindowRoi ? Color.FromRgb(255, 210, 41) : Color.FromRgb(54, 137, 168);

        // Window ROI margin (녹색 점선 박스) 와 점선 stroke 는 모두 제거.
        // Algorithm ROI 만 기존처럼 짧은 점선으로 구분.
        var rectangle = new Rectangle
        {
            Width = roiRect.Width,
            Height = roiRect.Height,
            Stroke = new SolidColorBrush(isActive ? activeColor : inactiveColor),
            StrokeDashArray = isWindowRoi ? null : new DoubleCollection { 2, 2 },
            StrokeThickness = isActive ? 2.2 : 1.4,
            // Window ROI 내부는 완전 투명 (Fill=null). Algorithm ROI 만 살짝 색조 유지.
            Fill = isWindowRoi
                ? null
                : new SolidColorBrush(isActive
                    ? Color.FromArgb(42, 128, 223, 255)
                    : Color.FromArgb(20, 128, 223, 255))
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
