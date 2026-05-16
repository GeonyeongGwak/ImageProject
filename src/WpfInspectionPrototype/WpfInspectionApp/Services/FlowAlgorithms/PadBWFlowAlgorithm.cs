using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Interop;

namespace WpfInspectionApp.Services.FlowAlgorithms;

public sealed class PadBWParameters : ObservableObject, IFlowAlgorithmParameters
{
    private int _binaryMin = 125;
    private int _binaryMax = 255;
    private bool _useTeachArea;
    private double _teachAreaRateMin = 80;
    private double _teachAreaRateMax = 120;
    private bool _useFillHole;
    private int _filterLevel;

    public int BinaryMin { get => _binaryMin; set => SetProperty(ref _binaryMin, value); }
    public int BinaryMax { get => _binaryMax; set => SetProperty(ref _binaryMax, value); }
    public bool UseTeachArea { get => _useTeachArea; set => SetProperty(ref _useTeachArea, value); }
    public double TeachAreaRateMin { get => _teachAreaRateMin; set => SetProperty(ref _teachAreaRateMin, value); }
    public double TeachAreaRateMax { get => _teachAreaRateMax; set => SetProperty(ref _teachAreaRateMax, value); }
    public bool UseFillHole { get => _useFillHole; set => SetProperty(ref _useFillHole, value); }
    public int FilterLevel { get => _filterLevel; set => SetProperty(ref _filterLevel, value); }
}

public sealed class PadBWFlowAlgorithm : IFlowAlgorithm
{
    public string DisplayName => "PadBW";
    public int AlgoType => MptiFlowNativeBridge.EALGO_PADBW;
    // PadBW typically lives in eINSP_PAD windows in the reference framework.
    public int InspType => MptiFlowNativeBridge.EINSP_PAD;
    public IFlowAlgorithmParameters CreateParameters() => new PadBWParameters();

    public void ApplyParams(FlowAlgorithmSlot slot, IFlowAlgorithmParameters parameters)
    {
        var pp = (PadBWParameters)parameters;
        var p = new MptiBridgeFlowPadBWParams
        {
            BinaryMin = pp.BinaryMin, BinaryMax = pp.BinaryMax, UseInsp2D = 1,
            UseTeachArea = pp.UseTeachArea ? 1 : 0,
            TeachAreaRateMin = pp.TeachAreaRateMin, TeachAreaRateMax = pp.TeachAreaRateMax,
            UseShift = 1, TeachShiftX = 0, TeachShiftY = 0,
            UseBlobWidth = 1, UseBlobLength = 1, UseBlobArea = 1,
            FilterLevel = pp.FilterLevel,
            UseFillHole = pp.UseFillHole ? 1 : 0,
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
