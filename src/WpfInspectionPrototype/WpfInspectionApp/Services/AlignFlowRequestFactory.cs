using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class AlignFlowRequestFactory : IAlignFlowRequestFactory
{
    private readonly IAlgorithmLightService _lightService;

    public AlignFlowRequestFactory()
        : this(new AlgorithmLightService())
    {
    }

    public AlignFlowRequestFactory(IAlgorithmLightService lightService)
    {
        _lightService = lightService;
    }

    public AlignFlowRequest Create(InspectionModel model, string pttPath, int sourceWidth, int sourceHeight)
    {
        model.EnsureStructure();

        var activeWindow = ResolveActiveWindow(model);
        var activeAlgorithm = ResolveActiveAlgorithm(model, activeWindow);
        var lightState = _lightService.ReadState(activeAlgorithm);
        var activeRoi = activeWindow?.Roi;
        var windowCenterX = activeRoi is { IsValid: true } ? activeRoi.Value.X + activeRoi.Value.Width / 2 : 0;
        var windowCenterY = activeRoi is { IsValid: true } ? activeRoi.Value.Y + activeRoi.Value.Height / 2 : 0;
        var windowWidth = activeRoi is { IsValid: true } ? activeRoi.Value.Width : 0;
        var windowHeight = activeRoi is { IsValid: true } ? activeRoi.Value.Height : 0;

        var searchNum = Net48Compat.Clamp(model.AlignSearchNum, 1, 4);
        var searchPointsX = new int[4];
        var searchPointsY = new int[4];
        var searchSizeW = new int[4];
        var searchSizeH = new int[4];
        var fallbackRois = model.Part.Windows
            .Where(window => window.Roi.IsValid)
            .Select(window => window.Roi)
            .Take(4)
            .ToArray();

        for (var index = 0; index < 4; index++)
        {
            var roi = ReadSearchRoi(model, fallbackRois, index);
            if (roi.HasValue)
            {
                searchPointsX[index] = roi.Value.X + roi.Value.Width / 2;
                searchPointsY[index] = roi.Value.Y + roi.Value.Height / 2;
            }

            searchSizeW[index] = ResolveSearchWidth(model, roi);
            searchSizeH[index] = ResolveSearchHeight(model, roi);
        }

        var minBinary = Net48Compat.Clamp(Math.Min(model.Threshold2D, model.Threshold2DMax), 0, 255);
        var maxBinary = Net48Compat.Clamp(Math.Max(model.Threshold2D, model.Threshold2DMax), 0, 255);
        var minHeight = Math.Min(model.Threshold3D, model.Threshold3DMax);
        var maxHeight = Math.Max(model.Threshold3D, model.Threshold3DMax);

        return new AlignFlowRequest(
            PttPath: pttPath,
            WindowWidth: windowWidth,
            WindowHeight: windowHeight,
            WindowCenterX: windowCenterX,
            WindowCenterY: windowCenterY,
            SearchNum: searchNum,
            SearchPointsX: searchPointsX,
            SearchPointsY: searchPointsY,
            SearchSizeWidths: searchSizeW,
            SearchSizeHeights: searchSizeH,
            SearchMargin: model.AlignSearchMargin,
            MinBinary: minBinary,
            MaxBinary: maxBinary,
            TypeRange2D: model.AlignRange2DType,
            UseInsp2D: model.Use2D,
            InvertCheck: model.AlignInvertCheck,
            UseInsp3D: model.Use3D,
            HeightRateMin: minHeight,
            HeightRateMax: maxHeight,
            HeightAverage: model.AlignHeightAverage,
            TypeRange3D: model.AlignRange3DType,
            UseShift: model.AlignShiftEnabled,
            MaxShiftX: model.AlignShiftX,
            MaxShiftY: model.AlignShiftY,
            UseAngle: model.AlignAngleEnabled,
            MaxAngle: model.AlignAngle,
            SameSize: model.AlignSameSize,
            MinBlobArea: model.AlignFilter,
            FillHole: model.AlignFillHole,
            InspOption: model.AlignInspectionAreaCount ? 0x01 : 0,
            UseIpc: model.IpcUse,
            IpcClass: ResolveIpcClass(model.IpcClass),
            PixelResolutionX: model.Part?.PixelResolutionX ?? 0.0,
            PixelResolutionY: model.Part?.PixelResolutionY ?? 0.0,
            LightType: lightState.LightType,
            RedValue: lightState.RedValue,
            GreenValue: lightState.GreenValue,
            BlueValue: lightState.BlueValue,
            WhiteValue: lightState.WhiteValue,
            LightCnt: lightState.LightType == AlgorithmLightService.UserLight ? lightState.UserCells.Count : 0,
            ArrRedValue: CreateLightArray(lightState.UserCells, cell => cell.RedValue),
            ArrGreenValue: CreateLightArray(lightState.UserCells, cell => cell.GreenValue),
            ArrBlueValue: CreateLightArray(lightState.UserCells, cell => cell.BlueValue),
            ArrWhiteValue: CreateLightArray(lightState.UserCells, cell => cell.WhiteValue),
            ArrCalculation: CreateLightArray(lightState.UserCells, cell => cell.Operator),
            ArrLightPosition: CreateLightArray(lightState.UserCells, cell => cell.Position));
    }

    private static InspectionWindowData? ResolveActiveWindow(InspectionModel model)
    {
        return string.IsNullOrWhiteSpace(model.SelectedWindowId)
            ? model.Part.Windows.FirstOrDefault(window => window.Roi.IsValid)
            : model.Part.Windows.FirstOrDefault(window => window.Id == model.SelectedWindowId)
                ?? model.Part.Windows.FirstOrDefault(window => window.Roi.IsValid);
    }

    private static InspectionAlgorithmData? ResolveActiveAlgorithm(InspectionModel model, InspectionWindowData? activeWindow)
    {
        var algorithms = activeWindow?.Algorithms ?? model.Part.Windows.SelectMany(window => window.Algorithms).ToList();
        return algorithms.FirstOrDefault(algorithm => string.Equals(algorithm.Type, model.Algorithm, StringComparison.OrdinalIgnoreCase))
            ?? algorithms.FirstOrDefault(algorithm => string.Equals(algorithm.Type, "AlgoAlign", StringComparison.OrdinalIgnoreCase))
            ?? algorithms.FirstOrDefault();
    }

    private static RoiRect? ReadSearchRoi(InspectionModel model, RoiRect[] fallbackRois, int index)
    {
        if (index < model.AlignSearchRois.Length && model.AlignSearchRois[index] is { IsValid: true } roi)
        {
            return roi;
        }

        return index < fallbackRois.Length ? fallbackRois[index] : null;
    }

    private static int ResolveSearchWidth(InspectionModel model, RoiRect? roi)
    {
        if (!model.AlignSameSize && roi is { IsValid: true })
        {
            return Math.Max(1, roi.Value.Width);
        }

        return Math.Max(1, model.AlignSearchSizeX);
    }

    private static int ResolveSearchHeight(InspectionModel model, RoiRect? roi)
    {
        if (!model.AlignSameSize && roi is { IsValid: true })
        {
            return Math.Max(1, roi.Value.Height);
        }

        return Math.Max(1, model.AlignSearchSizeY);
    }

    private static int ResolveIpcClass(string ipcClass)
    {
        return ipcClass?.Trim().ToUpperInvariant() switch
        {
            "CLASS1" => 0,
            "CLASS3" => 2,
            _ => 1
        };
    }

    private int[] CreateLightArray(IReadOnlyList<AlgorithmLightCell> cells, Func<AlgorithmLightCell, int> selector)
    {
        var values = new int[_lightService.MaximumUserCellCount];
        var count = Math.Min(values.Length, cells.Count);
        for (var index = 0; index < count; index++)
        {
            values[index] = selector(cells[index]);
        }

        return values;
    }
}
