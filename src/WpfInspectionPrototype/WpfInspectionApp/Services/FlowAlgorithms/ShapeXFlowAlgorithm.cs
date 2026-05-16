using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Interop;

namespace WpfInspectionApp.Services.FlowAlgorithms;

public sealed class ShapeXParameters : ObservableObject, IFlowAlgorithmParameters
{
    private short _matchScore = 700;
    private float _aspectRatio;
    private float _maxNgArea;
    private float _criticalArea;
    private float _tieBarRate = 0.6f;
    private int _contrastValue;

    public short MatchScore { get => _matchScore; set => SetProperty(ref _matchScore, value); }
    public float AspectRatio { get => _aspectRatio; set => SetProperty(ref _aspectRatio, value); }
    public float MaxNgArea { get => _maxNgArea; set => SetProperty(ref _maxNgArea, value); }
    public float CriticalArea { get => _criticalArea; set => SetProperty(ref _criticalArea, value); }
    public float TieBarRate { get => _tieBarRate; set => SetProperty(ref _tieBarRate, value); }
    public int ContrastValue { get => _contrastValue; set => SetProperty(ref _contrastValue, value); }
}

public sealed class ShapeXFlowAlgorithm : IFlowAlgorithm
{
    public string DisplayName => "ShapeX";
    public int AlgoType => MptiFlowNativeBridge.EALGO_SHAPEX;
    public int InspType => MptiFlowNativeBridge.EINSP_MOUNT;
    public IFlowAlgorithmParameters CreateParameters() => new ShapeXParameters();

    public void ApplyParams(FlowAlgorithmSlot slot, IFlowAlgorithmParameters parameters)
    {
        var pp = (ShapeXParameters)parameters;
        var p = new MptiBridgeFlowShapeXParams
        {
            PartAngle = 0, LeadTipDirection = 0, RoiCnt = 0,
            MatchScore = pp.MatchScore, Hist1 = 0, Hist1Low = 0, Hist1Up = 0,
            Hist2 = 0, Hist2Low = 0, Hist2Up = 0,
            InspOption = 0,
            AspectRatio = pp.AspectRatio, MinScarThickness = 0,
            VerticalMaxLength = 0, HorizontalMaxLength = 0,
            MaxNgArea = pp.MaxNgArea, CriticalArea = pp.CriticalArea, MaxChippingLength = 0,
            StartIdx = 0, TieBarRate = pp.TieBarRate, ContrastValue = pp.ContrastValue,
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
