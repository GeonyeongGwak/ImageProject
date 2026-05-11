#include "StdAfx.h"
#include "ProcMil_Pattern.h"
#include "GeoMatch.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char BASED_CODE THIS_FILE[] = __FILE__;
//#endif

// MModelManager에도 바꿔줘야한다.
#define PAT_ACCURACY	M_MEDIUM//M_HIGH
#define PAT_SPEED		M_MEDIUM 
#define PAT_ATCSCORE	40


CProcMil_Pattern* g_MilPat = NULL;

void Log_MilPatModel(CString msg)
{
	//g_pMPTI->AddLog(msg);
}


CProcMil_Pattern::CProcMil_Pattern(void)
{
	g_MilPat = this;
	m_className = _T("CProcMil_Pattern");

	m_milPatModel_Temp = M_NULL;
	m_milPatModel = M_NULL;
	m_milPatResult = M_NULL;

	m_milPatModel_Rotate = M_NULL;

	m_ModelMng = std::shared_ptr<CMModelManager<tagModelFile>>(new CMModelManager<tagModelFile>(eAlgoPattern, _T(".mod"), Log_MilPatModel));
}

CProcMil_Pattern::~CProcMil_Pattern(void)
{

}

int CProcMil_Pattern::InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib)
{
	if(milApp == M_NULL || milSys == M_NULL)
	{
		return eMPAT_FAIL;	
	}

	CProcMil :: InitMil(milApp, milSys,bUseImagePilLib);

	/* Allocate result buffer. */
	MpatAllocResult(*milSys, 1L, &m_milPatResult);

	return eMPAT_SUCCESS;
}

int CProcMil_Pattern::FreeMil()
{
#ifndef _DISABLE_MIL	
	if(m_milPatResult != M_NULL)
	{
		MpatFree(m_milPatResult);
		m_milPatResult = M_NULL;
	}

	DeleteModelBuf();

	CProcMil :: FreeMil();
#endif

	return eMPAT_SUCCESS;
}

void CProcMil_Pattern::DeleteModelBuf()
{
	if(m_milPatModel_Temp != M_NULL)
	{
		MpatFree(m_milPatModel_Temp);
		m_milPatModel_Temp = M_NULL;
	}

	if(m_milPatModel_Rotate != M_NULL)
	{
		MpatFree(m_milPatModel_Rotate);
		m_milPatModel_Rotate = M_NULL;
	}

}

int CProcMil_Pattern::AllocPatModel(void* milSrc, int cx, int cy, int width, int height)
{
	int ret  = eMPAT_SUCCESS;

	int stX = 0;
	int stY = 0;

	stX = (int)(cx - (width / 2.0));
	stY = (int)(cy - (height / 2.0));

	DeleteModelBuf();
	Im::PIL_ID rst = MpatAllocModel(*m_milSys, (Im::PIL_ID)milSrc, (MIL_INT)stX, (MIL_INT)stY, (MIL_INT)width, (MIL_INT)height, M_NORMALIZED, &m_milPatModel_Temp);


	if(rst == M_NULL)
		ret = eMPAT_FAIL;
	else
		m_milPatModel = m_milPatModel_Temp;

	return ret;
}

int CProcMil_Pattern::AllocPatModel(UCHAR* userSrc, int width, int height, double rotateAngle, int rotateW, int rotateH)
{
	int ret  = eMPAT_SUCCESS;

	Im::PIL_ID milSrc = AllocBuff(width, height);
	MbufPut(milSrc, userSrc);

	DeleteModelBuf();
	Im::PIL_ID rst = MpatAllocModel(*m_milSys, milSrc, (MIL_INT)0, (MIL_INT)0, (MIL_INT)width, (MIL_INT)height, M_NORMALIZED, &m_milPatModel_Temp);

	if(rotateAngle > 0)
	{
		//Im::PIL_ID rotateImg = AllocRotateBuff(milSrc, rotateAngle);
		//Im::PIL_ID rst2 = MpatAllocModel(*m_milSys, rotateImg, (MIL_INT)0, (MIL_INT)0, (MIL_INT)rotateW, (MIL_INT)rotateH, M_NORMALIZED, &m_milPatModel_Rotate);
		Im::PIL_ID rst2 = MpatAllocRotatedModel(*m_milSys, m_milPatModel_Temp, rotateAngle, M_BILINEAR, M_NORMALIZED, &m_milPatModel_Rotate);
		//FreeMilImageBuff(&rotateImg);

		if(rst2 == M_NULL)
			ret = eMPAT_FAIL;
	}

	if(rst == M_NULL)
		ret = eMPAT_FAIL;
	else
		m_milPatModel = m_milPatModel_Temp;


	FreeMilImageBuff(&milSrc);

	return ret;
}

int CProcMil_Pattern::SearchPattern(UCHAR* userSrc, int width, int height)
{
	Im::PIL_ID milTemp = M_NULL;

	milTemp = AllocBuff(width, height);
	MbufPut(milTemp, userSrc);

	MpatFindModel(milTemp, m_milPatModel, m_milPatResult);
	FreeMilImageBuff(&milTemp);

	MIL_INT count = MpatGetNumber(m_milPatResult, M_NULL);	
	return (int)count;
}

int CProcMil_Pattern::SearchPattern_Draw(UCHAR* userSrc, int width, int height, UCHAR* userDst,int Channel)
{
	Im::PIL_ID milTemp = M_NULL;

	milTemp = AllocBuff(width, height);
	MbufPut(milTemp, userSrc);

	MpatFindModel(milTemp, m_milPatModel, m_milPatResult);
	FreeMilImageBuff(&milTemp);

	MIL_INT count = MpatGetNumber(m_milPatResult, M_NULL);	
	return (int)count;
}


int CProcMil_Pattern::SearchPattern(void* milSrc, void* milSrc_color, int cx, int cy, int width, int height)
{	
	CSize size = GetImageSize((Im::PIL_ID)milSrc);
	Im::PIL_ID milTemp = AllocBuff(size.cx, size.cy); //원 이미지에 M_PROC속성이 없어 새로 생성하여 카피함..
	MbufCopy((Im::PIL_ID)milSrc, milTemp);

	int Wid = MpatInquire(m_milPatModel, M_ALLOC_SIZE_X, M_NULL);
	int Len = MpatInquire(m_milPatModel, M_ALLOC_SIZE_Y, M_NULL);
	Im::PIL_ID patImg = MbufAlloc2d(M_DEFAULT_HOST, Wid, Len, 8+M_UNSIGNED, M_IMAGE+M_PROC, M_NULL);
	MpatCopy(m_milPatModel, patImg, M_DEFAULT);

	MbufFree(patImg);

	patImg=M_NULL;
	int stX = 0;
	int stY = 0;

	stX = (int)(cx - (width / 2.0));
	stY = (int)(cy - (height / 2.0));

	MpatSetPosition(m_milPatModel, stX, stY, width, height);
	MpatFindModel(milTemp, m_milPatModel, m_milPatResult);
	FreeMilImageBuff(&milTemp);


	MIL_INT count = MpatGetNumber(m_milPatResult, M_NULL);
	return (int)count;
}

void CProcMil_Pattern::PreprocModel(UCHAR* userSrc, int sizeX, int sizeY)
{
	Im::PIL_ID milTemp = AllocBuff(sizeX, sizeY); 
	MbufPut(milTemp, userSrc);

	MpatPreprocModel(milTemp, m_milPatModel, M_DEFAULT);

	FreeMilImageBuff(&milTemp);
}

void CProcMil_Pattern::PreprocModel(void* milSrc, BOOL rotMode, int use_algo, int nLight_Number)
{
	MpatPreprocModel(M_NULL, m_milPatModel, M_DEFAULT);

	if(rotMode == TRUE)
		MpatPreprocModel(M_NULL, m_milPatModel_Rotate, M_DEFAULT);

// 	CSize size = GetImageSize((Im::PIL_ID)milSrc);
// 	Im::PIL_ID milTemp = AllocBuff(size.cx, size.cy); //원 이미지에 M_PROC속성이 없어 새로 생성하여 카피함..
// 	MbufCopy((Im::PIL_ID)milSrc, milTemp);
// 
// 	SaveWorkImg(milTemp, _T("patternSrcImg.bmp"));
// 
// 	MpatPreprocModel(milTemp, m_milPatModel, M_DEFAULT);
// 
// 	if(rotMode == TRUE)
// 		MpatPreprocModel(milTemp, m_milPatModel_Rotate, M_DEFAULT);
// 
// 	FreeMilImageBuff(&milTemp);
}

int CProcMil_Pattern::GetPatResult(double* retScore, double* retAngle, double* retPosX, double* retPosY)
{
	int ret = eMPAT_SUCCESS;
	MIL_INT count = MpatGetNumber(m_milPatResult, M_NULL);

	if((int)count == 1)
	{
		double posX = 0;
		double posY = 0;
		double angle = 0;
		double score = 0;

		MpatGetResult(m_milPatResult, M_POSITION_X, &posX);  //cX
		MpatGetResult(m_milPatResult, M_POSITION_Y, &posY);  //cY
		MpatGetResult(m_milPatResult, M_ANGLE, &angle);
		MpatGetResult(m_milPatResult, M_SCORE, &score);

		*retScore = score;
		*retAngle = angle;
		*retPosX = posX;
		*retPosY = posY;
	}
	else if((int)count > 1)
	{
		/*double*  posX = new double[(int)count];
		double*  posY = new double[(int)count];
		double*  angle = new double[(int)count];
		double*  score = new double[(int)count];*/
		double*  posX = g_pMManager->pem_new<double>(true, (int)count, (PCHAR)__FUNCTION__, __LINE__);
		double*  posY = g_pMManager->pem_new<double>(true, (int)count, (PCHAR)__FUNCTION__, __LINE__);
		double*  angle = g_pMManager->pem_new<double>(true, (int)count, (PCHAR)__FUNCTION__, __LINE__);
		double*  score = g_pMManager->pem_new<double>(true, (int)count, (PCHAR)__FUNCTION__, __LINE__);

		MpatGetResult(m_milPatResult, M_POSITION_X, posX);
		MpatGetResult(m_milPatResult, M_POSITION_Y, posY);
		MpatGetResult(m_milPatResult, M_ANGLE, angle);
		MpatGetResult(m_milPatResult, M_SCORE, score);

		int maxIndex = 0;
		double maxVal = 0;
		for(int i = 0; i < (int)count; i++)
		{
			if(maxVal < score[i])
			{
				maxVal = score[i];
				maxIndex = i;
			}
		}

		*retScore = score[maxIndex];
		*retAngle = angle[maxIndex];
		*retPosX = posX[maxIndex];
		*retPosY = posY[maxIndex];

		/*delete [] posX;
		delete [] posY;
		delete [] angle;
		delete [] score;*/
		g_pMManager->pem_delete(posX, true);
		g_pMManager->pem_delete(posY, true);
		g_pMManager->pem_delete(angle, true);
		g_pMManager->pem_delete(score, true);
	}
	else
	{
		*retScore = 0;
		*retAngle = 0;
		*retPosX = 0;
		*retPosY = 0;

		ret = eMPAT_FAIL;
	}

	if (*retAngle > 180)
	{
		*retAngle -= 360;
	}
	else if(*retAngle < -180)
	{
		*retAngle += 360;
	}

	return ret;
}

int CProcMil_Pattern::WritePatternModel(void* milSrc, int cx, int cy, int width, int height, CString strPath)
{
	int ret = eMPAT_SUCCESS;
	int stX = 0;
	int stY = 0;

	if(m_milPatModel == M_NULL)
		return eMPAT_FAIL;

	stX = (int)(cx - (width / 2.0));
	stY = (int)(cy - (height / 2.0));

	CString strImagePath = _T("");
	strImagePath = strPath + _T(".tif");

	ModelSave(strPath);
	SaveClipImg((Im::PIL_ID)milSrc, 1, cx, cy, width, height, strImagePath);


// 	if(mode == TRUE)
// 	{
// 		CString strTemp = _T("");
// 
// 		Im::PIL_ID milClip = M_NULL;
// 		milClip = AllocClipBuff((Im::PIL_ID)milSrc, cx, cy, width, height);
// 
// 		int tempCx = (int)(height / 2.0);
// 		int tempCy = (int)(width / 2.0);
// 		int tempW = height;
// 		int tempH = width;
// 
// 		//90
// 		Im::PIL_ID milRotate = M_NULL;
// 		milRotate = AllocRotateBuff(milClip, 90.0);
// 
// 		strTemp = strPath + _T("_90.tif");
// 		SaveClipImg((Im::PIL_ID)milRotate, 1, tempCx, tempCy, tempW, tempH, strTemp);
// 		FreeMilImageBuff(&milRotate);
// 
// 		//180
// 		tempCx = (int)(width / 2.0);
// 		tempCy = (int)(height / 2.0);
// 		tempW = width;
// 		tempH = height;
// 		milRotate = AllocRotateBuff(milClip, 180.0);	
// 
// 		strTemp = strPath + _T("_180.tif");
// 		SaveClipImg((Im::PIL_ID)milRotate, 1, tempCx, tempCy, tempW, tempH, strTemp);
// 		FreeMilImageBuff(&milRotate);
// 
// 		//270
// 		tempCx = (int)(height / 2.0);
// 		tempCy = (int)(width / 2.0);
// 		tempW = height;
// 		tempH = width;
// 		milRotate = AllocRotateBuff(milClip, 270.0);
// 
// 		strTemp = strPath + _T("_270.tif");
// 		SaveClipImg((Im::PIL_ID)milRotate, 1, tempCx, tempCy, tempW, tempH, strTemp);
// 		FreeMilImageBuff(&milRotate);
// 
// 
// 		FreeMilImageBuff(&milClip);
// 	}

	return ret;
}

int CProcMil_Pattern::WritePatternModel(UCHAR* userSrc, int width, int height, double rotateAngle, CString strPath, CString strExt)
{
	int ret = eMPAT_SUCCESS;

	if(userSrc == NULL || width < 0 || height < 0)
		return eMPAT_FAIL;

	Im::PIL_ID milSrc = AllocBuff(width, height);
	MbufPut(milSrc, userSrc);

	if(m_milPatModel == M_NULL)
		return eMPAT_FAIL;

	CString strImagePath = _T("");
	//strImagePath = strPath;// +_T(".bmp");
	strImagePath.Format(_T("%s%s"), strPath, strExt);

	if(rotateAngle == 0)
	{ 
		if(m_milPatModel != M_NULL)
		{
			ModelSave(m_milPatModel, strPath);
			SaveBuff(milSrc, strImagePath);
		}
		else
		{
			ret = eMPAT_FAIL;
		}
	}
	else
	{
		if(m_milPatModel_Rotate != M_NULL)
		{
			Im::PIL_ID rotateImg = AllocRotateBuff(milSrc, rotateAngle);
			ModelSave(m_milPatModel_Rotate, strPath);
			SaveBuff(rotateImg, strImagePath);

			FreeMilImageBuff(&rotateImg);
		}
		else
		{
			ret = eMPAT_FAIL;
		}
	}

	FreeMilImageBuff(&milSrc);
	return ret;
}

int CProcMil_Pattern::WritePatternModel(CString strPath)
{
	int ret = eMPAT_SUCCESS;

	if(m_milPatModel == M_NULL)
		return eMPAT_FAIL;

	ModelSave(strPath);

	return ret;
}

void CProcMil_Pattern::ModelSave(CString strPath)
{
	if(m_milPatModel == M_NULL)
		return;

	CString strModelPath = _T("");
	strModelPath = strPath + _T(".mod");

	CString strModelPath_ppd = strPath + _T(".ppd");
	DeleteFile(strModelPath_ppd);

	MpatSave(strModelPath, m_milPatModel);
}

void CProcMil_Pattern::ModelSave(Im::PIL_ID milModel, CString strPath)
{
	if(milModel == M_NULL)
		return;

	CString strModelPath = _T("");
	strModelPath = strPath + _T(".mod");

	CString strModelPath_ppd = strPath + _T(".ppd");
	DeleteFile(strModelPath_ppd);

	MpatSave(strModelPath, milModel);
}
// 
// int CProcMil_Pattern::ModelList_Load(CString strPath)
// {
// 	int ret = eMPAT_SUCCESS;
// 
// 	if (!m_ModelMng->LoadFileList(strPath))
// 		ret = eMPAT_FAIL;
// 
// 	return ret;
// }

int CProcMil_Pattern::ModelList_Load(fileAlgoPath* strPath,int ptrFileAlgoPathCnt)
{
	int ret = eMPAT_SUCCESS;

	if (!m_ModelMng->LoadFileList(strPath,ptrFileAlgoPathCnt,m_resolX,m_resolY))
		ret = eMPAT_FAIL;

	return ret;
}
int CProcMil_Pattern::ExtModelList_Load()
{
	int ret = eMPAT_SUCCESS;

	if (!m_ModelMng->ExtLoadFileList_P( m_resolX, m_resolY))
		ret = eMPAT_FAIL;

	return ret;
}

int CProcMil_Pattern::ModelList_Check(CString strPath)
{
	int ret = eMPAT_SUCCESS;

	if (!m_ModelMng->CheckFileList(strPath,m_resolX,m_resolY))
		ret = eMPAT_FAIL;

	return ret;
}
int CProcMil_Pattern::ExtModelList_Check()
{
	int ret = eMPAT_SUCCESS;

	if (!m_ModelMng->ExtCheckFileList(m_resolX, m_resolY ,1))
		ret = eMPAT_FAIL;

	return ret;
}
int CProcMil_Pattern::ModelList_Clear(int nLane)
{
	int ret = eMPAT_SUCCESS;

	ret = m_ModelMng->Clear_File(nLane);

	return ret;
}

int CProcMil_Pattern::ModelLoad(CString strPath)
{
	int ret = eMPAT_SUCCESS;

	DeleteModelBuf();


	CString strModelPath = _T("");
	strModelPath = strPath + _T(".mod");
	strPath.MakeLower();

	m_ModelFile = m_ModelMng->GetModel(strPath);
	
	if (m_ModelFile.get() == NULL)
	{
		CString strLog;
		strLog.Format(_T("Add Pattern Load Model  = %s "), strPath);
		ext::Log::add(strLog);
		ext::Log::add(_T("ModelLoad Faile "));


		ret = eMPAT_FAIL;
	}

	else if(m_ModelFile->isAllocedInternally() == true)
	{
		ret = eMPAT_AllocedInternally;
	}

	return ret;
}

int CProcMil_Pattern::CodeSearchModelLoad(CString strPath)
{
	int ret = eMPAT_SUCCESS;

	DeleteModelBuf();

	CString strModelPath = _T("");
	strModelPath = strPath + _T(".mod");
	strPath.MakeLower();
	m_ModelFile = m_ModelMng->GetPartSearchModel(strPath);

	if(m_ModelFile.get() == NULL)
	{
		ret = eMPAT_FAIL;
	}

	if(m_ModelFile->isAllocedInternally() == true)
	{
		ret = eMPAT_AllocedInternally;
	}

	return ret;
}

int CProcMil_Pattern::SimilarModelListLoad(CString Parcode, bool rear)
{
	DeleteModelBuf();

	m_SimilarModelFile.reserve( m_ModelMng->GetModelCount(rear) );

//	m_SimilarModelFileCnt = m_ModelMng->GetSimilarModel(Parcode,rear,&m_SimilarModelFile[m_SimilarModelFileCnt+1]);
	m_ModelMng->GetSimilarModel(Parcode,rear,m_SimilarModelFile);

//	return m_SimilarModelFileCnt;
	return m_SimilarModelFile.size();
}

void CProcMil_Pattern::SimilarCntReset()
{
	m_SimilarModelFile.clear();

//	m_SimilarModelFileCnt = -1;
}

int CProcMil_Pattern::SimilarModelLoad(int index)
{
	int ret = eMPAT_SUCCESS;

	m_ModelFile = m_SimilarModelFile[index];

	if(m_ModelFile == NULL)
	{
		ret = eMPAT_FAIL;
	}

	return ret;
}

int CProcMil_Pattern::ModelFileLoad(CString strPath)
{
	int ret = eMPAT_SUCCESS;

	DeleteModelBuf();

	CString strModelPath = _T("");
	strModelPath = strPath + _T(".mod");
	CString strModelPath_ppd = strPath + _T(".ppd");

	if(FileExists(strModelPath))
	{
		m_ModelFile = std::shared_ptr<tagModelFile>(new tagModelFile(eAlgoPattern));
		m_ModelFile->LoadFile(strModelPath,m_resolX,m_resolY);

		if(m_ModelFile->milModel == M_NULL)
			return eMPAT_FAIL;
	}
	else if(FileExists(strModelPath_ppd))
	{
		m_ModelFile = std::shared_ptr<tagModelFile>(new tagModelFile(eAlgoPattern));
		m_ModelFile->LoadFile(strModelPath_ppd,m_resolX,m_resolY);
		if(m_ModelFile->isAllocedInternally() == true)
			ret = eMPAT_AllocedInternally;
		else
			return eMPAT_FAIL;
	}
	else
	{
		ret = eMPAT_FAIL;
		return ret;
	}

	m_milPatModel = m_ModelFile->milModel;

	return ret;
}

void CProcMil_Pattern::ModelPreproc(double dAngle,int ModelPreproc)
{
	switch((int)dAngle)
	{
	case 0:
		m_milPatModel = m_ModelFile->milModel;
		break;
	case 90:
		{
			if (m_ModelFile->milModel_90 == M_NULL)
			{
				MpatAllocRotatedModel(M_DEFAULT_HOST, m_ModelFile->milModel, 0, M_DEFAULT, M_NORMALIZED, &m_ModelFile->milModel_90);

				MpatSetAngle(m_ModelFile->milModel_90, M_SEARCH_ANGLE, dAngle); //search start angle
				MpatPreprocModel(M_NULL, m_ModelFile->milModel_90, M_DEFAULT);
			}

			m_milPatModel = m_ModelFile->milModel_90;
		}
		break;
	case 180:
		{
			if (m_ModelFile->milModel_180 == M_NULL)
			{
				MpatAllocRotatedModel(M_DEFAULT_HOST, m_ModelFile->milModel, 0, M_DEFAULT, M_NORMALIZED, &m_ModelFile->milModel_180);

				MpatSetAngle(m_ModelFile->milModel_180, M_SEARCH_ANGLE, dAngle); //search start angle
				MpatPreprocModel(M_NULL, m_ModelFile->milModel_180, M_DEFAULT);
			}

			m_milPatModel = m_ModelFile->milModel_180;
		}
		break;
	case 270:
		{
			if (m_ModelFile->milModel_270 == M_NULL)
			{
				MpatAllocRotatedModel(M_DEFAULT_HOST, m_ModelFile->milModel, 0, M_DEFAULT, M_NORMALIZED, &m_ModelFile->milModel_270);

				MpatSetAngle(m_ModelFile->milModel_270, M_SEARCH_ANGLE, dAngle); //search start angle
				MpatPreprocModel(M_NULL, m_ModelFile->milModel_270, M_DEFAULT);
			}

			m_milPatModel = m_ModelFile->milModel_270;
		}
		break;
	default:
		m_milPatModel = m_ModelFile->milModel;
		break;
	}
//	SetAngleMode(40, 40, ModelPreproc,FALSE);
}

int CProcMil_Pattern::RotateImage(CString sPathImageOld, CString sPathImageNew, double dAngRotate)
{
	int ret = ModelLoad(sPathImageOld);
	if(ret == eMPAT_FAIL)
		return ret;

	Im::PIL_ID rst2 = MpatAllocRotatedModel(*m_milSys, m_milPatModel, dAngRotate, M_BILINEAR, M_NORMALIZED, &m_milPatModel_Rotate);
	if(rst2 == M_NULL)
		ret = eMPAT_FAIL;

	MpatSave(sPathImageNew + _T(".mod"), m_milPatModel_Rotate);
}

void CProcMil_Pattern::SetAccuracy(int speedFactor, int accuracy, BOOL rotMode)
{
	MpatSetSpeed(m_milPatModel, speedFactor);
	MpatSetAccuracy(m_milPatModel, accuracy);
	 
	if(rotMode == TRUE || m_milPatModel_Rotate != M_NULL)
	{
		MpatSetSpeed(m_milPatModel_Rotate, speedFactor);
		MpatSetAccuracy(m_milPatModel_Rotate, accuracy);
	}

}

int CProcMil_Pattern::GetResultImg(UCHAR* userSrc, int srcSizeX, int srcSizeY, UCHAR* userDst)
{
	int ret = eMPAT_SUCCESS;

	if(m_milPatResult == M_NULL || userSrc == NULL || userDst == NULL || srcSizeX < 0 || srcSizeY < 0)
		return eMPAT_FAIL;

	Im::PIL_ID srcTemp = M_NULL;
	srcTemp = AllocBuff(srcSizeX, srcSizeY);
	MbufPut(srcTemp, userSrc);

	Im::PIL_ID milTemp = M_NULL;
	milTemp = AllocBuffColor(srcSizeX, srcSizeY);
	MbufCopy(srcTemp, milTemp);

	//MgraColor(M_DEFAULT, M_COLOR_GREEN);
	//MpatDraw(M_DEFAULT, m_milPatResult, milTemp, M_DRAW_BOX, M_DEFAULT, M_DEFAULT);

	//MgraColor(M_DEFAULT, M_COLOR_RED);
	//MpatDraw(M_DEFAULT, m_milPatResult, milTemp, M_DRAW_POSITION, M_DEFAULT, M_DEFAULT);

	MbufGetColor(milTemp, M_PACKED+M_BGR24, M_ALL_BANDS, userDst);

	FreeMilImageBuff(&srcTemp);
	FreeMilImageBuff(&milTemp);
	
	return ret;
}

void CProcMil_Pattern::SetAngleMode(double posLimit, double negLimit, double accuracy, BOOL rotMode, bool bAngleModel)
{
	SetAccuracy(PAT_SPEED, PAT_ACCURACY, rotMode);

	MpatSetAngle(m_milPatModel, M_SEARCH_ANGLE_MODE, M_ENABLE);
	MpatSetAngle(m_milPatModel, M_SEARCH_ANGLE_INTERPOLATION_MODE, M_BILINEAR);
	MpatSetAngle(m_milPatModel, M_SEARCH_ANGLE_DELTA_NEG, negLimit);
	MpatSetAngle(m_milPatModel, M_SEARCH_ANGLE_DELTA_POS, posLimit);
	MpatSetAngle(m_milPatModel, M_SEARCH_ANGLE_ACCURACY, accuracy);

	MpatSetAcceptance(m_milPatModel, PAT_ATCSCORE);	

	if(rotMode == TRUE || m_milPatModel_Rotate != M_NULL)
	{
		MpatSetAngle(m_milPatModel_Rotate, M_SEARCH_ANGLE_MODE, M_ENABLE);
		MpatSetAngle(m_milPatModel_Rotate, M_SEARCH_ANGLE_INTERPOLATION_MODE, M_BILINEAR);
		MpatSetAngle(m_milPatModel_Rotate, M_SEARCH_ANGLE_DELTA_NEG, negLimit);
		MpatSetAngle(m_milPatModel_Rotate, M_SEARCH_ANGLE_DELTA_POS, posLimit);
		MpatSetAngle(m_milPatModel_Rotate, M_SEARCH_ANGLE_ACCURACY, accuracy);

		MpatSetAcceptance(m_milPatModel_Rotate, PAT_ATCSCORE);	
	}
}

void CProcMil_Pattern::SetSearchStartAngle(double angle)
{
	MpatSetAngle(m_milPatModel, M_SEARCH_ANGLE, angle); //search start angle
}

bool CProcMil_Pattern::SetParam_DivArea(BOOL bUseCharDiv, int* DiviLnC, int* DiviLnR, double ** GapLnC, double ** GapLnR, double ** DiviScore, bool bDetailSearch, RECT modelRect)
{
	//	PIL 클래스의 신규 기능으로 MIL에는 관리 안함
	return true;
}

void CProcMil_Pattern::GetPatSubScore(double* retScore, int Pitch)
{
	//	PIL 클래스의 신규 기능으로 MIL에는 관리 안함
}

void CProcMil_Pattern::GetParam_DivArea(BOOL& bUseCharDiv, int & DiviLnC, int & DiviLnR, double * GapLnC, double * GapLnR, double * DiviScore, BOOL & bDetailSearch, RECT & modelRect)
{
	//	PIL 클래스의 신규 기능으로 MIL에는 관리 안함
}
int CProcMil_Pattern::GetMaskImage(CString sPath, UCHAR * Img, int Wid, int Len, int Pitch)
{
	return eMPAT_FAIL;
}

int CProcMil_Pattern::SetMaskImage(UCHAR * Img, int Wid, int Len, int Pitch, double dAngle)
{
	return eMPAT_FAIL;
}int CProcMil_Pattern::GetnChannel()
{
	return 0;//	PIL 클래스의 신규 기능으로 MIL에는 관리 안함
}
int CProcMil_Pattern::GetModelAlgo()
{
	return 5;
}
void CProcMil_Pattern::SetnChannel(int nChannel)
{
	//	PIL 클래스의 신규 기능으로 MIL에는 관리 안함
}
void CProcMil_Pattern::GetnStep(int step)
{
	//	PIL 클래스의 신규 기능으로 MIL에는 관리 안함
}
void CProcMil_Pattern::GetClipModelImg(cv::Mat ClipModelImg)
{
	//	PIL 클래스의 신규 기능으로 MIL에는 관리 안함
}

bool CProcMil_Pattern::IsDivInspAvailable()
{
	return false;
}
void CProcMil_Pattern::ImgProcess(void* milSrc, void* milSrc_color, int cx, int cy, int width, int height,std::shared_ptr<ImgProcessing> Proc_Img)
{
	//	PIL 클래스의 신규 기능으로 MIL에는 관리 안함
}
void CProcMil_Pattern::ImgProcess(UCHAR* userSrc, int width, int height,std::shared_ptr<ImgProcessing> Proc_Img)
{
	//	PIL 클래스의 신규 기능으로 MIL에는 관리 안함
}
void CProcMil_Pattern::SetFindSubPixel(BOOL rotMode,bool bFindModel)
{
	//	PIL 클래스의 신규 기능으로 MIL에는 관리 안함
}
int CProcMil_Pattern::AllocModePatlexceptMask(UCHAR* userSrc, int width, int height, double rotateAngle, int rotateW, int rotateH)
{
	//	PIL 클래스의 신규 기능으로 MIL에는 관리 안함
	return false;
}
SIZE CProcMil_Pattern::getModelSz()
{
	//	PIL 클래스의 신규 기능으로 MIL에는 관리 안함
	SIZE rst;
	rst.cx = 0;
	rst.cy = 0;
	return rst;
}