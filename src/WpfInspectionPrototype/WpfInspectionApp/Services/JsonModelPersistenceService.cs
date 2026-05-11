using System.IO;
using System.Text;
using System.Text.Json;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public sealed class JsonModelPersistenceService : IModelPersistenceService
{
    private readonly JsonSerializerOptions _jsonOptions = new() { WriteIndented = true };

    public string Save(InspectionModel model, string directory)
    {
        model.EnsureStructure();
        Directory.CreateDirectory(directory);
        var fileName = MakeSafeFileName(model.ModelName);
        var path = Path.Combine(directory, $"{fileName}.json");
        File.WriteAllText(path, JsonSerializer.Serialize(model, _jsonOptions));
        return path;
    }

    public InspectionModel? Load(string path)
    {
        var model = JsonSerializer.Deserialize<InspectionModel>(File.ReadAllText(path), _jsonOptions);
        model?.EnsureStructure();
        return model;
    }

    private static string MakeSafeFileName(string value)
    {
        var invalid = Path.GetInvalidFileNameChars();
        var builder = new StringBuilder(value.Length);
        foreach (var character in value)
        {
            builder.Append(invalid.Contains(character) ? '_' : character);
        }

        var fileName = builder.ToString().Trim();
        return string.IsNullOrWhiteSpace(fileName) ? "model" : fileName;
    }
}
