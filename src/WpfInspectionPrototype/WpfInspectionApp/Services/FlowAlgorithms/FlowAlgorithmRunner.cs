using System.Diagnostics;
using System.IO;
using System.Text;
using System.Windows.Input;
using WpfInspectionApp.Commands;
using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Interop;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services.FlowAlgorithms;

// Per-algorithm view-model item. The XAML ItemsControl binds to a collection of these
// — each one renders as: [Run] DisplayName  →  Summary text. Hides all the flow
// plumbing (LoadPtt / BeginPart / AddWindow / Commit / InspProc) behind RunCommand so
// the algorithm class itself only needs to provide ApplyParams + ReadResult.
public sealed class FlowAlgorithmRunner : ObservableObject
{
    private readonly IFlowAlgorithm _algorithm;
    private readonly Func<string?> _resolvePttPath;          // model's PTT path (or pop dialog)
    private readonly Func<(double X, double Y)?> _resolveResolution; // model resolution
    private bool _isRunning;
    private string _summary = "Not run yet.";
    private bool _lastSuccess;

    public FlowAlgorithmRunner(
        IFlowAlgorithm algorithm,
        Func<string?> resolvePttPath,
        Func<(double X, double Y)?> resolveResolution)
    {
        _algorithm = algorithm;
        _resolvePttPath = resolvePttPath;
        _resolveResolution = resolveResolution;
        // Created once and reused across Run clicks so user-tuned values persist. The
        // XAML matches an implicit DataTemplate against the concrete Parameters type.
        Parameters = algorithm.CreateParameters();
        RunCommand = new AsyncRelayCommand(RunAsync, () => !_isRunning);
    }

    public string DisplayName => _algorithm.DisplayName;
    public int AlgoType => _algorithm.AlgoType;
    public int InspType => _algorithm.InspType;
    public IFlowAlgorithmParameters Parameters { get; }
    public ICommand RunCommand { get; }

    public bool IsRunning
    {
        get => _isRunning;
        private set
        {
            if (SetProperty(ref _isRunning, value))
            {
                ((AsyncRelayCommand)RunCommand).RaiseCanExecuteChanged();
            }
        }
    }

    public string Summary
    {
        get => _summary;
        private set => SetProperty(ref _summary, value);
    }

    public bool LastSuccess
    {
        get => _lastSuccess;
        private set => SetProperty(ref _lastSuccess, value);
    }

    private async Task RunAsync()
    {
        var ptt = _resolvePttPath();
        if (string.IsNullOrWhiteSpace(ptt) || !File.Exists(ptt))
        {
            Summary = "PTT file not selected.";
            LastSuccess = false;
            return;
        }

        IsRunning = true;
        try
        {
            var result = await Task.Run(() => RunOnce(ptt!));
            Summary = $"[{(result.Success ? "OK" : "NG")}] {result.Summary} (defect={result.DefectCode})";
            LastSuccess = result.Success;
        }
        catch (Exception ex)
        {
            DiagnosticsLog.Write($"[{_algorithm.DisplayName}] flow failed: {ex}");
            Summary = $"FAILED: {ex.GetType().Name}: {ex.Message}";
            LastSuccess = false;
        }
        finally
        {
            IsRunning = false;
        }
    }

    private FlowAlgorithmResult RunOnce(string pttPath)
    {
        NativeDependencyPath.EnsureInitialized();

        var sb = new StringBuilder(512);
        var sw = Stopwatch.StartNew();

        int code = MptiFlowNativeBridge.MptiBridgeLoadPtt(
            pttPath, out int partW, out int partH, 0, 1, sb, sb.Capacity);
        if (code != 0 || partW <= 0 || partH <= 0)
        {
            return Fail($"LoadPtt {code}: {sb}");
        }

        // Resolution: explicit override > .pot file > native 1.0 fallback.
        var resolution = _resolveResolution();
        double rx = resolution?.X ?? 0;
        double ry = resolution?.Y ?? 0;
        if (rx <= 0 || ry <= 0)
        {
            if (LegacyPttImageLoader.TryReadPotResolution(pttPath, out var pr, out var pry))
            { rx = pr; ry = pry; }
        }
        if (rx > 0 && ry > 0)
            MptiFlowNativeBridge.MptiBridgeSetFlowResolution(rx, ry);

        sb.Clear();
        MptiFlowNativeBridge.MptiBridgeSetRawDataFovInfo(
            pttPath, null, 0, 0, 0, 0, 0, out _, sb, sb.Capacity);

        MptiFlowNativeBridge.MptiBridgeBeginPart(
            partW / 2.0, partH / 2.0, partW, partH, 0.0, partW, partH);

        int wndW = Math.Max(64, partW / 3);
        int wndH = Math.Max(64, partH / 3);
        int wndIdx = MptiFlowNativeBridge.MptiBridgeAddWindow(
            _algorithm.InspType, partW / 2.0, partH / 2.0, wndW, wndH, 0, 0);
        if (wndIdx < 0) return Fail($"AddWindow {wndIdx}");

        int algoIdx = MptiFlowNativeBridge.MptiBridgeAddAlgo(wndIdx, _algorithm.AlgoType, 1);
        if (algoIdx < 0) return Fail($"AddAlgo {algoIdx}");

        var slot = new FlowAlgorithmSlot(_algorithm.InspType, wndIdx, algoIdx);
        var ctx = new FlowAlgorithmContext { PartWidth = partW, PartHeight = partH };
        using (FlowAlgorithmContext.Push(ctx))
        {
            try { _algorithm.ApplyParams(slot, Parameters); }
            catch (Exception ex) { return Fail($"ApplyParams threw: {ex.Message}"); }

            sb.Clear();
            int commitCode = MptiFlowNativeBridge.MptiBridgeCommitInspParam(sb, sb.Capacity);
            if (commitCode != 0) return Fail($"Commit {commitCode}: {sb}");

            sb.Clear();
            int procCode = MptiFlowNativeBridge.MptiBridgeInspProc(sb, sb.Capacity);

            FlowAlgorithmResult result;
            try { result = _algorithm.ReadResult(slot); }
            catch (Exception ex) { return Fail($"ReadResult threw: {ex.Message}"); }

            sw.Stop();
            return result with
            {
                Summary = $"{result.Summary} | InspProc={procCode} | {sw.Elapsed.TotalMilliseconds:F1} ms"
            };
        }
    }

    private static FlowAlgorithmResult Fail(string message) => new(
        Success: false, ReaderCode: -1, IsInsp: false, IsOk: false, DefectCode: 0,
        Summary: message, Fields: new Dictionary<string, string>());
}
