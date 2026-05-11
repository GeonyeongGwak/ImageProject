#include "PInsp_AlgoNGBlob.h"

CPInsp_AlgoNGBlob::CPInsp_AlgoNGBlob(void)
{
}


CPInsp_AlgoNGBlob::~CPInsp_AlgoNGBlob(void)
{
}
void CPInsp_AlgoNGBlob::MapClear()
{
	std::map<CString, NGBlobModel *>::iterator it;

	for (it = m_MapModelExc.begin(); it != m_MapModelExc.end(); ++it)
	{
		CString sLog;
		sLog.Format(_T("NGBlobClear() : %s"), it->first);
		g_pMPTI->AddLog(sLog);

		it->second->ModelClear();

		delete it->second;
	}
	m_MapModelExc.clear();
}
void CPInsp_AlgoNGBlob::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
	MapClear();
}

unsigned long long CPInsp_AlgoNGBlob::GetInspAlgoData()
{
	return eSPCAlgoNGBlob;
}

int CPInsp_AlgoNGBlob::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = 0;

	eWholeNgTypeTemp[TypeForeign] = e_NG;

	RstAlgoNGBlob * rstAlgo = (RstAlgoNGBlob *)vRstInspAlgo;

	if (!rstAlgo->bIsOKWarpage)
	{
		eWholeNgTypeTemp[TypeWarpage] = e_NG;
		nCurrentNgType = TypeWarpage;
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoNGBlob::InspWindowArea(int nType)
{
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return true;
}

BOOL CPInsp_AlgoNGBlob::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	return bResult;
}

BOOL CPInsp_AlgoNGBlob::InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* Mask_buf)
{
	BOOL bResult = FALSE;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();

	PIAL::_AlgoNGBlob algoNGBlob;
	PIAL::_RstAlgoNGBlob rstAlgo;
	AlgoNGBlob* algo =  (AlgoNGBlob*)InspAlgo.m_ptrInspAlgoParam; 
	InspWrapper->ConvertAlgo(algo, algoNGBlob);
	std::vector<PIAL::PInspData*> pDataSet;
	int nPartID = g_pInspMng->m_pInspBoardInfo->nPartIDOrg;
	bool bUseAISeg = ext::irc::get()->_CtrlServer.First()->prod.bUseAI && ((algo->idata2 & (int)_NGBlob_UseAI) == (int)_NGBlob_UseAI);
#if MultiProcessFunc 
	if (bUseAISeg)
	{
		if (!g_pInspMng->ProcessAIDataByPartID(nPartID, bUseAISeg, pImg_buf, algoParam.m_dWndW, algoParam.m_dWndH))
			return FALSE;
	}
#endif
	if (InspDataSet) pDataSet = InspDataSet->GetInspDataSet();

	if (algoParam.m_pvInspRstPoly)
	{
		pImg_buf->bUseDSI = true;
		pImg_buf->nWindowID = g_pInspMng->GetWindowID(algoParam.m_nWndIndex);
		pImg_buf->nAlgoID = InspAlgo.m_nAlgoId;
	}

	bResult = InspWrapper->m_PInspAlgo->InspNGBlob(algoNGBlob, *pImg_buf, &rstAlgo, PIALTieArea, vecAlignResult, pDataSet, Mask_buf,false, (void*)&m_MapModelExc);

	if (algoParam.m_pvInspRstPoly)
	{
		for (size_t i = 0; i < pImg_buf->vecDSI.size(); i++)
		{
			InspRstPolyAlgo poly;
			InspWrapper->ConvertAlgo(&pImg_buf->vecDSI[i], &poly);
			algoParam.m_pvInspRstPoly->push_back(poly);
		}
	}

	//if (algoNGBlob.narrdata[NGBlob_by_MinimumNG] > rstAlgo.blob_count)
	//	bResult = true;
	
	if (rstAlgo.blob_count > 200)
		rstAlgo.blob_count = 200;


	for (int i = 0; i < rstAlgo.blob_count; ++i)
	{
		rstAlgo.m_rcArrRect[i].left += algoParam.m_dx;
		rstAlgo.m_rcArrRect[i].right += algoParam.m_dx;
		rstAlgo.m_rcArrRect[i].top += algoParam.m_dy;
		rstAlgo.m_rcArrRect[i].bottom += algoParam.m_dy;		
		if ((rstAlgo.m_rcArrRect[i].right - rstAlgo.m_rcArrRect[i].left) <= 5)
		{
			rstAlgo.m_rcArrRect[i].right += ((float)(5 - (rstAlgo.m_rcArrRect[i].right - rstAlgo.m_rcArrRect[i].left))/2) +0.5;
			rstAlgo.m_rcArrRect[i].left -= ((float)(5 - (rstAlgo.m_rcArrRect[i].right - rstAlgo.m_rcArrRect[i].left))/2)+ 0.5;
			if (rstAlgo.m_rcArrRect[i].right > pImg_buf->m_p2D->Width() + algoParam.m_dx)
				rstAlgo.m_rcArrRect[i].right = pImg_buf->m_p2D->Width() + algoParam.m_dx;
			if (rstAlgo.m_rcArrRect[i].left < algoParam.m_dx)
				rstAlgo.m_rcArrRect[i].left = algoParam.m_dx;
		}
		if ((rstAlgo.m_rcArrRect[i].bottom - rstAlgo.m_rcArrRect[i].top) <= 5)
		{
			rstAlgo.m_rcArrRect[i].bottom += ((float)(5 - (rstAlgo.m_rcArrRect[i].bottom - rstAlgo.m_rcArrRect[i].top))/2) + 0.5;
			rstAlgo.m_rcArrRect[i].top -= ((float)(5 - (rstAlgo.m_rcArrRect[i].bottom - rstAlgo.m_rcArrRect[i].top)) / 2) + 0.5;
			if (rstAlgo.m_rcArrRect[i].bottom > pImg_buf->m_p2D->Length() + algoParam.m_dy)
				rstAlgo.m_rcArrRect[i].bottom = pImg_buf->m_p2D->Length() + algoParam.m_dy;
			if (rstAlgo.m_rcArrRect[i].top < algoParam.m_dy)
				rstAlgo.m_rcArrRect[i].top = algoParam.m_dy;
		}
	}
	InspWrapper->ConvertRstAlgo(rstAlgo, (RstAlgoNGBlob*)sRstAlgo);

	return bResult;
}

BOOL CPInsp_AlgoNGBlob::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	BOOL bRet = FALSE;

	return bRet;
}
int CPInsp_AlgoNGBlob::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoNGBlob::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}
void CPInsp_AlgoNGBlob::LoadExceptModel(CString sPath)
{

	CString extension = sPath.Mid(sPath.GetLength() - 4, 4).MakeLower();
	if (extension != _T(".ppd"))
		return;
	CFile file;
	CFileException fe;
	if (file.Open(sPath, CFile::modeRead, NULL, &fe) == FALSE)
		return;

	CArchive ar(&file, CArchive::load, file.GetLength());

	double dResolX = g_pInspMng->GetResolX();
	double dResolY = g_pInspMng->GetResolY();


	if (g_pMPTI->GetUseMultiProcess())
		ext::InspRoot_Server::get()->ExtModelAdd(&file, sPath, ext::ModelBufferFlag::eExtModel_NGBlobExc);

	//std::shared_ptr<GeoModel> shptr_model = std::shared_ptr<GeoModel>(new GeoModel);

	//shptr_model->LoadFile(ar, dResolX, dResolY);

	ar.Close();
	file.Close();

}
BOOL CPInsp_AlgoNGBlob::LoadExtModel(ext::ModelStatus pCtrl, BYTE * Data, BOOL bCreateImgFile)
{
	//ext::ModelStatus pCtrl = ext::InspRoot_Server::get()->_PatternModelCtrl.First()[nIndex];
	CString sPath(pCtrl.sFilePath);
	void  *Model;
	CString extension = sPath.Mid(sPath.GetLength() - 4, 4).MakeLower();
	if ( extension == _T(".ppd"))
		Model  = &m_MapModelExc;
	else
		return false;


	CMemFile file;
	CArchive arStore(&file, CArchive::store); // °øÀ¯¸Þ¸ð¸®¿¡¼­ °¡Á®¿Í ¼ÂÆÃ.
	arStore.Write(Data, pCtrl.img.imgSz);
	arStore.Close();
	file.Flush();
	file.SeekToBegin();


	CArchive ar(&file, CArchive::load, file.GetLength());

	BOOL bSuccess = LoadExtModelSetting(sPath, Model, ar, m_resolX, m_resolY);

	if (bSuccess && bCreateImgFile)
	{
		CString str;
		str.Format(_T("%s.bmp"), sPath);
		cv::imwrite(std::string(CT2A(str)), m_stModel.Img2D[0]);
	}

	ar.Close();
	file.Close();

	return bSuccess;
}
BOOL CPInsp_AlgoNGBlob::LoadExtModelSetting(CString sPath, void * Model, CArchive& ar, double dResolX, double dResolY)
{
	CString sName(_T("pemtron")), sReadName;
	std::map<CString, NGBlobModel *> * mpModelData;
	mpModelData = static_cast<std::map<CString, NGBlobModel *> *> (Model);

	int nModelType = -1;
	CString extension = sPath.Mid(sPath.GetLength() - 4, 4).MakeLower();
	if ( extension != _T(".ppd"))
	return FALSE;

	NGBlobModel * stModel = new NGBlobModel();
	stModel->ModelClear();


	// PatternDiffExc
	{
		stModel->geoMaster.LoadFile(ar, m_resolX, m_resolY);
		stModel->geoMaster.Preprocess();
		// °¢µµº° process ÇÊ¿ä
		stModel->geoMaster_90.setMatchAlgo(stModel->geoMaster.getMatchAlgo());
		stModel->geoMaster_90.CopyOf(stModel->geoMaster, 90.0F);
		if (stModel->geoMaster_90.isProcessed() == false)
			stModel->geoMaster_90.Preprocess();

		stModel->geoMaster_180.setMatchAlgo(stModel->geoMaster.getMatchAlgo());
		stModel->geoMaster_180.CopyOf(stModel->geoMaster, 180.0F);
		if (stModel->geoMaster_180.isProcessed() == false)
			stModel->geoMaster_180.Preprocess();

		stModel->geoMaster_270.setMatchAlgo(stModel->geoMaster.getMatchAlgo());
		stModel->geoMaster_270.CopyOf(stModel->geoMaster, 270.0F);
		if (stModel->geoMaster_270.isProcessed() == false)
			stModel->geoMaster_270.Preprocess();


	}


	mpModelData->insert(std::make_pair(sPath, stModel));

	return TRUE;
}