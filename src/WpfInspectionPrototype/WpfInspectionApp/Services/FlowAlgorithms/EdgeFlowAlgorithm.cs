using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Interop;

namespace WpfInspectionApp.Services.FlowAlgorithms;

public sealed class EdgeParameters : ObservableObject, IFlowAlgorithmParameters
{
    private int _minBinary = 100;
    private int _maxBinary = 255;
    private int _setLineCnt = 1;
    private double _lineFindRate = 0.5;
    private bool _useAngle;
    private double _teachRotate;

    public int MinBinary { get => _minBinary; set => SetProperty(ref _minBinary, value); }
    public int MaxBinary { get => _maxBinary; set => SetProperty(ref _maxBinary, value); }
    public int SetLineCnt { get => _setLineCnt; set => SetProperty(ref _setLineCnt, value); }
    public double LineFindRate { get => _lineFindRate; set => SetProperty(ref _lineFindRate, value); }
    public bool UseAngle { get => _useAngle; set => SetProperty(ref _useAngle, value); }
    public double TeachRotate { get => _teachRotate; set => SetProperty(ref _teachRotate, value); }
}

public sealed class EdgeFlowAlgorithm : IFlowAlgorithm
{
    public string DisplayName => "Edge";
    public int AlgoType => MptiFlowNativeBridge.EALGO_EDGE;
    public int InspType => MptiFlowNativeBridge.EINSP_MOUNT;
    public IFlowAlgorithmParameters CreateParameters() => new EdgeParameters();

    public void ApplyParams(FlowAlgorithmSlot slot, IFlowAlgorithmParameters parameters)
    {
        var pp = (EdgeParameters)parameters;
        var p = new MptiBridgeFlowEdgeParams
        {
            UseInsp2D = 1, MinBinary = pp.MinBinary, MaxBinary = pp.MaxBinary, TypeRange2D = 0,
            UseShift = 1, ShiftX = 10, ShiftY = 10,
            UseArea = 0, AreaMin = 10, AreaMax = 1_000_000,
            SetLineCnt = pp.SetLineCnt, UseGroup = 0,
            LineFindType = 0, LineFindRate = pp.LineFindRate,
            UseAngle = pp.UseAngle ? 1 : 0, TeachRotate = pp.TeachRotate,
            UseDistanceX = 0, UseDistanceY = 0,
        };
        MptiFlowNativeBridge.MptiBridgeSetAlgoParamsEdge(slot.WndIdx, slot.AlgoIdx, ref p);
    }

    public FlowAlgorithmResult ReadResult(FlowAlgorithmSlot slot)
    {
        var r = new MptiBridgeFlowEdgeResult();
        int code = MptiFlowNativeBridge.MptiBridgeResultEdge(slot.WndType, slot.WndIdx, slot.AlgoIdx, ref r);
        var fields = new Dictionary<string, string>
        {
            ["shift"]    = $"({r.RstShiftX:F1},{r.RstShiftY:F1})",
            ["angle"]    = r.RstAngle.ToString("F3"),
            ["length0"]  = r.RstLength0.ToString("F1"),
            ["distance"] = $"({r.RstDistanceX:F1},{r.RstDistanceY:F1})",
            ["missing"]  = r.Missing.ToString(),
        };
        string summary = code == 0
            ? $"shift=({r.RstShiftX:F1},{r.RstShiftY:F1}) angle={r.RstAngle:F3}° missing={r.Missing}"
            : $"ResultEdge code={code}";
        return new FlowAlgorithmResult(
            Success: code == 0 && r.Hdr.AlgoIsInsp != 0,
            ReaderCode: code, IsInsp: r.Hdr.AlgoIsInsp != 0, IsOk: r.Hdr.AlgoIsOk != 0,
            DefectCode: r.Hdr.AlgoDefectCode, Summary: summary, Fields: fields);
    }
}
