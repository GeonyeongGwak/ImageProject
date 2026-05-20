using System.Collections.ObjectModel;
using System.Globalization;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using WpfInspectionApp.Commands;
using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Models;
using WpfInspectionApp.Services;

namespace WpfInspectionApp.ViewModels;

public sealed class LightControlViewModel : ViewModelBase
{
    private readonly IAlgorithmLightService _lightService;
    private readonly Action _schedulePreview;
    private readonly RelayCommand _addUserCellCommand;
    private readonly RelayCommand _removeUserCellCommand;
    private readonly RelayCommand _toggleUserCellOperatorCommand;
    private readonly RelayCommand _toggleUserPreviewModeCommand;
    private InspectionAlgorithmData? _algorithm;
    private bool _isLoading;
    private bool _isUserMixPreview = true;
    private int _lightType;
    private LightCellViewModel? _selectedUserCell;

    public LightControlViewModel(IAlgorithmLightService lightService, Action schedulePreview)
    {
        _lightService = lightService;
        _schedulePreview = schedulePreview;

        LightTypeOptions =
        [
            new LightOptionViewModel(AlgorithmLightService.TopLight, "TOP"),
            new LightOptionViewModel(AlgorithmLightService.MiddleLight, "MIDDLE"),
            new LightOptionViewModel(AlgorithmLightService.BottomLight, "BOTTOM"),
            new LightOptionViewModel(AlgorithmLightService.UserLight, "USER"),
            new LightOptionViewModel(AlgorithmLightService.SideRedLight, "SIDE RED"),
            new LightOptionViewModel(AlgorithmLightService.SideGreenLight, "SIDE GREEN"),
            new LightOptionViewModel(AlgorithmLightService.SideBlueLight, "SIDE BLUE"),
            new LightOptionViewModel(AlgorithmLightService.Side4Light, "SIDE 4"),
            new LightOptionViewModel(AlgorithmLightService.ThreeDLight, "3D")
        ];

        PositionOptions =
        [
            new LightOptionViewModel(AlgorithmLightService.TopLight, "TOP"),
            new LightOptionViewModel(AlgorithmLightService.MiddleLight, "MIDDLE"),
            new LightOptionViewModel(AlgorithmLightService.BottomLight, "BOTTOM")
        ];

        OperatorOptions =
        [
            new LightOptionViewModel(0, "NONE"),
            new LightOptionViewModel(1, "ADD"),
            new LightOptionViewModel(2, "SUB")
        ];

        NormalChannels =
        [
            new LightChannelViewModel("R", "Red", CreateBrush(0xEF, 0x44, 0x44), _lightService.MaximumChannelValue, OnNormalChannelChanged),
            new LightChannelViewModel("G", "Green", CreateBrush(0x22, 0xC5, 0x5E), _lightService.MaximumChannelValue, OnNormalChannelChanged),
            new LightChannelViewModel("B", "Blue", CreateBrush(0x3B, 0x82, 0xF6), _lightService.MaximumChannelValue, OnNormalChannelChanged),
            new LightChannelViewModel("W", "White", CreateBrush(0xF8, 0xFA, 0xFC), _lightService.MaximumChannelValue, OnNormalChannelChanged)
        ];

        _addUserCellCommand = new RelayCommand(_ => AddUserCell(), _ => CanAddUserCell);
        _removeUserCellCommand = new RelayCommand(_ => RemoveUserCell(), _ => CanRemoveUserCell);
        _toggleUserCellOperatorCommand = new RelayCommand(
            parameter => ToggleUserCellOperator(parameter as LightCellViewModel),
            parameter => CanToggleUserCellOperator(parameter as LightCellViewModel));
        _toggleUserPreviewModeCommand = new RelayCommand(ToggleUserPreviewMode, () => IsEnabled && IsUserMode);
        AddUserCellCommand = _addUserCellCommand;
        RemoveUserCellCommand = _removeUserCellCommand;
        ToggleUserCellOperatorCommand = _toggleUserCellOperatorCommand;
        ToggleUserPreviewModeCommand = _toggleUserPreviewModeCommand;
        Load(null);
    }

    public ObservableCollection<LightOptionViewModel> LightTypeOptions { get; }

    public ObservableCollection<LightOptionViewModel> PositionOptions { get; }

    public ObservableCollection<LightOptionViewModel> OperatorOptions { get; }

    public ObservableCollection<LightChannelViewModel> NormalChannels { get; }

    public ObservableCollection<LightCellViewModel> UserCells { get; } = [];

    public ICommand AddUserCellCommand { get; }

    public ICommand RemoveUserCellCommand { get; }

    public ICommand ToggleUserCellOperatorCommand { get; }

    public ICommand ToggleUserPreviewModeCommand { get; }

    public bool IsEnabled => _algorithm != null;

    public bool IsNormalMode => LightType != AlgorithmLightService.UserLight;

    public bool IsUserMode => LightType == AlgorithmLightService.UserLight;

    public bool HasSelectedUserCell => SelectedUserCell != null;

    public int MaximumUserCellCount => _lightService.MaximumUserCellCount;

    public bool CanAddUserCell => IsEnabled && IsUserMode && UserCells.Count < MaximumUserCellCount;

    public bool CanRemoveUserCell => IsEnabled && IsUserMode && UserCells.Count > 0;

    public string UserCellCountText => $"{UserCells.Count}/{MaximumUserCellCount}";

    public string UserPreviewModeText => IsUserMixPreview ? "MIX" : "CURRENT";

    public bool IsUserMixPreview
    {
        get => _isUserMixPreview;
        private set
        {
            if (SetProperty(ref _isUserMixPreview, value))
            {
                OnPropertyChanged(nameof(UserPreviewModeText));
            }
        }
    }

    public string LightTypeLabel => LightTypeOptions.FirstOrDefault(item => item.Value == LightType)?.Label
        ?? LightType.ToString(CultureInfo.InvariantCulture);

    public int LightType
    {
        get => _lightType;
        set
        {
            var next = Math.Max(AlgorithmLightService.TopLight, Math.Min(AlgorithmLightService.ThreeDLight, value));
            if (_lightType == next)
            {
                return;
            }

            _lightType = next;
            OnPropertyChanged();
            OnPropertyChanged(nameof(LightTypeLabel));
            OnLightTypeChanged();
        }
    }

    public LightCellViewModel? SelectedUserCell
    {
        get => _selectedUserCell;
        set
        {
            if (ReferenceEquals(_selectedUserCell, value))
            {
                return;
            }

            if (_selectedUserCell != null)
            {
                _selectedUserCell.IsSelected = false;
            }

            _selectedUserCell = value;
            if (_selectedUserCell != null)
            {
                _selectedUserCell.IsSelected = true;
            }

            OnPropertyChanged();
            OnPropertyChanged(nameof(HasSelectedUserCell));
            RaiseCommandStates();

            if (!_isLoading && IsUserMode && !IsUserMixPreview)
            {
                _schedulePreview();
            }
        }
    }

    public void Load(InspectionAlgorithmData? algorithm)
    {
        _algorithm = algorithm;
        var state = _lightService.ReadState(algorithm);
        ApplyStateToView(state, preferredSelectedIndex: 0);
        OnPropertyChanged(nameof(IsEnabled));
        RaiseModeProperties();
        RaiseCommandStates();
    }

    public AlgorithmLightState CreatePreviewState(AlgorithmLightState state)
    {
        if (state.LightType != AlgorithmLightService.UserLight || IsUserMixPreview || SelectedUserCell == null)
        {
            return state;
        }

        return new AlgorithmLightState
        {
            LightType = state.LightType,
            RedValue = state.RedValue,
            GreenValue = state.GreenValue,
            BlueValue = state.BlueValue,
            WhiteValue = state.WhiteValue,
            UserCells = [SelectedUserCell.ToModel()]
        };
    }

    private void OnLightTypeChanged()
    {
        RaiseModeProperties();
        if (_isLoading)
        {
            return;
        }

        var state = _lightService.CreateStateForLightType(LightType, CaptureState());
        ApplyStateToView(state, preferredSelectedIndex: SelectedUserCell?.Index ?? 0);
        SaveAndSchedule();
    }

    private void OnNormalChannelChanged(LightChannelViewModel channel)
    {
        SaveAndSchedule();
    }

    private void OnUserCellChanged(LightCellViewModel cell)
    {
        SaveAndSchedule();
    }

    private void ToggleUserPreviewMode()
    {
        if (!IsEnabled || !IsUserMode)
        {
            return;
        }

        IsUserMixPreview = !IsUserMixPreview;
        _schedulePreview();
        RaiseCommandStates();
    }

    private void AddUserCell()
    {
        if (!CanAddUserCell)
        {
            return;
        }

        var cell = CreateCellViewModel(_lightService.CreateDefaultUserCell());
        UserCells.Add(cell);
        RefreshCellIndexes();
        SelectedUserCell = cell;
        OnPropertyChanged(nameof(UserCellCountText));
        RaiseCommandStates();
        SaveAndSchedule();
    }

    private void RemoveUserCell()
    {
        if (!CanRemoveUserCell)
        {
            return;
        }

        var removeIndex = SelectedUserCell == null
            ? UserCells.Count - 1
            : Math.Max(0, UserCells.IndexOf(SelectedUserCell));
        RemoveUserCellAt(removeIndex, removeIndex);
    }

    private bool CanToggleUserCellOperator(LightCellViewModel? cell)
    {
        return IsEnabled
            && IsUserMode
            && cell != null
            && UserCells.Contains(cell)
            && cell.Index < MaximumUserCellCount - 1;
    }

    private void ToggleUserCellOperator(LightCellViewModel? cell)
    {
        if (!CanToggleUserCellOperator(cell))
        {
            return;
        }

        var index = UserCells.IndexOf(cell!);
        var oldOperator = cell!.OperatorType;
        var nextOperator = NextReferenceOperator(index, UserCells.Count, oldOperator);
        cell.OperatorType = nextOperator;

        if (index == UserCells.Count - 1 && oldOperator == 0 && nextOperator != 0 && CanAddUserCell)
        {
            AddUserCell();
            return;
        }

        if (index == UserCells.Count - 2 && oldOperator != 0 && nextOperator == 0 && UserCells.Count > 1)
        {
            RemoveUserCellAt(UserCells.Count - 1, index);
            return;
        }

        RefreshCellIndexes();
        RaiseCommandStates();
    }

    private static int NextReferenceOperator(int index, int count, int oldOperator)
    {
        if (index < count - 2)
        {
            return oldOperator == 1 ? 2 : 1;
        }

        return oldOperator switch
        {
            0 => 1,
            1 => 2,
            _ => 0
        };
    }

    private void RemoveUserCellAt(int removeIndex, int preferredSelectedIndex)
    {
        if (removeIndex < 0 || removeIndex >= UserCells.Count)
        {
            return;
        }

        UserCells.RemoveAt(removeIndex);
        RefreshCellIndexes();
        SelectedUserCell = UserCells.Count == 0
            ? null
            : UserCells[Math.Max(0, Math.Min(preferredSelectedIndex, UserCells.Count - 1))];
        OnPropertyChanged(nameof(UserCellCountText));
        RaiseCommandStates();
        SaveAndSchedule();
    }

    private void ApplyStateToView(AlgorithmLightState state, int preferredSelectedIndex)
    {
        _isLoading = true;
        try
        {
            _lightType = Math.Max(AlgorithmLightService.TopLight, Math.Min(AlgorithmLightService.ThreeDLight, state.LightType));
            OnPropertyChanged(nameof(LightType));
            OnPropertyChanged(nameof(LightTypeLabel));

            var availability = _lightService.GetChannelAvailability(_lightType, userCellPosition: false);
            NormalChannels[0].Load(state.RedValue, availability.Red);
            NormalChannels[1].Load(state.GreenValue, availability.Green);
            NormalChannels[2].Load(state.BlueValue, availability.Blue);
            NormalChannels[3].Load(state.WhiteValue, availability.White);

            UserCells.Clear();
            foreach (var cell in state.UserCells.Take(MaximumUserCellCount))
            {
                UserCells.Add(CreateCellViewModel(cell));
            }

            RefreshCellIndexes();
            SelectedUserCell = UserCells.Count == 0
                ? null
                : UserCells[Math.Max(0, Math.Min(preferredSelectedIndex, UserCells.Count - 1))];
            OnPropertyChanged(nameof(UserCellCountText));
        }
        finally
        {
            _isLoading = false;
        }

        RaiseModeProperties();
        RaiseCommandStates();
    }

    private LightCellViewModel CreateCellViewModel(AlgorithmLightCell cell)
    {
        var vm = new LightCellViewModel(_lightService, OnUserCellChanged);
        vm.Load(cell, UserCells.Count);
        return vm;
    }

    private AlgorithmLightState CaptureState()
    {
        return new AlgorithmLightState
        {
            LightType = LightType,
            RedValue = NormalChannels[0].Value,
            GreenValue = NormalChannels[1].IsEditable ? NormalChannels[1].Value : 0,
            BlueValue = NormalChannels[2].Value,
            WhiteValue = NormalChannels[3].IsEditable ? NormalChannels[3].Value : 0,
            UserCells = UserCells.Select(cell => cell.ToModel()).ToList()
        };
    }

    private void SaveAndSchedule()
    {
        if (_isLoading || _algorithm == null)
        {
            return;
        }

        _lightService.SaveState(_algorithm, CaptureState());
        _schedulePreview();
        RaiseCommandStates();
    }

    private void RefreshCellIndexes()
    {
        for (var index = 0; index < UserCells.Count; index++)
        {
            UserCells[index].Index = index;
            UserCells[index].IsLastUserCell = index == UserCells.Count - 1;
            UserCells[index].IsOperatorButtonVisible = index < MaximumUserCellCount - 1;
        }
    }

    private void RaiseModeProperties()
    {
        OnPropertyChanged(nameof(IsNormalMode));
        OnPropertyChanged(nameof(IsUserMode));
        OnPropertyChanged(nameof(CanAddUserCell));
        OnPropertyChanged(nameof(CanRemoveUserCell));
        OnPropertyChanged(nameof(UserCellCountText));
        OnPropertyChanged(nameof(UserPreviewModeText));
    }

    private void RaiseCommandStates()
    {
        OnPropertyChanged(nameof(CanAddUserCell));
        OnPropertyChanged(nameof(CanRemoveUserCell));
        _addUserCellCommand.RaiseCanExecuteChanged();
        _removeUserCellCommand.RaiseCanExecuteChanged();
        _toggleUserCellOperatorCommand.RaiseCanExecuteChanged();
        _toggleUserPreviewModeCommand.RaiseCanExecuteChanged();
    }

    private static SolidColorBrush CreateBrush(byte r, byte g, byte b)
    {
        var brush = new SolidColorBrush(Color.FromRgb(r, g, b));
        brush.Freeze();
        return brush;
    }
}

public sealed class LightOptionViewModel
{
    public LightOptionViewModel(int value, string label)
    {
        Value = value;
        Label = label;
    }

    public int Value { get; }

    public string Label { get; }
}

public sealed class LightChannelViewModel : ViewModelBase
{
    private readonly Action<LightChannelViewModel> _changed;
    private readonly RelayCommand? _presetCommand;
    private readonly int _maximum;
    private int _value;
    private string _valueText = "0";
    private bool _isEditable = true;
    private bool _isLoading;

    public LightChannelViewModel(
        string code,
        string label,
        Brush swatch,
        int maximum,
        Action<LightChannelViewModel> changed,
        Action<LightChannelViewModel>? preset = null)
    {
        Code = code;
        Label = label;
        Swatch = swatch;
        _maximum = maximum;
        _changed = changed;
        if (preset != null)
        {
            _presetCommand = new RelayCommand(_ => preset(this), _ => IsEditable);
            PresetCommand = _presetCommand;
        }
    }

    public string Code { get; }

    public string Label { get; }

    public Brush Swatch { get; }

    public int Maximum => _maximum;

    public ICommand? PresetCommand { get; }

    public string PercentText => $"{Value}%";

    public int Value
    {
        get => _value;
        set
        {
            var next = Math.Max(0, Math.Min(_maximum, value));
            if (_value == next)
            {
                SyncValueText(next);
                return;
            }

            _value = next;
            OnPropertyChanged();
            SyncValueText(next);
            OnPropertyChanged(nameof(PercentText));
            if (!_isLoading)
            {
                _changed(this);
            }
        }
    }

    public string ValueText
    {
        get => _valueText;
        set
        {
            var next = value?.Trim() ?? "";
            if (!SetProperty(ref _valueText, next) || _isLoading)
            {
                return;
            }

            if (int.TryParse(next, NumberStyles.Integer, CultureInfo.InvariantCulture, out var parsed))
            {
                Value = parsed;
                return;
            }

            if (!string.IsNullOrWhiteSpace(next))
            {
                SyncValueText(Value);
            }
        }
    }

    public bool IsEditable
    {
        get => _isEditable;
        private set
        {
            if (SetProperty(ref _isEditable, value))
            {
                _presetCommand?.RaiseCanExecuteChanged();
            }
        }
    }

    public void Load(int value, bool isEditable)
    {
        _isLoading = true;
        try
        {
            IsEditable = isEditable;
            Value = value < 0 ? 0 : value;
            SyncValueText(Value);
            OnPropertyChanged(nameof(PercentText));
        }
        finally
        {
            _isLoading = false;
        }
    }

    private void SyncValueText(int value)
    {
        var text = value.ToString(CultureInfo.InvariantCulture);
        if (_valueText == text)
        {
            return;
        }

        _valueText = text;
        OnPropertyChanged(nameof(ValueText));
    }
}

public sealed class LightCellViewModel : ViewModelBase
{
    private static readonly Brush TopLayerBrush = CreateBrush(0x38, 0xB4, 0xDF);
    private static readonly Brush MiddleLayerBrush = CreateBrush(0xA3, 0xE6, 0x35);
    private static readonly Brush BottomLayerBrush = CreateBrush(0xF5, 0x9E, 0x0B);

    private readonly IAlgorithmLightService _lightService;
    private readonly Action<LightCellViewModel> _changed;
    private bool _isLoading;
    private int _index;
    private int _position;
    private int _operatorType;
    private bool _isSelected;
    private bool _isLastUserCell;
    private bool _isOperatorButtonVisible = true;

    public LightCellViewModel(IAlgorithmLightService lightService, Action<LightCellViewModel> changed)
    {
        _lightService = lightService;
        _changed = changed;
        Channels =
        [
            new LightChannelViewModel("R", "Red", CreateBrush(0xEF, 0x44, 0x44), _lightService.MaximumChannelValue, _ => OnChannelChanged(), channel => SetExclusiveChannel(channel.Code)),
            new LightChannelViewModel("G", "Green", CreateBrush(0x22, 0xC5, 0x5E), _lightService.MaximumChannelValue, _ => OnChannelChanged(), channel => SetExclusiveChannel(channel.Code)),
            new LightChannelViewModel("B", "Blue", CreateBrush(0x3B, 0x82, 0xF6), _lightService.MaximumChannelValue, _ => OnChannelChanged(), channel => SetExclusiveChannel(channel.Code)),
            new LightChannelViewModel("W", "White", CreateBrush(0xF8, 0xFA, 0xFC), _lightService.MaximumChannelValue, _ => OnChannelChanged(), channel => SetExclusiveChannel(channel.Code))
        ];
    }

    public ObservableCollection<LightChannelViewModel> Channels { get; }

    public int Index
    {
        get => _index;
        set
        {
            if (SetProperty(ref _index, value))
            {
                OnPropertyChanged(nameof(DisplayIndex));
                OnPropertyChanged(nameof(HeaderText));
            }
        }
    }

    public int DisplayIndex => Index + 1;

    public string HeaderText => $"{DisplayIndex}. {PositionLabel}";

    public string PositionShortText => Position switch
    {
        AlgorithmLightService.MiddleLight => "MID",
        AlgorithmLightService.BottomLight => "BTM",
        _ => "TOP"
    };

    public Brush PositionBrush => Position switch
    {
        AlgorithmLightService.MiddleLight => MiddleLayerBrush,
        AlgorithmLightService.BottomLight => BottomLayerBrush,
        _ => TopLayerBrush
    };

    public string ChannelSummaryText
    {
        get
        {
            var values = Channels
                .Where(channel => channel.IsEditable && channel.Value > 0)
                .Select(channel => $"{channel.Code}{channel.Value}")
                .ToList();
            return values.Count == 0 ? "OFF" : string.Join(" ", values);
        }
    }

    public string CellToolTip => $"{DisplayIndex}. {PositionLabel} / {ChannelSummaryText}";

    public bool IsSelected
    {
        get => _isSelected;
        set => SetProperty(ref _isSelected, value);
    }

    public bool IsLastUserCell
    {
        get => _isLastUserCell;
        set
        {
            if (SetProperty(ref _isLastUserCell, value))
            {
                OnPropertyChanged(nameof(OperatorToolTip));
            }
        }
    }

    public bool IsOperatorButtonVisible
    {
        get => _isOperatorButtonVisible;
        set
        {
            if (SetProperty(ref _isOperatorButtonVisible, value))
            {
                OnPropertyChanged(nameof(OperatorButtonVisibility));
            }
        }
    }

    public Visibility OperatorButtonVisibility => IsOperatorButtonVisible ? Visibility.Visible : Visibility.Hidden;

    public int Position
    {
        get => _position;
        set
        {
            var next = Math.Max(AlgorithmLightService.TopLight, Math.Min(AlgorithmLightService.BottomLight, value));
            if (!SetProperty(ref _position, next))
            {
                return;
            }

            OnPropertyChanged(nameof(PositionLabel));
            OnPropertyChanged(nameof(PositionShortText));
            OnPropertyChanged(nameof(PositionBrush));
            OnPropertyChanged(nameof(HeaderText));
            RaiseSummaryProperties();
            if (!_isLoading)
            {
                var defaults = _lightService.CreateDefaultUserCell(Position, OperatorType);
                LoadChannels(defaults);
                NotifyChanged();
            }
        }
    }

    public string PositionLabel => Position switch
    {
        AlgorithmLightService.MiddleLight => "MIDDLE",
        AlgorithmLightService.BottomLight => "BOTTOM",
        _ => "TOP"
    };

    public int OperatorType
    {
        get => _operatorType;
        set
        {
            var next = Math.Max(0, Math.Min(2, value));
            if (!SetProperty(ref _operatorType, next))
            {
                return;
            }

            OnPropertyChanged(nameof(OperatorText));
            OnPropertyChanged(nameof(OperatorButtonText));
            OnPropertyChanged(nameof(OperatorToolTip));
            if (!_isLoading)
            {
                NotifyChanged();
            }
        }
    }

    public string OperatorText => OperatorType switch
    {
        1 => "+",
        2 => "-",
        _ => ""
    };

    public string OperatorButtonText => string.IsNullOrEmpty(OperatorText) ? " " : OperatorText;

    public string OperatorToolTip => IsLastUserCell
        ? "Click to add the next User Light cell with + or -."
        : "Click to switch this User Light operator.";

    public void Load(AlgorithmLightCell cell, int index)
    {
        _isLoading = true;
        try
        {
            Index = index;
            Position = cell.Position;
            OperatorType = cell.Operator;
            LoadChannels(cell);
        }
        finally
        {
            _isLoading = false;
        }

        OnPropertyChanged(nameof(PositionLabel));
        OnPropertyChanged(nameof(PositionShortText));
        OnPropertyChanged(nameof(PositionBrush));
        OnPropertyChanged(nameof(HeaderText));
        OnPropertyChanged(nameof(OperatorText));
        OnPropertyChanged(nameof(OperatorButtonText));
        OnPropertyChanged(nameof(OperatorToolTip));
        RaiseSummaryProperties();
    }

    public AlgorithmLightCell ToModel()
    {
        return new AlgorithmLightCell
        {
            Position = Position,
            Operator = OperatorType,
            RedValue = Channels[0].IsEditable ? Channels[0].Value : -1,
            GreenValue = Channels[1].IsEditable ? Channels[1].Value : -1,
            BlueValue = Channels[2].IsEditable ? Channels[2].Value : -1,
            WhiteValue = Channels[3].IsEditable ? Channels[3].Value : -1
        };
    }

    private void LoadChannels(AlgorithmLightCell cell)
    {
        var availability = _lightService.GetChannelAvailability(cell.Position, userCellPosition: true);
        Channels[0].Load(cell.RedValue, availability.Red);
        Channels[1].Load(cell.GreenValue, availability.Green);
        Channels[2].Load(cell.BlueValue, availability.Blue);
        Channels[3].Load(cell.WhiteValue, availability.White);
        RaiseSummaryProperties();
    }

    public bool CanEditChannel(string? code)
    {
        return FindChannel(code)?.IsEditable ?? false;
    }

    public void SetExclusiveChannel(string code)
    {
        var target = FindChannel(code);
        if (target == null || !target.IsEditable)
        {
            return;
        }

        foreach (var channel in Channels.Where(channel => channel.IsEditable))
        {
            channel.Value = string.Equals(channel.Code, target.Code, StringComparison.OrdinalIgnoreCase)
                ? 100
                : 0;
        }
    }

    private LightChannelViewModel? FindChannel(string? code)
    {
        return string.IsNullOrWhiteSpace(code)
            ? null
            : Channels.FirstOrDefault(channel => string.Equals(channel.Code, code, StringComparison.OrdinalIgnoreCase));
    }

    private void OnChannelChanged()
    {
        RaiseSummaryProperties();
        NotifyChanged();
    }

    private void RaiseSummaryProperties()
    {
        OnPropertyChanged(nameof(ChannelSummaryText));
        OnPropertyChanged(nameof(CellToolTip));
    }

    private void NotifyChanged()
    {
        if (!_isLoading)
        {
            _changed(this);
        }
    }

    private static SolidColorBrush CreateBrush(byte r, byte g, byte b)
    {
        var brush = new SolidColorBrush(Color.FromRgb(r, g, b));
        brush.Freeze();
        return brush;
    }
}
