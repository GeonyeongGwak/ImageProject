using WpfInspectionApp.Interop;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed record PreviewThresholdResult(
    byte[] Pixels,
    int Width,
    int Height,
    int Stride,
    NativeThresholdResponse Response,
    double UiElapsedMs);

public interface IPreviewProcessingService
{
    Task<PreviewThresholdResult> ThresholdAsync(
        byte[] source,
        int width,
        int height,
        int sourceStride,
        int threshold,
        RoiRect? roi,
        CancellationToken cancellationToken);
}
