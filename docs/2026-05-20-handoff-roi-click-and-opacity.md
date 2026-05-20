# WPF Inspection App Handoff - ROI 클릭 선택 + 비선택 ROI 투명도

작성일: 2026-05-20
프로젝트: `C:\Users\USER\Documents\NewProject\ImageProject`
선행 문서:
- `docs/2026-05-20-handoff-align-light-mvvm.md`
- `docs/2026-05-20-handoff-cam03-toggle.md`
- `docs/2026-05-20-handoff-cam01-color.md`
- `docs/2026-05-20-handoff-window-roi-style.md`
- `docs/2026-05-20-handoff-import-progress.md`
- `docs/2026-05-20-handoff-topbar-theme-combobox.md`
- `docs/2026-05-20-handoff-default-image-and-roi-mm.md`

## 1. 작업 원칙

- MVVM 위반 여부는 기능 추가 전/후 항상 확인한다.
- View/XAML/code-behind 에는 화면 이벤트 연결, 바인딩, UI 표시만 둔다.
- 값 검증, enable/disable 상태 판단, 파라미터 저장/정규화는 ViewModel/Model/Service.
- 기존 사용자가 만든 변경은 되돌리지 않는다. `git reset --hard`, `git checkout --` 금지.

## 2. 이번 세션에서 한 작업

### 2.1 사용자 요청

1. 이미지에 그려진 ROI 를 클릭하면 오른쪽 트리(Inspection Tree)에서 해당 Window 가 선택되도록.
2. 선택되지 않은 ROI 는 투명도를 낮춰서 (더 옅게) 시각적으로 후퇴.

### 2.2 ROI 클릭 → 트리 Window 선택

#### A. MainViewModel.cs — 새 helper `TrySelectWindowById`

`SelectTreeNode` 가 트리 노드 객체를 받아 처리하던 흐름과 별도로, ID 기반 진입 경로 추가:

```csharp
public bool TrySelectWindowById(string windowId)
{
    if (_refreshingTree || string.IsNullOrWhiteSpace(windowId)) return false;
    var window = Model.Part.Windows.FirstOrDefault(w => w.Id == windowId);
    if (window == null) return false;

    Model.SelectedWindowId = window.Id;
    var firstAlgorithm = window.Algorithms.FirstOrDefault();
    if (firstAlgorithm != null && AlgorithmTypes.Contains(firstAlgorithm.Type))
    {
        SelectedAlgorithm = firstAlgorithm.Type;
    }

    RefreshModelBindings();
    TreeRefreshRequested?.Invoke(window.Id);  // 트리 노드의 IsSelected highlight 갱신
    SelectionChanged?.Invoke();                // ROI overlay active 상태 + 알고리즘 패널 갱신
    return true;
}
```

차이점:
- `SelectTreeNode` 는 WPF TreeView 가 자체 highlight 처리해서 `TreeRefreshRequested` 가 필요 없음.
- 외부 트리거 (ROI 클릭) 인 경우 TreeView 가 모르므로 `TreeRefreshRequested?.Invoke(window.Id)` 로 명시적 rebuild → 트리 노드의 `IsSelected` 가 새 SelectedWindowId 기준으로 다시 계산됨.

#### B. MainWindow.RoiInteraction.cs — 클릭 hit-test

`ImageOverlay_MouseLeftButtonDown` 의 흐름:

1. Drawing 모드 켜져 있으면 `_roiCanvasViewModel.TryBegin` 으로 새 ROI 그리기 (기존).
2. **NEW**: Drawing 모드가 아니면 `TrySelectWindowAtPoint(canvas, point)` 호출 → 클릭 좌표가 어느 Window ROI 안인지 hit-test → 매칭되면 `_viewModel.TrySelectWindowById(...)` 호출.
3. 매칭 없으면 기존처럼 `TryBeginImagePan` 으로 fallback.

Hit-test 로직:

```csharp
private bool TrySelectWindowAtPoint(Canvas canvas, Point canvasPoint)
{
    var width = _imageRuntimeStateService.SourceWidth;
    var height = _imageRuntimeStateService.SourceHeight;
    if (width <= 0 || height <= 0) return false;

    var display = _roiGeometryService.GetImageDisplayRect(canvas.ActualWidth, canvas.ActualHeight, width, height, _viewModel.CurrentImageZoom);
    if (display.IsEmpty) return false;

    var imagePoint = _roiGeometryService.ToImagePixel(canvasPoint, display, width, height);
    var sx = (int)imagePoint.X;
    var sy = (int)imagePoint.Y;

    var hit = _viewModel.Model.Part.Windows
        .Where(w => w.Roi.IsValid && sx >= w.Roi.X && sx < w.Roi.X + w.Roi.Width
                                  && sy >= w.Roi.Y && sy < w.Roi.Y + w.Roi.Height)
        .OrderBy(w => w.Roi.Width * w.Roi.Height)
        .FirstOrDefault();

    return hit != null && _viewModel.TrySelectWindowById(hit.Id);
}
```

- `_roiGeometryService.GetImageDisplayRect(...)` + `ToImagePixel(...)` 로 zoom / letterbox 보정된 좌표 변환.
- 중첩된 Window ROI 가 있을 경우 (보통 없음) 면적이 가장 작은 것 우선 — 안쪽 ROI 가 사용자 의도일 가능성.

### 2.3 비선택 ROI 투명도

`Views/RoiOverlayRenderer.cs` 의 `DrawSingleRoi` 안의 `Rectangle` + `TextBlock` 양쪽에 `Opacity` 추가:

```csharp
Opacity = isActive ? 1.0 : 0.4
```

- 선택된 ROI: `Opacity = 1.0` (완전 불투명).
- 비선택 ROI: `Opacity = 0.4` (40%) — 색 / stroke / fill / label 배경 모두 동일 비율로 후퇴.
- 기존 색상 (yellow / cyan) 과 stroke 두께 (2.2 / 1.4) 차이는 그대로 두고 Opacity 만 추가 — 활성 표시 신호가 색상 + 두께 + 투명도 3중 강화됨.

### 2.4 영향 범위 / MVVM

- View (Renderer / XAML) 는 순수 렌더링.
- Click→Window 선택 흐름의 비즈니스 로직 (어느 모델 Window 를 어떻게 활성화) 은 모두 ViewModel 의 `TrySelectWindowById` 안.
- MainWindow code-behind 의 `TrySelectWindowAtPoint` 는 좌표 변환 + ViewModel 호출만 — 모델 mutation 없음.
- `SelectTreeNode` 의 후처리 (Selected 알고리즘 sync, Refresh) 와 동일한 패턴을 따라 일관성 유지.

## 3. 변경된 파일

- `src/WpfInspectionPrototype/WpfInspectionApp/ViewModels/MainViewModel.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.RoiInteraction.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Views/RoiOverlayRenderer.cs`

## 4. 검증 결과

```
dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0개, 오류 0개, 9.24초

src/WpfInspectionPrototype/WpfInspectionApp/bin/x64/Debug/net48/WpfInspectionApp.exe --smoke-test
→ EXIT=0
```

## 5. 남은 위험 / 다음 확인

- GUI 수동 확인:
  - Part Import 후 여러 Window ROI 가 그려진 상태에서 ROI 안쪽 클릭 → 오른쪽 트리에서 해당 Window 가 강조되는지.
  - 클릭 시 알고리즘 패널 (오른쪽 아래) 도 해당 Window 의 첫 알고리즘 패널로 전환되는지.
  - 비선택 ROI 가 시각적으로 40% 투명도로 후퇴해 보이는지. 선택된 ROI 만 또렷한지.
  - Drawing 모드 (Window ROI 그리기 버튼 누른 상태) 에서는 기존처럼 새 ROI 가 그려지는지 (hit-test 가 drawing 을 가로채지 않는지).
  - Zoom 이 1배가 아닐 때도 클릭 hit-test 가 정확히 매칭되는지 (좌표 변환 검증).
- Algorithm ROI 클릭으로도 해당 알고리즘을 선택하고 싶을 수 있음. 현재는 Window 단위만 선택. 요청 시 추가 가능.
- Window ROI 가 중첩된 경우 가장 작은 것 우선. 사용자가 큰 ROI 를 선택하고 싶으면 트리에서 직접 선택해야 함.
- Opacity 0.4 가 너무 옅거나 진하다고 느끼면 0.3 / 0.5 등으로 조정 가능.

## 6. 다음 작업 추천 순서

1. GUI 수동 확인.
2. (요청 시) Algorithm ROI 클릭도 동일하게 선택 흐름 연결.
3. (요청 시) 비선택 ROI Opacity 수치 미세 조정.
4. (요청 시) 가장 큰 ROI 우선 / 마우스 hover 표시 등 UX 옵션 추가.
5. 새 알고리즘 UI 작업 전, 선행 §4 의 재점검 명령 다시 실행.

## 7. 빌드 / 재현 명령

```powershell
dotnet build src\WpfInspectionPrototype\WpfInspectionApp\WpfInspectionApp.csproj `
  -p:Configuration=Debug -p:Platform=x64

& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe --smoke-test
```
