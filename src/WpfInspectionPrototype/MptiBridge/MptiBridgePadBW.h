#pragma once

// PadBW algorithm C++ <-> C# marshalling structs (lightweight first pass).

#pragma pack(push, 8)

struct MptiBridgePadBWParams
{
    int     binaryMin;          // 2D binary min (0-255)
    int     binaryMax;          // 2D binary max (0-255)
    int     useInsp2D;          // (0/1)
    int     invertCheck;        // (0/1)
    int     useTeachArea;       // (0/1) - check pad area matches taught
    double  teachArea;          // Expected pad area (pixels)
    double  teachAreaRateMin;   // % min (e.g. 80)
    double  teachAreaRateMax;   // % max (e.g. 120)
    int     useShift;           // (0/1) - check pad position
    double  teachShiftX;        // pixel tolerance
    double  teachShiftY;        // pixel tolerance
    int     expectedCenterX;    // Part-space expected pad center X
    int     expectedCenterY;    // Part-space expected pad center Y
    int     useBlobArea;        // (0/1) - check minimum blob size
    double  blobAreaMin;        // pixel
    int     useFillHole;        // (0/1)
    int     filterLevel;
    int     minBlobArea;
};

struct MptiBridgePadBWResult
{
    int     isInsp;
    int     isOK;
    int     okArea;             // teach area within tolerance (0/1)
    int     okShiftX;
    int     okShiftY;
    int     okBlobArea;
    int     foundCenterX;
    int     foundCenterY;
    double  measuredArea;
    double  measuredAreaRate;   // measured / teachArea * 100
    double  shiftX;
    double  shiftY;
    int     foregroundPixels;
    int     blobCount;
    double  elapsedMs;
    int     errorCode;
    wchar_t message[256];
};

#pragma pack(pop)
