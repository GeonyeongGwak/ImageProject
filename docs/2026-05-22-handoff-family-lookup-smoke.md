# 2026-05-22 Handoff - MptiBridgeGetAlgoFamily 회귀 잠금

## 배경

직전 두 작업 (`native-family-dispatch`, `bridge-adapter-family-driven`) 으로 native
`MptiBridgeGetAlgoFamily` 가 `RuntimeFlowAlgorithmAdapter` + `AlgorithmNativeBridgeAdapter`
양쪽의 dispatch 진입점이 됐다. 본 함수의 매핑이 깨지면 두 어댑터가 모두 잘못된 라우팅
하므로 회귀 잠금이 필요하다.

## 추가된 smoke test — `VerifyAlgoFamilyLookupMatchesCatalog`

`SmokeTestRunner.cs` 의 `RunServiceRegressionChecks` 에 추가. 두 가지를 동시 검증:

1. **`AlgorithmCatalog.NativeAlgoType` 값** — 19 개 algorithm type 의 `Find(type).NativeAlgoType`
   이 기대된 native enum 값과 일치.
2. **`MptiBridgeGetAlgoFamily(catalog.NativeAlgoType)` 반환** — 같은 algorithm 의 family 가
   기대된 enum 으로 떨어짐.

### 커버하는 19개 algorithm + 8 family

| Family | Algorithms |
|---|---|
| `FAMILY_ALIGN` | AlgoAlign |
| `FAMILY_BLOB` | AlgoBlob, AlgoBody_Blob, AlgoNGBlob, AlgoBump |
| `FAMILY_EDGE` | AlgoEdge, AlgoBodyEdge, AlgoLine, AlgoDistance, AlgoEdgePoint |
| `FAMILY_PATTERN` | AlgoPattern, AlgoPatternDiff, AlgoOCR, AlgoPOCR, AlgoForeignOCV |
| `FAMILY_BGA` | AlgoBGA, AlgoLQBGA |
| `FAMILY_PADBW` | AlgoPadBW |
| `FAMILY_SHAPEX` | AlgoShapeX |
| `FAMILY_BW` | (eAlgoBW = 0 native 호출 직접 검증) |

### 추가로 검증하는 edge case

- `MptiBridgeGetAlgoFamily(0)` (== `eAlgoBW`) == `FAMILY_BW` — native 가 enum 0 을 BW
  family 로 처리하는지.
- `MptiBridgeGetAlgoFamily(-1)` == `FAMILY_UNKNOWN` — 음수 입력 안전 처리.
- `MptiBridgeGetAlgoFamily(99999)` == `FAMILY_UNKNOWN` — 범위 밖 입력 안전 처리.
- `AlgorithmCatalog.Find("AlgoTilt").NativeAlgoType == Unknown` — flow path 미연결 알고리즘이
  의도대로 Unknown 으로 떨어짐 (IsFamily helper 의 early-return 보호 작동).

### 검증되는 회귀 시나리오

이 테스트가 잡아낼 수 있는 깨짐:
- native `MptiBridgeGetAlgoFamily` switch 에 case 누락 또는 잘못된 family 반환
- `NativeAlgoTypeIds.cs` 의 enum 값 오타 (예: Bump=39 로 잘못 지정)
- `AlgorithmCatalog` entry 의 `nativeAlgoType` 인수 누락 또는 잘못된 상수 사용
- C# `FAMILY_*` 상수가 native `MPTI_FAMILY_*` 와 불일치
- 신규 algorithm 추가 시 cross-source 동기화 누락

## 부수 변경

`SmokeTestRunner.cs` 에 `using WpfInspectionApp.Interop;` 추가 — `MptiFlowNativeBridge` 접근용.

## 변경된 파일

- `src/WpfInspectionPrototype/WpfInspectionApp/Diagnostics/SmokeTestRunner.cs`
  - using 추가
  - `VerifyAlgoFamilyLookupMatchesCatalog()` 메서드 추가
  - `RunServiceRegressionChecks` 에서 호출

## 검증

```
dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0, 오류 0, 7.76초

src/WpfInspectionPrototype/WpfInspectionApp/bin/x64/Debug/net48/WpfInspectionApp.exe --smoke-test
→ EXIT=0 (19 algorithm + 3 edge case 모두 통과)
```

native DLL 은 재빌드 없이 기존 `bin/Debug/x64/MptiBridge.dll` 사용 — 직전 family
expansion 변경이 이미 반영된 상태.

## 다음 작업 후보 (직전 handoff §"다음 작업 후보" 잔여)

1. ~~AlgorithmCatalog 통합~~ ✅
2. ~~AlgorithmNativeBridgeAdapter family-driven~~ ✅
3. ~~MptiBridgeGetAlgoFamily smoke 회귀~~ ✅ (본 작업)
4. **새 family (Height / Volume) 추가** — AlgoHeight_Mean, AlgoHeight_Diff, AlgoVolume,
   AlgoBridge, AlgoLead_* 등 현재 Unknown 알고리즘 flow path 지원
5. **신규 6 항목 (Bump/Line/Distance/EdgePoint/ForeignOCV/LQBGA) 의 native
   `MptiBridgeAddAlgo` case 추가** — flow path 실제 작동 (현재는 native 가 param=nullptr
   로 안전 fail, family lookup 만 유효).

## 빌드 / 재현 명령

```powershell
dotnet build src\WpfInspectionPrototype\WpfInspectionApp\WpfInspectionApp.csproj `
  -p:Configuration=Debug -p:Platform=x64

& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe --smoke-test
```
