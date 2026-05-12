using System.Windows;

namespace WpfInspectionApp.Infrastructure;

public interface IDialogOwner
{
    Window GetDialogOwner();
}
