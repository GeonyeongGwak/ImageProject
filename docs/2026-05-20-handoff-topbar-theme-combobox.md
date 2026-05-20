# WPF Inspection App Handoff - TopBar Clipping Fix + Theme ComboBox

작성일: 2026-05-20
프로젝트: `C:\Users\USER\Documents\NewProject\ImageProject`
선행 문서:
- `docs/2026-05-20-handoff-align-light-mvvm.md`
- `docs/2026-05-20-handoff-cam03-toggle.md`
- `docs/2026-05-20-handoff-cam01-color.md`
- `docs/2026-05-20-handoff-window-roi-style.md`
- `docs/2026-05-20-handoff-import-progress.md`

## 1. 작업 원칙

- MVVM 위반 여부는 기능 추가 전/후 항상 확인한다.
- View/XAML/code-behind에는 화면 이벤트 연결, 바인딩, UI 표시만 둔다.
- 값 검증, enable/disable 상태 판단, 파라미터 저장/정규화는 ViewModel/Model/Service 쪽에서 처리한다.
- 기존 사용자가 만든 변경은 되돌리지 않는다. `git reset --hard`, `git checkout --` 금지.

## 2. 이번 세션에서 한 작업

### 2.1 사용자 요청

상단 TopBar 의 `Dark / Light / Pink` 테마 옵션 버튼과 `C++ BRIDGE ACTIVE` 라벨이 윗부분이 잘려서 보이는 현상. "이쪽 부분이 짤려있어 수정해줘".

### 2.2 원인 분석

- TopBar 행 높이가 `42px` 였고, Border `Padding="12,6"` + `BorderThickness="1"` 적용 시 콘텐츠 영역은 약 `28px`.
- ThemeOption 버튼의 명목상 `Height="24"` 였지만 실제 콘텐츠 (StackPanel + 10x10 색 칩 + TextBlock 12pt) 가 패딩/라인 높이까지 더해 25~28px 근처가 되어 vertical centering 시 윗쪽으로 1~2px 튀어나감.
- WPF Border 는 기본으로 자식 클리핑을 하지 않아 튀어나간 부분이 부모 Grid 의 `Margin="10"` 상한과 Window 자체 클립 영역 사이에서 잘려 보임.

### 2.3 수정 내용

#### A. TopBar 행 높이 확장 — `42` → `50`

`MainWindow.xaml` `<Grid.RowDefinitions>`:

```xml
<RowDefinition Height="50" />   <!-- 이전 42 -->
<RowDefinition Height="*" />
```

이유:
- 8px 여유 확보로 어떤 인라인 컨트롤 (Button / ComboBox / TextBlock) 도 vertical centering 후 클리핑 안 됨.
- `WindowChrome.CaptionHeight="50"` 과 자연스럽게 일치 — TopBar 전체가 드래그 가능 영역이고 인터랙티브 컨트롤만 `IsHitTestVisibleInChrome="True"` 로 빠짐.

#### B. 테마 선택 UI — 가로 Button 3개 → ComboBox

기존: `ItemsControl ItemsSource="{Binding ThemeOptions}"` + `ItemsPanel = horizontal StackPanel` + 각 항목마다 Button (`ThemeOptionButton` 스타일).

변경: 단일 `<ComboBox>`:

```xml
<ComboBox Grid.Column="2"
          Margin="0,0,12,0"
          MinWidth="130"
          VerticalAlignment="Center"
          WindowChrome.IsHitTestVisibleInChrome="True"
          ItemsSource="{Binding ThemeOptions}"
          SelectedValuePath="Key"
          SelectedValue="{Binding SelectedThemeKey, Mode=TwoWay, UpdateSourceTrigger=PropertyChanged}">
    <ComboBox.ItemTemplate>
        <DataTemplate>
            <StackPanel Orientation="Horizontal">
                <Border Width="10" Height="10" CornerRadius="2"
                        Background="{Binding SwatchBrush}"
                        Margin="0,0,6,0" VerticalAlignment="Center" />
                <TextBlock Text="{Binding DisplayName}" VerticalAlignment="Center" />
            </StackPanel>
        </DataTemplate>
    </ComboBox.ItemTemplate>
</ComboBox>
```

- `MinWidth="130"` 으로 짤림 방지.
- `SelectedValue` 가 TwoWay 로 `SelectedThemeKey` 와 묶임 → setter 트리거 → `SetSelectedTheme` 호출 → `ThemeChanged` 이벤트로 색상 적용.
- 항목 표시는 기존과 동일하게 색 칩 + 이름.

#### C. `SelectedThemeKey` setter public + 재진입 가드

`MainViewModel.cs` 의 setter 가 기존 `private set` → `public set`. setter 내부에서 동일 키 재할당 시 무시, 다른 키일 때만 `SetSelectedTheme(value)` 호출. `SetSelectedTheme` 는 백킹 필드에 직접 쓰고 `SetProperty(ref _selectedThemeKey, next.Key, nameof(SelectedThemeKey))` 로 INPC 발화 — 그 INPC 가 다시 ComboBox 에 같은 값으로 돌아와도 setter 가 동일 값 가드로 무시 → 재진입 없음.

### 2.4 영향 범위

- `SelectThemeCommand` 는 그대로 남음. ComboBox 가 SelectedValue 바인딩으로 대체했지만, 다른 곳에서 (예: 메뉴 / 단축키) command 를 부를 수 있어 제거하지 않음.
- `ThemeOptionButton` 스타일은 더 이상 참조처가 없지만 dead code 가능성 — 다른 곳에서 쓰는지 확신 없어 이번엔 남겨둠.
- `ThemeOptions` 컬렉션의 `ButtonBackground` / `ButtonBorderBrush` 도 ComboBox 가 안 쓰지만 마찬가지로 남김.

## 3. 변경된 파일

- `src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.xaml`
- `src/WpfInspectionPrototype/WpfInspectionApp/ViewModels/MainViewModel.cs`

## 4. 검증 결과

```
dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0개, 오류 0개, 9.86초

src/WpfInspectionPrototype/WpfInspectionApp/bin/x64/Debug/net48/WpfInspectionApp.exe --smoke-test
→ EXIT=0
```

### MVVM 점검
- 선택 로직은 ViewModel (`SelectedThemeKey` setter → `SetSelectedTheme`) 에 그대로 존재.
- XAML 은 순수 바인딩만 — Click / SelectionChanged 코드비하인드 없음.
- 선행 문서 §4 재점검 명령 4개 모두 매칭 0.

## 5. 남은 위험 / 다음 확인

- GUI 수동 확인:
  - TopBar 의 ComboBox 선택값이 시작 시 "Dark" 로 보이는지 (기본).
  - ComboBox 드롭다운에서 Light / Pink 선택 시 즉시 테마 적용되는지.
  - C++ BRIDGE ACTIVE 라벨 / 시스템 버튼 (Min/Max/Close) 도 더 이상 잘림 없는지.
  - 창 크기 변경 / 최대화 시에도 정상.
- `ThemeOptionButton` 스타일과 `ThemeOptionViewModel.ButtonBackground` / `ButtonBorderBrush` 가 dead code 인지 확인 후 정리 가능 (다음 작업 후보).
- 드롭다운 항목의 SwatchBrush 가 테마 색을 정확히 반영하는지 시각 확인.

## 6. 다음 작업 추천 순서

1. 사용자 GUI 에서 §5 의 수동 확인 항목 검증.
2. (요청 시) `ThemeOptionButton` 스타일 + `ThemeOptionViewModel.ButtonBackground/BorderBrush` dead code 정리.
3. (요청 시) Align Panel 의 하드코딩 배지 색 (`#3CFF50` 등) 도 테마 추종으로 교체.
4. 새 알고리즘 UI 작업 전, 선행 §4 의 재점검 명령 다시 실행.

## 7. 빌드 / 재현 명령

```powershell
dotnet build src\WpfInspectionPrototype\WpfInspectionApp\WpfInspectionApp.csproj `
  -p:Configuration=Debug -p:Platform=x64

& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe --smoke-test
```
