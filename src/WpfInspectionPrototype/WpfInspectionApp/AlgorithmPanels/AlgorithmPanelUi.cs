using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Media;

namespace WpfInspectionApp.AlgorithmPanels;

public static class AlgorithmPanelUi
{
    public static Border RootBorder(StackPanel content)
    {
        return new Border
        {
            Padding = new Thickness(8),
            Margin = new Thickness(0, 0, 0, 10),
            Background = Brush("#0B1320"),
            BorderBrush = Brush("#155FA4"),
            BorderThickness = new Thickness(1),
            CornerRadius = new CornerRadius(4),
            Child = content
        };
    }

    public static TextBlock Text(string text, double size, string color, FontWeight weight)
    {
        return new TextBlock
        {
            Text = text,
            FontSize = size,
            Foreground = Brush(color),
            FontWeight = weight,
            TextWrapping = TextWrapping.Wrap,
            Margin = new Thickness(0, 0, 0, 4)
        };
    }

    public static Button Button(string content, Action action)
    {
        var button = new Button
        {
            Content = content,
            MinHeight = 28,
            Margin = new Thickness(0, 0, 4, 4),
            Padding = new Thickness(8, 3, 8, 3),
            Foreground = Brush("#E4F3FF"),
            Background = Brush("#10243D"),
            BorderBrush = Brush("#236EA8"),
            FontWeight = FontWeights.SemiBold
        };
        button.Click += (_, _) => action();
        return button;
    }

    public static CheckBox Check(string label, string currentValue, bool fallback, Action<string> write)
    {
        var check = new CheckBox
        {
            Content = label,
            IsChecked = currentValue.Equals("true", StringComparison.OrdinalIgnoreCase),
            Foreground = Brush("#D6E8FF"),
            FontWeight = FontWeights.SemiBold,
            Margin = new Thickness(0, 2, 0, 2)
        };
        check.Checked += (_, _) => write("true");
        check.Unchecked += (_, _) => write("false");
        if (string.IsNullOrWhiteSpace(currentValue))
        {
            check.IsChecked = fallback;
        }

        return check;
    }

    public static FrameworkElement Number(string label, string currentValue, Action<string> write)
    {
        var box = new TextBox
        {
            Text = currentValue,
            Height = 26,
            Padding = new Thickness(6, 2, 6, 2),
            Foreground = Brush("#E4F3FF"),
            Background = Brush("#07101C"),
            BorderBrush = Brush("#284A72")
        };
        box.TextChanged += (_, _) => write(box.Text);
        return Row(label, box);
    }

    public static FrameworkElement Slider(string label, string currentValue, int min, int max, string fallback, Action<string> write, double labelWidth = 90)
    {
        var initial = int.TryParse(currentValue, NumberStyles.Integer, CultureInfo.InvariantCulture, out var value)
            ? value
            : int.Parse(fallback, CultureInfo.InvariantCulture);
        var valueText = Text(initial.ToString(CultureInfo.InvariantCulture), 12, "#FFB020", FontWeights.Bold);
        var slider = new Slider
        {
            Minimum = min,
            Maximum = max,
            Value = Net48Compat.Clamp(initial, min, max),
            TickFrequency = 1
        };
        slider.ValueChanged += (_, args) =>
        {
            var next = ((int)Math.Round(args.NewValue)).ToString(CultureInfo.InvariantCulture);
            valueText.Text = next;
            write(next);
        };

        var grid = new Grid { Margin = new Thickness(0, 2, 6, 6) };
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(labelWidth) });
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(46) });
        grid.Children.Add(Text(label, 12, "#9DB4D0", FontWeights.Normal));
        Grid.SetColumn(slider, 1);
        grid.Children.Add(slider);
        Grid.SetColumn(valueText, 2);
        grid.Children.Add(valueText);
        return grid;
    }

    public static FrameworkElement Combo(string label, string[] values, int selectedIndex, Action<int> write)
    {
        var combo = new ComboBox
        {
            Height = 26,
            Foreground = Brush("#E4F3FF"),
            Background = Brush("#07101C"),
            BorderBrush = Brush("#284A72"),
            ItemsSource = values,
            SelectedIndex = Net48Compat.Clamp(selectedIndex, 0, Math.Max(0, values.Length - 1))
        };
        combo.SelectionChanged += (_, _) => write(Math.Max(0, combo.SelectedIndex));
        return Row(label, combo);
    }

    public static FrameworkElement Row(string label, FrameworkElement editor)
    {
        var grid = new Grid { Margin = new Thickness(0, 2, 6, 6) };
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(92) });
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
        grid.Children.Add(Text(label, 12, "#9DB4D0", FontWeights.Normal));
        Grid.SetColumn(editor, 1);
        grid.Children.Add(editor);
        return grid;
    }

    public static StackPanel TabPanel()
    {
        return new StackPanel { Margin = new Thickness(0, 8, 0, 0) };
    }

    public static UniformGrid EditGrid(int columns = 2)
    {
        return new UniformGrid
        {
            Columns = columns,
            Margin = new Thickness(0, 6, 0, 0)
        };
    }

    public static TabItem Tab(string header, FrameworkElement content)
    {
        return new TabItem
        {
            Header = header,
            Content = content,
            MinWidth = 100,
            Padding = new Thickness(10, 5, 10, 5)
        };
    }

    public static SolidColorBrush Brush(string color)
    {
        return new SolidColorBrush((Color)ColorConverter.ConvertFromString(color));
    }
}
