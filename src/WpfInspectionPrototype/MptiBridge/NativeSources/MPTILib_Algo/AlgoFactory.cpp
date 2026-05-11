#include "stdafx.h"
#include "AlgoFactory.h"


void CAlgoFactory::CreateCAlgoClone(InspAlgoType eAlgoType, LPVOID lpSource, LPVOID& lpTarget)
{
	switch (eAlgoType)
	{
	case eAlgoAlign:
		lpTarget = new AlgoAlign();
		memcpy_s(lpTarget, sizeof(AlgoAlign), lpSource, sizeof(AlgoAlign));
		break;
	case eAlgoBW:
		lpTarget = new AlgoBW();
		memcpy_s(lpTarget, sizeof(AlgoBW), lpSource, sizeof(AlgoBW));
		break;
	case eAlgoBlob:
		lpTarget = new AlgoBlob();
		memcpy_s(lpTarget, sizeof(AlgoBlob), lpSource, sizeof(AlgoBlob));
		break;
	case eAlgoBody_Blob:
		lpTarget = new AlgoBodyBlob();
		memcpy_s(lpTarget, sizeof(AlgoBodyBlob), lpSource, sizeof(AlgoBodyBlob));
		break;
	case eAlgoTilt:
		lpTarget = new AlgoTilt();
		memcpy_s(lpTarget, sizeof(AlgoTilt), lpSource, sizeof(AlgoTilt));
		break;
	case eAlgoOCR:
		lpTarget = new AlgoOCR();
		memcpy_s(lpTarget, sizeof(AlgoOCR), lpSource, sizeof(AlgoOCR));
		break;
	case eAlgoPattern:
		lpTarget = new AlgoPattern();
		memcpy_s(lpTarget, sizeof(AlgoPattern), lpSource, sizeof(AlgoPattern));
		break;
	case eAlgoColor:
	case eAlgoLead_Color:
		lpTarget = new AlgoColor();
		memcpy_s(lpTarget, sizeof(AlgoColor), lpSource, sizeof(AlgoColor));
		break;
	case eAlgoGray_Mean:
		lpTarget = new AlgoGrayMean();
		memcpy_s(lpTarget, sizeof(AlgoGrayMean), lpSource, sizeof(AlgoGrayMean));
		break;
	case eAlgoHeight_Mean:
		lpTarget = new AlgoHeightMean();
		memcpy_s(lpTarget, sizeof(AlgoHeightMean), lpSource, sizeof(AlgoHeightMean));
		break;
	case eAlgoGray_Diff:
		lpTarget = new AlgoGrayDiff();
		memcpy_s(lpTarget, sizeof(AlgoGrayDiff), lpSource, sizeof(AlgoGrayDiff));
		break;
	case eAlgoHeight_Diff:
		lpTarget = new AlgoHeightDiff();
		memcpy_s(lpTarget, sizeof(AlgoHeightDiff), lpSource, sizeof(AlgoHeightDiff));
		break;
	case eAlgoLead_Search:
	case eAlgoTab_Search:
		lpTarget = new AlgoLeadSearch();
		memcpy_s(lpTarget, sizeof(AlgoLeadSearch), lpSource, sizeof(AlgoLeadSearch));
		break;
	case eAlgoBridge:
		lpTarget = new AlgoBridge();
		memcpy_s(lpTarget, sizeof(AlgoBridge), lpSource, sizeof(AlgoBridge));
		break;
	case eAlgoLead_Tip:
		lpTarget = new AlgoLeadTip();
		memcpy_s(lpTarget, sizeof(AlgoLeadTip), lpSource, sizeof(AlgoLeadTip));
		break;
	case eAlgoLead_Lift:
		lpTarget = new AlgoLeadLift();
		memcpy_s(lpTarget, sizeof(AlgoLeadLift), lpSource, sizeof(AlgoLeadLift));
		break;
	case eAlgoLead_Solder:
		lpTarget = new AlgoLeadSolder();
		memcpy_s(lpTarget, sizeof(AlgoLeadSolder), lpSource, sizeof(AlgoLeadSolder));
		break;
	case eAlgoLead_SideSolder:
		lpTarget = new AlgoLeadSideSolder();
		memcpy_s(lpTarget, sizeof(AlgoLeadSideSolder), lpSource, sizeof(AlgoLeadSideSolder));
		break;
	case eAlgoWidth:
		lpTarget = new AlgoLength();
		memcpy_s(lpTarget, sizeof(AlgoLength), lpSource, sizeof(AlgoLength));
		break;
	case eAlgoVolume:
		lpTarget = new AlgoVolume();
		memcpy_s(lpTarget, sizeof(AlgoVolume), lpSource, sizeof(AlgoVolume));
		break;
	case eAlgoTab:
		lpTarget = new AlgoTab();
		memcpy_s(lpTarget, sizeof(AlgoTab), lpSource, sizeof(AlgoTab));
		break;
	case eAlgoGrid:
		lpTarget = new AlgoGrid();
		memcpy_s(lpTarget, sizeof(AlgoGrid), lpSource, sizeof(AlgoGrid));
		break;
	case eAlgoLine:
		lpTarget = new AlgoLine();
		memcpy_s(lpTarget, sizeof(AlgoLine), lpSource, sizeof(AlgoLine));
		break;
	case eAlgoEdge:
		lpTarget = new AlgoEdge();
		memcpy_s(lpTarget, sizeof(AlgoEdge), lpSource, sizeof(AlgoEdge));
		break;
	case eAlgoSolderCone:
		lpTarget = new AlgoSolderCone();
		memcpy_s(lpTarget, sizeof(AlgoSolderCone), lpSource, sizeof(AlgoSolderCone));
		break;
	case eAlgoColorXY:
		lpTarget = new AlgoColorXY();
		memcpy_s(lpTarget, sizeof(AlgoColorXY), lpSource, sizeof(AlgoColorXY));
		break;
	case eAlgoAlignEdge:
		lpTarget = new AlgoAlignEdge();
		memcpy_s(lpTarget, sizeof(AlgoAlignEdge), lpSource, sizeof(AlgoAlignEdge));
		break;
	case eAlgoPadAlign:
		lpTarget = new AlgoPadAlign();
		memcpy_s(lpTarget, sizeof(AlgoPadAlign), lpSource, sizeof(AlgoPadAlign));
		break;
	case eAlgoPOCR:
		lpTarget = new AlgoPOCR();
		memcpy_s(lpTarget, sizeof(AlgoPOCR), lpSource, sizeof(AlgoPOCR));
		break;
	case eAlgoWire:
		lpTarget = new AlgoWire();
		memcpy_s(lpTarget, sizeof(AlgoWire), lpSource, sizeof(AlgoWire));
		break;
	case eAlgoFoot:
		lpTarget = new AlgoFoot();
		memcpy_s(lpTarget, sizeof(AlgoFoot), lpSource, sizeof(AlgoFoot));
		break;
	case eAlgoBarcode:
		lpTarget = new AlgoBarcode();
		memcpy_s(lpTarget, sizeof(AlgoBarcode), lpSource, sizeof(AlgoBarcode));
		break;
	case eAlgoFillet:
		lpTarget = new AlgoFillet();
		memcpy_s(lpTarget, sizeof(AlgoFillet), lpSource, sizeof(AlgoFillet));
		break;
	case eAlgoBGA:
		lpTarget = new AlgoBGA();
		memcpy_s(lpTarget, sizeof(AlgoBGA), lpSource, sizeof(AlgoBGA));
		break;
	case eAlgoBump:
		lpTarget = new AlgoBump();
		memcpy_s(lpTarget, sizeof(AlgoBump), lpSource, sizeof(AlgoBump));
		break;
	case eAlgoNGBlob:
		lpTarget = new AlgoNGBlob();
		memcpy_s(lpTarget, sizeof(AlgoNGBlob), lpSource, sizeof(AlgoNGBlob));
		break;
	case eAlgoPadBW:
		lpTarget = new AlgoPadBW();
		memcpy_s(lpTarget, sizeof(AlgoPadBW), lpSource, sizeof(AlgoPadBW));
		break;
	case eAlgoBodyEdge:
		lpTarget = new AlgoBodyEdge();
		memcpy_s(lpTarget, sizeof(AlgoBodyEdge), lpSource, sizeof(AlgoBodyEdge));
		break;
	case eAlgoDistance:
		lpTarget = new AlgoDistance();//g_pMManager->pem_new<AlgoDistance>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		memcpy_s(lpTarget, sizeof(AlgoDistance), lpSource, sizeof(AlgoDistance));
		break;
	case eAlgoPatternDiff:
		lpTarget = new AlgoPatternDiff();
		memcpy_s(lpTarget, sizeof(AlgoPatternDiff), lpSource, sizeof(AlgoPatternDiff));
		break;
	case eAlgoShapeX:
		lpTarget = new AlgoShapeX();
		memcpy_s(lpTarget, sizeof(AlgoShapeX), lpSource, sizeof(AlgoShapeX));
		break;
	case eAlgoPadArray:
		lpTarget = g_pMManager->pem_new<AlgoPadArray>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		memcpy_s(lpTarget, sizeof(AlgoPadArray), lpSource, sizeof(AlgoPadArray));
		break;
	default:
		break;
	}
}

void CAlgoFactory::CreateRstAlgoClone(InspAlgoType eAlgoType, LPVOID& lpTarget, bool IsUseMultiArea)
{
	switch (eAlgoType)
	{
	case eAlgoAlign:
		lpTarget = g_pMManager->pem_new<RstAlgoAlign>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoBW:
		lpTarget = g_pMManager->pem_new<RstAlgoBlackWhite>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoBlob:
		lpTarget = g_pMManager->pem_new<RstAlgoBlob>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoBody_Blob:
		lpTarget = g_pMManager->pem_new<RstAlgoBodyBlob>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoTilt:
		lpTarget = g_pMManager->pem_new<RstAlgoTilt>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoOCR:
		lpTarget = g_pMManager->pem_new<RstAlgoOCR>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoPattern:
		lpTarget = g_pMManager->pem_new<RstAlgoPattern>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoTab:
		lpTarget = g_pMManager->pem_new<RstAlgoTab>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoColor:
	case eAlgoLead_Color:
	{
		if (IsUseMultiArea == false)
			lpTarget = g_pMManager->pem_new<RstAlgoColor>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		else
			lpTarget = g_pMManager->pem_new<RstAlgoTab>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
	}
	break;
	case eAlgoGray_Mean:
		lpTarget = g_pMManager->pem_new<RstAlgoGrayMean>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoHeight_Mean:
		lpTarget = g_pMManager->pem_new<RstAlgoHeightMean>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoGray_Diff:
		lpTarget = g_pMManager->pem_new<RstAlgoGrayDiff>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoHeight_Diff:
		lpTarget = g_pMManager->pem_new<RstAlgoHeightDiff>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoLead_Search:
	case eAlgoTab_Search:
		break;
	case eAlgoBridge:
		lpTarget = g_pMManager->pem_new<RstAlgoBridge>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoLead_Tip:
		lpTarget = g_pMManager->pem_new<RstAlgoLeadTip>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoLead_Lift:
		lpTarget = g_pMManager->pem_new<RstAlgoLeadLift>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoLead_Solder:
		lpTarget = g_pMManager->pem_new<RstAlgoLeadSolder>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoLead_SideSolder:
		lpTarget = g_pMManager->pem_new<RstAlgoLeadSideSolder>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoWidth:
		lpTarget = g_pMManager->pem_new<RstAlgoLength>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoVolume:
		lpTarget = g_pMManager->pem_new<RstAlgoVolume>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoGrid:
		lpTarget = g_pMManager->pem_new<RstAlgoGrid>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoLine:
		lpTarget = g_pMManager->pem_new<RstAlgoLine>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoEdge:
		lpTarget = g_pMManager->pem_new<RstAlgoEdge>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoSolderCone:
		lpTarget = g_pMManager->pem_new<RstAlgoSolderCone>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoColorXY:
		lpTarget = g_pMManager->pem_new<RstAlgoColorXY>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoAlignEdge:
		lpTarget = g_pMManager->pem_new<RstAlgoAlignEdge>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoPadAlign:
		lpTarget = g_pMManager->pem_new<RstAlgoPadAlign>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoPOCR:
		lpTarget = g_pMManager->pem_new<RstAlgoPOCR>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoWire:
		lpTarget = g_pMManager->pem_new<RstAlgoWire>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoFoot:
		lpTarget = g_pMManager->pem_new<RstAlgoFoot>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoBarcode:
		lpTarget = g_pMManager->pem_new<RstAlgoBarcode>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoFillet:
		lpTarget = g_pMManager->pem_new<RstAlgoFillet>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoBGA:
		break;
	case eAlgoBump:
		lpTarget = g_pMManager->pem_new<RstAlgoBump>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoNGBlob:
		lpTarget = g_pMManager->pem_new<RstAlgoNGBlob>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoPadBW:
		lpTarget = g_pMManager->pem_new<RstAlgoPadBW>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoDistance:
		lpTarget = g_pMManager->pem_new<RstAlgoDistance>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoBodyEdge:
		lpTarget = g_pMManager->pem_new<RstAlgoBodyEdge>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoPatternDiff:
		lpTarget = g_pMManager->pem_new<RstAlgoPatternDiff>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
	case eAlgoShapeX:
		lpTarget = g_pMManager->pem_new<RstAlgoShapeX>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
		break;
		//shw Delete Xcase eAlgoNewAlgo1:#@lpTarget = g_pMManager->pem_new<RstAlgoNewAlgo1>(false, 0, (PCHAR)__FUNCTION__, __LINE__);#@break;
	default:
		break;
	}
}


IPINSP_ALGO* CAlgoFactory::CreatePInspAlgo(InspAlgoType eAlgoType)
{
	IPINSP_ALGO* ptr = nullptr;
	switch (eAlgoType)
	{
	case eAlgoBW:
		ptr = g_pMManager->pem_new<CPInsp_AlgoBW>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoBlob:
		ptr = g_pMManager->pem_new<CPInsp_AlgoBlob>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoAlign:
		ptr = g_pMManager->pem_new<CPInsp_AlgoAlign>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoBody_Blob:
		ptr = g_pMManager->pem_new<CPInsp_AlgoBodyBlob>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoTilt:
		ptr = g_pMManager->pem_new<CPInsp_AlgoTilt>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoOCR:
		ptr = g_pMManager->pem_new<CPInsp_AlgoOCR>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoPattern:
		ptr = g_pMManager->pem_new<CPInsp_AlgoPattern>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoColor:
		ptr = g_pMManager->pem_new<CPInsp_AlgoColor>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoGray_Mean:
		ptr = g_pMManager->pem_new<CPInsp_AlgoGrayMean>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoHeight_Mean:
		ptr = g_pMManager->pem_new<CPInsp_AlgoHeightMean>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoGray_Diff:
		ptr = g_pMManager->pem_new<CPInsp_AlgoGrayDiff>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoHeight_Diff:
		ptr = g_pMManager->pem_new<CPInsp_AlgoHeightDiff>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoLead_Search:
		ptr = g_pMManager->pem_new<CPInsp_AlgoLeadSearch>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoBridge:
		ptr = g_pMManager->pem_new<CPInsp_AlgoBridge>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoLead_Tip:
		ptr = g_pMManager->pem_new<CPInsp_AlgoLeadTip>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoLead_Lift:
		ptr = g_pMManager->pem_new<CPInsp_AlgoLeadLift>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoLead_Solder:
		ptr = g_pMManager->pem_new<CPInsp_AlgoLeadSolder>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoLead_Color:
		ptr = g_pMManager->pem_new<CPInsp_AlgoLeadColor>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoWidth:
		ptr = g_pMManager->pem_new<CPInsp_AlgoWidth>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoVolume:
		ptr = g_pMManager->pem_new<CPInsp_AlgoVolume>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoLead_SideSolder:
		ptr = g_pMManager->pem_new<CPInsp_AlgoLeadSideSolder>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoTab_Search:
		ptr = g_pMManager->pem_new<CPInsp_AlgoTabSearch>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoTab:
		ptr = g_pMManager->pem_new<CPInsp_AlgoTab>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoColorBand_Search:
		break;
	case eAlgoGrid:
		ptr = g_pMManager->pem_new<CPInsp_AlgoGrid>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoLine:
		ptr = g_pMManager->pem_new<CPInsp_AlgoLine>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoEdge:
		ptr = g_pMManager->pem_new<CPInsp_AlgoEdge>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoSolderCone:
		ptr = g_pMManager->pem_new<CPInsp_AlgoSolderCone>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoColorXY:
		ptr = g_pMManager->pem_new<CPInsp_AlgoColorXY>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoAlignEdge:
		ptr = g_pMManager->pem_new<CPInsp_AlgoAlignEdge>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoPadAlign:
		ptr = g_pMManager->pem_new<CPInsp_AlgoPadAlign>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoPOCR:
		ptr = g_pMManager->pem_new<CPInsp_AlgoPOCR>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoDisColor:
		break;
	case eAlgoWire:
		ptr = g_pMManager->pem_new<CPInsp_AlgoWire>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoFoot:
		ptr = g_pMManager->pem_new<CPInsp_AlgoFoot>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoBarcode:
		ptr = g_pMManager->pem_new<CPInsp_AlgoBarcode>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoFillet:
		ptr = g_pMManager->pem_new<CPInsp_AlgoFillet>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoBGA:
		ptr = g_pMManager->pem_new<CPInsp_AlgoBGA>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoBump:
		ptr = g_pMManager->pem_new<CPInsp_AlgoBump>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoNGBlob:
		ptr = g_pMManager->pem_new<CPInsp_AlgoNGBlob>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoPadBW:
		ptr = g_pMManager->pem_new<CPInsp_AlgoPadBW>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoBodyEdge:
		ptr = g_pMManager->pem_new<CPInsp_AlgoBodyEdge>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoDistance:
		ptr = g_pMManager->pem_new<CPInsp_AlgoDistance>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoGWire:
		ptr = g_pMManager->pem_new<CPInsp_AlgoGWire>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoPatternDiff:
		ptr = g_pMManager->pem_new<CPInsp_AlgoPatternDiff>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	case eAlgoShapeX:
		ptr = g_pMManager->pem_new<CPInsp_AlgoShapeX>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
		break;
	//case eAlgoPadArray:
	//	ptr = g_pMManager->pem_new<CPInsp_AlgoPadArray>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	//	break;
		//shw Delete Xcase eAlgoNewAlgo1 : #@ptr = g_pMManager->pem_new<CPInsp_AlgoNewAlgo1>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true); #@break;
	case eAlgoNum:
		break;
	default:
		break;
	}
	return ptr;
}


