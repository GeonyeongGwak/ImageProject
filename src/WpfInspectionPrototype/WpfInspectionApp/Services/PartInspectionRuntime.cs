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

    public bool NativeAlignAvailable { get; set; }
    public int AlignOkCount { get; set; }
    public double AlignOffsetX { get; set; }
    public double AlignOffsetY { get; set; }
    public double AlignTheta { get; set; }
    public bool AlignOkShiftX { get; set; }
    public bool AlignOkShiftY { get; set; }
    public bool AlignOkAngle { get; set; }
    public string NativeAlignMessage { get; set; } = "";
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
                    Enabled = window.IsEnabled && ReadBoolean(algorithm.Parameters, "Common.bAlgoEnable", true),
                    Required = ReadBoolean(algorithm.Parameters, "Common.IsRequired", false),
                    Parameters = AlgorithmParameterStore.CloneCaseInsensitive(algorithm.Parameters),
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
        var referenceAlgorithms = BuildReferenceAlgorithmMap(referencePacket);

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
                SetSourceParameter(algorithm, "Runtime.LastRun", DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"));
                SetSourceParameter(algorithm, "Runtime.WindowIndex", window.Index.ToString());
                SetSourceParameter(algorithm, "Runtime.AlgorithmIndex", algorithm.Index.ToString());
                SetSourceParameter(algorithm, "Runtime.InspectionRoi", FormatRoi(algorithm.InspectionRoi));
                SetSourceParameter(algorithm, "Runtime.UsedAlign", result.UsedAlign.ToString());

                if (IsAlignFamily(algorithm.Type))
                {
                    alignState = CreateAlignState(model, window, algorithm, result);
                    SetSourceParameter(algorithm, "Runtime.AlignResult", alignState.ToRuntimeText());
                    if (result.NativeAlignAvailable)
                    {
                        SetSourceParameter(algorithm, "Runtime.AlignBridge", "native");
                        SetSourceParameter(algorithm, "Runtime.AlignOkCount", result.AlignOkCount.ToString());
                        SetSourceParameter(algorithm, "Runtime.AlignOkShiftX", result.AlignOkShiftX.ToString());
                        SetSourceParameter(algorithm, "Runtime.AlignOkShiftY", result.AlignOkShiftY.ToString());
                        SetSourceParameter(algorithm, "Runtime.AlignOkAngle", result.AlignOkAngle.ToString());
                    }
                    else
                    {
                        SetSourceParameter(algorithm, "Runtime.AlignBridge", "fallback");
                        if (!string.IsNullOrWhiteSpace(result.NativeAlignMessage))
                        {
                            SetSourceParameter(algorithm, "Runtime.AlignBridgeMessage", result.NativeAlignMessage);
                        }
                    }
                }
                else if (alignState != null)
                {
                    SetSourceParameter(algorithm, "Runtime.AlignResult", alignState.ToRuntimeText());
                }
            }
        }

        stopwatch.Stop();
        run.ElapsedMs = stopwatch.Elapsed.TotalMilliseconds;
        return run;
    }

    private static void ApplyReferenceRuntimeData(AlgorithmRuntimePacket algorithm, ReferenceAlgorithmPacket referenceAlgorithm)
    {
        SetSourceParameter(algorithm, "Reference.InspAlgoType", referenceAlgorithm.ReferenceName);
        SetSourceParameter(algorithm, "Reference.InspAlgoTypeOrdinal", referenceAlgorithm.InspAlgoTypeOrdinal.ToString());
        SetSourceParameter(algorithm, "Reference.ResultBucket", referenceAlgorithm.ResultBucket);
        SetSourceParameter(algorithm, "Reference.WindowIndex", referenceAlgorithm.WindowIndex.ToString());
        SetSourceParameter(algorithm, "Reference.AlgorithmIndex", referenceAlgorithm.AlgorithmIndex.ToString());
        SetSourceParameter(algorithm, "Reference.RawFieldCount", referenceAlgorithm.RawFields.Count.ToString());
        SetSourceParameter(algorithm, "Reference.NumericArrayCount", referenceAlgorithm.NumericArrays.Count.ToString());
        SetSourceParameter(algorithm, "Reference.Summary", referenceAlgorithm.ToSummary());
    }

    private static Dictionary<string, ReferenceAlgorithmPacket> BuildReferenceAlgorithmMap(ReferenceInspectionPacket referencePacket)
    {
        var map = new Dictionary<string, ReferenceAlgorithmPacket>(StringComparer.OrdinalIgnoreCase);
        foreach (var algorithm in referencePacket.Windows.SelectMany(window => window.Algorithms))
        {
            var key = CreateReferenceKey(algorithm.WindowIndex, algorithm.AlgorithmIndex);
            if (!map.ContainsKey(key))
            {
                map[key] = algorithm;
            }
        }

        return map;
    }

    private static void SetSourceParameter(AlgorithmRuntimePacket algorithm, string key, string value)
    {
        AlgorithmParameterStore.Set(algorithm.Source.Parameters, key, value);
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
            if (string.Equals(algorithm.Type, "AlgoAlign", StringComparison.OrdinalIgnoreCase))
            {
                RunAlignViaBridge(image, model, window, algorithm, result);
            }
            else if (string.Equals(algorithm.Type, "AlgoShapeX", StringComparison.OrdinalIgnoreCase))
            {
                RunShapeXViaBridge(image, model, algorithm, result);
            }
            else if (string.Equals(algorithm.Type, "AlgoPadBW", StringComparison.OrdinalIgnoreCase))
            {
                RunPadBWViaBridge(image, model, algorithm, result);
            }
            else if (TryMapToGenericKind(algorithm.Type, out var genericKind))
            {
                RunGenericViaBridge(image, model, algorithm, result, genericKind);
            }
            else
            {
                RunThresholdFallback(image, model, algorithm, result);
            }
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

    private static AlignRuntimeState CreateAlignState(InspectionModel model, WindowRuntimePacket window, AlgorithmRuntimePacket algorithm, AlgorithmRunResult? lastResult)
    {
        var roi = algorithm.InspectionRoi;
        var windowCenterX = window.Roi.X + window.Roi.Width / 2.0;
        var windowCenterY = window.Roi.Y + window.Roi.Height / 2.0;
        var roiCenterX = roi.X + roi.Width / 2.0;
        var roiCenterY = roi.Y + roi.Height / 2.0;

        if (lastResult is { NativeAlignAvailable: true })
        {
            return new AlignRuntimeState
            {
                AlgorithmId = algorithm.Id,
                OffsetX = lastResult.AlignOffsetX,
                OffsetY = lastResult.AlignOffsetY,
                Angle = lastResult.AlignTheta,
                CenterX = roiCenterX,
                CenterY = roiCenterY
            };
        }

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

    private static void RunAlignViaBridge(PartRuntimeImage image, InspectionModel model, WindowRuntimePacket window, AlgorithmRuntimePacket algorithm, AlgorithmRunResult result)
    {
        var alignParams = AlignBridgeAdapter.BuildParams(model, window.Roi, algorithm);
        var response = MptiAlignNativeBridge.Run(
            image.SourcePixels, image.Width, image.Height, image.SourceStride,
            window.Roi, alignParams);

        result.UsedImage = true;
        result.NativeAlignAvailable = response.Available;
        if (response.Available && response.Success)
        {
            result.ElapsedMs = response.ElapsedMs;
            result.ForegroundPixels = response.ForegroundPixels;
            result.BlobCount = response.BlobCount;
            result.Bounds = ComputeAlignBounds(alignParams, response);
            result.AlignOkCount = response.OkCount;
            result.AlignOffsetX = response.OffsetX;
            result.AlignOffsetY = response.OffsetY;
            result.AlignTheta = response.Theta;
            result.AlignOkShiftX = response.OkShiftX;
            result.AlignOkShiftY = response.OkShiftY;
            result.AlignOkAngle = response.OkAngle;
            result.NativeAlignMessage = response.Message;

            algorithm.Source.AlignDetectedCentersX = (int[])response.DetectedCentersX.Clone();
            algorithm.Source.AlignDetectedCentersY = (int[])response.DetectedCentersY.Clone();
            algorithm.Source.AlignOffsetX = response.OffsetX;
            algorithm.Source.AlignOffsetY = response.OffsetY;
            algorithm.Source.AlignTheta = response.Theta;
        }
        else
        {
            RunThresholdFallback(image, model, algorithm, result);
            result.NativeAlignMessage = response.Message;
        }
    }

    private static void RunShapeXViaBridge(PartRuntimeImage image, InspectionModel model, AlgorithmRuntimePacket algorithm, AlgorithmRunResult result)
    {
        var roi = algorithm.InspectionRoi;
        var parameters = AlignBridgeAdapter.BuildShapeXParams(model, roi, algorithm);
        var response = MptiAlgorithmNativeBridge.RunShapeX(
            image.SourcePixels, image.Width, image.Height, image.SourceStride,
            roi, parameters);

        result.UsedImage = true;
        if (response.Available && response.Success)
        {
            result.ElapsedMs = response.ElapsedMs;
            result.ForegroundPixels = response.ForegroundPixels;
            result.BlobCount = response.BlobCount;
            result.Bounds = response.ForegroundPixels > 0 ? roi : (RoiRect?)null;
            SetSourceParameter(algorithm, "Runtime.ShapeXBridge", "native");
            SetSourceParameter(algorithm, "Runtime.ShapeXIsOK", response.IsOK.ToString());
            SetSourceParameter(algorithm, "Runtime.ShapeXAreaRatio", response.AreaRate.ToString("F4"));
            SetSourceParameter(algorithm, "Runtime.ShapeXShift", $"{response.ShiftX:F2},{response.ShiftY:F2}");
            SetSourceParameter(algorithm, "Runtime.ShapeXOkFlags", response.OkFlagsMask.ToString());
        }
        else
        {
            SetSourceParameter(algorithm, "Runtime.ShapeXBridge", "fallback");
            SetSourceParameter(algorithm, "Runtime.ShapeXBridgeMessage", response.Message);
            RunThresholdFallback(image, model, algorithm, result);
        }
    }

    private static void RunPadBWViaBridge(PartRuntimeImage image, InspectionModel model, AlgorithmRuntimePacket algorithm, AlgorithmRunResult result)
    {
        var roi = algorithm.InspectionRoi;
        var parameters = AlignBridgeAdapter.BuildPadBWParams(model, roi, algorithm);
        var response = MptiAlgorithmNativeBridge.RunPadBW(
            image.SourcePixels, image.Width, image.Height, image.SourceStride,
            roi, parameters);

        result.UsedImage = true;
        if (response.Available && response.Success)
        {
            result.ElapsedMs = response.ElapsedMs;
            result.ForegroundPixels = response.ForegroundPixels;
            result.BlobCount = response.BlobCount;
            result.Bounds = response.ForegroundPixels > 0 ? roi : (RoiRect?)null;
            SetSourceParameter(algorithm, "Runtime.PadBWBridge", "native");
            SetSourceParameter(algorithm, "Runtime.PadBWIsOK", response.IsOK.ToString());
            SetSourceParameter(algorithm, "Runtime.PadBWAreaRate", response.AreaRate.ToString("F2"));
            SetSourceParameter(algorithm, "Runtime.PadBWShift", $"{response.ShiftX:F2},{response.ShiftY:F2}");
            SetSourceParameter(algorithm, "Runtime.PadBWOkFlags", response.OkFlagsMask.ToString());
        }
        else
        {
            SetSourceParameter(algorithm, "Runtime.PadBWBridge", "fallback");
            SetSourceParameter(algorithm, "Runtime.PadBWBridgeMessage", response.Message);
            RunThresholdFallback(image, model, algorithm, result);
        }
    }

    private static bool TryMapToGenericKind(string algorithmType, out MptiBridgeAlgoKind kind)
    {
        switch (algorithmType.ToUpperInvariant())
        {
            case "ALGOBGA":
                kind = MptiBridgeAlgoKind.BGA;
                return true;
            case "ALGOBLOB":
                kind = MptiBridgeAlgoKind.Blob;
                return true;
            case "ALGOEDGE":
                kind = MptiBridgeAlgoKind.Edge;
                return true;
            case "ALGOPATTERN":
                kind = MptiBridgeAlgoKind.Pattern;
                return true;
            default:
                kind = MptiBridgeAlgoKind.Unknown;
                return false;
        }
    }

    private static void RunGenericViaBridge(PartRuntimeImage image, InspectionModel model, AlgorithmRuntimePacket algorithm, AlgorithmRunResult result, MptiBridgeAlgoKind kind)
    {
        var roi = algorithm.InspectionRoi;
        var parameters = new MptiBridgeGenericParams
        {
            AlgoKind = (int)kind,
            BinaryMin = Net48Compat.Clamp(model.Threshold2D, 0, 255),
            BinaryMax = 255,
            UseInsp2D = 1,
            InvertCheck = 0,
            MinBlobArea = 10,
            ExpectedCenterX = roi.X + roi.Width / 2,
            ExpectedCenterY = roi.Y + roi.Height / 2,
            MinAreaRatio = 0.05f,
            MaxAreaRatio = 0.95f,
            ShiftXTolerance = 10,
            ShiftYTolerance = 10
        };

        var response = MptiAlgorithmNativeBridge.RunGeneric(
            image.SourcePixels, image.Width, image.Height, image.SourceStride,
            roi, parameters);

        result.UsedImage = true;
        if (response.Available && response.Success)
        {
            result.ElapsedMs = response.ElapsedMs;
            result.ForegroundPixels = response.ForegroundPixels;
            result.BlobCount = response.BlobCount;
            result.Bounds = response.ForegroundPixels > 0 ? roi : (RoiRect?)null;
            SetSourceParameter(algorithm, "Runtime.GenericBridge", "native");
            SetSourceParameter(algorithm, "Runtime.GenericAlgoKind", kind.ToString());
            SetSourceParameter(algorithm, "Runtime.GenericIsOK", response.IsOK.ToString());
            SetSourceParameter(algorithm, "Runtime.GenericAreaRatio", response.AreaRate.ToString("F4"));
            SetSourceParameter(algorithm, "Runtime.GenericShift", $"{response.ShiftX:F2},{response.ShiftY:F2}");
        }
        else
        {
            SetSourceParameter(algorithm, "Runtime.GenericBridge", "fallback");
            SetSourceParameter(algorithm, "Runtime.GenericBridgeMessage", response.Message);
            RunThresholdFallback(image, model, algorithm, result);
        }
    }

    private static void RunThresholdFallback(PartRuntimeImage image, InspectionModel model, AlgorithmRuntimePacket algorithm, AlgorithmRunResult result)
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

    private static RoiRect? ComputeAlignBounds(MptiBridgeAlignParams parameters, AlignBridgeResponse response)
    {
        if (response.OkCount <= 0)
        {
            return null;
        }

        var minX = int.MaxValue;
        var minY = int.MaxValue;
        var maxX = int.MinValue;
        var maxY = int.MinValue;
        for (var i = 0; i < parameters.SearchNum; i++)
        {
            minX = Math.Min(minX, response.DetectedCentersX[i]);
            minY = Math.Min(minY, response.DetectedCentersY[i]);
            maxX = Math.Max(maxX, response.DetectedCentersX[i]);
            maxY = Math.Max(maxY, response.DetectedCentersY[i]);
        }

        return new RoiRect(minX, minY, Math.Max(1, maxX - minX + 1), Math.Max(1, maxY - minY + 1));
    }

    private static int ReadAlgorithmThreshold(InspectionModel model, AlgorithmRuntimePacket algorithm)
    {
        var family = AlgorithmCatalog.Find(algorithm.Type).ParameterFamily;
        if (AlgorithmParameterStore.TryGetValue(algorithm.Parameters, $"{family}.Threshold", out var value) && int.TryParse(value, out var threshold))
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
        if (!AlgorithmParameterStore.TryGetValue(parameters, key, out var value) || string.IsNullOrWhiteSpace(value))
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
