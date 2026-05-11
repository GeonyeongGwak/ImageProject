#include "InspectionTypes.h"

#include <algorithm>
#include <chrono>
#include <cwchar>

namespace
{
    RoiRect ClampRoi(const RoiRect* roi, int width, int height)
    {
        if (roi == nullptr || roi->width <= 0 || roi->height <= 0)
        {
            return { 0, 0, width, height };
        }

        const int x = std::clamp(roi->x, 0, std::max(0, width - 1));
        const int y = std::clamp(roi->y, 0, std::max(0, height - 1));
        const int right = std::clamp(roi->x + roi->width, x + 1, width);
        const int bottom = std::clamp(roi->y + roi->height, y + 1, height);
        return { x, y, right - x, bottom - y };
    }

    void WriteMessage(InspectionResult* result, const wchar_t* message)
    {
        if (result == nullptr)
        {
            return;
        }

        wcsncpy_s(result->message, message, _TRUNCATE);
    }
}

INSPECTION_CORE_API int CoreThresholdBgraToGray8(
    const std::uint8_t* source,
    int width,
    int height,
    int sourceStride,
    std::uint8_t* destination,
    int destinationStride,
    int threshold,
    const RoiRect* roi,
    InspectionResult* result)
{
    if (result != nullptr)
    {
        *result = {};
    }

    if (source == nullptr || destination == nullptr || width <= 0 || height <= 0)
    {
        if (result != nullptr)
        {
            result->errorCode = -1;
            WriteMessage(result, L"Invalid image buffer");
        }

        return -1;
    }

    if (sourceStride < width * 4 || destinationStride < width)
    {
        if (result != nullptr)
        {
            result->errorCode = -2;
            WriteMessage(result, L"Invalid stride");
        }

        return -2;
    }

    const auto start = std::chrono::high_resolution_clock::now();
    const int clampedThreshold = std::clamp(threshold, 0, 255);
    const RoiRect area = ClampRoi(roi, width, height);

    std::fill(destination, destination + static_cast<std::size_t>(destinationStride) * height, 0);

    int foreground = 0;
    int minX = width;
    int minY = height;
    int maxX = -1;
    int maxY = -1;

    for (int y = area.y; y < area.y + area.height; ++y)
    {
        const std::uint8_t* sourceRow = source + static_cast<std::size_t>(y) * sourceStride;
        std::uint8_t* destinationRow = destination + static_cast<std::size_t>(y) * destinationStride;

        for (int x = area.x; x < area.x + area.width; ++x)
        {
            const std::uint8_t* pixel = sourceRow + x * 4;
            const int blue = pixel[0];
            const int green = pixel[1];
            const int red = pixel[2];
            const int gray = (77 * red + 150 * green + 29 * blue) >> 8;
            const std::uint8_t binary = gray >= clampedThreshold ? 255 : 0;

            destinationRow[x] = binary;

            if (binary == 0)
            {
                continue;
            }

            ++foreground;
            minX = std::min(minX, x);
            minY = std::min(minY, y);
            maxX = std::max(maxX, x);
            maxY = std::max(maxY, y);
        }
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double, std::milli> elapsed = end - start;

    if (result != nullptr)
    {
        result->foregroundPixels = foreground;
        result->blobCount = foreground > 0 ? 1 : 0;
        result->minX = foreground > 0 ? minX : 0;
        result->minY = foreground > 0 ? minY : 0;
        result->maxX = foreground > 0 ? maxX : 0;
        result->maxY = foreground > 0 ? maxY : 0;
        result->elapsedMs = elapsed.count();
        result->errorCode = 0;
        WriteMessage(result, L"Core threshold completed");
    }

    return 0;
}
