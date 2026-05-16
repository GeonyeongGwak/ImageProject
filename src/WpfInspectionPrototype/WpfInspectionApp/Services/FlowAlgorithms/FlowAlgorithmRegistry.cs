namespace WpfInspectionApp.Services.FlowAlgorithms;

// Holds the set of IFlowAlgorithm instances the UI exposes. AppServices populates this
// once at startup. ItemsControl in the XAML iterates the runners; each runner wraps one
// IFlowAlgorithm and has its own Run command.
public sealed class FlowAlgorithmRegistry
{
    private readonly List<IFlowAlgorithm> _algorithms = new();

    public IReadOnlyList<IFlowAlgorithm> All => _algorithms;

    public void Register(IFlowAlgorithm algorithm)
    {
        if (algorithm == null) throw new ArgumentNullException(nameof(algorithm));
        _algorithms.Add(algorithm);
    }
}
