using System.Windows;

namespace WpfInspectionApp.Services;

public interface IFileDialogService
{
    string? BrowseImage(Window owner);

    string? BrowsePtt(Window owner);

    string? BrowseModel(Window owner, string initialDirectory);

    string? BrowsePart(Window owner, string initialDirectory);
}
