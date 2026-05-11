using System.Globalization;
using System.IO;
using System.Runtime.InteropServices;

namespace WpfInspectionApp.Models;

public sealed record LegacyPttImageData(
    int Width,
    int Height,
    float PixelResolutionX,
    float PixelResolutionY,
    short MinValue,
    short MaxValue,
    int ChannelIndex,
    byte[] PreviewPixels,
    int BytesPerPixel,
    string Loader,
    string SourcePath);

public static class LegacyPttImageLoader
{
    private const int MinimumHeaderSize = 32;
    private const int LegacyRgbPttChannelCount = 3;
    private const string MptiDllDirectory = @"D:\Work\GGY\AlgorithmTool_MultiProcess_Micron_Clean\x64\Release";
    private const string MptiDllPath = @"D:\Work\GGY\AlgorithmTool_MultiProcess_Micron_Clean\x64\Release\MPTILib_Algo.dll";

    [DllImport("kernel32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
    private static extern bool SetDllDirectory(string? lpPathName);

    [DllImport(MptiDllPath)]
    private static extern int MPTI_Init();

    [DllImport(MptiDllPath)]
    private static extern int MPTI_SetFovPixelResolution(double resolX, double resolY);

    [DllImport(MptiDllPath)]
    private static extern int MPTI_SetFovPixelNumber(int fovWidth, int fovLength);

    [DllImport(MptiDllPath)]
    private static extern int MPTI_GetPttFileLoad(IntPtr filePath, ref int width, ref int height, bool callSpi = false);

    [DllImport(MptiDllPath)]
    private static extern IntPtr MPTI_GetPttFileChannel(int channel);

    public static LegacyPttImageData Load(string path)
    {
        try
        {
            return LoadWithMpti(path);
        }
        catch (DllNotFoundException)
        {
            return LoadRawFallback(path);
        }
        catch (BadImageFormatException)
        {
            return LoadRawFallback(path);
        }
        catch (EntryPointNotFoundException)
        {
            return LoadRawFallback(path);
        }
        catch (InvalidDataException)
        {
            return LoadRawFallback(path);
        }
    }

    private static LegacyPttImageData LoadWithMpti(string path)
    {
        var previousDirectory = Environment.CurrentDirectory;
        if (Directory.Exists(MptiDllDirectory))
        {
            SetDllDirectory(MptiDllDirectory);
            Environment.CurrentDirectory = MptiDllDirectory;
        }

        try
        {
            return LoadWithMptiCore(path);
        }
        finally
        {
            Environment.CurrentDirectory = previousDirectory;
        }
    }

    private static LegacyPttImageData LoadWithMptiCore(string path)
    {
            var width = 0;
            var height = 0;
            var pixelResolutionX = 0.0f;
            var pixelResolutionY = 0.0f;

            MPTI_Init();

            var pttBstr = StringToBstr(path);
            try
            {
                var code = MPTI_GetPttFileLoad(pttBstr, ref width, ref height, false);
                if (code != 0 && code != 1)
                {
                    throw new InvalidDataException($"MPTI_GetPttFileLoad returned {code}.");
                }
            }
            finally
            {
                FreeBstr(pttBstr);
            }

            ApplyPotResolution(path, width, height, ref pixelResolutionX, ref pixelResolutionY);

        if (width <= 0 || height <= 0)
        {
            throw new InvalidDataException("MPTI did not return a valid PTT size.");
        }

        var red = MPTI_GetPttFileChannel(0);
        var green = MPTI_GetPttFileChannel(1);
        var blue = MPTI_GetPttFileChannel(2);
        if (red == IntPtr.Zero || green == IntPtr.Zero || blue == IntPtr.Zero)
        {
            throw new InvalidDataException("MPTI did not return RGB PTT channels.");
        }

        var pixelCount = checked(width * height);
        var redBytes = new byte[pixelCount];
        var greenBytes = new byte[pixelCount];
        var blueBytes = new byte[pixelCount];
        Marshal.Copy(red, redBytes, 0, pixelCount);
        Marshal.Copy(green, greenBytes, 0, pixelCount);
        Marshal.Copy(blue, blueBytes, 0, pixelCount);

        var preview = new byte[pixelCount * 3];
        for (var i = 0; i < pixelCount; i++)
        {
            var offset = i * 3;
            preview[offset] = blueBytes[i];
            preview[offset + 1] = greenBytes[i];
            preview[offset + 2] = redBytes[i];
        }

        return new LegacyPttImageData(
            width,
            height,
            NormalizeResolution(pixelResolutionX),
            NormalizeResolution(pixelResolutionY),
            0,
            255,
            0,
            preview,
            3,
            "MPTI_GetPttFileLoad",
            path);
    }

    private static IntPtr StringToBstr(string value)
    {
        return Marshal.StringToBSTR(value);
    }

    private static void FreeBstr(IntPtr value)
    {
        if (value != IntPtr.Zero)
        {
            Marshal.FreeBSTR(value);
        }
    }

    private static void ApplyPotResolution(string pttPath, int width, int height, ref float pixelResolutionX, ref float pixelResolutionY)
    {
        var potPath = Path.ChangeExtension(pttPath, ".pot");
        if (!File.Exists(potPath))
        {
            return;
        }

        try
        {
            var pot = File.ReadAllText(potPath);
            var values = pot
                .Split(new[] { ',', '\r', '\n', '\t', ' ' }, StringSplitOptions.RemoveEmptyEntries)
                .Select(value => float.TryParse(value, NumberStyles.Float, CultureInfo.InvariantCulture, out var parsed) ? parsed : 0)
                .Where(value => value > 0 && value < 10)
                .ToList();
            if (values.Count >= 2)
            {
                pixelResolutionX = values[0];
                pixelResolutionY = values[1];
                MPTI_SetFovPixelNumber(width, height);
                MPTI_SetFovPixelResolution(pixelResolutionX, pixelResolutionY);
            }
        }
        catch (IOException)
        {
        }
    }

    public static LegacyPttImageData LoadRawFallback(string path)
    {
        var bytes = File.ReadAllBytes(path);
        if (bytes.Length < MinimumHeaderSize)
        {
            throw new InvalidDataException("PTT file is too small.");
        }

        var width = BitConverter.ToInt32(bytes, 0);
        var height = BitConverter.ToInt32(bytes, 4);
        var pixelResolutionX = BitConverter.ToSingle(bytes, 8);
        var pixelResolutionY = BitConverter.ToSingle(bytes, 12);

        if (width <= 0 || height <= 0 || width > 100000 || height > 100000)
        {
            throw new InvalidDataException("PTT header width/height is not valid.");
        }

        var pixelCount64 = (long)width * height;
        if (pixelCount64 > int.MaxValue)
        {
            throw new InvalidDataException("PTT image is too large for the preview panel.");
        }

        var pixelCount = (int)pixelCount64;
        var channelBytes = pixelCount * sizeof(short);
        var headerSize = DetectHeaderSize(bytes.Length, pixelCount, channelBytes);
        if (bytes.Length < headerSize + channelBytes)
        {
            throw new InvalidDataException("PTT file does not contain enough height data.");
        }

        var channelIndex = SelectBestChannel(bytes, headerSize, pixelCount, channelBytes);
        var channelOffset = headerSize + channelIndex * channelBytes;
        var preview = new byte[pixelCount];
        var min = short.MaxValue;
        var max = short.MinValue;

        for (var i = 0; i < pixelCount; i++)
        {
            var value = BitConverter.ToInt16(bytes, channelOffset + i * sizeof(short));
            min = Math.Min(min, value);
            max = Math.Max(max, value);
        }

        if (min == max)
        {
            for (var i = 0; i < preview.Length; i++) preview[i] = 128;
        }
        else
        {
            var range = max - min;
            for (var i = 0; i < pixelCount; i++)
            {
                var value = BitConverter.ToInt16(bytes, channelOffset + i * sizeof(short));
                preview[i] = (byte)Net48Compat.Clamp((int)Math.Round((value - min) * 255.0 / range), 0, 255);
            }
        }

        return new LegacyPttImageData(
            width,
            height,
            NormalizeResolution(pixelResolutionX),
            NormalizeResolution(pixelResolutionY),
            min,
            max,
            channelIndex,
            preview,
            1,
            $"Raw PTT fallback (header {headerSize})",
            path);
    }

    private static int DetectHeaderSize(int fileLength, int pixelCount, int channelBytes)
    {
        var expectedRgbHeader = fileLength - LegacyRgbPttChannelCount * channelBytes;
        if (expectedRgbHeader >= MinimumHeaderSize && expectedRgbHeader < 4096)
        {
            return expectedRgbHeader;
        }

        return MinimumHeaderSize;
    }

    private static int SelectBestChannel(byte[] bytes, int headerSize, int pixelCount, int channelBytes)
    {
        var availableChannels = Math.Min(4, Math.Max(1, (bytes.Length - headerSize) / channelBytes));
        var bestChannel = 0;
        var bestRange = -1;
        var bestDisplayChannel = -1;
        var bestDisplayRange = -1;

        for (var channel = 0; channel < availableChannels; channel++)
        {
            var offset = headerSize + channel * channelBytes;
            var min = short.MaxValue;
            var max = short.MinValue;
            var step = Math.Max(1, pixelCount / 8192);

            for (var i = 0; i < pixelCount; i += step)
            {
                var value = BitConverter.ToInt16(bytes, offset + i * sizeof(short));
                min = Math.Min(min, value);
                max = Math.Max(max, value);
            }

            var range = max - min;
            if (min >= 0 && max <= byte.MaxValue && range > bestDisplayRange)
            {
                bestDisplayRange = range;
                bestDisplayChannel = channel;
            }

            if (range > bestRange)
            {
                bestRange = range;
                bestChannel = channel;
            }
        }

        if (bestDisplayChannel >= 0)
        {
            return bestDisplayChannel;
        }

        return bestChannel;
    }

    private static float NormalizeResolution(float value)
    {
        if (float.IsNaN(value) || float.IsInfinity(value) || value <= 0)
        {
            return 0;
        }

        return float.Parse(value.ToString("0.####", CultureInfo.InvariantCulture), CultureInfo.InvariantCulture);
    }
}

