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
    // Runs at module load (before Main, before any static ctor of any WpfInspectionApp type,
    // and - crucially - before WPF / PresentationFramework / VS XAML Tap injection touches
    // floating-point transform math). Masks every x87 FPU exception so an early FE_INEXACT
    // (raised because some loaded native DLL - MFC / IPP / PEM3D ActiveX / VS WpfTap -
    // unmasked fp exceptions) cannot crash WPF's Matrix.CreateRotationRadians during
    // Application init.
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
            TryMask();
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
    }
}
