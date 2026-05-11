using System.Diagnostics;
using WpfInspectionApp.Interop;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed record PartRuntimeImage(
    byte[] SourcePixels,
    int Width,
    int Height,
    int SourceStride,
    int Threshold2D);

public sealed class PartRuntimePacket
{
    public string ModelName { get; set; } = "";
    public string PartName { get; set; } = "";
    public List<WindowRuntimePacket> Windows { get; set; } = [];
    public int AlgorithmCount => Windows.Sum(window => window.Algorithms.Count);
}

public sealed class WindowRuntimePacket
{
    public int Index { get; set; }
    public string Id { get; set; } = "";
    public string Name { get; set; } = "";
    public RoiRect Roi { get; set; }
    public List<AlgorithmRuntimePacket> Algorithms { get; set; } = [];
}

public sealed class AlgorithmRuntimePacket
{
    public int Index { get; set; }
    public string Id { get; set; } = "";
    public string Type { get; set; } = "";
    public string DisplayName { get; set; } = "";
    public string LegacyName { get; set; } = "";
    public LegacyAlgorithmGroup LegacyGroup { get; set; }
    public int LegacyFlag { get; set; }
    public RoiRect? AlgorithmRoi { get; set; }
    public RoiRect InspectionRoi { get; set; }
    public bool Enabled { get; set; }
    public bool Required { get; set; }
    public Dictionary<string, string> Parameters { get; set; } = [];
    internal InspectionAlgorithmData Source { get; set; } = null!;
}

public sealed class AlignRuntimeState
{
    public string AlgorithmId { get; set; } = "";
    public double OffsetX { get; set; }
    public double OffsetY { get; set; }
    public double Angle { get; set; }
    public double CenterX { get; set; }
    public double CenterY { get; set; }

    public string ToRuntimeText()
    {
        return $"Align X {OffsetX:0.###}, Y {OffsetY:0.###}, Angle {Angle:0.###}, Center {CenterX:0.#}/{CenterY:0.#}";
    }
}

public sealed class AlgorithmRunResult
{
    public int WindowIndex { get; set; }
    public string WindowName { get; set; } = "";
    public int AlgorithmIndex { get; set; }
    public string AlgorithmType { get; set; } = "";
    public string AlgorithmName { get; set; } = "";
    public bool Skipped { get; set; }
    public bool UsedImage { get; set; }
    public bool UsedAlign { get; set; }
    public string Message { get; set; } = "";
    public double ElapsedMs { get; set; }
    public int ForegroundPixels { get; set; }
    public int BlobCount { get; set; }
    public RoiRect? Bounds { get; set; }
    public RoiRect InspectionRoi { get; set; }
}

public sealed class WindowRunResult
{
    public int Index { get; set; }
    public string Name { get; set; } = "";
    public RoiRect Roi { get; set; }
    public List<AlgorithmRunResult> Algorithms { get; set; } = [];
}

public sealed class PartRunResult
{
    public PartRuntimePacket Packet { get; set; } = new();
    public ReferenceInspectionPacket ReferencePacket { get; set; } = new();
    public List<WindowRunResult> Windows { get; set; } = [];
    public double ElapsedMs { get; set; }
    public int ExecutedCount => Windows.Sum(window => window.Algorithms.Count(result => !result.Skipped));
    public int SkippedCount => Windows.Sum(window => window.Algorithms.Count(result => result.Skipped));
    public int TotalCount => Windows.Sum(window => window.Algorithms.Count);
}

public sealed class PartInspectionRuntime
{
    public PartRuntimePacket BuildPartParam(InspectionModel model)
    {
        model.EnsureStructure();

        var packet = new PartRuntimePacket
        {
            ModelName = model.ModelName,
            PartName = model.Part.Name
        };

        for (var windowIndex = 0; windowIndex < model.Part.Windows.Count; windowIndex++)
        {
            var window = model.Part.Windows[windowIndex];
            var windowPacket = new WindowRuntimePacket
            {
                Index = windowIndex + 1,
                Id = window.Id,
                Name = window.Name,
                Roi = window.Roi
            };

            for (var algorithmIndex = 0; algorithmIndex < window.Algorithms.Count; algorithmIndex++)
            {
                var algorithm = window.Algorithms[algorithmIndex];
                algorithm.ApplyCatalogDefaults();

                var inspectionRoi = algorithm.AlgorithmRoi ?? window.Roi;
                windowPacket.Algorithms.Add(new AlgorithmRuntimePacket
                {
                    Index = algorithmIndex + 1,
                    Id = algorithm.Id,
                    Type = algorithm.Type,
                    DisplayName = algorithm.DisplayName,
                    LegacyGroup = algorithm.LegacyGroup,
                    LegacyFlag = algorithm.LegacyFlag,
                    LegacyName = algorithm.LegacyName,
                    AlgorithmRoi = algorithm.AlgorithmRoi,
                    InspectionRoi = inspectionRoi,
                    Enabled = ReadBoolean(algorithm.Parameters, "Common.bAlgoEnable", true),
                    Required = ReadBoolean(algorithm.Parameters, "Common.IsRequired", false),
                    Parameters = new Dictionary<string, string>(algorithm.Parameters, StringComparer.OrdinalIgnoreCase),
                    Source = algorithm
                });
            }

            packet.Windows.Add(windowPacket);
        }

        return packet;
    }

    public PartRunResult Run(InspectionModel model, PartRuntimeImage? image)
    {
        var stopwatch = Stopwatch.StartNew();
        var packet = BuildPartParam(model);
        var referencePacket = ReferenceInspectionPacketBuilder.Build(packet);
        var referenceAlgorithms = referencePacket.Windows
            .SelectMany(window => window.Algorithms)
            .ToDictionary(algorithm => CreateReferenceKey(algorithm.WindowIndex, algorithm.AlgorithmIndex), StringComparer.OrdinalIgnoreCase);

        var run = new PartRunResult
        {
            Packet = packet,
            ReferencePacket = referencePacket
        };

        foreach (var window in packet.Windows)
        {
            var windowResult = new WindowRunResult
            {
                Index = window.Index,
                Name = window.Name,
                Roi = window.Roi
            };
            run.Windows.Add(windowResult);

            AlignRuntimeState? alignState = null;

            foreach (var algorithm in window.Algorithms)
            {
                var result = RunAlgorithm(model, window, algorithm, image, alignState);
                if (referenceAlgorithms.TryGetValue(CreateReferenceKey(window.Index, algorithm.Index), out var referenceAlgorithm))
                {
                    ApplyReferenceRuntimeData(algorithm, referenceAlgorithm);
                    result.Message = $"{result.Message} | Reference {referenceAlgorithm.ToSummary()}";
                }

                windowResult.Algorithms.Add(result);

                algorithm.Source.Result = new InspectionResultData
                {
                    Message = result.Message,
                    ElapsedMs = result.ElapsedMs,
                    ForegroundPixels = result.ForegroundPixels,
                    BlobCount = result.BlobCount,
                    Bounds = result.Bounds
                };
                algorithm.Source.Parameters["Runtime.LastRun"] = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss");
                algorithm.Source.Parameters["Runtime.WindowIndex"] = window.Index.ToString();
                algorithm.Source.Parameters["Runtime.AlgorithmIndex"] = algorithm.Index.ToString();
                algorithm.Source.Parameters["Runtime.InspectionRoi"] = FormatRoi(algorithm.InspectionRoi);
                algorithm.Source.Parameters["Runtime.UsedAlign"] = result.UsedAlign.ToString();

                if (IsAlignFamily(algorithm.Type))
                {
                    alignState = CreateAlignState(model, window, algorithm);
                    algorithm.Source.Parameters["Runtime.AlignResult"] = alignState.ToRuntimeText();
                }
                else if (alignState != null)
                {
                    algorithm.Source.Parameters["Runtime.AlignResult"] = alignState.ToRuntimeText();
                }
            }
        }

        stopwatch.Stop();
        run.ElapsedMs = stopwatch.Elapsed.TotalMilliseconds;
        return run;
    }

    private static void ApplyReferenceRuntimeData(AlgorithmRuntimePacket algorithm, ReferenceAlgorithmPacket referenceAlgorithm)
    {
        algorithm.Source.Parameters["Reference.InspAlgoType"] = referenceAlgorithm.ReferenceName;
        algorithm.Source.Parameters["Reference.InspAlgoTypeOrdinal"] = referenceAlgorithm.InspAlgoTypeOrdinal.ToString();
        algorithm.Source.Parameters["Reference.ResultBucket"] = referenceAlgorithm.ResultBucket;
        algorithm.Source.Parameters["Reference.WindowIndex"] = referenceAlgorithm.WindowIndex.ToString();
        algorithm.Source.Parameters["Reference.AlgorithmIndex"] = referenceAlgorithm.AlgorithmIndex.ToString();
        algorithm.Source.Parameters["Reference.RawFieldCount"] = referenceAlgorithm.RawFields.Count.ToString();
        algorithm.Source.Parameters["Reference.NumericArrayCount"] = referenceAlgorithm.NumericArrays.Count.ToString();
        algorithm.Source.Parameters["Reference.Summary"] = referenceAlgorithm.ToSummary();
    }

    private static string CreateReferenceKey(int windowIndex, int algorithmIndex)
    {
        return $"{windowIndex}:{algorithmIndex}";
    }

    private static AlgorithmRunResult RunAlgorithm(
        InspectionModel model,
        WindowRuntimePacket window,
        AlgorithmRuntimePacket algorithm,
        PartRuntimeImage? image,
        AlignRuntimeState? alignState)
    {
        var stopwatch = Stopwatch.StartNew();
        var result = new AlgorithmRunResult
        {
            WindowIndex = window.Index,
            WindowName = window.Name,
            AlgorithmIndex = algorithm.Index,
            AlgorithmType = algorithm.Type,
            AlgorithmName = algorithm.DisplayName,
            InspectionRoi = algorithm.InspectionRoi,
            UsedAlign = alignState != null && !IsAlignFamily(algorithm.Type)
        };

        if (!algorithm.Enabled)
        {
            result.Skipped = true;
            result.Message = "Skipped: Common.bAlgoEnable is false";
            return result;
        }

        if (!algorithm.InspectionRoi.IsValid)
        {
            result.Skipped = algorithm.Required;
            result.Message = algorithm.Required
                ? "NG: Required algorithm has no valid ROI"
                : "Skipped: no valid ROI";
            return result;
        }

        if (image != null && image.Width > 0 && image.Height > 0)
        {
            var destinationStride = image.Width;
            var destination = new byte[destinationStride * image.Height];
            var threshold = ReadAlgorithmThreshold(model, algorithm);
            var response = NativeInspectionBridge.ThresholdBgra(
                image.SourcePixels,
                image.Width,
                image.Height,
                image.SourceStride,
                destination,
                destinationStride,
                threshold,
                algorithm.InspectionRoi);

            var native = response.Result;
            result.UsedImage = true;
            result.ElapsedMs = native.ElapsedMs;
            result.ForegroundPixels = native.ForegroundPixels;
            result.BlobCount = native.BlobCount;
            result.Bounds = native.ForegroundPixels > 0
                ? new RoiRect(native.MinX, native.MinY, Math.Max(1, native.MaxX - native.MinX + 1), Math.Max(1, native.MaxY - native.MinY + 1))
                : null;
        }
        else
        {
            result.ForegroundPixels = EstimateForeground(algorithm.InspectionRoi, algorithm);
            result.BlobCount = result.ForegroundPixels > 0 ? 1 : 0;
            result.Bounds = algorithm.InspectionRoi;
        }

        stopwatch.Stop();
        if (result.ElapsedMs <= 0)
        {
            result.ElapsedMs = stopwatch.Elapsed.TotalMilliseconds;
        }

        result.Message = CreateResultMessage(algorithm, result, alignState);
        return result;
    }

    private static string CreateResultMessage(AlgorithmRuntimePacket algorithm, AlgorithmRunResult result, AlignRuntimeState? alignState)
    {
        var roiKind = algorithm.AlgorithmRoi.HasValue ? "Algorithm ROI" : "Window ROI";
        if (IsAlignFamily(algorithm.Type))
        {
            return $"{algorithm.DisplayName} OK | {roiKind} | Align reference updated";
        }

        var alignText = alignState == null ? "without Align reference" : $"using {alignState.ToRuntimeText()}";
        return $"{algorithm.DisplayName} OK | {roiKind} | {alignText}";
    }

    private static AlignRuntimeState CreateAlignState(InspectionModel model, WindowRuntimePacket window, AlgorithmRuntimePacket algorithm)
    {
        var roi = algorithm.InspectionRoi;
        var windowCenterX = window.Roi.X + window.Roi.Width / 2.0;
        var windowCenterY = window.Roi.Y + window.Roi.Height / 2.0;
        var roiCenterX = roi.X + roi.Width / 2.0;
        var roiCenterY = roi.Y + roi.Height / 2.0;

        return new AlignRuntimeState
        {
            AlgorithmId = algorithm.Id,
            OffsetX = model.AlignShiftEnabled ? roiCenterX - windowCenterX : 0,
            OffsetY = model.AlignShiftEnabled ? roiCenterY - windowCenterY : 0,
            Angle = model.AlignAngleEnabled ? model.AlignAngle : 0,
            CenterX = roiCenterX,
            CenterY = roiCenterY
        };
    }

    private static int ReadAlgorithmThreshold(InspectionModel model, AlgorithmRuntimePacket algorithm)
    {
        var family = AlgorithmCatalog.Find(algorithm.Type).ParameterFamily;
        if (algorithm.Parameters.TryGetValue($"{family}.Threshold", out var value) && int.TryParse(value, out var threshold))
        {
            return Net48Compat.Clamp(threshold, 0, 255);
        }

        return Net48Compat.Clamp(model.Threshold2D, 0, 255);
    }

    private static int EstimateForeground(RoiRect roi, AlgorithmRuntimePacket algorithm)
    {
        var area = Math.Max(1, roi.Width * roi.Height);
        var divisor = IsAlignFamily(algorithm.Type) ? 8 : 12;
        return Math.Max(1, area / divisor);
    }

    private static bool IsAlignFamily(string algorithmType)
    {
        return string.Equals(algorithmType, "AlgoAlign", StringComparison.OrdinalIgnoreCase)
            || string.Equals(algorithmType, "AlgoAlignEdge", StringComparison.OrdinalIgnoreCase)
            || string.Equals(algorithmType, "AlgoPadAlign", StringComparison.OrdinalIgnoreCase);
    }

    private static bool ReadBoolean(Dictionary<string, string> parameters, string key, bool fallback)
    {
        if (!parameters.TryGetValue(key, out var value) || string.IsNullOrWhiteSpace(value))
        {
            return fallback;
        }

        if (bool.TryParse(value, out var boolean))
        {
            return boolean;
        }

        if (int.TryParse(value, out var number))
        {
            return number != 0;
        }

        return fallback;
    }

    private static string FormatRoi(RoiRect roi)
    {
        var centerX = roi.X + roi.Width / 2;
        var centerY = roi.Y + roi.Height / 2;
        return $"X {roi.X} Y {roi.Y} W {roi.Width} H {roi.Height} | CX {centerX} CY {centerY}";
    }
}
