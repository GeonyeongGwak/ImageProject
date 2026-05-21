using System.Globalization;
using WpfInspectionApp.Interop;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

internal readonly record struct AlgorithmGenericBridgeProfile(
    MptiBridgeAlgoKind Kind,
    string Name);

internal static class AlgorithmNativeBridgeAdapter
{
    // 기존 4 HashSet (DirectBlobTypes/DirectBGATypes/DirectEdgeTypes/DirectPatternTypes)
    // 을 native MptiBridgeGetAlgoFamily lookup 으로 통합. 카탈로그가 NativeAlgoType 을 알고,
    // native 가 family 분류를 책임지므로 C# 에 별도 enum 매핑 테이블 불필요.
    //
    // NativeAlgoType == Unknown 인 알고리즘은 family lookup 도 Unknown 으로 떨어지므로
    // 자동으로 fallback (Generic bridge) 경로로 라우팅된다.
    private static bool IsFamily(string algorithmType, int expectedFamily)
    {
        var catalog = AlgorithmCatalog.Find(algorithmType);
        if (catalog.NativeAlgoType == NativeAlgoTypeIds.Unknown)
        {
            return false;
        }

        return MptiFlowNativeBridge.MptiBridgeGetAlgoFamily(catalog.NativeAlgoType) == expectedFamily;
    }

    private static readonly Dictionary<string, AlgorithmGenericBridgeProfile> GenericProfiles = BuildGenericProfiles();

    public static bool TryBuildBlobParams(InspectionModel model, AlgorithmRuntimePacket algorithm, out MptiBridgeBlobParams parameters)
    {
        if (!IsFamily(algorithm.Type, MptiFlowNativeBridge.FAMILY_BLOB))
        {
            parameters = default;
            return false;
        }

        var catalog = AlgorithmCatalog.Find(algorithm.Type);
        var family = catalog.ParameterFamily;
        var roi = algorithm.InspectionRoi;
        var roiArea = Math.Max(1, roi.Width * roi.Height);
        var binaryMin = ReadIntAny(algorithm.Parameters, Net48Compat.Clamp(model.Threshold2D, 0, 255),
            $"{family}.BinaryMin", $"{family}.MinBinary", $"{family}.MinValue", $"{family}.Min2D", "Common.BinaryMin");
        var binaryMax = ReadIntAny(algorithm.Parameters, 255,
            $"{family}.BinaryMax", $"{family}.MaxBinary", $"{family}.MaxValue", $"{family}.Max2D", "Common.BinaryMax");
        var useArea = ReadBoolAny(algorithm.Parameters, true,
            $"{family}.UseArea", $"{family}.AreaUse", $"{family}.UseBlobArea");
        var areaMin = useArea
            ? ReadDoubleAny(algorithm.Parameters, 10, $"{family}.AreaMin", $"{family}.BlobAreaMin", $"{family}.MinArea")
            : 0;
        var areaMax = useArea
            ? ReadDoubleAny(algorithm.Parameters, roiArea, $"{family}.AreaMax", $"{family}.BlobAreaMax", $"{family}.MaxArea")
            : roiArea;
        var useShift = ReadBoolAny(algorithm.Parameters, false,
            $"{family}.UseShift", $"{family}.UseShiftX", $"{family}.ShiftUse");
        var defaultTolerance = Math.Max(roi.Width, roi.Height) + 1000.0;

        parameters = new MptiBridgeBlobParams
        {
            BinaryMin = Net48Compat.Clamp(Math.Min(binaryMin, binaryMax), 0, 255),
            BinaryMax = Net48Compat.Clamp(Math.Max(binaryMin, binaryMax), 0, 255),
            UseInsp2D = ReadBoolAny(algorithm.Parameters, true, $"{family}.Use2D", $"{family}.UseInsp2D") ? 1 : 0,
            InvertCheck = ReadBoolAny(algorithm.Parameters, false, $"{family}.InvertCheck", $"{family}.Invert", $"{family}.InvChk") ? 1 : 0,
            MinBlobArea = Math.Max(1, (int)Math.Round(Math.Max(1, areaMin), MidpointRounding.AwayFromZero)),
            UseArea = useArea ? 1 : 0,
            AreaMin = Math.Max(0, areaMin),
            AreaMax = Math.Max(Math.Max(0, areaMin), areaMax),
            UseShift = useShift ? 1 : 0,
            ShiftX = useShift
                ? Math.Abs(ReadDoubleAny(algorithm.Parameters, 10.0, $"{family}.ShiftX", $"{family}.ShiftXTol", $"{family}.DistanceX"))
                : defaultTolerance,
            ShiftY = useShift
                ? Math.Abs(ReadDoubleAny(algorithm.Parameters, 10.0, $"{family}.ShiftY", $"{family}.ShiftYTol", $"{family}.DistanceY"))
                : defaultTolerance,
            ExpectedCenterX = roi.X + roi.Width / 2,
            ExpectedCenterY = roi.Y + roi.Height / 2,
            FillHole = ReadBoolAny(algorithm.Parameters, false, $"{family}.FillHole", $"{family}.UseFillHole") ? 1 : 0,
            TypeSelectBlob = ReadIntAny(algorithm.Parameters, 0, $"{family}.TypeSelectBlob", $"{family}.BlobSelectType")
        };
        return true;
    }

    public static bool TryBuildBGAParams(InspectionModel model, AlgorithmRuntimePacket algorithm, out MptiBridgeBGAParams parameters)
    {
        if (!IsFamily(algorithm.Type, MptiFlowNativeBridge.FAMILY_BGA))
        {
            parameters = default;
            return false;
        }

        var catalog = AlgorithmCatalog.Find(algorithm.Type);
        var family = catalog.ParameterFamily;
        var roi = algorithm.InspectionRoi;
        var roiArea = Math.Max(1, roi.Width * roi.Height);
        var binaryMin = ReadIntAny(algorithm.Parameters, Net48Compat.Clamp(model.Threshold2D, 0, 255),
            $"{family}.BinaryMin", $"{family}.MinBinary", $"{family}.MinValue", $"{family}.Min2D", "Common.BinaryMin");
        var binaryMax = ReadIntAny(algorithm.Parameters, 255,
            $"{family}.BinaryMax", $"{family}.MaxBinary", $"{family}.MaxValue", $"{family}.Max2D", "Common.BinaryMax");
        var useArea = ReadBoolAny(algorithm.Parameters, true,
            $"{family}.UseArea", $"{family}.AreaUse", $"{family}.UseBlobArea");
        var areaMin = useArea
            ? ReadDoubleAny(algorithm.Parameters, 10, $"{family}.AreaMin", $"{family}.BlobAreaMin", $"{family}.MinArea")
            : 0;
        var areaMax = useArea
            ? ReadDoubleAny(algorithm.Parameters, roiArea, $"{family}.AreaMax", $"{family}.BlobAreaMax", $"{family}.MaxArea")
            : roiArea;
        var useShift = ReadBoolAny(algorithm.Parameters, false,
            $"{family}.UseShift", $"{family}.UseShiftX", $"{family}.ShiftUse");
        var defaultTolerance = Math.Max(roi.Width, roi.Height) + 1000.0;
        var useCircleRate = ReadBoolAny(algorithm.Parameters, false, $"{family}.UseCircleRate", $"{family}.UseCircle");
        var useCoplanarity = ReadBoolAny(algorithm.Parameters, false, $"{family}.UseCoplanarity", $"{family}.UseCoplanarity3D");

        parameters = new MptiBridgeBGAParams
        {
            BinaryMin = Net48Compat.Clamp(Math.Min(binaryMin, binaryMax), 0, 255),
            BinaryMax = Net48Compat.Clamp(Math.Max(binaryMin, binaryMax), 0, 255),
            UseInsp2D = ReadBoolAny(algorithm.Parameters, true, $"{family}.Use2D", $"{family}.UseInsp2D") ? 1 : 0,
            InvertCheck = ReadBoolAny(algorithm.Parameters, false, $"{family}.InvertCheck", $"{family}.Invert", $"{family}.InvChk") ? 1 : 0,
            MinBlobArea = Math.Max(1, (int)Math.Round(Math.Max(1, areaMin), MidpointRounding.AwayFromZero)),
            TypeSelectBlob = ReadIntAny(algorithm.Parameters, 0, $"{family}.TypeSelectBlob", $"{family}.BlobSelectType"),
            FillHole = ReadBoolAny(algorithm.Parameters, false, $"{family}.FillHole", $"{family}.UseFillHole") ? 1 : 0,
            UseArea = useArea ? 1 : 0,
            AreaMin = Math.Max(0, areaMin),
            AreaMax = Math.Max(Math.Max(0, areaMin), areaMax),
            TeachArea = ReadDoubleAny(algorithm.Parameters, Math.Max(1, areaMin), $"{family}.TeachArea"),
            TeachVolume = ReadDoubleAny(algorithm.Parameters, 0.0, $"{family}.TeachVolume", $"{family}.Volume"),
            UseShift = useShift ? 1 : 0,
            ShiftX = useShift
                ? Math.Abs(ReadDoubleAny(algorithm.Parameters, 10.0, $"{family}.ShiftX", $"{family}.ShiftXTol", $"{family}.GridOffsetX"))
                : defaultTolerance,
            ShiftY = useShift
                ? Math.Abs(ReadDoubleAny(algorithm.Parameters, 10.0, $"{family}.ShiftY", $"{family}.ShiftYTol", $"{family}.GridOffsetY"))
                : defaultTolerance,
            ExpectedCenterX = roi.X + roi.Width / 2,
            ExpectedCenterY = roi.Y + roi.Height / 2,
            UseTeachWidth = ReadBoolAny(algorithm.Parameters, false, $"{family}.UseTeachWidth", $"{family}.UseWidth") ? 1 : 0,
            TeachWidth = ReadDoubleAny(algorithm.Parameters, 0.0, $"{family}.TeachWidth", $"{family}.Width"),
            TeachWidthRateMin = ReadDoubleAny(algorithm.Parameters, 80.0, $"{family}.TeachWidthRateMin", $"{family}.WidthRateMin"),
            TeachWidthRateMax = ReadDoubleAny(algorithm.Parameters, 120.0, $"{family}.TeachWidthRateMax", $"{family}.WidthRateMax"),
            UseTeachLength = ReadBoolAny(algorithm.Parameters, false, $"{family}.UseTeachLength", $"{family}.UseLength") ? 1 : 0,
            TeachLength = ReadDoubleAny(algorithm.Parameters, 0.0, $"{family}.TeachLength", $"{family}.Length"),
            TeachLengthRateMin = ReadDoubleAny(algorithm.Parameters, 80.0, $"{family}.TeachLengthRateMin", $"{family}.LengthRateMin"),
            TeachLengthRateMax = ReadDoubleAny(algorithm.Parameters, 120.0, $"{family}.TeachLengthRateMax", $"{family}.LengthRateMax"),
            UseCircleRate = useCircleRate ? 1 : 0,
            TeachCircleRate = ReadDoubleAny(algorithm.Parameters, 80.0, $"{family}.TeachCircleRate", $"{family}.CircleRate"),
            UseCoplanarity = useCoplanarity ? 1 : 0,
            CoplanarityMin = ReadDoubleAny(algorithm.Parameters, 0.0, $"{family}.CoplanarityMin"),
            CoplanarityMax = ReadDoubleAny(algorithm.Parameters, 999999.0, $"{family}.CoplanarityMax")
        };
        return true;
    }

    public static bool TryBuildPatternParams(InspectionModel model, AlgorithmRuntimePacket algorithm, out MptiBridgePatternParams parameters)
    {
        if (!IsFamily(algorithm.Type, MptiFlowNativeBridge.FAMILY_PATTERN))
        {
            parameters = default;
            return false;
        }

        var catalog = AlgorithmCatalog.Find(algorithm.Type);
        var family = catalog.ParameterFamily;
        var roi = algorithm.InspectionRoi;
        var binaryMin = ReadIntAny(algorithm.Parameters, Net48Compat.Clamp(model.Threshold2D, 0, 255),
            $"{family}.BinaryMin", $"{family}.MinBinary", $"{family}.MinValue", $"{family}.Min2D", $"{family}.BinarizeValue", $"{family}.Threshold", "Common.BinaryMin");
        var binaryMax = ReadIntAny(algorithm.Parameters, 255,
            $"{family}.BinaryMax", $"{family}.MaxBinary", $"{family}.MaxValue", $"{family}.Max2D", "Common.BinaryMax");
        var acceptScore = NormalizeScore(ReadDoubleAny(algorithm.Parameters, 0.8,
            $"{family}.AcceptScore", $"{family}.MatchScore", $"{family}.ScoreAll", $"{family}.PatternScore", $"{family}.CharScore", $"{family}.AlignAcceptScore"));
        var useShift = ReadBoolAny(algorithm.Parameters, false,
            $"{family}.UseShift", $"{family}.ShiftUse", $"{family}.UseOffsetDistance");
        var defaultTolerance = Math.Max(roi.Width, roi.Height) + 1000.0;
        var rangeAngle = Math.Abs(ReadDoubleAny(algorithm.Parameters, 0.0, $"{family}.RangeAngle", $"{family}.AngleRange"));
        var wndAngle = ReadDoubleAny(algorithm.Parameters, 0.0, $"{family}.WndAngle", $"{family}.Theta", $"{family}.Angle");
        var angleMin = ReadDoubleAny(algorithm.Parameters, wndAngle - rangeAngle, $"{family}.SearchAngleRangeMin", $"{family}.AngleMin");
        var angleMax = ReadDoubleAny(algorithm.Parameters, wndAngle + rangeAngle, $"{family}.SearchAngleRangeMax", $"{family}.AngleMax");

        parameters = new MptiBridgePatternParams
        {
            BinaryMin = Net48Compat.Clamp(Math.Min(binaryMin, binaryMax), 0, 255),
            BinaryMax = Net48Compat.Clamp(Math.Max(binaryMin, binaryMax), 0, 255),
            UseInsp2D = ReadBoolAny(algorithm.Parameters, true, $"{family}.Use2D", $"{family}.UseInsp2D") ? 1 : 0,
            InvertCheck = ReadBoolAny(algorithm.Parameters, false, $"{family}.InvertCheck", $"{family}.Invert", $"{family}.InvChk") ? 1 : 0,
            MinPatternArea = Math.Max(1, ReadIntAny(algorithm.Parameters, 1, $"{family}.MinPatternArea", $"{family}.MinBlobArea", $"{family}.AreaMin")),
            UsePolarity = ReadBoolAny(algorithm.Parameters, false, $"{family}.UsePolarity", $"{family}.PatternPolarity") ? 1 : 0,
            AcceptScore = acceptScore,
            UseShift = useShift ? 1 : 0,
            ShiftX = useShift
                ? Math.Abs(ReadDoubleAny(algorithm.Parameters, 10.0, $"{family}.ShiftX", $"{family}.ShiftXTol"))
                : defaultTolerance,
            ShiftY = useShift
                ? Math.Abs(ReadDoubleAny(algorithm.Parameters, 10.0, $"{family}.ShiftY", $"{family}.ShiftYTol"))
                : defaultTolerance,
            ExpectedCenterX = roi.X + roi.Width / 2,
            ExpectedCenterY = roi.Y + roi.Height / 2,
            RangeAngle = rangeAngle,
            WndAngle = wndAngle,
            SearchAngleRangeMin = Math.Min(angleMin, angleMax),
            SearchAngleRangeMax = Math.Max(angleMin, angleMax),
            SamplingAngle = Math.Max(0, ReadIntAny(algorithm.Parameters, 1, $"{family}.SamplingAngle")),
            UseNgOpt = ReadBoolAny(algorithm.Parameters, false, $"{family}.UseNgOption", $"{family}.UseNgOpt") ? 1 : 0,
            UseCharacter = ReadBoolAny(algorithm.Parameters, false, $"{family}.Character", $"{family}.UseCharacter", $"{family}.UseCharScore") ? 1 : 0,
            ModelFilter = ReadIntAny(algorithm.Parameters, 0, $"{family}.ModelFilter", $"{family}.Filter"),
            CntPatternPath = Math.Max(1, ReadIntAny(algorithm.Parameters, 1, $"{family}.ModelCount", $"{family}.ModelAddCount", $"{family}.CntPatternPath")),
            FactorRed = ReadDoubleAny(algorithm.Parameters, 1.0, $"{family}.FactorRed", $"{family}.RedFactor"),
            FactorGreen = ReadDoubleAny(algorithm.Parameters, 1.0, $"{family}.FactorGreen", $"{family}.GreenFactor"),
            FactorBlue = ReadDoubleAny(algorithm.Parameters, 1.0, $"{family}.FactorBlue", $"{family}.BlueFactor"),
            ModelPathInspect1 = ReadStringAny(algorithm.Parameters, string.Empty, $"{family}.ModelPathInspect1", $"{family}.ModelPathInspect", $"{family}.ModelPath"),
            ModelPathTeach = ReadStringAny(algorithm.Parameters, string.Empty, $"{family}.ModelPathTeach", $"{family}.TeachModelPath")
        };
        return true;
    }

    public static bool TryBuildEdgeParams(InspectionModel model, AlgorithmRuntimePacket algorithm, out MptiBridgeEdgeParams parameters)
    {
        if (!IsFamily(algorithm.Type, MptiFlowNativeBridge.FAMILY_EDGE))
        {
            parameters = default;
            return false;
        }

        var catalog = AlgorithmCatalog.Find(algorithm.Type);
        var family = catalog.ParameterFamily;
        var roi = algorithm.InspectionRoi;
        var roiArea = Math.Max(1, roi.Width * roi.Height);
        var binaryMin = ReadIntAny(algorithm.Parameters, Net48Compat.Clamp(model.Threshold2D, 0, 255),
            $"{family}.BinaryMin", $"{family}.MinBinary", $"{family}.MinValue", $"{family}.Min2D", $"{family}.Threshold", "Common.BinaryMin");
        var binaryMax = ReadIntAny(algorithm.Parameters, 255,
            $"{family}.BinaryMax", $"{family}.MaxBinary", $"{family}.MaxValue", $"{family}.Max2D", "Common.BinaryMax");
        var useArea = ReadBoolAny(algorithm.Parameters, false,
            $"{family}.UseArea", $"{family}.AreaUse", $"{family}.UseBlobArea");
        var areaMin = useArea
            ? ReadDoubleAny(algorithm.Parameters, 1, $"{family}.AreaMin", $"{family}.BlobAreaMin", $"{family}.MinArea")
            : 0;
        var areaMax = useArea
            ? ReadDoubleAny(algorithm.Parameters, roiArea, $"{family}.AreaMax", $"{family}.BlobAreaMax", $"{family}.MaxArea")
            : roiArea;
        var useShift = ReadBoolAny(algorithm.Parameters, false,
            $"{family}.UseShift", $"{family}.UseShiftX", $"{family}.ShiftUse");
        var defaultTolerance = Math.Max(roi.Width, roi.Height) + 1000.0;
        var distanceX = ReadDoubleAny(algorithm.Parameters, 0.0, $"{family}.TeachDistanceX", $"{family}.DistanceX");
        var distanceY = ReadDoubleAny(algorithm.Parameters, 0.0, $"{family}.TeachDistanceY", $"{family}.DistanceY");

        parameters = new MptiBridgeEdgeParams
        {
            BinaryMin = Net48Compat.Clamp(Math.Min(binaryMin, binaryMax), 0, 255),
            BinaryMax = Net48Compat.Clamp(Math.Max(binaryMin, binaryMax), 0, 255),
            UseInsp2D = ReadBoolAny(algorithm.Parameters, true, $"{family}.Use2D", $"{family}.UseInsp2D") ? 1 : 0,
            InvertCheck = ReadBoolAny(algorithm.Parameters, false, $"{family}.InvertCheck", $"{family}.Invert", $"{family}.InvChk") ? 1 : 0,
            MinBlobArea = Math.Max(1, ReadIntAny(algorithm.Parameters, 1, $"{family}.MinBlobArea", $"{family}.BlobAreaMin", $"{family}.AreaMin")),
            UseArea = useArea ? 1 : 0,
            AreaMin = Math.Max(0, areaMin),
            AreaMax = Math.Max(Math.Max(0, areaMin), areaMax),
            UseShift = useShift ? 1 : 0,
            ShiftX = useShift
                ? Math.Abs(ReadDoubleAny(algorithm.Parameters, 10.0, $"{family}.ShiftX", $"{family}.ShiftXTol"))
                : defaultTolerance,
            ShiftY = useShift
                ? Math.Abs(ReadDoubleAny(algorithm.Parameters, 10.0, $"{family}.ShiftY", $"{family}.ShiftYTol"))
                : defaultTolerance,
            UseAngle = ReadBoolAny(algorithm.Parameters, false, $"{family}.UseAngle", $"{family}.UseAng") ? 1 : 0,
            TeachRotate = ReadDoubleAny(algorithm.Parameters, 0.0, $"{family}.TeachRotate", $"{family}.StdTeachRotate", $"{family}.AngleSpec"),
            AngleTolerance = Math.Abs(ReadDoubleAny(algorithm.Parameters, 5.0, $"{family}.AngleTolerance", $"{family}.MaxAngle", $"{family}.AngleTol")),
            UseDistanceX = ReadBoolAny(algorithm.Parameters, false, $"{family}.UseDistanceX", $"{family}.DistanceXUse") ? 1 : 0,
            TeachDistanceX = distanceX,
            DistanceXTolerance = Math.Abs(ReadDoubleAny(algorithm.Parameters, defaultTolerance, $"{family}.DistanceXTolerance", $"{family}.DistanceXTol", $"{family}.DistanceXMax")),
            UseDistanceY = ReadBoolAny(algorithm.Parameters, false, $"{family}.UseDistanceY", $"{family}.DistanceYUse") ? 1 : 0,
            TeachDistanceY = distanceY,
            DistanceYTolerance = Math.Abs(ReadDoubleAny(algorithm.Parameters, defaultTolerance, $"{family}.DistanceYTolerance", $"{family}.DistanceYTol", $"{family}.DistanceYMax")),
            ExpectedCenterX = roi.X + roi.Width / 2,
            ExpectedCenterY = roi.Y + roi.Height / 2,
            SetLineCnt = Math.Max(1, ReadIntAny(algorithm.Parameters, 1, $"{family}.SetLineCount", $"{family}.SetLineCnt", $"{family}.LineCount")),
            UseGroup = ReadBoolAny(algorithm.Parameters, false, $"{family}.Group", $"{family}.UseGroup") ? 1 : 0,
            LineFindType = ReadIntAny(algorithm.Parameters, 0, $"{family}.LineFindType"),
            LineFindRate = ReadDoubleAny(algorithm.Parameters, 100.0, $"{family}.LineFindRate"),
            Direction = ReadIntAny(algorithm.Parameters, 0, $"{family}.Direction", $"{family}.IsHorizon"),
            MeasureDirection = ReadIntAny(algorithm.Parameters, 0, $"{family}.MeasureDirection")
        };
        return true;
    }

    public static bool TryBuildGenericParams(
        InspectionModel model,
        AlgorithmRuntimePacket algorithm,
        out AlgorithmGenericBridgeProfile profile,
        out MptiBridgeGenericParams parameters)
    {
        if (!GenericProfiles.TryGetValue(algorithm.Type, out profile))
        {
            parameters = default;
            return false;
        }

        var catalog = AlgorithmCatalog.Find(algorithm.Type);
        var family = catalog.ParameterFamily;
        var roi = algorithm.InspectionRoi;
        var roiArea = Math.Max(1, roi.Width * roi.Height);
        var binaryMin = ReadIntAny(algorithm.Parameters, Net48Compat.Clamp(model.Threshold2D, 0, 255),
            $"{family}.BinaryMin", $"{family}.MinBinary", $"{family}.MinValue", $"{family}.Min2D", "Common.BinaryMin");
        var binaryMax = ReadIntAny(algorithm.Parameters, 255,
            $"{family}.BinaryMax", $"{family}.MaxBinary", $"{family}.MaxValue", $"{family}.Max2D", "Common.BinaryMax");
        var useArea = ReadBoolAny(algorithm.Parameters, profile.Kind is MptiBridgeAlgoKind.Blob or MptiBridgeAlgoKind.BGA,
            $"{family}.UseArea", $"{family}.AreaUse", $"{family}.UseBlobArea");
        var areaMin = useArea
            ? ReadDoubleAny(algorithm.Parameters, profile.Kind == MptiBridgeAlgoKind.Edge ? 1 : 10,
                $"{family}.AreaMin", $"{family}.BlobAreaMin", $"{family}.MinArea")
            : 0;
        var areaMax = useArea
            ? ReadDoubleAny(algorithm.Parameters, roiArea,
                $"{family}.AreaMax", $"{family}.BlobAreaMax", $"{family}.MaxArea")
            : roiArea;
        var useShift = ReadBoolAny(algorithm.Parameters, false,
            $"{family}.UseShift", $"{family}.UseShiftX", $"{family}.ShiftUse");
        var defaultTolerance = Math.Max(roi.Width, roi.Height) + 1000.0;
        var shiftX = useShift
            ? Math.Abs(ReadDoubleAny(algorithm.Parameters, 10.0, $"{family}.ShiftX", $"{family}.ShiftXTol", $"{family}.DistanceX"))
            : defaultTolerance;
        var shiftY = useShift
            ? Math.Abs(ReadDoubleAny(algorithm.Parameters, 10.0, $"{family}.ShiftY", $"{family}.ShiftYTol", $"{family}.DistanceY"))
            : defaultTolerance;

        parameters = new MptiBridgeGenericParams
        {
            AlgoKind = (int)profile.Kind,
            BinaryMin = Net48Compat.Clamp(Math.Min(binaryMin, binaryMax), 0, 255),
            BinaryMax = Net48Compat.Clamp(Math.Max(binaryMin, binaryMax), 0, 255),
            UseInsp2D = ReadBoolAny(algorithm.Parameters, true, $"{family}.Use2D", $"{family}.UseInsp2D") ? 1 : 0,
            InvertCheck = ReadBoolAny(algorithm.Parameters, false, $"{family}.InvertCheck", $"{family}.Invert", $"{family}.InvChk") ? 1 : 0,
            MinBlobArea = Math.Max(1, (int)Math.Round(Math.Max(1, areaMin), MidpointRounding.AwayFromZero)),
            ExpectedCenterX = roi.X + roi.Width / 2,
            ExpectedCenterY = roi.Y + roi.Height / 2,
            MinAreaRatio = NormalizeAreaRatio(areaMin, roiArea, 0.0f),
            MaxAreaRatio = NormalizeAreaRatio(areaMax, roiArea, 1.0f),
            ShiftXTolerance = (float)Math.Max(0, shiftX),
            ShiftYTolerance = (float)Math.Max(0, shiftY)
        };
        return true;
    }

    private static float NormalizeAreaRatio(double value, int roiArea, float fallback)
    {
        if (double.IsNaN(value) || double.IsInfinity(value))
        {
            return fallback;
        }

        var ratio = value <= 1.0 ? value : value / Math.Max(1, roiArea);
        return (float)Net48Compat.Clamp(ratio, 0.0, 1.0);
    }

    private static double NormalizeScore(double value)
    {
        if (double.IsNaN(value) || double.IsInfinity(value))
        {
            return 0.8;
        }

        return Net48Compat.Clamp(value > 1.0 ? value / 100.0 : value, 0.0, 1.0);
    }

    private static int ReadIntAny(Dictionary<string, string> parameters, int fallback, params string[] keys)
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

    private static double ReadDoubleAny(Dictionary<string, string> parameters, double fallback, params string[] keys)
    {
        foreach (var key in keys)
        {
            if (AlgorithmParameterStore.TryGetValue(parameters, key, out var raw)
                && double.TryParse(raw, NumberStyles.Float, CultureInfo.InvariantCulture, out var value))
            {
                return value;
            }
        }

        return fallback;
    }

    private static bool ReadBoolAny(Dictionary<string, string> parameters, bool fallback, params string[] keys)
    {
        foreach (var key in keys)
        {
            if (!AlgorithmParameterStore.TryGetValue(parameters, key, out var raw))
            {
                continue;
            }

            if (bool.TryParse(raw, out var value))
            {
                return value;
            }

            if (double.TryParse(raw, NumberStyles.Float, CultureInfo.InvariantCulture, out var number))
            {
                return Math.Abs(number) > double.Epsilon;
            }
        }

        return fallback;
    }

    private static string ReadStringAny(Dictionary<string, string> parameters, string fallback, params string[] keys)
    {
        foreach (var key in keys)
        {
            if (AlgorithmParameterStore.TryGetValue(parameters, key, out var value) && !string.IsNullOrWhiteSpace(value))
            {
                return value;
            }
        }

        return fallback;
    }

    private static Dictionary<string, AlgorithmGenericBridgeProfile> BuildGenericProfiles()
    {
        var profiles = new Dictionary<string, AlgorithmGenericBridgeProfile>(StringComparer.OrdinalIgnoreCase);
        Add(profiles, MptiBridgeAlgoKind.Blob, "Generic Blob",
            "AlgoBW", "AlgoBlob", "AlgoBody_Blob", "AlgoNGBlob", "AlgoBump",
            "AlgoColor", "AlgoDisColor", "AlgoColorXY", "AlgoLead_Color",
            "AlgoGray_Mean", "AlgoHeight_Mean", "AlgoGray_Diff", "AlgoHeight_Diff",
            "AlgoBridge", "AlgoLead_Tip", "AlgoLead_Lift", "AlgoLead_Solder",
            "AlgoWidth", "AlgoVolume", "AlgoLead_Search", "AlgoLead_SideSolder",
            "AlgoTab", "AlgoTab_Search", "AlgoColorBand_Search", "AlgoGrid",
            "AlgoSolderCone", "AlgoFillet", "AlgoPackageThickness", "AlgoGWire",
            "AlgoWire", "AlgoPadArray", "AlgoFoot", "AlgoBarcode");
        Add(profiles, MptiBridgeAlgoKind.Edge, "Generic Edge",
            "AlgoEdge", "AlgoLine", "AlgoDistance", "AlgoEdgePoint", "AlgoBodyEdge");
        Add(profiles, MptiBridgeAlgoKind.Pattern, "Generic Pattern",
            "AlgoPattern", "AlgoPatternDiff", "AlgoForeignOCV", "AlgoOCR", "AlgoPOCR");
        Add(profiles, MptiBridgeAlgoKind.BGA, "Generic BGA",
            "AlgoBGA", "AlgoLQBGA");
        return profiles;
    }

    private static void Add(Dictionary<string, AlgorithmGenericBridgeProfile> profiles, MptiBridgeAlgoKind kind, string name, params string[] types)
    {
        var profile = new AlgorithmGenericBridgeProfile(kind, name);
        foreach (var type in types)
        {
            profiles[type] = profile;
        }
    }
}
