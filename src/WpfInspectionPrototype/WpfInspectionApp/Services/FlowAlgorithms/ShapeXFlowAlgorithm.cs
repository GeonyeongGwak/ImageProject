using WpfInspectionApp.Interop;

namespace WpfInspectionApp.Services.FlowAlgorithms;

public sealed class ShapeXFlowAlgorithm : IFlowAlgorithm
{
    public string DisplayName => "ShapeX";
    public int AlgoType => MptiFlowNativeBridge.EALGO_SHAPEX;
    public int InspType => MptiFlowNativeBridge.EINSP_MOUNT;

    public void ApplyParams(FlowAlgorithmSlot slot)
    {
        var p = new MptiBridgeFlowShapeXParams
        {
            PartAngle = 0, LeadTipDirection = 0, RoiCnt = 0,
            MatchScore = 700, Hist1 = 0, Hist1Low = 0, Hist1Up = 0,
            Hist2 = 0, Hist2Low = 0, Hist2Up = 0,
            InspOption = 0,
            AspectRatio = 0, MinScarThickness = 0,
            VerticalMaxLength = 0, HorizontalMaxLength = 0,
            MaxNgArea = 0, CriticalArea = 0, MaxChippingLength = 0,
            StartIdx = 0, TieBarRate = 0.6f, ContrastValue = 0,
        };
        MptiFlowNativeBridge.MptiBridgeSetAlgoParamsShapeX(slot.WndIdx, slot.AlgoIdx, ref p);
    }

    public FlowAlgorithmResult ReadResult(FlowAlgorithmSlot slot)
    {
        var r = new MptiBridgeFlowShapeXResult();
        int code = MptiFlowNativeBridge.MptiBridgeResultShapeX(slot.WndType, slot.WndIdx, slot.AlgoIdx, ref r);
        var fields = new Dictionary<string, string>
        {
            ["roiCnt"]     = r.NRoiCnt.ToString(),
            ["ngArea"]     = r.NNgAreaRoiCnt.ToString(),
            ["shapeNg"]    = r.NShapeNgCnt.ToString(),
            ["wrForeign"]  = r.RstWrForeignCnt.ToString(),
            ["aiOk"]       = r.BAiOk.ToString(),
            ["aiScore"]    = r.StdAiScore.ToString("F3"),
        };
        string summary = code == 0
            ? $"roi={r.NRoiCnt} ngArea={r.NNgAreaRoiCnt} shapeNg={r.NShapeNgCnt} aiOk={r.BAiOk}"
            : $"ResultShapeX code={code}";
        return new FlowAlgorithmResult(
            Success: code == 0 && r.Hdr.AlgoIsInsp != 0,
            ReaderCode: code, IsInsp: r.Hdr.AlgoIsInsp != 0, IsOk: r.Hdr.AlgoIsOk != 0,
            DefectCode: r.Hdr.AlgoDefectCode, Summary: summary, Fields: fields);
    }
}
