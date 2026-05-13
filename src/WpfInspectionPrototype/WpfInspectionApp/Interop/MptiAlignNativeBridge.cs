using System.Runtime.InteropServices;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Interop;

[StructLayout(LayoutKind.Sequential, Pack = 8)]
public struct MptiBridgeAlignParams
{
    public int SearchNum;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)] public int[] SearchPointsX;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)] public int[] SearchPointsY;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)] public int[] SearchSizeW;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)] public int[] SearchSizeH;
    public int SearchMargin;
    public int MinBinary;
    public int MaxBinary;
    public int UseInsp2D;
    public int InvertCheck;
    public int UseShift;
    public double MaxShiftX;
    public double MaxShiftY;
    public int UseAngle;
    public double MaxAngle;
    public int SameSize;
    public int MinBlobArea;
}

[StructLayout(LayoutKind.Sequential, Pack = 8, CharSet = CharSet.Unicode)]
public struct MptiBridgeAlignResult
{
    public int OkCount;
    public double OffsetX;
    public double OffsetY;
    public double Theta;
    public int OkShiftX;
    public int OkShiftY;
    public int OkAngle;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)] public int[] DetectedCentersX;
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)] public int[] DetectedCentersY;
    public int ForegroundPixels;
    public int BlobCount;
    public double ElapsedMs;
    public int ErrorCode;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)] public string Message;
}

public sealed record AlignBridgeResponse(
    bool Available,
    bool Success,
    int Code,
    string Message,
    int OkCount,
    double OffsetX,
    double OffsetY,
    double Theta,
    bool OkShiftX,
    bool OkShiftY,
    bool OkAngle,
    int[] DetectedCentersX,
    int[] DetectedCentersY,
    int ForegroundPixels,
    int BlobCount,
    double ElapsedMs);

public static class MptiAlignNativeBridge
{
    [DllImport("MptiBridge.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeRunAlign(
        IntPtr image,
        int imageWidth,
        int imageHeight,
        int sourceStride,
        int windowX,
        int windowY,
        int windowW,
        int windowH,
        ref MptiBridgeAlignParams parameters,
        ref MptiBridgeAlignResult result);

    public static AlignBridgeResponse Run(
        byte[] image,
        int imageWidth,
        int imageHeight,
        int sourceStride,
        RoiRect windowRoi,
        MptiBridgeAlignParams parameters)
    {
        var result = CreateEmptyResult();

        try
        {
            NativeDependencyPath.EnsureInitialized();
            var handle = GCHandle.Alloc(image, GCHandleType.Pinned);
            try
            {
                var code = MptiBridgeRunAlign(
                    handle.AddrOfPinnedObject(),
                    imageWidth,
                    imageHeight,
                    sourceStride,
                    windowRoi.X,
                    windowRoi.Y,
                    windowRoi.Width,
                    windowRoi.Height,
                    ref parameters,
                    ref result);
                return ToResponse(true, code == 0, code, result);
            }
            finally
            {
                handle.Free();
            }
        }
        catch (DllNotFoundException ex)
        {
            return new AlignBridgeResponse(false, false, -900, ex.Message, 0, 0, 0, 0, false, false, false, [0, 0, 0, 0], [0, 0, 0, 0], 0, 0, 0);
        }
        catch (EntryPointNotFoundException ex)
        {
            return new AlignBridgeResponse(false, false, -901, ex.Message, 0, 0, 0, 0, false, false, false, [0, 0, 0, 0], [0, 0, 0, 0], 0, 0, 0);
        }
        catch (BadImageFormatException ex)
        {
            return new AlignBridgeResponse(false, false, -902, ex.Message, 0, 0, 0, 0, false, false, false, [0, 0, 0, 0], [0, 0, 0, 0], 0, 0, 0);
        }
    }

    private static MptiBridgeAlignResult CreateEmptyResult()
    {
        return new MptiBridgeAlignResult
        {
            DetectedCentersX = new int[4],
            DetectedCentersY = new int[4],
            Message = string.Empty
        };
    }

    private static AlignBridgeResponse ToResponse(bool available, bool success, int code, MptiBridgeAlignResult result)
    {
        return new AlignBridgeResponse(
            available,
            success,
            code,
            result.Message ?? string.Empty,
            result.OkCount,
            result.OffsetX,
            result.OffsetY,
            result.Theta,
            result.OkShiftX != 0,
            result.OkShiftY != 0,
            result.OkAngle != 0,
            result.DetectedCentersX ?? new int[4],
            result.DetectedCentersY ?? new int[4],
            result.ForegroundPixels,
            result.BlobCount,
            result.ElapsedMs);
    }
}
