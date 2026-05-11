using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using WpfInspectionApp.Models;
using WpfInspectionApp.ViewModels;

namespace WpfInspectionApp.Views;

public partial class AlignPanelView : UserControl
{
    private readonly AlignPanelViewModel _viewModel = new();

    public AlignPanelView()
    {
        InitializeComponent();
        DataContext = _viewModel;
    }

    public event SelectionChangedEventHandler? AlignTabSelectionChanged;
    public event SelectionChangedEventHandler? SearchNumSelectionChanged;
    public event RoutedEventHandler? ActiveRoiRequested;
    public event RoutedEventHandler? DrawWindowRoiRequested;
    public event RoutedEventHandler? DrawAlgorithmRoiRequested;
    public event RoutedEventHandler? SearchParameterChanged;
    public event RoutedEventHandler? SearchSizeChanged;
    public event RoutedEventHandler? TeachRequested;
    public event RoutedEventHandler? ParameterChanged;
    public event RoutedEventHandler? AlignParameterChanged;
    public event RoutedPropertyChangedEventHandler<double>? ThresholdSliderChanged;
    public event RoutedPropertyChangedEventHandler<double>? Threshold3DSliderChanged;
    public event RoutedPropertyChangedEventHandler<double>? EdgeGainSliderChanged;
    public event SelectionChangedEventHandler? IpcClassSelectionChanged;
    public event SelectionChangedEventHandler? PartTeachingOptionSelectionChanged;
    public event RoutedEventHandler? PartTeachingOptionChanged;
    public event RoutedEventHandler? PartTeachingIcRequested;
    public event RoutedEventHandler? PartTeachingOkRequested;
    public event RoutedEventHandler? PartTeachingCloseRequested;

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

    public void MirrorSearchSizeInput(object sender, InspectionModel model)
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
        SearchNumSelectionChanged?.Invoke(sender, e);
    }

    private void ActiveRoiButton_Click(object sender, RoutedEventArgs e)
    {
        ActiveRoiRequested?.Invoke(sender, e);
    }

    private void DrawRoiButton_Click(object sender, RoutedEventArgs e)
    {
        DrawWindowRoiRequested?.Invoke(sender, e);
    }

    private void DrawAlgorithmRoiButton_Click(object sender, RoutedEventArgs e)
    {
        DrawAlgorithmRoiRequested?.Invoke(sender, e);
    }

    private void SearchParameter_Changed(object sender, RoutedEventArgs e)
    {
        SearchParameterChanged?.Invoke(sender, e);
    }

    private void SearchSizeBox_TextChanged(object sender, TextChangedEventArgs e)
    {
        SearchSizeChanged?.Invoke(sender, e);
    }

    private void TeachButton_Click(object sender, RoutedEventArgs e)
    {
        TeachRequested?.Invoke(sender, e);
    }

    private void Parameter_Changed(object sender, RoutedEventArgs e)
    {
        ParameterChanged?.Invoke(sender, e);
    }

    private void AlignParameter_Changed(object sender, RoutedEventArgs e)
    {
        AlignParameterChanged?.Invoke(sender, e);
    }

    private void ThresholdSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
    {
        ThresholdSliderChanged?.Invoke(sender, e);
    }

    private void Threshold3DSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
    {
        Threshold3DSliderChanged?.Invoke(sender, e);
    }

    private void EdgeGainSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
    {
        EdgeGainSliderChanged?.Invoke(sender, e);
    }

    private void IpcClassCombo_SelectionChanged(object sender, SelectionChangedEventArgs e)
    {
        IpcClassSelectionChanged?.Invoke(sender, e);
    }

    private void PartTeachingOption_Changed(object sender, RoutedEventArgs e)
    {
        PartTeachingOptionChanged?.Invoke(sender, e);
    }

    private void PartTeachingOption_Changed(object sender, SelectionChangedEventArgs e)
    {
        PartTeachingOptionSelectionChanged?.Invoke(sender, e);
    }

    private void PartTeachingIcButton_Click(object sender, RoutedEventArgs e)
    {
        PartTeachingIcRequested?.Invoke(sender, e);
    }

    private void PartTeachingOkButton_Click(object sender, RoutedEventArgs e)
    {
        PartTeachingOkRequested?.Invoke(sender, e);
    }

    private void PartTeachingCloseButton_Click(object sender, RoutedEventArgs e)
    {
        PartTeachingCloseRequested?.Invoke(sender, e);
    }

}
