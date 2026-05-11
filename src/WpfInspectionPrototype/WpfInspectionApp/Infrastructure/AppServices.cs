using WpfInspectionApp.Services;

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
        ThresholdPreviewWorkflow = new ThresholdPreviewWorkflowService(PreviewProcessing, ImageRuntimeState, ThresholdResult);
        RoiInteraction = new RoiInteractionService(RoiGeometry);
        RoiUiState = new RoiUiStateService(RoiModel);
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

    public IRoiGeometryService RoiGeometry { get; } = new RoiGeometryService();

    public IRoiModelService RoiModel { get; } = new RoiModelService();

    public RoiInteractionService RoiInteraction { get; }

    public IRoiUiStateService RoiUiState { get; }

    public IPem3DViewerHostService Pem3DViewerHost { get; } = new Pem3DViewerHostService();

    public IPttLoadService PttLoad { get; } = new PttLoadService();

    public IAlignPartTeachingService AlignPartTeaching { get; } = new AlignPartTeachingService();

    public IAlignConditionService AlignCondition { get; } = new AlignConditionService();

    public IInspectionResultTextService InspectionResultText { get; } = new InspectionResultTextService();

    public IThresholdResultService ThresholdResult { get; }

    public IThresholdPreviewWorkflowService ThresholdPreviewWorkflow { get; }
}
