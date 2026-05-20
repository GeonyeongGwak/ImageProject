# WPF Inspection App Handoff - Window ROI Visual Style

작성일: 2026-05-20
프로젝트: `C:\Users\USER\Documents\NewProject\ImageProject`
선행 문서:
- `docs/2026-05-20-handoff-align-light-mvvm.md`
- `docs/2026-05-20-handoff-cam03-toggle.md`
- `docs/2026-05-20-handoff-cam01-color.md`

## 1. 작업 원칙

- MVVM 위반 여부는 기능 추가 전/후 항상 확인한다.
- View/XAML/code-behind에는 화면 이벤트 연결, 바인딩, UI 표시만 둔다.
- 값 검증, enable/disable 상태 판단, 파라미터 저장/정규화는 ViewModel/Model/Service 쪽에서 처리한다.
- 기존 사용자가 만든 변경은 되돌리지 않는다. `git reset --hard`, `git checkout --` 금지.

## 2. 이번 세션에서 한 작업

### 2.1 Window ROI 표시 스타일 변경 요청

사용자 요청 3가지:
1. Window ROI 사각형 **내부에 색이 아예 없도록** (`Fill` 제거).
2. Window ROI 색은 **옐로우** 로 (active / inactive 모두).
3. Window ROI 와 함께 그려지던 **점선 박스 (margin) 자체를 제거**.

### 2.2 RoiOverlayRenderer.cs 수정

`src/WpfInspectionPrototype/WpfInspectionApp/Views/RoiOverlayRenderer.cs` 의 `DrawSingleRoi` 메서드만 수정.

#### 제거된 것
- Window ROI margin (`isWindowRoi == true` 일 때 그려지던 녹색 점선 사각형). `context.CreateMarginRoi` 호출 + margin Rectangle 생성/추가 블록을 통째로 삭제. `CreateMarginRoi` 자체는 다른 코드에서 쓸 가능성이 있어 context 정의에서는 그대로 유지.

#### 변경된 것
- `activeColor` / `inactiveColor`:
  - Window ROI: 둘 다 `Color.FromRgb(255, 210, 41)` (옐로우 통일). inactive 의 어두운 톤은 제거.
  - Algorithm ROI: 기존 시안 톤 그대로 (`128,223,255` / `54,137,168`).
- Window ROI 의 `Fill`: 기존엔 `Color.FromArgb(34/18, 255, 210, 41)` 의 살짝 옐로우 톤이 채워졌으나 **`Fill = null`** 로 변경 → 사각형 내부 완전 투명.
- Algorithm ROI 의 `Fill` 은 기존 시안 톤 유지.
- `StrokeDashArray`: Window ROI 는 기존에도 solid (`null`) 였고 그대로 유지. Algorithm ROI 만 `{ 2, 2 }` 짧은 점선 유지.

#### 부수 효과
- `Fill = null` 인 사각형은 mouse hit-test 가 stroke 위에서만 잡힘 → ROI 내부 영역 클릭이 부모 `SourceOverlay` Canvas (Background="Transparent") 로 통과한다. ROI 안쪽 클릭으로 이미지/오버레이 이벤트가 정상 동작하므로 오히려 사용성에 +.
- active vs inactive 차이는 색이 아니라 stroke 두께(`2.2 vs 1.4`) 로만 표현됨.

### 2.3 영향 범위

- DrawSingleRoi 이외 함수(`DrawAlignSearchOverlay`, `DrawMarker`, `DrawArrow`) 는 수정 없음.
- Align search ROI 의 시안 점선 / 노란 디텍션 마커 등 Algorithm-쪽 시각화는 그대로 유지.

## 3. 변경된 파일

- `src/WpfInspectionPrototype/WpfInspectionApp/Views/RoiOverlayRenderer.cs`

(MainViewModel / XAML / native / interop / service 어디에도 수정 없음. 변경 표면이 매우 좁음.)

## 4. 검증 결과

```
dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0개, 오류 0개, 5.73초

src/WpfInspectionPrototype/WpfInspectionApp/bin/x64/Debug/net48/WpfInspectionApp.exe --smoke-test
→ EXIT=0
```

## 5. MVVM 점검

- 수정된 파일은 `Views/RoiOverlayRenderer.cs` 한 곳. ViewModel / Model / Service / native 변경 없음.
- Renderer 는 순수 함수형 (Canvas + Context → Shapes). MVVM 경계 위반 없음.
- 선행 문서 §4 의 재점검 명령 4개 모두 매칭 0 유지 확인.

## 6. 남은 위험 / 다음 확인

- GUI 수동 확인 항목:
  - Window ROI 가 노란색 테두리만 보이고 **내부는 완전 투명** 인지 (배경 이미지가 그대로 보여야 함).
  - 녹색 점선 margin 박스가 더 이상 그려지지 않는지.
  - inactive Window ROI 가 active 와 같은 옐로우(255,210,41) 로 보이고, 두께만 더 얇은지.
  - Window ROI 안쪽을 클릭했을 때 image overlay 의 panning / 다른 핸들러가 정상 동작하는지 (Fill=null 의 hit-test 변화).
- Algorithm ROI 의 점선/시안 fill 은 그대로. 사용자가 "전체 점선 제거" 의도였다면 알려주실 때 추가 작업.
- `RoiOverlayRenderContext.CreateMarginRoi` 는 현재 다른 호출 지점이 없어 dead code 일 수 있음. 다른 작업 흐름(예: ImageRuntimeStateService) 에서 쓰일 가능성도 있어 이번엔 정리 보류.
- 사용자 ROI 그리기 preview (`context.PreviewRoi`) 도 동일 `DrawSingleRoi` 경유 → preview 단계에서도 동일 스타일 적용됨. 의도된 동작.

## 7. 다음 작업 추천 순서

1. 사용자 GUI 에서 §6 의 4개 항목 직접 확인.
2. (요청 시) Algorithm ROI 의 점선/Fill 도 동일하게 정리.
3. (요청 시) `context.CreateMarginRoi` 와 호출처를 추적해서 dead code 인 경우 정리.
4. 새 알고리즘 UI 작업 전, 선행 §4 의 재점검 명령 다시 실행.

## 8. 빌드 / 재현 명령

```powershell
dotnet build src\WpfInspectionPrototype\WpfInspectionApp\WpfInspectionApp.csproj `
  -p:Configuration=Debug -p:Platform=x64

& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe --smoke-test
```
