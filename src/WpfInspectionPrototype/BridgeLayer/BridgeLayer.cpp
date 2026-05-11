#include "../InspectionCore/InspectionTypes.h"

#include <cwchar>
#include <exception>

#define BRIDGE_API extern "C" __declspec(dllexport)

namespace
{
    void SetBridgeError(InspectionResult* result, int code, const wchar_t* message)
    {
        if (result == nullptr)
        {
            return;
        }

        *result = {};
        result->errorCode = code;
        wcsncpy_s(result->message, message, _TRUNCATE);
    }
}

BRIDGE_API int BridgeThresholdBgra(
    const unsigned char* source,
    int width,
    int height,
    int sourceStride,
    unsigned char* destination,
    int destinationStride,
    int threshold,
    const RoiRect* roi,
    int useRoi,
    InspectionResult* result)
{
    try
    {
        return CoreThresholdBgraToGray8(
            source,
            width,
            height,
            sourceStride,
            destination,
            destinationStride,
            threshold,
            useRoi != 0 ? roi : nullptr,
            result);
    }
    catch (const std::exception&)
    {
        SetBridgeError(result, -100, L"C++ bridge exception");
        return -100;
    }
    catch (...)
    {
        SetBridgeError(result, -101, L"Unknown C++ bridge exception");
        return -101;
    }
}

BRIDGE_API int BridgeGetVersion(wchar_t* output, int outputLength)
{
    if (output == nullptr || outputLength <= 0)
    {
        return -1;
    }

    wcsncpy_s(output, static_cast<size_t>(outputLength), L"WPF native bridge prototype 0.1", _TRUNCATE);
    return 0;
}
