# 2026-05-22 Handoff - 새 Flow Algorithm 타입 5종 fallback-path 회귀 추가

## 배경

`docs/2026-05-21-handoff-exact-flow-algorithm-types.md` 의 "다음 추천 작업 #1" 에서
다음 6개 알고리즘 타입에 대해 `Runtime.FlowBridge=native-flow` 가 정상 기록되는지
실제 PTT 샘플로 검증 권고가 있었다:

- `AlgoBody_Blob`
- `AlgoNGBlob`
- `AlgoBodyEdge`
- `AlgoPatternDiff`
- `AlgoOCR`
- `AlgoPOCR`

이 중 `AlgoBody_Blob` 는 기존 `VerifyPartRuntimeRoutesBlobBridge` 가 이미 커버.
나머지 5종은 smoke test 에 없어, 매핑이 깨져도 CI 에서 잡히지 않는 상태였다.

flow-first path (`Runtime.FlowBridge=native-flow`) 자체는 real PTT 파일이 필요해
smoke test 로 자동화 불가 → 사용자 GUI 검증에 위임. 본 작업은 **fallback path**
(per-algo C++ bridge: Blob/Edge/Pattern) 가 새 타입에서도 망가지지 않음을 회귀로
잠갔다.

## 추가된 5개 smoke test

`SmokeTestRunner.cs` 에 다음 함수 추가:

| 함수 | 알고리즘 타입 | 기대 bridge family |
|---|---|---|
| `VerifyPartRuntimeRoutesNgBlobToBlobBridge` | `AlgoNGBlob` | Blob |
| `VerifyPartRuntimeRoutesBodyEdgeToEdgeBridge` | `AlgoBodyEdge` | Edge |
| `VerifyPartRuntimeRoutesPatternDiffToPatternBridge` | `AlgoPatternDiff` | Pattern |
| `VerifyPartRuntimeRoutesOcrToPatternBridge` | `AlgoOCR` | Pattern |
| `VerifyPartRuntimeRoutesPocrToPatternBridge` | `AlgoPOCR` | Pattern |

공통 헬퍼 `VerifyAlgorithmRoutesToBridgeFamily(...)` 추가하여 동일한 검증 로직 재사용:
- synthetic 256x256 이미지 + 알고리즘 1개 모델 생성
- `PartInspectionRuntime.Run(model, image)` 호출
- 검증 항목:
  - `NativeBridgeName == bridgeName` (Blob/Edge/Pattern)
  - `NativeBridgeMode == "native"`
  - `Runtime.<Family>Bridge == "native"`
  - `Runtime.<Family>IsOK == "True"`
  - `Runtime.NativeBridgeName == bridgeName`

위 5개를 `Run()` 진입점의 기존 4개 routes 검증 뒤에 추가:

```csharp
VerifyPartRuntimeRoutesBlobBridge();
VerifyPartRuntimeRoutesBGABridge();
VerifyPartRuntimeRoutesEdgeBridge();
VerifyPartRuntimeRoutesPatternBridge();
// 신규 5개
VerifyPartRuntimeRoutesNgBlobToBlobBridge();
VerifyPartRuntimeRoutesBodyEdgeToEdgeBridge();
VerifyPartRuntimeRoutesPatternDiffToPatternBridge();
VerifyPartRuntimeRoutesOcrToPatternBridge();
VerifyPartRuntimeRoutesPocrToPatternBridge();
```

## 변경된 파일

- `src/WpfInspectionPrototype/WpfInspectionApp/Diagnostics/SmokeTestRunner.cs`

(매핑 자체는 이전 세션 `RuntimeFlowAlgorithmAdapter.cs` 와 `AlgorithmNativeBridgeAdapter.cs`
변경에서 이미 추가됨. 본 세션은 회귀 잠금만.)

## 검증

```
dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0, 오류 0, 6.77초

src/WpfInspectionPrototype/WpfInspectionApp/bin/x64/Debug/net48/WpfInspectionApp.exe --smoke-test
→ EXIT=0
```

5개 신규 테스트가 모두 통과해 매핑이 의도대로 들어가 있음을 확인.

## 한계 / 다음 작업

- **GUI 검증 (real PTT 샘플)** — flow-first path 가 실제로 `Runtime.FlowBridge=native-flow`
  를 기록하는지는 이번 자동화로 못 잡음. `Models/1.zip` / `Models/2.zip` 같은 실 샘플로
  import 후 RunInspection → `algorithm.Parameters["Runtime.FlowBridge"]` 값 확인 필요.
  실패 케이스가 발견되면 `Runtime.FlowBridgeMessage`, `Runtime.FlowBridgeReaderCode` 와
  native `InspProc` 메시지로 디버그.
- **OCR/POCR 전용 파라미터** — 현재는 Pattern 베이스를 그대로 재사용. 참조 프로젝트
  `pemtoFramework` 의 RawData 에 `OCRString`/`OCRModelPath`/`OCRFontAngle` 등 추가
  필드가 있고 import normalizer 가 아직 매핑 안 함. 실 샘플 확보 후 보강 필요
  (handoff `2026-05-21-handoff-exact-flow-algorithm-types.md` §"다음 추천 작업 #3").
- **fallback 경로의 native 결과** — 본 smoke 는 native bridge 가 성공 ("True") 한다는
  전제 하에 검증. 실제 native 가 실패하는 경우 (예: 알고리즘 파라미터 부족) 는
  fallback 메시지로 떨어지므로 별도 negative test 추가 가능.

## 빌드 / 재현 명령

```powershell
dotnet build src\WpfInspectionPrototype\WpfInspectionApp\WpfInspectionApp.csproj `
  -p:Configuration=Debug -p:Platform=x64

& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe --smoke-test
```
