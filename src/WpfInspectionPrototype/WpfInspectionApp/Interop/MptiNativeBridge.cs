using System.Runtime.InteropServices;
using System.Text;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Interop;

public sealed record MptiBridgeCallResult(
    bool Available,
    bool Success,
    int Code,
    string Message);

public sealed record MptiPttLoadResult(
    bool Available,
    bool Success,
    int Code,
    string Message,
    int Width,
    int Height);

public sealed record MptiRawDataFovResult(
    bool Available,
    bool Success,
    int Code,
    string Message,
    int ResultNumber);

public sealed record MptiZmapInfoResult(
    bool Available,
    bool Success,
    int Code,
    string Message,
    int Width,
    int Height,
    bool HasPointer);

public static class MptiNativeBridge
{
    private const int MessageCapacity = 512;

    [DllImport("MptiBridge.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeGetVersion(StringBuilder output, int outputLength);

    [DllImport("MptiBridge.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeDebugProbe(int breakIntoDebugger, StringBuilder output, int outputLength);

    [DllImport("MptiBridge.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeSetMachineMode(int mode, int teach, StringBuilder message, int messageLength);

    [DllImport("MptiBridge.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeLoadPtt(
        [MarshalAs(UnmanagedType.LPWStr)] string pttPath,
        out int width,
        out int height,
        int callSpi,
        int useFactor,
        StringBuilder message,
        int messageLength);

    [DllImport("MptiBridge.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeLoadPot(
        [MarshalAs(UnmanagedType.LPWStr)] string potPath,
        out int width,
        out int height,
        out float pixelResX,
        out float pixelResY,
        StringBuilder message,
        int messageLength);

    [DllImport("MptiBridge.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeSetRawDataFovInfo(
        [MarshalAs(UnmanagedType.LPWStr)] string pttPath,
        [MarshalAs(UnmanagedType.LPWStr)] string potPath,
        int x,
        int y,
        int width,
        int height,
        int separatedPart,
        out int resultNumber,
        StringBuilder message,
        int messageLength);

    [DllImport("MptiBridge.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeGetRawDataZmapInfo(
        out int width,
        out int height,
        out int hasPointer,
        StringBuilder message,
        int messageLength);

    public static MptiBridgeCallResult GetVersion()
    {
        return Guard(() =>
        {
            var message = CreateMessageBuffer();
            var code = MptiBridgeGetVersion(message, message.Capacity);
            return new MptiBridgeCallResult(true, code == 0, code, message.ToString());
        });
    }

    public static MptiBridgeCallResult DebugProbe(bool breakIntoDebugger)
    {
        return Guard(() =>
        {
            var message = CreateMessageBuffer();
            var code = MptiBridgeDebugProbe(breakIntoDebugger ? 1 : 0, message, message.Capacity);
            return new MptiBridgeCallResult(true, code >= 0, code, message.ToString());
        });
    }

    public static MptiBridgeCallResult SetMachineMode(int mode, bool teach)
    {
        return Guard(() =>
        {
            var message = CreateMessageBuffer();
            var code = MptiBridgeSetMachineMode(mode, teach ? 1 : 0, message, message.Capacity);
            return new MptiBridgeCallResult(true, code == 0, code, message.ToString());
        });
    }

    public static MptiPttLoadResult LoadPtt(string pttPath, bool callSpi = false, bool useFactor = true)
    {
        try
        {
            NativeDependencyPath.EnsureInitialized();
            var message = CreateMessageBuffer();
            var code = MptiBridgeLoadPtt(pttPath, out var width, out var height, callSpi ? 1 : 0, useFactor ? 1 : 0, message, message.Capacity);
            return new MptiPttLoadResult(true, code == 0, code, message.ToString(), width, height);
        }
        catch (Exception ex) when (IsBridgeLoadException(ex))
        {
            return new MptiPttLoadResult(false, false, -900, ex.Message, 0, 0);
        }
    }

    public static MptiRawDataFovResult SetRawDataFovInfo(string pttPath, string? potPath, RoiRect roi, bool separatedPart)
    {
        try
        {
            NativeDependencyPath.EnsureInitialized();
            var message = CreateMessageBuffer();
            var code = MptiBridgeSetRawDataFovInfo(
                pttPath,
                potPath ?? string.Empty,
                roi.X,
                roi.Y,
                roi.Width,
                roi.Height,
                separatedPart ? 1 : 0,
                out var resultNumber,
                message,
                message.Capacity);
            return new MptiRawDataFovResult(true, code == 0, code, message.ToString(), resultNumber);
        }
        catch (Exception ex) when (IsBridgeLoadException(ex))
        {
            return new MptiRawDataFovResult(false, false, -900, ex.Message, 0);
        }
    }

    public static MptiZmapInfoResult GetRawDataZmapInfo()
    {
        try
        {
            NativeDependencyPath.EnsureInitialized();
            var message = CreateMessageBuffer();
            var code = MptiBridgeGetRawDataZmapInfo(out var width, out var height, out var hasPointer, message, message.Capacity);
            return new MptiZmapInfoResult(true, code == 0, code, message.ToString(), width, height, hasPointer != 0);
        }
        catch (Exception ex) when (IsBridgeLoadException(ex))
        {
            return new MptiZmapInfoResult(false, false, -900, ex.Message, 0, 0, false);
        }
    }

    private static MptiBridgeCallResult Guard(Func<MptiBridgeCallResult> call)
    {
        try
        {
            NativeDependencyPath.EnsureInitialized();
            return call();
        }
        catch (Exception ex) when (IsBridgeLoadException(ex))
        {
            return new MptiBridgeCallResult(false, false, -900, ex.Message);
        }
    }

    private static bool IsBridgeLoadException(Exception ex)
    {
        return ex is DllNotFoundException
            || ex is EntryPointNotFoundException
            || ex is BadImageFormatException;
    }

    private static StringBuilder CreateMessageBuffer()
    {
        return new StringBuilder(MessageCapacity);
    }
}
