#include "stdafx.h"
#include "MPTI.h"
#include "MptiBridgeShapeX.h"
#include "MptiBridgePadBW.h"
#include "MptiBridgeGeneric.h"
#include "NativeSources/MPTILib_Algo/PInsp_Algo/Align/PInsp_AlgoAlign.h"

#include <oleauto.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cwchar>
#include <exception>

#define MPTI_BRIDGE_API extern "C" __declspec(dllexport)

extern "C" IMAGE_DOS_HEADER __ImageBase;

#pragma pack(push, 8)
struct MptiBridgeAlignParams
{
    int     searchNum;
    int     searchPointsX[4];
    int     searchPointsY[4];
    int     searchSizeW[4];
    int     searchSizeH[4];
    int     searchMargin;
    int     minBinary;
    int     maxBinary;
    int     useInsp2D;
    int     invertCheck;
    int     useShift;
    double  maxShiftX;
    double  maxShiftY;
    int     useAngle;
    double  maxAngle;
    int     sameSize;
    int     minBlobArea;
};

struct MptiBridgeAlignResult
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
    int     foregroundPixels;
    int     blobCount;
    double  elapsedMs;
    int     errorCode;
    wchar_t message[256];
};

struct MptiBridgeLightPreviewParams
{
    int width;
    int height;
    int lightType;
    int redValue;
    int greenValue;
    int blueValue;
    int whiteValue;
    int lightCount;
    int arrRedValue[10];
    int arrGreenValue[10];
    int arrBlueValue[10];
    int arrWhiteValue[10];
    int arrLightPosition[10];
    int arrCalculation[10];
};
#pragma pack(pop)

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

MPTI_BRIDGE_API int MptiBridgeDebugProbe(int breakIntoDebugger, wchar_t* output, int outputLength)
{
    wchar_t modulePath[MAX_PATH]{};
    GetModuleFileNameW(reinterpret_cast<HMODULE>(&__ImageBase), modulePath, MAX_PATH);

    wchar_t message[1024]{};
    swprintf_s(
        message,
        L"MptiBridgeDebugProbe | debugger=%d | module=%s",
        IsDebuggerPresent() ? 1 : 0,
        modulePath);
    CopyMessage(output, outputLength, message);

    if (breakIntoDebugger != 0 && IsDebuggerPresent())
    {
        DebugBreak();
    }

    return IsDebuggerPresent() ? 1 : 0;
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

namespace
{
    constexpr int LightTop = 0;
    constexpr int LightMiddle = 1;
    constexpr int LightBottom = 2;
    constexpr int LightUser = 3;
    constexpr int LightSideRed = 4;
    constexpr int LightSideGreen = 5;
    constexpr int LightSideBlue = 6;

    constexpr int ChannelTR = 0;
    constexpr int ChannelTG = 1;
    constexpr int ChannelTB = 2;
    constexpr int ChannelBR = 3;
    constexpr int ChannelBB = 4;
    constexpr int ChannelMR = 5;
    constexpr int ChannelMB = 6;
    constexpr int ChannelTW = 7;

    static int ClampInt(int value, int minValue, int maxValue)
    {
        return std::max(minValue, std::min(maxValue, value));
    }

    static UCHAR* LoadedPttChannel(int channel)
    {
        return MPTI_GetPttFileChannel(channel);
    }

    static UCHAR* RequiredChannel(int channel, wchar_t* message, int messageLength)
    {
        auto* buffer = LoadedPttChannel(channel);
        if (buffer == nullptr)
        {
            Failure(message, messageLength, -4, L"MPTI PTT channel buffer is empty");
        }

        return buffer;
    }

    static int ComposeNormalLightPreview(
        const MptiBridgeLightPreviewParams* params,
        UCHAR* output,
        int imageSize,
        wchar_t* message,
        int messageLength)
    {
        UCHAR* red = nullptr;
        UCHAR* green = nullptr;
        UCHAR* blue = nullptr;
        UCHAR* white = nullptr;

        switch (params->lightType)
        {
        case LightMiddle:
            red = LoadedPttChannel(ChannelMR);
            blue = LoadedPttChannel(ChannelMB);
            green = red;
            white = red;
            break;
        case LightBottom:
            red = LoadedPttChannel(ChannelBR);
            blue = LoadedPttChannel(ChannelBB);
            green = red;
            white = red;
            break;
        case LightSideRed:
        case LightSideGreen:
        case LightSideBlue:
            red = LoadedPttChannel(ChannelTR);
            green = LoadedPttChannel(ChannelTG);
            blue = LoadedPttChannel(ChannelTB);
            white = red;
            break;
        case LightTop:
        default:
            red = LoadedPttChannel(ChannelTR);
            green = LoadedPttChannel(ChannelTG);
            blue = LoadedPttChannel(ChannelTB);
            white = LoadedPttChannel(ChannelTW);
            break;
        }

        if (red == nullptr)
        {
            return Failure(message, messageLength, -5, L"MPTI PTT red channel buffer is empty");
        }

        green = green == nullptr ? red : green;
        blue = blue == nullptr ? red : blue;
        white = white == nullptr ? red : white;

        MPTI_FULLImgCompose(
            red,
            green,
            blue,
            white,
            imageSize,
            params->width,
            params->height,
            0.0,
            0.0,
            0,
            0,
            ClampInt(params->redValue, 0, 200),
            ClampInt(params->greenValue, 0, 200),
            ClampInt(params->blueValue, 0, 200),
            ClampInt(params->whiteValue, 0, 200),
            output);

        return 0;
    }

    static int ComposeUserLightPreview(
        const MptiBridgeLightPreviewParams* params,
        UCHAR* output,
        wchar_t* message,
        int messageLength)
    {
        auto* tr = RequiredChannel(ChannelTR, message, messageLength);
        auto* tg = RequiredChannel(ChannelTG, message, messageLength);
        auto* tb = RequiredChannel(ChannelTB, message, messageLength);
        auto* tw = LoadedPttChannel(ChannelTW);
        auto* mr = LoadedPttChannel(ChannelMR);
        auto* mb = LoadedPttChannel(ChannelMB);
        auto* br = LoadedPttChannel(ChannelBR);
        auto* bb = LoadedPttChannel(ChannelBB);

        if (tr == nullptr || tg == nullptr || tb == nullptr)
        {
            return -4;
        }

        int red[10] = {};
        int green[10] = {};
        int blue[10] = {};
        int white[10] = {};
        int position[10] = {};
        int calculation[10] = {};

        const auto count = ClampInt(params->lightCount, 1, 10);
        for (int i = 0; i < count; ++i)
        {
            red[i] = ClampInt(params->arrRedValue[i], -1, 200);
            green[i] = ClampInt(params->arrGreenValue[i], -1, 200);
            blue[i] = ClampInt(params->arrBlueValue[i], -1, 200);
            white[i] = ClampInt(params->arrWhiteValue[i], -1, 200);
            position[i] = ClampInt(params->arrLightPosition[i], LightTop, LightBottom);
            calculation[i] = ClampInt(params->arrCalculation[i], 0, 2);
        }

        LightTypeBuf lightImage;
        lightImage.m_pucTRed = tr;
        lightImage.m_pucTGreen = tg;
        lightImage.m_pucTBlue = tb;
        lightImage.m_pucTWhite = tw == nullptr ? tr : tw;
        lightImage.m_pucMRed = mr == nullptr ? tr : mr;
        lightImage.m_pucMBlue = mb == nullptr ? tr : mb;
        lightImage.m_pucBRed = br == nullptr ? tr : br;
        lightImage.m_pucBBlue = bb == nullptr ? tr : bb;
        lightImage.m_nImgCnt = count;
        lightImage.m_nImgWidth = params->width;
        lightImage.m_nImgHeight = params->height;
        lightImage.m_pnRedValue = red;
        lightImage.m_pnGreenValue = green;
        lightImage.m_pnBlueValue = blue;
        lightImage.m_pnWhiteValue = white;
        lightImage.m_pnPosition = position;
        lightImage.m_pnCalculation = calculation;

        MPTI_FullImageClaculCompose(lightImage, output);
        return 0;
    }
}

// CAM-01 컬러 미리보기. PTT 의 TR/TG/TB 채널을 그대로 BGRA32 로 합성한다.
// Light 슬라이더(redValue/greenValue/blueValue)에는 영향을 받지 않으며, 사용자가 명시한
// 고정 가중치 R=G=B=100 즉 baseline 강도로 합성한다. 가중치는 200 이 max 이고 100 이
// 1.0 배 (즉 채널 그대로) 이므로 별도 스케일링이 필요 없다 (cv::merge 와 동등 결과).
MPTI_BRIDGE_API int MptiBridgeRenderColorPreview(
    int width,
    int height,
    unsigned char* output,
    int outputLength,
    wchar_t* message,
    int messageLength)
{
    if (output == nullptr)
    {
        return Failure(message, messageLength, -1, L"MptiBridgeRenderColorPreview invalid argument");
    }

    if (width <= 0 || height <= 0)
    {
        return Failure(message, messageLength, -2, L"MptiBridgeRenderColorPreview invalid image dimensions");
    }

    const auto pixelCount = width * height;
    const auto requiredBytes = pixelCount * 4;
    if (pixelCount <= 0 || outputLength < requiredBytes)
    {
        return Failure(message, messageLength, -3, L"MptiBridgeRenderColorPreview output buffer too small");
    }

    try
    {
        if (!EnsureInitialized(message, messageLength))
        {
            return -200;
        }

        auto* red = LoadedPttChannel(ChannelTR);
        auto* green = LoadedPttChannel(ChannelTG);
        auto* blue = LoadedPttChannel(ChannelTB);
        if (red == nullptr || green == nullptr || blue == nullptr)
        {
            return Failure(message, messageLength, -4, L"MPTI PTT RGB channel buffer is empty");
        }

        // BGRA32: [B, G, R, A] per pixel. Light 값과 무관하게 채널 raw 값 그대로 사용.
        for (int i = 0; i < pixelCount; ++i)
        {
            const auto offset = i * 4;
            output[offset + 0] = blue[i];
            output[offset + 1] = green[i];
            output[offset + 2] = red[i];
            output[offset + 3] = 255;
        }

        return Success(message, messageLength, L"MptiBridgeRenderColorPreview completed");
    }
    catch (const std::exception&)
    {
        return Failure(message, messageLength, -100, L"MptiBridgeRenderColorPreview C++ exception");
    }
    catch (...)
    {
        return GuardedUnknownFailure(message, messageLength, L"MptiBridgeRenderColorPreview");
    }
}

MPTI_BRIDGE_API int MptiBridgeRenderLightPreview(
    const MptiBridgeLightPreviewParams* params,
    unsigned char* output,
    int outputLength,
    wchar_t* message,
    int messageLength)
{
    if (params == nullptr || output == nullptr)
    {
        return Failure(message, messageLength, -1, L"MptiBridgeRenderLightPreview invalid argument");
    }

    if (params->width <= 0 || params->height <= 0)
    {
        return Failure(message, messageLength, -2, L"MptiBridgeRenderLightPreview invalid image dimensions");
    }

    const auto imageSize = params->width * params->height;
    if (imageSize <= 0 || outputLength < imageSize)
    {
        return Failure(message, messageLength, -3, L"MptiBridgeRenderLightPreview output buffer too small");
    }

    try
    {
        if (!EnsureInitialized(message, messageLength))
        {
            return -200;
        }

        std::fill(output, output + imageSize, 0);
        const auto code = params->lightType == LightUser
            ? ComposeUserLightPreview(params, output, message, messageLength)
            : ComposeNormalLightPreview(params, output, imageSize, message, messageLength);
        if (code != 0)
        {
            return code;
        }

        return Success(message, messageLength, L"MptiBridgeRenderLightPreview completed");
    }
    catch (const std::exception&)
    {
        return Failure(message, messageLength, -100, L"MptiBridgeRenderLightPreview C++ exception");
    }
    catch (...)
    {
        return GuardedUnknownFailure(message, messageLength, L"MptiBridgeRenderLightPreview");
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

static int MptiBridgeRunAlignLightweight(
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
    UNREFERENCED_PARAMETER(windowX);
    UNREFERENCED_PARAMETER(windowY);
    UNREFERENCED_PARAMETER(windowW);
    UNREFERENCED_PARAMETER(windowH);

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

    try
    {
        if (!EnsureInitialized(result->message, static_cast<int>(std::size(result->message))))
        {
            result->errorCode = -200;
            return -200;
        }

        const auto start = std::chrono::high_resolution_clock::now();
        const int searchNum = ClampValue(params->searchNum, 1, 4);

        std::vector<unsigned char> gray(static_cast<size_t>(imageWidth) * imageHeight);
        for (int y = 0; y < imageHeight; ++y)
        {
            const auto* src = image + static_cast<size_t>(y) * sourceStride;
            auto* dst = gray.data() + static_cast<size_t>(y) * imageWidth;
            for (int x = 0; x < imageWidth; ++x)
            {
                const auto b = src[x * 4 + 0];
                const auto g = src[x * 4 + 1];
                const auto r = src[x * 4 + 2];
                dst[x] = static_cast<unsigned char>((static_cast<int>(r) + g + b) / 3);
            }
        }

        AlgoAlign align{};
        align.m_nSearchNum = searchNum;
        align.m_nSearchMargin = std::max(0, params->searchMargin);
        align.m_nMinBinary = ClampValue(params->minBinary, 0, 255);
        align.m_nMaxBinary = ClampValue(params->maxBinary, 0, 255);
        align.m_bInsp2D = params->useInsp2D != 0;
        align.m_bInsp3D = FALSE;
        align.m_InvertCheck = params->invertCheck != 0;
        align.m_bUseShift = params->useShift != 0;
        align.m_dShiftX = params->maxShiftX;
        align.m_dShiftY = params->maxShiftY;
        align.m_bUseAngle = params->useAngle != 0;
        align.m_dAngle = params->maxAngle;
        align.m_bSameSize = params->sameSize != 0;
        align.m_nMinBlobArea = std::max(1, params->minBlobArea);

        for (int i = 0; i < searchNum; ++i)
        {
            align.m_sArrSearchPoint[i].x = static_cast<float>(params->searchPointsX[i] - imageWidth / 2.0);
            align.m_sArrSearchPoint[i].y = static_cast<float>(params->searchPointsY[i] - imageHeight / 2.0);
            align.m_sArrSearchSize[i].cx = std::max(1, params->searchSizeW[i]);
            align.m_sArrSearchSize[i].cy = std::max(1, params->searchSizeH[i]);
        }

        InspAlgo inspAlgo{};
        inspAlgo.m_eAlgoType = eAlgoAlign;
        inspAlgo.m_bAlgoEnable = TRUE;
        inspAlgo.m_ptrInspAlgoParam = &align;

        CRect bodyRect(0, 0, 0, 0);
        InspAlgoParam algoParam{};
        algoParam.m_bInspection = TRUE;
        algoParam.m_bUSeLeadAlign = FALSE;
        algoParam.m_rcBlobBody = &bodyRect;

        if (g_pMPTI != nullptr &&
            g_pInspMng != nullptr &&
            g_pInspMng->GetPtrInspAlgo() != nullptr &&
            g_pInspMng->GetPtrInspAlgo()->GetProcMil() == nullptr)
        {
            g_pInspMng->GetPtrInspAlgo()->InitDevice(
                g_pMPTI->m_milApp,
                g_pMPTI->m_milSys,
                imageWidth,
                imageHeight,
                1.0,
                1.0,
                g_pMPTI->isUseImagePilLib());
        }

        WndAlgoImg wndAlgoImg;
        wndAlgoImg.m_bIs2dCV = TRUE;
        wndAlgoImg.m_nWidth = imageWidth;
        wndAlgoImg.m_nHeight = imageHeight;
        wndAlgoImg.m_nWidth3D = imageWidth;
        wndAlgoImg.m_nHeight3D = imageHeight;
        wndAlgoImg.m_nChannel = 1;
        wndAlgoImg.m_nLight_index = eM2C_TR;
        Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &wndAlgoImg.m_ucArr2D, imageWidth * imageHeight);
        memcpy(wndAlgoImg.m_ucArr2D, gray.data(), static_cast<size_t>(imageWidth) * imageHeight);

        InspRoiImgBuf roiImg{};
        roiImg.nImageSizeX = imageWidth;
        roiImg.nImageSizeY = imageHeight;
        roiImg.imgTop_R = wndAlgoImg.m_ucArr2D;

        TotalInspExceptArea tieArea{};
        RstAlgoAlign alignResult{};

        CPInsp_AlgoAlign alignRunner;
        alignRunner.InitAlgo();
        const BOOL nativeOk = alignRunner.InspAlgorithm(
            inspAlgo,
            wndAlgoImg,
            &roiImg,
            &alignResult,
            tieArea,
            algoParam,
            nullptr);

        result->okCount = alignResult.m_nOKAreaCnt;
        result->offsetX = alignResult.m_dOffset_x;
        result->offsetY = alignResult.m_dOffset_y;
        result->theta = alignResult.m_dTheta;
        result->okShiftX = alignResult.m_bOKShiftX ? 1 : 0;
        result->okShiftY = alignResult.m_bOKShiftY ? 1 : 0;
        result->okAngle = alignResult.m_bOKAngle ? 1 : 0;
        result->blobCount = alignResult.m_nOKAreaCnt;

        for (int i = 0; i < searchNum; ++i)
        {
            result->detectedCentersX[i] = (alignResult.m_rcRect_I[i].left + alignResult.m_rcRect_I[i].right) / 2;
            result->detectedCentersY[i] = (alignResult.m_rcRect_I[i].top + alignResult.m_rcRect_I[i].bottom) / 2;
        }

        const auto end = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<double, std::milli> elapsed = end - start;
        result->elapsedMs = elapsed.count();
        result->errorCode = nativeOk ? 0 : -10;
        wcsncpy_s(result->message, nativeOk ? L"MptiBridgeRunAlign completed via PInsp_Algo/Align" : L"MptiBridgeRunAlign PInsp_Algo/Align returned NG", _TRUNCATE);
        return nativeOk ? 0 : -10;
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

namespace
{
    struct AlgorithmRoiAnalysis
    {
        int foregroundPixels;
        int centroidX;
        int centroidY;
        int minX;
        int minY;
        int maxX;
        int maxY;
    };

    AlgorithmRoiAnalysis AnalyseRoiForeground(
        const unsigned char* image,
        int imageWidth,
        int imageHeight,
        int sourceStride,
        int roiX,
        int roiY,
        int roiW,
        int roiH,
        int binaryMin,
        int binaryMax,
        int invertCheck)
    {
        AlgorithmRoiAnalysis info{};
        info.minX = imageWidth;
        info.minY = imageHeight;
        info.maxX = -1;
        info.maxY = -1;

        const int sx0 = std::max(0, roiX);
        const int sy0 = std::max(0, roiY);
        const int sx1 = std::min(imageWidth, roiX + roiW);
        const int sy1 = std::min(imageHeight, roiY + roiH);
        if (sx0 >= sx1 || sy0 >= sy1)
        {
            return info;
        }

        const int minB = ClampValue(binaryMin, 0, 255);
        const int maxB = ClampValue(std::max(minB, binaryMax), 0, 255);
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
                const int gray = (77 * px[2] + 150 * px[1] + 29 * px[0]) >> 8;
                const bool match = invert ? (gray < minB || gray > maxB)
                                          : (gray >= minB && gray <= maxB);
                if (!match)
                {
                    continue;
                }

                sumX += x;
                sumY += y;
                ++count;
                info.minX = std::min(info.minX, x);
                info.minY = std::min(info.minY, y);
                info.maxX = std::max(info.maxX, x);
                info.maxY = std::max(info.maxY, y);
            }
        }

        info.foregroundPixels = count;
        if (count > 0)
        {
            info.centroidX = static_cast<int>(sumX / count);
            info.centroidY = static_cast<int>(sumY / count);
        }
        return info;
    }
}

MPTI_BRIDGE_API int MptiBridgeRunShapeX(
    const unsigned char* image,
    int imageWidth,
    int imageHeight,
    int sourceStride,
    int roiX,
    int roiY,
    int roiW,
    int roiH,
    const MptiBridgeShapeXParams* params,
    MptiBridgeShapeXResult* result)
{
    if (result == nullptr)
    {
        return -1;
    }
    *result = {};
    wcsncpy_s(result->message, L"", _TRUNCATE);

    if (image == nullptr || params == nullptr || imageWidth <= 0 || imageHeight <= 0 || sourceStride < imageWidth * 4)
    {
        result->errorCode = -1;
        wcsncpy_s(result->message, L"MptiBridgeRunShapeX: invalid argument", _TRUNCATE);
        return -1;
    }

    try
    {
        const auto start = std::chrono::high_resolution_clock::now();

        const auto info = AnalyseRoiForeground(
            image, imageWidth, imageHeight, sourceStride,
            roiX, roiY, roiW, roiH,
            params->binaryMin, params->binaryMax, params->invertCheck);

        const double roiArea = static_cast<double>(std::max(1, roiW)) * std::max(1, roiH);
        const double ratio = roiArea > 0 ? static_cast<double>(info.foregroundPixels) / roiArea : 0.0;

        result->isInsp = 1;
        result->foregroundPixels = info.foregroundPixels;
        result->blobCount = info.foregroundPixels > 0 ? 1 : 0;
        result->foundCenterX = info.centroidX;
        result->foundCenterY = info.centroidY;
        result->shapeAreaRatio = static_cast<float>(ratio);

        const bool foundObject = info.foregroundPixels >= std::max(1, params->minBlobArea);
        result->okExist = (params->useExist == 0 || foundObject) ? 1 : 0;
        result->okShape = (params->useShape == 0 ||
                           (static_cast<float>(ratio) >= params->shapeAreaMin && static_cast<float>(ratio) <= params->shapeAreaMax))
                          ? 1 : 0;

        if (foundObject)
        {
            result->shiftX = static_cast<float>(info.centroidX - params->expectedCenterX);
            result->shiftY = static_cast<float>(info.centroidY - params->expectedCenterY);
        }
        result->okShift = (params->useShift == 0 ||
                          (std::fabs(result->shiftX) <= params->shiftXTolerance &&
                           std::fabs(result->shiftY) <= params->shiftYTolerance))
                         ? 1 : 0;

        result->isOK = result->okExist && result->okShape && result->okShift ? 1 : 0;

        const auto end = std::chrono::high_resolution_clock::now();
        result->elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();
        result->errorCode = 0;
        wcsncpy_s(result->message, L"MptiBridgeRunShapeX completed (lightweight)", _TRUNCATE);
        return 0;
    }
    catch (const std::exception&)
    {
        result->errorCode = -100;
        wcsncpy_s(result->message, L"MptiBridgeRunShapeX C++ exception", _TRUNCATE);
        return -100;
    }
    catch (...)
    {
        result->errorCode = -101;
        wcsncpy_s(result->message, L"MptiBridgeRunShapeX unknown C++ exception", _TRUNCATE);
        return -101;
    }
}

MPTI_BRIDGE_API int MptiBridgeRunPadBW(
    const unsigned char* image,
    int imageWidth,
    int imageHeight,
    int sourceStride,
    int roiX,
    int roiY,
    int roiW,
    int roiH,
    const MptiBridgePadBWParams* params,
    MptiBridgePadBWResult* result)
{
    if (result == nullptr)
    {
        return -1;
    }
    *result = {};
    wcsncpy_s(result->message, L"", _TRUNCATE);

    if (image == nullptr || params == nullptr || imageWidth <= 0 || imageHeight <= 0 || sourceStride < imageWidth * 4)
    {
        result->errorCode = -1;
        wcsncpy_s(result->message, L"MptiBridgeRunPadBW: invalid argument", _TRUNCATE);
        return -1;
    }

    try
    {
        const auto start = std::chrono::high_resolution_clock::now();

        const auto info = AnalyseRoiForeground(
            image, imageWidth, imageHeight, sourceStride,
            roiX, roiY, roiW, roiH,
            params->binaryMin, params->binaryMax, params->invertCheck);

        result->isInsp = 1;
        result->foregroundPixels = info.foregroundPixels;
        result->blobCount = info.foregroundPixels > 0 ? 1 : 0;
        result->foundCenterX = info.centroidX;
        result->foundCenterY = info.centroidY;
        result->measuredArea = info.foregroundPixels;

        const bool foundObject = info.foregroundPixels >= std::max(1, params->minBlobArea);
        const double rate = params->teachArea > 0
            ? (static_cast<double>(info.foregroundPixels) / params->teachArea) * 100.0
            : 0.0;
        result->measuredAreaRate = rate;

        result->okArea = (params->useTeachArea == 0 ||
                          (rate >= params->teachAreaRateMin && rate <= params->teachAreaRateMax))
                         ? 1 : 0;
        result->okBlobArea = (params->useBlobArea == 0 || info.foregroundPixels >= params->blobAreaMin) ? 1 : 0;

        if (foundObject)
        {
            result->shiftX = static_cast<double>(info.centroidX) - params->expectedCenterX;
            result->shiftY = static_cast<double>(info.centroidY) - params->expectedCenterY;
        }
        result->okShiftX = (params->useShift == 0 || std::fabs(result->shiftX) <= params->teachShiftX) ? 1 : 0;
        result->okShiftY = (params->useShift == 0 || std::fabs(result->shiftY) <= params->teachShiftY) ? 1 : 0;

        result->isOK = result->okArea && result->okShiftX && result->okShiftY && result->okBlobArea ? 1 : 0;

        const auto end = std::chrono::high_resolution_clock::now();
        result->elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();
        result->errorCode = 0;
        wcsncpy_s(result->message, L"MptiBridgeRunPadBW completed (lightweight)", _TRUNCATE);
        return 0;
    }
    catch (const std::exception&)
    {
        result->errorCode = -100;
        wcsncpy_s(result->message, L"MptiBridgeRunPadBW C++ exception", _TRUNCATE);
        return -100;
    }
    catch (...)
    {
        result->errorCode = -101;
        wcsncpy_s(result->message, L"MptiBridgeRunPadBW unknown C++ exception", _TRUNCATE);
        return -101;
    }
}

MPTI_BRIDGE_API int MptiBridgeRunGeneric(
    const unsigned char* image,
    int imageWidth,
    int imageHeight,
    int sourceStride,
    int roiX,
    int roiY,
    int roiW,
    int roiH,
    const MptiBridgeGenericParams* params,
    MptiBridgeGenericResult* result)
{
    if (result == nullptr)
    {
        return -1;
    }
    *result = {};
    wcsncpy_s(result->message, L"", _TRUNCATE);

    if (image == nullptr || params == nullptr || imageWidth <= 0 || imageHeight <= 0 || sourceStride < imageWidth * 4)
    {
        result->errorCode = -1;
        wcsncpy_s(result->message, L"MptiBridgeRunGeneric: invalid argument", _TRUNCATE);
        return -1;
    }

    try
    {
        const auto start = std::chrono::high_resolution_clock::now();

        const auto info = AnalyseRoiForeground(
            image, imageWidth, imageHeight, sourceStride,
            roiX, roiY, roiW, roiH,
            params->binaryMin, params->binaryMax, params->invertCheck);

        const double roiArea = static_cast<double>(std::max(1, roiW)) * std::max(1, roiH);
        const float ratio = roiArea > 0 ? static_cast<float>(info.foregroundPixels / roiArea) : 0.0f;

        result->isInsp = 1;
        result->foregroundPixels = info.foregroundPixels;
        result->blobCount = info.foregroundPixels >= std::max(1, params->minBlobArea) ? 1 : 0;
        result->foundCenterX = info.centroidX;
        result->foundCenterY = info.centroidY;
        result->areaRatio = ratio;

        if (info.foregroundPixels > 0)
        {
            result->shiftX = static_cast<float>(info.centroidX - params->expectedCenterX);
            result->shiftY = static_cast<float>(info.centroidY - params->expectedCenterY);
        }

        result->okArea = (ratio >= params->minAreaRatio && ratio <= params->maxAreaRatio) ? 1 : 0;
        result->okShift = (std::fabs(result->shiftX) <= params->shiftXTolerance &&
                          std::fabs(result->shiftY) <= params->shiftYTolerance)
                          ? 1 : 0;

        // Algorithm-specific OK criteria. Defaults to area+shift but can vary later.
        switch (params->algoKind)
        {
            case MptiAlgo_BGA:
            case MptiAlgo_Blob:
            case MptiAlgo_Pattern:
                result->isOK = result->okArea && result->okShift ? 1 : 0;
                break;
            case MptiAlgo_Edge:
                // Edge: just check there is enough foreground
                result->isOK = result->blobCount > 0 ? 1 : 0;
                break;
            default:
                result->isOK = result->okArea && result->okShift ? 1 : 0;
                break;
        }

        const auto end = std::chrono::high_resolution_clock::now();
        result->elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();
        result->errorCode = 0;
        wcsncpy_s(result->message, L"MptiBridgeRunGeneric completed (lightweight)", _TRUNCATE);
        return 0;
    }
    catch (const std::exception&)
    {
        result->errorCode = -100;
        wcsncpy_s(result->message, L"MptiBridgeRunGeneric C++ exception", _TRUNCATE);
        return -100;
    }
    catch (...)
    {
        result->errorCode = -101;
        wcsncpy_s(result->message, L"MptiBridgeRunGeneric unknown C++ exception", _TRUNCATE);
        return -101;
    }
}
