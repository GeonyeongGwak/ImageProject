# 2026-05-22 Handoff - Native-driven Algo Family Dispatch (Level 1 of IPINSP_ALGO 활용)

## 배경

사용자 질문: "InspAlgorithm_Dll, InspAlgorithm 추상화로 되어있잖아 이걸 잘 사용해서 엮을 순 없을까?"

### 현재 상태 분석

Native 측 (`NativeSources/MPTILib_Algo/PInsp_Algo/IPInsp_Algo.h`):
- `IPINSP_ALGO` 가 추상 base 로 모든 알고리즘의 공통 인터페이스:
  - `virtual void InitAlgo() abstract`
  - `virtual int AlgoJudgment(...) abstract`
  - `virtual BOOL InspAlgorithm(...) abstract`
  - `virtual BOOL InspAlgorithm_Dll(...) abstract`
  - `virtual BOOL SetAlignRes(...) abstract`
  - `virtual int UseColorImage(...) abstract`
- `CPInsp_AlgoAlign`, `CPInsp_AlgoBlob`, `CPInsp_AlgoBGA`, `CPInsp_AlgoOCR`, ... 모두 `IPINSP_ALGO` 상속.
- **InspManager 가 이미 polymorphic dispatch 사용** (`pAlgo->InspAlgorithm(...)`). Inspection 실행 path 는 새 algorithm type 추가해도 자동 동작.

C# 측 (`Services/FlowAlgorithms/RuntimeFlowAlgorithmAdapter.cs`):
- 13-case `switch (algorithm.Type)` 로 매 algorithm type 마다 적절한 FlowAlgorithm + Parameters 생성.
- 동일한 매핑이 `MptiBridgeAddAlgo` (native), `AlgorithmNativeBridgeAdapter` (fallback) 에도 분산.
- 새 algorithm 추가하려면 **4 곳을 동시 수정** 해야 함.

### 진짜 이슈

`IPINSP_ALGO` polymorphism 은 native 내부 inspection 실행 path 에 이미 활용되지만,
C# 의 **family 매핑이 source of truth 없이 분산** 되어 있어 추가 시 일관성 위험.

## 적용한 변경 (Level 1)

### Native 측 — family lookup 노출

`src/WpfInspectionPrototype/MptiBridge/MptiBridgeFlow.cpp`:

```cpp
enum MptiBridgeAlgoFamily {
    MPTI_FAMILY_UNKNOWN = 0,
    MPTI_FAMILY_ALIGN   = 1,
    MPTI_FAMILY_BLOB    = 2,
    MPTI_FAMILY_EDGE    = 3,
    MPTI_FAMILY_PATTERN = 4,
    MPTI_FAMILY_BGA     = 5,
    MPTI_FAMILY_PADBW   = 6,
    MPTI_FAMILY_SHAPEX  = 7,
    MPTI_FAMILY_BW      = 8
};

MPTI_BRIDGE_FLOW_API int MptiBridgeGetAlgoFamily(int algoType)
{
    switch (algoType) {
        case eAlgoAlign:            return MPTI_FAMILY_ALIGN;
        case eAlgoBlob:
        case eAlgoBody_Blob:
        case eAlgoNGBlob:           return MPTI_FAMILY_BLOB;
        case eAlgoEdge:
        case eAlgoBodyEdge:         return MPTI_FAMILY_EDGE;
        case eAlgoPattern:
        case eAlgoPatternDiff:
        case eAlgoOCR:
        case eAlgoPOCR:             return MPTI_FAMILY_PATTERN;
        case eAlgoBGA:              return MPTI_FAMILY_BGA;
        case eAlgoPadBW:            return MPTI_FAMILY_PADBW;
        case eAlgoShapeX:           return MPTI_FAMILY_SHAPEX;
        case eAlgoBW:               return MPTI_FAMILY_BW;
        default:                    return MPTI_FAMILY_UNKNOWN;
    }
}
```

→ Family 매핑이 native 의 single source of truth.

### C# 측 — Interop 노출

`src/WpfInspectionPrototype/WpfInspectionApp/Interop/MptiFlowNativeBridge.cs`:

```csharp
public const int FAMILY_UNKNOWN = 0;
public const int FAMILY_ALIGN   = 1;
public const int FAMILY_BLOB    = 2;
public const int FAMILY_EDGE    = 3;
public const int FAMILY_PATTERN = 4;
public const int FAMILY_BGA     = 5;
public const int FAMILY_PADBW   = 6;
public const int FAMILY_SHAPEX  = 7;
public const int FAMILY_BW      = 8;

[DllImport(Dll, CallingConvention = CallingConvention.Cdecl)]
public static extern int MptiBridgeGetAlgoFamily(int algoType);
```

### C# 측 — RuntimeFlowAlgorithmAdapter family-driven 리팩터

`Services/FlowAlgorithms/RuntimeFlowAlgorithmAdapter.cs`:

기존 13-case `switch (algorithm.Type)` → 8-case `switch (family)` + 보조 매핑.

핵심 변경:
1. `AlgoTypeMap` static dict — 13 entries, `algorithm.Type` (string) → native `algoType` (int) 매핑.
2. `ResolveInspType(algoType)` — OCR/POCR 만 `EINSP_OCR`, 나머지는 `EINSP_MOUNT` (Align 은 `EINSP_ALIGN`).
3. `ResolveDisplayName(algorithmType)` — `AlgorithmCatalog.Find(...)` 으로 이름 통일.
4. `MakeFamilyAlias(baseAlgo, displayName, algoType, inspType)` — base FlowAlgorithm 의 default 와 다를 때만 `FlowAlgorithmAlias` 로 wrapping. AlgoAlign 처럼 1:1 인 경우 wrapping 없이 그대로 반환.
5. `TryCreateFlowAlgorithm` 는 다음 흐름:
   - `algorithm.Type` 을 `AlgoTypeMap` 으로 native `algoType` 변환
   - `MptiBridgeGetAlgoFamily(algoType)` 으로 family lookup
   - family 에 따라 적절한 FlowAlgorithm + Parameters 생성

코드 라인 수: 130줄 → 110줄 + 도우미 30줄 (전체 비슷, 그러나 case 추가 시 1 entry only).

## 이득

- ✅ **새 algorithm type 추가 워크플로 단순화**: 같은 family 안의 새 type 추가 시 native 의 `MptiBridgeAddAlgo` switch + `MptiBridgeGetAlgoFamily` switch + C# `AlgoTypeMap` 한 줄만 추가하면 자동 dispatch.
- ✅ **Family 매핑의 source of truth 가 native**: C# 의 매핑이 native 와 일관성 보장.
- ✅ **IPINSP_ALGO polymorphism 활용 강화**: native 측 inspection dispatch 는 이미 polymorphic (변경 없음). C# 의 family lookup 도 native 가 책임지므로 추상화 경계가 명확해짐.
- ✅ **회귀 안전성**: 기존 동작 100% 보존 — `MakeFamilyAlias` 가 base FlowAlgorithm 의 default 와 일치하면 wrapping 생략, 다르면 기존과 동일하게 `FlowAlgorithmAlias` 로 wrapping.

## 의도적으로 안 한 것 (Level 3 의 큰 변경)

- `IPINSP_ALGO*` 직접 노출 + `InspAlgorithm` 13-매개변수 P/Invoke marshal:
  - `WndAlgoImg`, `InspRoiImgBuf`, `InspAlgoParam`, `TotalInspExceptArea`, `PIAL::PInspDataSet` 등 native struct 다수 mirror 필요.
  - C# 의 typed family Parameters struct 가 결국 다시 필요 → polymorphism 가치 작음.
  - 작업 surface 대비 이득 작음.

- C# 의 family-specific Parameters struct (`BlobParameters`, `EdgeParameters`, ...) 통합:
  - typed 안전성 잃고 native JSON 파싱 비용 추가.
  - 추천 안 함.

## 변경된 파일

- `src/WpfInspectionPrototype/MptiBridge/MptiBridgeFlow.cpp` (+ ~50줄)
- `src/WpfInspectionPrototype/WpfInspectionApp/Interop/MptiFlowNativeBridge.cs` (+ 15줄)
- `src/WpfInspectionPrototype/WpfInspectionApp/Services/FlowAlgorithms/RuntimeFlowAlgorithmAdapter.cs` (130줄 → 140줄, 구조 변경)

## 검증

```
MSBuild src/WpfInspectionPrototype/MptiBridge/MptiBridge.vcxproj /p:Configuration=Debug /p:Platform=x64
→ 빌드 성공, 신규 경고/오류 없음 (기존 legacy header LNK4217 만 유지)

dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0, 오류 0, 5.49초

src/WpfInspectionPrototype/WpfInspectionApp/bin/x64/Debug/net48/WpfInspectionApp.exe --smoke-test
→ EXIT=0 (5개 신규 smoke test 포함 모두 통과)
```

## 다음 작업 후보

1. **AlgorithmCatalog 에 native algoType 통합** — 현재 `AlgoTypeMap` 이 RuntimeFlowAlgorithmAdapter 내 static dict. `AlgorithmCatalogItem` record 에 `NativeAlgoType` int field 추가하면 single lookup 으로 통합.
2. **AlgorithmNativeBridgeAdapter (fallback) 도 family-driven 으로** — 본 작업과 동일 패턴 적용.
3. **`MptiBridgeGetAlgoFamily` 결과를 SmokeTestRunner 에서 회귀 검증** — 각 family 의 대표 algorithm 으로 family 값 확인.
4. **새 family 추가** (예: `FAMILY_HEIGHT` for AlgoHeight_Mean / AlgoHeight_Diff) — 현재 모두 `FAMILY_UNKNOWN` 으로 떨어져 flow path 미사용. 필요 시 native 측에 새 family + 새 FlowAlgorithm 추가.

## 빌드 / 재현 명령

```powershell
& 'C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe' `
  src\WpfInspectionPrototype\MptiBridge\MptiBridge.vcxproj `
  /p:Configuration=Debug /p:Platform=x64 /m

dotnet build src\WpfInspectionPrototype\WpfInspectionApp\WpfInspectionApp.csproj `
  -p:Configuration=Debug -p:Platform=x64

& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe --smoke-test
```
