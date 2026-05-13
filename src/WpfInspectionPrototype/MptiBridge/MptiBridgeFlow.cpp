#include "stdafx.h"
#include "MPTI.h"
#include "MptiBridgeFlow.h"
#include "NativeSources/MPTILib_Algo/InspManager.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/Align/InspParamDef_Align.h"

#include <cwchar>
#include <exception>
#include <vector>

namespace
{
    // Native-owned input model. Rebuilt on every MptiBridgeBeginPart().
    struct AlgoStorage
    {
        int       type = -1;     // InspAlgoType
        AlgoAlign alignParam{};   // valid when type == eAlgoAlign
    };

    struct WindowStorage
    {
        std::vector<InspAlgo>    algos;
        std::vector<AlgoStorage> algoStore;
    };

    InspPartInfo                s_part{};
    std::vector<InspPartParam>  s_windows;
    std::vector<WindowStorage>  s_windowStore;
    bool                        s_committed = false;
    bool                        s_inspected = false;
    DWORD                       s_lastInspProcSehCode = 0;

    int s_sourceWidth = 0;
    int s_sourceHeight = 0;

    void Reset()
    {
        memset(&s_part, 0, sizeof(s_part));
        s_windows.clear();
        s_windowStore.clear();
        s_committed = false;
        s_inspected = false;
        s_sourceWidth = s_sourceHeight = 0;
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

    int CallMptiInspProcGuarded()
    {
        s_lastInspProcSehCode = 0;
        __try
        {
            return MPTI_InspProc();
        }
        __except ((s_lastInspProcSehCode = GetExceptionCode()), EXCEPTION_EXECUTE_HANDLER)
        {
            return -300;
        }
    }
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
        s_part.angle = angle;
        s_part.partIndex = 0;
        s_part.nPartID = 1;
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
        wnd.wndIndex = static_cast<int>(s_windows.size()) + 1;
        wnd.groupIndex = static_cast<int>(s_windows.size()) + 1;   // one window == one group (SortingParamater skips groupIndex < 1)
        wnd.nAlignWndID = alignWndId;
        wnd.nParentWndID = parentWndId;
        wnd.WndInspType = static_cast<byte>(wndInspType);
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
        AlgoStorage as;
        as.type = algoType;   // default-constructed AlgoAlign carries reasonable defaults
        store.algoStore.push_back(as);
        return static_cast<int>(store.algos.size()) - 1;
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
        if (store.algoStore[algoIndex].type != eAlgoAlign)
            return -13;

        AlgoAlign& a = store.algoStore[algoIndex].alignParam;
        const int sn = params->searchNum < 1 ? 1 : (params->searchNum > 4 ? 4 : params->searchNum);
        a.m_nSearchNum = sn;
        a.m_nSearchMargin = params->searchMargin < 0 ? 0 : params->searchMargin;
        a.m_nMinBinary = params->minBinary;
        a.m_nMaxBinary = params->maxBinary;
        a.m_bInsp2D = params->useInsp2D != 0;
        a.m_bInsp3D = FALSE;
        a.m_InvertCheck = params->invertCheck != 0;
        a.m_bUseShift = params->useShift != 0;
        a.m_dShiftX = params->maxShiftX;
        a.m_dShiftY = params->maxShiftY;
        a.m_bUseAngle = params->useAngle != 0;
        a.m_dAngle = params->maxAngle;
        a.m_bSameSize = params->sameSize != 0;
        a.m_nMinBlobArea = params->minBlobArea < 1 ? 1 : params->minBlobArea;
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

MPTI_BRIDGE_FLOW_API int MptiBridgeCommitInspParam(wchar_t* message, int messageLength)
{
    try
    {
        if (g_pMPTI == nullptr)
            return Fail(message, messageLength, -200, L"MptiBridgeCommitInspParam: MPTI not initialized (call a load first)");
        if (s_windows.empty())
            return Fail(message, messageLength, -1, L"MptiBridgeCommitInspParam: no windows added");

        // Wire algorithm arrays + algo-param pointers now that all adds are done.
        for (size_t w = 0; w < s_windows.size(); ++w)
        {
            auto& store = s_windowStore[w];
            for (size_t a = 0; a < store.algos.size(); ++a)
            {
                if (store.algoStore[a].type == eAlgoAlign)
                    store.algos[a].m_ptrInspAlgoParam = &store.algoStore[a].alignParam;
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
            wchar_t buf[256];
            swprintf_s(buf, L"MPTI_InspProc SEH exception 0x%08X", s_lastInspProcSehCode);
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
