using System.Runtime.InteropServices;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Interop;

public sealed record NativeThresholdResponse(
    NativeInspectionResult Result,
    bool UsedNative,
    string RuntimeMessage);

[StructLayout(LayoutKind.Sequential)]
public struct NativeRoiRect
{
    public int X;
    public int Y;
    public int Width;
    public int Height;

    public static NativeRoiRect FromModel(RoiRect roi) => new()
    {
        X = roi.X,
        Y = roi.Y,
        Width = roi.Width,
        Height = roi.Height
    };
}

[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
public struct NativeInspectionResult
{
    public int ForegroundPixels;
    public int BlobCount;
    public int MinX;
    public int MinY;
    public int MaxX;
    public int MaxY;
    public double ElapsedMs;
    public int ErrorCode;

    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
    public string Message;
}

public static class NativeInspectionBridge
{
    [DefaultDllImportSearchPaths(DllImportSearchPath.AssemblyDirectory)]
    [DllImport("BridgeLayer.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int BridgeThresholdBgra(
        IntPtr source,
        int width,
        int height,
        int sourceStride,
        IntPtr destination,
        int destinationStride,
        int threshold,
        ref NativeRoiRect roi,
        int useRoi,
        out NativeInspectionResult result);

    public static NativeThresholdResponse ThresholdBgra(
        byte[] source,
        int width,
        int height,
        int sourceStride,
        byte[] destination,
        int destinationStride,
        int threshold,
        RoiRect? roi)
    {
        var nativeRoi = roi.HasValue ? NativeRoiRect.FromModel(roi.Value) : default;

        try
        {
            var sourceHandle = GCHandle.Alloc(source, GCHandleType.Pinned);
            var destinationHandle = GCHandle.Alloc(destination, GCHandleType.Pinned);

            try
            {
                var code = BridgeThresholdBgra(
                    sourceHandle.AddrOfPinnedObject(),
                    width,
                    height,
                    sourceStride,
                    destinationHandle.AddrOfPinnedObject(),
                    destinationStride,
                    threshold,
                    ref nativeRoi,
                    roi.HasValue ? 1 : 0,
                    out var result);

                if (code != 0 && result.ErrorCode == 0)
                {
                    result.ErrorCode = code;
                    result.Message = $"Bridge returned error {code}";
                }

                return new NativeThresholdResponse(result, true, result.Message);
            }
            finally
            {
                sourceHandle.Free();
                destinationHandle.Free();
            }
        }
        catch (DllNotFoundException error)
        {
            return ManagedThreshold(source, width, height, sourceStride, destination, destinationStride, threshold, roi, error.Message);
        }
        catch (EntryPointNotFoundException error)
        {
            return ManagedThreshold(source, width, height, sourceStride, destination, destinationStride, threshold, roi, error.Message);
        }
        catch (BadImageFormatException error)
        {
            return ManagedThreshold(source, width, height, sourceStride, destination, destinationStride, threshold, roi, error.Message);
        }
    }

    private static NativeThresholdResponse ManagedThreshold(
        byte[] source,
        int width,
        int height,
        int sourceStride,
        byte[] destination,
        int destinationStride,
        int threshold,
        RoiRect? roi,
        string nativeLoadMessage)
    {
        Array.Clear(destination, 0, destination.Length);
        var clampedThreshold = Net48Compat.Clamp(threshold, 0, 255);
        var area = ClampRoi(roi, width, height);
        var foreground = 0;
        var minX = width;
        var minY = height;
        var maxX = -1;
        var maxY = -1;

        for (var y = area.Y; y < area.Y + area.Height; y++)
        {
            var sourceRow = y * sourceStride;
            var destinationRow = y * destinationStride;

            for (var x = area.X; x < area.X + area.Width; x++)
            {
                var offset = sourceRow + x * 4;
                var blue = source[offset];
                var green = source[offset + 1];
                var red = source[offset + 2];
                var gray = (77 * red + 150 * green + 29 * blue) >> 8;
                var binary = gray >= clampedThreshold ? (byte)255 : (byte)0;
                destination[destinationRow + x] = binary;

                if (binary == 0)
                {
                    continue;
                }

                foreground++;
                minX = Math.Min(minX, x);
                minY = Math.Min(minY, y);
                maxX = Math.Max(maxX, x);
                maxY = Math.Max(maxY, y);
            }
        }

        var result = new NativeInspectionResult
        {
            ForegroundPixels = foreground,
            BlobCount = foreground > 0 ? 1 : 0,
            MinX = foreground > 0 ? minX : 0,
            MinY = foreground > 0 ? minY : 0,
            MaxX = foreground > 0 ? maxX : 0,
            MaxY = foreground > 0 ? maxY : 0,
            ErrorCode = 0,
            Message = "Managed fallback threshold completed"
        };

        return new NativeThresholdResponse(result, false, nativeLoadMessage);
    }

    private static RoiRect ClampRoi(RoiRect? roi, int width, int height)
    {
        if (!roi.HasValue || !roi.Value.IsValid)
        {
            return new RoiRect(0, 0, width, height);
        }

        var x = Net48Compat.Clamp(roi.Value.X, 0, Math.Max(0, width - 1));
        var y = Net48Compat.Clamp(roi.Value.Y, 0, Math.Max(0, height - 1));
        var right = Net48Compat.Clamp(roi.Value.X + roi.Value.Width, x + 1, width);
        var bottom = Net48Compat.Clamp(roi.Value.Y + roi.Value.Height, y + 1, height);
        return new RoiRect(x, y, right - x, bottom - y);
    }
}

