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

public sealed record MptiLightPreviewResult(
    bool Available,
    bool Success,
    int Code,
    string Message,
    byte[] GrayPixels,
    int Width,
    int Height);

// CAM-01 컬러 미리보기 결과. BGRA32 인터리브 (B, G, R, A 순) 4 byte * pixel 개수.
// Light 슬라이더와 무관하게 PTT 의 TR/TG/TB 채널을 그대로 사용한 결과.
public sealed record MptiColorPreviewResult(
    bool Available,
    bool Success,
    int Code,
    string Message,
    byte[] BgraPixels,
    int Width,
    int Height);

public static class MptiNativeBridge
{
    private const int MessageCapacity = 512;
    private const int MaximumLightCells = 10;

    [StructLayout(LayoutKind.Sequential)]
    private struct MptiBridgeLightPreviewParams
    {
        public int Width;
        public int Height;
        public int LightType;
        public int RedValue;
        public int GreenValue;
        public int BlueValue;
        public int WhiteValue;
        public int LightCount;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = MaximumLightCells)]
        public int[] ArrRedValue;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = MaximumLightCells)]
        public int[] ArrGreenValue;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = MaximumLightCells)]
        public int[] ArrBlueValue;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = MaximumLightCells)]
        public int[] ArrWhiteValue;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = MaximumLightCells)]
        public int[] ArrLightPosition;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = MaximumLightCells)]
        public int[] ArrCalculation;
    }

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

    [DllImport("MptiBridge.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeRenderLightPreview(
        ref MptiBridgeLightPreviewParams parameters,
        [Out] byte[] output,
        int outputLength,
        StringBuilder message,
        int messageLength);

    [DllImport("MptiBridge.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeRenderColorPreview(
        int width,
        int height,
        [Out] byte[] output,
        int outputLength,
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

    public static MptiLightPreviewResult RenderLightPreview(AlgorithmLightState state, int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            return new MptiLightPreviewResult(true, false, -2, "Invalid PTT preview dimensions.", [], 0, 0);
        }

        try
        {
            NativeDependencyPath.EnsureInitialized();
            var output = new byte[width * height];
            var parameters = CreateLightPreviewParams(state, width, height);
            var message = CreateMessageBuffer();
            var code = MptiBridgeRenderLightPreview(ref parameters, output, output.Length, message, message.Capacity);
            return new MptiLightPreviewResult(true, code == 0, code, message.ToString(), output, width, height);
        }
        catch (Exception ex) when (IsBridgeLoadException(ex))
        {
            return new MptiLightPreviewResult(false, false, -900, ex.Message, [], 0, 0);
        }
    }

    // CAM-01 컬러 미리보기. Light 슬라이더와 무관하게 PTT 의 TR/TG/TB 채널을 그대로 BGRA32 로
    // 합성한다. PTT 가 로드되어 있어야 하며, native 가 MPTI_GetPttFileChannel 로 TR/TG/TB
    // 채널을 읽어 BGRA 인터리브 출력으로 반환한다.
    public static MptiColorPreviewResult RenderColorPreview(int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            return new MptiColorPreviewResult(true, false, -2, "Invalid PTT preview dimensions.", [], 0, 0);
        }

        try
        {
            NativeDependencyPath.EnsureInitialized();
            var output = new byte[width * height * 4];
            var message = CreateMessageBuffer();
            var code = MptiBridgeRenderColorPreview(width, height, output, output.Length, message, message.Capacity);
            return new MptiColorPreviewResult(true, code == 0, code, message.ToString(), output, width, height);
        }
        catch (Exception ex) when (IsBridgeLoadException(ex))
        {
            return new MptiColorPreviewResult(false, false, -900, ex.Message, [], 0, 0);
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

    private static MptiBridgeLightPreviewParams CreateLightPreviewParams(AlgorithmLightState state, int width, int height)
    {
        var cells = state.UserCells.Count == 0
            ? new List<AlgorithmLightCell> { new() { Position = 0, Operator = 0, RedValue = 100, GreenValue = 0, BlueValue = 0, WhiteValue = 0 } }
            : state.UserCells.Take(MaximumLightCells).ToList();

        return new MptiBridgeLightPreviewParams
        {
            Width = width,
            Height = height,
            LightType = Clamp(state.LightType, 0, 8),
            RedValue = Clamp(state.RedValue, 0, 200),
            GreenValue = Clamp(state.GreenValue, 0, 200),
            BlueValue = Clamp(state.BlueValue, 0, 200),
            WhiteValue = Clamp(state.WhiteValue, 0, 200),
            LightCount = Clamp(cells.Count, 1, MaximumLightCells),
            ArrRedValue = CreateCellArray(cells, cell => cell.RedValue),
            ArrGreenValue = CreateCellArray(cells, cell => cell.GreenValue),
            ArrBlueValue = CreateCellArray(cells, cell => cell.BlueValue),
            ArrWhiteValue = CreateCellArray(cells, cell => cell.WhiteValue),
            ArrLightPosition = CreateCellArray(cells, cell => Clamp(cell.Position, 0, 2)),
            ArrCalculation = CreateCellArray(cells, cell => Clamp(cell.Operator, 0, 2))
        };
    }

    private static int[] CreateCellArray(IReadOnlyList<AlgorithmLightCell> cells, Func<AlgorithmLightCell, int> selector)
    {
        var values = new int[MaximumLightCells];
        for (var index = 0; index < values.Length && index < cells.Count; index++)
        {
            values[index] = selector(cells[index]);
        }

        return values;
    }

    private static int Clamp(int value, int min, int max)
    {
        return Math.Max(min, Math.Min(max, value));
    }
}
