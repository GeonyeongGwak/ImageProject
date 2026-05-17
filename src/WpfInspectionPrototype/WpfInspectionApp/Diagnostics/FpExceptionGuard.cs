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
    // Aggressive FPU exception mask installed before WPF startup.
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

        // x64 CONTEXT offsets we care about. The structure has a top-level MxCsr at
        // offset 0x34 AND a FltSave (XMM_SAVE_AREA32) sub-block at offset 0x100 with
        // its own ControlWord (x87 FCW) and a duplicate MxCsr. On Windows 10+ the
        // FltSave fields actually take precedence on EXCEPTION_CONTINUE_EXECUTION
        // restore — leaving them alone means the fp exception re-fires immediately,
        // VEH is re-invoked, and stack overflows after a few iterations. So we MUST
        // write all three.
        //
        //   P1Home..P6Home : 6 * DWORD64 = 48 bytes      -> 0x00..0x2F
        //   ContextFlags   : DWORD at                      0x30
        //   MxCsr          : DWORD at                      0x34
        //   ...
        //   FltSave (XMM_SAVE_AREA32) at                   0x100
        //     ControlWord (x87 FCW, WORD) at               0x100
        //     StatusWord  (x87 FSW, WORD) at               0x102
        //     ...
        //     MxCsr (duplicate, DWORD) at                  0x118
        private const int CONTEXT_MXCSR_OFFSET_X64           = 0x34;
        private const int CONTEXT_FLTSAVE_CONTROLWORD_OFFSET = 0x100;
        private const int CONTEXT_FLTSAVE_STATUSWORD_OFFSET  = 0x102;
        private const int CONTEXT_FLTSAVE_MXCSR_OFFSET       = 0x118;

        // x87 FPU control word: low 6 bits are exception masks (1 = masked). Default
        // CW is 0x027F = all 6 masked + round-to-nearest + 53-bit precision.
        private const ushort X87_FCW_MASK_ALL = 0x003F;
        private const ushort X87_FCW_DEFAULT  = 0x027F;

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
        private static long s_vehGiveUpCount;

        // Per-thread re-entry depth. If the same thread enters the VEH more than a few
        // times in a row, our CONTEXT modification clearly isn't sticking — give up and
        // return EXCEPTION_CONTINUE_SEARCH so the stack doesn't overflow. (Saw 0xC00000FD
        // stack overflow with no FltSave update — Windows 10+ uses FltSave.MxCsr to
        // restore SSE state, and the original CONTEXT.MxCsr update was ignored.)
        [System.ThreadStatic] private static int t_vehDepth;

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

            // Do not touch WPF media/rendering APIs from the module initializer.
            // Under VS native debugging, that can pull PresentationCore into the
            // process before Application construction and fail before App-level
            // handlers can run. App.OnStartup applies software rendering after WPF
            // has reached a safer initialization point.
            Diag("ModuleInit: WPF render-mode setup deferred to App.OnStartup");

            // Note: AssemblyLoad and FirstChanceException hooks were removed.
            //
            // FirstChanceException + Diag's File.AppendAllText was a hidden infinite-
            // recursion bug. If File.AppendAllText itself raised a first-chance
            // exception (path resolution, lock contention, ACL fail under VS attach,
            // etc.), the handler called Diag, which raised another first-chance,
            // which re-entered the handler... -> 0xC00000FD stack overflow. Only
            // triggered under VS native debugging because the debugger increases
            // file I/O contention enough to push the first failure over the edge.
            //
            // VEH already handles fp masking on every thread; these hooks were only
            // diagnostic and not worth the risk.

            try
            {
                AppDomain.CurrentDomain.UnhandledException += (_, e) =>
                {
                    Diag($"UNHANDLED: terminating={e.IsTerminating} exception={(e.ExceptionObject as Exception)?.GetType().FullName} :: {(e.ExceptionObject as Exception)?.Message}");
                };
                AppDomain.CurrentDomain.ProcessExit += (_, _) =>
                    Diag($"ProcessExit (VEH swallowed {System.Threading.Interlocked.Read(ref s_fpSwallowCount)} fp SEHs, gave up {System.Threading.Interlocked.Read(ref s_vehGiveUpCount)} times)");
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
        // than the original fp issue. Also MUST avoid CRT fp calls (_controlfp_s etc.)
        // because those have been observed to themselves perform fp ops that re-enter
        // the VEH and blow the stack.
        private static int OnVectoredException(IntPtr exceptionInfo)
        {
            // Re-entry guard. If we've taken multiple consecutive faults on the same
            // thread, our CONTEXT mods clearly aren't being honored — bail out instead
            // of overflowing the stack (saw 0xC00000FD without this).
            t_vehDepth++;
            try
            {
                if (t_vehDepth > 4)
                {
                    System.Threading.Interlocked.Increment(ref s_vehGiveUpCount);
                    return EXCEPTION_CONTINUE_SEARCH;
                }

                // EXCEPTION_POINTERS { ExceptionRecord*, ContextRecord* }
                IntPtr pRecord  = Marshal.ReadIntPtr(exceptionInfo, 0);
                IntPtr pContext = Marshal.ReadIntPtr(exceptionInfo, IntPtr.Size);
                if (pRecord == IntPtr.Zero || pContext == IntPtr.Zero)
                    return EXCEPTION_CONTINUE_SEARCH;

                // EXCEPTION_RECORD.ExceptionCode at offset 0 (DWORD).
                uint code = unchecked((uint)Marshal.ReadInt32(pRecord, 0));
                bool isFp =
                    (code >= STATUS_FLOAT_DENORMAL_OPERAND && code <= STATUS_FLOAT_UNDERFLOW) ||
                    code == STATUS_FLOAT_MULTIPLE_FAULTS ||
                    code == STATUS_FLOAT_MULTIPLE_TRAPS;
                if (!isFp) return EXCEPTION_CONTINUE_SEARCH;

                // (1) Top-level CONTEXT.MxCsr (legacy / some restore paths).
                int mxcsrTop = Marshal.ReadInt32(pContext, CONTEXT_MXCSR_OFFSET_X64);
                mxcsrTop |=  MXCSR_MASK_ALL;
                mxcsrTop &= ~MXCSR_STATUS_BITS;
                Marshal.WriteInt32(pContext, CONTEXT_MXCSR_OFFSET_X64, mxcsrTop);

                // (2) FltSave.MxCsr — on Windows 10+ this is what xrstor uses to
                //     reconstruct SSE state on EXCEPTION_CONTINUE_EXECUTION. The
                //     top-level CONTEXT.MxCsr alone is NOT sufficient.
                int mxcsrFlt = Marshal.ReadInt32(pContext, CONTEXT_FLTSAVE_MXCSR_OFFSET);
                mxcsrFlt |=  MXCSR_MASK_ALL;
                mxcsrFlt &= ~MXCSR_STATUS_BITS;
                Marshal.WriteInt32(pContext, CONTEXT_FLTSAVE_MXCSR_OFFSET, mxcsrFlt);

                // (3) x87 FPU control word (FCW) + clear status. Covers the rare path
                //     where the faulting instruction is x87 (e.g. fnstcw / fld).
                short fcw = Marshal.ReadInt16(pContext, CONTEXT_FLTSAVE_CONTROLWORD_OFFSET);
                fcw = unchecked((short)((ushort)fcw | X87_FCW_MASK_ALL));
                Marshal.WriteInt16(pContext, CONTEXT_FLTSAVE_CONTROLWORD_OFFSET, fcw);
                Marshal.WriteInt16(pContext, CONTEXT_FLTSAVE_STATUSWORD_OFFSET, 0);

                System.Threading.Interlocked.Increment(ref s_fpSwallowCount);
                return EXCEPTION_CONTINUE_EXECUTION;
            }
            catch
            {
                return EXCEPTION_CONTINUE_SEARCH;
            }
            finally
            {
                t_vehDepth--;
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

        // Re-entry guard: if Diag's File I/O itself raises a first-chance exception
        // and a handler somewhere calls back into Diag, we'd recurse forever and blow
        // the stack. This guard makes recursive calls a no-op.
        [System.ThreadStatic] private static bool t_inDiag;

        // Public so App.xaml.cs / MainWindow.xaml.cs can drop breadcrumbs into fpguard.log
        // without re-implementing the re-entry guard. Best-effort: never throws, never
        // recurses.
        internal static void Diag(string line)
        {
            if (t_inDiag) return;
            t_inDiag = true;
            try
            {
                System.IO.File.AppendAllText(LogPath, $"{DateTime.Now:HH:mm:ss.fff} {line}{Environment.NewLine}");
            }
            catch
            {
                // best effort
            }
            finally
            {
                t_inDiag = false;
            }
        }
    }
}
