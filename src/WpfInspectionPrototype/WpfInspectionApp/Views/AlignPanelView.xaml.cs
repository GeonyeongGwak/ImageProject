using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using WpfInspectionApp.ViewModels;

namespace WpfInspectionApp.Views;

public partial class AlignPanelView : UserControl
{
    private readonly AlignPanelViewModel _viewModel = new();
    private bool _suppressUpdateRequests;

    public AlignPanelView()
    {
        InitializeComponent();
        DataContext = _viewModel;
        _viewModel.ActionRequested += (_, e) => ActionRequested?.Invoke(this, e);
    }

    public event EventHandler? SearchTabSelectionChanged;
    public event EventHandler<AlignPanelUpdateRequestedEventArgs>? UpdateRequested;
    public event EventHandler<AlignPanelActionRequestedEventArgs>? ActionRequested;

    public AlignPanelViewModel ViewModel => _viewModel;

    public bool IsSearchTabActive => true;

    public void ActivateSearchTab()
    {
        Keyboard.Focus(SearchNumCombo);
    }

    public AlignPanelModelState CaptureModelState(string selectedAlgorithm)
    {
        return SuppressUpdateRequests(() => _viewModel.CaptureModelState(selectedAlgorithm));
    }

    public void LoadState(AlignPanelModelState state)
    {
        SuppressUpdateRequests(() => _viewModel.LoadState(state));
    }

    public void SetSearchNum(int value)
    {
        SuppressUpdateRequests(() => _viewModel.SetSearchNum(value));
    }

    public void SetSearchSize(int width, int height)
    {
        SuppressUpdateRequests(() => _viewModel.SetSearchSize(width, height));
    }

    public void SetMaskDensity(int maskDensity)
    {
        _viewModel.SetMaskDensity(maskDensity);
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
        SuppressUpdateRequests(_viewModel.UpdatePartTeachingUi);
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
        if (ReferenceEquals(e.OriginalSource, AlignTabControl))
        {
            SearchTabSelectionChanged?.Invoke(this, EventArgs.Empty);
        }
    }

    private void AlignControl_Changed(object sender, RoutedEventArgs e)
    {
        if (_suppressUpdateRequests)
        {
            return;
        }

        if (sender is not FrameworkElement { Tag: AlignPanelUpdateKind kind })
        {
            return;
        }

        RequestUpdate(kind, sender);
    }

    private void RequestUpdate(AlignPanelUpdateKind kind, object? source)
    {
        UpdateRequested?.Invoke(this, new AlignPanelUpdateRequestedEventArgs(kind, source));
    }

    private void SuppressUpdateRequests(Action action)
    {
        var wasSuppressing = _suppressUpdateRequests;
        _suppressUpdateRequests = true;
        try
        {
            action();
        }
        finally
        {
            _suppressUpdateRequests = wasSuppressing;
        }
    }

    private T SuppressUpdateRequests<T>(Func<T> action)
    {
        var wasSuppressing = _suppressUpdateRequests;
        _suppressUpdateRequests = true;
        try
        {
            return action();
        }
        finally
        {
            _suppressUpdateRequests = wasSuppressing;
        }
    }

}
