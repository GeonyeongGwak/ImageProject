using System.Runtime.InteropServices;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Interop;

[StructLayout(LayoutKind.Sequential, Pack = 8)]
public struct MptiBridgeShapeXParams
{
    public int BinaryMin;
    public int BinaryMax;
    public int UseInsp2D;
    public int InvertCheck;
    public int UseShape;
    public int UseInner;
    public int UseExist;
    public int UseShift;
    public float ShapeAreaMin;
    public float ShapeAreaMax;
    public float ShiftXTolerance;
    public float ShiftYTolerance;
    public int ExpectedCenterX;
    public int ExpectedCenterY;
    public int MinBlobArea;
}

[StructLayout(LayoutKind.Sequential, Pack = 8, CharSet = CharSet.Unicode)]
public struct MptiBridgeShapeXResult
{
    public int IsInsp;
    public int IsOK;
    public int OkShape;
    public int OkExist;
    public int OkShift;
    public int FoundCenterX;
    public int FoundCenterY;
    public float ShapeAreaRatio;
    public float ShiftX;
    public float ShiftY;
    public int ForegroundPixels;
    public int BlobCount;
    public double ElapsedMs;
    public int ErrorCode;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)] public string Message;
}

[StructLayout(LayoutKind.Sequential, Pack = 8)]
public struct MptiBridgePadBWParams
{
    public int BinaryMin;
    public int BinaryMax;
    public int UseInsp2D;
    public int InvertCheck;
    public int UseTeachArea;
    public double TeachArea;
    public double TeachAreaRateMin;
    public double TeachAreaRateMax;
    public int UseShift;
    public double TeachShiftX;
    public double TeachShiftY;
    public int ExpectedCenterX;
    public int ExpectedCenterY;
    public int UseBlobArea;
    public double BlobAreaMin;
    public int UseFillHole;
    public int FilterLevel;
    public int MinBlobArea;
}

[StructLayout(LayoutKind.Sequential, Pack = 8, CharSet = CharSet.Unicode)]
public struct MptiBridgePadBWResult
{
    public int IsInsp;
    public int IsOK;
    public int OkArea;
    public int OkShiftX;
    public int OkShiftY;
    public int OkBlobArea;
    public int FoundCenterX;
    public int FoundCenterY;
    public double MeasuredArea;
    public double MeasuredAreaRate;
    public double ShiftX;
    public double ShiftY;
    public int ForegroundPixels;
    public int BlobCount;
    public double ElapsedMs;
    public int ErrorCode;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)] public string Message;
}

public sealed record AlgorithmBridgeResponse(
    bool Available,
    bool Success,
    int Code,
    string Message,
    bool IsOK,
    double Theta,
    double ShiftX,
    double ShiftY,
    double AreaRate,
    int ForegroundPixels,
    int BlobCount,
    double ElapsedMs,
    int FoundCenterX,
    int FoundCenterY,
    int OkFlagsMask);

public static class MptiAlgorithmNativeBridge
{
    [DefaultDllImportSearchPaths(DllImportSearchPath.AssemblyDirectory)]
    [DllImport("MptiBridge.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeRunShapeX(
        IntPtr image, int imageWidth, int imageHeight, int sourceStride,
        int roiX, int roiY, int roiW, int roiH,
        ref MptiBridgeShapeXParams parameters,
        ref MptiBridgeShapeXResult result);

    [DefaultDllImportSearchPaths(DllImportSearchPath.AssemblyDirectory)]
    [DllImport("MptiBridge.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Unicode)]
    private static extern int MptiBridgeRunPadBW(
        IntPtr image, int imageWidth, int imageHeight, int sourceStride,
        int roiX, int roiY, int roiW, int roiH,
        ref MptiBridgePadBWParams parameters,
        ref MptiBridgePadBWResult result);

    public static AlgorithmBridgeResponse RunShapeX(
        byte[] image,
        int imageWidth, int imageHeight, int sourceStride,
        RoiRect roi,
        MptiBridgeShapeXParams parameters)
    {
        return InvokeBridge(image, () =>
        {
            var result = new MptiBridgeShapeXResult { Message = string.Empty };
            var handle = GCHandle.Alloc(image, GCHandleType.Pinned);
            try
            {
                var code = MptiBridgeRunShapeX(
                    handle.AddrOfPinnedObject(),
                    imageWidth, imageHeight, sourceStride,
                    roi.X, roi.Y, roi.Width, roi.Height,
                    ref parameters, ref result);
                return ToShapeXResponse(code, result);
            }
            finally
            {
                handle.Free();
            }
        });
    }

    public static AlgorithmBridgeResponse RunPadBW(
        byte[] image,
        int imageWidth, int imageHeight, int sourceStride,
        RoiRect roi,
        MptiBridgePadBWParams parameters)
    {
        return InvokeBridge(image, () =>
        {
            var result = new MptiBridgePadBWResult { Message = string.Empty };
            var handle = GCHandle.Alloc(image, GCHandleType.Pinned);
            try
            {
                var code = MptiBridgeRunPadBW(
                    handle.AddrOfPinnedObject(),
                    imageWidth, imageHeight, sourceStride,
                    roi.X, roi.Y, roi.Width, roi.Height,
                    ref parameters, ref result);
                return ToPadBWResponse(code, result);
            }
            finally
            {
                handle.Free();
            }
        });
    }

    private static AlgorithmBridgeResponse InvokeBridge(byte[] image, Func<AlgorithmBridgeResponse> action)
    {
        try
        {
            return action();
        }
        catch (DllNotFoundException ex) { return BridgeUnavailable(-900, ex.Message); }
        catch (EntryPointNotFoundException ex) { return BridgeUnavailable(-901, ex.Message); }
        catch (BadImageFormatException ex) { return BridgeUnavailable(-902, ex.Message); }
    }

    private static AlgorithmBridgeResponse BridgeUnavailable(int code, string message)
    {
        return new AlgorithmBridgeResponse(false, false, code, message, false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

    private static AlgorithmBridgeResponse ToShapeXResponse(int code, MptiBridgeShapeXResult r)
    {
        var flags = (r.OkShape << 0) | (r.OkExist << 1) | (r.OkShift << 2);
        return new AlgorithmBridgeResponse(
            true,
            code == 0,
            code,
            r.Message ?? string.Empty,
            r.IsOK != 0,
            0,
            r.ShiftX,
            r.ShiftY,
            r.ShapeAreaRatio,
            r.ForegroundPixels,
            r.BlobCount,
            r.ElapsedMs,
            r.FoundCenterX,
            r.FoundCenterY,
            flags);
    }

    private static AlgorithmBridgeResponse ToPadBWResponse(int code, MptiBridgePadBWResult r)
    {
        var flags = (r.OkArea << 0) | (r.OkShiftX << 1) | (r.OkShiftY << 2) | (r.OkBlobArea << 3);
        return new AlgorithmBridgeResponse(
            true,
            code == 0,
            code,
            r.Message ?? string.Empty,
            r.IsOK != 0,
            0,
            r.ShiftX,
            r.ShiftY,
            r.MeasuredAreaRate,
            r.ForegroundPixels,
            r.BlobCount,
            r.ElapsedMs,
            r.FoundCenterX,
            r.FoundCenterY,
            flags);
    }
}
