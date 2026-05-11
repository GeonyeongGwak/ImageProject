using System.IO;

namespace WpfInspectionApp.Services;

public sealed class ApplicationPathService : IApplicationPathService
{
    public string? FindDefaultImagePath()
    {
        foreach (var directory in EnumerateParents(new DirectoryInfo(AppContext.BaseDirectory)))
        {
            var outputCandidate = Path.Combine(directory.FullName, "Image", "2D.jpg");
            if (File.Exists(outputCandidate))
            {
                return outputCandidate;
            }

            var repoCandidate = Path.Combine(directory.FullName, "..", "..", "..", "Image", "2D.jpg");
            var fullRepoCandidate = Path.GetFullPath(repoCandidate);
            if (File.Exists(fullRepoCandidate))
            {
                return fullRepoCandidate;
            }
        }

        return null;
    }

    public string GetModelDirectory()
    {
        foreach (var directory in EnumerateParents(new DirectoryInfo(AppContext.BaseDirectory)))
        {
            if (File.Exists(Path.Combine(directory.FullName, "WpfInspectionPrototype.sln")))
            {
                return Path.Combine(directory.FullName, "Models");
            }
        }

        return Path.Combine(AppContext.BaseDirectory, "Models");
    }

    private static IEnumerable<DirectoryInfo> EnumerateParents(DirectoryInfo start)
    {
        for (var current = start; current != null; current = current.Parent)
        {
            yield return current;
        }
    }
}
