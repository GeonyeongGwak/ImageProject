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

        // MxCsr (SSE control register) bits:
        //   bits 0..5  = exception status flags (IE, DE, ZE, OE, UE, PE) — sticky
        //   bits 7..12 = exception masks (IM, DM, ZM, OM, UM, PM); 1 = masked
        // Default Windows MxCsr is 0x1F80 (all 6 exceptions masked, no status flags).
        private const int MXCSR_MASK_ALL    = 0x1F80;
        private const int MXCSR_STATUS_BITS = 0x003F;

        // x64 CONTEXT offset where MxCsr lives.
        //   P1Home..P6Home : 6 * DWORD64 = 48 bytes  -> 0x00..0x2F
        //   ContextFlags   : DWORD at 0x30
        //   MxCsr          : DWORD at 0x34
        private const int CONTEXT_MXCSR_OFFSET_X64 = 0x34;

        // x64 NTSTATUS codes for floating-point SEH (all of these are what the SSE
        // hardware can raise depending on which mask bit is cleared).
        private const uint STATUS_FLOAT_DENORMAL_OPERAND  = 0xC000008D;
        private const uint STATUS_FLOAT_DIVIDE_BY_ZERO    = 0xC000008E;
        private const uint STATUS_FLOAT_INEXACT_RESULT    = 0xC000008F;
        private const uint STATUS_FLOAT_INVALID_OPERATION = 0xC0000090;
        private const uint STATUS_FLOAT_OVERFLOW          = 0xC0000091;
        private const uint STATUS_FLOAT_STACK_CHECK       = 0xC0000092;
        private const uint STATUS_FLOAT_UNDERFLOW         = 0xC0000093;
        private const uint STATUS_FLOAT_MULTIPLE_FAULTS   = 0xC00002B4;
        private const uint STATUS_FLOAT_MULTIPLE_TRAPS    = 0xC00002B5;

        private const int EXCEPTION_CONTINUE_SEARCH    =  0;
        private const int EXCEPTION_CONTINUE_EXECUTION = -1;

        [DllImport("ucrtbase.dll", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        private static extern int _controlfp_s(out uint currentControl, uint newControl, uint mask);

        [DllImport("ucrtbase.dll", CallingConvention = CallingConvention.Cdecl, ExactSpelling = true)]
        private static extern uint _clearfp();

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate int VectoredHandlerDelegate(IntPtr exceptionInfo);

        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern IntPtr AddVectoredExceptionHandler(uint first, VectoredHandlerDelegate handler);

        // Keep alive for process lifetime so the GC doesn't collect the delegate while
        // it's still referenced by the Windows vectored-handler chain.
        private static VectoredHandlerDelegate? s_vehHandler;
        private static long s_fpSwallowCount;

        [ModuleInitializer]
        internal static void MaskAtModuleLoad()
        {
            Diag("ModuleInit: enter");
            TryMaskLogged("ModuleInit: after first mask");

            // Install Vectored Exception Handler FIRST so it can swallow fp SEH that
            // fires on render threads / D3D probe threads / any thread we don't own.
            //
            // The previous VEH attempt (commit 8db4626, reverted) called _controlfp_s
            // on the current thread but did NOT modify EXCEPTION_POINTERS->ContextRecord
            // ->MxCsr. When Windows resumes execution after EXCEPTION_CONTINUE_EXECUTION,
            // it restores thread state FROM ContextRecord — so the fp mask we just set
            // via _controlfp_s was overwritten and the faulting instruction re-fired the
            // exact same exception (infinite loop / re-trigger / fail).
            //
            // This version writes MxCsr |= 0x1F80 (mask all 6 fp exceptions) directly
            // into ContextRecord so the resumed instruction sees masked state.
            try
            {
                s_vehHandler = OnVectoredException;
                var h = AddVectoredExceptionHandler(1u /* first = run before others */, s_vehHandler);
                Diag($"ModuleInit: VEH installed handle=0x{h.ToInt64():X}");
            }
            catch (Exception ex)
            {
                Diag($"ModuleInit: VEH install FAILED {ex.GetType().Name}: {ex.Message}");
            }

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
                    Diag($"AssemblyLoad: {a.LoadedAssembly.GetName().Name}");
                    TryMaskLogged($"  after AssemblyLoad: {a.LoadedAssembly.GetName().Name}");
                };
                Diag("ModuleInit: AssemblyLoad hook registered");
            }
            catch (Exception ex)
            {
                Diag($"ModuleInit: AssemblyLoad hook FAILED {ex.GetType().Name}: {ex.Message}");
            }

            try
            {
                AppDomain.CurrentDomain.FirstChanceException += (_, e) =>
                {
                    Diag($"FirstChance: {e.Exception.GetType().FullName} :: {e.Exception.Message}");
                    TryMaskLogged($"  after FirstChance: {e.Exception.GetType().Name}");
                };
                Diag("ModuleInit: FirstChanceException hook registered");
            }
            catch (Exception ex)
            {
                Diag($"ModuleInit: FirstChanceException hook FAILED {ex.GetType().Name}: {ex.Message}");
            }

            try
            {
                AppDomain.CurrentDomain.UnhandledException += (_, e) =>
                {
                    Diag($"UNHANDLED: terminating={e.IsTerminating} exception={(e.ExceptionObject as Exception)?.GetType().FullName} :: {(e.ExceptionObject as Exception)?.Message}");
                };
                AppDomain.CurrentDomain.ProcessExit += (_, _) =>
                    Diag($"ProcessExit (VEH swallowed {System.Threading.Interlocked.Read(ref s_fpSwallowCount)} fp SEHs)");
                Diag("ModuleInit: UnhandledException/ProcessExit hooks registered");
            }
            catch (Exception ex)
            {
                Diag($"ModuleInit: UnhandledException hook FAILED {ex.GetType().Name}: {ex.Message}");
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

        // Vectored exception handler. Runs on the faulting thread before any frame-based
        // SEH handler and before the CLR translates the SEH into ArithmeticException.
        //
        // MUST NOT throw — any exception that escapes here will crash the process worse
        // than the original fp issue. Wrapped in a `try { } catch { }` accordingly.
        private static int OnVectoredException(IntPtr exceptionInfo)
        {
            try
            {
                // EXCEPTION_POINTERS { ExceptionRecord*, ContextRecord* }
                IntPtr pRecord  = Marshal.ReadIntPtr(exceptionInfo, 0);
                IntPtr pContext = Marshal.ReadIntPtr(exceptionInfo, IntPtr.Size);
                if (pRecord == IntPtr.Zero || pContext == IntPtr.Zero)
                    return EXCEPTION_CONTINUE_SEARCH;

                // EXCEPTION_RECORD.ExceptionCode is at offset 0 (DWORD).
                uint code = unchecked((uint)Marshal.ReadInt32(pRecord, 0));

                bool isFp =
                    (code >= STATUS_FLOAT_DENORMAL_OPERAND && code <= STATUS_FLOAT_UNDERFLOW) ||
                    code == STATUS_FLOAT_MULTIPLE_FAULTS ||
                    code == STATUS_FLOAT_MULTIPLE_TRAPS;
                if (!isFp) return EXCEPTION_CONTINUE_SEARCH;

                // Critical: modify CONTEXT.MxCsr in place. Windows uses this on
                // EXCEPTION_CONTINUE_EXECUTION to restore the SSE control word for the
                // resumed instruction. If we only call _controlfp_s, the CONTEXT
                // restore will overwrite our mask and the instruction re-fires.
                int mxcsr = Marshal.ReadInt32(pContext, CONTEXT_MXCSR_OFFSET_X64);
                mxcsr |=  MXCSR_MASK_ALL;     // mask all 6 fp exception bits
                mxcsr &= ~MXCSR_STATUS_BITS;  // clear sticky status flags so retry sees clean state
                Marshal.WriteInt32(pContext, CONTEXT_MXCSR_OFFSET_X64, mxcsr);

                // Defensively also mask the running thread's fp state (covers x87 too,
                // _controlfp_s touches both CW and MxCsr on the CRT path).
                try
                {
                    _controlfp_s(out _, MCW_EM, MCW_EM);
                    _clearfp();
                }
                catch { /* never propagate */ }

                System.Threading.Interlocked.Increment(ref s_fpSwallowCount);
                return EXCEPTION_CONTINUE_EXECUTION;
            }
            catch
            {
                return EXCEPTION_CONTINUE_SEARCH;
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
