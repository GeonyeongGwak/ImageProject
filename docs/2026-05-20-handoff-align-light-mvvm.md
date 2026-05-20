# WPF Inspection App Handoff - Align / Light / MVVM

작성일: 2026-05-20  
프로젝트: `C:\Users\USER\Documents\NewProject\ImageProject`  
참조 프로젝트: `D:\Work\GGY\pemtoFrameworkAll_R_4.0.0.7`

## 1. 작업 원칙

- MVVM 위반 여부는 기능 추가 전/후 항상 확인한다.
- View/XAML/code-behind에는 화면 이벤트 연결, 바인딩, UI 표시만 둔다.
- 값 검증, enable/disable 상태 판단, 파라미터 저장/정규화는 ViewModel/Model/Service 쪽에서 처리한다.
- 기존 사용자가 만든 변경은 되돌리지 않는다. 현재 worktree에는 이번 작업 외 변경도 많으므로 `git reset --hard`, `git checkout --` 사용 금지.

## 2. 참고한 참조 프로젝트 파일

Align:
- `pemtoFramework\MVPPattern\InspWnd\CAlgorithm\CAlgoAlign.cs`
- `pemtoFramework\MVPPattern\InspWnd\CAlgorithm\SAlgoAling.cs`
- `pemtoFramework\MVPPattern\TeachingAlgorithm\UI\ucAlign.cs`
- `pemtoFramework\MVPPattern\TeachingAlgorithm\UI\ucAlign.Designer.cs`
- `MPTILib\PInspAlgo\INSPALGO_ALIGNE.*`

Light:
- `pemtoFramework\01.UI\UserControl\Teaching\LightColor\ucLightControl_Normal.cs`
- `pemtoFramework\01.UI\UserControl\Teaching\LightColor\ucLightControl_Normal.Designer.cs`
- `pemtoFramework\01.UI\UserControl\Teaching\LightColor\ucLightControl_User.cs`
- `pemtoFramework\01.UI\UserControl\Teaching\LightColor\ucLightControl_User.Designer.cs`
- `pemtoLib\pemtoFramework\MVPPattern\InspAlgorithm\InspAlgoBase.cs`
- `pemtoLib\pemtoFramework\02.DataClass\InspAlgoLight.cs`

## 3. 현재 완료된 내용

### 3.1 Align MVVM/상태 정리

- Align 입력값 정규화/검증은 `AlignPanelStateService`로 분리했다.
- `Use2D`, `Use3D`, `UseShift`, `UseAngle`, `SameSize`에 따른 enable/disable 판단은 ViewModel 중심으로 정리했다.
- `NormalizeDoubleText` 호출 중 TextBox 바인딩이 재진입하며 StackOverflow가 나는 흐름은 suppress guard로 막았다.
- Align code-behind는 UI 이벤트를 ViewModel 요청으로 전달하는 역할만 하도록 제한했다.

주요 파일:
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/AlignPanelStateService.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/IAlignPanelStateService.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/ViewModels/AlignPanelViewModel.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Views/AlignPanelView.xaml`
- `src/WpfInspectionPrototype/WpfInspectionApp/Views/AlignPanelView.xaml.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.AlignPanelEvents.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.AlignPanelHandlers.cs`

### 3.2 Algorithm panel 공통 이벤트/ShapeX 탭 진입 문제

- 공통 이벤트 처리 로직은 AlgorithmPanels 쪽 helper로 묶었다.
- `RegisterDiscoveredPanels()`가 이미 등록된 패널을 덮어쓰지 않도록 정리했다.
- `ExistingElementAlgorithmPanel`이 기존 local DataContext를 보존하도록 해서, Align 패널 VM이 다른 알고리즘 탭으로 오염되지 않게 했다.
- ShapeX에서 이진화 조절 시 다른 탭/알고리즘으로 들어가는 현상은 위 흐름과 관련된 버그로 보고 조치했다.

주요 파일:
- `src/WpfInspectionPrototype/WpfInspectionApp/AlgorithmPanels/AlgorithmPanelFactory.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/AlgorithmPanels/ExistingElementAlgorithmPanel.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/AlgorithmPanels/AlgorithmPanelCommonEvents.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/AlgorithmPanels/AlgorithmPanelInteraction.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/AlgorithmPanels/AlgorithmPanelUi.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/AlgorithmPanels/Types/ShapeXAlgorithmPanel.cs`

### 3.3 Light 공통 패널 이식

참조 프로젝트 기준:
- `InspLightType`: TOP=0, MIDDLE=1, BOTTOM=2, USER=3, SIDE_RED=4, SIDE_GREEN=5, SIDE_BLUE=6, SIDE4=7, 3D=8
- Normal Light 채널 최대값은 200이다.
- TOP은 R/G/B/W 사용 가능, 기본 R=100.
- MIDDLE/BOTTOM은 R/B 사용 가능, G/W 비활성.
- SIDE 계열은 RGB 사용 가능, W 비활성.
- User Light는 최대 10개 셀, 각 셀에 position/operator/R/G/B/W 저장.

현재 프로젝트 반영:
- Light UI를 모든 알고리즘 공통 패널로 분리했다.
- 알고리즘 내부 Light 항목은 제거하고 `MainWindow` 오른쪽 공통 패널에서만 제어한다.
- Normal/User Light 모두 MVVM WPF 구조로 구현했다.
- Light 값 저장/검증/enable 판단은 `AlgorithmLightService`가 담당한다.
- User Light의 ADD/DEL, Position, Operator, 채널 enable 상태는 ViewModel에서 처리한다.

주요 파일:
- `src/WpfInspectionPrototype/WpfInspectionApp/Models/AlgorithmLightModels.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/IAlgorithmLightService.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/AlgorithmLightService.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/ViewModels/LightControlViewModel.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Views/LightControlView.xaml`
- `src/WpfInspectionPrototype/WpfInspectionApp/Views/LightControlView.xaml.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Infrastructure/AppServices.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/ViewModels/MainViewModel.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.xaml`

### 3.4 Light Import 파싱

Import 시 참조 프로젝트의 Light 관련 값이 `Common.*` 키로 정규화되도록 보강했다.

공통 저장 키:
- `Common.LightTypeNum`
- `Common.RedValue`
- `Common.GreenValue`
- `Common.BlueValue`
- `Common.WhiteValue`
- `Common.LightCnt`
- `Common.ArrRedValueString`
- `Common.ArrGreenValueString`
- `Common.ArrBlueValueString`
- `Common.ArrWhiteValueString`
- `Common.ArrCalculationString`
- `Common.ArrLightPositionString`

배열 값은 `|` 구분자로 저장한다. Import는 `|`, `,`, `;` 구분 문자열과 XML 배열 컨테이너 형태를 모두 처리한다.

주요 파일:
- `src/WpfInspectionPrototype/WpfInspectionApp/Models/LegacyRawPartImportAdapter.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Models/ImportedAlgorithmNormalizer.cs`

### 3.5 Runtime/Smoke 회귀 보강

- `PartInspectionRuntime.BuildPartParam`에서 대소문자만 다른 중복 키가 들어와도 Dictionary duplicate key exception이 나지 않도록 `AlgorithmParameterStore.Set` 기반 병합으로 정리했다.
- 참조 알고리즘 lookup도 중복 이름을 안전하게 접도록 보강했다.
- 공통 Light 값은 Align flow request를 통해 native bridge까지 전달되도록 연결했다.
- `MptiBridgeSetAlgoLight` native export를 추가했고, x64 Release native build/export 확인을 마쳤다.
- Smoke test에 다음 회귀 검사를 추가했다.
  - `PartInspectionRuntime` 중복 키 병합
  - `AlgorithmLightService` User Light round-trip
  - Legacy import Light scalar/array 파싱

주요 파일:
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/PartInspectionRuntime.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/AlignFlowRequestFactory.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/InspectionFlowService.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/MptiFlowNativeBridge.cs`
- `src/WpfInspectionPrototype/native/MptiBridge/MptiBridgeFlow.h`
- `src/WpfInspectionPrototype/native/MptiBridge/MptiBridgeFlow.cpp`
- `src/WpfInspectionPrototype/WpfInspectionApp/Diagnostics/SmokeTestRunner.cs`

### 3.6 PTT 내부 로딩 + Light 2D Preview

참조 프로젝트 기준:
- `ucLightControl_Normal.LightLayerChange()`는 PTT/raw 채널을 선택한 뒤 `MPTI_FULLImgCompose`로 2D 합성 이미지를 만든다.
- `ucLightControl_User.LightLayerChange()`는 User Light 셀 정보를 `LightTypeBuf`로 구성한 뒤 `MPTI_FullImageClaculCompose`로 2D 합성 이미지를 만든다.
- PTT 채널은 `MPTI_GetPttFileLoad()`로 내부 버퍼에 읽힌 뒤 `MPTI_GetPttFileChannel()`에서 가져온다.
- 참조 채널 인덱스는 0=TR, 1=TG, 2=TB, 3=BR, 4=BB, 5=MR, 6=MB, 7=TW 흐름으로 확인했다.

현재 프로젝트 반영:
- PTT 로드는 3D Viewer를 띄우지 않고, 내부 MPTI bridge reading만 수행한다.
- PEM3D Viewer host service와 wrapper reference는 제거했다. 실행 폴더에도 PEM3D OCX/manifest를 복사하지 않는다.
- `Load PTT Data` 또는 Part Import 후 PTT 경로가 있으면 `PttViewerWorkflowService`가 `MPTI_GetPttFileLoad` 경로를 준비한다.
- Light Panel 값을 움직이면 `LightControlViewModel` 변경 콜백이 `MainViewModel`로 전달되고, `PttLightPreviewService`가 native preview 합성을 요청한다.
- Normal Light는 native bridge에서 `MPTI_FULLImgCompose`를 호출한다.
- User Light는 native bridge에서 `MPTI_FullImageClaculCompose`를 호출한다.
- 합성된 Gray8 preview는 WPF `LoadedImageFrame`으로 변환되어 기존 2D source image/threshold/overlay 경로에 들어간다.
- View와 code-behind에는 Light 값 계산, 채널 선택, preview 합성 로직을 두지 않았다.

주요 파일:
- `src/WpfInspectionPrototype/MptiBridge/MptiBridge.cpp`
- `src/WpfInspectionPrototype/WpfInspectionApp/Interop/MptiNativeBridge.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/IPttLightPreviewService.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/PttLightPreviewService.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/IPttViewerWorkflowService.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/PttViewerWorkflowService.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/IImageRuntimeStateService.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/ImageRuntimeStateService.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/ViewModels/MainViewModel.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.ModelLifecycle.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj`

## 4. MVVM 점검 결과

마지막 점검 기준:
- `LightControlView.xaml.cs`는 생성자만 있다.
- Light 관련 값 저장/검증/enable 판단은 ViewModel/Service에 있다.
- `LightControlView.xaml`에는 `Click=`, `SelectionChanged=`, `ValueChanged=`, `TextChanged=`, `MouseUp=` 같은 직접 이벤트 핸들러가 없다.
- `MainWindow.xaml.cs`와 `Views/*.xaml.cs`에서 Light 파라미터 키를 직접 다루는 흔적은 없다.
- 알고리즘 내부 UI에서 기존 Light 항목(`Light Type`, `Red Light`, `ROI Light`, `Light Data Count` 등) 검색 결과 잔여 항목 없음.

재점검 명령:

```powershell
rg -n "Common\.Light|Common\.RedValue|Common\.GreenValue|Common\.BlueValue|Common\.WhiteValue|LightCnt|ArrRedValueString|ArrGreenValueString|ArrBlueValueString|ArrWhiteValueString|ArrCalculationString|ArrLightPositionString|LightTypeNum" src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.xaml.cs src/WpfInspectionPrototype/WpfInspectionApp/Views
rg -n "Click=|SelectionChanged=|ValueChanged=|TextChanged=|MouseUp=|PreviewMouse" src/WpfInspectionPrototype/WpfInspectionApp/Views/LightControlView.xaml src/WpfInspectionPrototype/WpfInspectionApp/Views/LightControlView.xaml.cs
rg -n "Light Type|Red Light|Green Light|Blue Light|White Light|Select Light|Add Light|Sub Light|Light Data Count|ROI Light|Mid Light" src/WpfInspectionPrototype/WpfInspectionApp/Models src/WpfInspectionPrototype/WpfInspectionApp/AlgorithmPanels src/WpfInspectionPrototype/WpfInspectionApp/Views src/WpfInspectionPrototype/WpfInspectionApp/MainWindow.xaml
rg -n "EnsurePttViewerPanel|_pem3DViewerHostService|_pttViewerHost|_pttViewerPanel|LoadIntoControl\(|Loaded 3D PTT|Load 3D PTT|3D SOURCE|3D PTT" src/WpfInspectionPrototype/WpfInspectionApp -g "*.cs" -g "*.xaml"
```

위 명령들은 마지막 확인 시 Light 관련 MVVM 위반 흔적을 찾지 못했다.

## 5. 검증 결과

마지막 실행:

```powershell
dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
& 'src/WpfInspectionPrototype/WpfInspectionApp/bin/x64/Debug/net48/WpfInspectionApp.exe' --smoke-test
```

결과:
- Build 성공
- Warning 0
- Error 0
- Smoke test 성공
- Smoke test에서 duplicate key/Light service/Legacy Light import 회귀 검사 통과
- Native bridge x64 Debug build 성공
- `dumpbin /exports` 기준 `MptiBridgeSetAlgoLight`, `MptiBridgeRenderLightPreview` export 확인
- 3D Viewer UI/service 호출 경로 제거

## 6. 아직 남은 위험/다음 확인

- 이전에 보고된 `PartInspectionRuntime.BuildPartParam` duplicate key exception은 코드 수정 및 synthetic smoke 회귀 검사까지 완료했다. 실제 문제 Part 파일로 한 번 더 확인하면 더 안전하다.
- Light UI/import/runtime/native bridge 경로는 build/smoke/export 기준 통과했다. 실제 화면에서 Import 후 알고리즘별 Light 값이 기대대로 표시되는지, 실제 검사에서 Light 변경이 결과에 반영되는지는 샘플 Part/장비 환경으로 수동 확인이 필요하다.
- PTT Light preview는 참조 프로젝트와 같은 native compose 함수를 호출하도록 연결했다. 실제 PTT 파일별 채널 매핑/밝기 결과는 샘플 PTT로 화면 비교가 필요하다.
- `git status`상 변경 파일과 미추적 파일이 많다. 다음 담당자는 파일 단위로 diff를 확인하고, 관련 없는 변경을 되돌리지 말아야 한다.
- 참조 프로젝트의 장비별 특수 분기(UV machine, SCM_LFI label/image 교체 등)는 현재 WPF 패널에는 일반 규칙만 반영했다. 실제 장비 옵션 연동이 필요하면 Service에 옵션 모델을 추가해야 한다.

## 7. 다음 작업 추천 순서

1. 실제 샘플 Part Import로 `Common.LightTypeNum`, RGBW, User Light 배열이 참조 프로젝트와 같은 값으로 들어오는지 화면/데이터 양쪽에서 확인한다.
2. 실제 장비 또는 native 검사 환경에서 Light 변경이 알고리즘 결과에 반영되는지 확인한다.
3. 실제 PTT 파일을 로드한 뒤 Normal/User Light 조작 시 2D 이미지가 참조 프로젝트와 같은 방식으로 바뀌는지 비교한다.
4. 화면에서 Normal/User Light 조작 시 threshold preview가 과도하게 재진입하지 않는지 확인한다.
5. 샘플 Part/PTT가 확보되면 smoke test에 실제 import/preview fixture를 추가한다.
6. 새 알고리즘 UI 전개 전, 위 MVVM 점검 명령을 먼저 돌린다.
