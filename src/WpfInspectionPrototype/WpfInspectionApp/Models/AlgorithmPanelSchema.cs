namespace WpfInspectionApp.Models;

public static class AlgorithmPanelSchema
{
    public static AlgorithmPanelData Create(InspectionAlgorithmData algorithm)
    {
        var catalog = AlgorithmCatalog.Find(algorithm.Type);
        return catalog.Type switch
        {
            "AlgoAlign" => CreateDedicated(catalog, "Dedicated", ["Search", "BW Condition", "Inspection", "IPC"]),
            "AlgoAlignEdge" => CreateDedicated(catalog, "Dedicated", ["Edge", "Range", "Measure", "Anchor", "Serialize"]),
            _ => CreateDynamic(catalog)
        };
    }

    private static AlgorithmPanelData CreateDynamic(AlgorithmCatalogItem catalog)
    {
        var family = catalog.ParameterFamily;
        return new AlgorithmPanelData
        {
            AlgorithmType = catalog.Type,
            PanelKind = "Dynamic",
            ActiveTab = "Common",
            Tabs =
            [
                Tab("Common", "Common", ["Common.bAlgoEnable", "Common.IsRequired", "Common.bAlgoGroup"]),
                Tab("Condition", "Condition", [$"{family}.Use2D", $"{family}.Use3D", $"{family}.Threshold", $"{family}.Tolerance"]),
                Tab("ROI", "ROI / Mask", ["ROI.UseAlgorithmRoi", "Mask.UseMask", "Mask.UsePolygon", "Mask.ExceptRoiCount"]),
                Tab("Advanced", "Advanced", ["Common.ManualDefectCode", "Common.ManualSubDefectCode", "Command.ApplyAllTarget", "Command.TeachRequested"])
            ]
        };
    }

    private static AlgorithmPanelData CreateDedicated(AlgorithmCatalogItem catalog, string kind, string[] tabs)
    {
        return new AlgorithmPanelData
        {
            AlgorithmType = catalog.Type,
            PanelKind = kind,
            ActiveTab = tabs.FirstOrDefault() ?? "Common",
            Tabs = tabs.Select(tab => Tab(tab, tab, [])).ToList()
        };
    }

    private static AlgorithmPanelTabData Tab(string id, string title, IEnumerable<string> keys)
    {
        return new AlgorithmPanelTabData
        {
            Id = id,
            Title = title,
            ParameterKeys = keys.ToList()
        };
    }
}
