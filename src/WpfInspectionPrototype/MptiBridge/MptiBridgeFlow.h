#pragma once

// Reference-faithful inspection flow exposed to C#:
//   C# builder calls -> InspPartInfo + InspPartParam[] + InspAlgo[] (native-owned)
//   MptiBridgeCommitInspParam -> MPTI_SetInspParam
//   MptiBridgeInspProc        -> MPTI_InspProc -> InspManager::InspProc -> InspNormal_Ver2 -> InspWindowAlgo3 -> PInsp_Algo
//   MptiBridgeResult*         -> walk InspectionResult on the native side, return flat scalars
//
// All giant pointer-graph structs (InspPartInfo / InspPartParam / InspAlgo / InspectionResult)
// stay on the C++ side. C# never marshals them directly.

#pragma pack(push, 8)

// Mirrors PInsp_Algo/Align/InspParamDef_Align.h tagAlgoAlign, but flat & marshallable.
struct MptiBridgeFlowAlignParams
{
    int     searchNum;          // 1-4
    int     searchPointsX[4];   // part-space center X (pixel)
    int     searchPointsY[4];   // part-space center Y (pixel)
    int     searchSizeW[4];
    int     searchSizeH[4];
    int     searchMargin;
    int     minBinary;          // 0-255
    int     maxBinary;          // 0-255
    int     useInsp2D;          // 0/1
    int     invertCheck;        // 0/1
    int     useShift;           // 0/1
    double  maxShiftX;
    double  maxShiftY;
    int     useAngle;           // 0/1
    double  maxAngle;
    int     sameSize;           // 0/1
    int     minBlobArea;
};

// Mirrors PInsp_Algo/PadBW/InspParamDef_PadBW.h tagAlgoPadBW (subset). Captures the
// fields the WPF UI typically wires: binary thresholds (via the default-pad sub-struct),
// teach-area gating, shift tolerance, blob size gates.
struct MptiBridgeFlowPadBWParams
{
    int     binaryMin;          // sDefaultPad.m_nMinBinary
    int     binaryMax;          // sDefaultPad.m_nMaxBinary
    int     useInsp2D;          // sDefaultPad.m_bInsp2D
    int     invertCheck;        // unused on PadBW side, kept for symmetry with Align
    int     useTeachArea;       // bTeachAreaUse
    double  teachArea;          // dTeachArea
    double  teachAreaRateMin;   // dTeachAreaRateMin
    double  teachAreaRateMax;   // dTeachAreaRateMax
    int     useShift;           // bUseShift
    double  teachShiftX;        // dTeachShiftX
    double  teachShiftY;        // dTeachShiftY
    int     useBlobWidth;       // bUseBlobWidth
    double  blobSizeWidth;      // dBlobSizeWidth
    int     useBlobLength;      // bUseBlobLength
    double  blobSizeLength;     // dBlobSizeLength
    int     useBlobArea;        // bUseBlobArea
    double  blobArea;           // dBlobArea
    int     filterLevel;        // nFilterLevel
    int     useFillHole;        // sDefaultPad.m_bUseFillHole
};

// Flattened RstAlgoAlign for results.
struct MptiBridgeFlowAlignResult
{
    int     okCount;
    double  offsetX;
    double  offsetY;
    double  theta;
    int     okShiftX;
    int     okShiftY;
    int     okAngle;
    int     detectedCentersX[4];
    int     detectedCentersY[4];
    int     isInsp;
    int     isOk;
    int     defectCode;
};

#pragma pack(pop)

#define MPTI_BRIDGE_FLOW_API extern "C" __declspec(dllexport)

// --- builders (native owns memory; each Begin resets prior state) ---

// partCx/Cy/W/H/angle are part-space; sourceWidth/Height are the part image dims.
MPTI_BRIDGE_FLOW_API int MptiBridgeBeginPart(
    double partCx, double partCy, double partWidth, double partHeight, double angle,
    int sourceWidth, int sourceHeight);

// Adds an inspection window. wndInspType is eINSP_* (eINSP_ALIGN = 1, etc).
// Returns the window index (>= 0) or negative on error.
MPTI_BRIDGE_FLOW_API int MptiBridgeAddWindow(
    int wndInspType, double cx, double cy, double width, double height,
    int alignWndId, int parentWndId);

// Adds an algorithm to a window. algoType is eAlgo* (eAlgoAlign etc).
// Returns the algorithm index within the window (>= 0) or negative on error.
MPTI_BRIDGE_FLOW_API int MptiBridgeAddAlgo(int wndIndex, int algoType, int algoId);

// Fills the AlgoAlign param of an existing (window, algorithm) Align entry.
MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoParamsAlign(
    int wndIndex, int algoIndex, const MptiBridgeFlowAlignParams* params);

// Fills the AlgoPadBW param of an existing (window, algorithm) PadBW entry. The native
// AlgoPadBW struct has many more fields; the rest keep their default-constructed values.
MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoParamsPadBW(
    int wndIndex, int algoIndex, const MptiBridgeFlowPadBWParams* params);

// Finalizes inputs -> MPTI_SetInspParam + InitResultVal. Must call after PTT load.
MPTI_BRIDGE_FLOW_API int MptiBridgeCommitInspParam(wchar_t* message, int messageLength);

// Runs MPTI_InspProc().
MPTI_BRIDGE_FLOW_API int MptiBridgeInspProc(wchar_t* message, int messageLength);

// --- result readers (valid after MptiBridgeInspProc) ---

// Number of Align result windows (InspectionResult::alignArraySize).
MPTI_BRIDGE_FLOW_API int MptiBridgeResultAlignCount();

// i-th Align result window, first algorithm. Returns 0 on success.
MPTI_BRIDGE_FLOW_API int MptiBridgeResultAlign(int i, MptiBridgeFlowAlignResult* out);

// Diagnostic: dumps the full InspectionResult.alignResult[0] structure into `output`
// as a multi-line wide string. Reports why isInsp may be 0:
//   - alignResult ptr null?
//   - alignArraySize
//   - per-window: m_bIsInsp / m_bOk / m_nAlgorithmCnt / m_vArrRstInspAlgo null?
//   - per-algo: m_bIsInsp / m_bIsRequired / m_eAlgoType / m_vRstInspAlgo null?
//   - partImgBuf state, m_inspItemCnts/m_groupIndexCnts for eINSP_ALIGN.
// Returns 0 on success.
MPTI_BRIDGE_FLOW_API int MptiBridgeDumpAlignDiag(wchar_t* output, int outputLength);
