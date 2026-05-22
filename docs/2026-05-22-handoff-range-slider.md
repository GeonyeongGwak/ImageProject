# 2026-05-22 Handoff - 단일 Range Slider (2-thumb) 도입

## 배경

사용자: "Align Panel 은 왜 Min Max 가 나눠져있는지 모르겠어 참조프로젝트에서 확인해줘봐 ucAlign"

### 참조 프로젝트 조사 결과

`pemtoFrameworkAll_R_4.0.0.7\pemtoFramework\02.CommonClass\UI_Common.cs::CtrlEnableChange_Range_2D`
가 `TypeInspRange` 4 모드에 따라 Min/Max 슬라이더의 활성/비활성 + 기본값 분기:

| RangeType | Min 슬라이더 | Max 슬라이더 | 기본값 |
|---|---|---|---|
| Range_In (Inside) | ✅ 활성 | ✅ 활성 | (80, 120) |
| Range_Out (Outside) | ✅ 활성 | ✅ 활성 | (80, 120) |
| Range_Upper | ❌ disable | ✅ 활성 | Min=0, Max=80 |
| Range_Lower | ✅ 활성 | ❌ disable | Min=80, Max=255 |

→ 단일 임계 슬라이더 1개로는 4 모드 의미 표현 불가. Min/Max 분리는 의도된 설계.

### 현재 WPF 차이점

- Min/Max 두 개의 표준 `Slider` 가 별도 줄에 배치
- `Range2DType` 콤보가 RangeType 모드 전환
- **모드별 슬라이더 disable 로직 미구현** → Upper 모드에서도 Min 슬라이더 활성 → 사용자 혼란

사용자 요청: **두 슬라이더 → 한 컨트롤** 로 통합.

## 적용 변경 — 커스텀 `RangeSlider` UserControl

### 신규 파일

#### `Views/RangeSlider.xaml`
- `Grid` 위에 트랙 base (회색) + 활성 구간 fill (테마 강조색) + `Canvas` 안 `Thumb` 2 개
- Thumb 템플릿: `Ellipse` (AccentBlue fill / PrimaryText stroke). `IsEnabled=False` 시 Trigger 로 Opacity=0.35
- 모든 색상 `{DynamicResource ...}` — 테마 자동 추종

#### `Views/RangeSlider.xaml.cs`
- DependencyProperty: `Minimum` / `Maximum` / `MinValue` (TwoWay) / `MaxValue` (TwoWay) / `RangeMode` (TwoWay)
- RoutedEvent: `ValueChanged` (기존 `AlignControl_Changed` 패턴 호환)
- `MinThumb_DragDelta` / `MaxThumb_DragDelta` — 픽셀 → 값 변환, 다른 thumb 침범 방지 (Min ≤ MaxValue, Max ≥ MinValue 클램프)
- `ApplyRangeModeEnableState()` — RangeMode 변경 시:
  - Upper(2): `MinThumb.IsEnabled = false`, `MinValue = Minimum`
  - Lower(3): `MaxThumb.IsEnabled = false`, `MaxValue = Maximum`
  - Inside/Outside(0/1): 두 thumb 활성
  - 참조 `CtrlEnableChange_Range_2D` 의 enable 분기 그대로 매칭
- `UpdateLayoutPositions()` — 두 thumb 의 `Canvas.Left` + 활성 fill bar 의 `Margin`/`Width` 계산 (`Loaded`, `SizeChanged`, value change 모두 hook)
- `_suppressValueChangedEvent` 가드 — RangeMode 변경 중 내부 값 보정이 `ValueChanged` 다시 발화하지 않도록.

### AlignPanelView.xaml 변경

기존 두 `Grid` 블록 (`ThresholdSlider` + `Threshold2DMaxSlider`) **제거** → 단일 `Grid` 안 `RangeSlider`:

```xml
<views:RangeSlider x:Name="ThresholdRangeSlider"
                   Tag="{x:Static vm:AlignPanelUpdateKind.Threshold2D}"
                   Minimum="0" Maximum="255"
                   MinValue="{Binding Threshold2D, Mode=TwoWay}"
                   MaxValue="{Binding Threshold2DMax, Mode=TwoWay}"
                   RangeMode="{Binding Range2DType}"
                   IsEnabled="{Binding Is2DConditionEnabled}"
                   ValueChanged="AlignControl_Changed" />
```

- 라벨 `2D MIN/MAX`
- 우측 작은 텍스트로 `Threshold2DText` / `Threshold2DMaxText` 두 값 표시 (사용자가 슬라이더 외에 정확 수치 확인 가능)
- 기존 `Tag` + `ValueChanged="AlignControl_Changed"` 패턴 그대로 호환

xmlns `xmlns:views="clr-namespace:WpfInspectionApp.Views"` 추가.

## 동작

| Range2DType | UI 동작 |
|---|---|
| Inside (0) | Min thumb / Max thumb 모두 드래그 가능. 두 thumb 사이 fill 표시. |
| Outside (1) | 동일 (의미는 native 알고리즘에서 반전). |
| Upper (2) | Min thumb opacity 0.35 + 드래그 불가. Min=0 자동 잠금. Max thumb 만 조작. |
| Lower (3) | Max thumb opacity 0.35 + 드래그 불가. Max=255 자동 잠금. Min thumb 만 조작. |

## MVVM 점검

- `AlignPanelViewModel` 변경 없음 — `Threshold2D` / `Threshold2DMax` / `Range2DType` / `Is2DConditionEnabled` 기존 binding 그대로.
- `RangeSlider` 는 순수 View — DP + Routed event 만. 비즈니스 로직 없음.
- code-behind 의 핸들러는 view-only (thumb drag → DP 갱신).
- View 단계 RangeMode→thumb-enable 매핑은 참조 프로젝트의 UI helper 동작과 동등 — ViewModel 이 알 필요 없음.

## 변경된 파일

- `src/WpfInspectionPrototype/WpfInspectionApp/Views/RangeSlider.xaml` (신규)
- `src/WpfInspectionPrototype/WpfInspectionApp/Views/RangeSlider.xaml.cs` (신규)
- `src/WpfInspectionPrototype/WpfInspectionApp/Views/AlignPanelView.xaml` (xmlns:views 추가 + 2 슬라이더 블록 → RangeSlider 1 개)

## 검증

```
dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0, 오류 0, 8.95초

src/WpfInspectionPrototype/WpfInspectionApp/bin/x64/Debug/net48/WpfInspectionApp.exe --smoke-test
→ EXIT=0
```

## 다음 작업 후보

1. **3D MIN/MAX 도 동일 패턴 적용** — `Threshold3DSlider`/`Threshold3DMaxSlider` 두 개도
   `RangeSlider` 한 컨트롤로 통합 (`Range3DType` 도 동일 4 모드).
2. **RangeType 변경 시 참조 default 값 적용** — 현재는 thumb enable / 잠금만 매칭.
   참조는 Upper 진입 시 Max 기본값=80, Lower 진입 시 Min 기본값=80, In/Out 기본 (80, 120)
   까지 자동 세팅. 이 로직을 `ApplyRangeModeEnableState` 에 확장하거나 ViewModel 에서 처리.
3. **GUI 수동 검증** — 사용자가 모드별 thumb 동작 확인 + drag 위치 정확도 확인.
4. **다른 임계 영역 (Light Control, Algorithm panel 내 Min/Max) 에도 동일 RangeSlider** —
   재사용성 검토.

## GUI 수동 검증 권장 항목

- Inside/Outside 모드: 두 thumb 모두 자유 드래그 가능
- Upper 모드: Min thumb 가 시각적으로 흐려지고 드래그 안 됨. Min 값이 0 으로 고정. Max thumb 만 조작 가능.
- Lower 모드: 반대 동작 (Max=255 잠금).
- 모드 전환 시 잠긴 thumb 의 값이 자동 끝점으로 이동.
- thumb 가 서로 침범하지 않음 (Min > Max 불가능).
- 활성 fill bar 가 두 thumb 사이를 정확히 채움.
- `Is2DConditionEnabled = false` (예: Use2D 끔) 시 전체 컨트롤 비활성.

## 빌드 / 재현 명령

```powershell
dotnet build src\WpfInspectionPrototype\WpfInspectionApp\WpfInspectionApp.csproj `
  -p:Configuration=Debug -p:Platform=x64

& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe --smoke-test
```
