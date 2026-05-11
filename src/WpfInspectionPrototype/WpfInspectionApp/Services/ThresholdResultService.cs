using WpfInspectionApp.Interop;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class ThresholdResultService : IThresholdResultService
{
    private readonly IInspectionResultTextService _inspectionResultTextService;

    public ThresholdResultService(IInspectionResultTextService inspectionResultTextService)
    {
        _inspectionResultTextService = inspectionResultTextService;
    }

    public ThresholdResultApplyResult Apply(
        InspectionModel model,
        NativeThresholdResponse response,
        string nativeLabel,
        InspectionWindowData? activeWindow,
        InspectionAlgorithmData? activeAlgorithm,
        RoiRect? activeInspectionRoi,
        Func<RoiRect?, string> formatRoi)
    {
        if (activeAlgorithm != null)
        {
            var result = response.Result;
            activeAlgorithm.Result = new InspectionResultData
            {
                Message = result.Message,
                ElapsedMs = result.ElapsedMs,
                ForegroundPixels = result.ForegroundPixels,
                BlobCount = result.BlobCount,
                Bounds = result.ForegroundPixels > 0
                    ? new RoiRect(result.MinX, result.MinY, Math.Max(1, result.MaxX - result.MinX + 1), Math.Max(1, result.MaxY - result.MinY + 1))
                    : null
            };
        }

        var text = _inspectionResultTextService.FormatThresholdResult(
            model,
            response,
            nativeLabel,
            activeWindow,
            activeAlgorithm,
            activeInspectionRoi,
            formatRoi);

        return new ThresholdResultApplyResult(text, activeAlgorithm?.Id);
    }
}
