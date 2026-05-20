# WPF Inspection App Handoff - CAM-03 Binary/Origin Toggle

작성일: 2026-05-20
프로젝트: `C:\Users\USER\Documents\NewProject\ImageProject`
참조 프로젝트: `D:\Work\GGY\pemtoFrameworkAll_R_4.0.0.7`
선행 문서: `docs/2026-05-20-handoff-align-light-mvvm.md`

## 1. 작업 원칙

- MVVM 위반 여부는 기능 추가 전/후 항상 확인한다.
- View/XAML/code-behind에는 화면 이벤트 연결, 바인딩, UI 표시만 둔다.
- 값 검증, enable/disable 상태 판단, 파라미터 저장/정규화는 ViewModel/Model/Service 쪽에서 처리한다.
- 기존 사용자가 만든 변경은 되돌리지 않는다. worktree에는 이번 작업 외 변경도 많으므로 `git reset --hard`, `git checkout --` 사용 금지.

## 2. 이번 세션에서 한 작업

### 2.1 인수인계 기준선 확인

선행 문서 §4의 재점검 명령 4개를 그대로 실행 → 모두 매칭 0:

```powershell
rg -n "Common\.Light|Common\.RedValue|Common\.GreenValue|Common\.BlueValue|Common\.WhiteValue|LightCnt|ArrRedValueString|ArrGreenValueString|ArrBlueValueString|ArrWhiteValueString|ArrCalculationString|ArrLightPositionString|LightTypeNum" src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.xaml.cs src/WpfInspectionPrototype/WpfInspectionApp/Views
rg -n "Click=|SelectionChanged=|ValueChanged=|TextChanged=|MouseUp=|PreviewMouse" src/WpfInspectionPrototype/WpfInspectionApp/Views/LightControlView.xaml src/WpfInspectionPrototype/WpfInspectionApp/Views/LightControlView.xaml.cs
rg -n "Light Type|Red Light|Green Light|Blue Light|White Light|Select Light|Add Light|Sub Light|Light Data Count|ROI Light|Mid Light" src/WpfInspectionPrototype/WpfInspectionApp/Models src/WpfInspectionPrototype/WpfInspectionApp/AlgorithmPanels src/WpfInspectionPrototype/WpfInspectionApp/Views src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.xaml
rg -n "EnsurePttViewerPanel|_pem3DViewerHostService|_pttViewerHost|_pttViewerPanel|LoadIntoControl\(|Loaded 3D PTT|Load 3D PTT|3D SOURCE|3D PTT" src/WpfInspectionPrototype/WpfInspectionApp -g "*.cs" -g "*.xaml"
```

빌드/스모크 베이스라인:

```
dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0개, 오류 0개

src/WpfInspectionPrototype/WpfInspectionApp/bin/x64/Debug/net48/WpfInspectionApp.exe --smoke-test
→ EXIT=0
```

### 2.2 Align Panel 점검

`Views/AlignPanelView.xaml.cs`:
- 비즈니스 로직은 `AlignPanelStateService` + `AlignPanelViewModel` 로 분리됨 (선행 §3.1 결과 유지).
- code-behind는 UI 이벤트 → `UpdateRequested` / `ActionRequested` / `SearchTabSelectionChanged` 로 외부에 전달하는 bridging 만 수행.
- `_suppressUpdateRequests` 가드로 외부에서 상태 주입할 때 재진입 차단.
- MVVM 위반 없음.

`Views/AlignPanelView.xaml`:
- 모든 입력 컨트롤이 `{Binding}` + `{StaticResource}` / `{DynamicResource}` 로 처리됨.
- 사소한 이슈로 `Binary ROI` / `Margin` / `Size X` / `Size Y` 배지 라벨이 하드코딩된 색상(`#3CFF50`, `#FFD229`, `#06111C`) 을 사용 → 테마 미추종. 이번에는 변경하지 않음.

### 2.3 CAM-03 Binary/Origin 토글 추가

기존: CAM-02 는 PTT/Origin 영역, CAM-03 는 Binary 결과 전용. Light 슬라이더를 움직이면 자동으로 binary preview 가 재계산되어 사용자가 보지도 않는 Binary 가 계속 갱신됨.

변경 후:
- CAM-02 는 그대로(Origin/PTT 전용 유지).
- CAM-03 헤더에 토글 버튼 추가 → CAM-03 는 Origin (2D Light 합성) 또는 Binary 둘 다 표시 가능.
- 라벨은 "전환 후 모드" (현재 Origin 보고 있으면 버튼 라벨 "Binary").
- 토글 버튼 스타일은 기존 `DockToolButton` 재사용 → 테마 자동 추종.
- Light 변경 시: 현재 보이는 모드만 갱신.
  - Origin 모드: `SourceImage` 만 갱신, threshold preview 호출 skip.
  - Binary 모드: 기존과 동일 (threshold 재계산).
  - 모드 토글로 Binary 로 들어오는 순간 한 번 lazy 재계산해서 stale 상태 해소.

#### ViewModel 변경

`ViewModels/MainViewModel.cs`:
- 새 enum `Cam03PreviewMode { Origin, Binary }` 정의.
- 새 backing field `_cam03Mode` (기본값 `Binary` — 기존 동작 보존).
- 새 properties:
  - `Cam03Mode` (TwoWay 가 아닌 ViewModel 내부에서만 변경)
  - `Cam03Image` (계산: `Cam03Mode == Binary ? _binaryImage : _sourceImage`)
  - `Cam03ToggleLabel` (계산: `Cam03Mode == Binary ? "Origin" : "Binary"`)
  - `Cam03Title` (계산: `"CAM-03 | BINARY RESULT"` 또는 `"CAM-03 | ORIGIN"`)
- 새 command `Cam03ToggleCommand` → `ToggleCam03PreviewMode()`.
- `SourceImage` / `BinaryImage` setter 에 모드 가드 추가 — 현재 모드일 때만 `Cam03Image` PropertyChanged 발화.
- `HandleLightPanelChanged()`:
  - `ApplyPttLightPreviewForActiveAlgorithm()` 는 항상 호출 (CAM-01 메인 뷰가 동일 source 사용).
  - `OverlayRefreshRequested?.Invoke()` 도 항상.
  - `Cam03Mode == Binary` 일 때만 `ThresholdScheduleRequested?.Invoke()`.
- `ToggleCam03PreviewMode()`:
  - 모드 플립.
  - Origin → Binary 전환 시 `RequestOverlayAndThresholdRefresh()` 한 번 호출(lazy 갱신).
  - Binary → Origin 전환 시 `OverlayRefreshRequested?.Invoke()` 만.

#### View 변경

`MainWindow.xaml` (CAM-03 헤더 + 이미지 바인딩만):
- 헤더 그리드 컬럼 3 → 4 (`*` / `Auto` / `Auto` / `Auto`).
- 타이틀 `Text` → `{Binding Cam03Title}` 로 변경.
- 새 버튼 `Cam03ModeToggleButton`:
  - `Content="{Binding Cam03ToggleLabel}"`
  - `Command="{Binding Cam03ToggleCommand}"`
  - `Style="{StaticResource DockToolButton}"`, `MinWidth="64"`, `Margin="0,0,6,0"`.
- 기존 FLOAT 버튼은 컬럼 3 으로 이동.
- 이미지 `Source` → `{Binding BinaryImage}` 에서 `{Binding Cam03Image}` 로 변경.
- code-behind 수정 없음.

#### MVVM 확인

- Mode 상태, Command, 계산 프로퍼티 모두 ViewModel 에 존재.
- View 는 순수 Binding (Click 핸들러 추가 없음).
- code-behind 수정 없음.

## 3. 변경된 파일

- `src/WpfInspectionPrototype/WpfInspectionApp/ViewModels/MainViewModel.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.xaml`

## 4. 샘플 파일 / Part Import 확인

`src/WpfInspectionPrototype/Models/1.zip` 추출 결과:

- `RefID_2/RefID_2.xml` (legacy pemto schema)
- `RefID_2/RefID_2.ptt`, `.pot`, `.jpg`, `.pshx`, algorithm-별 `.png` 등

RefID_2.xml Light 관련 raw 태그 점검 결과: `<SubLight_1>` ~ `<SubLight_4>` (BLOB 카운트 매핑, `LegacyRawPartImportAdapter` line 3048 부근에서 `{prefix}.LightCount` 로만 사용) 외에는 `LightTypeNum`, `RedValue`, `GreenValue`, `BlueValue`, `WhiteValue`, `LightCnt`, `Arr*ValueString` 등 normalizer 가 인식하는 키 **없음**.

따라서 이 샘플로 Part Import 하면 Light 필드는 `ImportedAlgorithmNormalizer` 의 default fallback(`Common.LightTypeNum=0`(TOP), `Common.RedValue=100`, 나머지 0) 으로만 채워진다. 실제 Light 값 회귀 검증에는 부적합 — Light raw 태그를 포함한 part XML 별도 확보 필요.

## 5. 검증 결과

```
dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0개, 오류 0개, 4.66초

src/WpfInspectionPrototype/WpfInspectionApp/bin/x64/Debug/net48/WpfInspectionApp.exe --smoke-test
→ EXIT=0
```

## 6. MVVM 재점검 명령

선행 문서 §4 와 동일.

```powershell
rg -n "Common\.Light|Common\.RedValue|Common\.GreenValue|Common\.BlueValue|Common\.WhiteValue|LightCnt|ArrRedValueString|ArrGreenValueString|ArrBlueValueString|ArrWhiteValueString|ArrCalculationString|ArrLightPositionString|LightTypeNum" src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.xaml.cs src/WpfInspectionPrototype/WpfInspectionApp/Views
rg -n "Click=|SelectionChanged=|ValueChanged=|TextChanged=|MouseUp=|PreviewMouse" src/WpfInspectionPrototype/WpfInspectionApp/Views/LightControlView.xaml src/WpfInspectionPrototype/WpfInspectionApp/Views/LightControlView.xaml.cs
rg -n "Light Type|Red Light|Green Light|Blue Light|White Light|Select Light|Add Light|Sub Light|Light Data Count|ROI Light|Mid Light" src/WpfInspectionPrototype/WpfInspectionApp/Models src/WpfInspectionPrototype/WpfInspectionApp/AlgorithmPanels src/WpfInspectionPrototype/WpfInspectionApp/Views src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.xaml
rg -n "EnsurePttViewerPanel|_pem3DViewerHostService|_pttViewerHost|_pttViewerPanel|LoadIntoControl\(|Loaded 3D PTT|Load 3D PTT|3D SOURCE|3D PTT" src/WpfInspectionPrototype/WpfInspectionApp -g "*.cs" -g "*.xaml"
```

세션 종료 시점 기준 4개 모두 매칭 0.

## 7. 남은 위험 / 다음 확인

- 실제 GUI 에서 토글 동작 확인 필요:
  - 초기 상태(Binary)에서 라벨 "Origin", 타이틀 "CAM-03 | BINARY RESULT" 인지.
  - 토글 클릭 시 라벨이 "Binary" 로, 타이틀이 "CAM-03 | ORIGIN" 으로 바뀌고 영상이 2D Light 합성으로 전환되는지.
  - Light 슬라이더 빠른 조작 시 Origin 모드에서는 binary 가 동기 갱신 안 되는지 (CPU 사용량 / TimingText 변화로 확인 가능).
  - Origin → Binary 토글 순간 lazy 재계산이 최신 source 기준으로 잘 들어오는지.
- `BinaryOverlay` (ROI 캔버스) 는 binary/origin 모드 모두에서 동일 위치 + 동일 ROI 가 보임 — 의도된 동작이지만 사용자 시각에 어색하면 모드별로 다른 overlay 가 필요할 수 있음.
- Align Panel 의 배지 라벨(`Binary ROI`, `Margin`, `Size X`, `Size Y`) 가 하드코딩 색상으로 테마 미추종. 사용자가 명시적으로 요구할 때 따로 처리.
- RefID_2.xml 처럼 Light raw 태그가 없는 샘플로는 §3-§5 (선행 문서) 의 Light 회귀 검증이 어렵다. Normal Light / User Light 가 채워진 part XML 1 개 확보 후에 다시 import → 패널 값 비교 → native bridge 전달 확인을 한 사이클 더 돌려야 함.
- worktree 에 이번 작업 외 다수의 modified 파일 + untracked 파일이 누적되어 있음 (`git status` 다수). diff 단위로 검토하고 무관 변경 되돌리지 말 것.

## 8. 다음 작업 추천 순서

1. 사용자가 GUI 에서 CAM-03 토글 동작을 손으로 확인 (§7 첫 항목).
2. Light raw 태그가 들어있는 part XML 확보 후 선행 문서 §7-1 ~ §7-3 실행.
3. Origin 모드에서 Light 조작 중 빈번한 source rebuild 가 영향을 주는지 (`PttLightPreviewService.Render` 호출 빈도) 측정.
4. Align Panel 배지 라벨 테마 추종 작업 (요청 시).
5. 새 알고리즘 UI 작업 전, §6 의 재점검 명령 다시 실행.
