using WpfInspectionApp.Interop;

namespace WpfInspectionApp.Services.FlowAlgorithms;

public sealed class BlobFlowAlgorithm : IFlowAlgorithm
{
    public string DisplayName => "Blob";
    public int AlgoType => MptiFlowNativeBridge.EALGO_BLOB;
    public int InspType => MptiFlowNativeBridge.EINSP_MOUNT;

    public void ApplyParams(FlowAlgorithmSlot slot)
    {
        var p = new MptiBridgeFlowBlobParams
        {
            UseInsp2D = 1, MinBinary = 100, MaxBinary = 255, TypeRange2D = 0,
            UseArea = 1, AreaMin = 10, AreaMax = 1_000_000,
            UseShift = 1, ShiftX = 10, ShiftY = 10,
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
