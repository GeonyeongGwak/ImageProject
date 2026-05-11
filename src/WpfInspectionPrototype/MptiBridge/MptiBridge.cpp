#include "stdafx.h"
#include "MPTI.h"

#include <oleauto.h>

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
