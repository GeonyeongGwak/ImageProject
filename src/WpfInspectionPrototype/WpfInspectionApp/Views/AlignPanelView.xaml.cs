using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using WpfInspectionApp.Models;
using WpfInspectionApp.ViewModels;

namespace WpfInspectionApp.Views;

public enum AlignPanelUpdateKind
{
    SearchNum,
    SearchParameter,
    SearchSize,
    Parameter,
    AlignParameter,
    Threshold2D,
    Threshold3D,
    EdgeGain,
    IpcClass,
    PartTeachingOption
}

public enum AlignPanelActionKind
{
    ActiveRoi,
    DrawWindowRoi,
    DrawAlgorithmRoi,
    Teach,
    PartTeachingIc,
    PartTeachingOk,
    PartTeachingClose
}

public sealed record AlignPanelUpdateRequestedEventArgs(AlignPanelUpdateKind Kind, object? Source);

public sealed record AlignPanelActionRequestedEventArgs(AlignPanelActionKind Kind);

public partial class AlignPanelView : UserControl
{
    private readonly AlignPanelViewModel _viewModel = new();

    public AlignPanelView()
    {
        InitializeComponent();
        DataContext = _viewModel;
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

    public void ApplyToModel(InspectionModel model, string algorithm, int sourceWidth, int sourceHeight)
    {
        model.Algorithm = algorithm;
        _viewModel.ApplyToModel(model, sourceWidth, sourceHeight);
        _viewModel.LoadFromModel(model);
    }

    public void LoadFromModel(InspectionModel model)
    {
        _viewModel.LoadFromModel(model);
    }

    public void UpdateConditionUi(InspectionModel model)
    {
        _viewModel.LoadFromModel(model);
    }

    public void SetSearchNum(int value)
    {
        _viewModel.SetSearchNum(value);
    }

    public void SetSearchSize(int width, int height)
    {
        _viewModel.SetSearchSize(width, height);
    }

    public void MirrorSearchSizeInput(object? sender, InspectionModel model)
    {
        if (!_viewModel.SameSize)
        {
            return;
        }

        if (sender == SearchSizeXBox)
        {
            _viewModel.MirrorSearchSizeFromX();
            model.AlignSearchSizeY = model.AlignSearchSizeX;
        }
        else if (sender == SearchSizeYBox)
        {
            _viewModel.MirrorSearchSizeFromY();
            model.AlignSearchSizeX = model.AlignSearchSizeY;
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

    private void ActiveRoiButton_Click(object sender, RoutedEventArgs e)
    {
        RequestAction(AlignPanelActionKind.ActiveRoi);
    }

    private void DrawRoiButton_Click(object sender, RoutedEventArgs e)
    {
        RequestAction(AlignPanelActionKind.DrawWindowRoi);
    }

    private void DrawAlgorithmRoiButton_Click(object sender, RoutedEventArgs e)
    {
        RequestAction(AlignPanelActionKind.DrawAlgorithmRoi);
    }

    private void SearchParameter_Changed(object sender, RoutedEventArgs e)
    {
        RequestUpdate(AlignPanelUpdateKind.SearchParameter, sender);
    }

    private void SearchSizeBox_TextChanged(object sender, TextChangedEventArgs e)
    {
        RequestUpdate(AlignPanelUpdateKind.SearchSize, sender);
    }

    private void TeachButton_Click(object sender, RoutedEventArgs e)
    {
        RequestAction(AlignPanelActionKind.Teach);
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

    private void PartTeachingIcButton_Click(object sender, RoutedEventArgs e)
    {
        RequestAction(AlignPanelActionKind.PartTeachingIc);
    }

    private void PartTeachingOkButton_Click(object sender, RoutedEventArgs e)
    {
        RequestAction(AlignPanelActionKind.PartTeachingOk);
    }

    private void PartTeachingCloseButton_Click(object sender, RoutedEventArgs e)
    {
        RequestAction(AlignPanelActionKind.PartTeachingClose);
    }

    private void RequestUpdate(AlignPanelUpdateKind kind, object? source)
    {
        UpdateRequested?.Invoke(this, new AlignPanelUpdateRequestedEventArgs(kind, source));
    }

    private void RequestAction(AlignPanelActionKind kind)
    {
        ActionRequested?.Invoke(this, new AlignPanelActionRequestedEventArgs(kind));
    }
}
