using WpfInspectionApp.Interop;

namespace WpfInspectionApp.Services.FlowAlgorithms;

public sealed class PadBWFlowAlgorithm : IFlowAlgorithm
{
    public string DisplayName => "PadBW";
    public int AlgoType => MptiFlowNativeBridge.EALGO_PADBW;
    // PadBW typically lives in eINSP_PAD windows in the reference framework.
    public int InspType => MptiFlowNativeBridge.EINSP_PAD;

    public void ApplyParams(FlowAlgorithmSlot slot)
    {
        var p = new MptiBridgeFlowPadBWParams
        {
            BinaryMin = 125, BinaryMax = 255, UseInsp2D = 1,
            UseTeachArea = 0, TeachAreaRateMin = 80, TeachAreaRateMax = 120,
            UseShift = 1, TeachShiftX = 0, TeachShiftY = 0,
            UseBlobWidth = 1, UseBlobLength = 1, UseBlobArea = 1,
            FilterLevel = 0, UseFillHole = 0,
        };
        MptiFlowNativeBridge.MptiBridgeSetAlgoParamsPadBW(slot.WndIdx, slot.AlgoIdx, ref p);
    }

    public FlowAlgorithmResult ReadResult(FlowAlgorithmSlot slot)
    {
        var r = new MptiBridgeFlowPadBWResult();
        int code = MptiFlowNativeBridge.MptiBridgeResultPadBW(slot.WndType, slot.WndIdx, slot.AlgoIdx, ref r);
        var fields = new Dictionary<string, string>
        {
            ["okArea"]       = r.OkArea.ToString(),
            ["okShape"]      = r.OkShapeArea.ToString(),
            ["okWidth"]      = r.OkWidth.ToString(),
            ["okLength"]     = r.OkLength.ToString(),
            ["arrShapeCnt"]  = r.ArrShapeRectCnt.ToString(),
            ["arrRectCnt"]   = r.ArrRectCnt.ToString(),
        };
        string summary = code == 0
            ? $"okArea={r.OkArea} okShape={r.OkShapeArea} ngRects={r.ArrRectCnt}"
            : $"ResultPadBW code={code}";
        return new FlowAlgorithmResult(
            Success: code == 0 && r.Hdr.AlgoIsInsp != 0,
            ReaderCode: code, IsInsp: r.Hdr.AlgoIsInsp != 0, IsOk: r.Hdr.AlgoIsOk != 0,
            DefectCode: r.Hdr.AlgoDefectCode, Summary: summary, Fields: fields);
    }
}
