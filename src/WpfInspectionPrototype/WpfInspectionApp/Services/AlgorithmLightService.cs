using System.Globalization;
using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class AlgorithmLightService : IAlgorithmLightService
{
    public const int TopLight = 0;
    public const int MiddleLight = 1;
    public const int BottomLight = 2;
    public const int UserLight = 3;
    public const int SideRedLight = 4;
    public const int SideGreenLight = 5;
    public const int SideBlueLight = 6;
    public const int Side4Light = 7;
    public const int ThreeDLight = 8;

    public int MaximumChannelValue => 200;
    public int MaximumUserCellCount => 10;

    public AlgorithmLightState ReadState(InspectionAlgorithmData? algorithm)
    {
        if (algorithm == null)
        {
            return CreateStateForLightType(TopLight);
        }

        algorithm.ApplyCatalogDefaults();
        var parameters = algorithm.Parameters;
        var lightType = ReadInt(parameters, "Common.LightTypeNum", TopLight, TopLight, ThreeDLight);
        var state = new AlgorithmLightState
        {
            LightType = lightType,
            RedValue = ReadInt(parameters, "Common.RedValue", 100, 0, MaximumChannelValue),
            GreenValue = ReadInt(parameters, "Common.GreenValue", 0, 0, MaximumChannelValue),
            BlueValue = ReadInt(parameters, "Common.BlueValue", 0, 0, MaximumChannelValue),
            WhiteValue = ReadInt(parameters, "Common.WhiteValue", 0, 0, MaximumChannelValue),
            UserCells = ReadUserCells(parameters)
        };

        ApplyReferenceFallbacks(state);
        SaveState(algorithm, state);
        return state;
    }

    public void SaveState(InspectionAlgorithmData algorithm, AlgorithmLightState state)
    {
        algorithm.ApplyCatalogDefaults();
        var parameters = algorithm.Parameters;
        state.LightType = Net48Compat.Clamp(state.LightType, TopLight, ThreeDLight);
        ClampNormalChannels(state);

        AlgorithmParameterStore.Set(parameters, "Common.LightTypeNum", FormatInt(state.LightType));
        AlgorithmParameterStore.Set(parameters, "Common.RedValue", FormatInt(state.RedValue));
        AlgorithmParameterStore.Set(parameters, "Common.GreenValue", FormatInt(state.GreenValue));
        AlgorithmParameterStore.Set(parameters, "Common.BlueValue", FormatInt(state.BlueValue));
        AlgorithmParameterStore.Set(parameters, "Common.WhiteValue", FormatInt(state.WhiteValue));

        var cells = state.UserCells
            .Take(MaximumUserCellCount)
            .Select(NormalizeUserCell)
            .ToList();
        state.UserCells = cells;
        AlgorithmParameterStore.Set(parameters, "Common.LightCnt", FormatInt(cells.Count));
        AlgorithmParameterStore.Set(parameters, "Common.ArrRedValueString", Join(cells.Select(cell => cell.RedValue)));
        AlgorithmParameterStore.Set(parameters, "Common.ArrGreenValueString", Join(cells.Select(cell => cell.GreenValue)));
        AlgorithmParameterStore.Set(parameters, "Common.ArrBlueValueString", Join(cells.Select(cell => cell.BlueValue)));
        AlgorithmParameterStore.Set(parameters, "Common.ArrWhiteValueString", Join(cells.Select(cell => cell.WhiteValue)));
        AlgorithmParameterStore.Set(parameters, "Common.ArrCalculationString", Join(cells.Select(cell => cell.Operator)));
        AlgorithmParameterStore.Set(parameters, "Common.ArrLightPositionString", Join(cells.Select(cell => cell.Position)));
    }

    public AlgorithmLightState CreateStateForLightType(int lightType, AlgorithmLightState? currentState = null)
    {
        var state = currentState == null
            ? new AlgorithmLightState()
            : Clone(currentState);
        state.LightType = Net48Compat.Clamp(lightType, TopLight, ThreeDLight);

        var defaults = GetNormalDefaults(state.LightType);
        state.RedValue = defaults.RedValue;
        state.GreenValue = defaults.GreenValue;
        state.BlueValue = defaults.BlueValue;
        state.WhiteValue = defaults.WhiteValue;

        if (state.LightType == UserLight && state.UserCells.Count == 0)
        {
            state.UserCells.Add(CreateDefaultUserCell());
        }

        return state;
    }

    public AlgorithmLightCell CreateDefaultUserCell(int position = TopLight, int operatorType = 0)
    {
        var cell = new AlgorithmLightCell
        {
            Position = Net48Compat.Clamp(position, TopLight, BottomLight),
            Operator = Net48Compat.Clamp(operatorType, 0, 2)
        };
        ApplyUserPositionDefaults(cell);
        return cell;
    }

    public LightChannelAvailability GetChannelAvailability(int lightTypeOrPosition, bool userCellPosition)
    {
        var value = userCellPosition
            ? Net48Compat.Clamp(lightTypeOrPosition, TopLight, BottomLight)
            : Net48Compat.Clamp(lightTypeOrPosition, TopLight, ThreeDLight);
        return value switch
        {
            TopLight => new LightChannelAvailability(true, true, true, true),
            MiddleLight or BottomLight => new LightChannelAvailability(true, false, true, false),
            SideRedLight or SideGreenLight or SideBlueLight or Side4Light => new LightChannelAvailability(true, true, true, false),
            _ => new LightChannelAvailability(false, false, false, false)
        };
    }

    private List<AlgorithmLightCell> ReadUserCells(Dictionary<string, string> parameters)
    {
        var red = ReadIntArray(parameters, "Common.ArrRedValueString");
        var green = ReadIntArray(parameters, "Common.ArrGreenValueString");
        var blue = ReadIntArray(parameters, "Common.ArrBlueValueString");
        var white = ReadIntArray(parameters, "Common.ArrWhiteValueString");
        var operators = ReadIntArray(parameters, "Common.ArrCalculationString");
        var positions = ReadIntArray(parameters, "Common.ArrLightPositionString");
        var declaredCount = ReadInt(parameters, "Common.LightCnt", 0, 0, MaximumUserCellCount);
        var arrayCount = new[] { red.Count, green.Count, blue.Count, white.Count, operators.Count, positions.Count }.Max();
        var count = Net48Compat.Clamp(Math.Max(declaredCount, arrayCount), 0, MaximumUserCellCount);

        var cells = new List<AlgorithmLightCell>();
        for (var index = 0; index < count; index++)
        {
            var position = ReadArrayValue(positions, index, TopLight);
            var cell = CreateDefaultUserCell(position, ReadArrayValue(operators, index, 0));
            cell.RedValue = NormalizeStoredChannel(ReadArrayValue(red, index, cell.RedValue));
            cell.GreenValue = NormalizeStoredChannel(ReadArrayValue(green, index, cell.GreenValue));
            cell.BlueValue = NormalizeStoredChannel(ReadArrayValue(blue, index, cell.BlueValue));
            cell.WhiteValue = NormalizeStoredChannel(ReadArrayValue(white, index, cell.WhiteValue));
            cells.Add(NormalizeUserCell(cell));
        }

        return cells;
    }

    private void ApplyReferenceFallbacks(AlgorithmLightState state)
    {
        state.LightType = Net48Compat.Clamp(state.LightType, TopLight, ThreeDLight);
        ClampNormalChannels(state);

        if (state.LightType == UserLight && state.UserCells.Count == 0)
        {
            state.UserCells.Add(CreateDefaultUserCell());
        }

        if (state.LightType != UserLight
            && state.RedValue == 0
            && state.GreenValue == 0
            && state.BlueValue == 0
            && state.WhiteValue == 0)
        {
            state.RedValue = 100;
        }

        var availability = GetChannelAvailability(state.LightType, userCellPosition: false);
        if (!availability.Green)
        {
            state.GreenValue = 0;
        }

        if (!availability.White)
        {
            state.WhiteValue = 0;
        }

        if ((state.LightType == MiddleLight || state.LightType == BottomLight)
            && state.RedValue == 0
            && state.BlueValue == 0)
        {
            state.RedValue = 100;
        }
    }

    private void ClampNormalChannels(AlgorithmLightState state)
    {
        state.RedValue = Net48Compat.Clamp(state.RedValue, 0, MaximumChannelValue);
        state.GreenValue = Net48Compat.Clamp(state.GreenValue, 0, MaximumChannelValue);
        state.BlueValue = Net48Compat.Clamp(state.BlueValue, 0, MaximumChannelValue);
        state.WhiteValue = Net48Compat.Clamp(state.WhiteValue, 0, MaximumChannelValue);
    }

    private AlgorithmLightCell NormalizeUserCell(AlgorithmLightCell cell)
    {
        cell.Position = Net48Compat.Clamp(cell.Position, TopLight, BottomLight);
        cell.Operator = Net48Compat.Clamp(cell.Operator, 0, 2);
        cell.RedValue = NormalizeStoredChannel(cell.RedValue);
        cell.GreenValue = NormalizeStoredChannel(cell.GreenValue);
        cell.BlueValue = NormalizeStoredChannel(cell.BlueValue);
        cell.WhiteValue = NormalizeStoredChannel(cell.WhiteValue);

        var availability = GetChannelAvailability(cell.Position, userCellPosition: true);
        if (!availability.Green)
        {
            cell.GreenValue = -1;
        }

        if (!availability.White)
        {
            cell.WhiteValue = -1;
        }

        if (cell.RedValue <= 0
            && cell.BlueValue <= 0
            && (cell.Position == MiddleLight || cell.Position == BottomLight))
        {
            cell.RedValue = 100;
        }

        if (cell.RedValue <= 0 && cell.GreenValue <= 0 && cell.BlueValue <= 0 && cell.WhiteValue <= 0)
        {
            cell.RedValue = 100;
        }

        return cell;
    }

    private void ApplyUserPositionDefaults(AlgorithmLightCell cell)
    {
        cell.RedValue = 100;
        cell.BlueValue = 0;
        cell.GreenValue = cell.Position == TopLight ? 0 : -1;
        cell.WhiteValue = cell.Position == TopLight ? 0 : -1;
    }

    private static AlgorithmLightState Clone(AlgorithmLightState state)
    {
        return new AlgorithmLightState
        {
            LightType = state.LightType,
            RedValue = state.RedValue,
            GreenValue = state.GreenValue,
            BlueValue = state.BlueValue,
            WhiteValue = state.WhiteValue,
            UserCells = state.UserCells
                .Select(cell => new AlgorithmLightCell
                {
                    Position = cell.Position,
                    Operator = cell.Operator,
                    RedValue = cell.RedValue,
                    GreenValue = cell.GreenValue,
                    BlueValue = cell.BlueValue,
                    WhiteValue = cell.WhiteValue
                })
                .ToList()
        };
    }

    private AlgorithmLightCell GetNormalDefaults(int lightType)
    {
        return lightType switch
        {
            SideGreenLight => new AlgorithmLightCell { RedValue = 0, GreenValue = 100, BlueValue = 0, WhiteValue = 0 },
            SideBlueLight => new AlgorithmLightCell { RedValue = 0, GreenValue = 0, BlueValue = 100, WhiteValue = 0 },
            ThreeDLight => new AlgorithmLightCell { RedValue = 0, GreenValue = 0, BlueValue = 0, WhiteValue = 0 },
            _ => new AlgorithmLightCell { RedValue = 100, GreenValue = 0, BlueValue = 0, WhiteValue = 0 }
        };
    }

    private int ReadInt(Dictionary<string, string> parameters, string key, int fallback, int min, int max)
    {
        var raw = AlgorithmParameterStore.GetValue(parameters, key, fallback.ToString(CultureInfo.InvariantCulture));
        if (!int.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var value))
        {
            value = fallback;
        }

        return Net48Compat.Clamp(value, min, max);
    }

    private static List<int> ReadIntArray(Dictionary<string, string> parameters, string key)
    {
        var raw = AlgorithmParameterStore.GetValue(parameters, key, "");
        if (string.IsNullOrWhiteSpace(raw))
        {
            return [];
        }

        return raw
            .Split(new[] { '|', ',', ';' }, StringSplitOptions.RemoveEmptyEntries)
            .Select(token => token.Trim())
            .Where(token => token.Length > 0)
            .Select(token => int.TryParse(token, NumberStyles.Integer, CultureInfo.InvariantCulture, out var value) ? value : 0)
            .ToList();
    }

    private static int ReadArrayValue(IReadOnlyList<int> values, int index, int fallback)
    {
        return index >= 0 && index < values.Count ? values[index] : fallback;
    }

    private int NormalizeStoredChannel(int value)
    {
        return value < 0 ? -1 : Net48Compat.Clamp(value, 0, MaximumChannelValue);
    }

    private static string Join(IEnumerable<int> values)
    {
        return string.Join("|", values.Select(FormatInt));
    }

    private static string FormatInt(int value)
    {
        return value.ToString(CultureInfo.InvariantCulture);
    }
}
