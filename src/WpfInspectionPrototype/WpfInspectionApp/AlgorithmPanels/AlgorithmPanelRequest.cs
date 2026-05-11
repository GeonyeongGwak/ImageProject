namespace WpfInspectionApp.AlgorithmPanels;

public enum AlgorithmPanelRequestKind
{
    PreviewUpdate,
    TreeRefresh,
    WindowRoiDrawing,
    AlgorithmRoiDrawing,
    SetParameter
}

public sealed class AlgorithmPanelRequest
{
    private AlgorithmPanelRequest(AlgorithmPanelRequestKind kind, string? parameterName = null, string? parameterValue = null)
    {
        Kind = kind;
        ParameterName = parameterName;
        ParameterValue = parameterValue;
    }

    public AlgorithmPanelRequestKind Kind { get; }
    public string? ParameterName { get; }
    public string? ParameterValue { get; }

    public static AlgorithmPanelRequest PreviewUpdate()
    {
        return new AlgorithmPanelRequest(AlgorithmPanelRequestKind.PreviewUpdate);
    }

    public static AlgorithmPanelRequest TreeRefresh()
    {
        return new AlgorithmPanelRequest(AlgorithmPanelRequestKind.TreeRefresh);
    }

    public static AlgorithmPanelRequest WindowRoiDrawing()
    {
        return new AlgorithmPanelRequest(AlgorithmPanelRequestKind.WindowRoiDrawing);
    }

    public static AlgorithmPanelRequest AlgorithmRoiDrawing()
    {
        return new AlgorithmPanelRequest(AlgorithmPanelRequestKind.AlgorithmRoiDrawing);
    }

    public static AlgorithmPanelRequest SetParameter(string name, string value)
    {
        return new AlgorithmPanelRequest(AlgorithmPanelRequestKind.SetParameter, name, value);
    }
}
