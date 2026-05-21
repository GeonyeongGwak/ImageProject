namespace WpfInspectionApp.Models;

// Native InspAlgoType 의 enum 값. source of truth 는 native side
// NativeSources/MPTILib_Algo/PInsp_Algo/InspParamDef_Algo.h 의 eAlgo* 상수.
//
// AlgorithmCatalog 가 algorithm type 별 native algoType 을 보관하고,
// MptiFlowNativeBridge.EALGO_* alias 가 같은 값을 P/Invoke 호출 시 노출한다.
// → C# 의 algoType 정수가 어디서 나타나든 결국 본 상수의 별칭이 되어 일관성 보장.
//
// 새 algorithm 타입을 flow path 에 추가할 때:
//   1) native InspParamDef_Algo.h 의 eAlgo* enum 값 확인
//   2) 본 파일에 동일 이름/값 상수 추가
//   3) AlgorithmCatalog 의 해당 entry 에 nativeAlgoType 인수 채움
//   4) MptiBridgeFlow.cpp 의 MptiBridgeAddAlgo / MptiBridgeGetAlgoFamily switch 확장
//
// 0 (Unknown) 은 flow path 미지원 / 카탈로그 default 를 의미한다.
public static class NativeAlgoTypeIds
{
    public const int Unknown            = 0;
    public const int Blob               = 1;
    public const int Align              = 2;
    public const int BodyBlob           = 3;
    public const int Ocr                = 5;
    public const int Pattern            = 6;
    public const int GrayMean           = 8;
    public const int HeightMean         = 9;
    public const int GrayDiff           = 10;
    public const int HeightDiff         = 11;
    public const int Volume             = 19;
    public const int Line               = 25;
    public const int Edge               = 26;
    public const int Pocr               = 31;
    public const int Bga                = 37;
    public const int Bump               = 38;
    public const int NgBlob             = 39;
    public const int PadBw              = 40;
    public const int ForeignOcv         = 41;
    public const int BodyEdge           = 42;
    public const int PackageThickness   = 43;
    public const int Distance           = 44;
    public const int EdgePoint          = 46;
    public const int PatternDiff        = 47;
    public const int ShapeX             = 48;
    public const int LqBga              = 50;
}
