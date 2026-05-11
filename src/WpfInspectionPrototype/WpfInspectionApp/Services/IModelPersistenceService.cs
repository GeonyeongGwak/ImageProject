using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public interface IModelPersistenceService
{
    string Save(InspectionModel model, string directory);

    InspectionModel? Load(string path);
}
