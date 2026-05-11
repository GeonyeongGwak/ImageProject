using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class ModelWorkflowService : IModelWorkflowService
{
    private readonly IModelPersistenceService _modelPersistenceService;

    public ModelWorkflowService(IModelPersistenceService modelPersistenceService)
    {
        _modelPersistenceService = modelPersistenceService;
    }

    public string Save(InspectionModel model, string modelDirectory)
    {
        return _modelPersistenceService.Save(model, modelDirectory);
    }

    public ModelLoadWorkflowResult Load(string path)
    {
        var loaded = _modelPersistenceService.Load(path);
        if (loaded == null)
        {
            return new ModelLoadWorkflowResult(false, "Model load failed.", null);
        }

        loaded.EnsureStructure();
        return new ModelLoadWorkflowResult(true, $"Model loaded: {path}", loaded);
    }
}
