using Microsoft.Win32;
using System.Windows;

namespace WpfInspectionApp.Services;

public sealed class FileDialogService : IFileDialogService
{
    public string? BrowseImage(Window owner)
    {
        return Browse(
            owner,
            "Load 2D image",
            "Image Files|*.jpg;*.jpeg;*.png;*.bmp;*.tif;*.tiff",
            initialDirectory: null);
    }

    public string? BrowsePtt(Window owner)
    {
        return Browse(owner, "Load 3D PTT", "PTT 3D Files|*.ptt|All Files|*.*", initialDirectory: null);
    }

    public string? BrowseModel(Window owner, string initialDirectory)
    {
        return Browse(owner, "Load model", "Model JSON|*.json|All Files|*.*", initialDirectory);
    }

    public string? BrowsePart(Window owner, string initialDirectory)
    {
        return Browse(
            owner,
            "Import part",
            "Part or Model JSON/ZIP/XML|*.json;*.zip;*.xml|JSON|*.json|ZIP|*.zip|Legacy RawData XML|*.xml|All Files|*.*",
            initialDirectory);
    }

    public string? BrowsePatternModel(Window owner)
    {
        return Browse(
            owner,
            "Load Pattern model",
            "Pattern model|*.mdl;*.dat;*.pat|All Files|*.*",
            initialDirectory: null);
    }

    private static string? Browse(Window owner, string title, string filter, string? initialDirectory)
    {
        var dialog = new OpenFileDialog
        {
            Title = title,
            Filter = filter
        };

        if (!string.IsNullOrWhiteSpace(initialDirectory))
        {
            dialog.InitialDirectory = initialDirectory;
        }

        return dialog.ShowDialog(owner) == true ? dialog.FileName : null;
    }
}
