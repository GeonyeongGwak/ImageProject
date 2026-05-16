using WpfInspectionApp.Interop;

namespace WpfInspectionApp.Services.FlowAlgorithms;

public sealed class BGAFlowAlgorithm : IFlowAlgorithm
{
    public string DisplayName => "BGA";
    public int AlgoType => MptiFlowNativeBridge.EALGO_BGA;
    public int InspType => MptiFlowNativeBridge.EINSP_BGA;

    public void ApplyParams(FlowAlgorithmSlot slot)
    {
        var p = new MptiBridgeFlowBGAParams
        {
            UseInsp2D = 1, MinBinary = 100, MaxBinary = 255, TypeRange2D = 0,
            UseArea = 1, AreaMin = 10, AreaMax = 1_000_000,
            TeachArea = 100, TeachVolume = 0,
            UseShift = 1, ShiftX = 10, ShiftY = 10,
            UseTeachWidth = 0, TeachWidthRateMin = 80, TeachWidthRateMax = 120,
            UseTeachLength = 0, TeachLengthRateMin = 80, TeachLengthRateMax = 120,
            UseCircleRate = 0, TeachCircleRate = 80,
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
