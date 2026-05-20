# WPF Inspection App Handoff - 트리 선택 시 자동 스크롤 (BringIntoView)

작성일: 2026-05-20
프로젝트: `C:\Users\USER\Documents\NewProject\ImageProject`
선행 문서:
- `docs/2026-05-20-handoff-roi-click-and-opacity.md` 외 7개.

## 1. 작업 원칙

- MVVM 위반 여부는 기능 추가 전/후 항상 확인한다.
- View/XAML/code-behind 에는 화면 이벤트 연결, 바인딩, UI 표시만 둔다.
- 값 검증, enable/disable 상태 판단, 파라미터 저장/정규화는 ViewModel/Model/Service.

## 2. 이번 세션에서 한 작업

### 2.1 사용자 요청

ROI 클릭 시 오른쪽 셀의 Window 가 선택은 되는데, 그 셀이 viewport 바깥(스크롤로 내려야 보이는 위치)이면 화면이 같이 따라 내려오지 않아 어디가 선택됐는지 안 보임. 자동으로 스크롤되어 보이게 해줘.

### 2.2 변경 내용

`TreeViewItem` 스타일에 **`Selected` 이벤트 핸들러** 추가. 어떤 경로로든 (사용자가 트리 직접 클릭 / ROI 클릭 → VM.TrySelectWindowById → IsSelected 바인딩) TreeViewItem 의 IsSelected 가 true 가 되면 자동으로 `BringIntoView()` 호출.

#### MainWindow.xaml — EventSetter 추가

```xml
<Style TargetType="TreeViewItem">
    ...
    <EventSetter Event="PreviewMouseRightButtonDown" Handler="InspectionTreeItem_PreviewMouseRightButtonDown" />
    <EventSetter Event="Selected" Handler="InspectionTreeItem_Selected" />
    ...
</Style>
```

#### MainWindow.xaml.cs — 핸들러

```csharp
private void InspectionTreeItem_Selected(object sender, RoutedEventArgs e)
{
    if (sender is TreeViewItem item && ReferenceEquals(e.OriginalSource, item))
    {
        item.BringIntoView();
    }
}
```

- `e.OriginalSource == sender` 체크로 진짜 선택된 leaf TreeViewItem 일 때만 동작. `Selected` 이벤트는 자식 → 부모로 bubble 되므로 가드가 없으면 부모 TreeViewItem 도 같은 이벤트를 받아서 스크롤이 흔들림.
- `BringIntoView()` 는 부모 ScrollViewer 체인을 walk-up 하면서 자기를 viewport 안으로 이동 — TreeView 가 `ScrollViewer.VerticalScrollBarVisibility="Auto"` 로 감싸진 상태라 그대로 동작.
- `Selected` 이벤트는 컨테이너가 이미 layout 된 시점에 발화 — Dispatcher defer 불필요.

### 2.3 영향 범위

- 직접 트리 클릭 / ROI 클릭 / Expand-Collapse-All 후 첫 노드 선택 / Part Import 후 첫 Window 자동 선택 등 모든 선택 경로에서 동일하게 동작.
- 사용자가 의도적으로 트리를 스크롤해서 위/아래 다른 영역을 보고 있을 때, 외부 트리거(예: ROI 클릭) 로 선택이 바뀌면 viewport 가 따라 이동.
- 같은 노드를 다시 선택하면 `Selected` 이벤트가 다시 발화하지 않으니 추가 스크롤 없음.

### 2.4 MVVM 점검

- 코드비하인드는 View 의 표시 동작 (BringIntoView) 만 처리. 모델/VM 변경 없음.
- 선택 상태는 여전히 ViewModel 의 `Model.SelectedWindowId` 가 source of truth.

## 3. 변경된 파일

- `src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.xaml`
- `src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.xaml.cs`

## 4. 검증 결과

```
dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0개, 오류 0개, 10.07초

src/WpfInspectionPrototype/WpfInspectionApp/bin/x64/Debug/net48/WpfInspectionApp.exe --smoke-test
→ EXIT=0
```

## 5. 남은 위험 / 다음 확인

- GUI 수동 확인:
  - 트리에 Window 가 많아서 스크롤바가 생긴 상태 만들기 (Part Import).
  - 위쪽 ROI 클릭 → 트리 상단으로 스크롤되어 해당 Window highlight.
  - 아래쪽 ROI 클릭 → 트리 하단으로 스크롤되어 해당 Window highlight.
  - 사용자가 수동으로 다른 위치로 스크롤한 상태에서 ROI 클릭 시 viewport 가 정확히 이동하는지.
- 만약 BringIntoView 가 매번 발화해서 사용자가 트리 클릭으로 이미 보고 있던 노드도 재선택 시 살짝 점프하는 느낌이 든다면, 이전 SelectedItem 과 같으면 skip 하는 가드 추가 가능.

## 6. 다음 작업 추천 순서

1. GUI 수동 확인.
2. (요청 시) BringIntoView 시 부드러운 애니메이션 / 스크롤 가드 추가.
3. (요청 시) Algorithm ROI 클릭으로 알고리즘 노드까지 선택 + 스크롤.

## 7. 빌드 / 재현 명령

```powershell
dotnet build src\WpfInspectionPrototype\WpfInspectionApp\WpfInspectionApp.csproj `
  -p:Configuration=Debug -p:Platform=x64

& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe --smoke-test
```
