using WpfInspectionApp.Services;
using WpfInspectionApp.Services.FlowAlgorithms;

namespace WpfInspectionApp.Infrastructure;

public sealed class AppServices
{
    public AppServices()
    {
        InspectionWorkflow = new PartInspectionWorkflowService(InspectionEngine, InspectionResultText);
        PartImportWorkflow = new PartImportWorkflowService(PartImport, PartModelApply);
        ModelWorkflow = new ModelWorkflowService(ModelPersistence);
        ThresholdResult = new ThresholdResultService(InspectionResultText);
        ImageRuntimeState = new ImageRuntimeStateService(ImageFrame);
        ImageLoadWorkflow = new ImageLoadWorkflowService(ImageRuntimeState);
        PttLightPreview = new PttLightPreviewService(ImageFrame);
        ThresholdPreviewWorkflow = new ThresholdPreviewWorkflowService(PreviewProcessing, ImageRuntimeState, ThresholdResult);
        PttViewerWorkflow = new PttViewerWorkflowService(PttLoad);
        RoiInteraction = new RoiInteractionService(RoiGeometry);
        RoiUiState = new RoiUiStateService(RoiModel);

        // Plugin registry — each IFlowAlgorithm describes one algo end-to-end.
        // To add a new algorithm: create a new class implementing IFlowAlgorithm and
        // register it here. The UI auto-renders a Run button + result line for it.
        FlowAlgorithms = new FlowAlgorithmRegistry();
        FlowAlgorithms.Register(new AlignFlowAlgorithm());
        FlowAlgorithms.Register(new PadBWFlowAlgorithm());
        FlowAlgorithms.Register(new BlobFlowAlgorithm());
        FlowAlgorithms.Register(new BGAFlowAlgorithm());
        FlowAlgorithms.Register(new EdgeFlowAlgorithm());
        FlowAlgorithms.Register(new PatternFlowAlgorithm(FileDialog));
        FlowAlgorithms.Register(new ShapeXFlowAlgorithm());
    }

    public IModelPersistenceService ModelPersistence { get; } = new JsonModelPersistenceService();

    public IApplicationPathService ApplicationPath { get; } = new ApplicationPathService();

    public IFileDialogService FileDialog { get; } = new FileDialogService();

    public IPartImportService PartImport { get; } = new PartImportService();

    public IPartModelApplyService PartModelApply { get; } = new PartModelApplyService();

    public IInspectionEngineService InspectionEngine { get; } = new PartInspectionEngineService();

    public IInspectionWorkflowService InspectionWorkflow { get; }

    public IPartImportWorkflowService PartImportWorkflow { get; }

    public IModelWorkflowService ModelWorkflow { get; }

    public IPreviewProcessingService PreviewProcessing { get; } = new PreviewProcessingService();

    public IImageFrameService ImageFrame { get; } = new ImageFrameService();

    public IImageRuntimeStateService ImageRuntimeState { get; }

    public IImageLoadWorkflowService ImageLoadWorkflow { get; }

    public IPttLightPreviewService PttLightPreview { get; }

    public IRoiGeometryService RoiGeometry { get; } = new RoiGeometryService();

    public IRoiModelService RoiModel { get; } = new RoiModelService();

    public IRoiInteractionService RoiInteraction { get; }

    public IRoiUiStateService RoiUiState { get; }

    public IPttLoadService PttLoad { get; } = new PttLoadService();

    public IPttViewerWorkflowService PttViewerWorkflow { get; }

    public IAlignPartTeachingService AlignPartTeaching { get; } = new AlignPartTeachingService();

    public IAlignConditionService AlignCondition { get; } = new AlignConditionService();

    public IAlignFlowRequestFactory AlignFlowRequestFactory { get; } = new AlignFlowRequestFactory();

    public IAlgorithmLightService AlgorithmLight { get; } = new AlgorithmLightService();

    public IInspectionResultTextService InspectionResultText { get; } = new InspectionResultTextService();

    public IThresholdResultService ThresholdResult { get; }

    public IThresholdPreviewWorkflowService ThresholdPreviewWorkflow { get; }

    // Bridge-flow runner for MPTI_SetInspParam -> MPTI_InspProc -> MPTI_GetInspectionResult.
    // Separate from the single-shot InspectionWorkflow which targets per-algo bridges.
    public IInspectionFlowService InspectionFlow { get; } = new InspectionFlowService();

    // Plugin-style algorithm registry. MainViewModel reads this and exposes one
    // FlowAlgorithmRunner per registered IFlowAlgorithm; XAML ItemsControl renders
    // them as Run buttons + result lines without hardcoding per-algorithm UI.
    public FlowAlgorithmRegistry FlowAlgorithms { get; }
}
