namespace WpfInspectionApp.AlgorithmPanels.Types;

public sealed class PadBwAlgorithmPanel : DynamicAlgorithmPanel
{
    public PadBwAlgorithmPanel() : base("AlgoPadBW") { }

    protected override bool OnAlgorithmSpecificCommand(string key)
    {
        if (Context == null || !key.Contains("PadBW", StringComparison.OrdinalIgnoreCase))
        {
            return false;
        }

        AlgorithmPanelCommonEvents.WriteParameter(Context, "PadBW.Event.LastPanelCommand", key);
        Context.RequestPreviewUpdate();
        Context.RequestTreeRefresh();
        return true;
    }

    protected override void OnAlgorithmSpecificParameterChanged(string key, string value)
    {
        if (Context != null && key.Contains("PadBW", StringComparison.OrdinalIgnoreCase))
        {
            Context.Algorithm.Parameters["PadBW.Event.LastEditedParameter"] = key;
        }
    }
}
