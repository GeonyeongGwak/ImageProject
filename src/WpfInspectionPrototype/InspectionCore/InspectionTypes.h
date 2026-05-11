#pragma once

#include <cstdint>

#ifdef INSPECTIONCORE_EXPORTS
#define INSPECTION_CORE_API extern "C" __declspec(dllexport)
#else
#define INSPECTION_CORE_API extern "C" __declspec(dllimport)
#endif

struct RoiRect
{
    int x;
    int y;
    int width;
    int height;
};

struct InspectionResult
{
    int foregroundPixels;
    int blobCount;
    int minX;
    int minY;
    int maxX;
    int maxY;
    double elapsedMs;
    int errorCode;
    wchar_t message[256];
};

INSPECTION_CORE_API int CoreThresholdBgraToGray8(
    const std::uint8_t* source,
    int width,
    int height,
    int sourceStride,
    std::uint8_t* destination,
    int destinationStride,
    int threshold,
    const RoiRect* roi,
    InspectionResult* result);
