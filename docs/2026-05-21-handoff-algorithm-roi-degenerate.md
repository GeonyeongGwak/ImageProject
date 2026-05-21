# WPF Inspection App Handoff - Algorithm ROI 0.01x0.01 mm 유령 사각형 제거

작성일: 2026-05-21
프로젝트: `C:\Users\USER\Documents\NewProject\ImageProject`
선행 문서: `docs/2026-05-20-handoff-tree-bring-into-view.md` 외 9개.

## 1. 작업 원칙

- MVVM 위반 여부는 기능 추가 전/후 항상 확인한다.
- View/XAML/code-behind 에는 화면 이벤트 연결, 바인딩, UI 표시만 둔다.
- 값 검증, enable/disable 상태 판단, 파라미터 저장/정규화는 ViewModel/Model/Service.
- 기존 사용자가 만든 변경은 되돌리지 않는다.

## 2. 이번 세션 작업 (사용자 보고 → 진단 → 수정)

### 2.1 보고된 증상

U25 (2.zip) part import 후 ROI 라벨에 `AlgoHeight_Diff ROI 0.01x0.01 mm` 가 표시되고 사각형이 화면 좌상단에 1픽셀로 찍혀 있는 것처럼 보임. 사용자 표현: "랙트가 재대로 안나오는거 같아".

### 2.2 진단 — XML 직접 분석

U25.xml 구조 확인 결과:

#### 1) PartData/Roi (Part 전체 영역)
```xml
<Roi>
  <w>10.8568</w>
  <h>16.1209</h>
  <cx>12.3736</cx>
  <cy>67.7987</cy>
  <a>270.0</a>          ← Part 좌표계 자체가 270° 회전
</Roi>
```

#### 2) 모든 WindowData 의 RelRoi 패턴
| Window | w (mm) | h (mm) | cx | cy | a |
|---|---|---|---|---|---|
| window1 | 8.6783 | 13.6134 | 0.0000 | 0.0000 | 0.0 |
| window2 | 8.6783 | 13.6134 | 0.0000 | 0.0000 | 0.0 |
| window5 | 8.6173 | 13.4712 | 0.0000 | 0.0000 | 0.0 |
| window6 | 8.4216 | 13.2701 | 0.0000 | 0.0000 | 0.0 |

→ **모든 Window 의 cx/cy/a 가 0** — Part 회전을 어떻게 보정해도 (0,0) → (0,0) 이라 결과 변화 없음. 4개 Window 가 모두 image 중앙에 겹쳐 그려지는 건 import 버그가 아니라 **XML 데이터 자체의 한계** (export 시점에 cx/cy 가 다 0 으로 떨어진 것).

#### 3) Algorithm ROI 가 0.01x0.01 mm 로 들어오는 진짜 원인 (= 사용자가 본 증상)

`ParseOptionalAlgorithmRoi` 의 휴리스틱:
```csharp
var roi = algorithmElement.Elements()
    .FirstOrDefault(element => element.Name.LocalName.Contains("Roi", StringComparison.OrdinalIgnoreCase)
        && element.Elements().Any());   // 자식이 있는 첫 element
```

U25 첫 AlgorithmData 자식 중 이름에 "Roi" 가 들어가는 element 들:

| element | 자식 element | w/h 있음 | 진짜 ROI? |
|---|---|---|---|
| `<ROI1>-423,-666,423,666</ROI1>` | 없음 (텍스트만) | 아니오 | ✅ 실제 algorithm ROI (px CSV) |
| `<ROI1_mm>-4.22,-6.65,4.22,6.65</ROI1_mm>` | 없음 (텍스트만) | 아니오 | ✅ 실제 algorithm ROI (mm CSV) |
| `<ROI2>`, `<ROI2_mm>` | 없음 | 아니오 | ✅ |
| `<RcAngROI1>` ~ `<RcAngROI8>` | (다양) | ? | 아마도 회전 ROI |
| `<BROI2>` | `<BlobBN>`, `<BlobBF>` | ❌ **없음** | ❌ Blob 검사 파라미터 컨테이너 |

→ 휴리스틱이 `<BROI2>` (BlobBN/BlobBF 자식만 있는 Blob 설정 컨테이너) 를 매칭함. `<w>` `<h>` 가 없으므로 `ReadDouble` 이 0 반환 → `Math.Max(1, Round(0))` = **1 픽셀** → 0.01 mm/픽셀 × 1 = **0.01 mm × 0.01 mm 유령 ROI** 표시.

### 2.3 수정

`Models/LegacyRawPartImportAdapter.cs` `ParseOptionalAlgorithmRoi` 휴리스틱 강화:

```csharp
var roi = algorithmElement.Elements()
    .FirstOrDefault(element => element.Name.LocalName.Contains("Roi", StringComparison.OrdinalIgnoreCase)
        && element.Element("w") != null
        && element.Element("h") != null);   // ← 추가 가드
return roi == null ? null : ParseRoi(roi, transform);
```

- 이름에 "Roi" 가 들어가도 `<w>` `<h>` 자식이 둘 다 있어야 후보. `<BROI2>` (Blob 컨테이너) 제외됨.
- `algorithm-specific ROI` 가 정의 안 된 경우 (U25 의 모든 algorithm) → `AlgorithmRoi = null` 로 들어가서 화면에 algorithm ROI 자체를 안 그림. Window ROI 만 그려짐.
- 부수 효과: `<ROI1>` / `<ROI1_mm>` 같은 CSV 텍스트 형식의 진짜 algorithm ROI 는 여전히 import 안 됨 — 별도 파싱 로직 필요. 시각적으로는 0.01x0.01 mm 유령보다 차라리 "안 그림" 이 깔끔.

### 2.4 시도했다가 되돌린 것

이전 시도: `ParseRoi` 에 `<a>` 읽고 90/270° 면 W/H 스왑. 본 분석 결과 U25 의 모든 Window 가 a=0 이라 효과 없고, 사용자 요청에 따라 워킹트리 차원에서 롤백 후 본 변경만 진행.

Part 회전(`<Roi a=270>`) 보정도 모든 cx/cy 가 0 이라 이 데이터에선 동작 결과 동일. 적용 보류.

## 3. 변경된 파일

- `src/WpfInspectionPrototype/WpfInspectionApp/Models/LegacyRawPartImportAdapter.cs`

## 4. 검증 결과

```
dotnet build src/WpfInspectionPrototype/WpfInspectionApp/WpfInspectionApp.csproj -p:Configuration=Debug -p:Platform=x64
→ 경고 0개, 오류 0개, 4.91초

src/WpfInspectionPrototype/WpfInspectionApp/bin/x64/Debug/net48/WpfInspectionApp.exe --smoke-test
→ EXIT=0
```

MVVM: View/XAML/code-behind/ViewModel 변경 없음. Model layer 의 import 어댑터만 수정.

## 5. 남은 위험 / 다음 확인

- **GUI 확인 1**: U25 import 후 `AlgoHeight_Diff ROI 0.01x0.01 mm` 라벨이 사라지는지.
- **GUI 확인 2**: RefID_2 import (1.zip) 의 algorithm ROI 가 여전히 정상 표시되는지 (회귀 없는지).
- **데이터 한계**: U25 의 4개 Window 가 모두 동일 위치에 겹쳐 그려지는 건 데이터 한계. Part 회전 + cx/cy 계산이 다른 export 로직을 거친 part 가 있어야 회전 보정 코드의 가치가 확인됨.
- **다음 후보 작업**: `<ROI1>` / `<ROI1_mm>` CSV 텍스트 형식을 algorithm ROI 로 파싱 (left,top,right,bottom). U25 의 실제 algorithm ROI 가 화면에 보이려면 필요한 추가 작업.
- **그 외**: 동일 휴리스틱이 `ParseOptionalAlgorithmRoi` 외에 다른 곳에서도 쓰이는지 확인 (현재는 한 곳).

## 6. 빌드 / 재현 명령

```powershell
dotnet build src\WpfInspectionPrototype\WpfInspectionApp\WpfInspectionApp.csproj `
  -p:Configuration=Debug -p:Platform=x64

& src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48\WpfInspectionApp.exe --smoke-test
```
