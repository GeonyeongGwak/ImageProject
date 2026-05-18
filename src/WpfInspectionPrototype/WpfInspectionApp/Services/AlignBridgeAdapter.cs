using WpfInspectionApp.Interop;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

internal static class AlignBridgeAdapter
{
    public static MptiBridgeAlignParams BuildParams(InspectionModel model, RoiRect windowRoi, AlgorithmRuntimePacket algorithm)
    {
        var parameters = algorithm.Parameters;
        var searchNum = Net48Compat.Clamp(ReadInt(parameters, "Align.SearchNum", model.AlignSearchNum), 1, 4);
        var searchPointsX = new int[4];
        var searchPointsY = new int[4];
        var searchSizeW = new int[4];
        var searchSizeH = new int[4];

        var sameSize = ReadBool(parameters, "Align.SameSize", model.AlignSameSize);
        var defaultW = Math.Max(1, ReadInt(parameters, "Align.SearchSizeX", model.AlignSearchSizeX));
        var defaultH = Math.Max(1, ReadInt(parameters, "Align.SearchSizeY", model.AlignSearchSizeY));

        for (var i = 0; i < 4; i++)
        {
            var roi = i < model.AlignSearchRois.Length ? model.AlignSearchRois[i] : null;
            if (TryReadSearchPoint(parameters, i + 1, out var centerX, out var centerY))
            {
                var width = Math.Max(1, ReadInt(parameters, $"Align.SearchSize{i + 1}.W", defaultW));
                var height = Math.Max(1, ReadInt(parameters, $"Align.SearchSize{i + 1}.H", defaultH));
                searchPointsX[i] = centerX;
                searchPointsY[i] = centerY;
                searchSizeW[i] = sameSize ? defaultW : width;
                searchSizeH[i] = sameSize ? defaultH : height;
            }
            else if (roi.HasValue && roi.Value.IsValid)
            {
                searchPointsX[i] = roi.Value.X + roi.Value.Width / 2;
                searchPointsY[i] = roi.Value.Y + roi.Value.Height / 2;
                searchSizeW[i] = sameSize ? defaultW : Math.Max(1, roi.Value.Width);
                searchSizeH[i] = sameSize ? defaultH : Math.Max(1, roi.Value.Height);
            }
            else
            {
                var fallback = PlaceholderSearchPoint(windowRoi, i, defaultW, defaultH);
                searchPointsX[i] = fallback.X;
                searchPointsY[i] = fallback.Y;
                searchSizeW[i] = defaultW;
                searchSizeH[i] = defaultH;
            }
        }

        var threshold = ReadInt(parameters, "Align.Threshold", model.Threshold2D);
        var minBinary = ReadInt(parameters, "Align.BinaryMin", threshold);
        var maxBinary = ReadInt(parameters, "Align.BinaryMax", 255);

        return new MptiBridgeAlignParams
        {
            SearchNum = searchNum,
            SearchPointsX = searchPointsX,
            SearchPointsY = searchPointsY,
            SearchSizeW = searchSizeW,
            SearchSizeH = searchSizeH,
            SearchMargin = Math.Max(0, ReadInt(parameters, "Align.SearchMargin", model.AlignSearchMargin)),
            MinBinary = Net48Compat.Clamp(minBinary, 0, 255),
            MaxBinary = Net48Compat.Clamp(maxBinary, 0, 255),
            UseInsp2D = ReadBool(parameters, "Align.Use2D", true) ? 1 : 0,
            InvertCheck = ReadBool(parameters, "Align.InvertCheck", false) ? 1 : 0,
            UseShift = ReadBool(parameters, "Align.UseShift", model.AlignShiftEnabled) ? 1 : 0,
            MaxShiftX = ReadDouble(parameters, "Align.ShiftX", model.AlignShiftX),
            MaxShiftY = ReadDouble(parameters, "Align.ShiftY", model.AlignShiftY),
            UseAngle = ReadBool(parameters, "Align.UseAngle", model.AlignAngleEnabled) ? 1 : 0,
            MaxAngle = ReadDouble(parameters, "Align.Angle", model.AlignAngle),
            SameSize = sameSize ? 1 : 0,
            MinBlobArea = Math.Max(1, ReadInt(parameters, "Align.MinBlobArea", 5))
        };
    }

    private static (int X, int Y) PlaceholderSearchPoint(RoiRect windowRoi, int index, int defaultW, int defaultH)
    {
        var halfW = Math.Max(defaultW / 2, 1);
        var halfH = Math.Max(defaultH / 2, 1);
        var inset = Math.Max(halfW, halfH) + 4;
        var left = windowRoi.X + inset;
        var top = windowRoi.Y + inset;
        var right = windowRoi.X + Math.Max(windowRoi.Width - inset, inset + 1);
        var bottom = windowRoi.Y + Math.Max(windowRoi.Height - inset, inset + 1);
        return index switch
        {
            0 => (left, top),
            1 => (right, top),
            2 => (left, bottom),
            _ => (right, bottom),
        };
    }

    private static int ReadInt(Dictionary<string, string> parameters, string key, int fallback)
    {
        if (parameters.TryGetValue(key, out var raw)
            && int.TryParse(raw, System.Globalization.NumberStyles.Integer, System.Globalization.CultureInfo.InvariantCulture, out var parsed))
        {
            return parsed;
        }

        if (parameters.TryGetValue(key, out raw)
            && double.TryParse(raw, System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out var number))
        {
            return (int)Math.Round(number, MidpointRounding.AwayFromZero);
        }

        return fallback;
    }

    public static MptiBridgeShapeXParams BuildShapeXParams(InspectionModel model, RoiRect inspectionRoi, AlgorithmRuntimePacket algorithm)
    {
        var threshold = Net48Compat.Clamp(model.Threshold2D, 0, 255);
        return new MptiBridgeShapeXParams
        {
            BinaryMin = Net48Compat.Clamp(ReadInt(algorithm.Parameters, "ShapeX.BinaryMin", threshold), 0, 255),
            BinaryMax = Net48Compat.Clamp(ReadInt(algorithm.Parameters, "ShapeX.BinaryMax", 255), 0, 255),
            UseInsp2D = 1,
            InvertCheck = 0,
            UseShape = 1,
            UseInner = 0,
            UseExist = 1,
            UseShift = 1,
            ShapeAreaMin = ReadFloat(algorithm.Parameters, "ShapeX.AreaMin", 0.05f),
            ShapeAreaMax = ReadFloat(algorithm.Parameters, "ShapeX.AreaMax", 0.95f),
            ShiftXTolerance = ReadFloat(algorithm.Parameters, "ShapeX.ShiftXTol", 10f),
            ShiftYTolerance = ReadFloat(algorithm.Parameters, "ShapeX.ShiftYTol", 10f),
            ExpectedCenterX = inspectionRoi.X + inspectionRoi.Width / 2,
            ExpectedCenterY = inspectionRoi.Y + inspectionRoi.Height / 2,
            MinBlobArea = Math.Max(1, ReadInt(algorithm.Parameters, "ShapeX.MinBlobArea", 10))
        };
    }

    public static MptiBridgePadBWParams BuildPadBWParams(InspectionModel model, RoiRect inspectionRoi, AlgorithmRuntimePacket algorithm)
    {
        var parameters = algorithm.Parameters;
        var threshold = Net48Compat.Clamp(model.Threshold2D, 0, 255);
        var expectedArea = ReadDouble(parameters, "PadBW.TeachArea", inspectionRoi.Width * inspectionRoi.Height * 0.5);
        return new MptiBridgePadBWParams
        {
            BinaryMin = Net48Compat.Clamp(ReadInt(parameters, "PadBW.BinaryMin", threshold), 0, 255),
            BinaryMax = Net48Compat.Clamp(ReadInt(parameters, "PadBW.BinaryMax", 255), 0, 255),
            UseInsp2D = ReadBool(parameters, "PadBW.Use2D", true) ? 1 : 0,
            InvertCheck = ReadBool(parameters, "PadBW.InvertCheck", false) ? 1 : 0,
            UseTeachArea = ReadBool(parameters, "PadBW.UseTeachArea", true) ? 1 : 0,
            TeachArea = expectedArea,
            TeachAreaRateMin = ReadDouble(parameters, "PadBW.AreaRateMin", 80.0),
            TeachAreaRateMax = ReadDouble(parameters, "PadBW.AreaRateMax", 120.0),
            UseShift = ReadBool(parameters, "PadBW.UseShift", true) ? 1 : 0,
            TeachShiftX = ReadDouble(parameters, "PadBW.ShiftX", 10.0),
            TeachShiftY = ReadDouble(parameters, "PadBW.ShiftY", 10.0),
            ExpectedCenterX = inspectionRoi.X + inspectionRoi.Width / 2,
            ExpectedCenterY = inspectionRoi.Y + inspectionRoi.Height / 2,
            UseBlobArea = ReadBool(parameters, "PadBW.UseBlobArea", true) ? 1 : 0,
            BlobAreaMin = ReadDouble(parameters, "PadBW.BlobAreaMin", 50.0),
            UseFillHole = ReadBool(parameters, "PadBW.UseFillHole", false) ? 1 : 0,
            FilterLevel = ReadInt(parameters, "PadBW.FilterLevel", 0),
            MinBlobArea = Math.Max(1, ReadInt(parameters, "PadBW.MinBlobArea", 10))
        };
    }

    private static bool TryReadSearchPoint(Dictionary<string, string> parameters, int index, out int x, out int y)
    {
        x = 0;
        y = 0;
        if (!parameters.TryGetValue($"Align.SearchPoint{index}.X", out var rawX)
            || !parameters.TryGetValue($"Align.SearchPoint{index}.Y", out var rawY))
        {
            return false;
        }

        x = ReadIntValue(rawX, 0);
        y = ReadIntValue(rawY, 0);
        return true;
    }

    private static bool ReadBool(Dictionary<string, string> parameters, string key, bool fallback)
    {
        if (!parameters.TryGetValue(key, out var raw))
        {
            return fallback;
        }

        if (bool.TryParse(raw, out var parsed))
        {
            return parsed;
        }

        if (double.TryParse(raw, System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out var number))
        {
            return Math.Abs(number) > double.Epsilon;
        }

        return fallback;
    }

    private static int ReadIntValue(string raw, int fallback)
    {
        if (int.TryParse(raw, System.Globalization.NumberStyles.Integer, System.Globalization.CultureInfo.InvariantCulture, out var parsed))
        {
            return parsed;
        }

        if (double.TryParse(raw, System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out var number))
        {
            return (int)Math.Round(number, MidpointRounding.AwayFromZero);
        }

        return fallback;
    }

    private static float ReadFloat(Dictionary<string, string> parameters, string key, float fallback)
    {
        if (parameters.TryGetValue(key, out var raw) && float.TryParse(raw, System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out var parsed))
        {
            return parsed;
        }
        return fallback;
    }

    private static double ReadDouble(Dictionary<string, string> parameters, string key, double fallback)
    {
        if (parameters.TryGetValue(key, out var raw) && double.TryParse(raw, System.Globalization.NumberStyles.Float, System.Globalization.CultureInfo.InvariantCulture, out var parsed))
        {
            return parsed;
        }
        return fallback;
    }
}
