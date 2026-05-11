namespace WpfInspectionApp.AlgorithmPanels.Types;

public sealed class NgBlobAlgorithmPanel : DynamicAlgorithmPanel
{
    public NgBlobAlgorithmPanel() : base("AlgoNGBlob") { }

    protected override bool OnAlgorithmSpecificCommand(string key)
    {
        if (Context == null || !key.Contains("NGBlob", StringComparison.OrdinalIgnoreCase))
        {
            return false;
        }

        AlgorithmPanelCommonEvents.WriteParameter(Context, "NGBlob.Event.LastPanelCommand", key);
        Context.RequestPreviewUpdate?.Invoke();
        Context.RequestTreeRefresh?.Invoke();
        return true;
    }

    protected override void OnAlgorithmSpecificParameterChanged(string key, string value)
    {
        if (Context != null && key.Contains("NGBlob", StringComparison.OrdinalIgnoreCase))
        {
            Context.Algorithm.Parameters["NGBlob.Event.LastEditedParameter"] = key;
        }
    }
}
