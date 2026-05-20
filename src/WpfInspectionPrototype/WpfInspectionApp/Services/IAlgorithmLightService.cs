using WpfInspectionApp.Models;

namespace WpfInspectionApp.Services;

public interface IAlgorithmLightService
{
    int MaximumChannelValue { get; }
    int MaximumUserCellCount { get; }
    AlgorithmLightState ReadState(InspectionAlgorithmData? algorithm);
    void SaveState(InspectionAlgorithmData algorithm, AlgorithmLightState state);
    AlgorithmLightState CreateStateForLightType(int lightType, AlgorithmLightState? currentState = null);
    AlgorithmLightCell CreateDefaultUserCell(int position = 0, int operatorType = 0);
    LightChannelAvailability GetChannelAvailability(int lightTypeOrPosition, bool userCellPosition);
}
