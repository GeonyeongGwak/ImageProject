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
    int     typeRange2D;        // m_nTypeRange2D
    int     useInsp2D;          // 0/1
    int     invertCheck;        // 0/1
    int     useInsp3D;          // 0/1
    double  heightRateMin;
    double  heightRateMax;
    double  heightAverage;
    int     typeRange3D;        // m_nTypeRange3D
    int     useIpc;             // 0/1
    int     ipcClass;           // 0=Class1, 1=Class2, 2=Class3
    int     useShift;           // 0/1
    double  maxShiftX;
    double  maxShiftY;
    int     useAngle;           // 0/1
    double  maxAngle;
    int     sameSize;           // 0/1
    int     minBlobArea;
    int     fillHole;           // 0/1
    int     inspOption;         // m_byInspOPT bitmask
};

// Mirrors the common InspAlgo light envelope. Normal light modes use the scalar
// channel values; User_Light additionally consumes the first lightCnt entries of the
// array fields.
struct MptiBridgeFlowLightParams
{
    int     lightType;          // InspLightType (Top_Light..ThreeD)
    int     redValue;
    int     greenValue;
    int     blueValue;
    int     whiteValue;
    int     lightCnt;           // 0..10
    int     arrRedValue[10];
    int     arrGreenValue[10];
    int     arrBlueValue[10];
    int     arrWhiteValue[10];
    int     arrCalculation[10]; // 0=none, 1=add, 2=sub
    int     arrLightPosition[10]; // Top/Middle/Bottom for user light cells
};

// Mirrors PInsp_Algo/Blob/InspParamDef_Blob.h stAlgoBlob (subset).
struct MptiBridgeFlowBlobParams
{
    int     useInsp2D;          // m_bInsp2D
    int     minBinary;          // m_nMinBinary
    int     maxBinary;          // m_nMaxBinary
    int     typeRange2D;        // m_nTypeRange2D (0=in, 1=out, 2=upper, 3=lower)
    int     invertCheck;        // m_bInvertCheck
    int     typeSelectBlob;     // m_nTypeSelectBlob
    int     fillHole;           // m_bFillHole
    int     useArea;            // m_bAreaIsUse
    double  areaMin;            // m_dAreaMin
    double  areaMax;            // m_dAreaMax
    int     useShift;           // m_bShiftIsUse
    double  shiftX;             // m_dShiftX
    double  shiftY;             // m_dShiftY
    int     useTeachWidth;      // m_bTeachWidthUse
    double  teachWidth;         // m_dTeachWidth
    double  teachWidthRateMin;  // m_dTeachWidthRateMin
    double  teachWidthRateMax;  // m_dTeachWidthRateMax
    int     useTeachLength;     // m_bTeachLengthUse
    double  teachLength;        // m_dTeachLength
    double  teachLengthRateMin; // m_dTeachLengthRateMin
    double  teachLengthRateMax; // m_dTeachLengthRateMax
};

// Mirrors PInsp_Algo/BGA/InspParamDef_BGA.h stAlgoBGA (subset).
struct MptiBridgeFlowBGAParams
{
    int     useInsp2D;          // m_bInsp2D
    int     minBinary;          // m_nMinBinary
    int     maxBinary;          // m_nMaxBinary
    int     typeRange2D;        // m_nTypeRange2D
    int     invertCheck;        // m_bInvertCheck
    int     typeSelectBlob;     // m_nTypeSelectBlob
    int     fillHole;           // m_bFillHole
    int     useArea;            // m_bAreaIsUse
    double  areaMin;            // m_dAreaMin
    double  areaMax;            // m_dAreaMax
    double  teachArea;          // m_dTeachArea
    double  teachVolume;        // m_dTeachVolume
    int     useShift;           // m_bShiftIsUse
    double  shiftX;             // m_dShiftX
    double  shiftY;             // m_dShiftY
    int     useTeachWidth;      // m_bTeachWidthUse
    double  teachWidth;         // m_dTeachWidth
    double  teachWidthRateMin;  // m_dTeachWidthRateMin
    double  teachWidthRateMax;  // m_dTeachWidthRateMax
    int     useTeachLength;     // m_bTeachLengthUse
    double  teachLength;        // m_dTeachLength
    double  teachLengthRateMin; // m_dTeachLengthRateMin
    double  teachLengthRateMax; // m_dTeachLengthRateMax
    int     useCircleRate;      // m_bUseCircleRate
    double  teachCircleRate;    // m_dTeachCircleRate
};

// Mirrors PInsp_Algo/Edge/InspParamDef_Edge.h stAlgoEdge (subset). The Edge algorithm
// also has m_bArrIsHorizon[]/m_poArrSetTeachCenter[] per-line arrays which a future
// per-line setter would expose; this struct covers the global gating + first-line.
struct MptiBridgeFlowEdgeParams
{
    int     useInsp2D;          // m_bInsp2D
    int     minBinary;          // m_nMinBinary
    int     maxBinary;          // m_nMaxBinary
    int     typeRange2D;        // m_nTypeRange2D
    int     invertCheck;        // m_bInvertCheck
    int     useShift;           // m_bShiftIsUse
    double  shiftX;             // m_dShiftX
    double  shiftY;             // m_dShiftY
    int     useArea;            // m_bAreaIsUse
    double  areaMin;            // m_dAreaMin
    double  areaMax;            // m_dAreaMax
    int     setLineCnt;         // m_nSetLineCnt (1..EdgeLineTotalCnt)
    int     useGroup;           // m_bGroup
    int     lineFindType;       // m_nLineFindType
    double  lineFindRate;       // m_dLineFindRate
    int     useAngle;           // m_bUseAngle
    double  teachRotate;        // m_dTeachRotate
    int     useDistanceX;       // m_bDistanceX
    double  teachDistanceX;     // m_dTeachDistanceX
    int     useDistanceY;       // m_bDistanceY
    double  teachDistanceY;     // m_dTeachDistanceY
};

// Mirrors PInsp_Algo/Pattern/InspParamDef_Pattern.h tagAlgoPattern (subset). Model
// paths use a single MAX_STRLEN wide-char buffer; bridge copies into m_sArrPathModelInspect1
// and m_sPathModelTeach. ModelPath is optional (nullptr/empty leaves defaults).
struct MptiBridgeFlowPatternParams
{
    int       usePolarity;            // m_bUsePolarity
    double    acceptScore;            // m_dAcceptScore
    int       useShift;               // m_bShiftIsUse
    double    shiftX;                 // m_dShiftX
    double    shiftY;                 // m_dShiftY
    double    rangeAngle;             // m_dRangeAngle
    double    wndAngle;               // m_dWndAngle
    double    searchAngleRangeMin;    // m_dSearchAngleRange_Min
    double    searchAngleRangeMax;    // m_dSearchAngleRange_Max
    int       samplingAngle;          // m_SamplingAngle
    int       useNgOpt;               // m_bUseNGOpt
    int       useCharacter;           // m_bUseCharacter
    int       modelFilter;            // m_nModelFilter
    int       cntPatternPath;         // m_nCntPatternPath (1..16)
    double    factorRed;              // m_factor_red
    double    factorGreen;            // m_factor_green
    double    factorBlue;             // m_factor_blue
    // Wide-char model paths (UTF-16, null-terminated). Empty (first wchar == 0) means
    // "leave default-constructed value". 260 chars matches MAX_STRLEN in MPTI headers.
    wchar_t   modelPathInspect1[260];
    wchar_t   modelPathTeach[260];
};

// Mirrors PInsp_Algo/ShapeX/InspParamDef_ShapeX.h tagAlgoShapeX (subset). The full
// AlgoShapeX is huge (200-element ShapeXROI array) — this exposes the global gating;
// ROICnt=0 is a valid "no per-ROI shape inspection" state.
struct MptiBridgeFlowShapeXParams
{
    double  partAngle;          // dPartangle
    int     leadTipDirection;   // m_nLeadTipDirection
    int     roiCnt;             // ROICnt (keep 0 for global-only)
    short   matchScore;         // MatchSc
    short   hist1;              // nHist1
    short   hist1Low;           // nHistLow1
    short   hist1Up;            // nHistUp1
    short   hist2;              // nHist2
    short   hist2Low;           // nHistLow2
    short   hist2Up;            // nHistUp2
    int     inspOption;         // nInspOption (bitmask of SHAPEX_OPTION)
    float   aspectRatio;        // Aspectratio
    float   minScarThickness;   // MinScarThickness
    float   verticalMaxLength;  // Verticalmaxlength
    float   horizontalMaxLength; // Horizontalmaxlength
    float   maxNgArea;          // MaxNGArea
    float   criticalArea;       // CriticalArea
    float   maxChippingLength;  // MaxChippingLength
    int     startIdx;           // nStartIdx
    float   tieBarRate;         // fTieBarRate
    int     contrastValue;      // ContrastValue
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

// Common per-algo result envelope appended at the top of each typed result struct.
// Lets the caller see isInsp/isOk/defectCode + bounding rect uniformly without parsing
// the algo-specific payload.
struct MptiBridgeFlowWndAlgoHeader
{
    int wndIsInsp;     // InspWndResult.m_bIsInsp (window-level)
    int wndIsOk;       // InspWndResult.m_bOk
    int wndDefectCode; // InspWndResult.m_nDefectCode
    int algoIsInsp;    // InspAlgoResult.m_bIsInsp (per-algo)
    int algoIsOk;      // InspAlgoResult.m_bOk
    int algoIsRequired; // InspAlgoResult.m_bIsRequired
    int algoDefectCode; // InspAlgoResult.m_nDefectCode
    int algoAlgoType;   // InspAlgoResult.m_nAlgoType (= eAlgoXxx)
};

// Flattened RstAlgoBlob (subset).
struct MptiBridgeFlowBlobResult
{
    MptiBridgeFlowWndAlgoHeader hdr;
    double rstArea;
    double rstAreaRate;
    double rstShiftX;
    double rstShiftY;
    double rstWidth;
    double rstLength;
    double rstHeightMean;
    int    okArea;
    int    okShiftX;
    int    okShiftY;
    int    okWidth;
    int    okLength;
    int    okHeight;
    int    rectLeft;
    int    rectTop;
    int    rectRight;
    int    rectBottom;
    int    arrRectCnt;
};

// Flattened RstAlgoBGA (subset).
struct MptiBridgeFlowBGAResult
{
    MptiBridgeFlowWndAlgoHeader hdr;
    int    okCoplanarity;
    int    okGridOffsetX;
    int    okGridOffsetY;
    int    okTwist;
    float  coplanarity;
    float  gridOffsetX;
    float  gridOffsetY;
    float  twist;
    int    rectLeft;
    int    rectTop;
    int    rectRight;
    int    rectBottom;
};

// Flattened RstAlgoEdge (subset). Per-line arrays (length per line, etc.) are omitted
// here — see m_dRstLength[EdgeLineTotalCnt] in tagRstAlgoEdge for full data.
struct MptiBridgeFlowEdgeResult
{
    MptiBridgeFlowWndAlgoHeader hdr;
    double rstShiftX;
    double rstShiftY;
    double rstRealAngle;
    double rstAngle;
    double rstDistance;
    double rstDistanceX;
    double rstDistanceY;
    double rstLength0;     // m_dRstLength[0] (first line)
    int    okShiftX;
    int    okShiftY;
    int    okAngle;
    int    okLength;
    int    okDistance;
    int    okDistanceX;
    int    okDistanceY;
    int    missing;
};

// Flattened RstAlgoPattern (subset).
struct MptiBridgeFlowPatternResult
{
    MptiBridgeFlowWndAlgoHeader hdr;
    double score;
    double angle;
    double cogX;
    double cogY;
    double offsetX;
    double offsetY;
    int    isReverse;
    int    okFind;
    int    okScore;
    int    okAngle;
    int    okOffsetX;
    int    okOffsetY;
    int    okPolarity;
    int    modelNum;
    int    divisionNum;
    int    modelWidth;
    int    modelHeight;
};

// Flattened RstAlgoShapeX (top-level only — per-ROI arrays are exposed via the future
// per-ROI reader API).
struct MptiBridgeFlowShapeXResult
{
    MptiBridgeFlowWndAlgoHeader hdr;
    int    nRoiCnt;          // nROICnt
    int    nNgAreaRoiCnt;    // nNGAreaRoiCnt
    int    nShapeNgCnt;      // nShapeNGCnt
    int    rstWrForeignCnt;  // RstWrForeignCnt
    int    rstOkWrForeignCnt; // RstOKWrForeignCnt
    int    bAiOk;            // bAIOK
    float  stdAiScore;       // stdAIScore
};

// Flattened RstAlgoPadBW (subset).
struct MptiBridgeFlowPadBWResult
{
    MptiBridgeFlowWndAlgoHeader hdr;
    int    okShapeArea;
    int    okShapeShiftX;
    int    okShapeShiftY;
    int    okWidth;
    int    okLength;
    int    okArea;
    int    maskLoadSuccess;
    int    arrShapeRectCnt;
    int    arrRectCnt;
    double alignResultTheta;
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

// Sets the pixel resolution (mm/pixel) used by the inspection pipeline. Typically the
// caller reads this from the .pot file accompanying a .ptt (or from Part Import
// metadata) and calls this before MptiBridgeCommitInspParam. If never called, commit
// falls back to 1.0 (treats all coordinates as pixel-space). Passing 0 clears the
// explicit value and re-enables the fallback. Returns 0 on success.
MPTI_BRIDGE_FLOW_API int MptiBridgeSetFlowResolution(double resolX, double resolY);

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

// Fills the common InspAlgo light envelope of an existing (window, algorithm) entry.
MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoLight(
    int wndIndex, int algoIndex, const MptiBridgeFlowLightParams* params);

// Fills the AlgoAlign param of an existing (window, algorithm) Align entry.
MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoParamsAlign(
    int wndIndex, int algoIndex, const MptiBridgeFlowAlignParams* params);

// Fills the AlgoPadBW param of an existing (window, algorithm) PadBW entry. The native
// AlgoPadBW struct has many more fields; the rest keep their default-constructed values.
MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoParamsPadBW(
    int wndIndex, int algoIndex, const MptiBridgeFlowPadBWParams* params);

MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoParamsBlob(
    int wndIndex, int algoIndex, const MptiBridgeFlowBlobParams* params);

MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoParamsBGA(
    int wndIndex, int algoIndex, const MptiBridgeFlowBGAParams* params);

MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoParamsEdge(
    int wndIndex, int algoIndex, const MptiBridgeFlowEdgeParams* params);

MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoParamsPattern(
    int wndIndex, int algoIndex, const MptiBridgeFlowPatternParams* params);

MPTI_BRIDGE_FLOW_API int MptiBridgeSetAlgoParamsShapeX(
    int wndIndex, int algoIndex, const MptiBridgeFlowShapeXParams* params);

// Finalizes inputs -> MPTI_SetInspParam + InitResultVal. Must call after PTT load.
MPTI_BRIDGE_FLOW_API int MptiBridgeCommitInspParam(wchar_t* message, int messageLength);

// Runs MPTI_InspProc().
MPTI_BRIDGE_FLOW_API int MptiBridgeInspProc(wchar_t* message, int messageLength);

// --- result readers (valid after MptiBridgeInspProc) ---

// Number of Align result windows (InspectionResult::alignArraySize).
MPTI_BRIDGE_FLOW_API int MptiBridgeResultAlignCount();

// i-th Align result window, first algorithm. Returns 0 on success.
MPTI_BRIDGE_FLOW_API int MptiBridgeResultAlign(int i, MptiBridgeFlowAlignResult* out);

// Typed result readers for the non-Align algorithms. They take (wndType, wndIdx, algoIdx)
// because non-Align algos can live in any window-type result array (mountResult /
// padResult / BGAResult / etc.). wndType uses the eINSP_* enum from InspParamDef.h
// (eINSP_MOUNT=0, eINSP_ALIGN=1, eINSP_PAD=7, eINSP_BGA=9, ...). Returns 0 on success.
MPTI_BRIDGE_FLOW_API int MptiBridgeResultBlob(
    int wndType, int wndIdx, int algoIdx, MptiBridgeFlowBlobResult* out);
MPTI_BRIDGE_FLOW_API int MptiBridgeResultBGA(
    int wndType, int wndIdx, int algoIdx, MptiBridgeFlowBGAResult* out);
MPTI_BRIDGE_FLOW_API int MptiBridgeResultEdge(
    int wndType, int wndIdx, int algoIdx, MptiBridgeFlowEdgeResult* out);
MPTI_BRIDGE_FLOW_API int MptiBridgeResultPattern(
    int wndType, int wndIdx, int algoIdx, MptiBridgeFlowPatternResult* out);
MPTI_BRIDGE_FLOW_API int MptiBridgeResultShapeX(
    int wndType, int wndIdx, int algoIdx, MptiBridgeFlowShapeXResult* out);
MPTI_BRIDGE_FLOW_API int MptiBridgeResultPadBW(
    int wndType, int wndIdx, int algoIdx, MptiBridgeFlowPadBWResult* out);

// Diagnostic: dumps the full InspectionResult.alignResult[0] structure into `output`
// as a multi-line wide string. Reports why isInsp may be 0:
//   - alignResult ptr null?
//   - alignArraySize
//   - per-window: m_bIsInsp / m_bOk / m_nAlgorithmCnt / m_vArrRstInspAlgo null?
//   - per-algo: m_bIsInsp / m_bIsRequired / m_eAlgoType / m_vRstInspAlgo null?
//   - partImgBuf state, m_inspItemCnts/m_groupIndexCnts for eINSP_ALIGN.
// Returns 0 on success.
MPTI_BRIDGE_FLOW_API int MptiBridgeDumpAlignDiag(wchar_t* output, int outputLength);
