using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

// For net48, [ModuleInitializer] is recognized by the C# compiler as long as the
// attribute exists in metadata. .NET 5+ has it in BCL; for net48 we polyfill it here.
namespace System.Runtime.CompilerServices
{
    [AttributeUsage(AttributeTargets.Method, Inherited = false)]
    internal sealed class ModuleInitializerAttribute : Attribute { }
}

namespace WpfInspectionApp.Diagnostics
{
    // Runs at module load - BEFORE Main, BEFORE App static ctor, BEFORE WPF / PresentationCore
    // gets a chance to query DirectX capabilities and load nvd3dumx.dll (NVIDIA D3D driver
    // unmasks x87 FPU exceptions on init - this is the root cause of 0xC000008F WPF
    // ArithmeticException crashes during Application init under native debugging).
    internal static class FpExceptionGuard
    {
        // _MCW_EM (mask of all 6 fp exception bits)
        private const uint MCW_EM = 0x0008001F;

        [DllImport("ucrtbase.dll", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        private static extern int _controlfp_s(out uint currentControl, uint newControl, uint mask);

        [ModuleInitializer]
        internal static void MaskAtModuleLoad()
        {
            WriteDiag("ModuleInitializer: enter");
            TryMask();
            WriteDiag("ModuleInitializer: fp masked");

            // Force WPF software rendering BEFORE PresentationCore tries to probe the GPU
            // and load nvd3dumx.dll. RenderOptions.ProcessRenderMode is just a static field
            // here - touching it before any HwndSource exists is safe and prevents the
            // D3D device from ever being created.
            try
            {
                System.Windows.Media.RenderOptions.ProcessRenderMode =
                    System.Windows.Interop.RenderMode.SoftwareOnly;
                WriteDiag("ModuleInitializer: RenderMode forced to SoftwareOnly");
            }
            catch (Exception ex)
            {
                WriteDiag($"ModuleInitializer: RenderMode set FAILED {ex.GetType().Name}: {ex.Message}");
            }

            TryMask();
            WriteDiag("ModuleInitializer: exit");
        }

        internal static void TryMask()
        {
            try
            {
                _controlfp_s(out _, MCW_EM, MCW_EM);
            }
            catch
            {
                // best effort
            }
        }

        // Cheap append-only file write that runs at module-init time when DiagnosticsLog
        // is not yet usable. Used to prove the ModuleInitializer actually executed.
        private static void WriteDiag(string line)
        {
            try
            {
                var path = System.IO.Path.Combine(
                    System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location) ?? ".",
                    "fpguard.log");
                System.IO.File.AppendAllText(path, $"{DateTime.Now:HH:mm:ss.fff} {line}{Environment.NewLine}");
            }
            catch
            {
                // best effort
            }
        }
    }
}
