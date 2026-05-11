using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public interface IInspectionEngineService
{
    Task<PartRunResult> RunPartAsync(InspectionModel model, PartRuntimeImage? image);
}
