// Native Vectored Exception Handler for floating-point SEH.
//
// Why native (not managed C# VEH): under VS native debugging, every SEH on every
// thread (DLL load notifications, breakpoints, page faults, trace exceptions, plus
// our fp inexact) routes through every registered VEH. A managed VEH incurs a
// CLR managed-to-native transition (~200 bytes of stack frames per call) for EACH
// SEH — the cumulative stack consumption from VS-debugger-injected SEHs is enough
// to trigger 0xC00000FD STACK_OVERFLOW before any real fp exception even happens.
//
// This C++ implementation does ~16 bytes/call. Same CONTEXT.MxCsr + FltSave fix
// logic the C# version had, just without the CLR transition.

#include <windows.h>

namespace
{
    // x64 CONTEXT offsets (verified against winnt.h on Windows 10/11 SDK):
    //   P1Home..P6Home : 0x00..0x2F (6 * DWORD64)
    //   ContextFlags   : 0x30 (DWORD)
    //   MxCsr          : 0x34 (DWORD)
    //   ...
    //   FltSave (XMM_SAVE_AREA32) at 0x100:
    //     ControlWord (x87 FCW) : 0x100 (WORD)
    //     StatusWord  (x87 FSW) : 0x102 (WORD)
    //     ...
    //     MxCsr (duplicate)     : 0x118 (DWORD)
    constexpr SIZE_T CONTEXT_MXCSR_OFFSET_X64           = 0x34;
    constexpr SIZE_T CONTEXT_FLTSAVE_CONTROLWORD_OFFSET = 0x100;
    constexpr SIZE_T CONTEXT_FLTSAVE_STATUSWORD_OFFSET  = 0x102;
    constexpr SIZE_T CONTEXT_FLTSAVE_MXCSR_OFFSET       = 0x118;

    // MxCsr exception mask bits 7..12 (set = mask all 6 fp exceptions).
    // Status bits 0..5 cleared each time so the resumed instruction starts clean.
    constexpr DWORD MXCSR_MASK_ALL    = 0x1F80;
    constexpr DWORD MXCSR_STATUS_BITS = 0x003F;

    // x87 FPU control word low 6 bits = exception masks.
    constexpr WORD  X87_FCW_MASK_ALL  = 0x003F;

    volatile LONG g_fpSwallowCount = 0;
    PVOID         g_vehHandle = nullptr;

    inline bool IsFpExceptionCode(DWORD code) noexcept
    {
        return (code >= STATUS_FLOAT_DENORMAL_OPERAND && code <= STATUS_FLOAT_UNDERFLOW)
            || code == STATUS_FLOAT_MULTIPLE_FAULTS
            || code == STATUS_FLOAT_MULTIPLE_TRAPS;
    }

    LONG WINAPI FpVectoredHandler(EXCEPTION_POINTERS* ep) noexcept
    {
        // Hot-path bail for non-fp: must be as cheap as possible because under
        // native debugging this runs on EVERY SEH (incl. VS-injected breakpoint /
        // DLL-load / page-guard SEHs). One DWORD compare + branch.
        if (!ep || !ep->ExceptionRecord) return EXCEPTION_CONTINUE_SEARCH;
        const DWORD code = ep->ExceptionRecord->ExceptionCode;
        if (!IsFpExceptionCode(code)) return EXCEPTION_CONTINUE_SEARCH;

        CONTEXT* ctx = ep->ContextRecord;
        if (!ctx) return EXCEPTION_CONTINUE_SEARCH;

        BYTE* base = reinterpret_cast<BYTE*>(ctx);

        // (1) Top-level CONTEXT.MxCsr
        DWORD& topMxCsr = *reinterpret_cast<DWORD*>(base + CONTEXT_MXCSR_OFFSET_X64);
        topMxCsr = (topMxCsr | MXCSR_MASK_ALL) & ~MXCSR_STATUS_BITS;

        // (2) FltSave.MxCsr — what xrstor actually uses on Windows 10+.
        DWORD& fltMxCsr = *reinterpret_cast<DWORD*>(base + CONTEXT_FLTSAVE_MXCSR_OFFSET);
        fltMxCsr = (fltMxCsr | MXCSR_MASK_ALL) & ~MXCSR_STATUS_BITS;

        // (3) FltSave.ControlWord (x87 FCW) + clear FSW. Covers rare x87 paths.
        WORD& fcw = *reinterpret_cast<WORD*>(base + CONTEXT_FLTSAVE_CONTROLWORD_OFFSET);
        fcw = static_cast<WORD>(fcw | X87_FCW_MASK_ALL);
        *reinterpret_cast<WORD*>(base + CONTEXT_FLTSAVE_STATUSWORD_OFFSET) = 0;

        InterlockedIncrement(&g_fpSwallowCount);
        return EXCEPTION_CONTINUE_EXECUTION;
    }
}

extern "C" __declspec(dllexport) int MptiBridgeInstallFpVeh()
{
    // Idempotent — calling twice is safe (Windows ignores duplicate handler).
    if (g_vehHandle) return 1;
    // FirstHandler=1 -> place at front of chain. Order doesn't change stack usage
    // (every VEH runs anyway), but earlier dispatch lets us swallow before any
    // managed VEH (if both were registered).
    g_vehHandle = AddVectoredExceptionHandler(1u, FpVectoredHandler);
    return g_vehHandle != nullptr ? 0 : -1;
}

extern "C" __declspec(dllexport) long MptiBridgeGetFpSwallowCount()
{
    return InterlockedCompareExchange(&g_fpSwallowCount, 0, 0);
}
