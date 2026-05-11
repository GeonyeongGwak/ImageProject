using System.IO;
using WpfInspectionApp.Interop;

namespace WpfInspectionApp.Services;

public sealed class PttLoadService : IPttLoadService
{
    public string CreateFileDiagnostics(string path)
    {
        if (!File.Exists(path))
        {
            throw new FileNotFoundException("PTT file was not found.", path);
        }

        var potPath = Path.ChangeExtension(path, ".pot");
        var pttLength = new FileInfo(path).Length;
        var potExists = File.Exists(potPath);
        var potLength = potExists ? new FileInfo(potPath).Length : 0;
        return $"PTT file length={pttLength}, POT exists={potExists}, POT length={potLength}";
    }

    public PttLoadPreparationResult PrepareMptiBridge(string path)
    {
        var version = MptiNativeBridge.GetVersion();
        if (!version.Available)
        {
            DiagnosticsLog.Write($"MPTI bridge unavailable: {version.Message}");
            return new PttLoadPreparationResult(false, "MPTI bridge unavailable");
        }

        var ptt = MptiNativeBridge.LoadPtt(path);
        DiagnosticsLog.Write($"MPTI LoadPtt result: available={ptt.Available}, success={ptt.Success}, code={ptt.Code}, size={ptt.Width}x{ptt.Height}, message={ptt.Message}");
        if (!ptt.Success)
        {
            return new PttLoadPreparationResult(false, $"MPTI PTT load failed: {ptt.Code}", ptt.Width, ptt.Height);
        }

        // MPTI_SetRawDataFovInfo can block on some imported PTT/POT pairs. Part Import should
        // complete once the part data and PEM3D PTT viewer are loaded; run deeper RawData prep
        // only from an explicit inspection/teaching flow.
        return new PttLoadPreparationResult(true, $"MPTI PTT loaded: {ptt.Width}x{ptt.Height}", ptt.Width, ptt.Height);
    }
}
