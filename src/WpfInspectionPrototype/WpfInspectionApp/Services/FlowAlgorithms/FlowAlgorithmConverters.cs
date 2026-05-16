using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace WpfInspectionApp.Services.FlowAlgorithms;

// Tiny converters used by FlowAlgorithm runner XAML. They're scoped to this namespace
// so they don't leak into the global converter pool. Each exposes a singleton via a
// static field so XAML can use {x:Static fa:XxxConverter.NotNull} without the usual
// StaticResource ceremony.

public sealed class NullToBoolConverter : IValueConverter
{
    public static readonly NullToBoolConverter NotNull = new() { _invert = false };

    private bool _invert;

    public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
        => (value != null) ^ _invert;

    public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
        => throw new NotSupportedException();
}

public sealed class NullToVisibilityConverter : IValueConverter
{
    public static readonly NullToVisibilityConverter NotNullToVisible = new();

    public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
        => value != null ? Visibility.Visible : Visibility.Collapsed;

    public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
        => throw new NotSupportedException();
}
