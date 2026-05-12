#include "stdafx.h"
#include "MPTI.h"
#include "MptiBridgeAlign.h"

#include <oleauto.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cwchar>
#include <exception>

#define MPTI_BRIDGE_API extern "C" __declspec(dllexport)

int MPTI_SetFovPixelNumber(int numX, int numY);
int MPTI_SetFovPixelResolution(double resolX, double resolY);

namespace
{
    constexpr int MessageLength = 512;
    bool g_initialized = false;

    struct ScopedBstr
    {
        explicit ScopedBstr(const wchar_t* value)
            : value(SysAllocString(value == nullptr ? L"" : value))
        {
        }

        ~ScopedBstr()
        {
            SysFreeString(value);
        }

        BSTR value;
    };

    void CopyMessage(wchar_t* output, int outputLength, const wchar_t* message)
    {
        if (output == nullptr || outputLength <= 0)
        {
            return;
        }

        wcsncpy_s(output, static_cast<size_t>(outputLength), message == nullptr ? L"" : message, _TRUNCATE);
    }

    int Failure(wchar_t* message, int messageLength, int code, const wchar_t* failureMessage)
    {
        CopyMessage(message, messageLength, failureMessage);
        return code;
    }

    int Success(wchar_t* message, int messageLength, const wchar_t* successMessage)
    {
        CopyMessage(message, messageLength, successMessage);
        return 0;
    }

    bool EnsureInitialized(wchar_t* message, int messageLength)
    {
        if (g_initialized)
        {
            return true;
        }

        const auto code = MPTI_Init();
        if (code != 0 && code != 1)
        {
            return Failure(message, messageLength, code, L"MPTI_Init returned failure") == 0;
        }

        g_initialized = true;
        return true;
    }

    int GuardedUnknownFailure(wchar_t* message, int messageLength, const wchar_t* functionName)
    {
        wchar_t buffer[MessageLength]{};
        swprintf_s(buffer, L"%s unknown C++ exception", functionName);
        return Failure(message, messageLength, -101, buffer);
    }
}

MPTI_BRIDGE_API int MptiBridgeGetVersion(wchar_t* output, int outputLength)
{
    CopyMessage(output, outputLength, L"MPTI bridge 0.3 | backend MPTILib_Algo source-linked");
    return 0;
}

MPTI_BRIDGE_API int MptiBridgeSetMachineMode(int mode, int teach, wchar_t* message, int messageLength)
{
    UNREFERENCED_PARAMETER(teach);

    try
    {
        if (!EnsureInitialized(message, messageLength))
        {
            return -200;
        }

        MPTI_SetRawDataMachineType(mode);
        return Success(message, messageLength, L"MPTI machine mode applied");
    }
    catch (const std::exception&)
    {
        return Failure(message, messageLength, -100, L"MPTI machine mode C++ exception");
    }
    catch (...)
    {
        return GuardedUnknownFailure(message, messageLength, L"MPTI machine mode");
    }
}

MPTI_BRIDGE_API int MptiBridgeLoadPtt(const wchar_t* pttPath, int* width, int* height, int callSpi, int useFactor, wchar_t* message, int messageLength)
{
    UNREFERENCED_PARAMETER(useFactor);

    if (pttPath == nullptr || width == nullptr || height == nullptr)
    {
        return Failure(message, messageLength, -1, L"MptiBridgeLoadPtt invalid argument");
    }

    try
    {
        if (!EnsureInitialized(message, messageLength))
        {
            return -200;
        }

        ScopedBstr path(pttPath);
        *width = 0;
        *height = 0;
        const auto code = MPTI_GetPttFileLoad(path.value, width, height, callSpi != 0);
        if (code != 0 && code != 1)
        {
            return Failure(message, messageLength, code, L"MPTI_GetPttFileLoad returned failure");
        }

        return Success(message, messageLength, L"MPTI_GetPttFileLoad completed");
    }
    catch (const std::exception&)
    {
        return Failure(message, messageLength, -100, L"MPTI_GetPttFileLoad C++ exception");
    }
    catch (...)
    {
        return GuardedUnknownFailure(message, messageLength, L"MPTI_GetPttFileLoad");
    }
}

MPTI_BRIDGE_API int MptiBridgeLoadPot(const wchar_t* potPath, int* width, int* height, float* pixelResX, float* pixelResY, wchar_t* message, int messageLength)
{
    UNREFERENCED_PARAMETER(potPath);

    if (width != nullptr)
    {
        *width = 0;
    }

    if (height != nullptr)
    {
        *height = 0;
    }

    if (pixelResX != nullptr)
    {
        *pixelResX = 0;
    }

    if (pixelResY != nullptr)
    {
        *pixelResY = 0;
    }

    return Failure(message, messageLength, -202, L"MPTILib_Algo backend has no MPTI_GetPotFileLoad export");
}

MPTI_BRIDGE_API int MptiBridgeSetRawDataFovInfo(
    const wchar_t* pttPath,
    const wchar_t* potPath,
    int x,
    int y,
    int width,
    int height,
    int separatedPart,
    int* resultNumber,
    wchar_t* message,
    int messageLength)
{
    if (pttPath == nullptr || resultNumber == nullptr)
    {
        return Failure(message, messageLength, -1, L"MptiBridgeSetRawDataFovInfo invalid argument");
    }

    try
    {
        if (!EnsureInitialized(message, messageLength))
        {
            return -200;
        }

        ScopedBstr ptt(pttPath);
        ScopedBstr pot(potPath == nullptr ? L"" : potPath);
        auto fovWidth = width;
        auto fovHeight = height;
        const auto code = MPTI_SetRawDataFovInfo(ptt.value, pot.value, x, y, separatedPart != 0, &fovWidth, &fovHeight);
        *resultNumber = fovWidth > 0 && fovHeight > 0 ? 1 : 0;
        if (code != 0 && code != 1)
        {
            return Failure(message, messageLength, code, L"MPTI_SetRawDataFovInfo returned failure");
        }

        return Success(message, messageLength, L"MPTI_SetRawDataFovInfo completed");
    }
    catch (const std::exception&)
    {
        return Failure(message, messageLength, -100, L"MPTI_SetRawDataFovInfo C++ exception");
    }
    catch (...)
    {
        return GuardedUnknownFailure(message, messageLength, L"MPTI_SetRawDataFovInfo");
    }
}

MPTI_BRIDGE_API int MptiBridgeGetRawDataZmapInfo(int* width, int* height, int* hasPointer, wchar_t* message, int messageLength)
{
    if (width == nullptr || height == nullptr || hasPointer == nullptr)
    {
        return Failure(message, messageLength, -1, L"MptiBridgeGetRawDataZmapInfo invalid argument");
    }

    try
    {
        if (!EnsureInitialized(message, messageLength))
        {
            return -200;
        }

        *width = MPTI_GetRawDataZmapWidth();
        *height = MPTI_GetRawDataZmapHeight();
        *hasPointer = MPTI_GetRawDataZmap() == nullptr ? 0 : 1;
        return Success(message, messageLength, L"MPTI raw data zmap info read");
    }
    catch (const std::exception&)
    {
        return Failure(message, messageLength, -100, L"MPTI raw data zmap C++ exception");
    }
    catch (...)
    {
        return GuardedUnknownFailure(message, messageLength, L"MPTI raw data zmap");
    }
}

MPTI_BRIDGE_API int MptiBridgeSetResolForInsp(int fovWidth, int fovLength, double resolX, double resolY, wchar_t* message, int messageLength)
{
    try
    {
        if (!EnsureInitialized(message, messageLength))
        {
            return -200;
        }

        const auto numberCode = MPTI_SetFovPixelNumber(fovWidth, fovLength);
        if (numberCode != 0 && numberCode != 1)
        {
            return Failure(message, messageLength, numberCode, L"MPTI_SetFovPixelNumber returned failure");
        }

        const auto resolutionCode = MPTI_SetFovPixelResolution(resolX, resolY);
        if (resolutionCode != 0 && resolutionCode != 1)
        {
            return Failure(message, messageLength, resolutionCode, L"MPTI_SetFovPixelResolution returned failure");
        }

        return Success(message, messageLength, L"MPTI FOV resolution applied");
    }
    catch (const std::exception&)
    {
        return Failure(message, messageLength, -100, L"MPTI FOV resolution C++ exception");
    }
    catch (...)
    {
        return GuardedUnknownFailure(message, messageLength, L"MPTI FOV resolution");
    }
}

namespace
{
    template <typename T>
    inline T ClampValue(T value, T low, T high)
    {
        return value < low ? low : (value > high ? high : value);
    }

    struct AlignSearchHit
    {
        int centerX;
        int centerY;
        int blobPixels;
        bool found;
    };

    AlignSearchHit FindCentroidInSearchRoi(
        const unsigned char* image,
        int imageWidth,
        int imageHeight,
        int sourceStride,
        int searchLeft,
        int searchTop,
        int searchRight,
        int searchBottom,
        int minBinary,
        int maxBinary,
        int invertCheck,
        int minBlobArea)
    {
        AlignSearchHit hit{};

        const int sx0 = std::max(0, searchLeft);
        const int sy0 = std::max(0, searchTop);
        const int sx1 = std::min(imageWidth, searchRight);
        const int sy1 = std::min(imageHeight, searchBottom);

        if (sx0 >= sx1 || sy0 >= sy1)
        {
            return hit;
        }

        const int minB = ClampValue(minBinary, 0, 255);
        const int maxB = ClampValue(std::max(minB, maxBinary), 0, 255);
        const bool invert = invertCheck != 0;

        long long sumX = 0;
        long long sumY = 0;
        int count = 0;

        for (int y = sy0; y < sy1; ++y)
        {
            const unsigned char* row = image + static_cast<size_t>(y) * sourceStride;
            for (int x = sx0; x < sx1; ++x)
            {
                const unsigned char* px = row + x * 4;
                const int blue = px[0];
                const int green = px[1];
                const int red = px[2];
                const int gray = (77 * red + 150 * green + 29 * blue) >> 8;
                const bool match = invert ? (gray < minB || gray > maxB)
                                          : (gray >= minB && gray <= maxB);
                if (!match)
                {
                    continue;
                }

                sumX += x;
                sumY += y;
                ++count;
            }
        }

        if (count >= std::max(1, minBlobArea))
        {
            hit.centerX = static_cast<int>(sumX / count);
            hit.centerY = static_cast<int>(sumY / count);
            hit.blobPixels = count;
            hit.found = true;
        }

        return hit;
    }
}

MPTI_BRIDGE_API int MptiBridgeRunAlign(
    const unsigned char* image,
    int imageWidth,
    int imageHeight,
    int sourceStride,
    int windowX,
    int windowY,
    int windowW,
    int windowH,
    const MptiBridgeAlignParams* params,
    MptiBridgeAlignResult* result)
{
    if (result == nullptr)
    {
        return -1;
    }

    *result = {};
    wcsncpy_s(result->message, L"", _TRUNCATE);

    if (image == nullptr || params == nullptr)
    {
        result->errorCode = -1;
        wcsncpy_s(result->message, L"MptiBridgeRunAlign: null pointer", _TRUNCATE);
        return -1;
    }

    if (imageWidth <= 0 || imageHeight <= 0 || sourceStride < imageWidth * 4)
    {
        result->errorCode = -2;
        wcsncpy_s(result->message, L"MptiBridgeRunAlign: invalid image dimensions", _TRUNCATE);
        return -2;
    }

    const int searchNum = ClampValue(params->searchNum, 1, 4);

    try
    {
        const auto start = std::chrono::high_resolution_clock::now();

        long long sumDeltaX = 0;
        long long sumDeltaY = 0;
        int okCount = 0;
        int totalForeground = 0;

        for (int i = 0; i < searchNum; ++i)
        {
            const int margin = std::max(0, params->searchMargin);
            const int sizeW = std::max(1, params->searchSizeW[i]);
            const int sizeH = std::max(1, params->searchSizeH[i]);

            const int searchLeft = params->searchPointsX[i] - sizeW / 2 - margin;
            const int searchTop = params->searchPointsY[i] - sizeH / 2 - margin;
            const int searchRight = params->searchPointsX[i] + sizeW / 2 + margin;
            const int searchBottom = params->searchPointsY[i] + sizeH / 2 + margin;

            const auto hit = FindCentroidInSearchRoi(
                image, imageWidth, imageHeight, sourceStride,
                searchLeft, searchTop, searchRight, searchBottom,
                params->minBinary, params->maxBinary, params->invertCheck,
                params->minBlobArea);

            if (hit.found)
            {
                result->detectedCentersX[i] = hit.centerX;
                result->detectedCentersY[i] = hit.centerY;
                sumDeltaX += static_cast<long long>(hit.centerX) - params->searchPointsX[i];
                sumDeltaY += static_cast<long long>(hit.centerY) - params->searchPointsY[i];
                totalForeground += hit.blobPixels;
                ++okCount;
            }
            else
            {
                result->detectedCentersX[i] = params->searchPointsX[i];
                result->detectedCentersY[i] = params->searchPointsY[i];
            }
        }

        if (okCount > 0)
        {
            result->offsetX = static_cast<double>(sumDeltaX) / okCount;
            result->offsetY = static_cast<double>(sumDeltaY) / okCount;
        }

        if (okCount >= 2)
        {
            // 첫 2 hit으로 회전각 추정
            int firstIdx = -1;
            int secondIdx = -1;
            for (int i = 0; i < searchNum; ++i)
            {
                if (result->detectedCentersX[i] != params->searchPointsX[i] ||
                    result->detectedCentersY[i] != params->searchPointsY[i])
                {
                    if (firstIdx < 0) firstIdx = i;
                    else if (secondIdx < 0) { secondIdx = i; break; }
                }
            }

            if (firstIdx >= 0 && secondIdx >= 0)
            {
                const double refDx = params->searchPointsX[secondIdx] - params->searchPointsX[firstIdx];
                const double refDy = params->searchPointsY[secondIdx] - params->searchPointsY[firstIdx];
                const double detDx = result->detectedCentersX[secondIdx] - result->detectedCentersX[firstIdx];
                const double detDy = result->detectedCentersY[secondIdx] - result->detectedCentersY[firstIdx];
                const double refAng = std::atan2(refDy, refDx);
                const double detAng = std::atan2(detDy, detDx);
                result->theta = (detAng - refAng) * 180.0 / 3.14159265358979323846;
            }
        }

        result->okCount = okCount;
        result->foregroundPixels = totalForeground;
        result->blobCount = okCount;

        const bool checkShift = params->useShift != 0;
        const bool checkAngle = params->useAngle != 0;
        result->okShiftX = (!checkShift || std::abs(result->offsetX) <= params->maxShiftX) ? 1 : 0;
        result->okShiftY = (!checkShift || std::abs(result->offsetY) <= params->maxShiftY) ? 1 : 0;
        result->okAngle = (!checkAngle || std::abs(result->theta) <= params->maxAngle) ? 1 : 0;

        const auto end = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<double, std::milli> elapsed = end - start;
        result->elapsedMs = elapsed.count();
        result->errorCode = 0;
        wcsncpy_s(result->message, L"MptiBridgeRunAlign completed (lightweight)", _TRUNCATE);
        return 0;
    }
    catch (const std::exception&)
    {
        result->errorCode = -100;
        wcsncpy_s(result->message, L"MptiBridgeRunAlign C++ exception", _TRUNCATE);
        return -100;
    }
    catch (...)
    {
        result->errorCode = -101;
        wcsncpy_s(result->message, L"MptiBridgeRunAlign unknown C++ exception", _TRUNCATE);
        return -101;
    }
}
