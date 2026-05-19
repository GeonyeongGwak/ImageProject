using System.Linq;
using System.Windows;
using System.Windows.Media;

namespace WpfInspectionApp;

public partial class MainWindow
{
    private void ApplyTheme(string themeKey)
    {
        var palette = ThemePalette.For(themeKey);

        SetThemeBrush("WindowBackground", palette.WindowBackground);
        SetThemeBrush("TopBarBackground", palette.TopBarBackground);
        SetThemeBrush("TopBarBorder", palette.TopBarBorder);
        SetThemeBrush("PanelBackground", palette.PanelBackground);
        SetThemeBrush("BorderBlue", palette.Border);
        SetThemeBrush("AccentBlue", palette.Accent);
        SetThemeBrush("AccentGreen", palette.Green);
        SetThemeBrush("AccentYellow", palette.Yellow);
        SetThemeBrush("PrimaryText", palette.PrimaryText);
        SetThemeBrush("SecondaryText", palette.SecondaryText);
        SetThemeBrush("MutedText", palette.MutedText);
        SetThemeBrush("DisabledText", palette.DisabledText);
        SetThemeBrush("SurfaceBackground", palette.SurfaceBackground);
        SetThemeBrush("ControlBackground", palette.ControlBackground);
        SetThemeBrush("ControlHover", palette.ControlHover);
        SetThemeBrush("ControlBorder", palette.ControlBorder);
        SetThemeBrush("TreeHeaderBackground", palette.TreeHeaderBackground);
        SetThemeBrush("TreeSelectedBackground", palette.TreeSelectedBackground);
        SetThemeBrush("ComboDarkBackground", palette.InputBackground);
        SetThemeBrush("ComboDarkHover", palette.ControlHover);
        SetThemeBrush("ComboDarkSelected", palette.SelectionBackground);

        SetThemeBrush(SystemColors.WindowBrushKey, palette.InputBackground);
        SetThemeBrush(SystemColors.WindowTextBrushKey, palette.PrimaryText);
        SetThemeBrush(SystemColors.HighlightBrushKey, palette.SelectionBackground);
        SetThemeBrush(SystemColors.HighlightTextBrushKey, palette.SelectionText);

        if (_pttViewerPanel != null)
        {
            _pttViewerPanel.BackColor = System.Drawing.Color.FromArgb(
                palette.SurfaceBackground.R,
                palette.SurfaceBackground.G,
                palette.SurfaceBackground.B);
        }

        foreach (var floatingWindow in _cameraDockStates.Values.Select(state => state.FloatingWindow).Where(window => window != null))
        {
            floatingWindow!.Background = Resources["WindowBackground"] as Brush ?? new SolidColorBrush(palette.WindowBackground);
        }
    }

    private void SetThemeBrush(object key, Color color)
    {
        if (Resources.Contains(key) && Resources[key] is SolidColorBrush brush)
        {
            brush.Color = color;
            return;
        }

        Resources[key] = new SolidColorBrush(color);
    }

    private sealed record ThemePalette(
        Color WindowBackground,
        Color TopBarBackground,
        Color TopBarBorder,
        Color PanelBackground,
        Color Border,
        Color Accent,
        Color Green,
        Color Yellow,
        Color PrimaryText,
        Color SecondaryText,
        Color MutedText,
        Color DisabledText,
        Color SurfaceBackground,
        Color ControlBackground,
        Color ControlHover,
        Color ControlBorder,
        Color TreeHeaderBackground,
        Color TreeSelectedBackground,
        Color InputBackground,
        Color SelectionBackground,
        Color SelectionText)
    {
        public static ThemePalette For(string key)
        {
            return key switch
            {
                "Light" => new ThemePalette(
                    Rgb(244, 247, 251),
                    Rgb(255, 255, 255),
                    Rgb(167, 198, 232),
                    Rgb(255, 255, 255),
                    Rgb(182, 207, 234),
                    Rgb(17, 104, 182),
                    Rgb(6, 122, 72),
                    Rgb(164, 101, 0),
                    Rgb(16, 32, 51),
                    Rgb(32, 50, 74),
                    Rgb(94, 113, 138),
                    Rgb(140, 160, 181),
                    Rgb(236, 241, 247),
                    Rgb(232, 241, 251),
                    Rgb(216, 232, 248),
                    Rgb(146, 180, 216),
                    Rgb(232, 241, 251),
                    Rgb(210, 232, 255),
                    Rgb(255, 255, 255),
                    Rgb(47, 128, 208),
                    Rgb(255, 255, 255)),
                "Pink" => new ThemePalette(
                    Rgb(24, 11, 22),
                    Rgb(38, 16, 33),
                    Rgb(217, 92, 165),
                    Rgb(33, 16, 31),
                    Rgb(169, 59, 126),
                    Rgb(255, 112, 189),
                    Rgb(57, 217, 138),
                    Rgb(255, 209, 102),
                    Rgb(255, 240, 250),
                    Rgb(250, 213, 236),
                    Rgb(212, 163, 196),
                    Rgb(134, 96, 119),
                    Rgb(8, 4, 9),
                    Rgb(52, 23, 46),
                    Rgb(71, 33, 62),
                    Rgb(169, 59, 126),
                    Rgb(52, 23, 46),
                    Rgb(90, 33, 74),
                    Rgb(27, 13, 25),
                    Rgb(186, 62, 136),
                    Rgb(255, 255, 255)),
                _ => new ThemePalette(
                    Rgb(7, 11, 18),
                    Rgb(17, 24, 39),
                    Rgb(31, 112, 184),
                    Rgb(11, 19, 32),
                    Rgb(21, 95, 164),
                    Rgb(39, 166, 255),
                    Rgb(24, 224, 123),
                    Rgb(255, 176, 32),
                    Rgb(228, 243, 255),
                    Rgb(214, 236, 255),
                    Rgb(139, 165, 196),
                    Rgb(99, 116, 138),
                    Rgb(2, 5, 10),
                    Rgb(16, 36, 61),
                    Rgb(16, 36, 61),
                    Rgb(35, 110, 168),
                    Rgb(13, 32, 56),
                    Rgb(18, 58, 92),
                    Rgb(7, 16, 28),
                    Rgb(14, 94, 168),
                    Rgb(255, 255, 255))
            };
        }

        private static Color Rgb(byte red, byte green, byte blue)
        {
            return Color.FromRgb(red, green, blue);
        }
    }
}
