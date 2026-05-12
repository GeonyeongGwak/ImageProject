using WpfInspectionApp.Interop;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

internal static class AlignBridgeAdapter
{
    public static MptiBridgeAlignParams BuildParams(InspectionModel model, RoiRect windowRoi, AlgorithmRuntimePacket algorithm)
    {
        var searchNum = Net48Compat.Clamp(model.AlignSearchNum, 1, 4);
        var searchPointsX = new int[4];
        var searchPointsY = new int[4];
        var searchSizeW = new int[4];
        var searchSizeH = new int[4];

        var sameSize = model.AlignSameSize;
        var defaultW = Math.Max(1, model.AlignSearchSizeX);
        var defaultH = Math.Max(1, model.AlignSearchSizeY);

        for (var i = 0; i < 4; i++)
        {
            var roi = i < model.AlignSearchRois.Length ? model.AlignSearchRois[i] : null;
            if (roi.HasValue && roi.Value.IsValid)
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

        var threshold = ReadInt(algorithm.Parameters, "Align.Threshold", model.Threshold2D);
        var minBinary = ReadInt(algorithm.Parameters, "Align.BinaryMin", threshold);
        var maxBinary = ReadInt(algorithm.Parameters, "Align.BinaryMax", 255);

        return new MptiBridgeAlignParams
        {
            SearchNum = searchNum,
            SearchPointsX = searchPointsX,
            SearchPointsY = searchPointsY,
            SearchSizeW = searchSizeW,
            SearchSizeH = searchSizeH,
            SearchMargin = Math.Max(0, model.AlignSearchMargin),
            MinBinary = Net48Compat.Clamp(minBinary, 0, 255),
            MaxBinary = Net48Compat.Clamp(maxBinary, 0, 255),
            UseInsp2D = 1,
            InvertCheck = 0,
            UseShift = model.AlignShiftEnabled ? 1 : 0,
            MaxShiftX = model.AlignShiftX,
            MaxShiftY = model.AlignShiftY,
            UseAngle = model.AlignAngleEnabled ? 1 : 0,
            MaxAngle = model.AlignAngle,
            SameSize = model.AlignSameSize ? 1 : 0,
            MinBlobArea = Math.Max(1, ReadInt(algorithm.Parameters, "Align.MinBlobArea", 5))
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
        if (parameters.TryGetValue(key, out var raw) && int.TryParse(raw, out var parsed))
        {
            return parsed;
        }

        return fallback;
    }
}
