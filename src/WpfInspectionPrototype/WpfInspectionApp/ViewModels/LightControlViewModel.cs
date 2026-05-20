using System.Collections.ObjectModel;
using System.Globalization;
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
    private InspectionAlgorithmData? _algorithm;
    private bool _isLoading;
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
        AddUserCellCommand = _addUserCellCommand;
        RemoveUserCellCommand = _removeUserCellCommand;
        Load(null);
    }

    public ObservableCollection<LightOptionViewModel> LightTypeOptions { get; }

    public ObservableCollection<LightOptionViewModel> PositionOptions { get; }

    public ObservableCollection<LightOptionViewModel> OperatorOptions { get; }

    public ObservableCollection<LightChannelViewModel> NormalChannels { get; }

    public ObservableCollection<LightCellViewModel> UserCells { get; } = [];

    public ICommand AddUserCellCommand { get; }

    public ICommand RemoveUserCellCommand { get; }

    public bool IsEnabled => _algorithm != null;

    public bool IsNormalMode => LightType != AlgorithmLightService.UserLight;

    public bool IsUserMode => LightType == AlgorithmLightService.UserLight;

    public bool HasSelectedUserCell => SelectedUserCell != null;

    public int MaximumUserCellCount => _lightService.MaximumUserCellCount;

    public bool CanAddUserCell => IsEnabled && IsUserMode && UserCells.Count < MaximumUserCellCount;

    public bool CanRemoveUserCell => IsEnabled && IsUserMode && UserCells.Count > 0;

    public string UserCellCountText => $"{UserCells.Count}/{MaximumUserCellCount}";

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
            if (SetProperty(ref _selectedUserCell, value))
            {
                OnPropertyChanged(nameof(HasSelectedUserCell));
                RaiseCommandStates();
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
        UserCells.RemoveAt(removeIndex);
        RefreshCellIndexes();
        SelectedUserCell = UserCells.Count == 0
            ? null
            : UserCells[Math.Min(removeIndex, UserCells.Count - 1)];
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
        }
    }

    private void RaiseModeProperties()
    {
        OnPropertyChanged(nameof(IsNormalMode));
        OnPropertyChanged(nameof(IsUserMode));
        OnPropertyChanged(nameof(CanAddUserCell));
        OnPropertyChanged(nameof(CanRemoveUserCell));
        OnPropertyChanged(nameof(UserCellCountText));
    }

    private void RaiseCommandStates()
    {
        OnPropertyChanged(nameof(CanAddUserCell));
        OnPropertyChanged(nameof(CanRemoveUserCell));
        _addUserCellCommand.RaiseCanExecuteChanged();
        _removeUserCellCommand.RaiseCanExecuteChanged();
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
    private readonly int _maximum;
    private int _value;
    private string _valueText = "0";
    private bool _isEditable = true;
    private bool _isLoading;

    public LightChannelViewModel(string code, string label, Brush swatch, int maximum, Action<LightChannelViewModel> changed)
    {
        Code = code;
        Label = label;
        Swatch = swatch;
        _maximum = maximum;
        _changed = changed;
    }

    public string Code { get; }

    public string Label { get; }

    public Brush Swatch { get; }

    public int Maximum => _maximum;

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
        private set => SetProperty(ref _isEditable, value);
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
    private readonly IAlgorithmLightService _lightService;
    private readonly Action<LightCellViewModel> _changed;
    private bool _isLoading;
    private int _index;
    private int _position;
    private int _operatorType;

    public LightCellViewModel(IAlgorithmLightService lightService, Action<LightCellViewModel> changed)
    {
        _lightService = lightService;
        _changed = changed;
        Channels =
        [
            new LightChannelViewModel("R", "Red", CreateBrush(0xEF, 0x44, 0x44), _lightService.MaximumChannelValue, _ => NotifyChanged()),
            new LightChannelViewModel("G", "Green", CreateBrush(0x22, 0xC5, 0x5E), _lightService.MaximumChannelValue, _ => NotifyChanged()),
            new LightChannelViewModel("B", "Blue", CreateBrush(0x3B, 0x82, 0xF6), _lightService.MaximumChannelValue, _ => NotifyChanged()),
            new LightChannelViewModel("W", "White", CreateBrush(0xF8, 0xFA, 0xFC), _lightService.MaximumChannelValue, _ => NotifyChanged())
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
            OnPropertyChanged(nameof(HeaderText));
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
        OnPropertyChanged(nameof(HeaderText));
        OnPropertyChanged(nameof(OperatorText));
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
