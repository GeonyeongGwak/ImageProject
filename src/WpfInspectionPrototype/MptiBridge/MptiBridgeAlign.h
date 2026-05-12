#pragma once

// Align algorithm C++ <-> C# marshalling structs.
// All BOOL flags use int (4 bytes) for layout safety.

#pragma pack(push, 8)

struct MptiBridgeAlignParams
{
    int     searchNum;          // 1-4
    int     searchPointsX[4];   // Part-space center X (pixel)
    int     searchPointsY[4];   // Part-space center Y (pixel)
    int     searchSizeW[4];     // Search ROI width (pixel)
    int     searchSizeH[4];     // Search ROI height (pixel)
    int     searchMargin;       // Allowed position margin (pixel)
    int     minBinary;          // 2D binary min (0-255)
    int     maxBinary;          // 2D binary max (0-255)
    int     useInsp2D;          // Use 2D inspection (0/1)
    int     invertCheck;        // Invert binary (0/1)
    int     useShift;           // Check shift NG (0/1)
    double  maxShiftX;          // Max allowed shift X (pixel)
    double  maxShiftY;          // Max allowed shift Y (pixel)
    int     useAngle;           // Check angle NG (0/1)
    double  maxAngle;           // Max allowed angle (degree)
    int     sameSize;           // All search ROIs same size (0/1)
    int     minBlobArea;        // Min blob pixel count
};

struct MptiBridgeAlignResult
{
    int     okCount;            // Successful search count
    double  offsetX;            // Alignment X offset
    double  offsetY;            // Alignment Y offset
    double  theta;              // Alignment angle (degree)
    int     okShiftX;           // Shift X within tolerance (0/1)
    int     okShiftY;           // Shift Y within tolerance (0/1)
    int     okAngle;            // Angle within tolerance (0/1)
    int     detectedCentersX[4];// Detected center X (Part coords)
    int     detectedCentersY[4];// Detected center Y
    int     foregroundPixels;   // Total foreground pixel count
    int     blobCount;          // Number of detected blobs
    double  elapsedMs;
    int     errorCode;
    wchar_t message[256];
};

#pragma pack(pop)
