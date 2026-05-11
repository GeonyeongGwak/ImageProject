using System.Text;
using WpfInspectionApp.Interop;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class InspectionResultTextService : IInspectionResultTextService
{
    public string FormatPartRun(PartRunResult run)
    {
        var builder = new StringBuilder();
        builder.AppendLine($"Part: {run.Packet.PartName}");
        builder.AppendLine($"Model: {run.Packet.ModelName}");
        builder.AppendLine($"Structure: {run.Packet.Windows.Count} Window(s), {run.Packet.AlgorithmCount} Algorithm(s)");
        builder.AppendLine("Runtime: Part -> Window -> Algorithm -> Inspection Result");
        builder.AppendLine($"Elapsed: {run.ElapsedMs:F3} ms / Executed {run.ExecutedCount} / Skipped {run.SkippedCount}");

        foreach (var window in run.Windows)
        {
            builder.AppendLine();
            builder.AppendLine($"[Window ROI {window.Index}] {window.Name} | {FormatRoiStatic(window.Roi)}");

            foreach (var algorithm in window.Algorithms)
            {
                var status = algorithm.Skipped ? "SKIP" : "OK";
                var align = algorithm.UsedAlign ? " | Align-linked" : "";
                builder.AppendLine($"  [{status}] Algo {algorithm.AlgorithmIndex}: {algorithm.AlgorithmType} / {algorithm.AlgorithmName}{align}");
                builder.AppendLine($"       ROI: {FormatRoiStatic(algorithm.InspectionRoi)}");
                builder.AppendLine($"       Result: {algorithm.Message} | FG {algorithm.ForegroundPixels:N0} | Blob {algorithm.BlobCount} | {algorithm.ElapsedMs:F3} ms");
            }
        }

        return builder.ToString();
    }

    public string FormatThresholdResult(
        InspectionModel model,
        NativeThresholdResponse response,
        string nativeLabel,
        InspectionWindowData? activeWindow,
        InspectionAlgorithmData? activeAlgorithm,
        RoiRect? activeInspectionRoi,
        Func<RoiRect?, string> formatRoi)
    {
        var result = response.Result;
        return
            $"Part: {model.Part.Name}{Environment.NewLine}" +
            $"Window: {activeWindow?.Name ?? "none"}{Environment.NewLine}" +
            $"Algorithm: {activeAlgorithm?.Type ?? "none"}{Environment.NewLine}" +
            $"Runtime: {nativeLabel}{Environment.NewLine}" +
            $"Message: {result.Message}{Environment.NewLine}" +
            $"Foreground: {result.ForegroundPixels:N0}{Environment.NewLine}" +
            $"Blob placeholder: {result.BlobCount}{Environment.NewLine}" +
            $"Bounds: X {result.MinX} Y {result.MinY} - X {result.MaxX} Y {result.MaxY}{Environment.NewLine}" +
            $"Window ROI: {formatRoi(activeWindow?.Roi)}{Environment.NewLine}" +
            $"Algorithm ROI: {formatRoi(activeAlgorithm?.AlgorithmRoi)}{Environment.NewLine}" +
            $"Inspection ROI: {formatRoi(activeInspectionRoi)}{Environment.NewLine}" +
            $"Structure: {model.Part.Windows.Count} Window(s), {model.Part.Windows.Sum(window => window.Algorithms.Count)} Algorithm(s){Environment.NewLine}" +
            $"Align: Shift({model.AlignShiftEnabled}) X {model.AlignShiftX:0.00} Y {model.AlignShiftY:0.00}, Angle({model.AlignAngleEnabled}) {model.AlignAngle:0.##}, Filter {model.AlignFilter}, IPC {model.IpcClass} {model.IpcPercent:0.##}%";
    }

    private static string FormatRoiStatic(RoiRect roi)
    {
        var centerX = roi.X + roi.Width / 2;
        var centerY = roi.Y + roi.Height / 2;
        return $"X {roi.X} Y {roi.Y} W {roi.Width} H {roi.Height} | CX {centerX} CY {centerY}";
    }
}
