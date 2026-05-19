using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using WpfInspectionApp.ViewModels;

namespace WpfInspectionApp.Views;

public partial class AlignPanelView : UserControl
{
    private readonly AlignPanelViewModel _viewModel = new();

    public AlignPanelView()
    {
        InitializeComponent();
        DataContext = _viewModel;
        _viewModel.ActionRequested += (_, e) => ActionRequested?.Invoke(this, e);
    }

    public event SelectionChangedEventHandler? AlignTabSelectionChanged;
    public event EventHandler<AlignPanelUpdateRequestedEventArgs>? UpdateRequested;
    public event EventHandler<AlignPanelActionRequestedEventArgs>? ActionRequested;

    public AlignPanelViewModel ViewModel => _viewModel;

    public bool IsSearchTabActive => AlignTabControl.SelectedIndex == 0;

    public void ActivateSearchTab()
    {
        AlignTabControl.SelectedIndex = 0;
    }

    public AlignPanelModelState CaptureModelState(string selectedAlgorithm)
    {
        return _viewModel.CaptureModelState(selectedAlgorithm);
    }

    public void LoadState(AlignPanelModelState state)
    {
        _viewModel.LoadState(state);
    }

    public void SetSearchNum(int value)
    {
        _viewModel.SetSearchNum(value);
    }

    public void SetSearchSize(int width, int height)
    {
        _viewModel.SetSearchSize(width, height);
    }

    public void MirrorSearchSizeInput(object? sender)
    {
        if (!_viewModel.SameSize)
        {
            return;
        }

        if (sender == SearchSizeXBox)
        {
            _viewModel.MirrorSearchSizeFromX();
        }
        else if (sender == SearchSizeYBox)
        {
            _viewModel.MirrorSearchSizeFromY();
        }
    }

    public void SetMaskDensity(int maskDensity)
    {
        _viewModel.MaskDensityText = $"{maskDensity}%";
    }

    public void SetActiveRoiText(string text)
    {
        _viewModel.ActiveRoiText = text;
    }

    public void SetRoiText(string text)
    {
        _viewModel.RoiText = text;
    }

    public void SetWindowRoiDrawingState(bool isDrawingWindow)
    {
        _viewModel.SetWindowRoiDrawingState(isDrawingWindow);
    }

    public void UpdatePartTeachingUi()
    {
        _viewModel.UpdatePartTeachingUi();
    }

    public void SetPartTeachingStatus(string status)
    {
        _viewModel.PartTeachingStatus = status;
    }

    public void ClosePartTeaching()
    {
        _viewModel.ClosePartTeaching();
    }

    private void AlignTabControl_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        AlignTabSelectionChanged?.Invoke(sender, e);
    }

    private void SearchNumCombo_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        RequestUpdate(AlignPanelUpdateKind.SearchNum, sender);
    }

    private void SearchParameter_Changed(object sender, RoutedEventArgs e)
    {
        RequestUpdate(AlignPanelUpdateKind.SearchParameter, sender);
    }

    private void SearchSizeBox_TextChanged(object sender, TextChangedEventArgs e)
    {
        RequestUpdate(AlignPanelUpdateKind.SearchSize, sender);
    }

    private void Parameter_Changed(object sender, RoutedEventArgs e)
    {
        RequestUpdate(AlignPanelUpdateKind.Parameter, sender);
    }

    private void AlignParameter_Changed(object sender, RoutedEventArgs e)
    {
        RequestUpdate(AlignPanelUpdateKind.AlignParameter, sender);
    }

    private void ThresholdSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
    {
        RequestUpdate(AlignPanelUpdateKind.Threshold2D, sender);
    }

    private void Threshold3DSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
    {
        RequestUpdate(AlignPanelUpdateKind.Threshold3D, sender);
    }

    private void EdgeGainSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
    {
        RequestUpdate(AlignPanelUpdateKind.EdgeGain, sender);
    }

    private void IpcClassCombo_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        RequestUpdate(AlignPanelUpdateKind.IpcClass, sender);
    }

    private void PartTeachingOption_Changed(object sender, RoutedEventArgs e)
    {
        RequestUpdate(AlignPanelUpdateKind.PartTeachingOption, sender);
    }

    private void PartTeachingOption_Changed(object sender, SelectionChangedEventArgs e)
    {
        RequestUpdate(AlignPanelUpdateKind.PartTeachingOption, sender);
    }

    private void RequestUpdate(AlignPanelUpdateKind kind, object? source)
    {
        UpdateRequested?.Invoke(this, new AlignPanelUpdateRequestedEventArgs(kind, source));
    }

}
