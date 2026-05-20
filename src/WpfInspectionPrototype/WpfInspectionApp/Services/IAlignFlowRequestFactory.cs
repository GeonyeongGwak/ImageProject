using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public interface IAlignFlowRequestFactory
{
    AlignFlowRequest Create(InspectionModel model, string pttPath, int sourceWidth, int sourceHeight);
}
