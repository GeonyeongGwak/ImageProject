using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Shell;

namespace WpfInspectionApp;

// Window chrome (Min/Max/Close) handlers. Pure View concern — these manipulate the
// Window's own state, not the model or ViewModel, so MVVM is unaffected. Kept in a
// partial file alongside MainWindow.Theme.cs so the chrome plumbing stays near the
// theme/resource plumbing.
public partial class MainWindow
{
    private void ChromeMinimizeButton_Click(object sender, RoutedEventArgs e)
    {
        SystemCommands.MinimizeWindow(this);
    }

    private void ChromeMaximizeButton_Click(object sender, RoutedEventArgs e)
    {
        // Toggle between Maximized and Normal. SystemCommands handles the actual
        // window-state transition through user32 — no manual WindowState assignment
        // (which would not animate / interact properly with WindowChrome resize
        // borders).
        if (WindowState == WindowState.Maximized)
        {
            SystemCommands.RestoreWindow(this);
            ChromeMaximizeButton.Content = "";  // Segoe MDL2 'Maximize'
            ChromeMaximizeButton.ToolTip = "Maximize";
        }
        else
        {
            SystemCommands.MaximizeWindow(this);
            ChromeMaximizeButton.Content = "";  // Segoe MDL2 'Restore'
            ChromeMaximizeButton.ToolTip = "Restore";
        }
    }

    private void ChromeCloseButton_Click(object sender, RoutedEventArgs e)
    {
        SystemCommands.CloseWindow(this);
    }

    // ---------- Floating camera window chrome ----------
    //
    // Wraps a floating window's content in a Grid with a custom title bar (using the
    // same DynamicResource theme keys as MainWindow's TopBar) and installs WindowChrome.
    // Result: floating windows lose the white OS title bar and follow the active theme.
    //
    // MVVM note: this is pure View-layer composition. No ViewModel/Model interaction.
    private static void ApplyFloatingWindowChrome(Window window, FrameworkElement content, string title)
    {
        // 1. WindowChrome: caption area = our title bar height (40 + 2 border). Resize
        //    borders kept so the window can still be resized from edges.
        WindowChrome.SetWindowChrome(window, new WindowChrome
        {
            CaptionHeight = 42,
            GlassFrameThickness = new Thickness(0),
            ResizeBorderThickness = new Thickness(6),
            CornerRadius = new CornerRadius(0),
            UseAeroCaptionButtons = false
        });

        // 2. Build the title bar: title text on the left, system buttons on the right.
        //    All colors are DynamicResource so ApplyTheme repaints them automatically.
        var titleBar = new Border
        {
            Background = (Brush?)Application.Current?.TryFindResource("TopBarBackground"),
            BorderBrush = (Brush?)Application.Current?.TryFindResource("TopBarBorder"),
            BorderThickness = new Thickness(0, 0, 0, 1),
            Padding = new Thickness(12, 0, 0, 0),
            Height = 40
        };
        titleBar.SetResourceReference(Border.BackgroundProperty, "TopBarBackground");
        titleBar.SetResourceReference(Border.BorderBrushProperty, "TopBarBorder");

        var titleGrid = new Grid();
        titleGrid.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto });
        titleGrid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
        titleGrid.ColumnDefinitions.Add(new ColumnDefinition { Width = GridLength.Auto });

        var titleText = new TextBlock
        {
            Text = title,
            VerticalAlignment = VerticalAlignment.Center,
            FontWeight = FontWeights.SemiBold,
            FontSize = 13
        };
        titleText.SetResourceReference(TextBlock.ForegroundProperty, "PrimaryText");
        Grid.SetColumn(titleText, 0);
        titleGrid.Children.Add(titleText);

        var systemButtons = new StackPanel { Orientation = Orientation.Horizontal, VerticalAlignment = VerticalAlignment.Center };
        WindowChrome.SetIsHitTestVisibleInChrome(systemButtons, true);

        var minButton = CreateChromeButton("", "Minimize", "ChromeSystemButton", (_, _) => SystemCommands.MinimizeWindow(window));
        var maxButton = CreateChromeButton("", "Maximize", "ChromeSystemButton", (s, _) =>
        {
            if (window.WindowState == WindowState.Maximized)
            {
                SystemCommands.RestoreWindow(window);
                if (s is Button b) { b.Content = ""; b.ToolTip = "Maximize"; }
            }
            else
            {
                SystemCommands.MaximizeWindow(window);
                if (s is Button b) { b.Content = ""; b.ToolTip = "Restore"; }
            }
        });
        var closeButton = CreateChromeButton("", "Close", "ChromeSystemCloseButton", (_, _) => SystemCommands.CloseWindow(window));

        systemButtons.Children.Add(minButton);
        systemButtons.Children.Add(maxButton);
        systemButtons.Children.Add(closeButton);
        Grid.SetColumn(systemButtons, 2);
        titleGrid.Children.Add(systemButtons);

        titleBar.Child = titleGrid;

        // 3. Wrap in 2-row grid: title bar (Auto) + original content (*).
        var rootGrid = new Grid();
        rootGrid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
        rootGrid.RowDefinitions.Add(new RowDefinition { Height = new GridLength(1, GridUnitType.Star) });

        Grid.SetRow(titleBar, 0);
        rootGrid.Children.Add(titleBar);

        Grid.SetRow(content, 1);
        rootGrid.Children.Add(content);

        window.Content = rootGrid;
    }

    private static Button CreateChromeButton(string glyph, string tooltip, string styleKey, RoutedEventHandler onClick)
    {
        var button = new Button { Content = glyph, ToolTip = tooltip };
        button.SetResourceReference(Button.StyleProperty, styleKey);
        WindowChrome.SetIsHitTestVisibleInChrome(button, true);
        button.Click += onClick;
        return button;
    }
}
