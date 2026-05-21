using System.IO;
using WpfInspectionApp.Infrastructure;
using WpfInspectionApp.Interop;
using WpfInspectionApp.Models;
using WpfInspectionApp.Services;
using WpfInspectionApp.Services.FlowAlgorithms;
using WpfInspectionApp.ViewModels;

namespace WpfInspectionApp.Diagnostics;

// Headless smoke test that exercises Align/ShapeX/PadBW native bridges using
// the default 2D image and synthetic Window/Algorithm data. Used by the
// `--smoke-test` CLI argument and CI to detect regressions without UI.
internal static class SmokeTestRunner
{
    public static int Run(AppServices services)
    {
        var log = new System.Text.StringBuilder();
        void Write(string message)
        {
            DiagnosticsLog.Write($"[SMOKE] {message}");
            log.AppendLine(message);
        }

        try
        {
            Write("smoke test starting");

            // Use a small synthetic BGRA image with a clear checkerboard pattern.
            // This avoids the heavy 15.8M pixel default JPG and isolates bridge correctness.
            const int width = 256;
            const int height = 256;
            const int stride = width * 4;
            var pixels = CreateSyntheticImage(width, height);
            Write($"synthetic image {width}x{height} prepared");

            var model = CreateSyntheticModel(width, height);
            Write("model built");
            var runtimeImage = new PartRuntimeImage(pixels, width, height, stride, model.Threshold2D);
            Write("runtime image built");

            // Call each bridge directly to isolate any hang
            Write("calling MptiNativeBridge.GetVersion (sanity check DLL load)...");
            var verSw = System.Diagnostics.Stopwatch.StartNew();
            var verResp = Interop.MptiNativeBridge.GetVersion();
            verSw.Stop();
            Write($"GetVersion ({verSw.ElapsedMilliseconds}ms): available={verResp.Available} success={verResp.Success} message={verResp.Message}");

            Write("calling MptiAlignNativeBridge.Run directly (sync)...");
            var alignSw = System.Diagnostics.Stopwatch.StartNew();
            var alignParams = new Interop.MptiBridgeAlignParams
            {
                SearchNum = 4,
                SearchPointsX = new[] { 64, 192, 64, 192 },
                SearchPointsY = new[] { 64, 64, 192, 192 },
                SearchSizeW = new[] { 40, 40, 40, 40 },
                SearchSizeH = new[] { 40, 40, 40, 40 },
                SearchMargin = 5,
                MinBinary = 128,
                MaxBinary = 255,
                UseInsp2D = 1,
                InvertCheck = 0,
                UseShift = 1,
                MaxShiftX = 10,
                MaxShiftY = 10,
                UseAngle = 1,
                MaxAngle = 5,
                SameSize = 1,
                MinBlobArea = 5
            };
            var alignResp = Interop.MptiAlignNativeBridge.Run(pixels, width, height, stride, new RoiRect(0, 0, width, height), alignParams);
            alignSw.Stop();
            Write($"Align bridge ({alignSw.ElapsedMilliseconds}ms): available={alignResp.Available} success={alignResp.Success} okCount={alignResp.OkCount} message={alignResp.Message}");

            Write("calling MptiAlgorithmNativeBridge.RunShapeX...");
            var shapeXParams = new Interop.MptiBridgeShapeXParams
            {
                BinaryMin = 128, BinaryMax = 255, UseInsp2D = 1, UseShape = 1, UseExist = 1, UseShift = 1,
                ShapeAreaMin = 0.05f, ShapeAreaMax = 0.95f, ShiftXTolerance = 10, ShiftYTolerance = 10,
                ExpectedCenterX = 128, ExpectedCenterY = 128, MinBlobArea = 5
            };
            var shapeXSw = System.Diagnostics.Stopwatch.StartNew();
            var shapeXResp = Interop.MptiAlgorithmNativeBridge.RunShapeX(pixels, width, height, stride, new RoiRect(50, 50, 156, 156), shapeXParams);
            shapeXSw.Stop();
            Write($"ShapeX bridge ({shapeXSw.ElapsedMilliseconds}ms): available={shapeXResp.Available} success={shapeXResp.Success} isOK={shapeXResp.IsOK} areaRate={shapeXResp.AreaRate:F3} message={shapeXResp.Message}");

            Write("calling MptiAlgorithmNativeBridge.RunPadBW...");
            var padBWParams = new Interop.MptiBridgePadBWParams
            {
                BinaryMin = 128, BinaryMax = 255, UseInsp2D = 1, UseTeachArea = 1,
                TeachArea = 1000, TeachAreaRateMin = 80, TeachAreaRateMax = 120,
                UseShift = 1, TeachShiftX = 10, TeachShiftY = 10,
                ExpectedCenterX = 128, ExpectedCenterY = 128, UseBlobArea = 1, BlobAreaMin = 50,
                MinBlobArea = 5
            };
            var padBWSw = System.Diagnostics.Stopwatch.StartNew();
            var padBWResp = Interop.MptiAlgorithmNativeBridge.RunPadBW(pixels, width, height, stride, new RoiRect(50, 50, 156, 156), padBWParams);
            padBWSw.Stop();
            Write($"PadBW bridge ({padBWSw.ElapsedMilliseconds}ms): available={padBWResp.Available} success={padBWResp.Success} isOK={padBWResp.IsOK} areaRate={padBWResp.AreaRate:F1}% message={padBWResp.Message}");

            Write("calling MptiAlgorithmNativeBridge.RunEdge...");
            var edgeParams = new Interop.MptiBridgeEdgeParams
            {
                BinaryMin = 128, BinaryMax = 255, UseInsp2D = 1, MinBlobArea = 5,
                UseArea = 0, AreaMin = 1, AreaMax = 1_000_000,
                UseShift = 0, ShiftX = 10, ShiftY = 10,
                UseAngle = 0, TeachRotate = 0, AngleTolerance = 5,
                ExpectedCenterX = 128, ExpectedCenterY = 128,
                SetLineCnt = 1, LineFindRate = 100
            };
            var edgeSw = System.Diagnostics.Stopwatch.StartNew();
            var edgeResp = Interop.MptiAlgorithmNativeBridge.RunEdge(pixels, width, height, stride, new RoiRect(50, 50, 156, 156), edgeParams);
            edgeSw.Stop();
            Write($"Edge bridge ({edgeSw.ElapsedMilliseconds}ms): available={edgeResp.Available} success={edgeResp.Success} isOK={edgeResp.IsOK} angle={edgeResp.Theta:F3} message={edgeResp.Message}");

            Write("calling MptiAlgorithmNativeBridge.RunPattern...");
            var patternParams = new Interop.MptiBridgePatternParams
            {
                BinaryMin = 128, BinaryMax = 255, UseInsp2D = 1, MinPatternArea = 5,
                AcceptScore = 0.8, UseShift = 0, ShiftX = 10, ShiftY = 10,
                ExpectedCenterX = 128, ExpectedCenterY = 128,
                RangeAngle = 10, WndAngle = 0, SearchAngleRangeMin = -10, SearchAngleRangeMax = 10,
                SamplingAngle = 1, CntPatternPath = 1,
                FactorRed = 1, FactorGreen = 1, FactorBlue = 1,
                ModelPathInspect1 = string.Empty,
                ModelPathTeach = string.Empty
            };
            var patternSw = System.Diagnostics.Stopwatch.StartNew();
            var patternResp = Interop.MptiAlgorithmNativeBridge.RunPattern(pixels, width, height, stride, new RoiRect(50, 50, 156, 156), patternParams);
            patternSw.Stop();
            Write($"Pattern bridge ({patternSw.ElapsedMilliseconds}ms): available={patternResp.Available} success={patternResp.Success} isOK={patternResp.IsOK} score={patternResp.AreaRate:F3} message={patternResp.Message}");

            Write("calling MptiAlgorithmNativeBridge.RunBGA...");
            var bgaParams = new Interop.MptiBridgeBGAParams
            {
                BinaryMin = 128, BinaryMax = 255, UseInsp2D = 1, MinBlobArea = 5,
                UseArea = 1, AreaMin = 10, AreaMax = 1_000_000,
                TeachArea = 100, TeachVolume = 0,
                UseShift = 0, ShiftX = 10, ShiftY = 10,
                ExpectedCenterX = 128, ExpectedCenterY = 128,
                UseCircleRate = 0, TeachCircleRate = 80,
                UseCoplanarity = 0, CoplanarityMin = 0, CoplanarityMax = 100
            };
            var bgaSw = System.Diagnostics.Stopwatch.StartNew();
            var bgaResp = Interop.MptiAlgorithmNativeBridge.RunBGA(pixels, width, height, stride, new RoiRect(50, 50, 156, 156), bgaParams);
            bgaSw.Stop();
            Write($"BGA bridge ({bgaSw.ElapsedMilliseconds}ms): available={bgaResp.Available} success={bgaResp.Success} isOK={bgaResp.IsOK} areaRatio={bgaResp.AreaRate:F3} message={bgaResp.Message}");

            RunServiceRegressionChecks(services, Write);

            var allAvailable = alignResp.Available && shapeXResp.Available && padBWResp.Available && edgeResp.Available && patternResp.Available && bgaResp.Available;
            var allSuccess = alignResp.Success && shapeXResp.Success && padBWResp.Success && edgeResp.Success && patternResp.Success && bgaResp.Success;
            Write(allAvailable && allSuccess ? "PASS: all bridges native + success" : $"PARTIAL: available={allAvailable} success={allSuccess}");
            return allAvailable && allSuccess ? 0 : 1;
        }
        catch (Exception ex)
        {
            DiagnosticsLog.Write($"[SMOKE] EXCEPTION {ex}");
            return 5;
        }
    }

    private static void RunServiceRegressionChecks(AppServices services, Action<string> write)
    {
        write("running service regression checks...");
        VerifyPartRuntimeHandlesCaseDuplicateKeys();
        VerifyPartRuntimeRoutesBlobBridge();
        VerifyPartRuntimeRoutesBGABridge();
        VerifyPartRuntimeRoutesEdgeBridge();
        VerifyPartRuntimeRoutesPatternBridge();
        // 2026-05-21 handoff (exact-flow-algorithm-types) 에서 추가된 새 algorithm type 들의
        // per-algo bridge fallback 회귀 커버. flow-first path (Runtime.FlowBridge=native-flow)
        // 는 PTT 파일이 필요하므로 별도 GUI 검증이 필요하며, 여기서는 fallback 라우팅이
        // 망가지지 않았음만 보장한다.
        VerifyPartRuntimeRoutesNgBlobToBlobBridge();
        VerifyPartRuntimeRoutesBodyEdgeToEdgeBridge();
        VerifyPartRuntimeRoutesPatternDiffToPatternBridge();
        VerifyPartRuntimeRoutesOcrToPatternBridge();
        VerifyPartRuntimeRoutesPocrToPatternBridge();
        // 2026-05-22 handoff (native-family-dispatch / bridge-adapter-family-driven) 의
        // family lookup 회귀 잠금. AlgorithmCatalog 의 NativeAlgoType 과 native
        // MptiBridgeGetAlgoFamily 의 매핑이 의도된 family 로 떨어지는지 확인.
        VerifyAlgoFamilyLookupMatchesCatalog();
        VerifyFlowAlgorithmExecutionServiceRejectsMissingPtt();
        VerifyLightServiceRoundTrip(services.AlgorithmLight);
        VerifyNormalLightViewModelQuickPreset(services.AlgorithmLight);
        VerifyUserLightViewModelReferenceFlow(services.AlgorithmLight);
        VerifyLegacyLightImportParsing();
        VerifyLegacyCsvAlgorithmRoiParsing();
        VerifyAlignSearchSizeDoesNotResizeWindow(services);
        VerifyAlignNextRoiUsesSearchSlot(services);
        VerifyAlignDeleteClearsSearchSlot(services);
        write("service regression checks passed");
    }

    private static void VerifyPartRuntimeHandlesCaseDuplicateKeys()
    {
        var model = CreateSyntheticModel(128, 128);
        var algorithm = model.Part.Windows[0].Algorithms[0];
        algorithm.ApplyCatalogDefaults();
        algorithm.Parameters["Common.bAlgoEnable"] = "true";
        algorithm.Parameters["common.balgoenable"] = "false";
        algorithm.Parameters["Align.Threshold"] = "128";
        algorithm.Parameters["align.threshold"] = "64";

        var runtime = new PartInspectionRuntime();
        var result = runtime.Run(model, image: null);
        Assert(result.TotalCount == 3, "Part runtime should keep all synthetic algorithms.");

        var runtimeAlgorithm = result.Packet.Windows[0].Algorithms[0];
        Assert(CountKeys(runtimeAlgorithm.Parameters, "Common.bAlgoEnable") == 1, "Runtime packet should collapse Common.bAlgoEnable duplicates.");
        Assert(CountKeys(runtimeAlgorithm.Parameters, "Align.Threshold") == 1, "Runtime packet should collapse Align.Threshold duplicates.");
        Assert(CountKeys(algorithm.Parameters, "Runtime.LastRun") == 1, "Runtime source parameters should be written without duplicate Runtime.LastRun keys.");
    }

    private static void VerifyLightServiceRoundTrip(IAlgorithmLightService service)
    {
        var algorithm = new InspectionAlgorithmData { Type = "AlgoShapeX", DisplayName = "Light RoundTrip" };
        algorithm.ApplyCatalogDefaults();

        service.SaveState(algorithm, new AlgorithmLightState
        {
            LightType = AlgorithmLightService.UserLight,
            UserCells =
            [
                new AlgorithmLightCell { Position = AlgorithmLightService.TopLight, Operator = 1, RedValue = 120, GreenValue = 30, BlueValue = 20, WhiteValue = 10 },
                new AlgorithmLightCell { Position = AlgorithmLightService.BottomLight, Operator = 2, RedValue = 80, GreenValue = -1, BlueValue = 40, WhiteValue = -1 }
            ]
        });

        var read = service.ReadState(algorithm);
        Assert(read.LightType == AlgorithmLightService.UserLight, "Light service should preserve User Light mode.");
        Assert(read.UserCells.Count == 2, "Light service should preserve user light cell count.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.ArrRedValueString") == "120|80", "Light service should store red user-light array.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.ArrCalculationString") == "1|2", "Light service should store user-light operators.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.ArrLightPositionString") == "0|2", "Light service should store user-light positions.");
    }

    private static void VerifyNormalLightViewModelQuickPreset(IAlgorithmLightService service)
    {
        var algorithm = new InspectionAlgorithmData { Type = "AlgoShapeX", DisplayName = "Normal Light ViewModel" };
        algorithm.ApplyCatalogDefaults();
        service.SaveState(algorithm, new AlgorithmLightState
        {
            LightType = AlgorithmLightService.TopLight,
            RedValue = 100,
            GreenValue = 0,
            BlueValue = 0,
            WhiteValue = 0
        });

        var observedPreviewStates = new List<AlgorithmLightState>();
        var viewModel = new LightControlViewModel(service, () => observedPreviewStates.Add(service.ReadState(algorithm)));
        viewModel.Load(algorithm);

        var bluePreset = viewModel.NormalChannels[2].PresetCommand;
        Assert(bluePreset != null, "Normal Light channel should expose a quick preset command.");
        bluePreset!.Execute(null);

        Assert(viewModel.NormalChannels[0].Value == 0, "Normal Light preset should clear red.");
        Assert(viewModel.NormalChannels[1].Value == 0, "Normal Light preset should clear green.");
        Assert(viewModel.NormalChannels[2].Value == 100, "Normal Light preset should set blue to 100.");
        Assert(viewModel.NormalChannels[3].Value == 0, "Normal Light preset should clear white.");
        Assert(observedPreviewStates.Count == 1, "Normal Light preset should save and preview once after all channel values are applied.");
        Assert(observedPreviewStates[0].RedValue == 0 && observedPreviewStates[0].BlueValue == 100, "Normal Light preview should not restore red while applying a blue quick preset.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.RedValue") == "0", "Normal Light preset should persist red as zero.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.BlueValue") == "100", "Normal Light preset should persist blue as 100.");
    }

    private static void VerifyUserLightViewModelReferenceFlow(IAlgorithmLightService service)
    {
        var algorithm = new InspectionAlgorithmData { Type = "AlgoShapeX", DisplayName = "Light ViewModel" };
        algorithm.ApplyCatalogDefaults();
        service.SaveState(algorithm, new AlgorithmLightState
        {
            LightType = AlgorithmLightService.UserLight,
            UserCells =
            [
                new AlgorithmLightCell { Position = AlgorithmLightService.TopLight, Operator = 0, RedValue = 100, GreenValue = 0, BlueValue = 0, WhiteValue = 0 }
            ]
        });

        var previewRequests = 0;
        var viewModel = new LightControlViewModel(service, () => previewRequests++);
        viewModel.Load(algorithm);

        Assert(viewModel.UserCells.Count == 1, "User Light view model should load one cell.");
        var first = viewModel.UserCells[0];
        viewModel.ToggleUserCellOperatorCommand.Execute(first);
        Assert(viewModel.UserCells.Count == 2, "Last blank operator should append a new User Light cell.");
        Assert(first.OperatorType == 1, "First operator should become Add.");
        Assert(viewModel.SelectedUserCell == viewModel.UserCells[1], "Newly appended User Light cell should be selected.");

        viewModel.ToggleUserCellOperatorCommand.Execute(first);
        Assert(viewModel.UserCells.Count == 2, "Add operator should cycle to Sub without changing cell count.");
        Assert(first.OperatorType == 2, "First operator should become Sub.");

        viewModel.ToggleUserCellOperatorCommand.Execute(first);
        Assert(viewModel.UserCells.Count == 1, "Second-last operator returning to blank should remove the trailing cell.");
        Assert(first.OperatorType == 0, "First operator should return to None.");

        var previewRequestsBeforePreset = previewRequests;
        first.Channels[2].PresetCommand?.Execute(null);
        Assert(first.Channels[0].Value == 0 && first.Channels[2].Value == 100, "Channel preset should solo the selected channel.");
        Assert(previewRequests == previewRequestsBeforePreset + 1, "User Light preset should save and preview once after all channel values are applied.");

        viewModel.ToggleUserPreviewModeCommand.Execute(null);
        var previewState = viewModel.CreatePreviewState(service.ReadState(algorithm));
        Assert(!viewModel.IsUserMixPreview, "Preview mode should toggle to selected-cell preview.");
        Assert(previewState.UserCells.Count == 1, "Selected-cell preview should render only one User Light cell.");
        Assert(previewRequests > 0, "User Light view model should request preview refreshes.");
    }

    private static void VerifyLegacyLightImportParsing()
    {
        const string xml = """
            <RawDataContainer>
              <PartData>
                <Name>SmokeLightPart</Name>
              </PartData>
              <WindowDataList>
                <WindowData>
                  <ID>W1</ID>
                  <Name>Main</Name>
                  <RelRoi>
                    <cx>64</cx>
                    <cy>64</cy>
                    <w>100</w>
                    <h>100</h>
                  </RelRoi>
                  <AlgorithmDataList>
                    <AlgorithmData>
                      <ID>A1</ID>
                      <Type>Align</Type>
                      <LightType>User</LightType>
                      <RedValue>35</RedValue>
                      <GreenValue>25</GreenValue>
                      <BlueValue>15</BlueValue>
                      <WhiteValue>5</WhiteValue>
                      <LightCnt>2</LightCnt>
                      <ArrRedValue>
                        <Value>120</Value>
                        <Value>80</Value>
                      </ArrRedValue>
                      <ArrGreenValue>30, -1</ArrGreenValue>
                      <ArrBlueValue>20;40</ArrBlueValue>
                      <ArrWhiteValue>10|-1</ArrWhiteValue>
                      <ArrCalculation>1|2</ArrCalculation>
                      <ArrLightPosition>0|2</ArrLightPosition>
                    </AlgorithmData>
                  </AlgorithmDataList>
                </WindowData>
              </WindowDataList>
            </RawDataContainer>
            """;

        var parsed = LegacyRawPartImportAdapter.TryParse(xml, out var part, out var status);
        Assert(parsed, $"Legacy light XML should parse. Status: {status}");
        var algorithm = part.Windows.Single().Algorithms.Single();
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.LightTypeNum") == "3", "Import should normalize User Light type.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.RedValue") == "35", "Import should parse normal red value.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.LightCnt") == "2", "Import should parse light count.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.ArrRedValueString") == "120|80", "Import should parse light array container.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.ArrGreenValueString") == "30|-1", "Import should parse comma-separated light array.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.ArrBlueValueString") == "20|40", "Import should parse semicolon-separated light array.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Common.ArrWhiteValueString") == "10|-1", "Import should parse pipe-separated light array.");

        var previewRequests = 0;
        var lightViewModel = new LightControlViewModel(new AlgorithmLightService(), () => previewRequests++);
        lightViewModel.Load(algorithm);

        Assert(lightViewModel.LightType == AlgorithmLightService.UserLight, "Imported light type should show as User Light in the Teaching UI.");
        Assert(lightViewModel.IsUserMode, "Teaching Light UI should switch to User Light mode for imported User data.");
        Assert(lightViewModel.UserCells.Count == 2, "Teaching Light UI should show imported User Light cells.");

        var topCell = lightViewModel.UserCells[0];
        Assert(topCell.Position == AlgorithmLightService.TopLight, "First imported User Light cell should keep Top position.");
        Assert(topCell.OperatorType == 1, "First imported User Light cell should keep Add operator.");
        Assert(topCell.Channels[0].Value == 120, "First imported User Light cell should show red value.");
        Assert(topCell.Channels[1].Value == 30, "First imported User Light cell should show green value.");
        Assert(topCell.Channels[2].Value == 20, "First imported User Light cell should show blue value.");
        Assert(topCell.Channels[3].Value == 10, "First imported User Light cell should show white value.");

        var bottomCell = lightViewModel.UserCells[1];
        Assert(bottomCell.Position == AlgorithmLightService.BottomLight, "Second imported User Light cell should keep Bottom position.");
        Assert(bottomCell.OperatorType == 2, "Second imported User Light cell should keep Sub operator.");
        Assert(bottomCell.Channels[0].Value == 80, "Second imported User Light cell should show red value.");
        Assert(!bottomCell.Channels[1].IsEditable && bottomCell.Channels[1].Value == 0, "Bottom User Light should hide imported disabled green value.");
        Assert(bottomCell.Channels[2].Value == 40, "Second imported User Light cell should show blue value.");
        Assert(!bottomCell.Channels[3].IsEditable && bottomCell.Channels[3].Value == 0, "Bottom User Light should hide imported disabled white value.");
        Assert(previewRequests == 0, "Loading imported light should not request a preview before user edits.");
    }

    private static void VerifyLegacyCsvAlgorithmRoiParsing()
    {
        const string xml = """
            <RawDataContainer>
              <PixelResolutionX>0.01</PixelResolutionX>
              <PixelResolutionY>0.01</PixelResolutionY>
              <ImageWidth>1000</ImageWidth>
              <ImageHeight>800</ImageHeight>
              <PartData>
                <Name>SmokeCsvRoiPart</Name>
              </PartData>
              <WindowDataList>
                <WindowData>
                  <ID>W1</ID>
                  <Name>Main</Name>
                  <RelRoi>
                    <cx>500</cx>
                    <cy>400</cy>
                    <w>600</w>
                    <h>500</h>
                  </RelRoi>
                  <AlgorithmDataList>
                    <AlgorithmData>
                      <ID>A1</ID>
                      <Type>Height_Diff</Type>
                      <ROI1_mm>-1.5,-2.0,1.5,2.0</ROI1_mm>
                      <ROI1>-150,-200,150,200</ROI1>
                      <BROI2>
                        <BlobBN>1,2,3</BlobBN>
                        <BlobBF>4,5,6</BlobBF>
                      </BROI2>
                    </AlgorithmData>
                  </AlgorithmDataList>
                </WindowData>
              </WindowDataList>
            </RawDataContainer>
            """;

        var parsed = LegacyRawPartImportAdapter.TryParse(xml, out var part, out var status);
        Assert(parsed, $"Legacy CSV ROI XML should parse. Status: {status}");

        var algorithm = part.Windows.Single().Algorithms.Single();
        Assert(algorithm.AlgorithmRoi.HasValue, "CSV ROI import should create an Algorithm ROI.");
        var roi = algorithm.AlgorithmRoi!.Value;
        Assert(roi.X == 350 && roi.Y == 200, $"CSV ROI should be centered by imported image origin, got X={roi.X} Y={roi.Y}.");
        Assert(roi.Width == 300 && roi.Height == 400, $"CSV ROI should use ROI1_mm size, got W={roi.Width} H={roi.Height}.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Heightdiff.Roi1.Mm") == "-1.5,-2.0,1.5,2.0", "CSV ROI raw mm value should be preserved.");
    }

    private static void VerifyAlignSearchSizeDoesNotResizeWindow(AppServices services)
    {
        var model = CreateSyntheticModel(200, 160);
        model.Part.SourceWidth = 200;
        model.Part.SourceHeight = 160;
        model.AlignSearchNum = 2;
        model.AlignActiveRoiIndex = 0;
        model.AlignSameSize = false;
        model.AlignSearchMargin = 5;
        model.AlignSearchSizeX = 80;
        model.AlignSearchSizeY = 30;
        model.AlignSearchRois[0] = new RoiRect(40, 40, 20, 20);

        var activeWindow = model.Part.Windows[0];
        var originalWindowRoi = activeWindow.Roi;
        var viewModel = CreateSmokeMainViewModel(model, services);
        var changed = viewModel.ResizeActiveSearchRoiFromSearchInputs(200, 160, FormatSmokeRoi);

        Assert(changed, "Search Size edit should resize the active Align Search ROI.");
        Assert(activeWindow.Roi.Equals(originalWindowRoi), "Search Size edit should not resize the selected Window ROI.");
        Assert(model.AlignSearchRois[0] is { IsValid: true }, "Search Size edit should keep active Align Search ROI valid.");

        var roi = model.AlignSearchRois[0]!.Value;
        Assert(roi.Width == 80 && roi.Height == 30, $"Search ROI should adopt input size, got W={roi.Width} H={roi.Height}.");
        Assert(roi.X == 10 && roi.Y == 35, $"Search ROI should keep its center while resizing, got X={roi.X} Y={roi.Y}.");

        var align = activeWindow.Algorithms.First(algorithm => string.Equals(algorithm.Type, "AlgoAlign", StringComparison.OrdinalIgnoreCase));
        Assert(AlgorithmParameterStore.GetValue(align.Parameters, "Align.SearchSize1.W") == "80", "Search Size edit should persist Align.SearchSize1.W.");
        Assert(AlgorithmParameterStore.GetValue(align.Parameters, "Align.SearchSize1.H") == "30", "Search Size edit should persist Align.SearchSize1.H.");
        Assert(AlgorithmParameterStore.GetValue(align.Parameters, "Align.SearchSize2.W") != "80", "Search ROI slot 2 should not overwrite slot 1 parameters.");
    }

    private static void VerifyAlignNextRoiUsesSearchSlot(AppServices services)
    {
        var model = CreateSyntheticModel(200, 160);
        model.Part.SourceWidth = 200;
        model.Part.SourceHeight = 160;
        model.AlignSearchNum = 3;
        model.AlignActiveRoiIndex = 0;
        model.AlignSearchSizeX = 77;
        model.AlignSearchSizeY = 55;
        model.AlignSearchRois[1] = null;

        var selectedWindowId = model.SelectedWindowId;
        var selectedWindowRoi = model.Part.Windows[0].Roi;
        var viewModel = CreateSmokeMainViewModel(model, services);

        viewModel.SelectNextAlignRoi();
        Assert(model.SelectedWindowId == selectedWindowId, "Next ROI should not change the selected Window.");
        Assert(model.Part.Windows[0].Roi.Equals(selectedWindowRoi), "Next ROI should not resize or replace the selected Window ROI.");
        Assert(model.AlignActiveRoiIndex == 1, $"Next ROI should advance AlignActiveRoiIndex to slot 1, got {model.AlignActiveRoiIndex}.");

        var sync = viewModel.CreateRoiUiSyncState(200, 160, FormatSmokeRoi);
        Assert(sync.ActiveRoiText == "ROI - 2 / 3", $"Active ROI label should describe the search slot, got '{sync.ActiveRoiText}'.");
        Assert(!sync.SearchSizeRoi.HasValue, "Empty active search slot should not overwrite Search Size text boxes.");
        Assert(model.AlignSearchSizeX == 77 && model.AlignSearchSizeY == 55, "ROI UI sync should not mutate Search Size from Window ROI.");
    }

    private static void VerifyAlignDeleteClearsSearchSlot(AppServices services)
    {
        var model = CreateSyntheticModel(200, 160);
        model.Part.SourceWidth = 200;
        model.Part.SourceHeight = 160;
        model.AlignSearchNum = 2;
        model.AlignActiveRoiIndex = 1;
        model.AlignSearchRois[0] = new RoiRect(10, 20, 30, 40);
        model.AlignSearchRois[1] = new RoiRect(70, 80, 50, 60);

        var selectedWindowId = model.SelectedWindowId;
        var windowCount = model.Part.Windows.Count;
        var activeWindow = model.Part.Windows[0];
        var align = activeWindow.Algorithms.First(algorithm => string.Equals(algorithm.Type, "AlgoAlign", StringComparison.OrdinalIgnoreCase));
        AlgorithmParameterStore.Set(align.Parameters, "Align.SearchPoint1.X", "25");
        AlgorithmParameterStore.Set(align.Parameters, "Align.SearchPoint1.Y", "40");
        AlgorithmParameterStore.Set(align.Parameters, "Align.SearchSize1.W", "30");
        AlgorithmParameterStore.Set(align.Parameters, "Align.SearchSize1.H", "40");
        AlgorithmParameterStore.Set(align.Parameters, "Align.SearchPoint2.X", "95");
        AlgorithmParameterStore.Set(align.Parameters, "Align.SearchPoint2.Y", "110");
        AlgorithmParameterStore.Set(align.Parameters, "Align.SearchSize2.W", "50");
        AlgorithmParameterStore.Set(align.Parameters, "Align.SearchSize2.H", "60");

        var viewModel = CreateSmokeMainViewModel(model, services);
        viewModel.DeleteActiveAlignRoi();

        Assert(model.Part.Windows.Count == windowCount, "Delete in Align Search should not delete a Window.");
        Assert(model.SelectedWindowId == selectedWindowId, "Delete in Align Search should not change selected Window.");
        Assert(model.AlignSearchRois[0].HasValue, "Delete should preserve other Align Search ROI slots.");
        Assert(!model.AlignSearchRois[1].HasValue, "Delete should clear the active Align Search ROI slot.");
        Assert(AlgorithmParameterStore.GetValue(align.Parameters, "Align.SearchPoint1.X") == "25", "Delete should preserve slot 1 parameters.");
        Assert(!AlgorithmParameterStore.ContainsKey(align.Parameters, "Align.SearchPoint2.X"), "Delete should remove active slot SearchPoint X.");
        Assert(!AlgorithmParameterStore.ContainsKey(align.Parameters, "Align.SearchSize2.W"), "Delete should remove active slot SearchSize W.");
    }

    private static void VerifyPartRuntimeRoutesBlobBridge()
    {
        const int width = 256;
        const int height = 256;
        var model = new InspectionModel
        {
            ModelName = "GenericBridgeSmoke",
            Threshold2D = 128
        };
        model.EnsureStructure();

        var window = new InspectionWindowData
        {
            Name = "GenericWindow",
            Roi = new RoiRect(48, 48, 160, 160)
        };
        var algorithm = new InspectionAlgorithmData
        {
            Type = "AlgoBody_Blob",
            DisplayName = "Generic Body Blob",
            AlgorithmRoi = new RoiRect(64, 64, 128, 128)
        };
        window.Algorithms.Add(algorithm);
        model.Part.Windows.Add(window);
        model.SelectedWindowId = window.Id;
        model.EnsureStructure();

        var image = new PartRuntimeImage(CreateSyntheticImage(width, height), width, height, width * 4, model.Threshold2D);
        var result = new PartInspectionRuntime().Run(model, image);
        var runAlgorithm = result.Windows.Single().Algorithms.Single();

        Assert(runAlgorithm.NativeBridgeName == "Blob", "Body Blob should route through the dedicated Blob C++ bridge.");
        Assert(runAlgorithm.NativeBridgeMode == "native", "Blob bridge should report native mode when MptiBridgeRunBlob succeeds.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Runtime.BlobBridge") == "native", "Blob bridge status should be persisted to algorithm parameters.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Runtime.BlobIsOK") == "True", "Blob bridge OK state should be persisted.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Runtime.NativeBridgeName") == "Blob", "Unified native bridge name should be persisted.");
    }

    private static void VerifyPartRuntimeRoutesEdgeBridge()
    {
        const int width = 256;
        const int height = 256;
        var model = new InspectionModel
        {
            ModelName = "EdgeBridgeSmoke",
            Threshold2D = 128
        };
        model.EnsureStructure();

        var window = new InspectionWindowData
        {
            Name = "EdgeWindow",
            Roi = new RoiRect(48, 48, 160, 160)
        };
        var algorithm = new InspectionAlgorithmData
        {
            Type = "AlgoEdge",
            DisplayName = "Smoke Edge",
            AlgorithmRoi = new RoiRect(64, 64, 128, 128)
        };
        window.Algorithms.Add(algorithm);
        model.Part.Windows.Add(window);
        model.SelectedWindowId = window.Id;
        model.EnsureStructure();

        var image = new PartRuntimeImage(CreateSyntheticImage(width, height), width, height, width * 4, model.Threshold2D);
        var result = new PartInspectionRuntime().Run(model, image);
        var runAlgorithm = result.Windows.Single().Algorithms.Single();

        Assert(runAlgorithm.NativeBridgeName == "Edge", "Edge should route through the dedicated Edge C++ bridge.");
        Assert(runAlgorithm.NativeBridgeMode == "native", "Edge bridge should report native mode when MptiBridgeRunEdge succeeds.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Runtime.EdgeBridge") == "native", "Edge bridge status should be persisted to algorithm parameters.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Runtime.EdgeIsOK") == "True", "Edge bridge OK state should be persisted.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Runtime.NativeBridgeName") == "Edge", "Unified native bridge name should be persisted.");
    }

    private static void VerifyPartRuntimeRoutesBGABridge()
    {
        const int width = 256;
        const int height = 256;
        var model = new InspectionModel
        {
            ModelName = "BGABridgeSmoke",
            Threshold2D = 128
        };
        model.EnsureStructure();

        var window = new InspectionWindowData
        {
            Name = "BGAWindow",
            Roi = new RoiRect(48, 48, 160, 160)
        };
        var algorithm = new InspectionAlgorithmData
        {
            Type = "AlgoBGA",
            DisplayName = "Smoke BGA",
            AlgorithmRoi = new RoiRect(64, 64, 128, 128)
        };
        window.Algorithms.Add(algorithm);
        model.Part.Windows.Add(window);
        model.SelectedWindowId = window.Id;
        model.EnsureStructure();

        var image = new PartRuntimeImage(CreateSyntheticImage(width, height), width, height, width * 4, model.Threshold2D);
        var result = new PartInspectionRuntime().Run(model, image);
        var runAlgorithm = result.Windows.Single().Algorithms.Single();

        Assert(runAlgorithm.NativeBridgeName == "BGA", "BGA should route through the dedicated BGA C++ bridge.");
        Assert(runAlgorithm.NativeBridgeMode == "native", "BGA bridge should report native mode when MptiBridgeRunBGA succeeds.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Runtime.BGABridge") == "native", "BGA bridge status should be persisted to algorithm parameters.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Runtime.BGAIsOK") == "True", "BGA bridge OK state should be persisted.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Runtime.NativeBridgeName") == "BGA", "Unified native bridge name should be persisted.");
    }

    private static void VerifyPartRuntimeRoutesPatternBridge()
    {
        const int width = 256;
        const int height = 256;
        var model = new InspectionModel
        {
            ModelName = "PatternBridgeSmoke",
            Threshold2D = 128
        };
        model.EnsureStructure();

        var window = new InspectionWindowData
        {
            Name = "PatternWindow",
            Roi = new RoiRect(48, 48, 160, 160)
        };
        var algorithm = new InspectionAlgorithmData
        {
            Type = "AlgoPattern",
            DisplayName = "Smoke Pattern",
            AlgorithmRoi = new RoiRect(64, 64, 128, 128)
        };
        AlgorithmParameterStore.Set(algorithm.Parameters, "Pattern.MatchScore", "80");
        window.Algorithms.Add(algorithm);
        model.Part.Windows.Add(window);
        model.SelectedWindowId = window.Id;
        model.EnsureStructure();

        var image = new PartRuntimeImage(CreateSyntheticImage(width, height), width, height, width * 4, model.Threshold2D);
        var result = new PartInspectionRuntime().Run(model, image);
        var runAlgorithm = result.Windows.Single().Algorithms.Single();

        Assert(runAlgorithm.NativeBridgeName == "Pattern", "Pattern should route through the dedicated Pattern C++ bridge.");
        Assert(runAlgorithm.NativeBridgeMode == "native", "Pattern bridge should report native mode when MptiBridgeRunPattern succeeds.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Runtime.PatternBridge") == "native", "Pattern bridge status should be persisted to algorithm parameters.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Runtime.PatternIsOK") == "True", "Pattern bridge OK state should be persisted.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Runtime.NativeBridgeName") == "Pattern", "Unified native bridge name should be persisted.");
    }

    // -------------------------------------------------------------------------------------
    // 2026-05-21 handoff (exact-flow-algorithm-types) 후속 회귀 테스트.
    // 새로 flow path 에 연결된 algorithm type 들이 PTT 없는 환경에서도 적어도
    // per-algo C++ bridge (Blob/Edge/Pattern) 로 안전하게 떨어지는지 확인한다.
    //
    // Runtime.FlowBridge=native-flow 의 verification 은 실 PTT 파일이 필요한 통합
    // 테스트이며, 현재는 docs/2026-05-21-handoff-exact-flow-algorithm-types.md 의 §"다음 추천 작업"
    // 1 항에 따라 사용자 GUI 검증으로 위임한다.
    // -------------------------------------------------------------------------------------

    private static void VerifyPartRuntimeRoutesNgBlobToBlobBridge()
    {
        VerifyAlgorithmRoutesToBridgeFamily(
            algorithmType: "AlgoNGBlob",
            displayName: "Smoke NG Blob",
            bridgeName: "Blob",
            runtimeStatusKey: "Runtime.BlobBridge",
            runtimeIsOkKey: "Runtime.BlobIsOK");
    }

    private static void VerifyPartRuntimeRoutesBodyEdgeToEdgeBridge()
    {
        VerifyAlgorithmRoutesToBridgeFamily(
            algorithmType: "AlgoBodyEdge",
            displayName: "Smoke Body Edge",
            bridgeName: "Edge",
            runtimeStatusKey: "Runtime.EdgeBridge",
            runtimeIsOkKey: "Runtime.EdgeIsOK");
    }

    private static void VerifyPartRuntimeRoutesPatternDiffToPatternBridge()
    {
        VerifyAlgorithmRoutesToBridgeFamily(
            algorithmType: "AlgoPatternDiff",
            displayName: "Smoke Pattern Diff",
            bridgeName: "Pattern",
            runtimeStatusKey: "Runtime.PatternBridge",
            runtimeIsOkKey: "Runtime.PatternIsOK");
    }

    private static void VerifyPartRuntimeRoutesOcrToPatternBridge()
    {
        VerifyAlgorithmRoutesToBridgeFamily(
            algorithmType: "AlgoOCR",
            displayName: "Smoke OCR",
            bridgeName: "Pattern",
            runtimeStatusKey: "Runtime.PatternBridge",
            runtimeIsOkKey: "Runtime.PatternIsOK");
    }

    private static void VerifyPartRuntimeRoutesPocrToPatternBridge()
    {
        VerifyAlgorithmRoutesToBridgeFamily(
            algorithmType: "AlgoPOCR",
            displayName: "Smoke POCR",
            bridgeName: "Pattern",
            runtimeStatusKey: "Runtime.PatternBridge",
            runtimeIsOkKey: "Runtime.PatternIsOK");
    }

    // 공통 helper: synthetic 이미지 + 알고리즘 1 개로 PartInspectionRuntime 을 돌리고,
    // 기대된 native bridge family 로 라우팅 되었는지 확인.
    private static void VerifyAlgorithmRoutesToBridgeFamily(
        string algorithmType,
        string displayName,
        string bridgeName,
        string runtimeStatusKey,
        string runtimeIsOkKey)
    {
        const int width = 256;
        const int height = 256;
        var model = new InspectionModel
        {
            ModelName = $"{algorithmType}BridgeSmoke",
            Threshold2D = 128
        };
        model.EnsureStructure();

        var window = new InspectionWindowData
        {
            Name = $"{algorithmType}Window",
            Roi = new RoiRect(48, 48, 160, 160)
        };
        var algorithm = new InspectionAlgorithmData
        {
            Type = algorithmType,
            DisplayName = displayName,
            AlgorithmRoi = new RoiRect(64, 64, 128, 128)
        };
        window.Algorithms.Add(algorithm);
        model.Part.Windows.Add(window);
        model.SelectedWindowId = window.Id;
        model.EnsureStructure();

        var image = new PartRuntimeImage(CreateSyntheticImage(width, height), width, height, width * 4, model.Threshold2D);
        var result = new PartInspectionRuntime().Run(model, image);
        var runAlgorithm = result.Windows.Single().Algorithms.Single();

        Assert(runAlgorithm.NativeBridgeName == bridgeName, $"{algorithmType} should route through the dedicated {bridgeName} C++ bridge.");
        Assert(runAlgorithm.NativeBridgeMode == "native", $"{bridgeName} bridge should report native mode when MptiBridgeRun{bridgeName} succeeds for {algorithmType}.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, runtimeStatusKey) == "native", $"{bridgeName} bridge status should be persisted to algorithm parameters for {algorithmType}.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, runtimeIsOkKey) == "True", $"{bridgeName} bridge OK state should be persisted for {algorithmType}.");
        Assert(AlgorithmParameterStore.GetValue(algorithm.Parameters, "Runtime.NativeBridgeName") == bridgeName, $"Unified native bridge name should be persisted as {bridgeName} for {algorithmType}.");
    }

    // -------------------------------------------------------------------------------------
    // 2026-05-22 handoff family-dispatch 후속 회귀 테스트.
    // AlgorithmCatalog 의 NativeAlgoType → MptiBridgeGetAlgoFamily 매핑이
    // 의도한 family 로 떨어지는지 algorithm type 별로 잠근다.
    //
    // family 이 깨지면 RuntimeFlowAlgorithmAdapter 의 flow path 선택 +
    // AlgorithmNativeBridgeAdapter 의 per-algo bridge 라우팅 양쪽이 동시에 잘못된다.
    // 단일 native export 호출이라 비용도 낮다.
    // -------------------------------------------------------------------------------------
    private static void VerifyAlgoFamilyLookupMatchesCatalog()
    {
        // 각 family 의 모든 의도된 algorithm type. 새 entry 추가 시 여기 한 줄 + catalog +
        // NativeAlgoTypeIds + native switch 4 곳에 동기화되어야 한다.
        var expectations = new (string AlgorithmType, int ExpectedFamily, int ExpectedNativeAlgoType)[]
        {
            // FAMILY_ALIGN
            ("AlgoAlign", MptiFlowNativeBridge.FAMILY_ALIGN, NativeAlgoTypeIds.Align),

            // FAMILY_BLOB
            ("AlgoBlob",      MptiFlowNativeBridge.FAMILY_BLOB, NativeAlgoTypeIds.Blob),
            ("AlgoBody_Blob", MptiFlowNativeBridge.FAMILY_BLOB, NativeAlgoTypeIds.BodyBlob),
            ("AlgoNGBlob",    MptiFlowNativeBridge.FAMILY_BLOB, NativeAlgoTypeIds.NgBlob),
            ("AlgoBump",      MptiFlowNativeBridge.FAMILY_BLOB, NativeAlgoTypeIds.Bump),

            // FAMILY_EDGE
            ("AlgoEdge",      MptiFlowNativeBridge.FAMILY_EDGE, NativeAlgoTypeIds.Edge),
            ("AlgoBodyEdge",  MptiFlowNativeBridge.FAMILY_EDGE, NativeAlgoTypeIds.BodyEdge),
            ("AlgoLine",      MptiFlowNativeBridge.FAMILY_EDGE, NativeAlgoTypeIds.Line),
            ("AlgoDistance",  MptiFlowNativeBridge.FAMILY_EDGE, NativeAlgoTypeIds.Distance),
            ("AlgoEdgePoint", MptiFlowNativeBridge.FAMILY_EDGE, NativeAlgoTypeIds.EdgePoint),

            // FAMILY_PATTERN
            ("AlgoPattern",     MptiFlowNativeBridge.FAMILY_PATTERN, NativeAlgoTypeIds.Pattern),
            ("AlgoPatternDiff", MptiFlowNativeBridge.FAMILY_PATTERN, NativeAlgoTypeIds.PatternDiff),
            ("AlgoOCR",         MptiFlowNativeBridge.FAMILY_PATTERN, NativeAlgoTypeIds.Ocr),
            ("AlgoPOCR",        MptiFlowNativeBridge.FAMILY_PATTERN, NativeAlgoTypeIds.Pocr),
            ("AlgoForeignOCV",  MptiFlowNativeBridge.FAMILY_PATTERN, NativeAlgoTypeIds.ForeignOcv),

            // FAMILY_BGA
            ("AlgoBGA",   MptiFlowNativeBridge.FAMILY_BGA, NativeAlgoTypeIds.Bga),
            ("AlgoLQBGA", MptiFlowNativeBridge.FAMILY_BGA, NativeAlgoTypeIds.LqBga),

            // 단일 algorithm 인 family 들
            ("AlgoPadBW",  MptiFlowNativeBridge.FAMILY_PADBW,  NativeAlgoTypeIds.PadBw),
            ("AlgoShapeX", MptiFlowNativeBridge.FAMILY_SHAPEX, NativeAlgoTypeIds.ShapeX),
            // FAMILY_BW: AlgoBW 는 NativeAlgoType=Unknown (flow path 미연결) 이지만 native
            // GetAlgoFamily 자체는 eAlgoBW 에 대해 BW family 를 반환한다. 카탈로그 NativeAlgoType
            // 이 0 이라 직접 검증은 별도 처리 (아래 BW 전용 assert).
        };

        foreach (var (algorithmType, expectedFamily, expectedNativeAlgoType) in expectations)
        {
            var catalog = AlgorithmCatalog.Find(algorithmType);
            Assert(
                catalog.NativeAlgoType == expectedNativeAlgoType,
                $"Catalog NativeAlgoType for {algorithmType} should be {expectedNativeAlgoType}, got {catalog.NativeAlgoType}.");

            var actualFamily = MptiFlowNativeBridge.MptiBridgeGetAlgoFamily(catalog.NativeAlgoType);
            Assert(
                actualFamily == expectedFamily,
                $"MptiBridgeGetAlgoFamily({catalog.NativeAlgoType}) for {algorithmType} should return {expectedFamily}, got {actualFamily}.");
        }

        // FAMILY_BW: AlgoBW 카탈로그 entry 의 NativeAlgoType 가 Unknown 이지만 native
        // 측에서는 eAlgoBW (= 0) 에 대해 BW family 를 반환해야 한다. 그러나 Unknown==0 과
        // eAlgoBW==0 이 같은 값이라 카탈로그 경로로 구분 불가 → native 가 0 에 대해 어떤
        // family 를 돌려주는지만 확인. 현재 native 는 0 입력시 BW family 를 의도.
        var bwFamily = MptiFlowNativeBridge.MptiBridgeGetAlgoFamily(0);
        // 카탈로그 NativeAlgoType=Unknown=0 인 알고리즘 (AlgoTilt 등) 은 그래도 BW family
        // 로 분류된다는 의미. 실제 코드 경로에서는 RuntimeFlowAlgorithmAdapter /
        // IsFamily helper 가 catalog.NativeAlgoType == Unknown 가드로 먼저 걸러내기 때문에
        // 잘못된 family 라우팅이 발생하지 않는다. 본 assert 는 native 가 enum 0 을 어떻게
        // 다루는지의 회귀 잠금이다.
        Assert(
            bwFamily == MptiFlowNativeBridge.FAMILY_BW,
            $"MptiBridgeGetAlgoFamily(0) (= eAlgoBW) should return FAMILY_BW, got {bwFamily}.");

        // FAMILY_UNKNOWN: 정의되지 않은 enum 값은 Unknown 으로 떨어져야 한다.
        var negativeFamily = MptiFlowNativeBridge.MptiBridgeGetAlgoFamily(-1);
        Assert(
            negativeFamily == MptiFlowNativeBridge.FAMILY_UNKNOWN,
            $"MptiBridgeGetAlgoFamily(-1) should return FAMILY_UNKNOWN, got {negativeFamily}.");

        var outOfRangeFamily = MptiFlowNativeBridge.MptiBridgeGetAlgoFamily(99999);
        Assert(
            outOfRangeFamily == MptiFlowNativeBridge.FAMILY_UNKNOWN,
            $"MptiBridgeGetAlgoFamily(99999) should return FAMILY_UNKNOWN, got {outOfRangeFamily}.");

        // catalog NativeAlgoType=Unknown 인 algorithm 의 IsFamily 가드 동작 확인
        // (예: AlgoTilt → MptiBridgeRunBlob 분기 못 들어감).
        var tiltCatalog = AlgorithmCatalog.Find("AlgoTilt");
        Assert(
            tiltCatalog.NativeAlgoType == NativeAlgoTypeIds.Unknown,
            $"AlgoTilt catalog NativeAlgoType should be Unknown, got {tiltCatalog.NativeAlgoType}.");
    }

    private static void VerifyFlowAlgorithmExecutionServiceRejectsMissingPtt()
    {
        var execution = new FlowAlgorithmExecutionService();
        var algorithm = new BlobFlowAlgorithm();
        var result = execution.Run(
            Path.Combine(Path.GetTempPath(), "missing-flow-smoke.ptt"),
            algorithm,
            algorithm.CreateParameters());

        Assert(!result.Success, "Flow algorithm execution should fail cleanly when PTT is missing.");
        Assert(result.Summary.IndexOf("PTT file", StringComparison.OrdinalIgnoreCase) >= 0, "Missing PTT failure should explain the missing PTT.");
    }

    private static MainViewModel CreateSmokeMainViewModel(InspectionModel model, AppServices services)
    {
        return new MainViewModel(
            model,
            new SmokeDialogOwner(),
            services.FileDialog,
            services.ModelWorkflow,
            services.ApplicationPath,
            services.PartImportWorkflow,
            services.ImageLoadWorkflow,
            services.AlignPartTeaching,
            services.AlignCondition,
            new RoiCanvasViewModel(services.RoiInteraction, services.RoiModel),
            services.RoiUiState,
            services.ImageRuntimeState,
            services.InspectionWorkflow,
            services.InspectionFlow,
            services.AlignFlowRequestFactory,
            services.ThresholdPreviewWorkflow,
            services.AlgorithmLight,
            services.PttLightPreview,
            services.PttViewerWorkflow,
            services.FlowAlgorithms);
    }

    private static string FormatSmokeRoi(RoiRect? roi)
    {
        return roi.HasValue
            ? $"X {roi.Value.X} Y {roi.Value.Y} W {roi.Value.Width} H {roi.Value.Height}"
            : "none";
    }

    private sealed class SmokeDialogOwner : IDialogOwner
    {
        public System.Windows.Window GetDialogOwner()
        {
            return null!;
        }
    }

    private static void Assert(bool condition, string message)
    {
        if (!condition)
        {
            throw new InvalidOperationException(message);
        }
    }

    private static int CountKeys(Dictionary<string, string> parameters, string key)
    {
        return parameters.Keys.Count(candidate => string.Equals(candidate, key, StringComparison.OrdinalIgnoreCase));
    }

    private static bool IsBridgeAlgorithm(string type)
    {
        return string.Equals(type, "AlgoAlign", StringComparison.OrdinalIgnoreCase)
            || string.Equals(type, "AlgoShapeX", StringComparison.OrdinalIgnoreCase)
            || string.Equals(type, "AlgoPadBW", StringComparison.OrdinalIgnoreCase)
            || string.Equals(type, "AlgoBGA", StringComparison.OrdinalIgnoreCase)
            || string.Equals(type, "AlgoEdge", StringComparison.OrdinalIgnoreCase)
            || string.Equals(type, "AlgoPattern", StringComparison.OrdinalIgnoreCase);
    }

    private static string ReadParam(Dictionary<string, string> parameters, string key)
    {
        return parameters.TryGetValue(key, out var value) ? value : "-";
    }

    private static byte[] CreateSyntheticImage(int width, int height)
    {
        // BGRA buffer with bright square in the center (used as fiducial target)
        var pixels = new byte[width * height * 4];
        var cx0 = width / 4;
        var cy0 = height / 4;
        var cx1 = width * 3 / 4;
        var cy1 = height * 3 / 4;
        for (var y = 0; y < height; y++)
        {
            for (var x = 0; x < width; x++)
            {
                var offset = (y * width + x) * 4;
                var inSquare = x >= cx0 && x < cx1 && y >= cy0 && y < cy1;
                var gray = inSquare ? (byte)220 : (byte)40;
                pixels[offset + 0] = gray;
                pixels[offset + 1] = gray;
                pixels[offset + 2] = gray;
                pixels[offset + 3] = 255;
            }
        }
        return pixels;
    }

    private static InspectionModel CreateSyntheticModel(int imageWidth, int imageHeight)
    {
        var model = new InspectionModel
        {
            ModelName = "SmokeTest",
            Algorithm = "AlgoAlign",
            Threshold2D = 128
        };
        model.EnsureStructure();

        var windowW = Math.Max(64, imageWidth / 4);
        var windowH = Math.Max(64, imageHeight / 4);
        var windowX = (imageWidth - windowW) / 2;
        var windowY = (imageHeight - windowH) / 2;
        var window = new InspectionWindowData
        {
            Name = "SmokeWindow",
            Roi = new RoiRect(windowX, windowY, windowW, windowH)
        };

        var alignSearchSize = 40;
        for (var i = 0; i < 4; i++)
        {
            var sx = i % 2 == 0 ? windowX + 16 : windowX + windowW - 16;
            var sy = i < 2 ? windowY + 16 : windowY + windowH - 16;
            model.AlignSearchRois[i] = new RoiRect(
                sx - alignSearchSize / 2,
                sy - alignSearchSize / 2,
                alignSearchSize,
                alignSearchSize);
        }
        model.AlignSearchSizeX = alignSearchSize;
        model.AlignSearchSizeY = alignSearchSize;
        model.AlignSearchNum = 4;
        model.AlignShiftEnabled = true;
        model.AlignAngleEnabled = true;

        window.Algorithms.Add(new InspectionAlgorithmData { Type = "AlgoAlign", DisplayName = "Smoke Align" });
        window.Algorithms.Add(new InspectionAlgorithmData
        {
            Type = "AlgoShapeX",
            DisplayName = "Smoke ShapeX",
            AlgorithmRoi = new RoiRect(windowX + 8, windowY + 8, windowW - 16, windowH - 16)
        });
        window.Algorithms.Add(new InspectionAlgorithmData
        {
            Type = "AlgoPadBW",
            DisplayName = "Smoke PadBW",
            AlgorithmRoi = new RoiRect(windowX + 12, windowY + 12, windowW - 24, windowH - 24)
        });

        model.Part.Windows.Add(window);
        model.SelectedWindowId = window.Id;
        model.EnsureStructure();
        return model;
    }
}
