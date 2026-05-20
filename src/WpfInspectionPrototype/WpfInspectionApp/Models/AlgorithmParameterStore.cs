namespace WpfInspectionApp.Models;

public static class AlgorithmParameterStore
{
    public static Dictionary<string, string> CloneCaseInsensitive(IDictionary<string, string>? source)
    {
        var copy = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
        if (source == null)
        {
            return copy;
        }

        foreach (var pair in source)
        {
            if (string.IsNullOrWhiteSpace(pair.Key) || copy.ContainsKey(pair.Key))
            {
                continue;
            }

            copy[pair.Key] = pair.Value;
        }

        return copy;
    }

    public static bool ContainsKey(IDictionary<string, string>? parameters, string key)
    {
        return FindKey(parameters, key) != null;
    }

    public static bool TryGetValue(IDictionary<string, string>? parameters, string key, out string value)
    {
        var existingKey = FindKey(parameters, key);
        if (existingKey != null && parameters!.TryGetValue(existingKey, out value!))
        {
            return true;
        }

        value = "";
        return false;
    }

    public static string GetValue(IDictionary<string, string>? parameters, string key, string fallback = "")
    {
        return TryGetValue(parameters, key, out var value) ? value : fallback;
    }

    public static void Set(IDictionary<string, string> parameters, string key, string value)
    {
        var existingKey = FindKey(parameters, key);
        if (existingKey != null)
        {
            parameters[existingKey] = value;
            return;
        }

        parameters[key] = value;
    }

    public static void SetDefault(IDictionary<string, string> parameters, string key, string value)
    {
        if (!ContainsKey(parameters, key))
        {
            parameters[key] = value;
        }
    }

    private static string? FindKey(IDictionary<string, string>? parameters, string key)
    {
        if (parameters == null || string.IsNullOrWhiteSpace(key))
        {
            return null;
        }

        if (parameters.ContainsKey(key))
        {
            return key;
        }

        return parameters.Keys.FirstOrDefault(candidate => string.Equals(candidate, key, StringComparison.OrdinalIgnoreCase));
    }
}
