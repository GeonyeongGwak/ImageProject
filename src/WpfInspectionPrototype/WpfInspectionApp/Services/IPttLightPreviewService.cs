using System.Windows.Media;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed record PttLightPreviewResult(
    bool Success,
    bool Available,
    string StatusMessage,
    LoadedImageFrame? Frame);

// CAM-01 컬러 미리보기 결과. Light 영향 없음, PTT 로드 직후 1 회만 계산해서 사용.
public sealed record PttColorPreviewResult(
    bool Success,
    bool Available,
    string StatusMessage,
    ImageSource? ColorImage,
    int Width,
    int Height);

public interface IPttLightPreviewService
{
    PttLightPreviewResult Render(AlgorithmLightState state, int width, int height);

    // PTT 의 TR/TG/TB 채널을 그대로 BGRA32 로 합성한 컬러 이미지. Light 와 무관.
    PttColorPreviewResult RenderColor(int width, int height);
}
