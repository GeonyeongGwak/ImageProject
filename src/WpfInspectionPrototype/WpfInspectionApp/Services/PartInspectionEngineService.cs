using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class PartInspectionEngineService : IInspectionEngineService
{
    private readonly PartInspectionRuntime _runtime = new();

    public Task<PartRunResult> RunPartAsync(InspectionModel model, PartRuntimeImage? image)
    {
        return Task.Run(() => _runtime.Run(model, image));
    }
}
