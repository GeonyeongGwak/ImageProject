using System.Windows.Input;
using WpfInspectionApp.Commands;
using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Interop;

namespace WpfInspectionApp.Services.FlowAlgorithms;

public sealed class PatternParameters : ObservableObject, IFlowAlgorithmParameters
{
    private bool _usePolarity;
    private double _acceptScore = 0.7;
    private double _rangeAngle = 10;
    private double _wndAngle;
    private double _searchAngleRangeMin = -5;
    private double _searchAngleRangeMax = 5;
    private string _modelPathInspect = string.Empty;

    public PatternParameters(Func<string?>? browseModelFile = null)
    {
        // BrowseCommand stays available even when the picker isn't wired (null
        // callback) — it just no-ops. XAML can bind unconditionally.
        BrowseModelCommand = new RelayCommand(() =>
        {
            var picked = browseModelFile?.Invoke();
            if (!string.IsNullOrWhiteSpace(picked))
                ModelPathInspect = picked!;
        });
    }

    public bool UsePolarity { get => _usePolarity; set => SetProperty(ref _usePolarity, value); }
    public double AcceptScore { get => _acceptScore; set => SetProperty(ref _acceptScore, value); }
    public double RangeAngle { get => _rangeAngle; set => SetProperty(ref _rangeAngle, value); }
    public double WndAngle { get => _wndAngle; set => SetProperty(ref _wndAngle, value); }
    public double SearchAngleRangeMin { get => _searchAngleRangeMin; set => SetProperty(ref _searchAngleRangeMin, value); }
    public double SearchAngleRangeMax { get => _searchAngleRangeMax; set => SetProperty(ref _searchAngleRangeMax, value); }
    public string ModelPathInspect { get => _modelPathInspect; set => SetProperty(ref _modelPathInspect, value ?? string.Empty); }
    public ICommand BrowseModelCommand { get; }
}

public sealed class PatternFlowAlgorithm : IFlowAlgorithm
{
    private readonly Services.IFileDialogService? _fileDialog;

    // File-dialog service is optional so other call sites (tests, harness) can use the
    // algorithm without WPF dependencies. When null, the Browse button no-ops.
    public PatternFlowAlgorithm(Services.IFileDialogService? fileDialog = null)
    {
        _fileDialog = fileDialog;
    }

    public string DisplayName => "Pattern";
    public int AlgoType => MptiFlowNativeBridge.EALGO_PATTERN;
    public int InspType => MptiFlowNativeBridge.EINSP_MOUNT;

    public IFlowAlgorithmParameters CreateParameters() =>
        new PatternParameters(BrowseModelFile);

    private string? BrowseModelFile()
    {
        if (_fileDialog == null) return null;
        // Application.Current.MainWindow is the dialog owner — set in App.xaml.cs.
        var owner = System.Windows.Application.Current?.MainWindow;
        return _fileDialog.BrowsePatternModel(owner!);
    }

    public void ApplyParams(FlowAlgorithmSlot slot, IFlowAlgorithmParameters parameters)
    {
        var pp = (PatternParameters)parameters;
        var p = new MptiBridgeFlowPatternParams
        {
            UsePolarity = pp.UsePolarity ? 1 : 0, AcceptScore = pp.AcceptScore,
            UseShift = 1, ShiftX = 10, ShiftY = 10, RangeAngle = pp.RangeAngle,
            WndAngle = pp.WndAngle,
            SearchAngleRangeMin = pp.SearchAngleRangeMin, SearchAngleRangeMax = pp.SearchAngleRangeMax,
            SamplingAngle = 1, UseNgOpt = 0, UseCharacter = 0,
            ModelFilter = 0, CntPatternPath = 1,
            FactorRed = 1, FactorGreen = 1, FactorBlue = 1,
            ModelPathInspect1 = pp.ModelPathInspect ?? string.Empty,
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
