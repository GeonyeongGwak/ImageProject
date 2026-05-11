using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public interface IAlignConditionService
{
    int CalculateMaskDensity(InspectionModel model);
}
