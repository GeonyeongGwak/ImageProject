using System.Windows.Media;
using WpfInspectionApp.Infrastructure;

namespace WpfInspectionApp.ViewModels;

public sealed class ThemeOptionViewModel : ViewModelBase
{
    private bool _isSelected;
    private Brush _buttonBackground;
    private Brush _buttonBorderBrush;

    public ThemeOptionViewModel(string key, string displayName, Color swatchColor)
    {
        Key = key;
        DisplayName = displayName;
        SwatchBrush = new SolidColorBrush(swatchColor);
        _buttonBackground = new SolidColorBrush(Color.FromArgb(22, swatchColor.R, swatchColor.G, swatchColor.B));
        _buttonBorderBrush = new SolidColorBrush(Color.FromArgb(120, swatchColor.R, swatchColor.G, swatchColor.B));
    }

    public string Key { get; }

    public string DisplayName { get; }

    public Brush SwatchBrush { get; }

    public bool IsSelected
    {
        get => _isSelected;
        set
        {
            if (!SetProperty(ref _isSelected, value))
            {
                return;
            }

            UpdateSelectionBrushes();
        }
    }

    public Brush ButtonBackground
    {
        get => _buttonBackground;
        private set => SetProperty(ref _buttonBackground, value);
    }

    public Brush ButtonBorderBrush
    {
        get => _buttonBorderBrush;
        private set => SetProperty(ref _buttonBorderBrush, value);
    }

    private void UpdateSelectionBrushes()
    {
        if (SwatchBrush is not SolidColorBrush swatch)
        {
            return;
        }

        var color = swatch.Color;
        ButtonBackground = new SolidColorBrush(Color.FromArgb(IsSelected ? (byte)84 : (byte)22, color.R, color.G, color.B));
        ButtonBorderBrush = new SolidColorBrush(Color.FromArgb(IsSelected ? (byte)255 : (byte)120, color.R, color.G, color.B));
    }
}
