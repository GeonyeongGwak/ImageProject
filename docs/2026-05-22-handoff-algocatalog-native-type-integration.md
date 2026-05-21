# 2026-05-22 Handoff - AlgorithmCatalog 에 native algoType 통합

## 배경

직전 작업 (`2026-05-22-handoff-native-family-dispatch.md`) 에서 `RuntimeFlowAlgorithmAdapter`
가 family-driven dispatch 로 단순화됐지만, `algorithm.Type` (string) → native `algoType` (int)
매핑이 `RuntimeFlowAlgorithmAdapter.cs` 안의 private static dict 에 13 entry 로 분산되어
있었다. 카탈로그/native/매핑 dict 3 군데 동기화 필요 → single source of truth 부재.

## 변경 — 카탈로그가 native algoType source of truth

### 1) `Models/NativeAlgoTypeIds.cs` 신규

```csharp
public static class NativeAlgoTypeIds
{
    public const int Unknown      = 0;
    public const int Blob         = 1;
    public const int Align        = 2;
    public const int BodyBlob     = 3;
    public const int Ocr          = 5;
    public const int Pattern      = 6;
    public const int Edge         = 26;
    public const int Pocr         = 31;
    public const int Bga          = 37;
    public const int NgBlob       = 39;
    public const int PadBw        = 40;
    public const int BodyEdge     = 42;
    public const int PatternDiff  = 47;
    public const int ShapeX       = 48;
}
```

- Models 레이어에 위치 → Models, Interop, Services 모두 의존 안전.
- 값은 native `NativeSources/MPTILib_Algo/PInsp_Algo/InspParamDef_Algo.h` 의 `eAlgo*` 와 동일.

### 2) `Models/AlgorithmCatalog.cs` 확장

`AlgorithmCatalogItem` record 에 `NativeAlgoType` 필드 추가 (기본값 `Unknown=0`):

```csharp
public sealed record AlgorithmCatalogItem(
    string Type,
    string DisplayName,
    LegacyAlgorithmGroup Group,
    int LegacyFlag,
    string LegacyName,
    string ParameterFamily,
    int NativeAlgoType = NativeAlgoTypeIds.Unknown);
```

flow path 연결된 13개 entry 에 `NativeAlgoTypeIds.*` 인수 채움:

| Catalog entry | NativeAlgoType |
|---|---|
| AlgoAlign | Align (2) |
| AlgoBlob | Blob (1) |
| AlgoBody_Blob | BodyBlob (3) |
| AlgoNGBlob | NgBlob (39) |
| AlgoEdge | Edge (26) |
| AlgoBodyEdge | BodyEdge (42) |
| AlgoPattern | Pattern (6) |
| AlgoPatternDiff | PatternDiff (47) |
| AlgoOCR | Ocr (5) |
| AlgoPOCR | Pocr (31) |
| AlgoBGA | Bga (37) |
| AlgoPadBW | PadBw (40) |
| AlgoShapeX | ShapeX (48) |

`Normal()` / `Extended()` helper 도 optional `nativeAlgoType` 인수 받도록 확장.

### 3) `Interop/MptiFlowNativeBridge.cs` 의 `EALGO_*` → alias

```csharp
public const int EALGO_BLOB = WpfInspectionApp.Models.NativeAlgoTypeIds.Blob;
public const int EALGO_ALIGN = WpfInspectionApp.Models.NativeAlgoTypeIds.Align;
// ... 13 개 alias
```

- 기존 P/Invoke 호출부 / `BlobFlowAlgorithm` 등의 `MptiFlowNativeBridge.EALGO_*` 참조 그대로 유지.
- 값 변경 시 `NativeAlgoTypeIds` 한 곳만 수정.

### 4) `Services/FlowAlgorithms/RuntimeFlowAlgorithmAdapter.cs` 단순화

기존 `AlgoTypeMap` private static dict (13 entry) 제거.

```csharp
var catalog = AlgorithmCatalog.Find(algorithm.Type);
if (catalog.NativeAlgoType == NativeAlgoTypeIds.Unknown)
{
    return false;  // flow path 미연결 → 호출자가 per-algo bridge fallback
}

var algoType = catalog.NativeAlgoType;
var family = MptiFlowNativeBridge.MptiBridgeGetAlgoFamily(algoType);
var displayName = string.IsNullOrWhiteSpace(catalog.DisplayName) ? algorithm.Type : catalog.DisplayName;
var inspType = ResolveInspType(algoType);
```

`ResolveInspType` 도 `MptiFlowNativeBridge.EALGO_*` 대신 `NativeAlgoTypeIds.*` 사용
(가독성, layer 깔끔).

## 이득

- ✅ **카탈로그가 single source of truth** — algorithm 의 native algoType 을 알고 싶으면
  `AlgorithmCatalog.Find(type).NativeAlgoType` 한 곳만 보면 됨.
- ✅ **`RuntimeFlowAlgorithmAdapter` 의 정적 매핑 제거** — 새 algorithm 추가 시 더 이상
  adapter 수정 필요 없음.
- ✅ **새 algorithm flow 연결 워크플로**:
  1. native `InspParamDef_Algo.h` 의 `eAlgo*` 값 확인
  2. `NativeAlgoTypeIds` 에 상수 추가
  3. `AlgorithmCatalog` 의 해당 entry 에 `NativeAlgoTypeIds.X` 인수 추가
  4. native `MptiBridgeAddAlgo` / `MptiBridgeGetAlgoFamily` switch 확장
  → 끝.
- ✅ **회귀 0** — `MptiFlowNativeBridge.EALGO_*` API 표면 유지 (alias 로 같은 값).
- ✅ **flow path 미지원 알고리즘은 명시적** — 카탈로그에서 `NativeAlgoType` 가 `Unknown` 이면
  자동으로 fallback. AlgoBW / AlgoTilt / AlgoGray_Mean / Lead 계열 등이 명확히 표시됨.

## 변경된 파일

- `src/WpfInspectionPrototype/WpfInspectionApp/Models/NativeAlgoTypeIds.cs` (신규)
- `src/WpfInspectionPrototype/WpfInspectionApp/Models/AlgorithmCatalog.cs` (record + 13 entry 확장)
- `src/WpfInspectionPrototype/WpfInspectionApp/Interop/MptiFlowNativeBridge.cs` (EALGO_* alias 화)
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/FlowAlgorithms/RuntimeFlowAlgorithmAdapter.cs` (AlgoTypeMap dict 제거)

## 검증

```
dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0, 오류 0, 5.42초

src/WpfInspectionPrototype/WpfInspectionApp/bin/x64/Debug/net48/WpfInspectionApp.exe --smoke-test
→ EXIT=0 (smoke test 모두 통과)
```

## 다음 작업 후보 (직전 handoff §"다음 작업 후보" 잔여)

1. ~~AlgorithmCatalog 에 native algoType 통합~~ (✅ 본 작업)
2. **`AlgorithmNativeBridgeAdapter` fallback 도 family-driven 으로** — `RuntimeFlowAlgorithmAdapter`
   와 동일 패턴 적용. catalog.NativeAlgoType + MptiBridgeGetAlgoFamily lookup 으로 redirect.
3. **`MptiBridgeGetAlgoFamily` smoke 회귀** — 각 family 의 대표 algorithm 의 family 값 회귀 잠금.
4. **새 family (Height / Volume / Line / Distance 계열)** — 현재 `Unknown` 으로 떨어지는
   알고리즘들 native flow 지원 시 catalog 한 줄 + native switch 한 줄로 자동 연결.

## 빌드 / 재현 명령

```powershell
dotnet build src\WpfInspectionPrototype\WpfInspectionApp\WpfInspectionApp.csproj `
  -p:Configuration=Debug -p:Platform=x64

& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe --smoke-test
```
