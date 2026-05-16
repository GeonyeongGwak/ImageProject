using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Interop;

namespace WpfInspectionApp.Services.FlowAlgorithms;

public sealed class BlobParameters : ObservableObject, IFlowAlgorithmParameters
{
    private int _minBinary = 100;
    private int _maxBinary = 255;
    private bool _invertCheck;
    private double _areaMin = 10;
    private double _areaMax = 1_000_000;
    private double _shiftX = 10;
    private double _shiftY = 10;

    public int MinBinary { get => _minBinary; set => SetProperty(ref _minBinary, value); }
    public int MaxBinary { get => _maxBinary; set => SetProperty(ref _maxBinary, value); }
    public bool InvertCheck { get => _invertCheck; set => SetProperty(ref _invertCheck, value); }
    public double AreaMin { get => _areaMin; set => SetProperty(ref _areaMin, value); }
    public double AreaMax { get => _areaMax; set => SetProperty(ref _areaMax, value); }
    public double ShiftX  { get => _shiftX;  set => SetProperty(ref _shiftX,  value); }
    public double ShiftY  { get => _shiftY;  set => SetProperty(ref _shiftY,  value); }
}

public sealed class BlobFlowAlgorithm : IFlowAlgorithm
{
    public string DisplayName => "Blob";
    public int AlgoType => MptiFlowNativeBridge.EALGO_BLOB;
    public int InspType => MptiFlowNativeBridge.EINSP_MOUNT;
    public IFlowAlgorithmParameters CreateParameters() => new BlobParameters();

    public void ApplyParams(FlowAlgorithmSlot slot, IFlowAlgorithmParameters parameters)
    {
        var pp = (BlobParameters)parameters;
        var p = new MptiBridgeFlowBlobParams
        {
            UseInsp2D = 1, MinBinary = pp.MinBinary, MaxBinary = pp.MaxBinary, TypeRange2D = 0,
            InvertCheck = pp.InvertCheck ? 1 : 0,
            UseArea = 1, AreaMin = pp.AreaMin, AreaMax = pp.AreaMax,
            UseShift = 1, ShiftX = pp.ShiftX, ShiftY = pp.ShiftY,
            UseTeachWidth = 0, TeachWidthRateMin = 80, TeachWidthRateMax = 120,
            UseTeachLength = 0, TeachLengthRateMin = 80, TeachLengthRateMax = 120,
        };
        MptiFlowNativeBridge.MptiBridgeSetAlgoParamsBlob(slot.WndIdx, slot.AlgoIdx, ref p);
    }

    public FlowAlgorithmResult ReadResult(FlowAlgorithmSlot slot)
    {
        var r = new MptiBridgeFlowBlobResult();
        int code = MptiFlowNativeBridge.MptiBridgeResultBlob(slot.WndType, slot.WndIdx, slot.AlgoIdx, ref r);
        var fields = new Dictionary<string, string>
        {
            ["area"]    = r.RstArea.ToString("F1"),
            ["areaRate"] = r.RstAreaRate.ToString("F2"),
            ["shift"]   = $"({r.RstShiftX:F1},{r.RstShiftY:F1})",
            ["size"]    = $"{r.RstWidth:F0}x{r.RstLength:F0}",
            ["okArea"]  = r.OkArea.ToString(),
            ["rectCnt"] = r.ArrRectCnt.ToString(),
        };
        string summary = code == 0
            ? $"area={r.RstArea:F1} rate={r.RstAreaRate:F2} ok={r.OkArea}"
            : $"ResultBlob code={code}";
        return new FlowAlgorithmResult(
            Success: code == 0 && r.Hdr.AlgoIsInsp != 0,
            ReaderCode: code, IsInsp: r.Hdr.AlgoIsInsp != 0, IsOk: r.Hdr.AlgoIsOk != 0,
            DefectCode: r.Hdr.AlgoDefectCode, Summary: summary, Fields: fields);
    }
}
