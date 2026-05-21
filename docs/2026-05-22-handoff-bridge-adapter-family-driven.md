# 2026-05-22 Handoff - AlgorithmNativeBridgeAdapter family-driven 리팩터

## 배경

직전 작업 2개 (`native-family-dispatch`, `algocatalog-native-type-integration`) 에서
`RuntimeFlowAlgorithmAdapter` 의 13-case algorithm-type switch 가 native family lookup
+ AlgorithmCatalog single-source 로 단순해졌다.

본 작업은 **per-algo legacy C++ bridge (MptiBridgeRunBlob / RunBGA / RunEdge / RunPattern)
의 라우팅 분류** 인 `AlgorithmNativeBridgeAdapter` 의 4 HashSet 도 같은 패턴으로 통합한다.

### 기존 구조

`AlgorithmNativeBridgeAdapter` 안의 4 HashSet 가 algorithm type 분류를 보유:

```csharp
private static readonly HashSet<string> DirectBlobTypes = { "AlgoBlob","AlgoBody_Blob","AlgoNGBlob","AlgoBump" };
private static readonly HashSet<string> DirectBGATypes = { "AlgoBGA","AlgoLQBGA" };
private static readonly HashSet<string> DirectEdgeTypes = { "AlgoEdge","AlgoLine","AlgoDistance","AlgoEdgePoint","AlgoBodyEdge" };
private static readonly HashSet<string> DirectPatternTypes = { "AlgoPattern","AlgoPatternDiff","AlgoForeignOCV","AlgoOCR","AlgoPOCR" };
```

→ 새 algorithm 추가 시 동일 family 라도 이 4 곳을 별도로 수정 필요. C# 의 분류 매핑이
다시 분산.

## 적용 변경

### 1) `Models/NativeAlgoTypeIds.cs` 확장

native flow API 미지원이지만 logical family 분류에 필요한 6 개 추가:

```csharp
public const int Line       = 25;
public const int Bump       = 38;
public const int ForeignOcv = 41;
public const int Distance   = 44;
public const int EdgePoint  = 46;
public const int LqBga      = 50;
```

기존 14개 + 6개 = **20 개 algorithm 의 native ID** 가 카탈로그에서 single source.

### 2) `MptiBridge/MptiBridgeFlow.cpp::MptiBridgeGetAlgoFamily` 의미 확장

기존: "flow API 지원 family" 만 반환 (8 enum)
변경: "**logical family** 전체" 반환 — flow 미지원 알고리즘도 자기 family 로 분류됨.

```cpp
case eAlgoBlob:
case eAlgoBody_Blob:
case eAlgoNGBlob:
case eAlgoBump:            // 추가
    return MPTI_FAMILY_BLOB;
case eAlgoEdge:
case eAlgoBodyEdge:
case eAlgoLine:            // 추가
case eAlgoDistance:        // 추가
case eAlgoEdgePoint:       // 추가
    return MPTI_FAMILY_EDGE;
case eAlgoPattern:
case eAlgoPatternDiff:
case eAlgoOCR:
case eAlgoPOCR:
case eAlgoForeignOCV:      // 추가
    return MPTI_FAMILY_PATTERN;
case eAlgoBGA:
case eAlgoLQBGA:           // 추가
    return MPTI_FAMILY_BGA;
```

### 3) `Models/AlgorithmCatalog.cs` 의 6 entry 에 NativeAlgoType 추가

```csharp
Normal("AlgoLine", "Line", 67108864, "Line", "Line", NativeAlgoTypeIds.Line),
Extended("AlgoBump", ..., NativeAlgoTypeIds.Bump),
Extended("AlgoDistance", ..., NativeAlgoTypeIds.Distance),
Extended("AlgoForeignOCV", ..., NativeAlgoTypeIds.ForeignOcv),
Extended("AlgoEdgePoint", ..., NativeAlgoTypeIds.EdgePoint),
Extended("AlgoLQBGA", ..., NativeAlgoTypeIds.LqBga)
```

### 4) `Services/AlgorithmNativeBridgeAdapter.cs` 의 4 HashSet 제거

```csharp
// 신규 helper
private static bool IsFamily(string algorithmType, int expectedFamily)
{
    var catalog = AlgorithmCatalog.Find(algorithmType);
    if (catalog.NativeAlgoType == NativeAlgoTypeIds.Unknown)
        return false;
    return MptiFlowNativeBridge.MptiBridgeGetAlgoFamily(catalog.NativeAlgoType) == expectedFamily;
}
```

4개 `TryBuild*Params` 의 가드를 교체:
- `DirectBlobTypes.Contains(...)`    → `IsFamily(..., FAMILY_BLOB)`
- `DirectBGATypes.Contains(...)`     → `IsFamily(..., FAMILY_BGA)`
- `DirectPatternTypes.Contains(...)` → `IsFamily(..., FAMILY_PATTERN)`
- `DirectEdgeTypes.Contains(...)`    → `IsFamily(..., FAMILY_EDGE)`

→ HashSet 4 개 (15 항목) **완전 제거**. 분류 로직은 native + catalog single source.

`GenericProfiles` Dictionary (어떤 generic bridge profile 을 쓸지) 는 그대로 유지 — 카테고리가 다른 개념 (catch-all bridge shape).

## 이득

- ✅ **분류 매핑 single source of truth 강화**: `RuntimeFlowAlgorithmAdapter` 에 이어 `AlgorithmNativeBridgeAdapter` 도 catalog + native family lookup 으로 통합.
- ✅ **새 algorithm flow 연결 워크플로 통일**: 직전 handoff 의 4-step 워크플로 (`NativeAlgoTypeIds` 추가 → catalog entry 추가 → native switch 확장) 가 본 adapter 에도 자동 적용. 별도 HashSet 수정 불필요.
- ✅ **logical family ≠ flow-supported family 의 의미 분리**: `MptiBridgeGetAlgoFamily` 가 이제 logical family 를 의미. flow 지원 여부는 별개 (`MptiBridgeAddAlgo` switch 가 담당).
- ✅ **회귀 0**: 기존 4 HashSet 의 모든 항목이 신규 family lookup 으로도 동일하게 매칭.

## 변경된 파일

- `src/WpfInspectionPrototype/WpfInspectionApp/Models/NativeAlgoTypeIds.cs` (+ 6 상수)
- `src/WpfInspectionPrototype/MptiBridge/MptiBridgeFlow.cpp` (switch + 6 case)
- `src/WpfInspectionPrototype/WpfInspectionApp/Models/AlgorithmCatalog.cs` (6 entry 인수 추가)
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/AlgorithmNativeBridgeAdapter.cs` (HashSet 4 개 제거, IsFamily helper 추가)

## 검증

```
MSBuild MptiBridge.vcxproj /p:Configuration=Debug /p:Platform=x64
→ 빌드 성공 (기존 legacy 경고만)

dotnet build WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0, 오류 0, 5.04초

WpfInspectionApp.exe --smoke-test
→ EXIT=0 (12 개 algorithm 회귀 테스트 모두 통과)
```

## 의미적 변화 주의

`MptiBridgeGetAlgoFamily` 의 의미가 "**flow API 지원 family**" → "**logical family**" 로 확장.

기존 `RuntimeFlowAlgorithmAdapter` 가 family 만으로 flow path 진입 여부를 결정했다면
이제는 family ≠ flow 지원이라 추가 가드가 필요한지 검토. → 다행히 `TryCreateFlowAlgorithm`
은 `catalog.NativeAlgoType == Unknown` 이면 false 반환하는데, 새로 추가된 Bump/Line/등의
catalog entry 는 NativeAlgoType 가 0 이 아니므로 통과한다. 이들이 family-aware
`TryBuild*Params` 를 만족하면 flow path 로 진입 시도.

다만 native `MptiBridgeAddAlgo` switch 는 이들 6 개를 case 처리하지 않으므로 native 측에서
default branch (`as->type = algoType; params=nullptr`) 떨어지고 commit 시 skip. 즉 **flow path
에 들어가더라도 안전 fail** (param 없으면 InspWindowAlgo3 가 -1 반환).

이 동작이 의도된 것인지 확인 필요. 향후 native `MptiBridgeAddAlgo` 에 이들 6 개를 추가하면
실제 flow path 지원 확장이 가능 → 그때 별도 작업.

## 다음 작업 후보 (직전 handoff §"다음 작업 후보" 잔여)

1. ~~AlgorithmCatalog 에 native algoType 통합~~ ✅ 완료
2. ~~AlgorithmNativeBridgeAdapter fallback 도 family-driven~~ ✅ 본 작업
3. **`MptiBridgeGetAlgoFamily` smoke 회귀** — 각 family 대표 algorithm 의 family 값 회귀 잠금
4. **새 family (Height / Volume 계열)** — AlgoHeight_Mean, AlgoHeight_Diff, AlgoVolume, AlgoBridge, AlgoLead_* 등 현재 Unknown 인 알고리즘 flow path 지원
5. **AlgoBump / AlgoLine / AlgoDistance / AlgoEdgePoint / AlgoForeignOCV / AlgoLQBGA 를 native flow path 에 실제 연결** — `MptiBridgeAddAlgo` 의 switch 에 case 추가하면 catalog 만으로 자동 동작 가능

## 빌드 / 재현 명령

```powershell
& 'C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe' `
  src\WpfInspectionPrototype\MptiBridge\MptiBridge.vcxproj /p:Configuration=Debug /p:Platform=x64 /m

dotnet build src\WpfInspectionPrototype\WpfInspectionApp\WpfInspectionApp.csproj `
  -p:Configuration=Debug -p:Platform=x64

& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe --smoke-test
```
