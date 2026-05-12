#pragma once

// ShapeX algorithm C++ <-> C# marshalling structs (lightweight first pass).
// Real ShapeX uses 200 ROIs and many flags; this prototype starts with the
// fields needed to exercise the bridge pipeline end-to-end.

#pragma pack(push, 8)

struct MptiBridgeShapeXParams
{
    int     binaryMin;          // 2D binary min (0-255)
    int     binaryMax;          // 2D binary max (0-255)
    int     useInsp2D;          // (0/1)
    int     invertCheck;        // (0/1)
    int     useShape;           // Shape check (0/1)
    int     useInner;           // Inner check (0/1)
    int     useExist;           // Exist check (0/1)
    int     useShift;           // Shift check (0/1)
    float   shapeAreaMin;       // Expected shape area min (fraction of ROI)
    float   shapeAreaMax;       // Expected shape area max (fraction of ROI)
    float   shiftXTolerance;    // pixel tolerance
    float   shiftYTolerance;    // pixel tolerance
    int     expectedCenterX;    // Part-space expected center X
    int     expectedCenterY;    // Part-space expected center Y
    int     minBlobArea;
};

struct MptiBridgeShapeXResult
{
    int     isInsp;             // Was inspection performed (0/1)
    int     isOK;               // Overall OK (0/1)
    int     okShape;            // Shape area within tolerance (0/1)
    int     okExist;            // Object detected (0/1)
    int     okShift;            // Position within tolerance (0/1)
    int     foundCenterX;       // Detected centroid X
    int     foundCenterY;       // Detected centroid Y
    float   shapeAreaRatio;     // Foreground / ROI area
    float   shiftX;             // foundCenter - expectedCenter
    float   shiftY;
    int     foregroundPixels;
    int     blobCount;
    double  elapsedMs;
    int     errorCode;
    wchar_t message[256];
};

#pragma pack(pop)
