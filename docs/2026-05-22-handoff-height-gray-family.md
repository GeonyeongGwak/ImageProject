# 2026-05-22 Handoff - 새 family (FAMILY_HEIGHT / FAMILY_GRAY) 추가

## 배경

직전까지 family lookup 인프라가 8 family (Align / Blob / Edge / Pattern / BGA / PadBW /
ShapeX / BW) 만 분류했고, 3D 측정 계열 (`AlgoHeight_Mean`, `AlgoHeight_Diff`, `AlgoVolume`,
`AlgoPackageThickness`) 및 2D gray 측정 (`AlgoGray_Mean`, `AlgoGray_Diff`) 은 `FAMILY_UNKNOWN`
으로 떨어지고 있었다.

본 작업은 family lookup 만 두 개 family 더 추가한다. 풀 flow path (각 algorithm 의
native param/result struct + `MptiBridgeAddAlgo` case + `IFlowAlgorithm` 클래스) 는 별도
작업으로 위임 — 각 algorithm 별로 native struct mirror + setter + reader + C# 측 wrap 가
독립적으로 필요.

## 변경 — family lookup 확장

### 1) `Models/NativeAlgoTypeIds.cs` — 6 개 enum 추가

```csharp
public const int GrayMean         = 8;
public const int HeightMean       = 9;
public const int GrayDiff         = 10;
public const int HeightDiff       = 11;
public const int Volume           = 19;
public const int PackageThickness = 43;
```

native `InspParamDef_Algo.h` 의 `eAlgo*` enum 순서 (0-base) 기준. 검증된 native struct
존재: `NativeSources/MPTILib_Algo/PInsp_Algo/HeightMean/InspParamDef_HeightMean.h`,
`HeightDiff/`, `Volume/`, `GrayMean/`, `GrayDiff/` — 모두 `AlgoBaseBW` 상속 + 측정 전용 필드 보유.

### 2) `MptiBridgeFlow.cpp` — `MPTI_FAMILY_HEIGHT` + `MPTI_FAMILY_GRAY`

```cpp
enum MptiBridgeAlgoFamily {
    ...
    MPTI_FAMILY_BW = 8,
    MPTI_FAMILY_HEIGHT = 9,   // 3D measurement
    MPTI_FAMILY_GRAY   = 10   // 2D gray measurement
};

MPTI_BRIDGE_FLOW_API int MptiBridgeGetAlgoFamily(int algoType) {
    switch (algoType) {
        ...
        case eAlgoHeight_Mean:
        case eAlgoHeight_Diff:
        case eAlgoVolume:
        case eAlgoPackageThickness:
            return MPTI_FAMILY_HEIGHT;
        case eAlgoGray_Mean:
        case eAlgoGray_Diff:
            return MPTI_FAMILY_GRAY;
        ...
    }
}
```

### 3) C# `MptiFlowNativeBridge.FAMILY_*` 상수 alias

```csharp
public const int FAMILY_HEIGHT = 9;
public const int FAMILY_GRAY   = 10;
```

### 4) `AlgorithmCatalog` 의 6 entry 에 `NativeAlgoType` 추가

| Catalog entry | NativeAlgoType | family |
|---|---|---|
| AlgoGray_Mean | GrayMean (8) | GRAY |
| AlgoHeight_Mean | HeightMean (9) | HEIGHT |
| AlgoGray_Diff | GrayDiff (10) | GRAY |
| AlgoHeight_Diff | HeightDiff (11) | HEIGHT |
| AlgoVolume | Volume (19) | HEIGHT |
| AlgoPackageThickness | PackageThickness (43) | HEIGHT |

### 5) `SmokeTestRunner.VerifyAlgoFamilyLookupMatchesCatalog` 확장

6 algorithm × (NativeAlgoType + Family) 회귀 잠금 추가. 총 25 algorithm 커버.

## 영향

- ✅ `MptiBridgeGetAlgoFamily` 가 25 algorithm 의 family 를 알게 됨 (기존 19 + 신규 6).
- ✅ `AlgorithmCatalog.Find(type).NativeAlgoType` 가 6 algorithm 에 대해 정확한 native ID 반환.
- ✅ 회귀 잠금: smoke test 가 매핑 깨짐을 잡아냄.
- ⚠ **풀 flow path 는 아직 미구현** — `RuntimeFlowAlgorithmAdapter.TryCreateFlowAlgorithm`
  의 switch 에 `FAMILY_HEIGHT` / `FAMILY_GRAY` case 가 없어서 default 로 떨어지고 false 반환.
  결과적으로 이들 algorithm 은 **per-algo bridge fallback** (`AlgorithmNativeBridgeAdapter`)
  로 떨어지는데, 거기서도 `IsFamily(...)` 의 Blob/BGA/Edge/Pattern 어느 family 에도 매칭
  안 되므로 결국 `GenericProfiles` 로 들어가 `MptiBridgeRunGeneric` 으로 처리됨 (기존 동작 유지).
- ✅ **회귀 없음**: 기존 `Unknown` → `Generic` 경로가 그대로 유지. 단지 family lookup 결과만
  더 정확해짐.

## 풀 flow path 구현을 위한 다음 단계 (필요 시)

각 algorithm 마다 다음 작업이 추가 필요:

### 예시 — `AlgoHeight_Mean` 풀 flow path

1. **Native struct mirror** (C# 측):
   ```csharp
   [StructLayout(LayoutKind.Sequential, Pack = 8)]
   public struct MptiBridgeFlowHeightMeanParams {
       public int UseHeight; public double HeightMin3D; public double HeightMax3D;
       public double AvgHeight3D; public int UseHighest; ...
   }
   [StructLayout(LayoutKind.Sequential, Pack = 8)]
   public struct MptiBridgeFlowHeightMeanResult {
       public MptiBridgeFlowResultHeader Hdr;
       public double RstHeightMean; public double RstHeighestValue; public double RstLowestValue;
       public int OkHeightMean; public int OkHeighest; public int OkLowest;
       public int NoSolder; public int Excess;
   }
   ```

2. **Native exports** (`MptiBridgeFlow.cpp`):
   - `MptiBridgeAddAlgo` switch 에 `case eAlgoHeight_Mean: as->Alloc<AlgoHeightMean>(algoType); break;`
   - `MptiBridgeSetAlgoParamsHeightMean(int wndIdx, int algoIdx, const MptiBridgeFlowHeightMeanParams* params)`
     — AlgoBaseBW 공통 필드 + Height 전용 필드 매핑
   - `MptiBridgeResultHeightMean(int wndType, int wndIdx, int algoIdx, MptiBridgeFlowHeightMeanResult* result)`
     — `RstAlgoHeightMean` 결과 읽어 mirror struct 채움

3. **C# `IFlowAlgorithm`** (`HeightMeanFlowAlgorithm.cs`):
   - `HeightMeanParameters : IFlowAlgorithmParameters` (ObservableObject)
   - `DisplayName="Height Mean"`, `AlgoType=NativeAlgoTypeIds.HeightMean`, `InspType=EINSP_MOUNT`
   - `ApplyParams` → `MptiFlowNativeBridge.MptiBridgeSetAlgoParamsHeightMean(slot, ref p)`
   - `ReadResult` → `MptiFlowNativeBridge.MptiBridgeResultHeightMean(slot, ref r)`

4. **`RuntimeFlowAlgorithmAdapter`** family handler:
   ```csharp
   case MptiFlowNativeBridge.FAMILY_HEIGHT:
       flowAlgorithm = ResolveHeightFlowAlgorithm(algoType, displayName, inspType);
       flowParameters = ResolveHeightParameters(algoType, model, algorithm);
       return true;
   ```
   `ResolveHeightFlowAlgorithm` 가 algoType 별로 `HeightMeanFlowAlgorithm` / `HeightDiffFlowAlgorithm` /
   `VolumeFlowAlgorithm` / `PackageThicknessFlowAlgorithm` 중 골라 반환.

5. **smoke test** — fallback 라우팅 회귀 + (선택) flow path 단위 테스트.

Family 1 개당 위 작업이 algorithm 수만큼 반복. Height family 는 4 algorithm,
Gray family 는 2 algorithm = 총 6 set 의 native + C# 작업.

## 변경된 파일

- `src/WpfInspectionPrototype/WpfInspectionApp/Models/NativeAlgoTypeIds.cs` (+ 6 상수)
- `src/WpfInspectionPrototype/MptiBridge/MptiBridgeFlow.cpp` (+ 2 family, + 6 case)
- `src/WpfInspectionPrototype/WpfInspectionApp/Interop/MptiFlowNativeBridge.cs` (+ 2 alias 상수)
- `src/WpfInspectionPrototype/WpfInspectionApp/Models/AlgorithmCatalog.cs` (6 entry 에 인수 추가)
- `src/WpfInspectionPrototype/WpfInspectionApp/Diagnostics/SmokeTestRunner.cs` (회귀 확장)

## 검증

```
MSBuild MptiBridge.vcxproj /p:Configuration=Debug /p:Platform=x64
→ 빌드 성공 (기존 legacy 경고만)

dotnet build WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0, 오류 0, 5.21초

WpfInspectionApp.exe --smoke-test
→ EXIT=0 (25 algorithm + 3 edge case 모두 통과)
```

## 다음 작업 후보 (직전 handoff §"다음 작업 후보" 잔여)

1. ~~AlgorithmCatalog 통합~~ ✅
2. ~~AlgorithmNativeBridgeAdapter family-driven~~ ✅
3. ~~MptiBridgeGetAlgoFamily smoke 회귀~~ ✅
4. ~~새 family (Height / Gray)~~ ✅ (lookup 만)
5. **Height/Gray family 의 풀 flow path 구현** — 위 "다음 단계" 섹션의 5-step 작업을
   AlgoHeight_Mean / AlgoHeight_Diff / AlgoVolume / AlgoGray_Mean 등에 적용. 1 algorithm 당
   ~150-300 LOC.
6. **신규 6 항목 (Bump/Line/Distance/EdgePoint/ForeignOCV/LQBGA) native `MptiBridgeAddAlgo`
   case 추가** — flow path 실제 작동 (family 자체는 이미 매핑됨).
7. **다른 family 추가** — `FAMILY_LEAD` (Lead_Tip/Lift/Solder/Color/Search/SideSolder),
   `FAMILY_BRIDGE` (Bridge), `FAMILY_COLOR` (Color/DisColor/ColorXY/ColorBand_Search/LeadColor),
   `FAMILY_TAB` (Tab/Tab_Search) 등.

## 빌드 / 재현 명령

```powershell
& 'C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe' `
  src\WpfInspectionPrototype\MptiBridge\MptiBridge.vcxproj /p:Configuration=Debug /p:Platform=x64 /m

dotnet build src\WpfInspectionPrototype\WpfInspectionApp\WpfInspectionApp.csproj `
  -p:Configuration=Debug -p:Platform=x64

& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe --smoke-test
```
