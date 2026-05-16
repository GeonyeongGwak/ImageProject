using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Interop;

namespace WpfInspectionApp.Services.FlowAlgorithms;

public sealed class BGAParameters : ObservableObject, IFlowAlgorithmParameters
{
    private int _minBinary = 100;
    private int _maxBinary = 255;
    private double _teachArea = 100;
    private double _teachVolume;
    private bool _useCircleRate;
    private double _teachCircleRate = 80;

    public int MinBinary { get => _minBinary; set => SetProperty(ref _minBinary, value); }
    public int MaxBinary { get => _maxBinary; set => SetProperty(ref _maxBinary, value); }
    public double TeachArea { get => _teachArea; set => SetProperty(ref _teachArea, value); }
    public double TeachVolume { get => _teachVolume; set => SetProperty(ref _teachVolume, value); }
    public bool UseCircleRate { get => _useCircleRate; set => SetProperty(ref _useCircleRate, value); }
    public double TeachCircleRate { get => _teachCircleRate; set => SetProperty(ref _teachCircleRate, value); }
}

public sealed class BGAFlowAlgorithm : IFlowAlgorithm
{
    public string DisplayName => "BGA";
    public int AlgoType => MptiFlowNativeBridge.EALGO_BGA;
    public int InspType => MptiFlowNativeBridge.EINSP_BGA;
    public IFlowAlgorithmParameters CreateParameters() => new BGAParameters();

    public void ApplyParams(FlowAlgorithmSlot slot, IFlowAlgorithmParameters parameters)
    {
        var pp = (BGAParameters)parameters;
        var p = new MptiBridgeFlowBGAParams
        {
            UseInsp2D = 1, MinBinary = pp.MinBinary, MaxBinary = pp.MaxBinary, TypeRange2D = 0,
            UseArea = 1, AreaMin = 10, AreaMax = 1_000_000,
            TeachArea = pp.TeachArea, TeachVolume = pp.TeachVolume,
            UseShift = 1, ShiftX = 10, ShiftY = 10,
            UseTeachWidth = 0, TeachWidthRateMin = 80, TeachWidthRateMax = 120,
            UseTeachLength = 0, TeachLengthRateMin = 80, TeachLengthRateMax = 120,
            UseCircleRate = pp.UseCircleRate ? 1 : 0, TeachCircleRate = pp.TeachCircleRate,
        };
        MptiFlowNativeBridge.MptiBridgeSetAlgoParamsBGA(slot.WndIdx, slot.AlgoIdx, ref p);
    }

    public FlowAlgorithmResult ReadResult(FlowAlgorithmSlot slot)
    {
        var r = new MptiBridgeFlowBGAResult();
        int code = MptiFlowNativeBridge.MptiBridgeResultBGA(slot.WndType, slot.WndIdx, slot.AlgoIdx, ref r);
        var fields = new Dictionary<string, string>
        {
            ["coplanarity"] = r.Coplanarity.ToString("F3"),
            ["gridOffset"]  = $"({r.GridOffsetX:F2},{r.GridOffsetY:F2})",
            ["twist"]       = r.Twist.ToString("F3"),
            ["okCoplan"]    = r.OkCoplanarity.ToString(),
            ["okTwist"]     = r.OkTwist.ToString(),
        };
        string summary = code == 0
            ? $"coplan={r.Coplanarity:F3} twist={r.Twist:F3} okCoplan={r.OkCoplanarity}"
            : $"ResultBGA code={code}";
        return new FlowAlgorithmResult(
            Success: code == 0 && r.Hdr.AlgoIsInsp != 0,
            ReaderCode: code, IsInsp: r.Hdr.AlgoIsInsp != 0, IsOk: r.Hdr.AlgoIsOk != 0,
            DefectCode: r.Hdr.AlgoDefectCode, Summary: summary, Fields: fields);
    }
}
