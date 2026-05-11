using System.Windows.Media;

namespace WpfInspectionApp.Services;

public sealed record ImageLoadWorkflowResult(
    ImageSource SourceImage,
    ImageSource BinaryImage,
    int Width,
    int Height,
    string StatusMessage);

public interface IImageLoadWorkflowService
{
    ImageLoadWorkflowResult Load(string path);
}
