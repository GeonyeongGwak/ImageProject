using System.Diagnostics;
using WpfInspectionApp.Interop;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class PreviewProcessingService : IPreviewProcessingService
{
    public Task<PreviewThresholdResult> ThresholdAsync(
        byte[] source,
        int width,
        int height,
        int sourceStride,
        int threshold,
        RoiRect? roi,
        CancellationToken cancellationToken)
    {
        return Task.Run(
            () =>
            {
                cancellationToken.ThrowIfCancellationRequested();
                var destinationStride = width;
                var destination = new byte[destinationStride * height];
                var stopwatch = Stopwatch.StartNew();
                var response = NativeInspectionBridge.ThresholdBgra(
                    source,
                    width,
                    height,
                    sourceStride,
                    destination,
                    destinationStride,
                    threshold,
                    roi);
                stopwatch.Stop();
                cancellationToken.ThrowIfCancellationRequested();
                return new PreviewThresholdResult(destination, width, height, destinationStride, response, stopwatch.Elapsed.TotalMilliseconds);
            },
            cancellationToken);
    }
}
