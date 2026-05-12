#pragma once

// Generic algorithm dispatcher for BGA / Blob / Edge / Pattern (1-stage stub).
// One Params/Result struct shared across multiple algorithms; algorithm specific
// behaviour is decided via algoType ordinal.

#pragma pack(push, 8)

enum MptiBridgeAlgoKind
{
    MptiAlgo_Unknown = 0,
    MptiAlgo_BGA = 1,
    MptiAlgo_Blob = 2,
    MptiAlgo_Edge = 3,
    MptiAlgo_Pattern = 4
};

struct MptiBridgeGenericParams
{
    int     algoKind;           // MptiBridgeAlgoKind
    int     binaryMin;          // 2D binary min (0-255)
    int     binaryMax;          // 2D binary max (0-255)
    int     useInsp2D;          // (0/1)
    int     invertCheck;        // (0/1)
    int     minBlobArea;        // Minimum blob pixel count
    int     expectedCenterX;    // Part-space expected center X
    int     expectedCenterY;    // Part-space expected center Y
    float   minAreaRatio;       // ROI foreground ratio min (0..1)
    float   maxAreaRatio;       // ROI foreground ratio max (0..1)
    float   shiftXTolerance;    // pixel tolerance
    float   shiftYTolerance;    // pixel tolerance
};

struct MptiBridgeGenericResult
{
    int     isInsp;
    int     isOK;
    int     okArea;
    int     okShift;
    int     foundCenterX;
    int     foundCenterY;
    float   areaRatio;
    float   shiftX;
    float   shiftY;
    int     foregroundPixels;
    int     blobCount;
    double  elapsedMs;
    int     errorCode;
    wchar_t message[256];
};

#pragma pack(pop)
