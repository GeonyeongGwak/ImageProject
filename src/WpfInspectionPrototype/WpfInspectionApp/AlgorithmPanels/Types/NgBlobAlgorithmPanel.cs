using WpfInspectionApp.Models;

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
        Context.RequestPreviewUpdate();
        Context.RequestTreeRefresh();
        return true;
    }

    protected override void OnAlgorithmSpecificParameterChanged(string key, string value)
    {
        if (Context != null && key.Contains("NGBlob", StringComparison.OrdinalIgnoreCase))
        {
            AlgorithmParameterStore.Set(Context.Algorithm.Parameters, "NGBlob.Event.LastEditedParameter", key);
        }
    }
}
