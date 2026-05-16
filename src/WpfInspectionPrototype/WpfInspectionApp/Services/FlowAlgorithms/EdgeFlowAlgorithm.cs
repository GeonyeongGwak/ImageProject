using WpfInspectionApp.Interop;

namespace WpfInspectionApp.Services.FlowAlgorithms;

public sealed class EdgeFlowAlgorithm : IFlowAlgorithm
{
    public string DisplayName => "Edge";
    public int AlgoType => MptiFlowNativeBridge.EALGO_EDGE;
    public int InspType => MptiFlowNativeBridge.EINSP_MOUNT;

    public void ApplyParams(FlowAlgorithmSlot slot)
    {
        var p = new MptiBridgeFlowEdgeParams
        {
            UseInsp2D = 1, MinBinary = 100, MaxBinary = 255, TypeRange2D = 0,
            UseShift = 1, ShiftX = 10, ShiftY = 10,
            UseArea = 0, AreaMin = 10, AreaMax = 1_000_000,
            SetLineCnt = 1, UseGroup = 0,
            LineFindType = 0, LineFindRate = 0.5,
            UseAngle = 0, TeachRotate = 0,
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
