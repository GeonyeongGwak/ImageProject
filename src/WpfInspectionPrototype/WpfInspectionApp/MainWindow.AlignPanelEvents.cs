using WpfInspectionApp.ViewModels;

namespace WpfInspectionApp;

public partial class MainWindow
{
    private void HandleAlignPanelUpdate(AlignPanelUpdateEffect update)
    {
        HandleUiChange(() => ApplyAlignPanelUpdate(update));
    }

    private void ApplyAlignPanelUpdate(AlignPanelUpdateEffect update)
    {
        if (update.HasFlag(AlignPanelUpdateEffect.PartTeachingUi))
        {
            UpdatePartTeachingUi();
        }

        if (update.HasFlag(AlignPanelUpdateEffect.Model))
        {
            UpdateModelFromUi();
        }

        if (update.HasFlag(AlignPanelUpdateEffect.AlgorithmPanels))
        {
            _viewModel.UpdateAlgorithmPanels();
        }

        if (update.HasFlag(AlignPanelUpdateEffect.RoiDrawButton))
        {
            _viewModel.UpdateRoiDrawButtonState();
        }

        if (update.HasFlag(AlignPanelUpdateEffect.ActiveRoiUi))
        {
            UpdateActiveRoiUi();
        }

        if (update.HasFlag(AlignPanelUpdateEffect.MaskDensity))
        {
            UpdateMaskDensity();
        }

        if (update.HasFlag(AlignPanelUpdateEffect.RoiText))
        {
            UpdateRoiText();
        }

        if (update.HasFlag(AlignPanelUpdateEffect.RoiOverlays))
        {
            DrawRoiOverlays();
        }

        if (update.HasFlag(AlignPanelUpdateEffect.Threshold))
        {
            ScheduleThreshold();
        }
    }

    private void AlignPanel_UpdateRequested(object? sender, AlignPanelUpdateRequestedEventArgs e)
    {
        switch (e.Kind)
        {
            case AlignPanelUpdateKind.SearchNum:
                SearchNumCombo_SelectionChanged();
                break;
            case AlignPanelUpdateKind.SearchSize:
                SearchSizeBox_TextChanged();
                break;
            default:
                HandleAlignPanelUpdate(e.Kind.ToEffect());
                break;
        }
    }

    private void AlignPanel_ActionRequested(object? sender, AlignPanelActionRequestedEventArgs e)
    {
        switch (e.Kind)
        {
            case AlignPanelActionKind.ActiveRoi:
                _viewModel.SelectNextAlignRoi();
                break;
            case AlignPanelActionKind.DrawWindowRoi:
                _viewModel.EnableWindowRoiDrawing();
                break;
            case AlignPanelActionKind.DrawAlgorithmRoi:
                _viewModel.EnableAlgorithmRoiDrawing();
                break;
            case AlignPanelActionKind.Teach:
                TeachActiveRoiSize();
                break;
            case AlignPanelActionKind.PartTeachingIc:
                RunAlignPartTeaching(useGerber: false);
                break;
            case AlignPanelActionKind.PartTeachingOk:
                RunAlignPartTeaching(useGerber: true);
                break;
            case AlignPanelActionKind.PartTeachingClose:
                CloseAlignPartTeaching();
                break;
        }
    }
}
