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
    // FPU exception swallower for managed/native mixed debugging.
    //
    // Root cause (confirmed via fpguard.log): under VS native debugging the WPF native
    // graphics / WindowsCodecs / UIAutomation init paths execute fp ops while the x87
    // exception mask bits are unmasked (NVIDIA D3D driver and similar legacy native
    // components do this on init). Result: 0xC000008F SEH -> ArithmeticException ->
    // WPF Application init aborts -> 0xC0000005 process exit.
    //
    // Strategy:
    //   1. Mask FPU at ModuleInit (Main + WPF static init).
    //   2. Force WPF SoftwareOnly so D3D probe is minimized.
    //   3. **Install a Vectored Exception Handler** that fires before any SEH frame
    //      handler / CLR translation. The VEH re-masks FPU + clears fp status + returns
    //      EXCEPTION_CONTINUE_EXECUTION. The faulting instruction re-runs with masked
    //      exceptions and produces the (well-defined IEEE) default result. CLR never
    //      sees the exception, no ArithmeticException, no app crash.
    internal static class FpExceptionGuard
    {
        // _MCW_EM (mask of all 6 fp exception bits)
        private const uint MCW_EM = 0x0008001F;

        [DllImport("ucrtbase.dll", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        private static extern int _controlfp_s(out uint currentControl, uint newControl, uint mask);

        [DllImport("ucrtbase.dll", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        private static extern uint _clearfp();

        // --- VEH P/Invoke ---
        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate int VectoredHandlerDelegate(IntPtr exceptionInfo);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr AddVectoredExceptionHandler(uint first, VectoredHandlerDelegate handler);

        private const int EXCEPTION_CONTINUE_SEARCH    = 0;
        private const int EXCEPTION_CONTINUE_EXECUTION = -1;

        // x87 SEH codes we want to swallow. Includes inexact/underflow/overflow/invalid/
        // divbyzero/denormal/stack-check.
        private const uint STATUS_FLOAT_DENORMAL_OPERAND  = 0xC000008D;
        private const uint STATUS_FLOAT_DIVIDE_BY_ZERO    = 0xC000008E;
        private const uint STATUS_FLOAT_INEXACT_RESULT    = 0xC000008F;
        private const uint STATUS_FLOAT_INVALID_OPERATION = 0xC0000090;
        private const uint STATUS_FLOAT_OVERFLOW          = 0xC0000091;
        private const uint STATUS_FLOAT_STACK_CHECK       = 0xC0000092;
        private const uint STATUS_FLOAT_UNDERFLOW         = 0xC0000093;

        // Keep delegate alive for the process lifetime (otherwise GC collects it
        // and the native vector table holds a dangling pointer).
        private static VectoredHandlerDelegate? _vehHandler;
        private static long _fpSwallowCount;

        [ModuleInitializer]
        internal static void MaskAtModuleLoad()
        {
            Diag("ModuleInit: enter");
            TryMaskLogged("ModuleInit: after first mask");

            // 1) Install VEH FIRST so it covers everything that follows.
            try
            {
                _vehHandler = OnVectoredException;
                var h = AddVectoredExceptionHandler(1u /* first = run before others */, _vehHandler);
                Diag($"ModuleInit: VEH installed handle=0x{h.ToInt64():X}");
            }
            catch (Exception ex)
            {
                Diag($"ModuleInit: VEH install FAILED {ex.GetType().Name}: {ex.Message}");
            }

            // 2) Force WPF software rendering before PresentationCore probes the GPU.
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

            // 3) Managed-side hooks (diagnostic + secondary safety net).
            try
            {
                AppDomain.CurrentDomain.AssemblyLoad += (_, a) =>
                {
                    Diag($"AssemblyLoad: {a.LoadedAssembly.GetName().Name}");
                    TryMaskLogged($"  after AssemblyLoad: {a.LoadedAssembly.GetName().Name}");
                };
                AppDomain.CurrentDomain.FirstChanceException += (_, e) =>
                {
                    Diag($"FirstChance: {e.Exception.GetType().FullName} :: {e.Exception.Message}");
                    TryMaskLogged($"  after FirstChance: {e.Exception.GetType().Name}");
                };
                AppDomain.CurrentDomain.UnhandledException += (_, e) =>
                {
                    Diag($"UNHANDLED: terminating={e.IsTerminating} exception={(e.ExceptionObject as Exception)?.GetType().FullName} :: {(e.ExceptionObject as Exception)?.Message}");
                };
                AppDomain.CurrentDomain.ProcessExit += (_, _) => Diag($"ProcessExit (VEH swallowed {Interlocked.Read(ref _fpSwallowCount)} fp SEHs)");
                Diag("ModuleInit: managed hooks registered");
            }
            catch (Exception ex)
            {
                Diag($"ModuleInit: managed hook register FAILED {ex.GetType().Name}: {ex.Message}");
            }

            TryMaskLogged("ModuleInit: exit");
        }

        // Native VEH entry point. Runs before any SEH frame handler and before the CLR
        // gets a chance to translate the SEH into ArithmeticException. MUST NOT throw
        // or do anything that allocates managed memory in the general case.
        private static int OnVectoredException(IntPtr exceptionInfo)
        {
            try
            {
                // EXCEPTION_POINTERS { ExceptionRecord*, ContextRecord* }
                var pRecord = Marshal.ReadIntPtr(exceptionInfo, 0);
                // EXCEPTION_RECORD.ExceptionCode is at offset 0.
                var code = unchecked((uint)Marshal.ReadInt32(pRecord, 0));

                if (code >= STATUS_FLOAT_DENORMAL_OPERAND && code <= STATUS_FLOAT_UNDERFLOW)
                {
                    // Re-mask all fp exception bits and clear pending status. The faulting
                    // instruction's defined IEEE result is now produced and execution
                    // resumes as if nothing happened.
                    _controlfp_s(out _, MCW_EM, MCW_EM);
                    try { _clearfp(); } catch { /* ignore */ }
                    Interlocked.Increment(ref _fpSwallowCount);
                    return EXCEPTION_CONTINUE_EXECUTION;
                }
            }
            catch
            {
                // never propagate from VEH
            }
            return EXCEPTION_CONTINUE_SEARCH;
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
                _controlfp_s(out var before, 0, 0);
                _controlfp_s(out _, MCW_EM, MCW_EM);
                _controlfp_s(out var after, 0, 0);
                var emBefore = before & MCW_EM;
                if (emBefore != MCW_EM)
                {
                    Diag($"{tag} :: fpcw before=0x{before:X8}(EM=0x{emBefore:X5}) after=0x{after:X8} *** UNMASKED ***");
                }
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
