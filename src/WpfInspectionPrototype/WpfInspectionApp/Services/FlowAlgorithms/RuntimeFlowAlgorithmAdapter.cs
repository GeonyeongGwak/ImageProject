using System.Globalization;
using System.IO;
using WpfInspectionApp.Interop;
using WpfInspectionApp.Models;
using WpfInspectionApp.Services;

namespace WpfInspectionApp.Services.FlowAlgorithms;

internal static class RuntimeFlowAlgorithmAdapter
{
    public static bool TryRunFlowFirst(
        InspectionModel model,
        WindowRuntimePacket window,
        AlgorithmRuntimePacket algorithm,
        IFlowAlgorithmExecutionService? executionService,
        AlgorithmRunResult result)
    {
        if (executionService == null)
        {
            return false;
        }

        var pttPath = model.Part.PttPath;
        if (string.IsNullOrWhiteSpace(pttPath) || !File.Exists(pttPath))
        {
            return false;
        }

        if (!TryCreateFlowAlgorithm(model, algorithm, out var flowAlgorithm, out var flowParameters))
        {
            return false;
        }

        var request = new FlowAlgorithmExecutionRequest(
            pttPath!,
            flowAlgorithm,
            flowParameters,
            WindowRoi: window.Roi.IsValid ? window.Roi : algorithm.InspectionRoi,
            Light: CreateLightParams(algorithm.Source),
            Resolution: ResolveResolution(model));

        var flow = executionService.Run(request);
        SetSourceParameter(algorithm, "Runtime.FlowBridge", flow.Success ? "native-flow" : "fallback");
        SetSourceParameter(algorithm, "Runtime.FlowBridgeAlgorithm", flowAlgorithm.DisplayName);
        SetSourceParameter(algorithm, "Runtime.FlowBridgeMessage", flow.Summary);
        SetSourceParameter(algorithm, "Runtime.FlowBridgeReaderCode", flow.ReaderCode.ToString(CultureInfo.InvariantCulture));
        SetSourceParameter(algorithm, "Runtime.FlowBridgeIsInsp", flow.IsInsp.ToString());
        SetSourceParameter(algorithm, "Runtime.FlowBridgeIsOK", flow.IsOk.ToString());
        foreach (var field in flow.Fields)
        {
            SetSourceParameter(algorithm, $"Runtime.Flow.{field.Key}", field.Value);
        }

        if (!flow.Success)
        {
            return false;
        }

        result.UsedImage = true;
        result.IsOk = flow.IsOk;
        result.NativeBridgeName = flowAlgorithm.DisplayName;
        result.NativeBridgeMode = "native-flow";
        result.NativeBridgeMessage = flow.Summary;
        result.Bounds = algorithm.InspectionRoi;
        result.BlobCount = flow.IsInsp ? 1 : 0;
        result.ForegroundPixels = Math.Max(1, algorithm.InspectionRoi.Width * algorithm.InspectionRoi.Height / 16);
        ApplyTypedFields(flow, algorithm, result);
        return true;
    }

    // C# 의 algorithm.Type (string) → native InspAlgoType (int) 매핑.
    // 새 algorithm 타입 추가 시 여기 한 줄 + native MptiBridgeAddAlgo / MptiBridgeGetAlgoFamily
    // switch 만 업데이트하면 family-based dispatch 가 자동 동작.
    private static readonly Dictionary<string, int> AlgoTypeMap = new(StringComparer.OrdinalIgnoreCase)
    {
        ["AlgoAlign"]       = MptiFlowNativeBridge.EALGO_ALIGN,
        ["AlgoPadBW"]       = MptiFlowNativeBridge.EALGO_PADBW,
        ["AlgoBlob"]        = MptiFlowNativeBridge.EALGO_BLOB,
        ["AlgoBody_Blob"]   = MptiFlowNativeBridge.EALGO_BODY_BLOB,
        ["AlgoNGBlob"]      = MptiFlowNativeBridge.EALGO_NGBLOB,
        ["AlgoBGA"]         = MptiFlowNativeBridge.EALGO_BGA,
        ["AlgoEdge"]        = MptiFlowNativeBridge.EALGO_EDGE,
        ["AlgoBodyEdge"]    = MptiFlowNativeBridge.EALGO_BODY_EDGE,
        ["AlgoPattern"]     = MptiFlowNativeBridge.EALGO_PATTERN,
        ["AlgoPatternDiff"] = MptiFlowNativeBridge.EALGO_PATTERN_DIFF,
        ["AlgoOCR"]         = MptiFlowNativeBridge.EALGO_OCR,
        ["AlgoPOCR"]        = MptiFlowNativeBridge.EALGO_POCR,
        ["AlgoShapeX"]      = MptiFlowNativeBridge.EALGO_SHAPEX,
    };

    // OCR / POCR 만 EINSP_OCR. 나머지 Pattern family 는 EINSP_MOUNT.
    private static int ResolveInspType(int algoType)
    {
        return algoType switch
        {
            MptiFlowNativeBridge.EALGO_ALIGN => MptiFlowNativeBridge.EINSP_ALIGN,
            MptiFlowNativeBridge.EALGO_OCR => MptiFlowNativeBridge.EINSP_OCR,
            MptiFlowNativeBridge.EALGO_POCR => MptiFlowNativeBridge.EINSP_OCR,
            _ => MptiFlowNativeBridge.EINSP_MOUNT
        };
    }

    private static string ResolveDisplayName(string algorithmType)
    {
        // 알고리즘 식별이 가능하도록 카탈로그의 display 이름을 우선 사용.
        var catalog = AlgorithmCatalog.Find(algorithmType);
        return string.IsNullOrWhiteSpace(catalog.DisplayName) ? algorithmType : catalog.DisplayName;
    }

    // 13-case algorithm-type switch 를 family-based dispatch 로 교체.
    // family lookup 의 source of truth 는 native MptiBridgeGetAlgoFamily.
    // → 새 algorithm 추가 시 native 의 family switch + 위 AlgoTypeMap 만 업데이트하면 됨.
    private static bool TryCreateFlowAlgorithm(
        InspectionModel model,
        AlgorithmRuntimePacket algorithm,
        out IFlowAlgorithm flowAlgorithm,
        out IFlowAlgorithmParameters flowParameters)
    {
        flowAlgorithm = null!;
        flowParameters = null!;

        if (!AlgoTypeMap.TryGetValue(algorithm.Type, out var algoType))
        {
            return false;
        }

        var family = MptiFlowNativeBridge.MptiBridgeGetAlgoFamily(algoType);
        var displayName = ResolveDisplayName(algorithm.Type);
        var inspType = ResolveInspType(algoType);

        switch (family)
        {
            case MptiFlowNativeBridge.FAMILY_ALIGN:
                flowAlgorithm = new AlignFlowAlgorithm();
                flowParameters = CreateAlignParameters(model, algorithm);
                return true;

            case MptiFlowNativeBridge.FAMILY_PADBW:
                flowAlgorithm = new PadBWFlowAlgorithm();
                flowParameters = CreatePadBWParameters(model, algorithm);
                return true;

            case MptiFlowNativeBridge.FAMILY_SHAPEX:
                flowAlgorithm = new ShapeXFlowAlgorithm();
                flowParameters = CreateShapeXParameters(algorithm);
                return true;

            case MptiFlowNativeBridge.FAMILY_BLOB:
                if (AlgorithmNativeBridgeAdapter.TryBuildBlobParams(model, algorithm, out var blob))
                {
                    flowAlgorithm = MakeFamilyAlias(new BlobFlowAlgorithm(), displayName, algoType, inspType);
                    flowParameters = CreateBlobParameters(blob);
                    return true;
                }
                break;

            case MptiFlowNativeBridge.FAMILY_EDGE:
                if (AlgorithmNativeBridgeAdapter.TryBuildEdgeParams(model, algorithm, out var edge))
                {
                    flowAlgorithm = MakeFamilyAlias(new EdgeFlowAlgorithm(), displayName, algoType, inspType);
                    flowParameters = CreateEdgeParameters(edge);
                    return true;
                }
                break;

            case MptiFlowNativeBridge.FAMILY_PATTERN:
                if (AlgorithmNativeBridgeAdapter.TryBuildPatternParams(model, algorithm, out var pattern))
                {
                    flowAlgorithm = MakeFamilyAlias(new PatternFlowAlgorithm(), displayName, algoType, inspType);
                    flowParameters = CreatePatternParameters(pattern);
                    return true;
                }
                break;

            case MptiFlowNativeBridge.FAMILY_BGA:
                if (AlgorithmNativeBridgeAdapter.TryBuildBGAParams(model, algorithm, out var bga))
                {
                    flowAlgorithm = MakeFamilyAlias(new BGAFlowAlgorithm(), displayName, algoType, inspType);
                    flowParameters = new BGAParameters
                    {
                        MinBinary = bga.BinaryMin,
                        MaxBinary = bga.BinaryMax,
                        TeachArea = bga.TeachArea,
                        TeachVolume = bga.TeachVolume,
                        UseCircleRate = bga.UseCircleRate != 0,
                        TeachCircleRate = bga.TeachCircleRate
                    };
                    return true;
                }
                break;

            case MptiFlowNativeBridge.FAMILY_BW:
                // BW family 는 현재 dedicated FlowAlgorithm 미보유. 향후 AlgoBW 확장 시
                // PadBW 와 같은 패턴으로 BWFlowAlgorithm 추가 후 여기에 연결.
                break;

            case MptiFlowNativeBridge.FAMILY_UNKNOWN:
            default:
                break;
        }

        return false;
    }

    // base FlowAlgorithm 의 default DisplayName/AlgoType/InspType 가 알고리즘 type 과
    // 같으면 alias wrapping 필요 없음 (예: AlgoAlign → AlignFlowAlgorithm 그대로 OK).
    // 다르면 (예: AlgoBody_Blob → BlobFlowAlgorithm base 지만 AlgoType 가 EALGO_BODY_BLOB)
    // FlowAlgorithmAlias 로 감싸야 native 가 올바른 algoType 으로 dispatch.
    private static IFlowAlgorithm MakeFamilyAlias(IFlowAlgorithm baseAlgo, string displayName, int algoType, int inspType)
    {
        if (baseAlgo.AlgoType == algoType && baseAlgo.InspType == inspType)
        {
            return baseAlgo;
        }

        return new FlowAlgorithmAlias(baseAlgo, displayName, algoType, inspType);
    }

    private static BlobParameters CreateBlobParameters(MptiBridgeBlobParams blob)
    {
        return new BlobParameters
        {
            MinBinary = blob.BinaryMin,
            MaxBinary = blob.BinaryMax,
            InvertCheck = blob.InvertCheck != 0,
            AreaMin = blob.AreaMin,
            AreaMax = blob.AreaMax,
            ShiftX = blob.ShiftX,
            ShiftY = blob.ShiftY
        };
    }

    private static EdgeParameters CreateEdgeParameters(MptiBridgeEdgeParams edge)
    {
        return new EdgeParameters
        {
            MinBinary = edge.BinaryMin,
            MaxBinary = edge.BinaryMax,
            SetLineCnt = edge.SetLineCnt,
            LineFindRate = edge.LineFindRate,
            UseAngle = edge.UseAngle != 0,
            TeachRotate = edge.TeachRotate
        };
    }

    private static PatternParameters CreatePatternParameters(MptiBridgePatternParams pattern)
    {
        return new PatternParameters
        {
            UsePolarity = pattern.UsePolarity != 0,
            AcceptScore = pattern.AcceptScore,
            RangeAngle = pattern.RangeAngle,
            WndAngle = pattern.WndAngle,
            SearchAngleRangeMin = pattern.SearchAngleRangeMin,
            SearchAngleRangeMax = pattern.SearchAngleRangeMax,
            ModelPathInspect = pattern.ModelPathInspect1 ?? string.Empty
        };
    }

    private static AlignParameters CreateAlignParameters(InspectionModel model, AlgorithmRuntimePacket algorithm)
    {
        var p = AlignBridgeAdapter.BuildParams(model, algorithm.InspectionRoi, algorithm);
        return new AlignParameters
        {
            SearchNum = p.SearchNum,
            MinBinary = p.MinBinary,
            MaxBinary = p.MaxBinary,
            SearchSizeX = FirstPositive(p.SearchSizeW, model.AlignSearchSizeX),
            SearchSizeY = FirstPositive(p.SearchSizeH, model.AlignSearchSizeY),
            SearchMargin = p.SearchMargin,
            TypeRange2D = model.AlignRange2DType,
            InvertCheck = p.InvertCheck != 0,
            UseInsp3D = model.Use3D,
            HeightRateMin = model.Threshold3D,
            HeightRateMax = model.Threshold3DMax,
            MaxShiftX = p.MaxShiftX,
            MaxShiftY = p.MaxShiftY,
            MaxAngle = p.MaxAngle,
            MinBlobArea = p.MinBlobArea,
            FillHole = model.AlignFillHole,
            InspectionAreaCount = model.AlignInspectionAreaCount,
            SearchPointsX = p.SearchPointsX,
            SearchPointsY = p.SearchPointsY,
            SearchSizeW = p.SearchSizeW,
            SearchSizeH = p.SearchSizeH
        };
    }

    private static PadBWParameters CreatePadBWParameters(InspectionModel model, AlgorithmRuntimePacket algorithm)
    {
        var p = AlignBridgeAdapter.BuildPadBWParams(model, algorithm.InspectionRoi, algorithm);
        return new PadBWParameters
        {
            BinaryMin = p.BinaryMin,
            BinaryMax = p.BinaryMax,
            UseTeachArea = p.UseTeachArea != 0,
            TeachAreaRateMin = p.TeachAreaRateMin,
            TeachAreaRateMax = p.TeachAreaRateMax,
            UseFillHole = p.UseFillHole != 0,
            FilterLevel = p.FilterLevel
        };
    }

    private static ShapeXParameters CreateShapeXParameters(AlgorithmRuntimePacket algorithm)
    {
        return new ShapeXParameters
        {
            MatchScore = (short)ReadInt(algorithm.Parameters, 700, "ShapeX.MatchScore", "ShapeX.Score"),
            AspectRatio = (float)ReadDouble(algorithm.Parameters, 0, "ShapeX.AspectRatio"),
            MaxNgArea = (float)ReadDouble(algorithm.Parameters, 0, "ShapeX.MaxNgArea", "ShapeX.AreaMax"),
            CriticalArea = (float)ReadDouble(algorithm.Parameters, 0, "ShapeX.CriticalArea"),
            TieBarRate = (float)ReadDouble(algorithm.Parameters, 0.6, "ShapeX.TieBarRate"),
            ContrastValue = ReadInt(algorithm.Parameters, 0, "ShapeX.ContrastValue")
        };
    }

    private static MptiBridgeFlowLightParams CreateLightParams(InspectionAlgorithmData source)
    {
        var parameters = source.Parameters;
        var lightCnt = ReadInt(parameters, 0, "Common.LightCnt");
        return new MptiBridgeFlowLightParams
        {
            LightType = ReadInt(parameters, AlgorithmLightService.TopLight, "Common.LightTypeNum"),
            RedValue = ReadInt(parameters, 100, "Common.RedValue"),
            GreenValue = ReadInt(parameters, 0, "Common.GreenValue"),
            BlueValue = ReadInt(parameters, 0, "Common.BlueValue"),
            WhiteValue = ReadInt(parameters, 0, "Common.WhiteValue"),
            LightCnt = lightCnt,
            ArrRedValue = ReadIntArray(parameters, "Common.ArrRedValueString"),
            ArrGreenValue = ReadIntArray(parameters, "Common.ArrGreenValueString"),
            ArrBlueValue = ReadIntArray(parameters, "Common.ArrBlueValueString"),
            ArrWhiteValue = ReadIntArray(parameters, "Common.ArrWhiteValueString"),
            ArrCalculation = ReadIntArray(parameters, "Common.ArrCalculationString"),
            ArrLightPosition = ReadIntArray(parameters, "Common.ArrLightPositionString")
        };
    }

    private static (double X, double Y)? ResolveResolution(InspectionModel model)
    {
        return model.Part.PixelResolutionX > 0 && model.Part.PixelResolutionY > 0
            ? (model.Part.PixelResolutionX, model.Part.PixelResolutionY)
            : null;
    }

    private static void ApplyTypedFields(FlowAlgorithmResult flow, AlgorithmRuntimePacket algorithm, AlgorithmRunResult result)
    {
        if (flow.Fields.TryGetValue("okCount", out var okCount) && int.TryParse(okCount, NumberStyles.Integer, CultureInfo.InvariantCulture, out var parsedOkCount))
        {
            result.NativeAlignAvailable = true;
            result.AlignOkCount = parsedOkCount;
            result.NativeAlignMessage = flow.Summary;
        }
        if (flow.Fields.TryGetValue("offsetX", out var offsetX) && TryParseDouble(offsetX, out var parsedOffsetX))
        {
            result.AlignOffsetX = parsedOffsetX;
            algorithm.Source.AlignOffsetX = parsedOffsetX;
        }
        if (flow.Fields.TryGetValue("offsetY", out var offsetY) && TryParseDouble(offsetY, out var parsedOffsetY))
        {
            result.AlignOffsetY = parsedOffsetY;
            algorithm.Source.AlignOffsetY = parsedOffsetY;
        }
        if (flow.Fields.TryGetValue("theta", out var theta) && TryParseDouble(theta, out var parsedTheta))
        {
            result.AlignTheta = parsedTheta;
            algorithm.Source.AlignTheta = parsedTheta;
        }
        if (flow.Fields.TryGetValue("area", out var area) && TryParseDouble(area, out var parsedArea))
        {
            result.ForegroundPixels = Math.Max(1, (int)Math.Round(parsedArea, MidpointRounding.AwayFromZero));
        }
        if (flow.Fields.TryGetValue("rectCnt", out var rectCnt) && int.TryParse(rectCnt, NumberStyles.Integer, CultureInfo.InvariantCulture, out var parsedRectCnt))
        {
            result.BlobCount = Math.Max(0, parsedRectCnt);
        }
    }

    private static int FirstPositive(int[]? values, int fallback)
    {
        if (values == null)
        {
            return fallback;
        }

        return values.FirstOrDefault(value => value > 0) is var found && found > 0 ? found : fallback;
    }

    private static int[] ReadIntArray(Dictionary<string, string> parameters, string key)
    {
        var values = new int[10];
        if (!AlgorithmParameterStore.TryGetValue(parameters, key, out var raw) || string.IsNullOrWhiteSpace(raw))
        {
            return values;
        }

        var tokens = raw.Split(new[] { '|', ',', ';' }, StringSplitOptions.RemoveEmptyEntries);
        for (var index = 0; index < Math.Min(values.Length, tokens.Length); index++)
        {
            values[index] = int.TryParse(tokens[index].Trim(), NumberStyles.Integer, CultureInfo.InvariantCulture, out var value)
                ? value
                : 0;
        }

        return values;
    }

    private static int ReadInt(Dictionary<string, string> parameters, int fallback, params string[] keys)
    {
        foreach (var key in keys)
        {
            if (!AlgorithmParameterStore.TryGetValue(parameters, key, out var raw))
            {
                continue;
            }

            if (int.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var value))
            {
                return value;
            }

            if (double.TryParse(raw, NumberStyles.Float, CultureInfo.InvariantCulture, out var number))
            {
                return (int)Math.Round(number, MidpointRounding.AwayFromZero);
            }
        }

        return fallback;
    }

    private static double ReadDouble(Dictionary<string, string> parameters, double fallback, params string[] keys)
    {
        foreach (var key in keys)
        {
            if (AlgorithmParameterStore.TryGetValue(parameters, key, out var raw) && TryParseDouble(raw, out var value))
            {
                return value;
            }
        }

        return fallback;
    }

    private static bool TryParseDouble(string value, out double parsed)
    {
        return double.TryParse(value, NumberStyles.Float, CultureInfo.InvariantCulture, out parsed);
    }

    private static void SetSourceParameter(AlgorithmRuntimePacket algorithm, string key, string value)
    {
        AlgorithmParameterStore.Set(algorithm.Source.Parameters, key, value);
    }

    private sealed class FlowAlgorithmAlias : IFlowAlgorithm
    {
        private readonly IFlowAlgorithm _inner;

        public FlowAlgorithmAlias(IFlowAlgorithm inner, string displayName, int algoType, int inspType)
        {
            _inner = inner;
            DisplayName = displayName;
            AlgoType = algoType;
            InspType = inspType;
        }

        public string DisplayName { get; }
        public int AlgoType { get; }
        public int InspType { get; }

        public IFlowAlgorithmParameters CreateParameters() => _inner.CreateParameters();

        public void ApplyParams(FlowAlgorithmSlot slot, IFlowAlgorithmParameters parameters)
        {
            _inner.ApplyParams(slot, parameters);
        }

        public FlowAlgorithmResult ReadResult(FlowAlgorithmSlot slot)
        {
            return _inner.ReadResult(slot);
        }
    }
}
