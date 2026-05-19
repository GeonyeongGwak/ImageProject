using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;
using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.ViewModels;

public enum InspectionTreeNodeKind
{
    Empty,
    Window,
    WindowInfo,
    Algorithm,
    AlgorithmRoi,
    InspectionResult
}

public sealed class AlgorithmMenuItemViewModel
{
    public AlgorithmMenuItemViewModel(string type, string displayName, ICommand command)
    {
        Type = type;
        DisplayName = displayName;
        Command = command;
        IconGeometry = Geometry.Parse(CreateIconPath(type, displayName));
        IconBrush = CreateIconBrush(type, displayName);
    }

    public string Type { get; }

    public string DisplayName { get; }

    public ICommand Command { get; }

    public Geometry IconGeometry { get; }

    public Brush IconBrush { get; }

    private static string CreateIconPath(string type, string displayName)
    {
        var key = $"{type} {displayName}".ToLowerInvariant();

        if (key.Contains("align"))
        {
            return "M8,2 L8,14 M2,8 L14,8 M4,4 L12,12 M12,4 L4,12";
        }

        if (key.Contains("blob") || key.Contains("bga") || key.Contains("bump"))
        {
            return "M4,7 L6,4 L10,4 L12,7 L11,11 L8,13 L5,11 Z M6,8 L10,8";
        }

        if (key.Contains("ocr") || key.Contains("barcode"))
        {
            return key.Contains("barcode")
                ? "M3,3 L3,13 M5,3 L5,13 M8,3 L8,13 M10,3 L10,13 M13,3 L13,13"
                : "M3,3 L13,3 L13,13 L3,13 Z M5,11 L7,5 L9,11 M6,9 L8,9 M10,5 L10,11 L12,11";
        }

        if (key.Contains("color"))
        {
            return "M3,3 L13,3 L13,13 L3,13 Z M3,8 L13,8 M8,3 L8,13 M3,3 L13,13";
        }

        if (key.Contains("height") || key.Contains("volume") || key.Contains("package"))
        {
            return "M3,13 L3,9 L5,9 L5,13 M7,13 L7,5 L9,5 L9,13 M11,13 L11,7 L13,7 L13,13";
        }

        if (key.Contains("lead") || key.Contains("pad") || key.Contains("tab"))
        {
            return "M3,5 L13,5 M4,5 L4,12 M6,5 L6,12 M8,5 L8,12 M10,5 L10,12 M12,5 L12,12";
        }

        if (key.Contains("pattern") || key.Contains("grid"))
        {
            return "M3,3 L13,3 L13,13 L3,13 Z M5,3 L5,13 M8,3 L8,13 M11,3 L11,13 M3,5 L13,5 M3,8 L13,8 M3,11 L13,11";
        }

        if (key.Contains("edge") || key.Contains("line") || key.Contains("width") || key.Contains("distance") || key.Contains("shape"))
        {
            return "M4,3 L12,3 L12,13 M4,13 L12,13 M3,8 L13,8";
        }

        if (key.Contains("wire"))
        {
            return "M2,12 C5,3 11,13 14,4";
        }

        if (key.Contains("solder") || key.Contains("fillet") || key.Contains("foot"))
        {
            return "M8,3 C12,6 13,10 8,13 C3,10 4,6 8,3 Z";
        }

        if (key.Contains("tilt"))
        {
            return "M3,12 L13,6 M10,4 L13,6 L12,9 M4,5 L7,5";
        }

        if (key.Contains("bridge"))
        {
            return "M3,11 L6,5 L10,5 L13,11 M5,9 L11,9";
        }

        return "M8,2 L14,8 L8,14 L2,8 Z";
    }

    private static Brush CreateIconBrush(string type, string displayName)
    {
        var key = $"{type} {displayName}".ToLowerInvariant();
        var color = key switch
        {
            var value when value.Contains("align") => "#8FD14F",
            var value when value.Contains("blob") || value.Contains("bga") || value.Contains("bump") => "#F97316",
            var value when value.Contains("ocr") || value.Contains("barcode") => "#E5E7EB",
            var value when value.Contains("color") => "#FB7185",
            var value when value.Contains("height") || value.Contains("volume") || value.Contains("package") => "#FBBF24",
            var value when value.Contains("lead") || value.Contains("pad") || value.Contains("tab") => "#A78BFA",
            var value when value.Contains("pattern") || value.Contains("grid") => "#34D399",
            var value when value.Contains("edge") || value.Contains("line") || value.Contains("width") || value.Contains("distance") || value.Contains("shape") => "#22C55E",
            var value when value.Contains("wire") => "#F472B6",
            var value when value.Contains("solder") || value.Contains("fillet") || value.Contains("foot") => "#F59E0B",
            var value when value.Contains("tilt") => "#C084FC",
            var value when value.Contains("bridge") => "#FACC15",
            _ => "#94A3B8"
        };

        return new SolidColorBrush((Color)ColorConverter.ConvertFromString(color));
    }
}

public sealed class InspectionTreeNodeViewModel : ViewModelBase
{
    private bool _isSelected;
    private bool _isInspectionEnabled = true;
    private bool _isEssential;
    private bool _isGrouped;
    private string _nameText = "";

    public string Header
    {
        get => NameText;
        init => _nameText = value;
    }

    public string NameText
    {
        get => _nameText;
        set
        {
            if (string.IsNullOrWhiteSpace(value))
            {
                OnPropertyChanged();
                return;
            }

            var next = value.Trim();
            if (!SetProperty(ref _nameText, next))
            {
                return;
            }

            if (Payload is InspectionWindowData window)
            {
                window.Name = next;
            }

            OnPropertyChanged(nameof(Header));
        }
    }

    public string TypeText { get; init; } = "";

    public string GroupId { get; init; } = "";

    public InspectionTreeNodeKind Kind { get; init; }

    public object? Payload { get; init; }

    public bool IsEnabled { get; init; } = true;

    private bool _isExpanded = true;
    public bool IsExpanded
    {
        // INPC 필요: ExpandAllNodes/CollapseAllNodes 명령이 코드에서 이 값을 바꿀 때
        // TreeViewItem.IsExpanded TwoWay 바인딩이 알림을 받아 트리가 실제로 펼침/접힘
        // 되어야 함. 단순 auto-property 면 setter 호출이 무시되어 UI 가 안 바뀜.
        get => _isExpanded;
        set => SetProperty(ref _isExpanded, value);
    }

    public Brush? Foreground { get; init; }

    public ObservableCollection<InspectionTreeNodeViewModel> Children { get; } = [];

    public ObservableCollection<AlgorithmMenuItemViewModel> AlgorithmMenuItems { get; } = [];

    public bool HasAlgorithmMenu => Kind == InspectionTreeNodeKind.Window;

    public Visibility GridCellVisibility => Kind is InspectionTreeNodeKind.Window or InspectionTreeNodeKind.Algorithm
        ? Visibility.Visible
        : Visibility.Hidden;

    public Visibility GroupIdVisibility => Kind == InspectionTreeNodeKind.Window
        ? Visibility.Visible
        : Visibility.Hidden;

    public bool CanEditGroup => Kind == InspectionTreeNodeKind.Algorithm;

    public Thickness NamePadding => Kind == InspectionTreeNodeKind.Algorithm
        ? new Thickness(20, 0, 0, 0)
        : new Thickness(2, 0, 0, 0);

    public Visibility NameEditorVisibility => Kind == InspectionTreeNodeKind.Window
        ? Visibility.Visible
        : Visibility.Collapsed;

    public Visibility NameDisplayVisibility => Kind == InspectionTreeNodeKind.Window
        ? Visibility.Collapsed
        : Visibility.Visible;

    public bool IsInspectionEnabled
    {
        get => _isInspectionEnabled;
        set
        {
            if (!SetProperty(ref _isInspectionEnabled, value))
            {
                return;
            }

            switch (Payload)
            {
                case InspectionWindowData window:
                    window.IsEnabled = value;
                    break;
                case InspectionAlgorithmData algorithm:
                    WriteBool(algorithm, "Common.bAlgoEnable", value);
                    break;
            }
        }
    }

    public bool IsEssential
    {
        get => _isEssential;
        set
        {
            if (!SetProperty(ref _isEssential, value))
            {
                return;
            }

            switch (Payload)
            {
                case InspectionWindowData:
                    foreach (var child in Children.Where(child => child.Kind == InspectionTreeNodeKind.Algorithm))
                    {
                        child.IsEssential = value;
                    }
                    break;
                case InspectionAlgorithmData algorithm:
                    WriteBool(algorithm, "Common.IsRequired", value);
                    break;
            }
        }
    }

    public bool IsGrouped
    {
        get => _isGrouped;
        set
        {
            if (!SetProperty(ref _isGrouped, value))
            {
                return;
            }

            switch (Payload)
            {
                case InspectionWindowData window:
                    window.IsGroup = value;
                    break;
                case InspectionAlgorithmData algorithm:
                    WriteBool(algorithm, "Common.bAlgoGroup", value);
                    break;
            }
        }
    }

    public bool IsSelected
    {
        get => _isSelected;
        set => SetProperty(ref _isSelected, value);
    }

    public static bool ReadBool(InspectionAlgorithmData algorithm, string key, bool fallback)
    {
        if (algorithm.Parameters == null || !algorithm.Parameters.TryGetValue(key, out var raw))
        {
            return fallback;
        }

        if (bool.TryParse(raw, out var parsed))
        {
            return parsed;
        }

        if (int.TryParse(raw, out var number))
        {
            return number != 0;
        }

        return fallback;
    }

    private static void WriteBool(InspectionAlgorithmData algorithm, string key, bool value)
    {
        algorithm.Parameters ??= [];
        algorithm.Parameters[key] = value ? "true" : "false";
    }
}
