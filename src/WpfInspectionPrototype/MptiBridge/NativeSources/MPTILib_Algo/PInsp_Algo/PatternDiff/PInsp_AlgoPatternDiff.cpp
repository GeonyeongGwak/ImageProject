#include "PInsp_AlgoPatternDiff.h"


CPInsp_AlgoPatternDiff::CPInsp_AlgoPatternDiff(void)
{
}


CPInsp_AlgoPatternDiff::~CPInsp_AlgoPatternDiff(void)
{
}

void CPInsp_AlgoPatternDiff::InitAlgo()
{
	m_pCPInsp_Algo = g_pInspMng->GetPtrInspAlgo();
	m_proc3d = g_pInspMng->GetPtrInspAlgo()->GetProc3D();
	m_pProcMilAlgo = g_pInspMng->GetPtrInspAlgo()->GetProcMil();
	m_resolX = g_pInspMng->GetPtrInspAlgo()->GetResolX();
	m_resolY = g_pInspMng->GetPtrInspAlgo()->GetResolY();
	//m_MapModel = new std::map<CString, PatternDiffModel>;
	
	MapClear();

}
void CPInsp_AlgoPatternDiff::MapClear()
{
	std::map<CString, PatternDiffModel *>::iterator it;

	for (it = m_MapModel.begin(); it != m_MapModel.end(); ++it)
	{
		CString sLog; 
		sLog.Format(_T("PatternDiffClear : %s") ,it->first );
		g_pMPTI->AddLog(sLog);

		it->second->ModelClear();
	
		delete it->second;
	}
	m_MapModel.clear();

	for (it = m_MapModelExc.begin(); it != m_MapModelExc.end(); ++it)
	{
		CString sLog;
		sLog.Format(_T("PatternDiffClear : %s"), it->first);
		g_pMPTI->AddLog(sLog);

		it->second->ModelClear();
		
		delete it->second;
	}
	m_MapModelExc.clear();
}

unsigned long long CPInsp_AlgoPatternDiff::GetInspAlgoData()
{
	unsigned long long ret = -1;
	ret = eSPCAlgoPatternDiff;
	//shw Delete Xret = eSPCAlgoPatternDiff;
	return ret;
}

int CPInsp_AlgoPatternDiff::AlgoJudgment(int nWndType, InspAlgo vArrAlgoParam, void * vRstInspAlgo, int *eWholeNgTypeTemp, int *eWholeNgType, unsigned long long &nCurDefectAlgo, int* eAlgoNgType, BOOL &bTipFaultNG, bool bCoiningInspect)
{
	int nCurrentNgType = e_OK;
	RstAlgoPatternDiff * rstAlgo = (RstAlgoPatternDiff *)vRstInspAlgo;
	if (rstAlgo->m_bOK == FALSE || rstAlgo->m_bRstChipping == FALSE)
	{
		eWholeNgTypeTemp[TypeForeign] = e_NG;
		nCurrentNgType = TypeForeign;
	}
	return nCurrentNgType;
}

bool CPInsp_AlgoPatternDiff::InspWindowArea(int nType)
{
	//★★ 코드 수정 필요
	if (nType == m_eInspWindowArea_Type_Lead)
		return false;
	else if (nType == m_eInspWindowArea_Type_ChipTracking)
		return false;
	return false;
}

BOOL CPInsp_AlgoPatternDiff::InspAlgorithm(const InspAlgo &sInspAlgo, WndAlgoImg &sWndAlgoImg, InspRoiImgBuf* sInspImageData, void* sRstAlgo, TotalInspExceptArea stTieArea, InspAlgoParam stAlgoParam, UCHAR* ucArrDstImg)
{
	BOOL bResult = FALSE;

	return bResult;
}
BOOL CPInsp_AlgoPatternDiff::InspAlgorithm_Dll(const InspAlgo& InspAlgo, std::vector<PIAL::_AlignResult> vecAlignResult, PIAL::Insp_Image* pImg_buf, InspAlgoParam algoParam, void* sRstAlgo, PIAL::PInspDataSet* InspDataSet, PIAL::_TotalInspExceptArea PIALTieArea, PIAL::BodyInfo* bodyInfo, PIAL::PI_Buff* dstImg)
{
	//for log

	BOOL bResult = FALSE;
	PInspAlgoWrapper* InspWrapper = g_pInspMng->GePInspWrapper();

	PIAL::_AlgoPatternDiff algoPatternDiff;
	PIAL::_RstAlgoPatternDiff rstAlgo;
	InspWrapper->ConvertAlgo((AlgoPatternDiff*)InspAlgo.m_ptrInspAlgoParam, algoPatternDiff);
	
	double dAngle = 0.0;
	POINTF ptOffset;// = SelectOffset(algoParam.m_nCurAlignID, vecAlignResult, dAngle, algoPatternDiff.ptMatchingCenter);
	ptOffset.x = 0; ptOffset.y = 0;
	algoPatternDiff.m_dTheta = dAngle;
	InspPartInfo* pInspBoardInfo = g_pInspMng->GetInspPartInfo();

	memset(&rstAlgo, 0, sizeof(PIAL::_RstAlgoPatternDiff));

	//일반각 고려
	if ((int)pImg_buf->inspPartImage->m_Angle % 90 != 0)pImg_buf->inspPartImage->m_Angle = 0;

	//Algorithm Polygon Mask

	////Mat
	cv::Mat LayerMask;
	BOOL bMaskLoad = FALSE;
	cv::Mat tmp(pImg_buf->m_p2D->Length(), pImg_buf->m_p2D->Width(), CV_8UC1);
	tmp.setTo(255);
	cv::Mat AlgoPolyMask;// = dstImg->Mat();

	BOOL useGrouping = g_pInspMng->m_ExtProdInfo->bPatternDiffGrouping;
	double Maxsize = g_pInspMng->m_ExtProdInfo->dPatternDiffMaxSize;
	double Distance = g_pInspMng->m_ExtProdInfo->dPatternDiffDistance;
	BOOL bUseAxisSize = g_pInspMng->m_ExtProdInfo->bPatternDiffUseAxisSize;

	if (algoParam.m_nInspType == 0)	//auto mode Multi Don't Modify
	{
		CString sPath;
		sPath.Format(_T("%s"), algoPatternDiff.m_sModelPath);
		bMaskLoad = GetLayerMask(sPath, LayerMask, pImg_buf->inspPartImage->m_Angle);



		//Get Success model Index, Before inspection 
		int nModelIndex = GetLastOKIdx(algoPatternDiff.m_sModelPath);
		algoPatternDiff.nSelectedModel = nModelIndex > _MAX_MODELCNT ? algoPatternDiff.nSelectedModel : nModelIndex;


	m_stModels.dwPDI =(QWORD)&m_MapModel;
	m_stModels.dwPPD = (QWORD)&m_MapModelExc;
    //Auto Inspection (ONLY)
	bResult = InspWrapper->m_PInspAlgo->InspPatternDiff(algoPatternDiff, *pImg_buf, &rstAlgo, PIALTieArea, ptOffset, 0, LayerMask, (void *)&m_stModels, AlgoPolyMask, bMaskLoad, dstImg,  useGrouping, Maxsize, Distance, bUseAxisSize);
	}
	else
	{
		bResult = InspWrapper->m_PInspAlgo->InspPatternDiff(algoPatternDiff, *pImg_buf, &rstAlgo, PIALTieArea, ptOffset, 1, LayerMask, NULL, AlgoPolyMask, bMaskLoad, dstImg, useGrouping, Maxsize, Distance, bUseAxisSize);
	}
	//결과 이물 데이터를 파트 픽셀 좌표계로 변환
	for (int i = 0; i < rstAlgo.m_nRectCnt; ++i)
	{
		rstAlgo.m_ptArrRstPtr[i][0].x += algoParam.m_dx;
		rstAlgo.m_ptArrRstPtr[i][0].y += algoParam.m_dy;
		rstAlgo.m_ptArrRstPtr[i][1].x += algoParam.m_dx;
		rstAlgo.m_ptArrRstPtr[i][1].y += algoParam.m_dy;
		rstAlgo.m_ptArrRstPtr[i][2].x += algoParam.m_dx;
		rstAlgo.m_ptArrRstPtr[i][2].y += algoParam.m_dy;
		rstAlgo.m_ptArrRstPtr[i][3].x += algoParam.m_dx;
		rstAlgo.m_ptArrRstPtr[i][3].y += algoParam.m_dy;
	}
	InspWrapper->ConvertRstAlgo(rstAlgo, (RstAlgoPatternDiff*)sRstAlgo);

	//Save Last OK model index
	if (algoParam.m_nInspType == 0 && bResult == TRUE)
		SetOKModelIdx(algoPatternDiff.m_sModelPath, rstAlgo.m_nSelectedModelIdx);

	//Normal Result & Chipping Result
	bResult = bResult == TRUE && rstAlgo.m_bRstChipping == TRUE ? TRUE : FALSE;

	return bResult;
}

BOOL CPInsp_AlgoPatternDiff::SetAlignRes(const InspAlgo &sInspAlgo, int inspType, void *ptrInspAlgoParam, void* sRstAlgo, AlignResult * pAlignRes, bool bStdAngle)
{
	//★★ 코드 수정 필요
	BOOL bRet = FALSE;

	if ((inspType == eINSP_MOUNT || inspType == eINSP_ALIGN))
	{
		AlgoPatternDiff *pAlgoPatternDiff = (AlgoPatternDiff *)sInspAlgo.m_ptrInspAlgoParam;
	RstAlgoPatternDiff * rst = (RstAlgoPatternDiff *)sRstAlgo;
		pAlignRes->offsetX = -rst->m_ptRstShiftXY.x;
		pAlignRes->offsetY = rst->m_ptRstShiftXY.y;
		pAlignRes->theta = rst->m_dTheta;
		pAlignRes->TeachCenterX = rst->m_ptRstMatchingCenter.x;
		pAlignRes->TeachCenterY = rst->m_ptRstMatchingCenter.y;
		bRet = TRUE;
	}
	bRet = TRUE;

	return bRet;
}
int CPInsp_AlgoPatternDiff::UseColorImage(const InspAlgo &sInspAlgo, int nIndex)
{
	int nData = COLOR_DATA_Non;

	return nData;
}
bool CPInsp_AlgoPatternDiff::SetAIImage(InspPartInfo *pInspBoardInfo, int inspType)
{
	return false;
}
BOOL CPInsp_AlgoPatternDiff::SaveModel(cv::Mat ModelImg, CString sPath, RECT rcMatchingArea, int nMultiAlignCnt, RECT* rtArrAlignArea)
{
	BOOL bRet = FALSE;

	CString extension = sPath.Mid(sPath.GetLength() - 4, 4).MakeLower();
	if (extension != _T(".pdi"))
		return bRet;

	CFile file;
	CFileException fe;
	if (file.Open(sPath, CFile::modeCreate | CFile::modeWrite, NULL, &fe) == FALSE)
		return bRet;
	CArchive ar(&file, CArchive::store);

	bRet = SaveModelFile(ar, ModelImg, rcMatchingArea, nMultiAlignCnt, rtArrAlignArea);

	if (bRet) 
	{
		CString strSave;
		strSave.Format(_T("%s"), sPath);
		strSave.Replace(_T(".pdi"), _T(".bmp"));
		cv::imwrite(std::string(CT2A(strSave)), ModelImg);
	}

	ar.Close();
	file.Close();

	return TRUE;
}
BOOL CPInsp_AlgoPatternDiff::SaveModelFile(CArchive& ar, cv::Mat ModelImg, RECT rcMatchingArea, int nMultiAlignCnt, RECT* rtArrAlignArea)
{
	if (ModelImg.data == NULL || ModelImg.rows <= 0 || ModelImg.cols <= 0)
		return FALSE;

	CString sName;
	if (nMultiAlignCnt >= 2 && rtArrAlignArea != nullptr)
		sName.Format(_T("pemtron_MultiAlign"));
	else
		sName.Format(_T("pemtron"));
	//1. 구분자
	ar << sName;

	//2. 이미지 크기
	ar << ModelImg.rows;
	ar << ModelImg.cols;

	//3. 데이터
	for (int y = 0; y < ModelImg.rows; y++)
	{
		uchar * ptr = ModelImg.ptr(y);
		ar.Write(ptr, ModelImg.cols);
	}
	
	//4. Resolution
	ar << m_resolX;
	ar << m_resolY;

	BOOL bUseMatchingArea = FALSE;
	//5. Matching ModelData
	if ((rcMatchingArea.right - rcMatchingArea.left) > 0 || (rcMatchingArea.bottom - rcMatchingArea.top) > 0)
	{

		GeoModel gModel_Master;

		cv::Rect rtMatchingArea = { rcMatchingArea.left,  rcMatchingArea.top,
			(rcMatchingArea.right + 1) - rcMatchingArea.left , (rcMatchingArea.bottom + 1) - rcMatchingArea.top };

		gModel_Master.Alloc(ModelImg(rtMatchingArea));
		gModel_Master.setSpeed(GeoModel::enmSpeed::spHigh);
		gModel_Master.setAccuracy(GeoModel::enmAccuracy::acHigh);
		gModel_Master.setAngleRange(false, 5, 5, 0.5);
		gModel_Master.setMatchAlgo(GeoModel::agDefault);
		gModel_Master.Preprocess();

		bUseMatchingArea = TRUE;
		
		ar << bUseMatchingArea;
		ar << rtMatchingArea.x;
		ar << rtMatchingArea.y;
		ar << rtMatchingArea.width;
		ar << rtMatchingArea.height;

		gModel_Master.SaveFile(ar, false, 11);
	}
	else
		ar << bUseMatchingArea;

	BOOL bUseMultiAlign = nMultiAlignCnt >= 2 ? TRUE : FALSE;
	ar << bUseMultiAlign;
	ar << nMultiAlignCnt;

	if (bUseMultiAlign == TRUE)
	{
		for (int i = 0; i < nMultiAlignCnt; i++)
		{
			GeoModel gModel_Master;

			if (rtArrAlignArea[i].left < 0)rtArrAlignArea[i].left = 0;
			if (rtArrAlignArea[i].top < 0)rtArrAlignArea[i].top = 0;
			if (rtArrAlignArea[i].right > ModelImg.cols)rtArrAlignArea[i].right = ModelImg.cols - 1;
			if (rtArrAlignArea[i].bottom > ModelImg.rows)rtArrAlignArea[i].bottom = ModelImg.rows - 1;

			cv::Rect rtMatchingArea = { rtArrAlignArea[i].left,  rtArrAlignArea[i].top,
				(rtArrAlignArea[i].right + 1) - rtArrAlignArea[i].left , (rtArrAlignArea[i].bottom + 1) - rtArrAlignArea[i].top };

			if (rtMatchingArea.width > 20 && rtMatchingArea.height > 20)
			{
				gModel_Master.Alloc(ModelImg(rtMatchingArea));
				gModel_Master.setSpeed(GeoModel::enmSpeed::spHigh);
				gModel_Master.setAccuracy(GeoModel::enmAccuracy::acHigh);
				gModel_Master.setAngleRange(false, 5, 5, 0.5);
				gModel_Master.setMatchAlgo(GeoModel::agDefault);
				gModel_Master.Preprocess();

				ar << rtMatchingArea.x;
				ar << rtMatchingArea.y;
				ar << rtMatchingArea.width;
				ar << rtMatchingArea.height;

				gModel_Master.SaveFile(ar, false, 11);
			}
		}
	}
	return TRUE;
}
BOOL CPInsp_AlgoPatternDiff::LoadModel(CString sPath, BOOL bCreateImgFile, BOOL bAutoInspLoad, BOOL bUseMultiAlign)
{
	CString extension = sPath.Mid(sPath.GetLength() - 4, 4).MakeLower();
	if (extension != _T(".pdi"))
		return false;
	CFile file;
	CFileException fe;
	if (file.Open(sPath, CFile::modeRead, NULL, &fe) == FALSE)
		return false;

	if (bAutoInspLoad == TRUE && g_pMPTI->GetUseMultiProcess())
		ext::InspRoot_Server::get()->ExtModelAdd(&file, sPath, ext::ModelBufferFlag::eExtModel_COB);

	CArchive ar(&file, CArchive::load, file.GetLength());

	BOOL bSuccess = LoadModelFile(ar, m_resolX, m_resolY);

	if (bSuccess && bCreateImgFile)
	{
		CString str;
		str.Format(_T("%s"), sPath);
		str.Replace(_T(".pdi"), _T(".bmp"));
		cv::imwrite(std::string(CT2A(str)), m_stModel.Img2D[0]);
	}

	ar.Close();
	file.Close();

	if (bAutoInspLoad == TRUE && !g_pMPTI->GetUseMultiProcess())
	{
		if (bSuccess)
		{
			stPatternDiffModel* stListModel = new stPatternDiffModel();
			if (bUseMultiAlign == FALSE)
			stListModel->sName.Format(_T("pemtron"));
			else
				stListModel->sName.Format(_T("pemtron_MultiAlign"));
			stListModel->nWidth = m_stModel.nWidth;
			stListModel->nHeight = m_stModel.nHeight;

			cv::Mat img2D = m_stModel.Img2D[0].clone();
			cv::Mat img2D_90 = m_stModel.Img2D[1].clone();
			cv::Mat img2D_180 = m_stModel.Img2D[2].clone();
			cv::Mat img2D_270 = m_stModel.Img2D[3].clone();

			stListModel->Img2D[0] = img2D.clone();
			stListModel->Img2D[1] = img2D_90.clone();
			stListModel->Img2D[2] = img2D_180.clone();
			stListModel->Img2D[3] = img2D_270.clone();
			stListModel->dResolX = m_stModel.dResolX;
			stListModel->dResolY = m_stModel.dResolY;

			stListModel->bUseMatching = m_stModel.bUseMatching;
			stListModel->nStx = m_stModel.nStx;
			stListModel->nSty = m_stModel.nSty;
			stListModel->nAreaWidth = m_stModel.nAreaWidth;
			stListModel->nAreaHeight = m_stModel.nAreaHeight;

			//Copy GeoModel
			stListModel->geoMaster.setMatchAlgo(m_stModel.geoMaster.getMatchAlgo());
			stListModel->geoMaster.CopyOf(m_stModel.geoMaster, 0);

			stListModel->geoMaster_90.setMatchAlgo(m_stModel.geoMaster.getMatchAlgo());
			stListModel->geoMaster_90.CopyOf(m_stModel.geoMaster, 90.0F);

			stListModel->geoMaster_180.setMatchAlgo(m_stModel.geoMaster.getMatchAlgo());
			stListModel->geoMaster_180.CopyOf(m_stModel.geoMaster, 180.0F);

			stListModel->geoMaster_270.setMatchAlgo(m_stModel.geoMaster.getMatchAlgo());
			stListModel->geoMaster_270.CopyOf(m_stModel.geoMaster, 270.0F);

			if (bUseMultiAlign == TRUE)
			{
				if (m_stModel.bUseMultiAlign == FALSE)
				{
					stListModel->bUseMultiAlign = FALSE;
				}
				else
				{
					stListModel->bUseMultiAlign = TRUE;
					stListModel->nMultiAlignCnt = m_stModel.nMultiAlignCnt;
					for (int i = 0; i < m_stModel.nMultiAlignCnt; i++)
					{
						stListModel->stAlignArea[i].stX = m_stModel.stAlignArea[i].stX;
						stListModel->stAlignArea[i].stY = m_stModel.stAlignArea[i].stY;
						stListModel->stAlignArea[i].nAreaWidth = m_stModel.stAlignArea[i].nAreaWidth;
						stListModel->stAlignArea[i].nAreaLength = m_stModel.stAlignArea[i].nAreaLength;
						stListModel->stAlignArea[i].geoMaster.setMatchAlgo(m_stModel.stAlignArea[i].geoMaster.getMatchAlgo());
						stListModel->stAlignArea[i].geoMaster.CopyOf(m_stModel.stAlignArea[i].geoMaster, 0);
					}
				}
			}
			else
			{
				stListModel->bUseMultiAlign = FALSE;
				stListModel->nMultiAlignCnt = 0;
			}
			m_MapModel.insert(std::make_pair(sPath, stListModel));
		}


	}
	return bSuccess;
}
BOOL CPInsp_AlgoPatternDiff::LoadExtModel(ext::ModelStatus pCtrl,BYTE * Data,BOOL bCreateImgFile)
{
	//ext::ModelStatus pCtrl = ext::InspRoot_Server::get()->_PatternModelCtrl.First()[nIndex];
	CString sPath(pCtrl.sFilePath);
	void  *Model;
	CString extension = sPath.Mid(sPath.GetLength() - 4, 4).MakeLower();
	if ((extension == _T(".pdi") || extension == _T(".ppd")))
		Model = (extension == _T(".pdi")) ? &m_MapModel : &m_MapModelExc;
	else
		return false;
	

	
	//CFile file;
	//CFileException fe;
	//if (file.Open(sPath, CFile::modeRead, NULL, &fe) == FALSE)
	//	return false;
	CMemFile file;
	CArchive arStore(&file, CArchive::store); // 공유메모리에서 가져와 셋팅.
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
BOOL CPInsp_AlgoPatternDiff::LoadExtModelSetting(CString sPath , void * Model , CArchive& ar, double dResolX, double dResolY)
{
	CString sName(_T("pemtron")), sReadName;
	std::map<CString, PatternDiffModel *> * mpModelData; 
	mpModelData = static_cast<std::map<CString, PatternDiffModel *> *> (Model);

	int nModelType = -1; 
	CString extension = sPath.Mid(sPath.GetLength() - 4, 4).MakeLower();
	if ((extension == _T(".pdi") || extension == _T(".ppd")))
		nModelType = (extension == _T(".pdi")) ? 1 : 2; // 1 Pattern Diff , PatternExc

	if (nModelType == -1)
	{

		return FALSE;
	}
		
	PatternDiffModel * stModel =  new PatternDiffModel();
	stModel->ModelClear();

	switch (nModelType)
	{
	case 1:
	{
		//1. 구분자
		ar >> sReadName;
		if (sReadName.Find(sName) == -1)
			return FALSE;
		else
			m_stModel.sName = sReadName;

		//2. 사이즈 
		int nWidth(0), nHeight(0);
		ar >> nHeight;
		ar >> nWidth;

		//3. 이미지 데이터
		if (nWidth > 0 && nHeight > 0)
		{
			stModel->ModelClear();
			stModel->sName = sReadName;
			stModel->nWidth = nWidth;
			stModel->nHeight = nHeight;

			stModel->Img2D[0] = cv::Mat(nHeight, nWidth, CV_8UC1);
			for (int y = 0; y < nHeight; y++)
			{
				uchar * ptr = stModel->Img2D[0].ptr(y);
				ar.Read(ptr, stModel->Img2D[0].cols);
			}

			//4 Direction ImageCreate
			int nRotWidth, nRotHeight;

			//Rotate
		m_stModel.Img2D[1] = cv::Mat(nWidth, nHeight, CV_8UC1);	//90
		m_stModel.Img2D[2] = cv::Mat(nHeight, nWidth, CV_8UC1);	//180
		m_stModel.Img2D[3] = cv::Mat(nWidth, nHeight, CV_8UC1);	//270

			for (int a = 1; a <= 3; a++)
			{
			g_pInspMng->m_procMil->RotateImg_ipp2(m_stModel.Img2D[0].data, &m_stModel.Img2D[a].data, nWidth, nHeight, a * 90, &nRotWidth, &nRotHeight);
			}
		}

		//4. Resolution
		double dModelResolX, dModelResolY;
		ar >> dModelResolX;
		ar >> dModelResolY;

		stModel->dResolX = dModelResolX;
		stModel->dResolY = dModelResolY;

	ReSizeModel(dModelResolX, dModelResolY);
		//5. 매칭 정보
		BOOL bUseMatchingArea = FALSE;
		ar >> stModel->bUseMatching;

		if (stModel->bUseMatching)
		{
			ar >> stModel->nStx;
			ar >> stModel->nSty;
			ar >> stModel->nAreaWidth;
			ar >> stModel->nAreaHeight;

			stModel->geoMaster.LoadFile(ar, m_resolX, m_resolY);

			stModel->geoMaster.setMatchAlgo(stModel->geoMaster.getMatchAlgo());
			stModel->geoMaster.CopyOf(stModel->geoMaster, 0);
				   
			stModel->geoMaster_90.setMatchAlgo(stModel->geoMaster.getMatchAlgo());
			stModel->geoMaster_90.CopyOf(stModel->geoMaster, 90.0F);
				   
			stModel->geoMaster_180.setMatchAlgo(stModel->geoMaster.getMatchAlgo());
			stModel->geoMaster_180.CopyOf(stModel->geoMaster, 180.0F);
				   
			stModel->geoMaster_270.setMatchAlgo(stModel->geoMaster.getMatchAlgo());
			stModel->geoMaster_270.CopyOf(stModel->geoMaster, 270.0F);
		}
		bool bUseMultiAlign = false;
		if (m_stModel.sName.Find(_T("MultiAlign")) != -1)
			bUseMultiAlign = true;
		if (bUseMultiAlign == true)
		{
			ar >> m_stModel.bUseMultiAlign;
			if (m_stModel.bUseMultiAlign == FALSE)
			{
				m_stModel.nMultiAlignCnt = 0;
			}
			else
			{
				ar >> m_stModel.nMultiAlignCnt;

				for (int i = 0; i < m_stModel.nMultiAlignCnt; i++)
				{
					ar >> m_stModel.stAlignArea[i].stX;
					ar >> m_stModel.stAlignArea[i].stY;
					ar >> m_stModel.stAlignArea[i].nAreaWidth;
					ar >> m_stModel.stAlignArea[i].nAreaLength;

					m_stModel.stAlignArea[i].geoMaster.LoadFile(ar, m_resolX, m_resolY);
				}
			}
		}
		else
		{
			m_stModel.bUseMultiAlign = FALSE;
			m_stModel.nMultiAlignCnt = 0;
		}

	} break;

	case 2: 
	{
		 // PatternDiffExc
		{
			stModel->geoMaster.LoadFile(ar, m_resolX, m_resolY);
			stModel->geoMaster.Preprocess();
			// 각도별 process 필요
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
	} break;
	}
		
	

	
	mpModelData->insert(std::make_pair(sPath, stModel));

	return TRUE;
}

BOOL CPInsp_AlgoPatternDiff::LoadModelFile(CArchive& ar, double dResolX, double dResolY)
{
	CString sName(_T("pemtron")), sReadName;

	//1. 구분자
	ar >> sReadName;
	if (sReadName.Find(sName) == -1)
		return FALSE;
	else
		m_stModel.sName = sReadName;


	//2. 사이즈 
	int nWidth(0), nHeight(0);
	ar >> nHeight;
	ar >> nWidth;

	//3. 이미지 데이터
	if (nWidth > 0 && nHeight > 0)
	{
		m_stModel.ModelClear();
		m_stModel.sName = sReadName;
		m_stModel.nWidth = nWidth;
		m_stModel.nHeight = nHeight;

		m_stModel.Img2D[0] = cv::Mat(nHeight, nWidth, CV_8UC1);
		for (int y = 0; y < nHeight; y++)
		{
			uchar * ptr = m_stModel.Img2D[0].ptr(y);
			ar.Read(ptr, m_stModel.Img2D[0].cols);
		}

		//4 Direction ImageCreate
		int nRotWidth, nRotHeight;

		//Rotate
		m_stModel.Img2D[1] = cv::Mat(nWidth, nHeight, CV_8UC1);	//90
		m_stModel.Img2D[2] = cv::Mat(nHeight, nWidth, CV_8UC1);	//180
		m_stModel.Img2D[3] = cv::Mat(nWidth, nHeight, CV_8UC1);	//270

		for (int a = 1; a <= 3; a++)
		{
			g_pInspMng->m_procMil->RotateImg_ipp2(m_stModel.Img2D[0].data, &m_stModel.Img2D[a].data, nWidth, nHeight, a * 90, &nRotWidth, &nRotHeight);
		}
	}

	//4. Resolution
	double dModelResolX, dModelResolY;
	ar >> dModelResolX;
	ar >> dModelResolY;

	m_stModel.dResolX = dModelResolX;
	m_stModel.dResolY = dModelResolY;

	ReSizeModel(dModelResolX, dModelResolY);

	//5. 매칭 정보
	BOOL bUseMatchingArea = FALSE;
	ar >> m_stModel.bUseMatching;

	if (m_stModel.bUseMatching == TRUE)
	{
		ar >> m_stModel.nStx;
		ar >> m_stModel.nSty;
		ar >> m_stModel.nAreaWidth;
		ar >> m_stModel.nAreaHeight;

		LoadMatchInfo(ar, dModelResolX, dModelResolY);
	}

	bool bUseMultiAlign = false;
	if (m_stModel.sName.Find(_T("MultiAlign")) != -1)
		bUseMultiAlign = true;

	//6. MultiAlign Area
	if (bUseMultiAlign == true)
	{
		ar >> m_stModel.bUseMultiAlign;
		if (m_stModel.bUseMultiAlign != TRUE)
		{
			m_stModel.nMultiAlignCnt = 0;
		}
		else
		{
			ar >> m_stModel.nMultiAlignCnt;

			for (int i = 0; i < m_stModel.nMultiAlignCnt; i++)
			{
				ar >> m_stModel.stAlignArea[i].stX;
				ar >> m_stModel.stAlignArea[i].stY;
				ar >> m_stModel.stAlignArea[i].nAreaWidth;
				ar >> m_stModel.stAlignArea[i].nAreaLength;

				m_stModel.stAlignArea[i].geoMaster.LoadFile(ar, m_resolX, m_resolY);
			}
		}
	}
	else
	{
		m_stModel.bUseMultiAlign = FALSE;
		m_stModel.nMultiAlignCnt = 0;
	}

	return TRUE;
}
void CPInsp_AlgoPatternDiff::ReSizeModel(double dResolX, double dResolY)
{
	if (dResolX == m_resolX && dResolY == m_resolY)
		return;
	if (dResolX == 0 || m_resolX == 0 || dResolY == 0 || m_resolY == 0)
		return;

	int nSizeX = (int)(m_stModel.Img2D[0].cols * m_resolX / dResolX);
	int nSizeY = (int)(m_stModel.Img2D[0].rows * m_resolY / dResolY);

	if (nSizeX == m_stModel.Img2D[0].cols && nSizeY == m_stModel.Img2D[0].rows)
		return;
	cv::resize(m_stModel.Img2D[0], m_stModel.Img2D[0], cv::Size(nSizeX, nSizeY));
}
POINTF CPInsp_AlgoPatternDiff::SelectOffset(int nTargetAlignID, std::vector<PIAL::_AlignResult> vecAlignResult, double& dAlignTheta, POINTF& ptMatchingCenter)
{
	POINTF ptRet;
	ptRet.x = 0;
	ptRet.y = 0;

	if (vecAlignResult.size() <= 0)
		return ptRet;

	int nBodysizeX = 0;
	int nBodysizeY = 0;

	//Teaching시 선택한 AlignWindow의 Result 받아오기
	for (int i = 0; i < vecAlignResult.size(); i++)
	{
		if (vecAlignResult[i].nWindowID == nTargetAlignID)
		{
			ptRet.x = vecAlignResult[i].offsetX;
			ptRet.y = vecAlignResult[i].offsetY;
			nBodysizeX = vecAlignResult[i].rcBodyRect.Width();
			nBodysizeY = vecAlignResult[i].rcBodyRect.Height();
			dAlignTheta = vecAlignResult[i].theta;
			ptMatchingCenter.x = vecAlignResult[i].TeachCenterX;
			ptMatchingCenter.y = vecAlignResult[i].TeachCenterY;
			continue;
		}
	}

	return ptRet;
}
short CPInsp_AlgoPatternDiff::LabelValue(int nLabelIdx)
{
	short sRet = 0x000;

	switch (nLabelIdx)
	{
	case 9: sRet = 0x0001; break;
	case 8: sRet = 0x0002; break;
	case 7: sRet = 0x0004; break;
	case 6: sRet = 0x0008; break;
	case 5: sRet = 0x0010; break;
	case 4: sRet = 0x0020; break;
	case 3: sRet = 0x0040; break;
	case 2: sRet = 0x0080; break;
	case 1: sRet = 0x0100; break;
	case 0: sRet = 0x0200; break;
	}
	return sRet;
}
void CPInsp_AlgoPatternDiff::ClearAutoInspParam(int nCurLane)
{
	int nSize = m_vLayerMask[nCurLane].size();

	if (m_vLayerMask[nCurLane].size() <= 0)
		return;

	for (auto iter = m_vLayerMask[nCurLane].begin(); iter != m_vLayerMask[nCurLane].end(); iter++)
	{
		if (iter->second.data != nullptr)
			iter->second.release();
	}

	m_vLayerMask[nCurLane].clear();

	//공유 메모리 모델 clear
	if (g_pMPTI->GetUseMultiProcess())
	{
		//공유 메모리 모델 clear
		if (g_pMPTI->GetUseMultiProcess())
			ext::irs::get()->ModelClear(ext::ModelBufferFlag::eExtModel_COB);
	}
	else
	{
		MapClear();
	}

	//Init Last OK model Index
	ClearModelIdx(nCurLane);

}
void CPInsp_AlgoPatternDiff::AddLayerMask(CString sPath, cv::Mat& Mask, int nCurLane)
{
	/*sPath.Replace(_T(".pdi"), _T("_LayerMask"));
	m_vLayerMask[nCurLane].insert({ sPath, Mask });*/

	//Create 4 Direction Mask (0, 90, 180, 270)
	sPath.Replace(_T(".pdi"), _T("_LayerMask"));
	for (int i = 0; i <= 270; i += 90)
	{
		CString sName;
		sName.Format(_T("%s_%d"), sPath, i);

		cv::Mat RotMask;
		switch (i)
		{
		case 0:
			RotMask = Mask.clone();
			break;
		case 90:
			cv::rotate(Mask, RotMask, cv::ROTATE_90_COUNTERCLOCKWISE);
			break;
		case 180:
			cv::rotate(Mask, RotMask, cv::ROTATE_180);
			break;
		case 270:
			cv::rotate(Mask, RotMask, cv::ROTATE_90_CLOCKWISE);
			break;
		}

#if _DEBUG
		cv::Mat testimg;
		RotMask.convertTo(testimg, CV_8UC1);
		cv::threshold(testimg, testimg, 0, 255, cv::THRESH_BINARY);
#endif

		m_vLayerMask[nCurLane].insert({ sName, RotMask });

		if (g_pMPTI->GetUseMultiProcess())
			ext::irs::get()->ExtMatImageAdd(&RotMask, sName, ext::ModelBufferFlag::eExtModel_COB);
	}



}
void CPInsp_AlgoPatternDiff::SetLayerMask(const InspAlgo& InspAlgo, cv::Mat& DstMask, int nWidth, int nHeight)
{
	BOOL bRet = FALSE;
	AlgoPatternDiff* pAlgoPatternDiff = (AlgoPatternDiff*)InspAlgo.m_ptrInspAlgoParam;
	int nTotalLayerCnt = pAlgoPatternDiff->m_nLayerCnt;

	std::vector<std::pair<int, cv::Mat>> vArrLayerMask;
	for (int i = 0; i < nTotalLayerCnt; i++)
	{
		if (pAlgoPatternDiff->m_nArrLayerType[i] == (int)_m_enPatternDiffLayerType::eChipping)
			continue;

		cv::Mat tmpLayerImg(DstMask.size(), CV_8UC1);
		ippsZero_8u(tmpLayerImg.data, nWidth * nHeight);
		std::vector<std::vector<cv::Point> > vArrPolyPoint;
		for (int j = 0; j < pAlgoPatternDiff->m_stPolygon[i].m_nAreaCnt; j++)
		{
			int nPolyCnt = pAlgoPatternDiff->m_stPolygon[i].m_nPoly[j].m_nPtrcnt;
			std::vector<cv::Point> vTmpPtr;
			for (int a = 0; a < nPolyCnt; a++)
			{
				cv::Point pt;
				pt.x = pAlgoPatternDiff->m_stPolygon[i].m_nPoly[j].m_dArrPtr[a].x;
				pt.y = pAlgoPatternDiff->m_stPolygon[i].m_nPoly[j].m_dArrPtr[a].y;
				if (pt.x < 0) pt.x = 0;
				if (pt.y < 0) pt.y = 0;
				if (pt.x > nWidth) pt.x = nWidth - 1;
				if (pt.y > nHeight) pt.y = nHeight - 1;
				vTmpPtr.push_back(pt);
			}
			vArrPolyPoint.push_back(vTmpPtr);
		}

		//Draw Each Layer Mat
		for (int cnt = 0; cnt < vArrPolyPoint.size(); cnt++)
		{
			cv::fillPoly(tmpLayerImg, vArrPolyPoint[cnt], cv::Scalar(255, 255, 255));
		}

		vArrLayerMask.push_back(std::make_pair(i, tmpLayerImg));
	}

	for (int a = 0; a < vArrLayerMask.size(); a++)
	{
		short sLayerValue = LabelValue(vArrLayerMask[a].first);

		for (int y = 0; y < nHeight; y++)
		{
			short* ptr_Mask = DstMask.ptr<short>(y);
			uchar* ptr_tmpMap = vArrLayerMask[a].second.ptr(y);
			for (int x = 0; x < nWidth; x++)
			{
				if (ptr_tmpMap[x] == 255)
					ptr_Mask[x] |= sLayerValue;
			}
		}
	}
}
BOOL CPInsp_AlgoPatternDiff::IsPointInPolygon(POINT p, POINTF* PointList, int size, int nImgWidth, int nImgHeight, int dX, int dY)
{
	int sides = size;
	int j = sides - 1;
	bool pointStatus = false;
	double ix = p.x;
	double iy = p.y;
	for (int i = 0; i < sides; i++)
	{
		POINT ptPolyCurrent, ptPolyBefore;
		ptPolyCurrent.x = ((PointList[i].x - dX));
		ptPolyCurrent.y = ((PointList[i].y - dY));

		ptPolyBefore.x = ((PointList[j].x - dX));
		ptPolyBefore.y = ((PointList[j].y - dY));
		if (ptPolyCurrent.y < iy && ptPolyBefore.y >= iy || ptPolyBefore.y < iy && ptPolyCurrent.y >= iy)
		{
			if (ptPolyCurrent.x + (iy - ptPolyCurrent.y) / (ptPolyBefore.y - ptPolyCurrent.y) * (ptPolyBefore.x - ptPolyCurrent.x) < ix)
			{
				pointStatus = !pointStatus;
			}
		}
		j = i;
	}
	return pointStatus;
}
BOOL CPInsp_AlgoPatternDiff::GetLayerMask(CString sPath, cv::Mat& Mask, double dAngle)
{
	BOOL bRet = FALSE;
	int nr = sPath.Find(_T("TEMP_REAR"));
	int nCurLane = 0;	//Front
	if (nr > 0)
		nCurLane = 1;	//Rear

	sPath.Replace(_T(".pdi"), _T("_LayerMask"));
	CString sName;
	sName.Format(_T("%s_%d"), sPath, (int)dAngle);
	if (g_pMPTI->GetUseMultiProcess())
	{
		ext::InspRoot_Server::get()->GetExtMatImage(&Mask, sName);
		if (Mask.data != nullptr && Mask.rows > 0 && Mask.cols > 0)
			bRet = TRUE;
	}
	else
	{
		auto item = m_vLayerMask[nCurLane].find(sName);
		if (item != m_vLayerMask[nCurLane].end())
		{
			Mask = item->second;
			bRet = TRUE;
		}
		else
		{
			bRet = FALSE;
		}
	}

#if _DEBUG
	cv::Mat testimg;
	Mask.convertTo(testimg, CV_8UC1);
	cv::threshold(testimg, testimg, 0, 255, cv::THRESH_BINARY);
#endif

	return bRet;

}
BOOL CPInsp_AlgoPatternDiff::LoadMatchInfo(CArchive &ar, double m_resolX, double m_resolY)
{
	m_stModel.geoMaster.LoadFile(ar, m_resolX, m_resolY);

	return TRUE;
}
void CPInsp_AlgoPatternDiff::SetMatchingArea(CString sPath, RECT rcMatchingArea, int nMultiAlignCnt, RECT* rtArrAlignArea)
{
	LoadModel(sPath, FALSE, FALSE);
	cv::Mat OrgModel = m_stModel.Img2D[0].clone();
	BOOL bRet = SaveModel(OrgModel, sPath, rcMatchingArea, nMultiAlignCnt, rtArrAlignArea);
}
BOOL CPInsp_AlgoPatternDiff::MatchNewModel(cv::Mat& SrcImg, CString sFstModelPath, RECT rcMatchingArea, POINT& ptRst)
{
	ptRst.x = 0;
	ptRst.y = 0;

	//현재 영상 안드러오거나 첫번재 모델Path가 없는경우
	if (SrcImg.data == nullptr || SrcImg.rows <= 0 || SrcImg.cols <= 0 || sFstModelPath.IsEmpty())
		return FALSE;

	//첫번째 모델 load
	BOOL bLoad = LoadModel(sFstModelPath, FALSE, FALSE);

	if (bLoad == FALSE)
		return FALSE;

	cv::Rect rtMatchingArea = { rcMatchingArea.left,  rcMatchingArea.top,(rcMatchingArea.right + 1) - rcMatchingArea.left , (rcMatchingArea.bottom + 1) - rcMatchingArea.top };

	GeoResult result; result.Alloc(1);
	GeoMatch gMatch;
	cv::Mat InputImg = SrcImg(rtMatchingArea);
	cv::Mat MasterImg = m_stModel.Img2D[0](rtMatchingArea);
	GeoModel gGeoMaster = m_stModel.geoMaster;
	if (!g_pMPTI->GetUseMultiProcess())
	{
		gGeoMaster.Alloc(MasterImg);
		gGeoMaster.setSpeed(GeoModel::enmSpeed::spHigh);
		gGeoMaster.setAccuracy(GeoModel::enmAccuracy::acHigh);
		gGeoMaster.setAngleRange(false, 5, 5, 0.5);
		gGeoMaster.setMatchAlgo(GeoModel::agDefault);
		gGeoMaster.RePreprocess();
	}
	std::shared_ptr<ImgProcessing> ImgProc = std::make_shared<ImgProcessing>();
	ImgProc->_ImgProcess(InputImg, gGeoMaster);
	gMatch.SetImgProcess(ImgProc);
	gMatch.FindModel(gGeoMaster, result, false);

#if _DEBUG
	//ForDebuging
	cv::Mat DebugImg = InputImg.clone();
	cv::cvtColor(DebugImg, DebugImg, cv::COLOR_GRAY2BGR);
	gMatch.DrawContours(DebugImg, gGeoMaster, result, cv::Scalar(255, 0, 0), 1);
#endif // _DEBUG

	//매칭률 낮으면 첫번째 모델과 Align이 맞지 않는 위치를 잡아줌.
	if (result._Equality[0] < 50.0)
		return FALSE;

	ptRst.x = (rtMatchingArea.width / 2) - (int)_RounD(result._Center_y[0]);
	ptRst.y = (rtMatchingArea.height / 2) - (int)_RounD(result._Center_x[0]);

	//상하좌우 4pix 추가 탐색
	cv::Rect rtImgMatch_Src(rtMatchingArea);
	rtImgMatch_Src.x += ptRst.x * -1.0;
	rtImgMatch_Src.y += ptRst.y * -1.0;

	if (rtImgMatch_Src.x >= 0 && rtImgMatch_Src.y >= 0 && rtImgMatch_Src.x + rtImgMatch_Src.width < SrcImg.cols - 1 && rtImgMatch_Src.y + rtImgMatch_Src.height < SrcImg.rows - 1)
	{
		cv::Rect rtImgMatch_Match(rtMatchingArea);
		int nSearchMargin = 4;
		rtImgMatch_Match.x += nSearchMargin;
		rtImgMatch_Match.y += nSearchMargin;
		rtImgMatch_Match.width -= nSearchMargin * 2;
		rtImgMatch_Match.height -= nSearchMargin * 2;

#if _DEBUG
		//ForDebuging
		cv::Mat SrcRoi = SrcImg(rtImgMatch_Src);
		cv::Mat MatchRoi = m_stModel.Img2D[0](rtImgMatch_Match);
#endif

		cv::Mat score_M;
		cv::Point	ptMin, ptMax;
		double		Min(0.0), Max(0.0);
		cv::matchTemplate(SrcImg(rtImgMatch_Src), m_stModel.Img2D[0](rtImgMatch_Match), score_M, cv::TM_CCOEFF_NORMED);
		cv::minMaxLoc(score_M, &Min, &Max, &ptMin, &ptMax);

		cv::Point ptMatch;
		ptMatch.x = ptMax.x - nSearchMargin;
		ptMatch.y = ptMax.y - nSearchMargin;

		ptRst.x -= ptMatch.x;
		ptRst.y -= ptMatch.y;

	}
	return TRUE;
}
void CPInsp_AlgoPatternDiff::ClearModelIdx(int nLane)
{ 
	if(m_vArrModelLastOKIdx[nLane].size() > 0)
		m_vArrModelLastOKIdx[nLane].clear();
}
void CPInsp_AlgoPatternDiff::SetOKModelIdx(CString sModelKey, int nSuccessIdx)
{
	//data check
	if (sModelKey == _T(""))
		return;

	//lane check
	int nr = sModelKey.Find(_T("TEMP_REAR"));
	int nCurLane = 0;	//Front
	if (nr > 0)
		nCurLane = 1;	//Rear

	sModelKey.Replace(_T(".pdi"), _T(""));
	int nIdx = sModelKey.ReverseFind('\\') + 1;
	CString strClip = sModelKey.Mid(nIdx, sModelKey.GetLength() - (nIdx));

	if (m_vArrModelLastOKIdx[nCurLane].size() > 0)
	{
		auto item = m_vArrModelLastOKIdx[nCurLane].find(strClip);
		if (item != m_vArrModelLastOKIdx[nCurLane].end())
			item->second = nSuccessIdx;
	}
	else
	{
		m_vArrModelLastOKIdx[nCurLane].insert(std::make_pair(strClip, nSuccessIdx));
	}

	//log
	//CString sLog;
	//sLog.Format(_T("[PatternDiff]%s, Set Result Model Index = %d"), strClip, nSuccessIdx);
	//g_pMPTI->AddLog(sLog);
}
int CPInsp_AlgoPatternDiff::GetLastOKIdx(CString sCurKey)
{
	int nRstModelIdx = 0;

	if (sCurKey == _T(""))
		return nRstModelIdx;
	int nr = sCurKey.Find(_T("TEMP_REAR"));
	int nCurLane = 0;	//Front
	if (nr > 0)
		nCurLane = 1;	//Rear
	sCurKey.Replace(_T(".pdi"), _T(""));
	int nIdx = sCurKey.ReverseFind('\\') + 1;
	CString strClip = sCurKey.Mid(nIdx, sCurKey.GetLength() - (nIdx));

	//lane check
	//int nr = sModelKey.Find(_T("TEMP_REAR"));
	//if (nr > 0)
	//	nCurLane = 1;	//Rear

	auto iter = m_vArrModelLastOKIdx[nCurLane].find(strClip);
	if (iter != m_vArrModelLastOKIdx[nCurLane].end())
	{
		nRstModelIdx = iter->second;

		//log
		//CString sLog;
		//sLog.Format(_T("[PatternDiff]%s, Get Result Model Index = %d"), strClip, nRstModelIdx);
		//g_pMPTI->AddLog(sLog);
	}
	
	return nRstModelIdx;
}
void CPInsp_AlgoPatternDiff::LoadExceptModel(CString sPath, BOOL bAutoInspSeq)
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

	if(!g_pMPTI->GetUseMultiProcess() && bAutoInspSeq == TRUE)
	{
		PatternDiffModel* tempExcModel = new PatternDiffModel();
		tempExcModel->geoMaster.LoadFile(ar, dResolX, dResolY);

		ar.Close();
		file.Close();

		tempExcModel->geoMaster.Preprocess();
		// 각도별 process 필요
		tempExcModel->geoMaster_90.setMatchAlgo(tempExcModel->geoMaster.getMatchAlgo());
		tempExcModel->geoMaster_90.CopyOf(tempExcModel->geoMaster, 90.0F);
		if (tempExcModel->geoMaster_90.isProcessed() == false)
			tempExcModel->geoMaster_90.Preprocess();

		tempExcModel->geoMaster_180.setMatchAlgo(tempExcModel->geoMaster.getMatchAlgo());
		tempExcModel->geoMaster_180.CopyOf(tempExcModel->geoMaster, 180.0F);
		if (tempExcModel->geoMaster_180.isProcessed() == false)
			tempExcModel->geoMaster_180.Preprocess();

		tempExcModel->geoMaster_270.setMatchAlgo(tempExcModel->geoMaster.getMatchAlgo());
		tempExcModel->geoMaster_270.CopyOf(tempExcModel->geoMaster, 270.0F);
		if (tempExcModel->geoMaster_270.isProcessed() == false)
			tempExcModel->geoMaster_270.Preprocess();

		m_MapModelExc.insert(std::make_pair(sPath, tempExcModel));
	}
	else
	{
		if (g_pMPTI->GetUseMultiProcess() && bAutoInspSeq == TRUE)
			ext::InspRoot_Server::get()->ExtModelAdd(&file, sPath, ext::ModelBufferFlag::eExtModel_Exc);

		std::shared_ptr<GeoModel> shptr_model = std::shared_ptr<GeoModel>(new GeoModel);
	shptr_model->LoadFile(ar, dResolX, dResolY);

	ar.Close();
	file.Close();
	}
}
bool CPInsp_AlgoPatternDiff::IsMainProcessCheck()
{
	////이 프로세스가 쓰고 있는 Virtual Memory:
	//int nCnt = 0;
	//bool bRet = true;
	//BOOL bContinue = true;
	//CString sProcessName = _T("Eagle3D");
	//PROCESS_MEMORY_COUNTERS_EX pmc;
	//DWORD nOutHandleCnt = 0;
	//DWORD threadCount = 0;
	//sProcessName.MakeUpper(); // 문자열 비교를 하기전 강제로 모두 대문자로 바꿔줌.

	//HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	//if ((int)hSnapshot != -1)
	//{
	//	PROCESSENTRY32 pe32;
	//	pe32.dwSize = sizeof(PROCESSENTRY32);
	//	CString tempProcessName;

	//	if (Process32First(hSnapshot, &pe32))
	//	{
	//		//프로세스 목록 검색 시작
	//		while (bContinue)
	//		{
	//			tempProcessName = pe32.szExeFile; //프로세스 목록 중 비교할 프로세스 이름;
	//			tempProcessName.MakeUpper();
	//			if ((tempProcessName.Find(sProcessName, 0) != -1))
	//			{

	//				HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, pe32.th32ProcessID);
	//				GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS *)&pmc, sizeof(pmc));
	//				GetProcessHandleCount(hProcess, &nOutHandleCnt);

	//				threadCount = pe32.cntThreads;
	//				CloseHandle(hProcess);
	//			}
	//			bContinue = Process32Next(hSnapshot, &pe32);
	//		}
	//	}
	//	CloseHandle(hSnapshot);

	//}
	////이 프로세스가 쓰고 있는 Physical Memory : 이 프로세스가 쓰고 있는 Virtual Memory에 다음을 추가합니다
	//SIZE_T physMemUsedByMe = pmc.WorkingSetSize;
	//CString slog;
	//int nMegaByte = 1048576;


	//slog.Format(_T("Eagle3D_Used,%lld,(MB),HandleCnt,%d,threadCount,%d,"),
	//	physMemUsedByMe / nMegaByte, nOutHandleCnt, threadCount);
	//g_pMPTI->AddLog(slog);

	return true;
}
