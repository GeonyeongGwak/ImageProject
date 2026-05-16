using WpfInspectionApp.Interop;

namespace WpfInspectionApp.Services.FlowAlgorithms;

// Per-algorithm result envelope returned by IFlowAlgorithm.ReadResult. The Summary is
// a one-line human-readable digest for the UI; Fields holds structured key/value pairs
// if a richer presentation is needed (e.g. a tooltip or data grid).
public sealed record FlowAlgorithmResult(
    bool Success,
    int ReaderCode,            // 0 on success; native bridge -1..-6 for placement errors
    bool IsInsp,
    bool IsOk,
    int DefectCode,
    string Summary,
    IReadOnlyDictionary<string, string> Fields);

// Context passed to ApplyParams / ReadResult. Lets the algorithm know which (window,
// algo) slot it owns within the flow build. wndType is the eINSP_* enum value, used
// for routing the result lookup to the correct InspectionResult sub-array.
public readonly record struct FlowAlgorithmSlot(int WndType, int WndIdx, int AlgoIdx);

// One MPTI flow algorithm registered with the plugin system. Implementations describe
// their identity (DisplayName / AlgoType / InspType), the native param-set call, and
// the typed result reader. The shared FlowAlgorithmRunner handles the rest of the
// pipeline (PTT load, BeginPart, AddWindow, AddAlgo, Commit, InspProc).
//
// Adding a new algorithm = create one file implementing this interface and register it
// in AppServices.RegisterFlowAlgorithms — no changes to MainViewModel/XAML needed.
public interface IFlowAlgorithm
{
    // Human-readable name shown next to the Run button in the UI.
    string DisplayName { get; }

    // Native InspAlgoType (eAlgoXxx, mirrored as MptiFlowNativeBridge.EALGO_*). Used by
    // MptiBridgeAddAlgo and by the type-tag check inside the result reader.
    int AlgoType { get; }

    // Native inspection-type (eINSP_*, mirrored as MptiFlowNativeBridge.EINSP_*). The
    // window's parent type determines which result array (mountResult / alignResult /
    // padResult / BGAResult / ...) the algorithm's window ends up in. Set this to
    // whatever wndType the runner should use when adding the window for this algo.
    int InspType { get; }

    // Calls the typed MptiBridgeSetAlgoParamsXxx with reasonable defaults so the
    // algorithm has enough info to run. UI-tuned params will plug in here later.
    void ApplyParams(FlowAlgorithmSlot slot);

    // Calls the typed MptiBridgeResultXxx and converts to a uniform envelope.
    FlowAlgorithmResult ReadResult(FlowAlgorithmSlot slot);
}
