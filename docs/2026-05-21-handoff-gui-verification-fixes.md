# WPF Inspection App Handoff - GUI 검증 회귀 수정 (P0 1차)

작성일: 2026-05-21
프로젝트: `C:\Users\USER\Documents\NewProject\ImageProject`

## 1. 배경

P0 GUI 수동 검증 결과 사용자가 보고한 3개 영역에서 회귀 발견:

- 11: U25 (Models/2.zip) 의 270° 회전 ROI 가 W/H 스왑 안 됨
- 12: Light 값이 XML 에서 패널로 import 안 됨
- 13: Threshold/Binary preview 가 일부 컨트롤 (Range Upper/Lower 콤보 등) 변경 시 갱신 안 됨

## 2. 진단

### 11. 회전 ROI W/H 스왑
- 모든 U25 WindowData 의 `<RelRoi>/<a>` = 0
- 그러나 `<PartData>/<Roi>/<a>` = 270 — **Part 좌표계 자체가 회전**
- 기존 swap 로직은 `RelRoi.a` 만 보고 있어 Part 회전이 누락됨

### 12. Light 값 XML import 누락
- legacy pemto RawData 의 algorithm 마다 `<LTInfo>LightType,R,G,B,W,extra</LTInfo>` CSV 형식으로 light 값 저장
- 예: `<LTInfo>0,100,0,0,100,0</LTInfo>` = TOP / R=100 / G=0 / B=0 / W=100
- `ApplyCommonLightParameters` 는 `<LightTypeNum>` / `<RedValue>` 같은 명시 tag 만 찾고 `<LTInfo>` CSV 는 무시 → 모든 algorithm 이 default fallback (R=100 only) 으로 떨어짐

### 13. Threshold 갱신 누락
- `AlignPanelUpdateKindExtensions.ToEffect` 에서 일부 kind 가 `Threshold` flag 누락:
  - `Threshold3D` → `Model | MaskDensity` (Threshold 빠짐 ❌)
  - `EdgeGain` → `Model | MaskDensity` (Threshold 빠짐 ❌)
- 다른 kind 는 OK (예: `Parameter` 는 Threshold 포함 → Range2DCombo/Range3DCombo Upper/Lower 변경은 동작)
- 사용자가 본 "Upper/Lower 동작 안 함" 은 정확히 어느 컨트롤인지 확정 못 했지만, 3D MIN/MAX 슬라이더 및 Edge gain 도 binary 갱신 안 되던 게 명백한 버그

## 3. 적용한 수정

### 3.1 Part-level rotation 전파 (`LegacyRawPartImportAdapter.cs`)

- `LegacyRoiTransform` record 에 `PartAngleDegrees` 필드 추가 (기본값 0).
- `CreateRoiTransform` 이 `<PartData>/<Roi>/<a>` 를 읽어 `PartAngleDegrees` 로 전달.
- `ParseRoi` 의 회전 swap 결정에 RelRoi 의 a 와 Part 의 a 를 합산:

```csharp
var localAngle = ReadDouble(roiElement, "a", "Angle");
var combinedAngle = ((localAngle + transform.PartAngleDegrees) % 360.0 + 360.0) % 360.0;
var shouldSwapWh = (combinedAngle > 45.0 && combinedAngle <= 135.0)
                || (combinedAngle > 225.0 && combinedAngle <= 315.0);
if (shouldSwapWh)
{
    (rawWidth, rawHeight) = (rawHeight, rawWidth);
}
```

- U25 (Part.a=270, RelRoi.a=0) → combined=270 → 스왑 동작
- RefID_2 (Part.a=0, RelRoi.a=0) → combined=0 → 스왑 없음 (회귀 없음)

### 3.2 `<LTInfo>` CSV light 파싱 (`LegacyRawPartImportAdapter.cs`)

- `ApplyCommonLightParameters` 첫 줄에 `ApplyLtInfoCsvIfPresent(algorithm, element)` 호출 추가.
- 새 helper `ApplyLtInfoCsvIfPresent`:
  - `<LTInfo>` 텍스트를 콤마로 split.
  - 6 필드 이상이면 `[0]=LightType, [1]=R, [2]=G, [3]=B, [4]=W` 매핑.
  - `Common.LightTypeNum`, `Common.RedValue`, `Common.GreenValue`, `Common.BlueValue`, `Common.WhiteValue` 에 SetInt.
  - 0~200 clamp.
- 명시 tag (`<LightTypeNum>`, `<RedValue>` 등) 가 별도로 있으면 그게 LTInfo 값을 덮어쓰도록 호출 순서 보장 (LTInfo 먼저).

### 3.3 Threshold flag 보강 (`ViewModels/AlignPanelRequests.cs`)

```csharp
AlignPanelUpdateKind.Threshold3D => Model | MaskDensity | Threshold,  // +Threshold
AlignPanelUpdateKind.EdgeGain   => Model | MaskDensity | Threshold,  // +Threshold
```

다른 kind 의 ToEffect 매핑은 변경 없음 (이미 적절).

## 4. 변경된 파일

- `src/WpfInspectionPrototype/WpfInspectionApp/Models/LegacyRawPartImportAdapter.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/ViewModels/AlignPanelRequests.cs`

## 5. 검증 결과

```
dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0개, 오류 0개, 6.34초

src/WpfInspectionPrototype/WpfInspectionApp/bin/x64/Debug/net48/WpfInspectionApp.exe --smoke-test
→ EXIT=0
```

## 6. 남은 위험 / 다음 확인

- GUI 수동 확인:
  - **U25 import**: 270° ROI 가 화면상 W/H 스왑되어 정확한 비율로 보이는지
  - **Light import**: RefID_2 import 후 알고리즘 별로 Light 값이 다르게 표시되는지 (이전엔 전부 R=100 같았음)
  - **Threshold 갱신**: 3D MIN/MAX 슬라이더, Edge gain 변경 시 CAM-03 binary 가 즉시 갱신되는지
  - **RefID_2 회귀**: 1.zip import 결과 이전과 동일하게 정상 (Part.a=0 이라 변화 없어야 함)
- LTInfo 의 6번째 값 (`,extra`) 은 현재 무시. 만약 algorithm 인덱스 또는 다른 의미라면 추가 처리 필요.
- AlignParameter / SearchParameter kind 의 ToEffect 도 Threshold 누락 — 의도된 동작인지 확인 필요. 예를 들어 Search size 변경은 ROI 만 영향이고 binary 갱신은 불필요할 수 있음.
- Light Type 별 채널 가용성 (TOP=RGBW, MIDDLE/BOTTOM=RB, SIDE=RGB) 와 LTInfo 값 충돌 시 처리: 현재는 raw 값 그대로 저장. AlgorithmLightService 가 LightType 별로 비활성 채널을 처리하므로 view 단에서는 보이는 채널만 표시될 것.

## 7. 다음 작업 추천

1. 사용자가 위 4개 GUI 항목 직접 확인 후 결과 보고
2. (요청 시) LTInfo 의 extra 필드 의미 파악 + 활용
3. (요청 시) AlignParameter / SearchParameter 의 Threshold flag 추가 여부 결정
4. P0 14번 (User Light) 등 미검증 항목 확인

## 8. 빌드 / 재현 명령

```powershell
dotnet build src\WpfInspectionPrototype\WpfInspectionApp\WpfInspectionApp.csproj `
  -p:Configuration=Debug -p:Platform=x64

& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe --smoke-test
```
