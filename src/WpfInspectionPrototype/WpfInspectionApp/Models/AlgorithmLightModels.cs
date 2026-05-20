namespace WpfInspectionApp.Models;

public sealed class AlgorithmLightState
{
    public int LightType { get; set; }
    public int RedValue { get; set; } = 100;
    public int GreenValue { get; set; }
    public int BlueValue { get; set; }
    public int WhiteValue { get; set; }
    public List<AlgorithmLightCell> UserCells { get; set; } = [];
}

public sealed class AlgorithmLightCell
{
    public int Position { get; set; }
    public int Operator { get; set; }
    public int RedValue { get; set; } = 100;
    public int GreenValue { get; set; }
    public int BlueValue { get; set; }
    public int WhiteValue { get; set; }
}

public readonly record struct LightChannelAvailability(
    bool Red,
    bool Green,
    bool Blue,
    bool White);
