# WPF Inspection App Handoff - Part Import Progress Overlay

작성일: 2026-05-20
프로젝트: `C:\Users\USER\Documents\NewProject\ImageProject`
선행 문서:
- `docs/2026-05-20-handoff-align-light-mvvm.md`
- `docs/2026-05-20-handoff-cam03-toggle.md`
- `docs/2026-05-20-handoff-cam01-color.md`
- `docs/2026-05-20-handoff-window-roi-style.md`

## 1. 작업 원칙

- MVVM 위반 여부는 기능 추가 전/후 항상 확인한다.
- View/XAML/code-behind에는 화면 이벤트 연결, 바인딩, UI 표시만 둔다.
- 값 검증, enable/disable 상태 판단, 파라미터 저장/정규화는 ViewModel/Model/Service 쪽에서 처리한다.
- 기존 사용자가 만든 변경은 되돌리지 않는다. `git reset --hard`, `git checkout --` 금지.

## 2. 이번 세션에서 한 작업

### 2.1 사용자 요청

"Part Import 누르면 잠시 UI가 프리징이 걸리는대 그때 무슨 작업한다고 프로그래스바 라던지 띄워줘서 무슨 작업하는지 알려줄 수 있어?"

→ Part Import 동안 무엇이 진행 중인지 시각화 + 가능한 한 freezing 자체도 줄이기.

### 2.2 변경 개요

- Import flow 를 **비동기 (async)** 로 변환하고, 무거운 단계 (XML 파싱 + 모델 변환, PTT 네이티브 로드, 이미지 디코드) 는 **Task.Run 으로 백그라운드 thread** 에서 실행.
- 단계 사이마다 `ImportProgressText` 를 갱신하고 `Task.Yield()` 로 UI paint cycle 을 양보 → 사용자가 어떤 작업 중인지 실시간으로 확인.
- 화면 전체에 **modal 오버레이** 를 띄워 IndeterminateProgressBar + 단계 텍스트 표시. 진행 중 추가 입력 차단.

### 2.3 변경된 파일

- `src/WpfInspectionPrototype/WpfInspectionApp/ViewModels/MainViewModel.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.xaml.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.xaml`

### 2.4 ViewModel 측 (MainViewModel.cs)

새로 주입된 서비스:
- `IPttViewerWorkflowService _pttViewerWorkflowService` (생성자 파라미터 추가).
  - 기존엔 PTT 로드를 `PttLoadRequested` 이벤트로 MainWindow.xaml.cs 의 `LoadPtt` 핸들러에 위임했고, 핸들러가 sync 로 native 호출.
  - 이제 VM 이 직접 service 를 보유해서 `await Task.Run(() => _pttViewerWorkflowService.Load(...))` 가능.
  - 기존 `PttLoadRequested` 이벤트는 다른 호출 (LoadPttCommand 등) 호환을 위해 그대로 유지.

새 상태/속성:
- `private bool _isImporting;`
- `private string _importProgressText = "";`
- `public bool IsImporting` — INPC + `ImportOverlayVisibility` 알림 + `ImportPartCommand` 의 `CanExecuteChanged` 까지 raise (중복 클릭 방지).
- `public string ImportProgressText` — INPC.
- `public Visibility ImportOverlayVisibility => IsImporting ? Visible : Collapsed;` — XAML 바인딩 전용.

Command:
- `ImportPartCommand = new RelayCommand(BrowseAndImportPart);` → `new AsyncRelayCommand(BrowseAndImportPartAsync, () => !IsImporting);`.

새/변경된 메서드:
- `private async Task BrowseAndImportPartAsync()` — 파일 다이얼로그(sync) → 경로 있으면 `await ImportPartFromPathAsync(...)`.
- `public async Task<bool> ImportPartFromPathAsync(string path)` — 본체. 단계별 흐름:
  1. `IsImporting = true`, `ImportProgressText = "Reading part file..."`, `await Task.Yield()`.
  2. `ImportProgressText = "Parsing XML and normalizing algorithms..."`,
     `await Task.Run(() => _partImportWorkflowService.ImportIntoModel(_model, path))`.
  3. `ImportProgressText = "Applying model to UI..."`, `await Task.Yield()`,
     `ModelViewRefreshRequested?.Invoke(...)` 등 UI thread 갱신.
  4. PTT 경로 있으면 `ImportProgressText = "Loading PTT (large file, may take a few seconds)..."`, `await LoadPttAsync(path, false)`.
  5. 이미지 경로 있으면 `ImportProgressText = "Loading source image..."`, `await Task.Run(() => LoadImageFromPath(...))`.
  6. `finally` 블록에서 `IsImporting = false`, `ImportProgressText = ""`.
- `public bool ImportPartFromPath(string path)` — 기존 sync API 호환 래퍼. 내부에서 `ImportPartFromPathAsync(...).GetAwaiter().GetResult()`. 테스트 / drag-drop 같은 sync 호출자 깨지지 않게 유지.
- `private async Task LoadPttAsync(string path, bool prepareMpti)` — `await Task.Run(() => _pttViewerWorkflowService.Load(...))` → `ApplyPttLoad(...)`. UI thread 에서 ApplyPttLoad 가 수행되도록 await 후에 호출.

스레드 안전:
- `InspectionModel` 의 컬렉션은 모두 `List<T>` (ObservableCollection 아님). 백그라운드 스레드에서 변경해도 WPF cross-thread exception 안 남.
- PTT 네이티브 호출은 자체 상태(`g_*`) 가 thread-local 이 아니라 process-global 이지만, import 중에 동시에 다른 native 호출이 일어나지 않는다는 가정 하에 안전. `ImportPartCommand` 자체가 `!IsImporting` 가드로 중복 실행 차단.

### 2.5 MainWindow.xaml.cs

`new MainViewModel(...)` 호출에 `App.Services.PttViewerWorkflow` 인자 1 개 추가 (마지막 `FlowAlgorithms` 직전).

이벤트 구독 `_viewModel.PttLoadRequested += LoadPtt` 그대로. 기존 sync PTT load 코드 경로는 변경 없음.

### 2.6 MainWindow.xaml — Overlay UI

루트 `<Grid Margin="10">` (line 552) 의 마지막 자식으로 추가. `Grid.Row="0" Grid.RowSpan="2"` 로 TopBar + Content 모두 덮음.

- 외곽 Grid: `Background="#88000000"` 반투명 검정 → 기존 UI dim + 입력 차단.
- 중앙 Border: 420px 폭, `PanelBackground` / `AccentBlue` border, `CornerRadius=6`.
  - `IMPORTING PART` 타이틀 (AccentBlue, Bold).
  - `ProgressBar IsIndeterminate=True` (높이 6).
  - `TextBlock Text="{Binding ImportProgressText}"` (Consolas, TextWrapping=Wrap).
- `Visibility="{Binding ImportOverlayVisibility}"` 로 VM 의 IsImporting 에 따라 자동 토글.

테마 색은 `DynamicResource` 로 잡혀 있어 Light/Dark/Pink 모두 자동 따라감.

### 2.7 영향 범위

- ImportPart 외 다른 명령 (LoadPtt, LoadImage 등) 은 그대로. 동일한 freezing 패턴.
- PTT load 자체는 이제 백그라운드 thread 에서 수행되므로 import 시 UI freezing 이 사실상 사라짐 — ProgressBar 애니메이션이 native 호출 동안에도 끊기지 않고 돌아간다.
- 단, native MPTI 가 process-global 상태를 쓰므로 PTT load 중 다른 native 명령을 동시에 누르는 케이스는 여전히 위험. 오버레이가 Hit-test 차단하므로 보호됨.

## 3. 검증 결과

```
dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0개, 오류 0개, 4.17초

src/WpfInspectionPrototype/WpfInspectionApp/bin/x64/Debug/net48/WpfInspectionApp.exe --smoke-test
→ EXIT=0
```

### MVVM 점검
- 오버레이는 순수 XAML 바인딩 (`Visibility`, `ImportProgressText`). code-behind 핸들러 추가 없음.
- 상태/명령/단계 텍스트는 모두 ViewModel.
- 선행 문서 §4 의 재점검 명령 4개 모두 매칭 0 유지.

## 4. 남은 위험 / 다음 확인

- GUI 수동 확인 항목:
  1. ImportPart 버튼 클릭 → 즉시 오버레이가 나타나는지.
  2. 단계 텍스트가 "Reading part file" → "Parsing XML..." → "Applying model..." → "Loading PTT..." → "Loading source image..." → "Finalizing..." 로 차례로 보이는지.
  3. ProgressBar 가 PTT 로드 동안에도 애니메이션 계속 도는지 (UI freezing 사실상 사라짐).
  4. Import 끝나면 오버레이 자동 사라지는지.
  5. 오버레이가 떠있는 동안 다른 버튼 / 트리 클릭이 차단되는지 (background dim 으로 입력 차단).
- 사용자가 ESC 같은 키로 import 취소를 원할 수도 있음. 현재는 cancel 없음. 필요 시 `CancellationTokenSource` 도입 가능.
- PTT 파일이 정말 클 때 (수백 MB) Task.Run 안의 native 호출은 여전히 시간을 많이 잡아먹음. ProgressBar 가 indeterminate 라 정확한 % 표시는 안 됨. 단계별 percentage 가 필요하면 native bridge 가 progress callback 을 노출해야 함.
- LegacyRawPartImportAdapter 가 큰 PTT 의 별도 fallback 경로를 메인 thread 에서 추가로 더 부르는지 확인 필요 (현재 점검 결과 모두 `_partImportWorkflowService.ImportIntoModel` 안에 들어있어 Task.Run scope 안).

## 5. 다음 작업 추천 순서

1. 사용자 GUI 에서 §4 의 5개 항목 직접 확인.
2. (요청 시) Import 중 ESC 키 / Cancel 버튼으로 취소 가능하게.
3. (요청 시) PTT 로드 진행률 % 표시 — native 측 callback hook 추가 필요.
4. (요청 시) 같은 오버레이 패턴을 LoadPtt / SaveModel / LoadModel 명령에도 적용.
5. 새 알고리즘 UI 작업 전, 선행 §4 의 재점검 명령 다시 실행.

## 6. 빌드 / 재현 명령

```powershell
dotnet build src\WpfInspectionPrototype\WpfInspectionApp\WpfInspectionApp.csproj `
  -p:Configuration=Debug -p:Platform=x64

& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe --smoke-test
```
