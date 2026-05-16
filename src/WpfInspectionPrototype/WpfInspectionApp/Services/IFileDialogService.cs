using System.Windows;

namespace WpfInspectionApp.Services;

public interface IFileDialogService
{
    string? BrowseImage(Window owner);

    string? BrowsePtt(Window owner);

    string? BrowseModel(Window owner, string initialDirectory);

    string? BrowsePart(Window owner, string initialDirectory);

    // Picks a Pattern algorithm model file (e.g. .mdl/.dat). Filter is broad so the
    // user can pick whatever the native MPTI model format ships as in their setup.
    string? BrowsePatternModel(Window owner);
}
