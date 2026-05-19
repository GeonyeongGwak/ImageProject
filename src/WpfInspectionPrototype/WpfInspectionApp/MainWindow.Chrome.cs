using System.Windows;

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
}
