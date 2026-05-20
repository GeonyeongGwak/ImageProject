using WpfInspectionApp.Models;

namespace WpfInspectionApp.AlgorithmPanels;

public sealed class AlgorithmPanelInteraction
{
    private readonly AlgorithmCatalogItem _catalog;
    private readonly Func<bool> _isBinding;
    private readonly Func<AlgorithmPanelContext?> _getContext;
    private readonly Action _requestRebuild;
    private readonly Func<string, bool> _onAlgorithmSpecificCommand;
    private readonly Action<string, string> _onAlgorithmSpecificParameterChanged;

    public AlgorithmPanelInteraction(
        AlgorithmCatalogItem catalog,
        Func<bool> isBinding,
        Func<AlgorithmPanelContext?> getContext,
        Action requestRebuild,
        Func<string, bool> onAlgorithmSpecificCommand,
        Action<string, string> onAlgorithmSpecificParameterChanged)
    {
        _catalog = catalog;
        _isBinding = isBinding;
        _getContext = getContext;
        _requestRebuild = requestRebuild;
        _onAlgorithmSpecificCommand = onAlgorithmSpecificCommand;
        _onAlgorithmSpecificParameterChanged = onAlgorithmSpecificParameterChanged;
    }

    public string Read(string key, string fallback)
    {
        var context = _getContext();
        return AlgorithmParameterStore.GetValue(context?.Algorithm.Parameters, key, fallback);
    }

    public int ReadIndex(string key, string fallback)
    {
        return int.TryParse(Read(key, fallback), out var value) ? value : 0;
    }

    public void Write(string key, string value)
    {
        var context = _getContext();
        if (_isBinding() || context == null)
        {
            return;
        }

        AlgorithmPanelCommonEvents.WriteParameter(context, key, value);
        _onAlgorithmSpecificParameterChanged(key, value);

        if (AlgorithmPanelControlStatePolicy.ShouldRebuildForDependency(key))
        {
            _requestRebuild();
        }
    }

    public void ExecuteCommand(string key, bool rebuild)
    {
        var context = _getContext();
        if (context == null)
        {
            return;
        }

        if (AlgorithmPanelCommonEvents.IsCommonCommand(_catalog, key))
        {
            AlgorithmPanelCommonEvents.TriggerCommand(context, _catalog, key);
        }
        else if (!_onAlgorithmSpecificCommand(key))
        {
            AlgorithmPanelCommonEvents.TriggerCommand(context, _catalog, key);
        }

        if (rebuild)
        {
            _requestRebuild();
        }
    }
}
