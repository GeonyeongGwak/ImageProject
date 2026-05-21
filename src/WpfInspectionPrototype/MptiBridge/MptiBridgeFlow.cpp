#include "stdafx.h"
#include "MPTI.h"
#include "MptiBridgeFlow.h"
#include "NativeSources/MPTILib_Algo/InspManager.h"
#include "NativeSources/PInspAlgo/PInspAlgo_Lib.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/InspParamDef_Algo.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/InspParamDef_AlgoBase.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/Align/InspParamDef_Align.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/BW/InspParamDef_BW.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/PadBW/InspParamDef_PadBW.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/Blob/InspParamDef_Blob.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/BGA/InspParamDef_BGA.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/Edge/InspParamDef_Edge.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/Pattern/InspParamDef_Pattern.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/ShapeX/InspParamDef_ShapeX.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/BodyBlob/InspParamDef_BodyBlob.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/NGBlob/InspParamDef_NGBlob.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/BodyEdge/InspParamDef_BodyEdge.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/PatternDiff/InspParamDef_PatternDiff.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/OCR/InspParamDef_OCR.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/POCR/InspParamDef_POCR.h"

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cwchar>
#include <exception>
#include <memory>
#include <vector>

namespace
{
    // Holder for the native AlgoXxx struct backing one (window, algo) slot. Each native
    // algo type has its own struct (AlgoAlign, AlgoPadBW, etc.) — some contain CString
    // members so we cannot reuse a single raw byte buffer for all of them. Instead we
    // hold a void* + an "operator vtable" (deleter, allocator) that knows the runtime
    // type. The deleter takes void* and re-casts to the right type before delete.
    struct AlgoStorage
    {
        int   type = -1;     // InspAlgoType
        void* params = nullptr;
        void (*deleter)(void*) = nullptr;
        int lightRed[10] = {};
        int lightGreen[10] = {};
        int lightBlue[10] = {};
        int lightWhite[10] = {};
        int lightCalculation[10] = {};
        int lightPosition[10] = {};

        AlgoStorage() = default;
        AlgoStorage(const AlgoStorage&) = delete;
        AlgoStorage& operator=(const AlgoStorage&) = delete;
        AlgoStorage(AlgoStorage&& other) noexcept
            : type(other.type), params(other.params), deleter(other.deleter)
        {
            other.type = -1; other.params = nullptr; other.deleter = nullptr;
        }
        AlgoStorage& operator=(AlgoStorage&& other) noexcept
        {
            if (this != &other)
            {
                if (deleter && params) deleter(params);
                type = other.type; params = other.params; deleter = other.deleter;
                other.type = -1; other.params = nullptr; other.deleter = nullptr;
            }
            return *this;
        }
        ~AlgoStorage() { if (deleter && params) deleter(params); }

        // Allocate a default-constructed native param struct of type T and remember how
        // to delete it. T must match the algo's native AlgoXxx struct exactly.
        template <typename T>
        T* Alloc(int algoType)
        {
            if (deleter && params) deleter(params);
            T* p = new T();
            params = p;
            deleter = [](void* raw) { delete static_cast<T*>(raw); };
            type = algoType;
            return p;
        }
    };

    struct WindowStorage
    {
        std::vector<InspAlgo>                    algos;
        std::vector<std::unique_ptr<AlgoStorage>> algoStore;
    };

    InspPartInfo                s_part{};
    std::vector<InspPartParam>  s_windows;
    std::vector<WindowStorage>  s_windowStore;
    bool                        s_committed = false;
    bool                        s_inspected = false;
    DWORD                       s_lastInspProcSehCode = 0;
    PVOID                       s_lastInspProcSehAddress = nullptr;
    wchar_t                     s_lastInspProcSehModule[260] = {0};

    int s_sourceWidth = 0;
    int s_sourceHeight = 0;

    // Resolution explicitly provided by the caller (typically read from .pot during
    // Part Import or PTT load on the C# side). 0 means "not set — fall back to 1.0".
    // Set via MptiBridgeSetFlowResolution.
    double s_explicitResolX = 0.0;
    double s_explicitResolY = 0.0;

    void Reset()
    {
        memset(&s_part, 0, sizeof(s_part));
        s_windows.clear();
        s_windowStore.clear();
        s_committed = false;
        s_inspected = false;
        s_sourceWidth = s_sourceHeight = 0;
        // Note: do NOT reset s_explicitResolX/Y here — resolution survives across
        // BeginPart calls so the WPF UI can set it once after PTT load and reuse it
        // for multiple Run-Flow invocations.
    }

    int WriteMsg(wchar_t* message, int len, const wchar_t* text)
    {
        if (message != nullptr && len > 0)
            wcsncpy_s(message, static_cast<size_t>(len), text, _TRUNCATE);
        return 0;
    }

    int Fail(wchar_t* message, int len, int code, const wchar_t* text)
    {
        WriteMsg(message, len, text);
        return code;
    }

    int ClampIntValue(int value, int minValue, int maxValue)
    {
        if (value < minValue) return minValue;
        if (value > maxValue) return maxValue;
        return value;
    }

    int ClampLightChannel(int value)
    {
        return ClampIntValue(value, 0, 200);
    }

    struct BridgeBinaryParams
    {
        int useInsp2D;
        int minBinary;
        int maxBinary;
        int typeRange2D;
        int invertCheck;
    };

    struct BridgeShiftParams
    {
        int useShift;
        double shiftX;
        double shiftY;
    };

    struct BridgeAreaParams
    {
        int useArea;
        double areaMin;
        double areaMax;
    };

    template <typename T>
    void ApplyCommonBinary(T& algo, const BridgeBinaryParams& params)
    {
        algo.m_bInsp2D      = params.useInsp2D != 0;
        algo.m_nMinBinary   = params.minBinary;
        algo.m_nMaxBinary   = params.maxBinary;
        algo.m_nTypeRange2D = params.typeRange2D;
        algo.m_bInvertCheck = params.invertCheck != 0;
    }

    void ApplyCommonBinary(AlgoAlign& algo, const BridgeBinaryParams& params)
    {
        algo.m_bInsp2D      = params.useInsp2D != 0;
        algo.m_nMinBinary   = params.minBinary;
        algo.m_nMaxBinary   = params.maxBinary;
        algo.m_nTypeRange2D = params.typeRange2D;
        algo.m_InvertCheck  = params.invertCheck != 0;
    }

    void ApplyCommonBinary(Bin& bin, const BridgeBinaryParams& params)
    {
        bin.m_bInsp2D      = params.useInsp2D != 0;
        bin.m_nMinBinary   = params.minBinary;
        bin.m_nMaxBinary   = params.maxBinary;
        bin.m_nTypeRange2D = params.typeRange2D;
    }

    void ApplyAlgoBaseBW(AlgoBaseBW& algo, const BridgeBinaryParams& params)
    {
        algo.m_b2dCheck     = params.useInsp2D != 0;
        algo.m_nMinValue    = params.minBinary;
        algo.m_nMaxValue    = params.maxBinary;
        algo.m_nRange       = params.typeRange2D;
        algo.m_bInvertCheck = params.invertCheck != 0;
    }

    template <typename T>
    void ApplyShift(T& algo, const BridgeShiftParams& params)
    {
        algo.m_bShiftIsUse = params.useShift != 0;
        algo.m_dShiftX     = params.shiftX;
        algo.m_dShiftY     = params.shiftY;
    }

    void ApplyShift(AlgoAlign& algo, const BridgeShiftParams& params)
    {
        algo.m_bUseShift = params.useShift != 0;
        algo.m_dShiftX   = params.shiftX;
        algo.m_dShiftY   = params.shiftY;
    }

    template <typename T>
    void ApplyArea(T& algo, const BridgeAreaParams& params)
    {
        algo.m_bAreaIsUse = params.useArea != 0;
        algo.m_dAreaMin   = params.areaMin;
        algo.m_dAreaMax   = params.areaMax;
    }

    int ScoreToPercent(double score)
    {
        if (score <= 1.0)
            score *= 100.0;
        if (score < 0.0) return 0;
        if (score > 100.0) return 100;
        return static_cast<int>(score + 0.5);
    }

    void BindLightStorage(InspAlgo& algo, AlgoStorage& storage)
    {
        algo.m_nArrRedValue = storage.lightRed;
        algo.m_nArrGreenValue = storage.lightGreen;
        algo.m_nArrBlueValue = storage.lightBlue;
        algo.m_nArrWhiteValue = storage.lightWhite;
        algo.m_nArrCalculation = storage.lightCalculation;
        algo.m_nArrLightPosition = storage.lightPosition;
    }

    // SEH filter that captures code + faulting address + module name. Runs in the
    // exception context BEFORE the __except handler. Returning EXCEPTION_EXECUTE_HANDLER
    // tells SEH to unwind to our handler.
    int CaptureInspProcSehFilter(EXCEPTION_POINTERS* ep)
    {
        if (ep && ep->ExceptionRecord)
        {
            s_lastInspProcSehCode = ep->ExceptionRecord->ExceptionCode;
            s_lastInspProcSehAddress = ep->ExceptionRecord->ExceptionAddress;
            HMODULE hMod = nullptr;
            if (GetModuleHandleExW(
                    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                    (LPCWSTR)ep->ExceptionRecord->ExceptionAddress, &hMod) && hMod)
            {
                GetModuleFileNameW(hMod, s_lastInspProcSehModule, 260);
            }
            else
            {
                s_lastInspProcSehModule[0] = L'\0';
            }
        }
        return EXCEPTION_EXECUTE_HANDLER;
    }

    int CallMptiInspProcGuarded()
    {
        s_lastInspProcSehCode = 0;
        s_lastInspProcSehAddress = nullptr;
        s_lastInspProcSehModule[0] = L'\0';
        __try
        {
            return MPTI_InspProc();
        }
        __except (CaptureInspProcSehFilter(GetExceptionInformation()))
        {
            return -300;
        }
    }
}

// Source of truth for algoType -> family mapping. C# 의 RuntimeFlowAlgorithmAdapter
// 가 이 함수를 호출해서 어느 family-specific param builder/result reader 를 써야 할지
// 결정한다. 새 algoType 추가 시 여기에 한 줄만 추가하면 C# 측이 자동으로 따라간다.
//
// IPINSP_ALGO 의 polymorphism 자체는 native side InspManager 가 InspAlgorithm /
// InspAlgorithm_Dll virtual call 로 dispatch 하므로 C# 가 알 필요 없음. C# 가 신경
// 쓰는 건 "이 algoType 에 어떤 family-specific param struct 를 보낼 것인가" 뿐.
enum MptiBridgeAlgoFamily
{
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
    // 본 함수는 "logical family" 를 반환 — flow API 지원 여부와 무관하게 알고리즘의
    // 의미적 카테고리를 분류한다. 예를 들어 eAlgoBump 는 flow path 미지원이지만
    // C# AlgorithmNativeBridgeAdapter 가 per-algo Blob bridge 로 라우팅하기 위해
    // BLOB family 가 필요하다.
    switch (algoType)
    {
    case eAlgoAlign:
        return MPTI_FAMILY_ALIGN;
    case eAlgoBlob:
    case eAlgoBody_Blob:
    case eAlgoNGBlob:
    case eAlgoBump:
        return MPTI_FAMILY_BLOB;
    case eAlgoEdge:
    case eAlgoBodyEdge:
    case eAlgoLine:
    case eAlgoDistance:
    case eAlgoEdgePoint:
        return MPTI_FAMILY_EDGE;
    case eAlgoPattern:
    case eAlgoPatternDiff:
    case eAlgoOCR:
    case eAlgoPOCR:
    case eAlgoForeignOCV:
        return MPTI_FAMILY_PATTERN;
    case eAlgoBGA:
    case eAlgoLQBGA:
        return MPTI_FAMILY_BGA;
    case eAlgoPadBW:
        return MPTI_FAMILY_PADBW;
    case eAlgoShapeX:
        return MPTI_FAMILY_SHAPEX;
    case eAlgoBW:
        return MPTI_FAMILY_BW;
    default:
        return MPTI_FAMILY_UNKNOWN;
    }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeSetFlowResolution(double resolX, double resolY)
{
    try
    {
        // Negative resolutions are nonsensical — treat as a reset.
        if (resolX < 0.0 || resolY < 0.0)
        {
            s_explicitResolX = 0.0;
            s_explicitResolY = 0.0;
            return 0;
        }
        s_explicitResolX = resolX;
        s_explicitResolY = resolY;
        return 0;
    }
    catch (...) { return -100; }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeBeginPart(
    double partCx, double partCy, double partWidth, double partHeight, double angle,
    int sourceWidth, int sourceHeight)
{
    try
    {
        Reset();
        s_part.partCx = partCx;
        s_part.partCy = partCy;
        s_part.partWidth = partWidth;
        s_part.partHeight = partHeight;
        // MakePartParam (reference, ucSamePartPreTest.cs:541-545 + InspectionFunc.cs) seeds
        // anyAngle* from the boundary ROI. For a non-rotated minimal flow it equals partRect.
        s_part.anyAngleCx = partCx;
        s_part.anyAngleCy = partCy;
        s_part.anyAngleWidth = partWidth;
        s_part.anyAngleLength = partHeight;
        s_part.angle = angle;
        s_part.partIndex = 0;
        s_part.nPartID = 1;
        // Minimum string fields the legacy code reads as wide-strings.
        wcsncpy_s(s_part.modelName, MAX_STRLEN, L"WpfInspectionPrototype", _TRUNCATE);
        wcsncpy_s(s_part.ModuleNo, MAX_STRLEN, L"M1", _TRUNCATE);
        wcsncpy_s(s_part.PartNo, MAX_STRLEN, L"P1", _TRUNCATE);
        s_sourceWidth = sourceWidth;
        s_sourceHeight = sourceHeight;
        return 0;
    }
    catch (...) { return -100; }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeAddWindow(
    int wndInspType, double cx, double cy, double width, double height,
    int alignWndId, int parentWndId)
{
    try
    {
        if (s_committed)
            return -10;

        InspPartParam wnd{};
        wnd.inspType = wndInspType;
        wnd.cx = cx;
        wnd.cy = cy;
        wnd.width = width;
        wnd.length = height;
        // MakePartParam (reference) seeds the window's anyAngle* from the boundary ROI; for
        // a non-rotated minimal flow it equals the window rect itself.
        wnd.anyAngleCx = cx;
        wnd.anyAngleCy = cy;
        wnd.anyAngleWidth = width;
        wnd.anyAngleLength = height;
        wnd.wndIndex = static_cast<int>(s_windows.size()) + 1;
        wnd.groupIndex = static_cast<int>(s_windows.size()) + 1;   // one window == one group (SortingParamater skips groupIndex < 1)
        wnd.nAlignWndID = alignWndId;
        wnd.nParentWndID = parentWndId;
        wnd.WndInspType = static_cast<byte>(wndInspType);
        wnd.nInspCameraType = 0;       // 0 = eCoaxial (top camera), reference default
        wnd.Gen2D = 0;                  // MakePartParam line 462
        wnd.m_nAlignPartWnd = 0;        // 1 would make InspNormal_Ver2 skip this window (line 1441)
        wnd.nAlgorithmCnt = 0;
        wnd.vArrAlgoParam = nullptr;
        wnd.m_nUsedWndPolygon = 0;

        s_windows.push_back(wnd);
        s_windowStore.emplace_back();
        return static_cast<int>(s_windows.size()) - 1;
    }
    catch (...) { return -100; }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeAddAlgo(int wndIndex, int algoType, int algoId)
{
    try
    {
        if (s_committed)
            return -10;
        if (wndIndex < 0 || wndIndex >= static_cast<int>(s_windows.size()))
            return -11;

        InspAlgo algo{};
        algo.m_nAlgoId = algoId;
        algo.m_eAlgoType = static_cast<InspAlgoType>(algoType);
        algo.m_bIsRequired = FALSE;
        algo.m_bAlgoEnable = TRUE;
        algo.m_bAlgoGroup = FALSE;
        algo.m_eLightType = Top_Light;
        algo.m_nRedValue = 255;
        algo.m_nGreenValue = 0;
        algo.m_nBlueValue = 0;
        algo.m_nWhiteValue = 0;
        algo.m_ptrInspAlgoParam = nullptr;   // wired in Commit
        algo.m_nLightCnt = 0;
        algo.m_nUsedMaskingValue = 0;
        algo.m_rcArrMaskingROI = nullptr;
        algo.m_nUsedInspPolygon = 0;
        algo.m_bConvetExceptROI = FALSE;
        algo.m_bUseAI = FALSE;
        algo.m_AIModelID = 0;
        algo.m_nMixCount = 0;

        auto& store = s_windowStore[wndIndex];
        store.algos.push_back(algo);
        auto as = std::make_unique<AlgoStorage>();
        // Allocate the matching native param struct so it's ready before any
        // MptiBridgeSetAlgoParamsXxx call. Unknown types still get a slot (type=-1) and
        // are skipped at commit-time wiring.
        switch (algoType)
        {
        case eAlgoBW:      as->Alloc<AlgoBW>(algoType); break;
        case eAlgoAlign:   as->Alloc<AlgoAlign>(algoType); break;
        case eAlgoPadBW:   as->Alloc<AlgoPadBW>(algoType); break;
        case eAlgoBlob:    as->Alloc<AlgoBlob>(algoType); break;
        case eAlgoBody_Blob: as->Alloc<AlgoBodyBlob>(algoType); break;
        case eAlgoNGBlob:  as->Alloc<AlgoNGBlob>(algoType); break;
        case eAlgoBGA:     as->Alloc<AlgoBGA>(algoType); break;
        case eAlgoEdge:    as->Alloc<AlgoEdge>(algoType); break;
        case eAlgoBodyEdge: as->Alloc<AlgoBodyEdge>(algoType); break;
        case eAlgoPattern: as->Alloc<AlgoPattern>(algoType); break;
        case eAlgoPatternDiff: as->Alloc<AlgoPatternDiff>(algoType); break;
        case eAlgoOCR:     as->Alloc<AlgoOCR>(algoType); break;
        case eAlgoPOCR:    as->Alloc<AlgoPOCR>(algoType); break;
        case eAlgoShapeX:  as->Alloc<AlgoShapeX>(algoType); break;
        // Add more algo types here as the WPF UI grows. Pattern:
        //   case eAlgoXxx: as->Alloc<AlgoXxx>(algoType); break;
        default:
            // Unknown algo type — leave params=nullptr; commit will skip m_ptrInspAlgoParam
            // and InspWindowAlgo3's param-null check will return -1 before InspProc.
            as->type = algoType;
            break;
        }
        BindLightStorage(store.algos.back(), *as);
        store.algoStore.push_back(std::move(as));
        return static_cast<int>(store.algos.size()) - 1;
    }
    catch (...) { return -100; }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoLight(
    int wndIndex, int algoIndex, const MptiBridgeFlowLightParams* params)
{
    try
    {
        if (s_committed)
            return -10;
        if (wndIndex < 0 || wndIndex >= static_cast<int>(s_windows.size()))
            return -11;
        if (params == nullptr)
            return -1;

        auto& store = s_windowStore[wndIndex];
        if (algoIndex < 0 || algoIndex >= static_cast<int>(store.algos.size()))
            return -12;
        if (algoIndex >= static_cast<int>(store.algoStore.size()) || store.algoStore[algoIndex] == nullptr)
            return -14;

        auto& algo = store.algos[algoIndex];
        auto& lightStore = *store.algoStore[algoIndex];
        BindLightStorage(algo, lightStore);

        algo.m_eLightType = static_cast<InspLightType>(ClampIntValue(params->lightType, Top_Light, ThreeD));
        algo.m_nRedValue = ClampLightChannel(params->redValue);
        algo.m_nGreenValue = ClampLightChannel(params->greenValue);
        algo.m_nBlueValue = ClampLightChannel(params->blueValue);
        algo.m_nWhiteValue = ClampLightChannel(params->whiteValue);

        const int lightCnt = ClampIntValue(params->lightCnt, 0, 10);
        algo.m_nLightCnt = lightCnt;
        for (int i = 0; i < 10; ++i)
        {
            if (i < lightCnt)
            {
                lightStore.lightRed[i] = ClampLightChannel(params->arrRedValue[i]);
                lightStore.lightGreen[i] = ClampLightChannel(params->arrGreenValue[i]);
                lightStore.lightBlue[i] = ClampLightChannel(params->arrBlueValue[i]);
                lightStore.lightWhite[i] = ClampLightChannel(params->arrWhiteValue[i]);
                lightStore.lightCalculation[i] = ClampIntValue(params->arrCalculation[i], 0, 2);
                lightStore.lightPosition[i] = ClampIntValue(params->arrLightPosition[i], Top_Light, Bottom_Light);
            }
            else
            {
                lightStore.lightRed[i] = 0;
                lightStore.lightGreen[i] = 0;
                lightStore.lightBlue[i] = 0;
                lightStore.lightWhite[i] = 0;
                lightStore.lightCalculation[i] = 0;
                lightStore.lightPosition[i] = Top_Light;
            }
        }

        return 0;
    }
    catch (...) { return -100; }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoParamsAlign(
    int wndIndex, int algoIndex, const MptiBridgeFlowAlignParams* params)
{
    try
    {
        if (s_committed)
            return -10;
        if (wndIndex < 0 || wndIndex >= static_cast<int>(s_windows.size()))
            return -11;
        if (params == nullptr)
            return -1;

        auto& store = s_windowStore[wndIndex];
        if (algoIndex < 0 || algoIndex >= static_cast<int>(store.algos.size()))
            return -12;
        if (store.algoStore[algoIndex]->type != eAlgoAlign)
            return -13;
        auto* algoPtr = static_cast<AlgoAlign*>(store.algoStore[algoIndex]->params);
        if (algoPtr == nullptr) return -14;

        AlgoAlign& a = *algoPtr;
        const int sn = params->searchNum < 1 ? 1 : (params->searchNum > 4 ? 4 : params->searchNum);
        const BridgeBinaryParams binary = {
            params->useInsp2D,
            params->minBinary,
            params->maxBinary,
            params->typeRange2D,
            params->invertCheck
        };
        ApplyCommonBinary(a, binary);
        a.m_nSearchNum = sn;
        a.m_nSearchMargin = params->searchMargin < 0 ? 0 : params->searchMargin;
        a.m_bInsp3D = params->useInsp3D != 0;
        a.m_dHeightRateMin = params->heightRateMin;
        a.m_dHeightRateMax = params->heightRateMax;
        a.m_dHeightAvg = params->heightAverage;
        a.m_nTypeRange3D = params->typeRange3D;
        a.m_bUseIPC = params->useIpc != 0;
        a.m_byIPCClass = static_cast<byte>(params->ipcClass < 0 ? 0 : (params->ipcClass > 2 ? 2 : params->ipcClass));
        const BridgeShiftParams shift = { params->useShift, params->maxShiftX, params->maxShiftY };
        ApplyShift(a, shift);
        a.m_bUseAngle = params->useAngle != 0;
        a.m_dAngle = params->maxAngle;
        a.m_bSameSize = params->sameSize != 0;
        a.m_nMinBlobArea = params->minBlobArea < 1 ? 1 : params->minBlobArea;
        a.m_bFillHole = params->fillHole != 0;
        a.m_byInspOPT = static_cast<byte>(params->inspOption & 0xFF);
        for (int i = 0; i < sn; ++i)
        {
            // tagAlgoAlign stores part-centered pixel coords (see MptiBridgeRunAlign).
            a.m_sArrSearchPoint[i].x = static_cast<float>(params->searchPointsX[i] - s_sourceWidth / 2.0);
            a.m_sArrSearchPoint[i].y = static_cast<float>(params->searchPointsY[i] - s_sourceHeight / 2.0);
            a.m_sArrSearchSize[i].cx = params->searchSizeW[i] < 1 ? 1 : params->searchSizeW[i];
            a.m_sArrSearchSize[i].cy = params->searchSizeH[i] < 1 ? 1 : params->searchSizeH[i];
        }
        return 0;
    }
    catch (...) { return -100; }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoParamsPadBW(
    int wndIndex, int algoIndex, const MptiBridgeFlowPadBWParams* params)
{
    try
    {
        if (s_committed)
            return -10;
        if (wndIndex < 0 || wndIndex >= static_cast<int>(s_windows.size()))
            return -11;
        if (params == nullptr)
            return -1;

        auto& store = s_windowStore[wndIndex];
        if (algoIndex < 0 || algoIndex >= static_cast<int>(store.algos.size()))
            return -12;
        if (store.algoStore[algoIndex]->type != eAlgoPadBW)
            return -13;
        auto* algoPtr = static_cast<AlgoPadBW*>(store.algoStore[algoIndex]->params);
        if (algoPtr == nullptr) return -14;

        AlgoPadBW& a = *algoPtr;
        // sDefaultPad holds the actual binarization params (Bin struct). Both this
        // struct and the per-light sArrInspPad[0] are used downstream depending on
        // m_nTotLightCnt — the default of 1 routes through sArrInspPad[0].stBin which
        // mirrors sDefaultPad, so we keep them in sync.
        const BridgeBinaryParams binary = {
            params->useInsp2D,
            params->binaryMin,
            params->binaryMax,
            eTypeRangeIn,
            params->invertCheck
        };
        ApplyCommonBinary(a.sDefaultPad, binary);
        a.sDefaultPad.m_bUseFillHole = params->useFillHole != 0;
        a.sArrInspPad[0].stBin     = a.sDefaultPad;
        a.m_nTotLightCnt           = 1;

        a.bTeachAreaUse     = params->useTeachArea != 0;
        a.dTeachArea        = params->teachArea;
        a.dTeachAreaRateMin = params->teachAreaRateMin;
        a.dTeachAreaRateMax = params->teachAreaRateMax;

        a.bUseShift         = params->useShift != 0;
        a.dTeachShiftX      = params->teachShiftX;
        a.dTeachShiftY      = params->teachShiftY;

        a.bUseBlobWidth     = params->useBlobWidth != 0;
        a.dBlobSizeWidth    = params->blobSizeWidth;
        a.bUseBlobLength    = params->useBlobLength != 0;
        a.dBlobSizeLength   = params->blobSizeLength;
        a.bUseBlobArea      = params->useBlobArea != 0;
        a.dBlobArea         = params->blobArea;

        a.nFilterLevel      = params->filterLevel;
        return 0;
    }
    catch (...) { return -100; }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoParamsBlob(
    int wndIndex, int algoIndex, const MptiBridgeFlowBlobParams* params)
{
    try
    {
        if (s_committed) return -10;
        if (wndIndex < 0 || wndIndex >= static_cast<int>(s_windows.size())) return -11;
        if (params == nullptr) return -1;
        auto& store = s_windowStore[wndIndex];
        if (algoIndex < 0 || algoIndex >= static_cast<int>(store.algos.size())) return -12;
        const BridgeBinaryParams binary = {
            params->useInsp2D,
            params->minBinary,
            params->maxBinary,
            params->typeRange2D,
            params->invertCheck
        };
        const BridgeAreaParams area = { params->useArea, params->areaMin, params->areaMax };
        const BridgeShiftParams shift = { params->useShift, params->shiftX, params->shiftY };

        switch (store.algoStore[algoIndex]->type)
        {
        case eAlgoBlob:
        {
            auto* algoPtr = static_cast<AlgoBlob*>(store.algoStore[algoIndex]->params);
            if (algoPtr == nullptr) return -14;

            AlgoBlob& a = *algoPtr;
            ApplyCommonBinary(a, binary);
            a.m_nTypeSelectBlob  = params->typeSelectBlob;
            a.m_bFillHole        = params->fillHole != 0;
            ApplyArea(a, area);
            ApplyShift(a, shift);
            a.m_bTeachWidthUse   = params->useTeachWidth != 0;
            a.m_dTeachWidth      = params->teachWidth;
            a.m_dTeachWidthRateMin = params->teachWidthRateMin;
            a.m_dTeachWidthRateMax = params->teachWidthRateMax;
            a.m_bTeachLengthUse  = params->useTeachLength != 0;
            a.m_dTeachLength     = params->teachLength;
            a.m_dTeachLengthRateMin = params->teachLengthRateMin;
            a.m_dTeachLengthRateMax = params->teachLengthRateMax;
            return 0;
        }
        case eAlgoBody_Blob:
        {
            auto* algoPtr = static_cast<AlgoBodyBlob*>(store.algoStore[algoIndex]->params);
            if (algoPtr == nullptr) return -14;

            AlgoBodyBlob& a = *algoPtr;
            ApplyCommonBinary(a, binary);
            a.m_nTypeSelectBlob  = params->typeSelectBlob;
            a.m_bFillHole        = params->fillHole != 0;
            ApplyArea(a, area);
            ApplyShift(a, shift);
            a.m_bShiftXUse       = params->useShift != 0;
            a.m_bShiftYUse       = params->useShift != 0;
            a.m_dTechCenterX     = params->shiftX;
            a.m_dTechCenterY     = params->shiftY;
            a.m_bTeachWidthUse   = params->useTeachWidth != 0;
            a.m_dTeachWidth      = params->teachWidth;
            a.m_dTeachWidthRateMin = params->teachWidthRateMin;
            a.m_dTeachWidthRateMax = params->teachWidthRateMax;
            a.m_bTeachLengthUse  = params->useTeachLength != 0;
            a.m_dTeachLength     = params->teachLength;
            a.m_dTeachLengthRateMin = params->teachLengthRateMin;
            a.m_dTeachLengthRateMax = params->teachLengthRateMax;
            return 0;
        }
        case eAlgoNGBlob:
        {
            auto* algoPtr = static_cast<AlgoNGBlob*>(store.algoStore[algoIndex]->params);
            if (algoPtr == nullptr) return -14;

            AlgoNGBlob& a = *algoPtr;
            int flags = (params->useInsp2D != 0) ? NGBlob_Bin2D : 0;
            if (params->useArea != 0) flags |= NGBlob_Area;
            if (params->useTeachWidth != 0) flags |= NGBlob_Width;
            if (params->useTeachLength != 0) flags |= NGBlob_Length;
            if (params->fillHole != 0) flags |= NGBlob_Fillhole;
            a.narrdata[NGBlob_by_Data] = static_cast<byte>(flags & 0xFF);
            a.narrdata[NGBlob_by_Min2D] = static_cast<byte>(ClampIntValue(params->minBinary, 0, 255));
            a.narrdata[NGBlob_by_Max2D] = static_cast<byte>(ClampIntValue(params->maxBinary, 0, 255));
            a.narrdata[NGBlob_by_Range2D] = static_cast<byte>(params->typeRange2D);
            a.farrdata[NGBlob_F_Area] = static_cast<float>(params->areaMax > 0 ? params->areaMax : params->areaMin);
            a.farrdata[NGBlob_F_Width] = static_cast<float>(params->teachWidth);
            a.farrdata[NGBlob_F_Length] = static_cast<float>(params->teachLength);
            return 0;
        }
        default:
            return -13;
        }
    }
    catch (...) { return -100; }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoParamsBGA(
    int wndIndex, int algoIndex, const MptiBridgeFlowBGAParams* params)
{
    try
    {
        if (s_committed) return -10;
        if (wndIndex < 0 || wndIndex >= static_cast<int>(s_windows.size())) return -11;
        if (params == nullptr) return -1;
        auto& store = s_windowStore[wndIndex];
        if (algoIndex < 0 || algoIndex >= static_cast<int>(store.algos.size())) return -12;
        if (store.algoStore[algoIndex]->type != eAlgoBGA) return -13;
        auto* algoPtr = static_cast<AlgoBGA*>(store.algoStore[algoIndex]->params);
        if (algoPtr == nullptr) return -14;

        AlgoBGA& a = *algoPtr;
        const BridgeBinaryParams binary = {
            params->useInsp2D,
            params->minBinary,
            params->maxBinary,
            params->typeRange2D,
            params->invertCheck
        };
        ApplyCommonBinary(a, binary);
        a.m_nTypeSelectBlob  = params->typeSelectBlob;
        a.m_bFillHole        = params->fillHole != 0;
        const BridgeAreaParams area = { params->useArea, params->areaMin, params->areaMax };
        ApplyArea(a, area);
        a.m_dTeachArea       = params->teachArea;
        a.m_dTeachVolume     = params->teachVolume;
        const BridgeShiftParams shift = { params->useShift, params->shiftX, params->shiftY };
        ApplyShift(a, shift);
        a.m_bTeachWidthUse   = params->useTeachWidth != 0;
        a.m_dTeachWidth      = params->teachWidth;
        a.m_dTeachWidthRateMin = params->teachWidthRateMin;
        a.m_dTeachWidthRateMax = params->teachWidthRateMax;
        a.m_bTeachLengthUse  = params->useTeachLength != 0;
        a.m_dTeachLength     = params->teachLength;
        a.m_dTeachLengthRateMin = params->teachLengthRateMin;
        a.m_dTeachLengthRateMax = params->teachLengthRateMax;
        a.m_bUseCircleRate   = params->useCircleRate != 0;
        a.m_dTeachCircleRate = params->teachCircleRate;
        return 0;
    }
    catch (...) { return -100; }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoParamsEdge(
    int wndIndex, int algoIndex, const MptiBridgeFlowEdgeParams* params)
{
    try
    {
        if (s_committed) return -10;
        if (wndIndex < 0 || wndIndex >= static_cast<int>(s_windows.size())) return -11;
        if (params == nullptr) return -1;
        auto& store = s_windowStore[wndIndex];
        if (algoIndex < 0 || algoIndex >= static_cast<int>(store.algos.size())) return -12;
        const BridgeBinaryParams binary = {
            params->useInsp2D,
            params->minBinary,
            params->maxBinary,
            params->typeRange2D,
            params->invertCheck
        };
        const BridgeShiftParams shift = { params->useShift, params->shiftX, params->shiftY };
        const BridgeAreaParams area = { params->useArea, params->areaMin, params->areaMax };

        switch (store.algoStore[algoIndex]->type)
        {
        case eAlgoEdge:
        {
            auto* algoPtr = static_cast<AlgoEdge*>(store.algoStore[algoIndex]->params);
            if (algoPtr == nullptr) return -14;

            AlgoEdge& a = *algoPtr;
            ApplyCommonBinary(a, binary);
            ApplyShift(a, shift);
            ApplyArea(a, area);
            // EdgeLineTotalCnt cap (defined in AlgoBase). Clamp to keep
            // m_bArrIsHorizon[] index safe.
            const int lineCntMax = sizeof(a.m_bArrIsHorizon) / sizeof(a.m_bArrIsHorizon[0]);
            a.m_nSetLineCnt      = (params->setLineCnt < 1) ? 1 :
                                    (params->setLineCnt > lineCntMax ? lineCntMax : params->setLineCnt);
            a.m_bGroup           = params->useGroup != 0;
            a.m_nLineFindType    = params->lineFindType;
            a.m_dLineFindRate    = params->lineFindRate;
            a.m_bUseAngle        = params->useAngle != 0;
            a.m_dTeachRotate     = params->teachRotate;
            a.m_bDistanceX       = params->useDistanceX != 0;
            a.m_dTeachDistanceX  = params->teachDistanceX;
            a.m_bDistanceY       = params->useDistanceY != 0;
            a.m_dTeachDistanceY  = params->teachDistanceY;
            return 0;
        }
        case eAlgoBodyEdge:
        {
            auto* algoPtr = static_cast<AlgoBodyEdge*>(store.algoStore[algoIndex]->params);
            if (algoPtr == nullptr) return -14;

            AlgoBodyEdge& a = *algoPtr;
            int flags = (params->useInsp2D != 0) ? BodyEdge_Data_Bin2D : 0;
            if (params->useShift != 0)
                flags |= BodyEdge_Data_UseShift | BodyEdge_Data_UseShiftX | BodyEdge_Data_UseShiftY;
            if (params->useAngle != 0)
                flags |= BodyEdge_Data_UseAngle;
            if (params->useDistanceX != 0 || params->useDistanceY != 0)
                flags |= BodyEdge_Data_UseDistance;
            a.narrdata[BodyEdge_N_Data] = flags;
            a.narrdata[BodyEdge_N_Min2D] = ClampIntValue(params->minBinary, 0, 255);
            a.narrdata[BodyEdge_N_Max2D] = ClampIntValue(params->maxBinary, 0, 255);
            a.narrdata[BodyEdge_N_Range2D] = params->typeRange2D;
            a.narrdata[BodyEdge_N_AreaNum] = (params->setLineCnt < 1) ? 1 :
                                             (params->setLineCnt > BODY_EDGE_RECT_CNTS ? BODY_EDGE_RECT_CNTS : params->setLineCnt);
            a.farrdata[BodyEdge_F_ShiftX] = static_cast<float>(params->shiftX);
            a.farrdata[BodyEdge_F_ShiftY] = static_cast<float>(params->shiftY);
            a.farrdata[BodyEdge_F_Angle] = static_cast<float>(params->teachRotate);
            a.farrdata[BodyEdge_F_StandardAngle] = static_cast<float>(params->teachRotate);
            a.farrdata[BodyEdge_F_TeachDistanceX] = static_cast<float>(params->teachDistanceX);
            a.farrdata[BodyEdge_F_TeachDistanceY] = static_cast<float>(params->teachDistanceY);
            return 0;
        }
        default:
            return -13;
        }
    }
    catch (...) { return -100; }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoParamsPattern(
    int wndIndex, int algoIndex, const MptiBridgeFlowPatternParams* params)
{
    try
    {
        if (s_committed) return -10;
        if (wndIndex < 0 || wndIndex >= static_cast<int>(s_windows.size())) return -11;
        if (params == nullptr) return -1;
        auto& store = s_windowStore[wndIndex];
        if (algoIndex < 0 || algoIndex >= static_cast<int>(store.algos.size())) return -12;

        switch (store.algoStore[algoIndex]->type)
        {
        case eAlgoPattern:
        {
            auto* algoPtr = static_cast<AlgoPattern*>(store.algoStore[algoIndex]->params);
            if (algoPtr == nullptr) return -14;

            AlgoPattern& a = *algoPtr;
            a.m_bUsePolarity            = params->usePolarity != 0;
            a.m_dAcceptScore            = params->acceptScore;
            a.m_bShiftIsUse             = params->useShift != 0;
            a.m_dShiftX                 = params->shiftX;
            a.m_dShiftY                 = params->shiftY;
            a.m_dRangeAngle             = params->rangeAngle;
            a.m_dWndAngle               = params->wndAngle;
            a.m_dSearchAngleRange_Min   = params->searchAngleRangeMin;
            a.m_dSearchAngleRange_Max   = params->searchAngleRangeMax;
            a.m_SamplingAngle           = params->samplingAngle;
            a.m_bUseNGOpt               = params->useNgOpt != 0;
            a.m_bUseCharacter           = params->useCharacter != 0;
            a.m_nModelFilter            = params->modelFilter;
            a.m_nCntPatternPath         = params->cntPatternPath < 1 ? 1 : params->cntPatternPath;
            a.m_factor_red              = params->factorRed;
            a.m_factor_green            = params->factorGreen;
            a.m_factor_blue             = params->factorBlue;

            // Copy model paths only when non-empty (first wchar != 0). 260 == MAX_STRLEN.
            if (params->modelPathInspect1[0] != L'\0')
                wcsncpy_s(a.m_sArrPathModelInspect1, MAX_STRLEN, params->modelPathInspect1, _TRUNCATE);
            if (params->modelPathTeach[0] != L'\0')
                wcsncpy_s(a.m_sPathModelTeach, MAX_STRLEN, params->modelPathTeach, _TRUNCATE);
            return 0;
        }
        case eAlgoPatternDiff:
        {
            auto* algoPtr = static_cast<AlgoPatternDiff*>(store.algoStore[algoIndex]->params);
            if (algoPtr == nullptr) return -14;

            AlgoPatternDiff& a = *algoPtr;
            a.m_bUsePattern = params->modelPathInspect1[0] != L'\0' || params->modelPathTeach[0] != L'\0';
            a.m_bPatternMatching = TRUE;
            a.m_nModelAddCnt = params->cntPatternPath < 1 ? 1 : params->cntPatternPath;
            a.m_dTheta = params->wndAngle;
            a.m_nAcceptAlignScore = ScoreToPercent(params->acceptScore);
            if (params->modelPathInspect1[0] != L'\0')
            {
                wcsncpy_s(a.m_sModelPath, MAX_STRLEN, params->modelPathInspect1, _TRUNCATE);
                wcsncpy_s(a.m_sModelPath1, MAX_STRLEN, params->modelPathInspect1, _TRUNCATE);
            }
            if (params->modelPathTeach[0] != L'\0')
                wcsncpy_s(a.m_sModelPath, MAX_STRLEN, params->modelPathTeach, _TRUNCATE);
            return 0;
        }
        case eAlgoOCR:
        {
            auto* algoPtr = static_cast<AlgoOCR*>(store.algoStore[algoIndex]->params);
            if (algoPtr == nullptr) return -14;

            AlgoOCR& a = *algoPtr;
            a.m_bUsePolarity = params->usePolarity != 0;
            a.m_dStdCharScore = ScoreToPercent(params->acceptScore);
            a.m_bUseCharScore = TRUE;
            a.m_dWndAngle = params->wndAngle;
            a.m_nThreshVal = 128;
            if (params->modelPathTeach[0] != L'\0')
                wcsncpy_s(a.m_sTargetFont, MAX_STRLEN, params->modelPathTeach, _TRUNCATE);
            if (params->modelPathInspect1[0] != L'\0')
                wcsncpy_s(a.m_sFontPath, MAX_STRLEN, params->modelPathInspect1, _TRUNCATE);
            return 0;
        }
        case eAlgoPOCR:
        {
            auto* algoPtr = static_cast<AlgoPOCR*>(store.algoStore[algoIndex]->params);
            if (algoPtr == nullptr) return -14;

            AlgoPOCR& a = *algoPtr;
            a.m_bUsePolarity = params->usePolarity != 0;
            a.m_dStdCharScore[0] = ScoreToPercent(params->acceptScore);
            a.m_dWndAngle = params->wndAngle;
            a.m_nThreshVal = 128;
            if (params->modelPathTeach[0] != L'\0')
                wcsncpy_s(a.m_sTargetFont, MAX_STRLEN, params->modelPathTeach, _TRUNCATE);
            if (params->modelPathInspect1[0] != L'\0')
                wcsncpy_s(a.m_sFontPath, MAX_STRLEN, params->modelPathInspect1, _TRUNCATE);
            return 0;
        }
        default:
            return -13;
        }
    }
    catch (...) { return -100; }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoParamsShapeX(
    int wndIndex, int algoIndex, const MptiBridgeFlowShapeXParams* params)
{
    try
    {
        if (s_committed) return -10;
        if (wndIndex < 0 || wndIndex >= static_cast<int>(s_windows.size())) return -11;
        if (params == nullptr) return -1;
        auto& store = s_windowStore[wndIndex];
        if (algoIndex < 0 || algoIndex >= static_cast<int>(store.algos.size())) return -12;
        if (store.algoStore[algoIndex]->type != eAlgoShapeX) return -13;
        auto* algoPtr = static_cast<AlgoShapeX*>(store.algoStore[algoIndex]->params);
        if (algoPtr == nullptr) return -14;

        AlgoShapeX& a = *algoPtr;
        a.dPartangle          = params->partAngle;
        a.m_nLeadTipDirection = params->leadTipDirection;
        // Clamp ROICnt to nShapeXCnt (200) — per-ROI setter would be a separate API.
        a.ROICnt              = (params->roiCnt < 0) ? 0 :
                                (params->roiCnt > nShapeXCnt ? nShapeXCnt : params->roiCnt);
        a.MatchSc             = params->matchScore;
        a.nHist1              = params->hist1;
        a.nHistLow1           = params->hist1Low;
        a.nHistUp1            = params->hist1Up;
        a.nHist2              = params->hist2;
        a.nHistLow2           = params->hist2Low;
        a.nHistUp2            = params->hist2Up;
        a.nInspOption         = params->inspOption;
        a.Aspectratio         = params->aspectRatio;
        a.MinScarThickness    = params->minScarThickness;
        a.Verticalmaxlength   = params->verticalMaxLength;
        a.Horizontalmaxlength = params->horizontalMaxLength;
        a.MaxNGArea           = params->maxNgArea;
        a.CriticalArea        = params->criticalArea;
        a.MaxChippingLength   = params->maxChippingLength;
        a.nStartIdx           = params->startIdx;
        a.fTieBarRate         = params->tieBarRate;
        a.ContrastValue       = params->contrastValue;
        return 0;
    }
    catch (...) { return -100; }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeCommitInspParam(wchar_t* message, int messageLength)
{
    try
    {
        if (g_pMPTI == nullptr)
            return Fail(message, messageLength, -200, L"MptiBridgeCommitInspParam: MPTI not initialized (call a load first)");
        if (s_windows.empty())
            return Fail(message, messageLength, -1, L"MptiBridgeCommitInspParam: no windows added");

        // Wire algorithm arrays + algo-param pointers now that all adds are done. Any
        // algo that didn't get a typed allocation (Alloc<T>) ends up with params=nullptr
        // and is left out — InspNormal_Ver2's null-param check at line 1313 will return
        // -1 before InspProc, surfacing the missing setter as an obvious failure.
        for (size_t w = 0; w < s_windows.size(); ++w)
        {
            auto& store = s_windowStore[w];
            for (size_t a = 0; a < store.algos.size(); ++a)
            {
                store.algos[a].m_ptrInspAlgoParam = store.algoStore[a]->params;
            }
            s_windows[w].nAlgorithmCnt = static_cast<int>(store.algos.size());
            s_windows[w].vArrAlgoParam = store.algos.empty() ? nullptr : store.algos.data();
        }
        s_part.pWindows = s_windows.data();
        s_part.nWindowCount = static_cast<int>(s_windows.size());

        // MPTI_GetPttFileLoad (the LoadPtt path) fills m_parrPartBuf but not the raw-data
        // size members; only MPTI_SetRawDataFovInfo (which needs a .pot) does. Backfill
        // them from the part dims so MPTI_SetInspParam reports a valid partImgBuf size.
        if (g_pMPTI->m_nSizeXRawData <= 0 && s_sourceWidth > 0)
        {
            g_pMPTI->m_nSizeXRawData = s_sourceWidth;
            g_pMPTI->m_nSizeYRawData = s_sourceHeight;
        }

        // Set FOV resolution if MPTI_SetRawDataFovInfo never ran (minimal flow without
        // .pot). PInspAlgoWrapper::WndSizeChange divides partWidth by
        // PIAL::PInspAlgo_Lib::m_resolX, so 0.0 causes wnd_w to become 0 ->
        // InspWindowAlgo3 returns e_NG before the algo body runs (line 8790 of
        // InspManager.cpp).
        //
        // Resolution priority:
        //   1. s_explicitResolX/Y    (set via MptiBridgeSetFlowResolution, normally
        //                             populated from the .pot file or Part Import metadata)
        //   2. fallback 1.0          (treats coords as already in pixel-space — only
        //                             correct when no real-world units are required)
        //
        // We set the fields directly instead of calling InspManager::SetResolution()
        // because that triggers MIL re-init (m_procMil = new CProcMil + InitMil...)
        // which AVs in a minimal flow where MIL device wasn't fully initialized.
        if (g_pInspMng && (!g_pInspMng->m_bSetResolution || g_pInspMng->m_resolX <= 0.0))
        {
            const int fovW = s_sourceWidth > 0 ? s_sourceWidth : g_pMPTI->m_nSizeXRawData;
            const int fovH = s_sourceHeight > 0 ? s_sourceHeight : g_pMPTI->m_nSizeYRawData;
            const double rx = s_explicitResolX > 0.0 ? s_explicitResolX : 1.0;
            const double ry = s_explicitResolY > 0.0 ? s_explicitResolY : 1.0;
            g_pInspMng->m_fovWidth = fovW;
            g_pInspMng->m_fovLength = fovH;
            g_pInspMng->m_resolX = rx;
            g_pInspMng->m_resolY = ry;
            g_pInspMng->m_bSetResolution = true;
            PIAL::PInspAlgo_Lib::m_resolX = rx;
            PIAL::PInspAlgo_Lib::m_resolY = ry;
        }

        const int ret = MPTI_SetInspParam(&s_part, s_windows.data(), static_cast<int>(s_windows.size()));
        if (ret != 1)
            return Fail(message, messageLength, ret == 0 ? -201 : ret, L"MPTI_SetInspParam returned failure");

        s_committed = true;
        s_inspected = false;
        return WriteMsg(message, messageLength, L"MptiBridgeCommitInspParam: MPTI_SetInspParam ok");
    }
    catch (const std::exception&)
    {
        return Fail(message, messageLength, -100, L"MptiBridgeCommitInspParam: C++ exception");
    }
    catch (...)
    {
        return Fail(message, messageLength, -101, L"MptiBridgeCommitInspParam: unknown C++ exception");
    }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeInspProc(wchar_t* message, int messageLength)
{
    try
    {
        if (!s_committed)
            return Fail(message, messageLength, -10, L"MptiBridgeInspProc: call MptiBridgeCommitInspParam first");

        const int ret = CallMptiInspProcGuarded();
        if (ret == -300 && s_lastInspProcSehCode != 0)
        {
            s_inspected = false;
            wchar_t buf[1024];
            // Extract bare module file name from full path for readability.
            const wchar_t* modName = s_lastInspProcSehModule;
            for (const wchar_t* p = s_lastInspProcSehModule; *p; ++p)
                if (*p == L'\\' || *p == L'/') modName = p + 1;
            swprintf_s(buf,
                L"MPTI_InspProc SEH 0x%08X @ 0x%p in %s",
                s_lastInspProcSehCode,
                s_lastInspProcSehAddress,
                modName && *modName ? modName : L"<unknown>");
            return Fail(message, messageLength, ret, buf);
        }

        s_inspected = true;
        wchar_t buf[256];
        swprintf_s(buf, L"MPTI_InspProc returned %d", ret);
        WriteMsg(message, messageLength, buf);
        return ret;
    }
    catch (const std::exception&)
    {
        return Fail(message, messageLength, -100, L"MptiBridgeInspProc: C++ exception");
    }
    catch (...)
    {
        return Fail(message, messageLength, -101, L"MptiBridgeInspProc: unknown C++ exception");
    }
}

// Diagnostics: returns 0 on success and fills whatever pointers are non-null.
MPTI_BRIDGE_FLOW_API int MptiBridgeDebugInfo(
    int* rawSizeX, int* rawSizeY,
    int* partImgSizeX, int* partImgSizeY, int* partImgTopRNull,
    int* alignItemCnt, int* alignGroupCnt)
{
    if (rawSizeX)        *rawSizeX = g_pMPTI ? g_pMPTI->m_nSizeXRawData : -1;
    if (rawSizeY)        *rawSizeY = g_pMPTI ? g_pMPTI->m_nSizeYRawData : -1;
    InspPartInfo* bi = g_pInspMng ? g_pInspMng->GetInspPartInfo() : nullptr;
    if (bi)
    {
        if (partImgSizeX)    *partImgSizeX = bi->partImgBuf.nImageSizeX;
        if (partImgSizeY)    *partImgSizeY = bi->partImgBuf.nImageSizeY;
        if (partImgTopRNull) *partImgTopRNull = bi->partImgBuf.imgTop_R == nullptr ? 1 : 0;
    }
    else
    {
        if (partImgSizeX)    *partImgSizeX = -1;
        if (partImgSizeY)    *partImgSizeY = -1;
        if (partImgTopRNull) *partImgTopRNull = -1;
    }
    if (g_pInspMng && g_pInspMng->m_inspItemCnts)
    {
        if (alignItemCnt)  *alignItemCnt = g_pInspMng->m_inspItemCnts[eINSP_ALIGN];
    }
    else if (alignItemCnt) *alignItemCnt = -1;
    if (g_pInspMng && g_pInspMng->m_groupIndexCnts)
    {
        if (alignGroupCnt) *alignGroupCnt = g_pInspMng->m_groupIndexCnts[eINSP_ALIGN];
    }
    else if (alignGroupCnt) *alignGroupCnt = -1;
    return 0;
}

MPTI_BRIDGE_FLOW_API int MptiBridgeResultAlignCount()
{
    if (!s_inspected || g_pInspMng == nullptr || g_pInspMng->m_inspectionResult == nullptr)
        return 0;
    const InspectionResult* r = g_pInspMng->m_inspectionResult;
    if (!r->isInspAlign || r->alignResult == nullptr)
        return 0;
    return r->alignArraySize;
}

MPTI_BRIDGE_FLOW_API int MptiBridgeResultAlign(int i, MptiBridgeFlowAlignResult* out)
{
    try
    {
        if (out == nullptr)
            return -1;
        memset(out, 0, sizeof(*out));
        if (!s_inspected || g_pInspMng == nullptr || g_pInspMng->m_inspectionResult == nullptr)
            return -2;
        const InspectionResult* r = g_pInspMng->m_inspectionResult;
        if (!r->isInspAlign || r->alignResult == nullptr || i < 0 || i >= r->alignArraySize)
            return -3;

        const InspWndResult& wnd = r->alignResult[i];
        out->isOk = wnd.m_bOk ? 1 : 0;
        out->defectCode = wnd.m_nDefectCode;
        if (wnd.m_vArrRstInspAlgo == nullptr || wnd.m_nAlgorithmCnt < 1)
            return 0;

        const InspAlgoResult& ar = wnd.m_vArrRstInspAlgo[0];
        out->isInsp = ar.m_bIsInsp ? 1 : 0;
        if (ar.m_vRstInspAlgo == nullptr)
            return 0;

        const RstAlgoAlign* a = static_cast<const RstAlgoAlign*>(ar.m_vRstInspAlgo);
        out->okCount = a->m_nOKAreaCnt;
        out->offsetX = a->m_dOffset_x;
        out->offsetY = a->m_dOffset_y;
        out->theta = a->m_dTheta;
        out->okShiftX = a->m_bOKShiftX ? 1 : 0;
        out->okShiftY = a->m_bOKShiftY ? 1 : 0;
        out->okAngle = a->m_bOKAngle ? 1 : 0;
        for (int k = 0; k < 4; ++k)
        {
            out->detectedCentersX[k] = (a->m_rcRect_I[k].left + a->m_rcRect_I[k].right) / 2;
            out->detectedCentersY[k] = (a->m_rcRect_I[k].top + a->m_rcRect_I[k].bottom) / 2;
        }
        return 0;
    }
    catch (...) { return -100; }
}

namespace
{
    // Returns the (resultArray, arraySize) pair for the given inspection type. The
    // InspectionResult struct stores per-type window-result arrays; non-Align algorithms
    // live in mountResult / padResult / BGAResult / etc. based on their parent window's
    // inspType. Returns (nullptr, 0) for types we don't currently expose readers for.
    void GetWndResultArray(int wndType, InspWndResult** outArr, int* outSize)
    {
        *outArr = nullptr; *outSize = 0;
        if (g_pInspMng == nullptr || g_pInspMng->m_inspectionResult == nullptr) return;
        InspectionResult* r = g_pInspMng->m_inspectionResult;
        switch (wndType)
        {
        case eINSP_MOUNT:      *outArr = r->mountResult;       *outSize = r->mountArraySize;      break;
        case eINSP_ALIGN:      *outArr = r->alignResult;       *outSize = r->alignArraySize;      break;
        case eINSP_OCR:        *outArr = r->ocrResult;         *outSize = r->ocrArraySize;        break;
        case eINSP_LEADSOLDER: *outArr = r->leadSolderResult;  *outSize = r->leadSolderArraySize; break;
        case eINSP_SOLDER:     *outArr = r->solderResult;      *outSize = r->solderArraySize;     break;
        case eINSP_TAB:        *outArr = r->tabResult;         *outSize = r->tabArraySize;        break;
        case eINSP_S_BALL:     *outArr = r->S_BallResult;      *outSize = r->S_BallArraySize;     break;
        case eINSP_PAD:        *outArr = r->PadResult;         *outSize = r->PadArraySize;        break;
        case eINSP_BGA:        *outArr = r->BGAResult;         *outSize = r->BGAArraySize;        break;
        default: break;
        }
    }

    // Resolves (wndType, wndIdx, algoIdx) -> InspAlgoResult* and fills the common
    // header on `hdr`. Returns 0 on success. Negative codes:
    //   -1 inspection result missing  -2 wndType not supported / array null
    //   -3 wndIdx OOB                  -4 algoIdx OOB or null
    //   -6 m_vRstInspAlgo is null (algorithm didn't actually run)
    int ResolveAlgoResultAny(int wndType, int wndIdx, int algoIdx,
                             MptiBridgeFlowWndAlgoHeader* hdr, InspAlgoResult** outAr)
    {
        *outAr = nullptr;
        if (!s_inspected) return -1;
        InspWndResult* arr = nullptr; int sz = 0;
        GetWndResultArray(wndType, &arr, &sz);
        if (arr == nullptr) return -2;
        if (wndIdx < 0 || wndIdx >= sz) return -3;

        const InspWndResult& wnd = arr[wndIdx];
        if (wnd.m_vArrRstInspAlgo == nullptr || algoIdx < 0 || algoIdx >= wnd.m_nAlgorithmCnt)
            return -4;
        InspAlgoResult& ar = wnd.m_vArrRstInspAlgo[algoIdx];

        if (hdr)
        {
            hdr->wndIsInsp     = wnd.m_bIsInsp ? 1 : 0;
            hdr->wndIsOk       = wnd.m_bOk ? 1 : 0;
            hdr->wndDefectCode = wnd.m_nDefectCode;
            hdr->algoIsInsp    = ar.m_bIsInsp ? 1 : 0;
            hdr->algoIsOk      = ar.m_bOk ? 1 : 0;
            hdr->algoIsRequired = ar.m_bIsRequired ? 1 : 0;
            hdr->algoDefectCode = ar.m_nDefectCode;
            hdr->algoAlgoType   = static_cast<int>(ar.m_nAlgoType);
        }
        *outAr = &ar;
        if (ar.m_vRstInspAlgo == nullptr) return -6;
        return 0;
    }

    // Same as ResolveAlgoResultAny, with an exact type guard for readers that have
    // not been generalized yet. Returns -5 for an unexpected algo type.
    int ResolveAlgoResult(int wndType, int wndIdx, int algoIdx, int expectedAlgoType,
                          MptiBridgeFlowWndAlgoHeader* hdr, InspAlgoResult** outAr)
    {
        int rc = ResolveAlgoResultAny(wndType, wndIdx, algoIdx, hdr, outAr);
        if (rc != 0) return rc;
        if (outAr == nullptr || *outAr == nullptr) return -4;
        if (static_cast<int>((*outAr)->m_nAlgoType) != expectedAlgoType) return -5;
        return 0;
    }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeResultBlob(
    int wndType, int wndIdx, int algoIdx, MptiBridgeFlowBlobResult* out)
{
    try
    {
        if (out == nullptr) return -1;
        memset(out, 0, sizeof(*out));
        InspAlgoResult* ar = nullptr;
        int rc = ResolveAlgoResultAny(wndType, wndIdx, algoIdx, &out->hdr, &ar);
        if (rc != 0) return rc;
        switch (ar->m_nAlgoType)
        {
        case eAlgoBlob:
        {
            const RstAlgoBlob* r = static_cast<const RstAlgoBlob*>(ar->m_vRstInspAlgo);
            out->rstArea       = r->m_dRstArea;
            out->rstAreaRate   = r->m_dRstAreaRate;
            out->rstShiftX     = r->m_dRstShiftX;
            out->rstShiftY     = r->m_dRstShiftY;
            out->rstWidth      = r->m_dRstWidth;
            out->rstLength     = r->m_dRstLength;
            out->rstHeightMean = r->m_dRstHeightMean;
            out->okArea        = r->m_bOKArea   ? 1 : 0;
            out->okShiftX      = r->m_bOKShiftX ? 1 : 0;
            out->okShiftY      = r->m_bOKShiftY ? 1 : 0;
            out->okWidth       = r->m_bOKWidth  ? 1 : 0;
            out->okLength      = r->m_bOKLength ? 1 : 0;
            out->okHeight      = r->m_bOKHeight ? 1 : 0;
            out->rectLeft      = r->m_rcRect_I.left;
            out->rectTop       = r->m_rcRect_I.top;
            out->rectRight     = r->m_rcRect_I.right;
            out->rectBottom    = r->m_rcRect_I.bottom;
            out->arrRectCnt    = r->m_nArrRectCnt;
            return 0;
        }
        case eAlgoBody_Blob:
        {
            const RstAlgoBodyBlob* r = static_cast<const RstAlgoBodyBlob*>(ar->m_vRstInspAlgo);
            out->rstArea       = r->m_dRstArea;
            out->rstAreaRate   = r->m_dRstAreaRate;
            out->rstShiftX     = r->m_dRstShiftX;
            out->rstShiftY     = r->m_dRstShiftY;
            out->rstWidth      = r->m_dRstWidth;
            out->rstLength     = r->m_dRstLength;
            out->rstHeightMean = r->m_dRstHeightMean;
            out->okArea        = r->m_bOKArea   ? 1 : 0;
            out->okShiftX      = r->m_bOKShiftX ? 1 : 0;
            out->okShiftY      = r->m_bOKShiftY ? 1 : 0;
            out->okWidth       = r->m_bOKWidth  ? 1 : 0;
            out->okLength      = r->m_bOKLength ? 1 : 0;
            out->okHeight      = r->m_bOKHeight ? 1 : 0;
            out->rectLeft      = r->m_rcBodyRect.left;
            out->rectTop       = r->m_rcBodyRect.top;
            out->rectRight     = r->m_rcBodyRect.right;
            out->rectBottom    = r->m_rcBodyRect.bottom;
            out->arrRectCnt    = (r->m_rcBodyRect.right > r->m_rcBodyRect.left &&
                                  r->m_rcBodyRect.bottom > r->m_rcBodyRect.top) ? 1 : 0;
            return 0;
        }
        case eAlgoNGBlob:
        {
            const RstAlgoNGBlob* r = static_cast<const RstAlgoNGBlob*>(ar->m_vRstInspAlgo);
            const int first = r->blob_count > 0 ? 0 : -1;
            out->rstArea       = first >= 0 ? r->fRstArea[first] : 0.0;
            out->rstAreaRate   = 0.0;
            out->rstShiftX     = 0.0;
            out->rstShiftY     = 0.0;
            out->rstWidth      = first >= 0 ? r->fRstWidth[first] : 0.0;
            out->rstLength     = first >= 0 ? r->fRstLength[first] : 0.0;
            out->rstHeightMean = first >= 0 ? r->fRstHeight[first] : 0.0;
            out->okArea        = r->bIsOKArea   ? 1 : 0;
            out->okShiftX      = 1;
            out->okShiftY      = 1;
            out->okWidth       = r->bIsOKWidth  ? 1 : 0;
            out->okLength      = r->bIsOKLength ? 1 : 0;
            out->okHeight      = r->bIsOKHeight ? 1 : 0;
            const RECT& rc     = first >= 0 ? r->m_rcArrRect[first] : r->m_BodyRect;
            out->rectLeft      = rc.left;
            out->rectTop       = rc.top;
            out->rectRight     = rc.right;
            out->rectBottom    = rc.bottom;
            out->arrRectCnt    = r->blob_count;
            return 0;
        }
        default:
            return -5;
        }
    }
    catch (...) { return -100; }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeResultBGA(
    int wndType, int wndIdx, int algoIdx, MptiBridgeFlowBGAResult* out)
{
    try
    {
        if (out == nullptr) return -1;
        memset(out, 0, sizeof(*out));
        InspAlgoResult* ar = nullptr;
        int rc = ResolveAlgoResult(wndType, wndIdx, algoIdx, eAlgoBGA, &out->hdr, &ar);
        if (rc != 0) return rc;
        const RstAlgoBGA* r = static_cast<const RstAlgoBGA*>(ar->m_vRstInspAlgo);
        out->okCoplanarity = r->m_bCoplanarityOK ? 1 : 0;
        out->okGridOffsetX = r->m_bOKGridOffsetX ? 1 : 0;
        out->okGridOffsetY = r->m_bOKGridOffsetY ? 1 : 0;
        out->okTwist       = r->m_bOKTwist       ? 1 : 0;
        out->coplanarity   = r->m_fCoplanarity;
        out->gridOffsetX   = r->m_fRstGridOffsetX;
        out->gridOffsetY   = r->m_fRstGridOffsetY;
        out->twist         = r->m_fRstTwist;
        out->rectLeft      = r->m_rcRect_I.left;
        out->rectTop       = r->m_rcRect_I.top;
        out->rectRight     = r->m_rcRect_I.right;
        out->rectBottom    = r->m_rcRect_I.bottom;
        return 0;
    }
    catch (...) { return -100; }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeResultEdge(
    int wndType, int wndIdx, int algoIdx, MptiBridgeFlowEdgeResult* out)
{
    try
    {
        if (out == nullptr) return -1;
        memset(out, 0, sizeof(*out));
        InspAlgoResult* ar = nullptr;
        int rc = ResolveAlgoResultAny(wndType, wndIdx, algoIdx, &out->hdr, &ar);
        if (rc != 0) return rc;
        switch (ar->m_nAlgoType)
        {
        case eAlgoEdge:
        {
            const RstAlgoEdge* r = static_cast<const RstAlgoEdge*>(ar->m_vRstInspAlgo);
            out->rstShiftX    = r->m_dRstShiftX;
            out->rstShiftY    = r->m_dRstShiftY;
            out->rstRealAngle = r->m_dRstRealAngle;
            out->rstAngle     = r->m_dRstAngle;
            out->rstDistance  = r->m_dRstDistance;
            out->rstDistanceX = r->m_dRstDistanceX;
            out->rstDistanceY = r->m_dRstDistanceY;
            out->rstLength0   = r->m_dRstLength[0];
            out->okShiftX     = r->m_bOKShiftX   ? 1 : 0;
            out->okShiftY     = r->m_bOKShiftY   ? 1 : 0;
            out->okAngle      = r->m_bOKAngle    ? 1 : 0;
            out->okLength     = r->m_bOKLength   ? 1 : 0;
            out->okDistance   = r->m_bDistance   ? 1 : 0;
            out->okDistanceX  = r->m_bDistanceX  ? 1 : 0;
            out->okDistanceY  = r->m_bDistanceY  ? 1 : 0;
            out->missing      = r->m_bMissing    ? 1 : 0;
            return 0;
        }
        case eAlgoBodyEdge:
        {
            const RstAlgoBodyEdge* r = static_cast<const RstAlgoBodyEdge*>(ar->m_vRstInspAlgo);
            out->rstShiftX    = r->m_dRstOffset_x;
            out->rstShiftY    = r->m_dRstOffset_y;
            out->rstRealAngle = r->m_dRstRealTheta;
            out->rstAngle     = r->m_dRstTheta;
            out->rstDistanceX = r->m_dRstDistanceX;
            out->rstDistanceY = r->m_dRstDistanceY;
            out->rstDistance  = std::sqrt((r->m_dRstDistanceX * r->m_dRstDistanceX) +
                                          (r->m_dRstDistanceY * r->m_dRstDistanceY));
            out->rstLength0   = r->m_dRstLength;
            out->okShiftX     = r->m_bOKShiftX    ? 1 : 0;
            out->okShiftY     = r->m_bOKShiftY    ? 1 : 0;
            out->okAngle      = r->m_bOKAngle     ? 1 : 0;
            out->okLength     = r->m_bOKLength    ? 1 : 0;
            out->okDistanceX  = r->m_bOKDistanceX ? 1 : 0;
            out->okDistanceY  = r->m_bOKDistanceY ? 1 : 0;
            out->okDistance   = (out->okDistanceX != 0 && out->okDistanceY != 0) ? 1 : 0;
            out->missing      = 0;
            return 0;
        }
        default:
            return -5;
        }
    }
    catch (...) { return -100; }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeResultPattern(
    int wndType, int wndIdx, int algoIdx, MptiBridgeFlowPatternResult* out)
{
    try
    {
        if (out == nullptr) return -1;
        memset(out, 0, sizeof(*out));
        InspAlgoResult* ar = nullptr;
        int rc = ResolveAlgoResultAny(wndType, wndIdx, algoIdx, &out->hdr, &ar);
        if (rc != 0) return rc;
        switch (ar->m_nAlgoType)
        {
        case eAlgoPattern:
        {
            const RstAlgoPattern* r = static_cast<const RstAlgoPattern*>(ar->m_vRstInspAlgo);
            out->score       = r->score;
            out->angle       = r->angle;
            out->cogX        = r->cogX;
            out->cogY        = r->cogY;
            out->offsetX     = r->offsetX;
            out->offsetY     = r->offsetY;
            out->isReverse   = r->isReverse  ? 1 : 0;
            out->okFind      = r->m_bOKFind  ? 1 : 0;
            out->okScore     = r->m_bOKScore ? 1 : 0;
            out->okAngle     = r->m_bOKAngle ? 1 : 0;
            out->okOffsetX   = r->m_bOKOffsetX ? 1 : 0;
            out->okOffsetY   = r->m_bOKOffsetY ? 1 : 0;
            out->okPolarity  = r->m_bOKPolarity ? 1 : 0;
            out->modelNum    = r->m_nModelNum;
            out->divisionNum = r->m_nDivisionNum;
            out->modelWidth  = r->ModelWidth;
            out->modelHeight = r->ModelHeight;
            return 0;
        }
        case eAlgoPatternDiff:
        {
            const RstAlgoPatternDiff* r = static_cast<const RstAlgoPatternDiff*>(ar->m_vRstInspAlgo);
            out->score       = r->m_dAlignScore > 0.0 ? r->m_dAlignScore : r->m_dRstPatternScore[0];
            out->angle       = r->m_dTheta;
            out->cogX        = r->m_ptRstMatchingCenter.x;
            out->cogY        = r->m_ptRstMatchingCenter.y;
            out->offsetX     = r->m_ptRstShiftXY.x;
            out->offsetY     = r->m_ptRstShiftXY.y;
            out->isReverse   = 0;
            out->okFind      = r->m_bOK ? 1 : 0;
            out->okScore     = r->m_bMatchingOK ? 1 : 0;
            out->okAngle     = 1;
            out->okOffsetX   = 1;
            out->okOffsetY   = 1;
            out->okPolarity  = 1;
            out->modelNum    = r->m_nSelectedModelIdx;
            out->divisionNum = r->m_nRectCnt;
            out->modelWidth  = r->m_nRectCnt > 0 ? static_cast<int>(r->m_dRstWidth[0]) : 0;
            out->modelHeight = r->m_nRectCnt > 0 ? static_cast<int>(r->m_dRstLength[0]) : 0;
            return 0;
        }
        case eAlgoOCR:
        {
            const RstAlgoOCR* r = static_cast<const RstAlgoOCR*>(ar->m_vRstInspAlgo);
            out->score       = r->m_dStringScore;
            out->angle       = r->m_dStrAngle;
            out->cogX        = r->m_dStrPosX;
            out->cogY        = r->m_dStrPosY;
            out->offsetX     = 0.0;
            out->offsetY     = 0.0;
            out->isReverse   = r->m_bIsReverse ? 1 : 0;
            out->okFind      = r->m_bOKString ? 1 : 0;
            out->okScore     = r->m_bOKScore ? 1 : 0;
            out->okAngle     = 1;
            out->okOffsetX   = 1;
            out->okOffsetY   = 1;
            out->okPolarity  = r->m_bOKPolarity ? 1 : 0;
            out->modelNum    = r->m_nStrCount;
            out->divisionNum = r->m_nCharCount;
            out->modelWidth  = r->m_nCharCount > 0 ? static_cast<int>(r->charWidth[0]) : 0;
            out->modelHeight = r->m_nCharCount > 0 ? static_cast<int>(r->charHeight[0]) : 0;
            return 0;
        }
        case eAlgoPOCR:
        {
            const RstAlgoPOCR* r = static_cast<const RstAlgoPOCR*>(ar->m_vRstInspAlgo);
            out->score       = r->m_dStringScore;
            out->angle       = static_cast<double>(r->RstRotate) * 180.0;
            out->cogX        = r->ModelX;
            out->cogY        = r->ModelY;
            out->offsetX     = r->dRstShiftX;
            out->offsetY     = r->dRstShiftY;
            out->isReverse   = r->RstRotate != 0 ? 1 : 0;
            out->okFind      = r->m_bOKString ? 1 : 0;
            out->okScore     = r->m_bOKScore ? 1 : 0;
            out->okAngle     = 1;
            out->okOffsetX   = r->bRstShiftX ? 1 : 0;
            out->okOffsetY   = r->bRstShiftY ? 1 : 0;
            out->okPolarity  = r->m_bOKPolarity ? 1 : 0;
            out->modelNum    = r->bAIOK ? 1 : 0;
            out->divisionNum = r->m_nCharCount;
            out->modelWidth  = static_cast<int>(r->ModelWidth);
            out->modelHeight = static_cast<int>(r->ModelHeight);
            return 0;
        }
        default:
            return -5;
        }
    }
    catch (...) { return -100; }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeResultShapeX(
    int wndType, int wndIdx, int algoIdx, MptiBridgeFlowShapeXResult* out)
{
    try
    {
        if (out == nullptr) return -1;
        memset(out, 0, sizeof(*out));
        InspAlgoResult* ar = nullptr;
        int rc = ResolveAlgoResult(wndType, wndIdx, algoIdx, eAlgoShapeX, &out->hdr, &ar);
        if (rc != 0) return rc;
        const RstAlgoShapeX* r = static_cast<const RstAlgoShapeX*>(ar->m_vRstInspAlgo);
        out->nRoiCnt          = r->nROICnt;
        out->nNgAreaRoiCnt    = r->nNGAreaRoiCnt;
        out->nShapeNgCnt      = r->nShapeNGCnt;
        out->rstWrForeignCnt  = r->RstWrForeignCnt;
        out->rstOkWrForeignCnt = r->RstOKWrForeignCnt ? 1 : 0;
        out->bAiOk            = r->bAIOK ? 1 : 0;
        out->stdAiScore       = r->stdAIScore;
        return 0;
    }
    catch (...) { return -100; }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeResultPadBW(
    int wndType, int wndIdx, int algoIdx, MptiBridgeFlowPadBWResult* out)
{
    try
    {
        if (out == nullptr) return -1;
        memset(out, 0, sizeof(*out));
        InspAlgoResult* ar = nullptr;
        int rc = ResolveAlgoResult(wndType, wndIdx, algoIdx, eAlgoPadBW, &out->hdr, &ar);
        if (rc != 0) return rc;
        const RstAlgoPadBW* r = static_cast<const RstAlgoPadBW*>(ar->m_vRstInspAlgo);
        out->okShapeArea     = r->m_bOKShapeArea ? 1 : 0;
        out->okShapeShiftX   = r->m_bOKShapeShiftX ? 1 : 0;
        out->okShapeShiftY   = r->m_bOKShapeShiftY ? 1 : 0;
        out->okWidth         = r->m_bOKWidth ? 1 : 0;
        out->okLength        = r->m_bOKLength ? 1 : 0;
        out->okArea          = r->m_bOKArea ? 1 : 0;
        out->maskLoadSuccess = r->m_bMaskLoadSuccess ? 1 : 0;
        out->arrShapeRectCnt = r->m_nArrShapeRectCnt;
        out->arrRectCnt      = r->m_nArrRectCnt;
        out->alignResultTheta = r->m_dAlignResultTheta;
        return 0;
    }
    catch (...) { return -100; }
}

namespace
{
    // Append helper that won't overrun the output buffer. `pos` is the current write
    // offset in WCHARs. Returns new pos.
    int Append(wchar_t* output, int cap, int pos, const wchar_t* fmt, ...)
    {
        if (output == nullptr || pos >= cap - 1) return pos;
        va_list ap;
        va_start(ap, fmt);
        int written = _vsnwprintf_s(output + pos, static_cast<size_t>(cap - pos),
                                    _TRUNCATE, fmt, ap);
        va_end(ap);
        return written > 0 ? pos + written : pos;
    }
}

MPTI_BRIDGE_FLOW_API int MptiBridgeDumpAlignDiag(wchar_t* output, int outputLength)
{
    if (output == nullptr || outputLength <= 0) return -1;
    output[0] = L'\0';
    int p = 0;

    if (g_pMPTI == nullptr)
    {
        p = Append(output, outputLength, p, L"g_pMPTI is NULL\n");
        return 0;
    }
    p = Append(output, outputLength, p,
        L"g_pMPTI ok | m_nSizeXRawData=%d m_nSizeYRawData=%d\n",
        g_pMPTI->m_nSizeXRawData, g_pMPTI->m_nSizeYRawData);

    if (g_pInspMng == nullptr)
    {
        p = Append(output, outputLength, p, L"g_pInspMng is NULL\n");
        return 0;
    }
    p = Append(output, outputLength, p,
        L"g_pInspMng ok | m_bSetResolution=%d fov=%dx%d resol=(%.6f,%.6f)\n",
        (int)g_pInspMng->m_bSetResolution,
        g_pInspMng->m_fovWidth, g_pInspMng->m_fovLength,
        g_pInspMng->m_resolX, g_pInspMng->m_resolY);
    p = Append(output, outputLength, p,
        L"  PInspAlgo_Lib::m_resolX=%.6f m_resolY=%.6f\n",
        PIAL::PInspAlgo_Lib::m_resolX, PIAL::PInspAlgo_Lib::m_resolY);

    // group meta
    if (g_pInspMng->m_inspItemCnts)
        p = Append(output, outputLength, p,
            L"  m_inspItemCnts[eINSP_ALIGN=%d] = %d\n",
            (int)eINSP_ALIGN, g_pInspMng->m_inspItemCnts[eINSP_ALIGN]);
    else
        p = Append(output, outputLength, p, L"  m_inspItemCnts: NULL\n");
    if (g_pInspMng->m_groupIndexCnts)
        p = Append(output, outputLength, p,
            L"  m_groupIndexCnts[eINSP_ALIGN] = %d\n",
            g_pInspMng->m_groupIndexCnts[eINSP_ALIGN]);
    else
        p = Append(output, outputLength, p, L"  m_groupIndexCnts: NULL\n");

    InspPartInfo* bi = g_pInspMng->GetInspPartInfo();
    if (bi == nullptr)
        p = Append(output, outputLength, p, L"  GetInspPartInfo: NULL\n");
    else
    {
        p = Append(output, outputLength, p,
            L"  partImgBuf: size=%dx%d topR=%s topG=%s topB=%s topW=%s\n",
            bi->partImgBuf.nImageSizeX, bi->partImgBuf.nImageSizeY,
            bi->partImgBuf.imgTop_R ? L"set" : L"NULL",
            bi->partImgBuf.imgTop_G ? L"set" : L"NULL",
            bi->partImgBuf.imgTop_B ? L"set" : L"NULL",
            bi->partImgBuf.imgTop_W ? L"set" : L"NULL");
        p = Append(output, outputLength, p,
            L"  partCx=%.1f partCy=%.1f partW=%.1f partH=%.1f nWindowCount=%d\n",
            bi->partCx, bi->partCy, bi->partWidth, bi->partHeight, bi->nWindowCount);
        p = Append(output, outputLength, p,
            L"  anyAngle: cx=%.1f cy=%.1f w=%.1f l=%.1f\n",
            bi->anyAngleCx, bi->anyAngleCy, bi->anyAngleWidth, bi->anyAngleLength);
        if (bi->pWindows && bi->nWindowCount > 0)
        {
            for (int i = 0; i < bi->nWindowCount && p < outputLength - 256; ++i)
            {
                const InspPartParam& w = bi->pWindows[i];
                p = Append(output, outputLength, p,
                    L"  wnd[%d]: inspType=%d WndInspType=%d cx=%.1f cy=%.1f w=%.1f l=%.1f "
                    L"anyAngle=(%.1f,%.1f,%.1f,%.1f) algoCnt=%d vArrAlgoParam=%s "
                    L"alignWndID=%d alignPartWnd=%d groupIndex=%d wndIndex=%d nInspCamType=%d Gen2D=%d\n",
                    i, w.inspType, (int)w.WndInspType, w.cx, w.cy, w.width, w.length,
                    w.anyAngleCx, w.anyAngleCy, w.anyAngleWidth, w.anyAngleLength,
                    w.nAlgorithmCnt, w.vArrAlgoParam ? L"set" : L"NULL",
                    w.nAlignWndID, w.m_nAlignPartWnd, w.groupIndex, w.wndIndex,
                    w.nInspCameraType, w.Gen2D);
                if (w.vArrAlgoParam && w.nAlgorithmCnt > 0)
                {
                    for (int a = 0; a < w.nAlgorithmCnt && p < outputLength - 256; ++a)
                    {
                        const InspAlgo& al = w.vArrAlgoParam[a];
                        p = Append(output, outputLength, p,
                            L"    algoIn[%d]: eAlgoType=%d enable=%d required=%d ptr=%s "
                            L"light=%d R=%d G=%d B=%d W=%d lightCnt=%d mix=%d\n",
                            a, (int)al.m_eAlgoType, (int)al.m_bAlgoEnable, (int)al.m_bIsRequired,
                            al.m_ptrInspAlgoParam ? L"set" : L"NULL",
                            (int)al.m_eLightType, al.m_nRedValue, al.m_nGreenValue,
                            al.m_nBlueValue, al.m_nWhiteValue, al.m_nLightCnt, al.m_nMixCount);
                    }
                }
            }
        }
    }

    InspectionResult* r = g_pInspMng->m_inspectionResult;
    if (r == nullptr)
    {
        p = Append(output, outputLength, p, L"m_inspectionResult: NULL\n");
        return 0;
    }
    p = Append(output, outputLength, p,
        L"InspectionResult: isInspAlign=%d alignArraySize=%d alignResult=%s\n",
        (int)r->isInspAlign, r->alignArraySize,
        r->alignResult ? L"set" : L"NULL");

    if (r->alignResult == nullptr || r->alignArraySize <= 0)
        return 0;

    for (int i = 0; i < r->alignArraySize && p < outputLength - 256; ++i)
    {
        const InspWndResult& w = r->alignResult[i];
        p = Append(output, outputLength, p,
            L"  alignResult[%d]: m_bIsInsp=%d m_bOk=%d m_nDefectCode=%d "
            L"m_nAlgorithmCnt=%d m_vArrRstInspAlgo=%s\n",
            i, (int)w.m_bIsInsp, (int)w.m_bOk, w.m_nDefectCode,
            w.m_nAlgorithmCnt, w.m_vArrRstInspAlgo ? L"set" : L"NULL");
        if (w.m_vArrRstInspAlgo == nullptr) continue;
        for (int j = 0; j < w.m_nAlgorithmCnt && p < outputLength - 128; ++j)
        {
            const InspAlgoResult& a = w.m_vArrRstInspAlgo[j];
            p = Append(output, outputLength, p,
                L"    algo[%d]: m_bIsInsp=%d m_bIsRequired=%d m_bOk=%d "
                L"m_eAlgoType=%d m_vRstInspAlgo=%s m_nDefectCode=%d\n",
                j, (int)a.m_bIsInsp, (int)a.m_bIsRequired, (int)a.m_bOk,
                (int)a.m_nAlgoType, a.m_vRstInspAlgo ? L"set" : L"NULL",
                a.m_nDefectCode);
        }
    }
    return 0;
}
