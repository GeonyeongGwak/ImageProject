using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Interop;

namespace WpfInspectionApp.Services.FlowAlgorithms;

// Per-algorithm parameter VM. Bound by AlignParametersTemplate in
// FlowAlgorithmTemplates.xaml (matched via DataType={x:Type AlignParameters}). The
// runner reuses the same instance across Run clicks so user-tuned values persist.
public sealed class AlignParameters : ObservableObject, IFlowAlgorithmParameters
{
    private int _minBinary = 100;
    private int _maxBinary = 255;
    private int _searchSize = 80;
    private double _maxShiftX = 20;
    private double _maxShiftY = 20;
    private double _maxAngle = 5;
    private int _minBlobArea = 10;

    public int MinBinary  { get => _minBinary;  set => SetProperty(ref _minBinary,  value); }
    public int MaxBinary  { get => _maxBinary;  set => SetProperty(ref _maxBinary,  value); }
    public int SearchSize { get => _searchSize; set => SetProperty(ref _searchSize, value); }
    public double MaxShiftX { get => _maxShiftX; set => SetProperty(ref _maxShiftX, value); }
    public double MaxShiftY { get => _maxShiftY; set => SetProperty(ref _maxShiftY, value); }
    public double MaxAngle  { get => _maxAngle;  set => SetProperty(ref _maxAngle,  value); }
    public int MinBlobArea  { get => _minBlobArea; set => SetProperty(ref _minBlobArea, value); }
}

// Align algorithm — pattern-detected anchor windows used to compute (offsetX, offsetY,
// theta) for a part. Uses 4 search points by default at the part quadrants; the runner
// passes part dimensions via the AlignContext singleton (filled in BeginPart).
public sealed class AlignFlowAlgorithm : IFlowAlgorithm
{
    public string DisplayName => "Align";
    public int AlgoType => MptiFlowNativeBridge.EALGO_ALIGN;
    public int InspType => MptiFlowNativeBridge.EINSP_ALIGN;
    public IFlowAlgorithmParameters CreateParameters() => new AlignParameters();

    public void ApplyParams(FlowAlgorithmSlot slot, IFlowAlgorithmParameters parameters)
    {
        var pp = (AlignParameters)parameters;
        var ctx = FlowAlgorithmContext.Current;
        int qx = ctx.PartWidth / 4;
        int qy = ctx.PartHeight / 4;
        int sz = Math.Max(8, pp.SearchSize);
        var p = new MptiBridgeFlowAlignParams
        {
            SearchNum = 4,
            SearchPointsX = new[] { qx, ctx.PartWidth - qx, qx, ctx.PartWidth - qx },
            SearchPointsY = new[] { qy, qy, ctx.PartHeight - qy, ctx.PartHeight - qy },
            SearchSizeW = new[] { sz, sz, sz, sz },
            SearchSizeH = new[] { sz, sz, sz, sz },
            SearchMargin = 10,
            MinBinary = pp.MinBinary,
            MaxBinary = pp.MaxBinary,
            UseInsp2D = 1,
            UseShift = 1,
            MaxShiftX = pp.MaxShiftX,
            MaxShiftY = pp.MaxShiftY,
            UseAngle = 1,
            MaxAngle = pp.MaxAngle,
            SameSize = 1,
            MinBlobArea = Math.Max(1, pp.MinBlobArea),
        };
        MptiFlowNativeBridge.MptiBridgeSetAlgoParamsAlign(slot.WndIdx, slot.AlgoIdx, ref p);
    }

    public FlowAlgorithmResult ReadResult(FlowAlgorithmSlot slot)
    {
        var r = new MptiBridgeFlowAlignResult
        {
            DetectedCentersX = new int[4],
            DetectedCentersY = new int[4]
        };
        int code = MptiFlowNativeBridge.MptiBridgeResultAlign(0, ref r);
        var fields = new Dictionary<string, string>
        {
            ["okCount"] = r.OkCount.ToString(),
            ["offsetX"] = r.OffsetX.ToString("F2"),
            ["offsetY"] = r.OffsetY.ToString("F2"),
            ["theta"]   = r.Theta.ToString("F3"),
            ["okShift"] = $"({r.OkShiftX},{r.OkShiftY})",
            ["okAngle"] = r.OkAngle.ToString(),
        };
        string summary = code == 0
            ? $"offset=({r.OffsetX:F2},{r.OffsetY:F2}) theta={r.Theta:F3}° okCount={r.OkCount}"
            : $"ResultAlign code={code}";
        return new FlowAlgorithmResult(
            Success: code == 0 && r.IsInsp != 0,
            ReaderCode: code, IsInsp: r.IsInsp != 0, IsOk: r.IsOk != 0,
            DefectCode: r.DefectCode, Summary: summary, Fields: fields);
    }
}

// Shared per-flow context populated by FlowAlgorithmRunner before ApplyParams runs.
// Algorithms read this when they need part dimensions for default search points etc.
public sealed class FlowAlgorithmContext
{
    public int PartWidth { get; init; }
    public int PartHeight { get; init; }

    // AsyncLocal so multiple concurrent flow runs on different threads don't collide.
    // FlowAlgorithmRunner sets this before invoking ApplyParams/ReadResult.
    private static readonly System.Threading.AsyncLocal<FlowAlgorithmContext?> _current = new();
    public static FlowAlgorithmContext Current => _current.Value
        ?? throw new InvalidOperationException("FlowAlgorithmContext not set — call inside FlowAlgorithmRunner.RunAsync.");
    internal static IDisposable Push(FlowAlgorithmContext ctx)
    {
        var prev = _current.Value;
        _current.Value = ctx;
        return new Popper(prev);
    }
    private sealed class Popper : IDisposable
    {
        private readonly FlowAlgorithmContext? _prev;
        public Popper(FlowAlgorithmContext? prev) { _prev = prev; }
        public void Dispose() { _current.Value = _prev; }
    }
}
