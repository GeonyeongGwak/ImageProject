# WPF Inspection App Handoff - Default Image 제거 + ROI mm 환산

작성일: 2026-05-20
프로젝트: `C:\Users\USER\Documents\NewProject\ImageProject`
선행 문서:
- `docs/2026-05-20-handoff-align-light-mvvm.md`
- `docs/2026-05-20-handoff-cam03-toggle.md`
- `docs/2026-05-20-handoff-cam01-color.md`
- `docs/2026-05-20-handoff-window-roi-style.md`
- `docs/2026-05-20-handoff-import-progress.md`
- `docs/2026-05-20-handoff-topbar-theme-combobox.md`

## 1. 작업 원칙

- MVVM 위반 여부는 기능 추가 전/후 항상 확인한다.
- View/XAML/code-behind 에는 화면 이벤트 연결, 바인딩, UI 표시만 둔다.
- 값 검증, enable/disable 상태 판단, 파라미터 저장/정규화는 ViewModel/Model/Service.
- 기존 사용자가 만든 변경은 되돌리지 않는다. `git reset --hard`, `git checkout --` 금지.

## 2. 이번 세션에서 한 작업

### 2.1 사용자 요청

1. 기본 이미지 (Image/2D.jpg) 로 2D 출력하는 자동 로딩 제거.
2. ROI 라벨이 픽셀 단위 (`px`) 가 아닌 `m_resolution_X / m_resolution_Y` 값을 사용해 mm 로 환산해서 보여주기.

### 2.2 기본 2D 이미지 자동 로딩 제거

`MainWindow.xaml.cs` 의 `RunStartupLoadWork()` 안에서:

기존:
```csharp
var defaultImage = _applicationPathService.FindDefaultImagePath();
if (defaultImage != null)
{
    if (App.StartupStabilityGuardsEnabled) { ... 스킵 ... }
    else { ViewModel.LoadImageFromPath(defaultImage); }
}
else
{
    ViewModel.StatusMessage = "Default Image/2D.jpg was not found.";
}
```

변경 후:
```csharp
// 기본 2D 이미지 (Image/2D.jpg) 자동 로딩은 사용자 요청으로 제거.
// CAM-01 / CAM-03 영역은 시작 시 비어있고, Part Import / Load PTT 후에만 채워짐.
ViewModel.StatusMessage = "Ready. Use Part Import or Load PTT to begin.";
```

`_applicationPathService.FindDefaultImagePath()` 자체는 다른 호출 가능성이 있어 남겨둠. 시작 경로의 자동 호출만 제거.

### 2.3 ROI 라벨 — 픽셀 → mm 환산

`Views/RoiOverlayRenderer.cs` 의 `DrawSingleRoi` 안에서 라벨 텍스트 포맷을 분리.

기존:
```csharp
Text = $"{labelText} {roi.Width}x{roi.Height}px",
```

변경 후:
```csharp
Text = $"{labelText} {FormatRoiSize(context, roi)}",
```

새 helper:
```csharp
private static string FormatRoiSize(RoiOverlayRenderContext context, RoiRect roi)
{
    var resolutionX = context.Model.Part?.PixelResolutionX ?? 0;
    var resolutionY = context.Model.Part?.PixelResolutionY ?? 0;
    if (resolutionX > 0 && resolutionY > 0)
    {
        var widthMm = roi.Width * resolutionX;
        var heightMm = roi.Height * resolutionY;
        return $"{widthMm:F2}x{heightMm:F2} mm";
    }
    return $"{roi.Width}x{roi.Height} px";
}
```

규칙:
- `PartInspectionData.PixelResolutionX/Y` (mm/pixel) 가 둘 다 0 초과면 mm 단위로 표시 (`F2` 소수 둘째 자리).
- 둘 중 하나라도 비어 있으면 (legacy / resolution 미정 모델) 기존처럼 픽셀 표시 fallback.
- Window ROI / Algorithm ROI / Preview ROI 모두 같은 헬퍼 사용 — 라벨 일관성.

### 2.4 영향 범위

- 시작 화면이 텅 빈 상태로 시작됨. 사용자가 Part Import 나 Load PTT 를 누르면 그때부터 표시.
- ROI 표시 단위가 mm 가 되어 실 장비 좌표계와 직접 비교 가능. resolution 정보가 없는 legacy 케이스는 자동으로 px 로 떨어짐 — 후방 호환 유지.

## 3. 변경된 파일

- `src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.xaml.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Views/RoiOverlayRenderer.cs`

## 4. 검증 결과

```
dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0개, 오류 0개, 8.60초

src/WpfInspectionPrototype/WpfInspectionApp/bin/x64/Debug/net48/WpfInspectionApp.exe --smoke-test
→ EXIT=0
```

### MVVM 점검
- View / VM / Model 경계는 그대로. RoiOverlayRenderer 는 순수 함수 (context → shapes).
- 시작 시 비즈니스 로직 (image 자동 로딩) 만 제거됨 — VM/Model 변경 없음.

## 5. 남은 위험 / 다음 확인

- GUI 수동 확인:
  - 앱 시작 시 CAM-01 / CAM-02 / CAM-03 모두 빈 화면 상태인지.
  - Part Import 후 CAM-01 컬러, CAM-03 binary/origin 정상 표시.
  - Part Import 후 ROI 가 그려질 때 라벨에 `WxH mm` 표시 (예: `Window ROI 1 12.40x8.30 mm`).
  - Legacy / resolution 없는 케이스에서 `WxH px` 로 자연 fallback.
- `FindDefaultImagePath` 가 더 이상 시작 자동 호출되지 않지만, 사용자 메뉴 / 단축키 등에서 호출하는 경로가 있는지 확인 권장 (있다면 그대로 작동).
- mm 환산 시 X / Y 가 다르게 표시 (`F2`) — 너무 길거나 짧으면 `F1` / `F3` 로 조정 가능.
- 라벨 외에도 RoiText (TextBlock RoiText / DrawAlignSearchOverlay summary 라벨) 등 다른 ROI 정보 출력 지점이 mm 환산 안 되어 있으면 사용자가 추가 요청할 수 있음.

## 6. 다음 작업 추천 순서

1. GUI 수동 확인.
2. (요청 시) Align Panel 의 `RoiText` 또는 다른 정보 라벨도 mm 표시로 통일.
3. (요청 시) 표시 자리수 정밀 조정 (F1 / F2 / F3).
4. 새 알고리즘 UI 작업 전, 선행 §4 의 재점검 명령 다시 실행.

## 7. 빌드 / 재현 명령

```powershell
dotnet build src\WpfInspectionPrototype\WpfInspectionApp\WpfInspectionApp.csproj `
  -p:Configuration=Debug -p:Platform=x64

& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe --smoke-test
```
