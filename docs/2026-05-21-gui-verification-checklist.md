# GUI 수동 검증 체크리스트 (P0 통합)

작성일: 2026-05-21
범위: 직전까지의 23개 인수인계 문서가 다룬 변경 사항 전체 1회 회귀 검증
샘플 데이터: `src/WpfInspectionPrototype/Models/1.zip` (RefID_2), `Models/2.zip` (U25)

## 환경 준비 확인 (자동화로 이미 통과)
- ✅ Native `MptiBridge.dll` 빌드 (x64 Debug)
- ✅ Managed `WpfInspectionApp` 빌드 (경고 0 / 오류 0)
- ✅ Smoke test EXIT=0
- ✅ Models 폴더 로컬 보존 (1, 1.zip, 2, 2.zip, 123, PCB-2026-0406-A1.json)

## 실행

```powershell
& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe
```

## 체크 항목 (14개)

각 항목 옆에 ✅ / ⚠ / ❌ 로 표시. 이상 발견 시 어디서 어떤 증상인지 함께 기록해서 알려주세요.

### 1. 시작 화면 (default image 자동 로드 제거)
- [ ] 앱 켜자마자 CAM-01 / CAM-02 / CAM-03 **모두 빈 상태** 인가?
- [ ] 상태 메시지가 `Ready. Use Part Import or Load PTT to begin.` 인가?
- ❌ 만약 자동으로 이미지가 뜨면 → `default-image-and-roi-mm.md` 회귀

### 2. TopBar 짤림 + 테마 ComboBox
- [ ] 상단 TopBar 의 `Dark / Light / Pink` ComboBox 가 짤림 없이 표시되는가?
- [ ] 우측 `C++ BRIDGE ACTIVE` 라벨도 잘 보이는가?
- [ ] Min/Max/Close 시스템 버튼 보이는가?
- [ ] ComboBox 드롭다운에서 Light / Pink 선택 시 즉시 테마 적용?
- ❌ 회귀: `topbar-theme-combobox.md`

### 3. Move/Select 모드 (M 키)
- [ ] 키보드 `M` 키 누르면 TopBar 에 `MODE MOVE` 표시?
- [ ] 다시 `M` 누르면 `MODE SELECT` 로 돌아옴?
- [ ] TextBox / ComboBox 안에 있을 때는 `M` 입력해도 모드 안 바뀜?
- ❌ 회귀: `viewer-move-select-mode.md`

### 4. Part Import 진행 오버레이
- [ ] `Part Import` 버튼 → `Models/1.zip` 안의 `RefID_2/RefID_2.xml` 선택
- [ ] 즉시 화면 중앙에 `IMPORTING PART` 모달 오버레이 표시?
- [ ] 단계 텍스트가 차례로 변하는지 (`Reading part file...` → `Parsing XML...` → `Loading PTT...` 등)?
- [ ] PTT 로드 중에도 ProgressBar 애니메이션 멈추지 않는가? (= UI freezing 사라졌는지)
- [ ] 완료 후 오버레이 자동 사라짐?
- [ ] 오버레이가 떠 있을 때 다른 버튼 / 트리 클릭이 차단되는가?
- ❌ 회귀: `import-progress.md`

### 5. CAM-01 컬러 이미지
- [ ] Part Import 완료 후 CAM-01 헤더가 `CAM-01 | COLOR (R/G/B)` 로 표시?
- [ ] CAM-01 에 컬러 이미지가 보이는가? (그레이가 아니라 R/G/B 합성)
- [ ] PTT 원본 비율 (가로/세로) 일치?
- ❌ 회귀: `cam01-color.md`

### 6. CAM-03 토글
- [ ] 기본 상태에서 CAM-03 헤더가 `CAM-03 | BINARY RESULT`, 토글 버튼 라벨이 `Origin` ?
- [ ] 토글 버튼 클릭 → 라벨이 `Binary` 로 바뀌고 타이틀이 `CAM-03 | ORIGIN`, 영상이 2D Light 합성 그레이로 전환?
- [ ] 다시 클릭 → 원상복귀?
- ❌ 회귀: `cam03-toggle.md`

### 7. Light 슬라이더 → CAM 영역 영향 분리
- [ ] LightControl 패널의 R / G / B / W 슬라이더 조작
- [ ] CAM-01 (컬러) 가 **변하지 않는가**? ← Light 무관 핵심 검증
- [ ] CAM-03 Origin 모드일 때만 그레이 영상이 따라 변하는가?
- [ ] CAM-03 Binary 모드일 때는 binary 가 갱신되는가? (Origin 모드면 binary 갱신 skip)
- ❌ 회귀: `cam01-color.md` + `cam03-toggle.md`

### 8. ROI 표시 스타일
- [ ] Window ROI 가 **노란 테두리만** 보이고 **내부는 완전 투명** 인가? (배경 이미지 그대로 비침)
- [ ] 녹색 점선 margin 박스가 더 이상 안 보이는가?
- [ ] inactive Window ROI 도 같은 옐로우, 두께만 얇은가?
- [ ] ROI 라벨이 `Window ROI 1 8.68x13.61 mm` 같은 **mm 단위** 로 표시?
- [ ] 선택되지 않은 ROI 의 투명도가 40% 로 후퇴되어 보이는가?
- ❌ 회귀: `window-roi-style.md`, `default-image-and-roi-mm.md`, `roi-click-and-opacity.md`

### 9. ROI 클릭 → 트리 선택 + 알고리즘 패널 + viewport 스크롤
- [ ] ROI 안쪽 클릭 → 오른쪽 트리에서 해당 Window highlight?
- [ ] 동시에 오른쪽 아래 알고리즘 패널이 해당 Window 의 첫 알고리즘으로 전환?
- [ ] 트리 viewport 밖의 ROI 를 클릭하면 트리가 자동으로 그 위치로 스크롤?
- [ ] Drawing 모드 (Window ROI 그리기 버튼) 에서는 새 ROI 그리기가 정상?
- [ ] MOVE 모드 (M 키) 에서는 ROI 클릭이 무시되고 image pan 만 됨?
- ❌ 회귀: `roi-click-and-opacity.md` + `tree-bring-into-view.md` + `viewer-move-select-mode.md`

### 10. Zoom / Pan 안정성
- [ ] 마우스 휠로 줌인/아웃, 패널 가장자리 근처에서도 휠 입력이 끊김 없이 동작?
- [ ] 줌 1배 초과 상태에서 image 좌클릭 드래그로 pan 가능?
- [ ] Pan 중에 ROI overlay 가 image 와 같이 따라 움직이는가?
- ❌ 회귀: `zoom-pan-input-stability.md`

### 11. U25 (Models/2.zip) 회전 + Algorithm ROI 검증
- [ ] `Models/2.zip` 의 `U25/U25.xml` Import
- [ ] **`AlgoHeight_Diff 0.01x0.01 mm` 같은 유령 ROI 라벨이 안 보이는가?** (BROI2 오매칭 가드)
- [ ] U25 의 `<a>270.0</a>` 회전 ROI 가 가로/세로 비율 맞게 표시? (W/H 스왑)
- [ ] U25 의 `ROI1_mm` CSV ROI (예: AlgoHeight_Diff 의 `-4.226048,-6.653836,4.226048,6.653836`) 가 실제 위치/크기 맞게 그려지는가?
- ❌ 회귀: `algorithm-roi-degenerate.md`, `roi-angle-swap.md`, `roi-csv-align-teach-flow-commonization.md`

### 12. RefID_2 (Models/1.zip) 비교 — 회귀 없는지
- [ ] 1.zip 의 `RefID_2/RefID_2.xml` Import
- [ ] ROI 와 algorithm ROI 가 이전처럼 정상 표시? (U25 변경이 1번 part 에 회귀 안 일으켰는지)
- [ ] Light import 후 default fallback (`Common.RedValue=100` 등) 으로 패널에 잘 들어가는가?

### 13. Align 패널 변경 사항
- [ ] Align 알고리즘 선택 후 `Lead Teach` 버튼 노출?
- [ ] Lead Teach 클릭 → Search ROI 2 개가 자동 설정 + Search Num=2?
- [ ] `Delete` 키 (Search 모드) → Window 가 삭제되지 않고 active Search ROI slot 만 비움?
- [ ] `Next ROI` 버튼 → Window 전환이 아니라 Search ROI 슬롯 순환?
- [ ] Search Size X/Y 변경 → Window ROI 는 그대로, Search ROI 만 리사이즈?
- ❌ 회귀: `align-lead-window-teach.md`, `align-delete-search-slot.md`, `align-next-roi-slot.md`, `align-search-size-slot.md`

### 14. User Light 표현식 UI
- [ ] User Light 카드형 UI 가 표시되는가?
- [ ] Operator 버튼 (`+`/`-`/blank) 이 cycling?
- [ ] 마지막 빈 operator → `+`/`-` 누르면 새 셀 자동 추가?
- [ ] 끝에서 두 번째 operator → blank 로 돌아오면 trailing 셀 자동 제거?
- [ ] `MIX` / `CURRENT` 토글 동작?
- [ ] 셀의 좌측 채널명 (R/G/B/W) 클릭 → 해당 채널 100, 나머지 0 quick preset?
- ❌ 회귀: `user-light-expression-ui.md`

## 결과 보고 양식

이상 항목 발견 시 다음 정보 함께 알려주세요:
- 어느 번호 (예: `9-3`)
- 어떤 행위 (예: `ROI 클릭`)
- 어떤 증상 (예: `트리는 highlight 되는데 viewport 가 안 따라옴`)
- 어떤 part 파일 (예: `RefID_2`)
- 스크린샷 가능하면 첨부

전부 통과면 → "전부 OK" 한 줄로 알려주시면 P1 (Align parity gaps) 로 진행.
