namespace WpfInspectionApp.Services;

public interface IApplicationPathService
{
    string? FindDefaultImagePath();

    string GetModelDirectory();
}
