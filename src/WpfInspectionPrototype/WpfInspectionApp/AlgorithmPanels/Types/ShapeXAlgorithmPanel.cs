using WpfInspectionApp.Models;

namespace WpfInspectionApp.AlgorithmPanels.Types;

public sealed class ShapeXAlgorithmPanel : DynamicAlgorithmPanel
{
    public ShapeXAlgorithmPanel() : base("AlgoShapeX") { }

    protected override bool OnAlgorithmSpecificCommand(string key)
    {
        if (Context == null || !key.Contains("ShapeX", StringComparison.OrdinalIgnoreCase))
        {
            return false;
        }

        AlgorithmPanelCommonEvents.WriteParameter(Context, "ShapeX.Event.LastPanelCommand", key);
        Context.RequestPreviewUpdate();
        Context.RequestTreeRefresh();
        return true;
    }

    protected override void OnAlgorithmSpecificParameterChanged(string key, string value)
    {
        if (Context != null && key.Contains("ShapeX", StringComparison.OrdinalIgnoreCase))
        {
            AlgorithmParameterStore.Set(Context.Algorithm.Parameters, "ShapeX.Event.LastEditedParameter", key);
        }
    }
}
