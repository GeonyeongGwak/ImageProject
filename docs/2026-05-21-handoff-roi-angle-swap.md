# WPF Inspection App Handoff - ROI Import 시 Angle 기반 W/H 스왑

작성일: 2026-05-21
프로젝트: `C:\Users\USER\Documents\NewProject\ImageProject`
선행 문서:
- `docs/2026-05-20-handoff-tree-bring-into-view.md` 외 9개.

## 1. 작업 원칙

- MVVM 위반 여부는 기능 추가 전/후 항상 확인한다.
- View/XAML/code-behind 에는 화면 이벤트 연결, 바인딩, UI 표시만 둔다.
- 값 검증, 정규화는 ViewModel/Model/Service.

## 2. 이번 세션에서 한 작업

### 2.1 문제 진단

사용자: "Import 됬을때 2.zip 같은 경우는 ROI 표시가 이상해서 각도에 따라서 W,H 를 바꿔주는 구문이 있는건가 싶었어"

조사 결과:
- `Models/2.zip` (U25 part) 의 `RelRoi` 안에 `<a>` (angle, degree) 태그 존재.
- 일부 Window 가 `<a>270.0</a>` 값을 가짐 (다른 것들은 `0.0`).
- `Models/1.zip` (RefID_2) 은 모든 ROI 의 angle 이 0 또는 태그 부재.
- `LegacyRawPartImportAdapter.ParseRoi` (line 3496) 가 `<a>` 를 **전혀 읽지 않음** → 270° 회전된 ROI 가 화면상 W/H 비율이 잘못 표시되는 원인.

축약 컨텍스트 (U25.xml):
```xml
<RelRoi>
  <w>8.6783</w>
  <h>13.6134</h>
  <cx>0.0000</cx>
  <cy>0.0000</cy>
  <a>270.0</a>   ← 무시되고 있던 회전각
</RelRoi>
```

### 2.2 적용한 수정

`Models/LegacyRawPartImportAdapter.cs` 의 `ParseRoi` 안에 angle 기반 W/H 스왑 로직 추가:

```csharp
var rawAngle = ReadDouble(roiElement, "a", "Angle");
var normalizedAngle = ((rawAngle % 360.0) + 360.0) % 360.0;
var shouldSwapWh = (normalizedAngle > 45.0 && normalizedAngle <= 135.0)
                || (normalizedAngle > 225.0 && normalizedAngle <= 315.0);
if (shouldSwapWh)
{
    (rawWidth, rawHeight) = (rawHeight, rawWidth);
}
```

- `<a>` 또는 `<Angle>` 태그를 읽음 (없으면 ReadDouble 이 0 반환 → no-op, 기존 동작 보존).
- 음수 / 360 초과 angle 도 modulo 로 정규화.
- 45° < a ≤ 135° 또는 225° < a ≤ 315° 범위면 W/H 스왑.
- 0°/180° 부근은 그대로 → RefID_2 같은 정상 part 에 영향 없음.

이후 기존 resolution 변환 / pixel 환산 / center→top-left 변환 코드는 변경 없이 동일.

### 2.3 영향 범위

- `ParseRoi` 는 Window ROI (`RelRoi`) + Algorithm ROI (`ParseOptionalAlgorithmRoi` 가 같은 함수 호출) 둘 다 사용 — 양쪽 모두 일관 적용.
- 회전 그 자체는 axis-aligned RoiRect 로 표현 못 함 (현 모델에는 angle 필드 없음). 화면상 bounding box 가로/세로 길이만 맞춰주는 보정.
- 45°·135°·225°·315° 등 비표준 각도가 들어와도 가까운 90° 배수 쪽으로 떨어짐 — 산업용 vision 의 일반적 사용 패턴(0/90/180/270)과 일치.

### 2.4 MVVM 점검

- Renderer / View / VM 변경 없음.
- Model import 단계 (Service/Model) 안에서만 처리.
- 기존 import 회귀 가능성은 낮음 (`<a>` 없을 때 무동작 보장).

## 3. 변경된 파일

- `src/WpfInspectionPrototype/WpfInspectionApp/Models/LegacyRawPartImportAdapter.cs`

## 4. 검증 결과

```
dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0개, 오류 0개, 1.51초

src/WpfInspectionPrototype/WpfInspectionApp/bin/x64/Debug/net48/WpfInspectionApp.exe --smoke-test
→ EXIT=0
```

## 5. 남은 위험 / 다음 확인

- GUI 수동 확인:
  - `Models/2.zip` (U25) Import → 270° 회전된 ROI 가 이제 가로/세로 비율 정확하게 보이는지.
  - `Models/1.zip` (RefID_2) Import → 변화 없이 정상 표시되는지.
  - ROI 라벨에 표시되는 `WxH mm` 도 swap 후 값 (예: 13.61x8.68 mm) 로 표시되는지.
- 회전된 ROI 를 화면에 실제 rotated rectangle 로 그리고 싶다면 (대각선 윤곽), `RoiRect` 에 `Angle` 필드 + Renderer 에 RotateTransform 추가 필요 — 별도 작업 후보. 현재는 axis-aligned bounding box 만.
- 임의 각도 (예: 45°) 가 들어오는 경우 가까운 쪽으로 떨어짐. 정밀 회전이 필요하다면 위 항목.
- Align 알고리즘 등의 내부 비교 (예: `MaxAngle`) 는 angle 값 자체이지 RoiRect 가 아니므로 영향 없음.

## 6. 다음 작업 추천 순서

1. GUI 에서 §5 항목 직접 확인.
2. (요청 시) RoiRect 에 Angle 필드 추가 + Renderer 에 회전 적용.
3. (요청 시) `<a>` 값을 알고리즘 파라미터로도 노출 (`Common.RoiAngle`) → algorithm flow 가 알 수 있게.

## 7. 빌드 / 재현 명령

```powershell
dotnet build src\WpfInspectionPrototype\WpfInspectionApp\WpfInspectionApp.csproj `
  -p:Configuration=Debug -p:Platform=x64

& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe --smoke-test
```
