# WPF Inspection App Handoff - CAM-01 Color Image

작성일: 2026-05-20
프로젝트: `C:\Users\USER\Documents\NewProject\ImageProject`
참조 프로젝트: `D:\Work\GGY\pemtoFrameworkAll_R_4.0.0.7`
선행 문서:
- `docs/2026-05-20-handoff-align-light-mvvm.md`
- `docs/2026-05-20-handoff-cam03-toggle.md`

## 1. 작업 원칙

- MVVM 위반 여부는 기능 추가 전/후 항상 확인한다.
- View/XAML/code-behind에는 화면 이벤트 연결, 바인딩, UI 표시만 둔다.
- 값 검증, enable/disable 상태 판단, 파라미터 저장/정규화는 ViewModel/Model/Service 쪽에서 처리한다.
- 기존 사용자가 만든 변경은 되돌리지 않는다. worktree에는 이번 작업 외 변경도 많으므로 `git reset --hard`, `git checkout --` 사용 금지.

## 2. 이번 세션에서 한 작업

### 2.1 CAM-01 컬러 이미지 도입

기존: CAM-01 은 `SourceImage` (2D Light 합성 그레이) 를 보여줬다. Light 슬라이더를 조작하면 그레이 영상도 같이 흔들렸다.

변경 후:
- CAM-01 은 **컬러 이미지** 를 보여준다. PTT 의 TR/TG/TB 채널을 그대로 BGRA32 로 합성한 결과.
- Light 슬라이더에는 **반응하지 않는다**. PTT 가 로드되는 시점에 1 회만 native 가 합성해서 반환하고, ViewModel 이 그 결과를 보관.
- 사용자가 명시한 baseline 가중치 R=G=B=100 사용. Normal Light 의 가중치 범위는 0~200 이고 100 은 1.0 배 (채널 그대로) 이므로 실제로는 채널 raw 값을 그대로 BGR 평면에 배치하는 것과 동등 — 별도 스케일링 없음.

### 2.2 Native 측 변경 (MptiBridge.dll)

`src/WpfInspectionPrototype/MptiBridge/MptiBridge.cpp`:

- 새 export `MptiBridgeRenderColorPreview(int width, int height, unsigned char* output, int outputLength, wchar_t* message, int messageLength)` 추가.
- 동작:
  1. `EnsureInitialized` 가드.
  2. `MPTI_GetPttFileChannel(0/1/2)` 로 TR / TG / TB 포인터 획득.
  3. 픽셀마다 `output[i*4 + 0] = blue[i]; +1 = green[i]; +2 = red[i]; +3 = 255` 로 BGRA32 인터리브 출력.
  4. 실패 경로: 인자 NULL / 사이즈 invalid / output buffer 부족 / 채널 NULL / std::exception / unknown.
- `MptiBridgeRenderLightPreview` 와 동일한 에러 코드 / 메시지 컨벤션 사용.

빌드:
```
"C:/Program Files/Microsoft Visual Studio/2022/Professional/MSBuild/Current/Bin/MSBuild.exe" \
  src/WpfInspectionPrototype/MptiBridge/MptiBridge.vcxproj \
  -p:Configuration=Debug -p:Platform=x64
```
결과: `src/WpfInspectionPrototype/bin/Debug/x64/MptiBridge.dll` (12,616,192 bytes). dumpbin 으로 `MptiBridgeRenderColorPreview` export 존재 확인 완료. WpfInspectionApp 빌드 시 자동으로 net48 출력 폴더에 복사됨.

### 2.3 Managed 측 변경

#### Interop (`MptiNativeBridge.cs`)
- `MptiColorPreviewResult` record 추가.
- `MptiBridgeRenderColorPreview` P/Invoke 선언.
- `RenderColorPreview(int width, int height)` 정적 메서드 추가 — `width * height * 4` byte 버퍼 할당 후 호출, 결과를 `MptiColorPreviewResult` 로 래핑.

#### Service (`PttLightPreviewService.cs`, `IPttLightPreviewService.cs`)
- 새 record `PttColorPreviewResult` 추가 (Success / Available / StatusMessage / ColorImage / Width / Height).
- 인터페이스에 `RenderColor(int width, int height)` 추가.
- 구현: native 호출 결과를 `BitmapSource.Create(Bgra32, stride=width*4)` 로 감싸고 `Freeze()` 한 뒤 반환.

#### ViewModel (`MainViewModel.cs`)
- 새 필드 `_cam01ColorImage` (ImageSource?).
- 새 public property `Cam01ColorImage` (private setter, INPC).
- `ApplyPttLoad()` 의 success 경로에서 `ApplyPttColorPreview()` 호출 추가.
- 새 private 메서드 `ApplyPttColorPreview()`:
  - `_lastPttWidth/_lastPttHeight` 검증.
  - `_pttLightPreviewService.RenderColor(...)` 호출.
  - 성공 시 `Cam01ColorImage = color.ColorImage`, 실패 시 `null`.
- `Light 슬라이더` 흐름인 `HandleLightPanelChanged()` 에서는 `Cam01ColorImage` 를 건드리지 **않음** — Light 영향 분리 의도 보존.
- PTT 로드 실패 경로에서는 `Cam01ColorImage = null` 로 초기화.

#### View (`MainWindow.xaml`)
- CAM-01 헤더 타이틀: `"CAM-01 | 2D SOURCE"` → `"CAM-01 | COLOR (R/G/B)"`.
- CAM-01 Image 바인딩: `Source="{Binding SourceImage}"` → `Source="{Binding Cam01ColorImage}"`.
- code-behind 수정 없음.

### 2.4 다른 영역 영향

- `SourceImage` 는 그대로 보존 — CAM-03 의 Origin 모드, threshold preview 의 source, 알고리즘 파이프라인 등 기존 소비자 모두 변경 없음.
- CAM-03 (직전 세션 토글 구현) 동작 그대로:
  - Origin 모드: `SourceImage` (Light 합성 그레이) 표시.
  - Binary 모드: `BinaryImage` 표시.
- CAM-02 (PTT 메타 / 빈 placeholder) 도 그대로.

## 3. 변경된 파일

- `src/WpfInspectionPrototype/MptiBridge/MptiBridge.cpp`
- `src/WpfInspectionPrototype/WpfInspectionApp/Interop/MptiNativeBridge.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/IPttLightPreviewService.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/PttLightPreviewService.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/ViewModels/MainViewModel.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.xaml`

## 4. 검증 결과

### Native build (x64 Debug)
- `MptiBridge.vcxproj` MSBuild 성공.
- 사전 존재하던 LNK4217 경고 외 신규 경고 없음.
- `dumpbin /exports MptiBridge.dll` → `MptiBridgeRenderColorPreview` 269번 ordinal 로 export 확인.
- 출력 DLL 이 WpfInspectionApp net48 출력 디렉토리에 자동 복사됨 (동일 size / timestamp 확인).

### Managed build (x64 Debug)
- `dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64`
- 경고 0개, 오류 0개, 7.24초.

### Smoke test
- `WpfInspectionApp.exe --smoke-test` → EXIT=0.

### 정적 MVVM 점검 (선행 §4 명령 4개)
- 모두 매칭 0.

## 5. MVVM 점검 결과

- 컬러 합성 / 채널 선택 / Light 분리 결정은 모두 native + Service + ViewModel 안에 존재.
- View 는 `{Binding Cam01ColorImage}` 한 줄 변경만 받음. 이벤트 핸들러 / code-behind 추가 없음.
- ViewModel 의 `Cam01ColorImage` setter 는 private — View 가 임의로 갈아끼우지 못함.
- `MainWindow.xaml.cs` / `Views/*.xaml.cs` 에서 채널 / Light 키 직접 다루는 코드 없음.

## 6. 남은 위험 / 다음 확인

- 실제 GUI 에서 CAM-01 컬러 표시 확인 필요:
  - PTT 가 정상 로드된 직후 CAM-01 가 R/G/B 합성 컬러로 나타나는지.
  - Light 슬라이더를 움직였을 때 CAM-01 은 **변하지 않고**, CAM-03 Origin 만 변하는지.
  - CAM-01 의 가로/세로 비율이 PTT 원본과 일치하는지 (stride = width * 4).
- 컬러 합성에 사용한 채널은 TR/TG/TB (인덱스 0/1/2) 만. MIDDLE / BOTTOM / SIDE 조명 / White 채널은 사용하지 않음. 사용자가 다른 채널 조합을 원하면 native 의 `ComposeColorPreview` 분기 추가 필요.
- 가중치 R=G=B=100 baseline 외 다른 강도(예: R=200, G=50 같은 사용자 정의) 가 필요하면 native export 시그니처에 가중치 파라미터를 추가하고 픽셀 루프에서 `(channel * value) / 100` clamp 255 처리.
- `LegacyPttImageLoader` 는 여전히 다른 경로(직접 MPTILib_Algo.dll DllImport) 로 동일한 RGB 합성을 하고 있음. 두 경로가 충돌하지 않는지(같은 PTT 가 두 곳에서 동시에 로드되지 않는지) 화면 비교로 확인 권장.
- `BitmapSource.Create` 에 `Freeze()` 했으므로 cross-thread 사용 안전. 다만 매우 큰 PTT (예: 10K x 10K) 에서는 4 byte/pixel × 픽셀 수 만큼 한 번에 할당됨 — OOM 위험 시 chunked 처리 필요.

## 7. 다음 작업 추천 순서

1. 사용자 GUI 에서 §6 의 첫 두 항목 직접 검증 (PTT 로드 → 컬러 표시, Light 슬라이더 → CAM-01 불변).
2. 컬러 모드의 zoom / pan 이 기존 그레이 모드와 동일하게 동작하는지 확인. `Image.Stretch="Uniform"` 과 `ScaleTransform` 바인딩은 그대로라 작동해야 함.
3. (요청 시) CAM-01 컬러 합성에 사용자 정의 R/G/B 가중치를 받아들이는 옵션 (예: `MainViewModel` 에 `Cam01Red/Green/Blue` 슬라이더 + native 시그니처 확장).
4. (요청 시) Align Panel 의 하드코딩 배지 색 (`#3CFF50`, `#FFD229`, `#06111C`) 을 테마 추종으로 교체.
5. 새 알고리즘 UI 작업 전, 선행 §4 의 재점검 명령 다시 실행.

## 8. 빌드 / 재현 명령

```powershell
# 1. Native (수정 시 매번 필요)
& 'C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe' `
  src\WpfInspectionPrototype\MptiBridge\MptiBridge.vcxproj `
  -p:Configuration=Debug -p:Platform=x64

# 2. Export 확인 (선택)
& 'C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\dumpbin.exe' `
  /exports src\WpfInspectionPrototype\bin\Debug\x64\MptiBridge.dll | Select-String 'MptiBridgeRender'

# 3. Managed
dotnet build src\WpfInspectionPrototype\WpfInspectionApp\WpfInspectionApp.csproj `
  -p:Configuration=Debug -p:Platform=x64

# 4. Smoke
& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe --smoke-test
```
