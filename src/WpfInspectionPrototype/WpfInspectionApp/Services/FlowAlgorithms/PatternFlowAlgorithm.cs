using WpfInspectionApp.Interop;

namespace WpfInspectionApp.Services.FlowAlgorithms;

public sealed class PatternFlowAlgorithm : IFlowAlgorithm
{
    public string DisplayName => "Pattern";
    public int AlgoType => MptiFlowNativeBridge.EALGO_PATTERN;
    public int InspType => MptiFlowNativeBridge.EINSP_MOUNT;

    public void ApplyParams(FlowAlgorithmSlot slot)
    {
        var p = new MptiBridgeFlowPatternParams
        {
            UsePolarity = 0, AcceptScore = 0.7,
            UseShift = 1, ShiftX = 10, ShiftY = 10, RangeAngle = 10,
            WndAngle = 0, SearchAngleRangeMin = -5, SearchAngleRangeMax = 5,
            SamplingAngle = 1, UseNgOpt = 0, UseCharacter = 0,
            ModelFilter = 0, CntPatternPath = 1,
            FactorRed = 1, FactorGreen = 1, FactorBlue = 1,
            // Model paths left empty → native keeps the default (no model file).
            // A real UI would pop a file picker and fill these.
            ModelPathInspect1 = string.Empty,
            ModelPathTeach    = string.Empty,
        };
        MptiFlowNativeBridge.MptiBridgeSetAlgoParamsPattern(slot.WndIdx, slot.AlgoIdx, ref p);
    }

    public FlowAlgorithmResult ReadResult(FlowAlgorithmSlot slot)
    {
        var r = new MptiBridgeFlowPatternResult();
        int code = MptiFlowNativeBridge.MptiBridgeResultPattern(slot.WndType, slot.WndIdx, slot.AlgoIdx, ref r);
        var fields = new Dictionary<string, string>
        {
            ["score"]   = r.Score.ToString("F3"),
            ["angle"]   = r.Angle.ToString("F3"),
            ["cog"]     = $"({r.CogX:F1},{r.CogY:F1})",
            ["offset"]  = $"({r.OffsetX:F1},{r.OffsetY:F1})",
            ["okFind"]  = r.OkFind.ToString(),
            ["okScore"] = r.OkScore.ToString(),
        };
        string summary = code == 0
            ? $"score={r.Score:F3} angle={r.Angle:F3}° okFind={r.OkFind} okScore={r.OkScore}"
            : $"ResultPattern code={code}";
        return new FlowAlgorithmResult(
            Success: code == 0 && r.Hdr.AlgoIsInsp != 0,
            ReaderCode: code, IsInsp: r.Hdr.AlgoIsInsp != 0, IsOk: r.Hdr.AlgoIsOk != 0,
            DefectCode: r.Hdr.AlgoDefectCode, Summary: summary, Fields: fields);
    }
}
