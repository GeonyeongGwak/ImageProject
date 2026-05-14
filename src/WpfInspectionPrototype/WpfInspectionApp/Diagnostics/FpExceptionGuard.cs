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
    // Aggressive FPU exception mask + WPF software rendering forced at module load.
    //
    // Root cause: under VS native debugging, something between our ModuleInitializer and
    // Application.ApplicationInit (NVIDIA D3D driver nvd3dumx.dll, OR the native debugger
    // injecting fp-tracking hooks) unmasks x87 FPU exceptions. WPF transform/trace code
    // in PresentationCore/PresentationFramework static ctors then hits inexact -> 0xC000008F
    // -> System.ArithmeticException -> 0xC0000005 -> process exit.
    //
    // Strategy: re-mask aggressively at every observable hook point during startup, log
    // each pass with the observed FPU control word so we can pinpoint where it gets
    // unmasked. (fpguard.log lives next to the exe.)
    internal static class FpExceptionGuard
    {
        // _MCW_EM (mask of all 6 fp exception bits): _EM_INVALID 0x10 | _EM_DENORMAL 0x80000 |
        // _EM_ZERODIVIDE 0x08 | _EM_OVERFLOW 0x04 | _EM_UNDERFLOW 0x02 | _EM_INEXACT 0x01.
        private const uint MCW_EM = 0x0008001F;

        [DllImport("ucrtbase.dll", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        private static extern int _controlfp_s(out uint currentControl, uint newControl, uint mask);

        [ModuleInitializer]
        internal static void MaskAtModuleLoad()
        {
            Diag("ModuleInit: enter");
            TryMaskLogged("ModuleInit: after first mask");

            try
            {
                System.Windows.Media.RenderOptions.ProcessRenderMode =
                    System.Windows.Interop.RenderMode.SoftwareOnly;
                Diag("ModuleInit: RenderMode = SoftwareOnly");
            }
            catch (Exception ex)
            {
                Diag($"ModuleInit: RenderMode set FAILED {ex.GetType().Name}: {ex.Message}");
            }

            // Hook every assembly load - between Application..ctor and the fp crash,
            // PresentationCore.resources/UIAutomation*/etc. all load and each load
            // is a chance for a native dependency to unmask fp.
            try
            {
                AppDomain.CurrentDomain.AssemblyLoad += (_, a) =>
                {
                    TryMaskLogged($"AssemblyLoad: {a.LoadedAssembly.GetName().Name}");
                };
                Diag("ModuleInit: AssemblyLoad hook registered");
            }
            catch (Exception ex)
            {
                Diag($"ModuleInit: AssemblyLoad hook FAILED {ex.GetType().Name}: {ex.Message}");
            }

            // FirstChanceException fires for every managed exception (incl. the
            // ArithmeticException) - re-mask immediately so the next op doesn't repeat.
            try
            {
                AppDomain.CurrentDomain.FirstChanceException += (_, e) =>
                {
                    TryMaskLogged($"FirstChance: {e.Exception.GetType().Name} :: {e.Exception.Message}");
                };
                Diag("ModuleInit: FirstChanceException hook registered");
            }
            catch (Exception ex)
            {
                Diag($"ModuleInit: FirstChanceException hook FAILED {ex.GetType().Name}: {ex.Message}");
            }

            TryMaskLogged("ModuleInit: exit");
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

        internal static void TryMaskLogged(string tag)
        {
            try
            {
                _controlfp_s(out var before, 0, 0);    // read without modifying
                _controlfp_s(out _, MCW_EM, MCW_EM);   // mask all fp exceptions
                _controlfp_s(out var after, 0, 0);     // read again to confirm

                // Only log when the EM bits differ from fully-masked, otherwise the log
                // floods with no-op entries. EM bits are the low byte plus DENORMAL bit.
                var emBefore = before & MCW_EM;
                if (emBefore != MCW_EM)
                {
                    Diag($"{tag} :: fpcw before=0x{before:X8}(EM=0x{emBefore:X5}) after=0x{after:X8} *** UNMASKED ***");
                }
                // (No log if everything was already masked - keeps log readable.)
            }
            catch
            {
                // best effort
            }
        }

        private static string LogPath
        {
            get
            {
                try
                {
                    return System.IO.Path.Combine(
                        System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location) ?? ".",
                        "fpguard.log");
                }
                catch
                {
                    return "fpguard.log";
                }
            }
        }

        private static void Diag(string line)
        {
            try
            {
                System.IO.File.AppendAllText(LogPath, $"{DateTime.Now:HH:mm:ss.fff} {line}{Environment.NewLine}");
            }
            catch
            {
                // best effort
            }
        }
    }
}
