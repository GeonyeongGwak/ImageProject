using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Text.Json;
using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public interface IPartImportService
{
    bool TryLoadImportPart(string path, out PartInspectionData importedPart, out string status, out string? importedImagePath, out string? importedPttPath);
}

public sealed class PartImportService : IPartImportService
{
    private readonly JsonSerializerOptions _importJsonOptions = new() { PropertyNameCaseInsensitive = true };

    public bool TryLoadImportPart(string path, out PartInspectionData importedPart, out string status, out string? importedImagePath, out string? importedPttPath)
    {
        importedPart = new PartInspectionData();
        importedImagePath = null;
        importedPttPath = null;

        var extension = System.IO.Path.GetExtension(path);
        if (string.Equals(extension, ".json", StringComparison.OrdinalIgnoreCase))
        {
            var json = File.ReadAllText(path);
            if (TryDeserializePartPayload(json, out importedPart))
            {
                status = "Part import payload loaded.";
                return true;
            }

            status = "Part import failed: supported WPF model/part JSON was not found.";
            return false;
        }

        if (string.Equals(extension, ".zip", StringComparison.OrdinalIgnoreCase))
        {
            return TryLoadImportPartZip(path, out importedPart, out status, out importedImagePath, out importedPttPath);
        }

        if (string.Equals(extension, ".xml", StringComparison.OrdinalIgnoreCase))
        {
            using var stream = File.OpenRead(path);
            using var initStream = OpenLegacyInitSidecar(path);
            if (LegacyRawPartImportAdapter.TryParse(stream, initStream, out importedPart, out status))
            {
                return true;
            }

            return false;
        }

        status = "Part import failed: select a .json, .zip, or .xml file.";
        return false;
    }

    private bool TryLoadImportPartZip(string path, out PartInspectionData importedPart, out string status, out string? importedImagePath, out string? importedPttPath)
    {
        importedPart = new PartInspectionData();
        importedImagePath = null;
        importedPttPath = null;

        if (TryLoadImportPartZipWithTar(path, out importedPart, out status, out importedImagePath, out importedPttPath))
        {
            return true;
        }

        if (TryLoadImportPartZipWithIonic(path, out importedPart, out status, out importedImagePath, out importedPttPath))
        {
            return true;
        }

        try
        {
            using var archive = ZipFile.OpenRead(path);
            var jsonEntries = archive.Entries
                .Where(entry => !string.IsNullOrWhiteSpace(entry.Name) && string.Equals(System.IO.Path.GetExtension(entry.Name), ".json", StringComparison.OrdinalIgnoreCase))
                .OrderBy(entry => ZipJsonPriority(entry.FullName))
                .ThenBy(entry => entry.FullName, StringComparer.OrdinalIgnoreCase)
                .ToList();

            foreach (var entry in jsonEntries)
            {
                using var stream = entry.Open();
                using var reader = new StreamReader(stream);
                var json = reader.ReadToEnd();
                if (TryDeserializePartPayload(json, out importedPart))
                {
                    ExtractImportAssets(path, archive, out importedImagePath, out importedPttPath);
                    status = $"Part import payload loaded from zip: {entry.FullName}";
                    return true;
                }
            }

            var xmlEntries = archive.Entries
                .Where(entry => !string.IsNullOrWhiteSpace(entry.Name) && string.Equals(System.IO.Path.GetExtension(entry.Name), ".xml", StringComparison.OrdinalIgnoreCase))
                .OrderBy(entry => ZipXmlPriority(entry.FullName))
                .ThenBy(entry => entry.FullName, StringComparer.OrdinalIgnoreCase)
                .ToList();

            foreach (var entry in xmlEntries)
            {
                using var stream = entry.Open();
                using var initStream = OpenLegacyInitEntry(archive, entry.FullName);
                if (LegacyRawPartImportAdapter.TryParse(stream, initStream, out importedPart, out status))
                {
                    ExtractImportAssets(path, archive, out importedImagePath, out importedPttPath);
                    status = $"{status} Source: {entry.FullName}";
                    return true;
                }
            }

            status = xmlEntries.Count > 0
                ? "Part import failed: XML files were found, but no RawDataContainer/WindowDataList part could be parsed."
                : "Part import failed: no supported WPF JSON or legacy RawData XML was found inside the zip.";
        }
        catch (InvalidDataException ex)
        {
            DiagnosticsLog.Write($"ZipArchive fallback failed: {ex}");
            status = "Part import failed: zip local header is damaged and tar extraction also failed.";
        }

        return false;
    }

    private bool TryLoadImportPartZipWithIonic(string path, out PartInspectionData importedPart, out string status, out string? importedImagePath, out string? importedPttPath)
    {
        importedPart = new PartInspectionData();
        importedImagePath = null;
        importedPttPath = null;

        try
        {
            using var archive = Ionic.Zip.ZipFile.Read(path);
            var jsonEntries = archive.Entries
                .Where(entry => !entry.IsDirectory && string.Equals(System.IO.Path.GetExtension(entry.FileName), ".json", StringComparison.OrdinalIgnoreCase))
                .OrderBy(entry => ZipJsonPriority(entry.FileName))
                .ThenBy(entry => entry.FileName, StringComparer.OrdinalIgnoreCase)
                .ToList();

            foreach (var entry in jsonEntries)
            {
                using var stream = entry.OpenReader();
                using var reader = new StreamReader(stream);
                var json = reader.ReadToEnd();
                if (TryDeserializePartPayload(json, out importedPart))
                {
                    ExtractImportAssets(archive, out importedImagePath, out importedPttPath);
                    status = $"Part import payload loaded from zip: {entry.FileName}";
                    DiagnosticsLog.Write($"Ionic zip import loaded JSON. image={importedImagePath ?? "<none>"}, ptt={importedPttPath ?? "<none>"}");
                    return true;
                }
            }

            var xmlEntries = archive.Entries
                .Where(entry => !entry.IsDirectory && string.Equals(System.IO.Path.GetExtension(entry.FileName), ".xml", StringComparison.OrdinalIgnoreCase))
                .OrderBy(entry => ZipXmlPriority(entry.FileName))
                .ThenBy(entry => entry.FileName, StringComparer.OrdinalIgnoreCase)
                .ToList();

            foreach (var entry in xmlEntries)
            {
                using var stream = entry.OpenReader();
                using var initStream = OpenLegacyInitEntry(archive, entry.FileName);
                if (LegacyRawPartImportAdapter.TryParse(stream, initStream, out importedPart, out status))
                {
                    ExtractImportAssets(archive, out importedImagePath, out importedPttPath);
                    status = $"{status} Source: {entry.FileName}";
                    DiagnosticsLog.Write($"Ionic zip import loaded XML. image={importedImagePath ?? "<none>"}, ptt={importedPttPath ?? "<none>"}");
                    return true;
                }
            }

            status = xmlEntries.Count > 0
                ? "Part import failed: XML files were found, but no RawDataContainer/WindowDataList part could be parsed."
                : "Part import failed: no supported WPF JSON or legacy RawData XML was found inside the zip.";
        }
        catch (Exception ex)
        {
            DiagnosticsLog.Write($"Ionic zip import failed: {ex}");
            status = $"Part import failed: DotNetZip fallback failed: {ex.Message}";
        }

        return false;
    }

    private bool TryLoadImportPartZipWithTar(string path, out PartInspectionData importedPart, out string status, out string? importedImagePath, out string? importedPttPath)
    {
        importedPart = new PartInspectionData();
        importedImagePath = null;
        importedPttPath = null;

        var directory = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "WpfInspectionPartImport");
        var importDirectory = System.IO.Path.Combine(directory, Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(importDirectory);

        if (!TryExtractZipWithTar(path, importDirectory, out var tarError))
        {
            DiagnosticsLog.Write($"zip tar extraction failed: {tarError}");
            status = "Part import failed: zip extraction failed.";
            return false;
        }

        var jsonFiles = Directory.EnumerateFiles(importDirectory, "*.json", SearchOption.AllDirectories)
            .OrderBy(file => ZipJsonPriority(GetRelativeImportName(importDirectory, file)))
            .ThenBy(file => file, StringComparer.OrdinalIgnoreCase)
            .ToList();

        foreach (var file in jsonFiles)
        {
            var json = File.ReadAllText(file);
            if (TryDeserializePartPayload(json, out importedPart))
            {
                importedImagePath = FindFirstExtractedImportImage(importDirectory);
                importedPttPath = FindFirstExtractedPtt(importDirectory);
                status = $"Part import payload loaded from zip: {GetRelativeImportName(importDirectory, file)}";
                DiagnosticsLog.Write($"Tar zip import loaded JSON. image={importedImagePath ?? "<none>"}, ptt={importedPttPath ?? "<none>"}");
                return true;
            }
        }

        var xmlFiles = Directory.EnumerateFiles(importDirectory, "*.xml", SearchOption.AllDirectories)
            .OrderBy(file => ZipXmlPriority(GetRelativeImportName(importDirectory, file)))
            .ThenBy(file => file, StringComparer.OrdinalIgnoreCase)
            .ToList();

        foreach (var file in xmlFiles)
        {
            using var stream = File.OpenRead(file);
            using var initStream = OpenLegacyInitSidecar(file);
            if (LegacyRawPartImportAdapter.TryParse(stream, initStream, out importedPart, out status))
            {
                importedImagePath = FindFirstExtractedImportImage(importDirectory);
                importedPttPath = FindFirstExtractedPtt(importDirectory);
                status = $"{status} Source: {GetRelativeImportName(importDirectory, file)}";
                DiagnosticsLog.Write($"Tar zip import loaded XML. image={importedImagePath ?? "<none>"}, ptt={importedPttPath ?? "<none>"}");
                return true;
            }
        }

        status = xmlFiles.Count > 0
            ? "Part import failed: XML files were found, but no RawDataContainer/WindowDataList part could be parsed."
            : "Part import failed: no supported WPF JSON or legacy RawData XML was found inside the zip.";
        return false;
    }

    private static bool TryExtractZipWithTar(string zipPath, string importDirectory, out string error)
    {
        error = string.Empty;
        var arguments = $"-xf {QuoteProcessArgument(zipPath)} -C {QuoteProcessArgument(importDirectory)}";
        var startInfo = new ProcessStartInfo
        {
            FileName = FindTarPath(),
            Arguments = arguments,
            CreateNoWindow = true,
            UseShellExecute = false,
            RedirectStandardError = true,
            RedirectStandardOutput = true
        };

        using var process = Process.Start(startInfo);
        if (process == null)
        {
            error = "tar.exe process could not be started.";
            return false;
        }

        error = process.StandardError.ReadToEnd();
        process.StandardOutput.ReadToEnd();
        process.WaitForExit();
        return process.ExitCode == 0;
    }

    private static string GetRelativeImportName(string importDirectory, string path)
    {
        var relative = path.StartsWith(importDirectory, StringComparison.OrdinalIgnoreCase)
            ? path.Substring(importDirectory.Length).TrimStart(System.IO.Path.DirectorySeparatorChar, System.IO.Path.AltDirectorySeparatorChar)
            : path;
        return relative.Replace('\\', '/');
    }

    private static string? FindFirstExtractedImportImage(string importDirectory)
    {
        return Directory.EnumerateFiles(importDirectory, "*.*", SearchOption.AllDirectories)
            .Where(file => IsImageExtension(System.IO.Path.GetExtension(file)) && new FileInfo(file).Length > 0)
            .OrderBy(file => file, StringComparer.OrdinalIgnoreCase)
            .FirstOrDefault();
    }

    private static string? FindFirstExtractedPtt(string importDirectory)
    {
        return Directory.EnumerateFiles(importDirectory, "*.ptt", SearchOption.AllDirectories)
            .Where(file => new FileInfo(file).Length > 0)
            .OrderBy(file => file, StringComparer.OrdinalIgnoreCase)
            .FirstOrDefault();
    }

    private static Stream? OpenLegacyInitSidecar(string rawXmlPath)
    {
        if (rawXmlPath.Contains("_Init", StringComparison.OrdinalIgnoreCase))
        {
            return null;
        }

        var directory = System.IO.Path.GetDirectoryName(rawXmlPath);
        if (string.IsNullOrWhiteSpace(directory))
        {
            return null;
        }

        var preferred = System.IO.Path.Combine(
            directory,
            $"{System.IO.Path.GetFileNameWithoutExtension(rawXmlPath)}_Init.xml");
        if (File.Exists(preferred))
        {
            return File.OpenRead(preferred);
        }

        var fallback = Directory.EnumerateFiles(directory, "*_Init.xml", SearchOption.TopDirectoryOnly)
            .OrderBy(file => file, StringComparer.OrdinalIgnoreCase)
            .FirstOrDefault();
        return fallback == null ? null : File.OpenRead(fallback);
    }

    private static Stream? OpenLegacyInitEntry(ZipArchive archive, string rawXmlEntryName)
    {
        var initEntry = FindLegacyInitEntry(
            archive.Entries.Where(entry => !string.IsNullOrWhiteSpace(entry.Name)).Select(entry => entry.FullName),
            rawXmlEntryName);
        return initEntry == null ? null : archive.GetEntry(initEntry)?.Open();
    }

    private static Stream? OpenLegacyInitEntry(Ionic.Zip.ZipFile archive, string rawXmlEntryName)
    {
        var initEntry = FindLegacyInitEntry(
            archive.Entries.Where(entry => !entry.IsDirectory).Select(entry => entry.FileName),
            rawXmlEntryName);
        return initEntry == null ? null : archive[initEntry]?.OpenReader();
    }

    private static string? FindLegacyInitEntry(IEnumerable<string> entryNames, string rawXmlEntryName)
    {
        if (rawXmlEntryName.Contains("_Init", StringComparison.OrdinalIgnoreCase))
        {
            return null;
        }

        var normalizedRawName = rawXmlEntryName.Replace('\\', '/');
        var rawDirectory = normalizedRawName.Contains('/')
            ? normalizedRawName.Substring(0, normalizedRawName.LastIndexOf('/'))
            : "";
        var rawBaseName = System.IO.Path.GetFileNameWithoutExtension(normalizedRawName);
        var preferred = string.IsNullOrWhiteSpace(rawDirectory)
            ? $"{rawBaseName}_Init.xml"
            : $"{rawDirectory}/{rawBaseName}_Init.xml";

        var names = entryNames
            .Select(name => name.Replace('\\', '/'))
            .Where(name => string.Equals(System.IO.Path.GetExtension(name), ".xml", StringComparison.OrdinalIgnoreCase))
            .ToList();
        return names.FirstOrDefault(name => string.Equals(name, preferred, StringComparison.OrdinalIgnoreCase))
            ?? names.FirstOrDefault(name =>
                name.StartsWith(rawDirectory, StringComparison.OrdinalIgnoreCase)
                && name.EndsWith("_Init.xml", StringComparison.OrdinalIgnoreCase));
    }

    private static void ExtractImportAssets(string zipPath, ZipArchive archive, out string? importedImagePath, out string? importedPttPath)
    {
        importedImagePath = null;
        importedPttPath = null;

        var pttEntry = archive.Entries.FirstOrDefault(entry =>
            !string.IsNullOrWhiteSpace(entry.Name) &&
            string.Equals(System.IO.Path.GetExtension(entry.Name), ".ptt", StringComparison.OrdinalIgnoreCase));
        var imageEntry = FindFirstImportImageEntry(archive);

        if (pttEntry == null && imageEntry == null)
        {
            return;
        }

        var directory = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "WpfInspectionPartImport");
        var importDirectory = System.IO.Path.Combine(directory, Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(importDirectory);

        if (TryExtractImportAssetsWithTar(zipPath, archive, importDirectory, pttEntry, imageEntry, out importedImagePath, out importedPttPath))
        {
            DiagnosticsLog.Write($"Import assets extracted: image={importedImagePath ?? "<none>"}, ptt={importedPttPath ?? "<none>"}");
            return;
        }

        importedImagePath = ExtractFirstImportImage(archive);
        importedPttPath = ExtractFirstImportPtt(archive);
        DiagnosticsLog.Write($"Import assets extracted by ZipArchive fallback: image={importedImagePath ?? "<none>"}, ptt={importedPttPath ?? "<none>"}");
    }

    private static void ExtractImportAssets(Ionic.Zip.ZipFile archive, out string? importedImagePath, out string? importedPttPath)
    {
        importedImagePath = null;
        importedPttPath = null;

        var pttEntry = archive.Entries.FirstOrDefault(entry =>
            !entry.IsDirectory &&
            string.Equals(System.IO.Path.GetExtension(entry.FileName), ".ptt", StringComparison.OrdinalIgnoreCase));
        var imageEntry = archive.Entries.FirstOrDefault(entry =>
            !entry.IsDirectory &&
            IsImageExtension(System.IO.Path.GetExtension(entry.FileName)));

        if (pttEntry == null && imageEntry == null)
        {
            return;
        }

        var directory = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "WpfInspectionPartImport");
        var importDirectory = System.IO.Path.Combine(directory, Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(importDirectory);

        if (imageEntry != null)
        {
            imageEntry.Extract(importDirectory, Ionic.Zip.ExtractExistingFileAction.OverwriteSilently);
            importedImagePath = GetExtractedIonicPath(importDirectory, imageEntry.FileName);
        }

        if (pttEntry == null)
        {
            return;
        }

        pttEntry.Extract(importDirectory, Ionic.Zip.ExtractExistingFileAction.OverwriteSilently);
        importedPttPath = GetExtractedIonicPath(importDirectory, pttEntry.FileName);

        var pttBaseName = System.IO.Path.GetFileNameWithoutExtension(pttEntry.FileName);
        var sourceDirectory = GetZipDirectoryName(pttEntry.FileName);
        var companionEntries = archive.Entries
            .Where(entry =>
                !entry.IsDirectory &&
                string.Equals(GetZipDirectoryName(entry.FileName), sourceDirectory, StringComparison.OrdinalIgnoreCase) &&
                string.Equals(System.IO.Path.GetFileNameWithoutExtension(entry.FileName), pttBaseName, StringComparison.OrdinalIgnoreCase) &&
                IsPttCompanionExtension(System.IO.Path.GetExtension(entry.FileName)))
            .ToList();

        foreach (var companionEntry in companionEntries)
        {
            companionEntry.Extract(importDirectory, Ionic.Zip.ExtractExistingFileAction.OverwriteSilently);
        }
    }

    private static string GetExtractedIonicPath(string importDirectory, string entryName)
    {
        var localName = entryName.Replace('/', System.IO.Path.DirectorySeparatorChar).Replace('\\', System.IO.Path.DirectorySeparatorChar);
        return System.IO.Path.Combine(importDirectory, localName);
    }

    private static ZipArchiveEntry? FindFirstImportImageEntry(ZipArchive archive)
    {
        return archive.Entries.FirstOrDefault(entry =>
        {
            if (string.IsNullOrWhiteSpace(entry.Name))
            {
                return false;
            }

            var extension = System.IO.Path.GetExtension(entry.Name);
            return string.Equals(extension, ".jpg", StringComparison.OrdinalIgnoreCase)
                || string.Equals(extension, ".jpeg", StringComparison.OrdinalIgnoreCase)
                || string.Equals(extension, ".png", StringComparison.OrdinalIgnoreCase)
                || string.Equals(extension, ".bmp", StringComparison.OrdinalIgnoreCase)
                || string.Equals(extension, ".tif", StringComparison.OrdinalIgnoreCase)
                || string.Equals(extension, ".tiff", StringComparison.OrdinalIgnoreCase);
        });
    }

    private static bool TryExtractImportAssetsWithTar(
        string zipPath,
        ZipArchive archive,
        string importDirectory,
        ZipArchiveEntry? pttEntry,
        ZipArchiveEntry? imageEntry,
        out string? importedImagePath,
        out string? importedPttPath)
    {
        importedImagePath = null;
        importedPttPath = null;

        var entriesToExtract = new List<string>();
        if (imageEntry != null)
        {
            entriesToExtract.Add(imageEntry.FullName);
        }

        if (pttEntry != null)
        {
            entriesToExtract.Add(pttEntry.FullName);
            var pttBaseName = System.IO.Path.GetFileNameWithoutExtension(pttEntry.Name);
            var sourceDirectory = GetZipDirectoryName(pttEntry.FullName);
            entriesToExtract.AddRange(archive.Entries
                .Where(entry =>
                    !string.IsNullOrWhiteSpace(entry.Name) &&
                    string.Equals(GetZipDirectoryName(entry.FullName), sourceDirectory, StringComparison.OrdinalIgnoreCase) &&
                    string.Equals(System.IO.Path.GetFileNameWithoutExtension(entry.Name), pttBaseName, StringComparison.OrdinalIgnoreCase) &&
                    IsPttCompanionExtension(System.IO.Path.GetExtension(entry.Name)))
                .Select(entry => entry.FullName));
        }

        entriesToExtract = entriesToExtract
            .Distinct(StringComparer.OrdinalIgnoreCase)
            .ToList();
        if (entriesToExtract.Count == 0)
        {
            return false;
        }

        var arguments = $"-xf {QuoteProcessArgument(zipPath)} -C {QuoteProcessArgument(importDirectory)} "
            + string.Join(" ", entriesToExtract.Select(QuoteProcessArgument));
        var startInfo = new ProcessStartInfo
        {
            FileName = FindTarPath(),
            Arguments = arguments,
            CreateNoWindow = true,
            UseShellExecute = false,
            RedirectStandardError = true,
            RedirectStandardOutput = true
        };

        using var process = Process.Start(startInfo);
        if (process == null)
        {
            return false;
        }

        var standardError = process.StandardError.ReadToEnd();
        process.StandardOutput.ReadToEnd();
        process.WaitForExit();
        if (process.ExitCode != 0)
        {
            DiagnosticsLog.Write($"tar extraction failed: {standardError}");
            return false;
        }

        if (imageEntry != null)
        {
            var extractedImage = GetExtractedZipEntryPath(importDirectory, imageEntry.FullName);
            if (File.Exists(extractedImage) && new FileInfo(extractedImage).Length > 0)
            {
                importedImagePath = extractedImage;
            }
        }

        if (pttEntry != null)
        {
            var extractedPtt = GetExtractedZipEntryPath(importDirectory, pttEntry.FullName);
            if (File.Exists(extractedPtt) && new FileInfo(extractedPtt).Length > 0)
            {
                importedPttPath = extractedPtt;
            }
        }

        return importedImagePath != null || importedPttPath != null;
    }

    private static bool IsPttCompanionExtension(string extension)
    {
        return string.Equals(extension, ".pot", StringComparison.OrdinalIgnoreCase)
            || string.Equals(extension, ".jpg", StringComparison.OrdinalIgnoreCase)
            || string.Equals(extension, ".jpeg", StringComparison.OrdinalIgnoreCase)
            || string.Equals(extension, ".png", StringComparison.OrdinalIgnoreCase);
    }

    private static string GetExtractedZipEntryPath(string importDirectory, string fullName)
    {
        var parts = fullName.Replace('\\', '/')
            .Split(new[] { '/' }, StringSplitOptions.RemoveEmptyEntries);
        return parts.Aggregate(importDirectory, System.IO.Path.Combine);
    }

    private static string QuoteProcessArgument(string value)
    {
        return "\"" + value.Replace("\"", "\\\"") + "\"";
    }

    private static string FindTarPath()
    {
        var systemTarPath = System.IO.Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.System), "tar.exe");
        return File.Exists(systemTarPath) ? systemTarPath : "tar.exe";
    }

    private bool TryDeserializePartPayload(string json, out PartInspectionData importedPart)
    {
        importedPart = new PartInspectionData();

        using var document = JsonDocument.Parse(json);
        var root = document.RootElement;
        if (root.ValueKind != JsonValueKind.Object)
        {
            return false;
        }

        if (TryGetProperty(root, "Part", out _))
        {
            var model = JsonSerializer.Deserialize<InspectionModel>(json, _importJsonOptions);
            if (model?.Part == null)
            {
                return false;
            }

            importedPart = model.Part;
            importedPart.Name = string.IsNullOrWhiteSpace(importedPart.Name) ? model.ModelName : importedPart.Name;
            return true;
        }

        if (TryGetProperty(root, "Windows", out _))
        {
            var part = JsonSerializer.Deserialize<PartInspectionData>(json, _importJsonOptions);
            if (part == null)
            {
                return false;
            }

            importedPart = part;
            return true;
        }

        return false;
    }


    private static string? ExtractFirstImportImage(ZipArchive archive)
    {
        var imageEntry = archive.Entries.FirstOrDefault(entry =>
            !string.IsNullOrWhiteSpace(entry.Name) &&
            IsImageExtension(System.IO.Path.GetExtension(entry.Name)));
        if (imageEntry == null)
        {
            return null;
        }

        var directory = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "WpfInspectionPartImport");
        Directory.CreateDirectory(directory);
        var path = System.IO.Path.Combine(directory, $"{Guid.NewGuid():N}{System.IO.Path.GetExtension(imageEntry.Name)}");
        imageEntry.ExtractToFile(path, true);
        return path;
    }

    private static string? ExtractFirstImportPtt(ZipArchive archive)
    {
        var pttEntry = archive.Entries.FirstOrDefault(entry =>
            !string.IsNullOrWhiteSpace(entry.Name) &&
            string.Equals(System.IO.Path.GetExtension(entry.Name), ".ptt", StringComparison.OrdinalIgnoreCase));
        if (pttEntry == null)
        {
            return null;
        }

        var directory = System.IO.Path.Combine(System.IO.Path.GetTempPath(), "WpfInspectionPartImport");
        var importDirectory = System.IO.Path.Combine(directory, Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(importDirectory);

        var pttBaseName = System.IO.Path.GetFileNameWithoutExtension(pttEntry.Name);
        var sourceDirectory = GetZipDirectoryName(pttEntry.FullName);
        var path = System.IO.Path.Combine(importDirectory, $"{pttBaseName}.ptt");
        pttEntry.ExtractToFile(path, true);

        ExtractSiblingImportFile(archive, sourceDirectory, pttBaseName, ".pot", importDirectory);
        ExtractSiblingImportFile(archive, sourceDirectory, pttBaseName, ".jpg", importDirectory);
        ExtractSiblingImportFile(archive, sourceDirectory, pttBaseName, ".jpeg", importDirectory);
        ExtractSiblingImportFile(archive, sourceDirectory, pttBaseName, ".png", importDirectory);
        return path;
    }

    private static void ExtractSiblingImportFile(ZipArchive archive, string sourceDirectory, string baseName, string extension, string targetDirectory)
    {
        var sibling = archive.Entries.FirstOrDefault(entry =>
            !string.IsNullOrWhiteSpace(entry.Name) &&
            string.Equals(GetZipDirectoryName(entry.FullName), sourceDirectory, StringComparison.OrdinalIgnoreCase) &&
            string.Equals(System.IO.Path.GetFileNameWithoutExtension(entry.Name), baseName, StringComparison.OrdinalIgnoreCase) &&
            string.Equals(System.IO.Path.GetExtension(entry.Name), extension, StringComparison.OrdinalIgnoreCase));
        if (sibling == null)
        {
            return;
        }

        sibling.ExtractToFile(System.IO.Path.Combine(targetDirectory, sibling.Name), true);
    }

    private static string GetZipDirectoryName(string fullName)
    {
        var normalized = fullName.Replace('\\', '/');
        var index = normalized.LastIndexOf('/');
        return index < 0 ? string.Empty : normalized.Substring(0, index);
    }

    private static int ZipJsonPriority(string name)
    {
        if (name.Contains("part", StringComparison.OrdinalIgnoreCase))
        {
            return 0;
        }

        if (name.Contains("model", StringComparison.OrdinalIgnoreCase) || name.Contains("inspection", StringComparison.OrdinalIgnoreCase))
        {
            return 1;
        }

        return 2;
    }

    private static int ZipXmlPriority(string name)
    {
        if (name.Contains("raw", StringComparison.OrdinalIgnoreCase)
            || name.Contains("part", StringComparison.OrdinalIgnoreCase))
        {
            return 0;
        }

        if (name.Contains("job", StringComparison.OrdinalIgnoreCase)
            || name.Contains("model", StringComparison.OrdinalIgnoreCase))
        {
            return 1;
        }

        return 2;
    }

    private static bool TryGetProperty(JsonElement element, string propertyName, out JsonElement value)
    {
        foreach (var property in element.EnumerateObject())
        {
            if (string.Equals(property.Name, propertyName, StringComparison.OrdinalIgnoreCase))
            {
                value = property.Value;
                return true;
            }
        }

        value = default;
        return false;
    }

    private static bool IsImageExtension(string extension)
    {
        return extension.Equals(".jpg", StringComparison.OrdinalIgnoreCase)
            || extension.Equals(".jpeg", StringComparison.OrdinalIgnoreCase)
            || extension.Equals(".png", StringComparison.OrdinalIgnoreCase)
            || extension.Equals(".bmp", StringComparison.OrdinalIgnoreCase)
            || extension.Equals(".tif", StringComparison.OrdinalIgnoreCase)
            || extension.Equals(".tiff", StringComparison.OrdinalIgnoreCase);
    }

}
