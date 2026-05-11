#pragma once
#include "StdAfx.h"
#include "ProcMil_OCR.h"
#include "ipp.h"

void Log_OCRModel(CString msg)
{
	//g_pMPTI->AddLog(msg);
}

CProcMil_OCR::CProcMil_OCR(void)
{
	m_className = _T("CProcMil_OCR");

	m_milStrContext_Temp = M_NULL;
	m_milStrContext = M_NULL; 
	m_milStrResult = M_NULL; 

	m_milBlobFeature = M_NULL;
	m_milBlobResult = M_NULL;

	m_ModelMng = std::shared_ptr<CMModelManager<tagModelFile>>(new CMModelManager<tagModelFile>(eAlgoOCR, _T(".msr"), Log_OCRModel));
}


CProcMil_OCR::~CProcMil_OCR(void)
{

}


int CProcMil_OCR::InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib)
{
	if(milApp == M_NULL || milSys == M_NULL)
	{
		return eSTR_FAIL;	
	}
		

	CProcMil :: InitMil(milApp, milSys,bUseImagePilLib);

	// Allocate a new empty String Reader context.
	//MstrAlloc(*m_milSys, M_FEATURE_BASED, M_DEFAULT, &m_milStrContext);
	// Allocate a new empty String Reader result buffer.
	//MstrAllocResult(*m_milSys, M_DEFAULT, &m_milStrResult);	// 2014/11/18
	/*
	Im::Blob::blobAllocFeatureList(*m_milSys, &m_milBlobFeature);   //Allocate blob feature list.
	Im::Blob::blobAllocResult(*m_milSys, &m_milBlobResult);         //Allocate blob result.

	//add feature list.
	Im::Blob::blobSelectFeature(m_milBlobFeature, M_AREA + M_SORT1_DOWN);
	Im::Blob::blobSelectFeature( m_milBlobFeature, M_CENTER_OF_GRAVITY);
	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BOX_X_MAX);
	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BOX_X_MIN);
	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BOX_Y_MAX);
	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BOX_Y_MIN);
	*/
	m_nUsecontinue = 0;
	InitMilAlgoBlob();
	memset(doubeZeroBuf,-1,sizeof(double)*MAX_STRLEN);
	ImgWidth=0;
	ImgHeight=0;

	return eSTR_SUCCESS;
}

int CProcMil_OCR::FreeMil()
{
#ifndef _DISABLE_MIL	

	if(m_milStrContext != M_NULL)
	{
		if(IsPreprocess())
			UnPreprocess();

// 		MstrFree(m_milStrContext);
// 		m_milStrContext = M_NULL;
	}

	if(m_milStrContext_Temp != M_NULL)
	{
		MstrFree(m_milStrContext_Temp);
		m_milStrContext_Temp = M_NULL;
	}

	if(m_milStrResult != M_NULL)
	{
		MstrFree(m_milStrResult);
		m_milStrResult = M_NULL;
	}

	if(m_milBlobFeature != M_NULL)
	{
		Im::Blob::blobFree(m_milBlobFeature);
		m_milBlobFeature = M_NULL;
	}

	if(m_milBlobResult != M_NULL)
	{
		Im::Blob::blobFree(m_milBlobResult);
		m_milBlobResult = M_NULL;
	}

	CProcMil :: FreeMil();
#endif

	return eSTR_SUCCESS;
}



Im::PIL_ID CProcMil_OCR::GetContextID()
{
	return m_milStrContext;
}

BOOL CProcMil_OCR::ExistContext()
{
	BOOL ret = TRUE;

	if(m_milStrContext == M_NULL)
		ret = FALSE;

	return ret;
}

BOOL CProcMil_OCR::ExistFont()
{
	BOOL ret = TRUE;

	if(!ExistContext())
		return FALSE;

	int fontCount = GetFontCount();

	if(fontCount <= 0)
		ret = FALSE;

	return ret;
}

template<typename T>
int CProcMil_OCR::StrInquire(int index, int type, T * controlValue)
{
	MstrInquire(m_milStrContext, index, type, controlValue);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::StrControl(int index, int type, double controlValue)
{
	MstrControl(m_milStrContext, index, type, controlValue);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::AddNewFont()//1
{ 
	if(m_milStrContext_Temp != M_NULL)
	{
		MstrFree(m_milStrContext_Temp);	
		m_milStrContext_Temp = M_NULL;
	}
	MstrAlloc(*m_milSys, M_FEATURE_BASED, M_DEFAULT, &m_milStrContext_Temp);
	m_milStrContext = m_milStrContext_Temp;

	StrControl(M_CONTEXT, M_FONT_ADD, M_USER_DEFINED);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::AddFont()//1
{ 
	if(m_milStrContext == M_NULL)
		return eSTR_FAIL;

	StrControl(M_CONTEXT, M_FONT_ADD, M_USER_DEFINED);
	return eSTR_SUCCESS;
}


int CProcMil_OCR::DeleteFont(int fontIndex)//1
{ 
	StrControl(M_CONTEXT, M_FONT_DELETE, fontIndex);//M_FONT_INDEX(fontIndex));

	return eSTR_SUCCESS;
}

int CProcMil_OCR::AddString()//2
{ 
	DeleteString();
	StrControl(M_CONTEXT, M_STRING_ADD, M_USER_DEFINED);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::DeleteString()//2
{ 
	StrControl(M_CONTEXT, M_STRING_DELETE ,  M_ALL);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::GetCharMaxCount()//2
{
	MIL_INT count = 0;
	StrInquire(M_STRING_INDEX(M_ALL), M_STRING_SIZE_MAX + M_TYPE_MIL_INT, &count);
	return count;
}

int CProcMil_OCR::SetCharMaxCount(int count)//2
{
	StrControl(M_STRING_INDEX(M_ALL), M_STRING_SIZE_MAX, count);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::GetCharMinCount()//2
{
	MIL_INT count=0;
	StrInquire(M_STRING_INDEX(0), M_STRING_SIZE_MIN + M_TYPE_MIL_INT, &count);

	return count;
}

int CProcMil_OCR::SetCharMinCount(int count)//2
{
	StrControl(M_STRING_INDEX(M_ALL), M_STRING_SIZE_MIN, count);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::SetStringCount(int count)//2
{
	StrControl(M_CONTEXT, M_STRING_NUMBER, count);
	StrControl(M_STRING_INDEX(0), M_STRING_NUMBER, count);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::SetSpaceChar(int space)
{
	StrControl(M_CONTEXT, M_SPACE_CHARACTER, space);

	return eSTR_SUCCESS;
}

int	CProcMil_OCR::SetThickenChar(int thickenChar)
{
	StrControl(M_CONTEXT, M_THICKEN_CHAR, thickenChar);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::StrEditFont(int index, int operation, int operationMode, MIL_INT param1, const void *param2)
{
  	MstrEditFont(m_milStrContext, index, operation,	operationMode, param1, param2 , M_NULL);

	return eSTR_SUCCESS;
}


int CProcMil_OCR::SetEditFont(UCHAR* srcImg, int sizeX, int sizeY, double angle, int foreGround, CString font, int fontIndex, 
												int rmLineCnt, int* rmLinePos, int* rmLineWid, int threshMode, int threshVal, int* retThresh)
{
	Im::PIL_ID milTemp = M_NULL;
	Im::PIL_ID milRotateImg = M_NULL;
	Im::PIL_ID milProcImg = M_NULL;
	int fontCount = GetFontCount(m_milStrContext);
	int fontLen = font.GetLength();
	int threshold = threshVal;
	int rstThresh = 0;
	int roiSizeX = sizeX;
	int roiSizeY = sizeY;
	
	if(fontLen>MAX_STRLEN)
	{
		return eSTR_OVERWRITE; 
	}

	
	int index = 0;
	int charCnts = 0;


	//////////////////////////////////////////////////////////////////////////
	//clip buff
	milTemp = AllocBuff(roiSizeX, roiSizeY);
	MbufPut(milTemp, srcImg);
	SaveWorkImg(milTemp, _T("fontClip.bmp"));

	//rotate buff
	if(angle > 0)
	{
		milRotateImg = AllocRotateBuff(milTemp, angle);	

		if(angle == 90 || angle == 270)
		{
			roiSizeX = sizeY;
			roiSizeY = sizeX;
		}
	}
	else
	{
		milRotateImg = milTemp;
	}
	SaveWorkImg(milRotateImg, _T("fontrotate.bmp"));

	BOOL bAllchar = FALSE;

	if (font.Find(_T("*")) == -1) // * 미사용시 
	{
		bAllchar = FALSE;
	}
	else  // * 사용시
	{
		bAllchar = TRUE;
	}

	//Highlight font
	milProcImg = AllocBuff(roiSizeX, roiSizeY, 0);

	rstThresh = FontHighlight(milRotateImg, milProcImg, foreGround, FALSE, fontLen, threshold,bAllchar);
	SaveWorkImg(milProcImg, _T("fontClip_remake.bmp"));

	// Draw Line (배경색으로 라인을 그려 붙은 글자 분리)
	SetDrawLine((void *)milProcImg, rmLineCnt, rmLinePos, rmLineWid, foreGround);
	SaveWorkImg(milProcImg, _T("font_line.bmp"));

	//edit font
	charCnts = EditFont(milProcImg, font, fontIndex, foreGround);
	//charCnts = EditFont(milProcImg, font, fontIndex, foreGround);

	if(charCnts <= 0)//1차 폰트 등록 실패후... 실행
	{
		//Highlight font
		MbufClear(milProcImg, 0);
		rstThresh = FontHighlight(milRotateImg, milProcImg, foreGround, TRUE, fontLen, threshVal,bAllchar);
		SaveWorkImg(milProcImg, _T("fontClip_remake2.bmp"));

		// Draw Line (배경색으로 라인을 그려 붙은 글자 분리)
		SetDrawLine((void *)milProcImg, rmLineCnt, rmLinePos, rmLineWid, foreGround);
		SaveWorkImg(milProcImg, _T("font_line2.bmp"));

		charCnts = EditFont(milProcImg, font, fontIndex, foreGround);
		//charCnts = EditFont(milProcImg, font, fontIndex, foreGround);
	}	


	//////////////////////////////////////////////////////////////////////////

	if(retThresh != NULL) //사용 안함
		*retThresh = rstThresh;

	//////////////////////////////////////////////////////////////////////////

	if(milTemp != M_NULL)
		FreeMilImageBuff(&milTemp);

	if(milProcImg != M_NULL)
		FreeMilImageBuff(&milProcImg);

	if(angle > 0)
	{
		if(milRotateImg != M_NULL)
			FreeMilImageBuff(&milRotateImg);
	}

	if(charCnts < 1)
		return eSTR_FAIL;
	else
		return eSTR_SUCCESS;
}

int CProcMil_OCR::EditFont(Im::PIL_ID srcImg, CString font, int fontIndex, int foreGround)
{
	char editFont[MAX_STRLEN];
	int fontCount = GetFontCount(m_milStrContext);
	MIL_INT charCnts = 0;

	wcstombs_s(NULL, editFont, MAX_STRLEN, font, _TRUNCATE);

	BOOL bAllchar = FALSE;

	if (font.Find(_T("*")) == -1) // * 미사용시 
	{
		bAllchar = FALSE;
	}
	else  // * 사용시
	{
		bAllchar = TRUE;
	}


	if(fontIndex < 0)
	{
		if (!bAllchar)
		{
			int nOperateMode = (font.GetLength() != 1) ? M_USER_DEFINED : M_USER_DEFINED + M_SINGLE;
			StrEditFont(M_FONT_INDEX(fontCount - 1), M_CHAR_ADD,nOperateMode + (foreGround ?  M_FOREGROUND_WHITE : M_FOREGROUND_BLACK), srcImg, editFont);
		}
		else
		{
			StrControl(M_CONTEXT, M_ENCODING , M_DEFAULT);
			StrEditFont(M_FONT_INDEX(fontCount - 1), M_CHAR_ADD,M_USER_DEFINED + M_SINGLE +(foreGround ?  M_FOREGROUND_WHITE : M_FOREGROUND_BLACK), srcImg, editFont);
		}
		
	}
	else
	{
		if (!bAllchar)
		{
			int nOperateMode = (font.GetLength() != 1) ? M_USER_DEFINED : M_USER_DEFINED + M_SINGLE;
			StrEditFont(M_FONT_INDEX(fontIndex), M_CHAR_ADD,nOperateMode + (foreGround ?  M_FOREGROUND_WHITE : M_FOREGROUND_BLACK), srcImg, editFont);
		}
		else
		{
			StrControl(M_CONTEXT, M_ENCODING , M_DEFAULT);
			StrEditFont(M_FONT_INDEX(fontIndex), M_CHAR_ADD,M_USER_DEFINED + M_SINGLE + (foreGround ?  M_FOREGROUND_WHITE : M_FOREGROUND_BLACK), srcImg, editFont);
		}
	}

// 	if(fontIndex < 0)
// 		StrEditFont(M_FONT_INDEX(fontCount - 1), M_CHAR_ADD,M_USER_DEFINED + (foreGround ?  M_FOREGROUND_WHITE : M_FOREGROUND_BLACK), srcImg, editFont);
// 	else
// 		StrEditFont(M_FONT_INDEX(fontIndex), M_CHAR_ADD,M_USER_DEFINED + (foreGround ?  M_FOREGROUND_WHITE : M_FOREGROUND_BLACK), srcImg, editFont);


	MstrInquire(m_milStrContext, M_FONT_INDEX(fontIndex),  M_NUMBER_OF_CHARS + M_TYPE_MIL_INT , &charCnts);

	return (int)charCnts;
}

int CProcMil_OCR::FontHighlight(Im::PIL_ID src, Im::PIL_ID dst, int foreGround, BOOL mode, int charCount, int threshold, BOOL bAllchar)
{
	Im::PIL_ID milProcTemp = M_NULL;
	CSize sz = GetImageSize(src);

	milProcTemp = AllocBuff((int)sz.cx, (int)sz.cy);

	int thresh = threshold;

	Binarize(src, milProcTemp, thresh, FALSE);

	//SaveWorkImg(src, _T("OCR_MIL_BW_ORG.bmp"));
	//SaveWorkImg(milProcTemp, _T("OCR_MIL_BW_Threshold.bmp"));

	if(mode)
	{
		if(foreGround)
		{
			MorClose(milProcTemp, milProcTemp, 1);
			MorOpen(milProcTemp, milProcTemp, 1);	
		}
		else
		{
			MorOpen(milProcTemp, milProcTemp, 1);	
			MorClose(milProcTemp, milProcTemp, 1);
		}
	}
	else
	{
		//2014/04/02 문자부분을 강조하고 배경은 찍은 이미지 그대로 사용하던것을 그냥 배경은 0으로 바꿈. (이상하게 등록 되는 경우가 있음)
		//ArithImage(src, milProcTemp, dst, M_OR );
	}
	

	if (!bAllchar)
	{
		MakeBlob(milProcTemp, milProcTemp, foreGround, charCount);
	}

	MbufCopy(milProcTemp, dst);

	//SaveWorkImg(milProcTemp, _T("OCR_MIL_BW_Result.bmp"));

	FreeMilImageBuff(&milProcTemp);

	return thresh;
}

//huj 2014/03/25 foreground, fontCount 추가
void CProcMil_OCR::MakeBlob(Im::PIL_ID src, Im::PIL_ID dst, int foreground, int fontCount)
{
	if(foreground == 1)
		Im::Blob::blobControl(m_milBlobResult, M_FOREGROUND_VALUE, M_NONZERO);
	else
		Im::Blob::blobControl(m_milBlobResult, M_FOREGROUND_VALUE, M_ZERO);

	Im::Blob::blobCalculate(src, M_NULL, m_milBlobFeature, m_milBlobResult);

	MIL_INT count = 0;
	Im::Blob::blobGetNumber(m_milBlobResult,  &count);

	if(count > fontCount)	// 등록한 Font 개수만큼만 Blob이 잡혔다면, Select Blob할 필요없다.
	{
		int areaThres = 0;
		if(count > 0 && fontCount > 0)
		{
			//double* area = new double[(int)count];
			double* area = g_pMManager->pem_new<double>(true, (int)count, (PCHAR)__FUNCTION__, __LINE__);
			Im::Blob::blobGetResult(m_milBlobResult, M_AREA + M_TYPE_DOUBLE, area);
			ippsSortDescend_64f_I(area, count);		// 내림차순 정렬

			areaThres = (int)area[fontCount];	// 등록할 글자 수+1 번째 Area를 받아서, 작거나 같은 Blob은 제거한다.
			//delete area;
			g_pMManager->pem_delete(area, false);
		}
		
		//blob select
		Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_AREA, M_LESS_OR_EQUAL, areaThres, M_NULL);
	}

	if(foreground == 1)
	{
		Im::Blob::blobFill(m_milBlobResult, dst, M_EXCLUDED_BLOBS, 0);   //redraw excluded blob (value : 0) -> erase
		Im::Blob::blobFill(m_milBlobResult, dst, M_INCLUDED_BLOBS, 255);  //redraw included blob (value : 255) -> draw
	}
	else
	{
		Im::Blob::blobFill(m_milBlobResult, dst, M_EXCLUDED_BLOBS, 255);   //redraw excluded blob (value : 255) -> draw 
		Im::Blob::blobFill(m_milBlobResult, dst, M_INCLUDED_BLOBS, 0);  //redraw included blob (value : 0) -> erase 
	}

	
}

int CProcMil_OCR::SetNomalizeFont(int refer, int size, int fontIndex)//1
{
	StrEditFont(M_FONT_INDEX(fontIndex), M_CHAR_NORMALIZE, (refer ?  M_SIZE_Y : M_SIZE_X), size, M_NULL);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::SetSpaceWidthFont(int mode, double width)//1
{
	double value;

	switch(mode)
	{
	case eSTR_INFINITE:
		{
			value = M_INFINITE ;
		}
		break;
	case eSTR_MAX:
		{
			value = M_MAX_CHAR_WIDTH  ;
		}
		break;
	case eSTR_MEAN:
		{
			value = M_MEAN_CHAR_WIDTH  ;
		}
		break;
	case eSTR_MIN:
		{
			value = M_MIN_CHAR_WIDTH  ;
		}
		break;
	case eSTR_QUARTER:
		{
			value = M_QUARTER_MAX_CHAR_WIDTH  ;
		}
		break;
	case eSTR_USER:
		{
			value = width ;
		}
		break;
	}

	//StrControl(M_FONT_INDEX(M_ALL),M_SPACE_WIDTH,value);
	StrControl(M_FONT_INDEX(M_ALL),M_SPACE_WIDTH,M_MAX_CHAR_WIDTH);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::SetStrAcceptScore(double score)//2
{
	StrControl(M_STRING_INDEX(M_ALL),M_STRING_ACCEPTANCE, score);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::SetStrTargetAcceptScore(double score)//2
{
	StrControl(M_STRING_INDEX(M_ALL),M_STRING_TARGET_ACCEPTANCE, score);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::SetCharAcceptScore(double score)//2
{
	StrControl(M_STRING_INDEX(M_ALL),M_CHAR_ACCEPTANCE, score);

	return eSTR_SUCCESS;
}

double CProcMil_OCR::GetCharSimilAcceptScore()//2
{
	MIL_DOUBLE score = 0;
	StrInquire(M_STRING_INDEX(0),M_CHAR_SIMILARITY_ACCEPTANCE + M_TYPE_MIL_DOUBLE , &score);

	return score;
}

int CProcMil_OCR::SetCharSimilAcceptScore(double score)//2
{
	StrControl(M_STRING_INDEX(M_ALL),M_CHAR_SIMILARITY_ACCEPTANCE , score);

	return eSTR_SUCCESS;
}

double CProcMil_OCR::GetCharHomoAcceptScore()//2
{
	MIL_DOUBLE score = 0;
	StrInquire(M_STRING_INDEX(0),M_CHAR_HOMOGENEITY_ACCEPTANCE + M_TYPE_MIL_DOUBLE  , &score);

	return score;
}

int CProcMil_OCR::SetCharHomoAcceptScore(double score)//2
{
	StrControl(M_STRING_INDEX(M_ALL),M_CHAR_HOMOGENEITY_ACCEPTANCE  , score);

	return eSTR_SUCCESS;
}



int CProcMil_OCR::SetCharAspectRatio(double maxVal, double minVal)//2
{
	StrControl(M_STRING_INDEX(M_ALL), M_CHAR_ASPECT_RATIO_MIN_FACTOR, minVal);
	StrControl(M_STRING_INDEX(M_ALL), M_CHAR_ASPECT_RATIO_MAX_FACTOR, maxVal);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::SetCharScaleFactor(double maxVal, double minVal)//2
{
	StrControl(M_STRING_INDEX(M_ALL), M_CHAR_SCALE_MIN_FACTOR, minVal);
	StrControl(M_STRING_INDEX(M_ALL), M_CHAR_SCALE_MAX_FACTOR, maxVal);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::SetStringScaleFactor(double maxVal, double minVal)//2
{
	StrControl(M_STRING_INDEX(M_ALL), M_STRING_SCALE_MIN_FACTOR, minVal);
	StrControl(M_STRING_INDEX(M_ALL), M_STRING_SCALE_MAX_FACTOR, maxVal);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::SetEnableCharAngle(bool val)//2
{
	if(val)
		StrControl(M_CONTEXT, M_SEARCH_CHAR_ANGLE, M_ENABLE);
	else
		StrControl(M_CONTEXT, M_SEARCH_CHAR_ANGLE, M_DISABLE);

	return eSTR_SUCCESS;
}



int CProcMil_OCR::SetConsecutiveSpace(int space)//2
{
	StrControl(M_STRING_INDEX(M_ALL),M_SPACE_MAX_CONSECUTIVE , 3);
	//StrControl(M_STRING_INDEX(M_ALL),M_SPACE_MAX_CONSECUTIVE , space);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::SetThreshHoldMode(int treshMode, int value)//2
{
	double threshMode = 0;

	switch(treshMode)
	{
	case eSTR_LOCAL:
		{
			threshMode = M_LOCAL ;
		}
		break;
	case eSTR_LOCAL_WITH_RESEG:
		{
			threshMode = M_LOCAL_WITH_RESEGMENTATION ;
		}
		break;
	case eSTR_USER_DEFINED:
		{ 
			threshMode = M_USER_DEFINED;

			if(value == -1)
			{
				StrControl(M_CONTEXT,M_THRESHOLD_VALUE  , M_AUTO_COMPUTE);
			}
			else
			{
				StrControl(M_CONTEXT,M_THRESHOLD_VALUE  , value);
			}
		}
		break;
	}

	StrControl(M_CONTEXT,M_THRESHOLD_MODE  , threshMode);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::GetStrForeGoundColor()//2
{
	MIL_INT foreGround=0;
	StrInquire(M_STRING_INDEX(0),M_FOREGROUND_VALUE + M_TYPE_MIL_INT, &foreGround );
	return foreGround == M_FOREGROUND_BLACK ? 0 : 1;
}

int CProcMil_OCR::SetStrForeGoundColor(int foreGround)//2
{
	StrControl(M_STRING_INDEX(M_ALL),M_FOREGROUND_VALUE,(foreGround ?  M_FOREGROUND_WHITE : M_FOREGROUND_BLACK) );
	return eSTR_SUCCESS;
}

int CProcMil_OCR::SaveStrFont(CString path)//1
{
	CString fullPath = CreatePath(path, _T("msr"));

 	MstrSave(fullPath,m_milStrContext, M_DEFAULT);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::ModelList_Load(CString strPath)
{
	int ret = eSTR_SUCCESS;

	if (!m_ModelMng->LoadFileList(strPath,m_resolX,m_resolY))
		ret = eSTR_FAIL;

	return ret;
}

int CProcMil_OCR::ModelList_Check(CString strPath)
{
	int ret = eSTR_SUCCESS;

	if (!m_ModelMng->CheckFileList(strPath,m_resolX,m_resolY))
		ret = eSTR_FAIL;

	return ret;
}

int CProcMil_OCR::ModelList_Clear(int nLane)
{
	int ret = eSTR_SUCCESS;

	ret = m_ModelMng->Clear_File(nLane);

	return ret;
}

void CProcMil_OCR::ModelRemove(CString strPath)
{
	m_ModelMng->Remove(strPath);
	m_ModelFile.reset();
}


int CProcMil_OCR::LoadStrFont(CString path, int* retFontCount, long * retFontSize, int nInspectionMode)//2
{
	int ret = eSTR_SUCCESS;

	if(m_milStrContext_Temp != M_NULL)
	{
		MstrFree(m_milStrContext_Temp);
		m_milStrContext_Temp = M_NULL;
	}

	CFileFind fn;
	CString fullPath = CreatePath(path, _T("msr"));

	if(!fn.FindFile(fullPath))	
		return eSTR_FAIL;

	if (nInspectionMode == eMI_Inspection)
		m_ModelFile = m_ModelMng->GetModel(fullPath);
	else
		m_ModelFile.reset();

	Im::PIL_ID rst = M_NULL;
	if (m_ModelFile == NULL)
	{
		rst = MstrRestore(fullPath, *m_milSys, M_DEFAULT, &m_milStrContext_Temp);
		m_milStrContext = m_milStrContext_Temp;
	}
	else
	{
		if(m_nUsecontinue == 1)
		{
			if (m_ModelFile->milModel_90 == M_NULL)
			{
				rst = MstrRestore(fullPath, *m_milSys, M_DEFAULT, &m_ModelFile->milModel_90);
			}
			m_milStrContext = m_ModelFile->milModel_90;
			rst = m_ModelFile->milModel;
		}
		else if(m_nUsecontinue == 2)
		{
			if (m_ModelFile->milModel_180 == M_NULL)
			{
				rst = MstrRestore(fullPath, *m_milSys, M_DEFAULT, &m_ModelFile->milModel_180);
			}
			m_milStrContext = m_ModelFile->milModel_180;
			rst = m_ModelFile->milModel;
		}
		else
		{
			m_milStrContext = m_ModelFile->milModel;
			rst = m_ModelFile->milModel;
		}
	}
												
	if(retFontCount != NULL)
	{														
		*retFontCount = GetFontCount(m_milStrContext);	
		if(*retFontCount == 0)
			return eSTR_FAIL;
	}

	if(rst == M_NULL)
	{
		ret = eSTR_FAIL;
	}
	else
	{
		if(retFontSize != NULL)
		{
			MIL_INT charCnts = 0;
			MstrInquire(m_milStrContext, M_FONT_INDEX(0),  M_NUMBER_OF_CHARS + M_TYPE_MIL_INT , &charCnts);
			//MIL_INT * arrSizeY = new MIL_INT[charCnts];
			MIL_INT * arrSizeY = g_pMManager->pem_new<MIL_INT>(true, charCnts, (PCHAR)__FUNCTION__, __LINE__);
 			MstrInquire(m_milStrContext, M_FONT_INDEX(0), M_CHAR_SIZE_Y+M_TYPE_MIL_INT, arrSizeY);	// Font Y Size 얻기
			*retFontSize = (long)arrSizeY[0];	// 가장 작은 사이즈 찾기
			for(int i = 1; i < charCnts; i++)
			{
				if(arrSizeY[i] < *retFontSize)
					*retFontSize = arrSizeY[i];
			}

 			Delete_1DArray(&arrSizeY);
		}
	}


	return ret;
}

void CProcMil_OCR::SetFontText(CString sFontText)
{
	if(m_ModelFile)
		m_ModelFile->sOcrText = sFontText;
}

CString CProcMil_OCR::GetFontText()
{
	if(m_ModelFile)
		return m_ModelFile->sOcrText;
	else
		return CString();
}

// CString CProcMil_OCR::CreatePath(CString path)
// {
// 	int count = -1;
// 	CString fullPath = _T("");
// 	CString temp = _T("");
// 
// 
// 	for(int i = 0; i<path.GetLength(); i++)
// 	{
// 		temp = path.Mid(i,1);
// 		if(temp == '.')
// 		{
// 			count = i;
// 		}
// 	}
// 
// 	if(count == -1)
// 	{
// 		fullPath.Format(_T("%s.msr"),path);
// 	}
// 	else
// 	{	
// 		temp = path.Mid(count+1,3);
// 		if(temp == _T("msr"))
// 		{
// 			fullPath = 	path;
// 		}
// 		else
// 		{
// 			temp = path.Left(count);
// 			fullPath.Format(_T("%s.msr"),temp); 
// 		}
// 	}
// 
// 	return fullPath;
// }

int CProcMil_OCR::GetFontCount(Im::PIL_ID milContext)
{
	MIL_INT fontCount = 0;
	if(milContext != M_NULL)
	{
		MstrInquire(milContext, M_CONTEXT, M_NUMBER_OF_FONTS+M_TYPE_MIL_INT  ,&fontCount );
	}
	else
	{
		if(m_milStrContext == M_NULL)
		{	
			fontCount = 0;
		}
		else
		{
			MstrInquire(m_milStrContext, M_CONTEXT, M_NUMBER_OF_FONTS+M_TYPE_MIL_INT  ,&fontCount );
		}		
	}

	return (int)fontCount;
}


int CProcMil_OCR::SetConstraint(int charPos, int type, wchar_t charList)//2
{
	MIL_INT constraintType;
	
	switch(type)
	{
	case eSTR_ANY:
		{
			constraintType = M_ANY;
		}
		break;
	case eSTR_DIGIT:
		{
			constraintType = M_DIGIT ;
		}
		break;
	case eSTR_LETTER:
		{
			constraintType = M_LETTER ;
		}
		break;
	case eSTR_LOWERCASE:
		{
			constraintType = M_LETTER + M_LOWERCASE;
		}
		break;
	case eSTR_UPPERCASE:
		{
			constraintType = M_LETTER + M_UPPERCASE;
		}
		break;
	}
	
	/*if(charList == NULL)
	{
		MstrSetConstraint(m_milStrContext, M_STRING_INDEX(M_ALL), (MIL_INT)charPos, M_ANY, M_NULL);
	}
	else
	{*/
		MstrSetConstraint(m_milStrContext, M_STRING_INDEX(M_ALL), (MIL_INT)charPos, constraintType, &charList);
	//}
	

	return eSTR_SUCCESS;
}

bool CProcMil_OCR::IsPreprocess()
{
	bool ret = false;
	if(m_milStrContext == M_NULL)
		return ret;

	MIL_INT isOk = 0;
	MstrInquire(m_milStrContext, M_CONTEXT, M_PREPROCESSED + M_TYPE_MIL_INT, &isOk);

	if(isOk == 0)
		ret = false;
	else
		ret = true;


	return ret;
}

int CProcMil_OCR::GetStringModelCount()
{
	return MstrInquire(m_milStrContext, M_CONTEXT, M_NUMBER_OF_STRING_MODELS, M_NULL);
}

void CProcMil_OCR::Preprocess()
{
	if(m_milStrContext == M_NULL)
		return;

	MstrPreprocess(m_milStrContext, M_DEFAULT);
}

void CProcMil_OCR::UnPreprocess()
{
	if(m_milStrContext == M_NULL)
		return;

	MstrPreprocess(m_milStrContext, M_RESET);
}

int CProcMil_OCR::StringReadStart(UCHAR* userBuff, int sizeX, int sizeY)//2
{
	if(m_milStrContext == M_NULL || userBuff == NULL || sizeX <= 0 || sizeY <= 0)
		return eSTR_FAIL;	

	Im::PIL_ID tempMilBuf = M_NULL;
	tempMilBuf = AllocBuff(sizeX, sizeY);
	MbufPut(tempMilBuf, userBuff);

	SaveWorkImg(tempMilBuf, _T("OCR_SRC.bmp"));

	//huj 2014/03/06
	//MstrPreprocess(m_milStrContext, M_DEFAULT); 
	if(!IsPreprocess())
		Preprocess();

	MstrRead(m_milStrContext, tempMilBuf, m_milStrResult);
	//MstrPreprocess(m_milStrContext, M_RESET);


	FreeMilImageBuff(&tempMilBuf);

	int cnts = 0;
	GetStringCount(&cnts);

	if(cnts < 1)
		return 0;


	return cnts;
}

int CProcMil_OCR::StringReadStart(void* milBuffID, int cX, int cY, int sizeX, int sizeY, double angle, int foreGround)//2
{
	Im::PIL_ID tempMilBuf = M_NULL;
	Im::PIL_ID tempMilBuf_rotate = M_NULL;
	int threshold = 0;

	if(m_milStrContext == M_NULL)
		return eSTR_FAIL;

	SaveWorkImg((Im::PIL_ID)milBuffID, _T("OCR_SRC.bmp"));

	tempMilBuf = AllocClipBuff((Im::PIL_ID)milBuffID, cX, cY, sizeX, sizeY);
	SaveWorkImg(tempMilBuf, _T("ocr_clip1.bmp"));


	if(angle > 0)
	{
		tempMilBuf_rotate = AllocRotateBuff(tempMilBuf, angle);
		FreeMilImageBuff(&tempMilBuf);
	}
	else
	{
		tempMilBuf_rotate = AllocBuff(sizeX, sizeY);
		MbufCopy(tempMilBuf, tempMilBuf_rotate);
		FreeMilImageBuff(&tempMilBuf);
	}
	SaveWorkImg(tempMilBuf_rotate, _T("ocr_clip2.bmp"));


	//huj 2014/03/06
	//MstrPreprocess(m_milStrContext, M_DEFAULT); 
	MstrRead(m_milStrContext, (Im::PIL_ID)tempMilBuf_rotate, m_milStrResult);
	//MstrPreprocess(m_milStrContext, M_RESET);

	FreeMilImageBuff(&tempMilBuf_rotate);

	int cnts = 0;
	GetStringCount(&cnts);

	if(cnts < 1)
		return eSTR_FAIL;
	

	return eSTR_SUCCESS;
}

BOOL CProcMil_OCR::IsFoundStr()
{
	int strCount = 0;
	BOOL ret = FALSE;

	GetStringCount(&strCount);

	if(strCount > 0)
	{
		ret = TRUE;
	}

	return ret;
}

int CProcMil_OCR::GetStringCount(int* count)
{
	MIL_INT numberOfStringRead;

	MstrGetResult(m_milStrResult, M_GENERAL, M_STRING_NUMBER + M_TYPE_MIL_INT, &numberOfStringRead);
	*count = (int)numberOfStringRead;

	return eSTR_SUCCESS;
}

int CProcMil_OCR::GetCharCount(int* count)
{
	MIL_INT numberOfCharRead;

	MstrGetResult(m_milStrResult, M_GENERAL, M_CHAR_NUMBER + M_TYPE_MIL_INT, &numberOfCharRead);
	if(numberOfCharRead < 0 || numberOfCharRead > MAX_STRLEN-1)
		numberOfCharRead = 0;

	*count = (int)numberOfCharRead;

	return eSTR_SUCCESS;
}

CString CProcMil_OCR::GetString(int index)	
{
	MIL_TEXT_CHAR  stringResult[MAX_STRLEN];
	CString result = _T("");

	if(!IsFoundStr())
	{
		return _T("");
	}
	
	MstrGetResult(m_milStrResult, index, M_STRING+M_TYPE_TEXT_CHAR, stringResult);
	if(stringResult[0] != '\0')
		result.Format(_T("%s"),stringResult);

	return result;
}

int CProcMil_OCR::GetStringScore(double* score,int index)
{
	double stringScore[20];

	if(!IsFoundStr())
	{
		return eSTR_FAIL;
	}

	MstrGetResult(m_milStrResult, index,  M_STRING_SCORE + M_TYPE_DOUBLE      , stringScore);
	*score = stringScore[0];

	return eSTR_SUCCESS;
}

int CProcMil_OCR::GetCharScore(double* score, int index)
{
	double* charScore = NULL;
	
	int charCount = 0;
	GetStringCount(&charCount);

	if(charCount <= 0)
	{
		return eSTR_FAIL;
	}

	//charScore = new double[MAX_STRLEN];
	charScore = g_pMManager->pem_new<double>(true, MAX_STRLEN, (PCHAR)__FUNCTION__, __LINE__);
	
	for(int i = 0; i<MAX_STRLEN; i++)
	{
		charScore[i] = -1.;
	}

	MstrGetResult(m_milStrResult, index,  M_CHAR_SCORE + M_TYPE_DOUBLE , charScore);
	memcpy(score, charScore, sizeof(double)*MAX_STRLEN);

	//delete charScore;
	g_pMManager->pem_delete(charScore, false);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::GetStringAngle(double* angle,int index)
{
	double strAngle[20];

	if(!IsFoundStr())
	{
		return eSTR_FAIL;
	}


	MstrGetResult(m_milStrResult, index,  M_STRING_ANGLE  + M_TYPE_DOUBLE , strAngle);
	*angle = strAngle[0];

	return eSTR_SUCCESS;
}

int CProcMil_OCR::GetStringPos(double* x, double* y,int index)
{
	double strPosX[20];
	double strPosY[20];

	if(!IsFoundStr())
	{
		return eSTR_FAIL;
	}


	MstrGetResult(m_milStrResult, index,   M_STRING_POSITION_X  + M_TYPE_DOUBLE , strPosX);
	MstrGetResult(m_milStrResult, index,   M_STRING_POSITION_Y  + M_TYPE_DOUBLE , strPosY);

	*x = strPosX[0];
	*y = strPosY[0];

	return eSTR_SUCCESS;
}



void CProcMil_OCR::GetStringImgSize(int* retOffsetX, int* retOffsetY, int* retWidth, int* retHeight)
{
	int offsetX = 0;
	int offsetY = 0;
	int width = 0;
	int height = 0;

	//offsetX
	MIL_INT ulX = 0;
	MIL_INT dlX = 0;
	MstrGetResult(m_milStrResult, 0, M_STRING_BOX_UL_X + M_TYPE_MIL_INT, &ulX);
	MstrGetResult(m_milStrResult, 0, M_STRING_BOX_BL_X + M_TYPE_MIL_INT, &dlX);

	offsetX = (int)((ulX > dlX)?  dlX : ulX);


	//offsetY
	MIL_INT ulY = 0;
	MIL_INT urY = 0;
	MstrGetResult(m_milStrResult, 0, M_STRING_BOX_UL_Y + M_TYPE_MIL_INT, &ulY);
	MstrGetResult(m_milStrResult, 0, M_STRING_BOX_UL_Y + M_TYPE_MIL_INT, &urY);

	offsetY = (int)((ulY > urY)?  urY : ulY);

	//size
	MIL_INT brX = 0;
	MIL_INT brY = 0;
	MstrGetResult(m_milStrResult, 0, M_STRING_BOX_BR_X + M_TYPE_MIL_INT, &brX);
	MstrGetResult(m_milStrResult, 0, M_STRING_BOX_BR_Y + M_TYPE_MIL_INT, &brY);

	width = (int)brX;
	height = (int)brY;



	*retOffsetX = offsetX;
	*retOffsetY = offsetY;
	*retWidth = width;
	*retHeight = height;
}

void CProcMil_OCR::GetFontImgSize(int fontIndex,int* retOffsetX, int* retOffsetY, int* retWidth, int* retHeight)
{
	MIL_INT charCnts = 0;
	MstrInquire(m_milStrContext, M_FONT_INDEX(fontIndex),  M_NUMBER_OF_CHARS + M_TYPE_MIL_INT , &charCnts);

	if(charCnts <= 0)
		return ;


	MIL_INT*  charSizeX = NULL;
	MIL_INT*  charSizeY = NULL;
	MIL_INT*  offsetX = NULL;
	MIL_INT*  offsetY = NULL;
	MIL_INT marginX = 0;
	MIL_INT marginY = 0;
	/*charSizeX = new MIL_INT[(int)charCnts];
	charSizeY = new MIL_INT[(int)charCnts];
	offsetX = new MIL_INT[(int)charCnts];
	offsetY = new MIL_INT[(int)charCnts];*/
	charSizeX = g_pMManager->pem_new<MIL_INT>(true, (int)charCnts, (PCHAR)__FUNCTION__, __LINE__);
	charSizeY = g_pMManager->pem_new<MIL_INT>(true, (int)charCnts, (PCHAR)__FUNCTION__, __LINE__);
	offsetX = g_pMManager->pem_new<MIL_INT>(true, (int)charCnts, (PCHAR)__FUNCTION__, __LINE__);
	offsetY = g_pMManager->pem_new<MIL_INT>(true, (int)charCnts, (PCHAR)__FUNCTION__, __LINE__);

	MstrInquire(m_milStrContext, M_FONT_INDEX(fontIndex), M_CHAR_SIZE_X + M_TYPE_MIL_INT, charSizeX);
	MstrInquire(m_milStrContext, M_FONT_INDEX(fontIndex), M_CHAR_SIZE_Y + M_TYPE_MIL_INT, charSizeY);
	MstrInquire(m_milStrContext, M_FONT_INDEX(fontIndex), M_DEFINITION_OFFSET_X + M_TYPE_MIL_INT, offsetX);
	MstrInquire(m_milStrContext, M_FONT_INDEX(fontIndex), M_DEFINITION_OFFSET_Y + M_TYPE_MIL_INT, offsetY);
	MstrInquire(m_milStrContext, M_FONT_INDEX(fontIndex) ,M_DRAW_BOX_MARGIN_X + M_TYPE_MIL_INT , &marginX);
	MstrInquire(m_milStrContext, M_FONT_INDEX(fontIndex) ,M_DRAW_BOX_MARGIN_Y + M_TYPE_MIL_INT , &marginY);

	int width = 0;
	int height = (int)charSizeY[0];

	for(int i = 0; i < charCnts; i++)
	{
		width += (int)charSizeX[i];

		if(height < (int)charSizeY[i])
		{
			height = (int)charSizeY[i];
		}
	}

	width += (int)(marginX * (charCnts * 2));
	height += (int)(marginX * 2);
	 
	*retOffsetX = (int)(offsetX[0] + marginX);
	*retOffsetY = (int)(offsetY[0] + marginY);
	*retWidth = width;
	*retHeight = height;

	/*delete charSizeX;
	delete charSizeY;
	delete offsetX;
	delete offsetY;*/
	g_pMManager->pem_delete(charSizeX, false);
	g_pMManager->pem_delete(charSizeY, false);
	g_pMManager->pem_delete(offsetX, false);
	g_pMManager->pem_delete(offsetY, false);
}

CString CProcMil_OCR::GetFontCharValue(int fontIndex)
{
	CString font = _T("");

	if(m_milStrContext == M_NULL)
		return font;


	MIL_INT charCnts = 0;
	MstrInquire(m_milStrContext, M_FONT_INDEX(fontIndex),  M_NUMBER_OF_CHARS + M_TYPE_MIL_INT , &charCnts);

	if(charCnts <= 0)
		return font;


	MIL_TEXT_CHAR fontChar[MAX_STRLEN] ;
	//memset(fontChar, 0, sizeof(MIL_TEXT_CHAR) *  (int)charCnts);
	MstrInquire(m_milStrContext, M_FONT_INDEX(fontIndex) ,M_CHAR_VALUE + M_TYPE_TEXT_CHAR , fontChar);	

	if(fontChar[0] != '\0')
	{	
		for(int i = 0; i < charCnts; i++)
		{
			font += fontChar[i];
			//font.Format(_T("%s"),fontChar);
		}
	}


	return font;
}



int CProcMil_OCR::GetBuffDrawFont(Im::PIL_ID dst, MIL_DOUBLE color, int fontIndex)//1
{
	StrControl(M_FONT_INDEX(fontIndex),M_DRAW_BOX_MARGIN_X ,1);
	StrControl(M_FONT_INDEX(fontIndex),M_DRAW_BOX_MARGIN_Y ,1);
	MgraColor(M_DEFAULT, color);
	MstrDraw(M_DEFAULT, m_milStrContext, dst, M_DRAW_CHAR, M_FONT_INDEX(fontIndex), M_NULL, M_DEFAULT); 

	return eSTR_SUCCESS;
}

int CProcMil_OCR::GetBuffDrawFont(UCHAR* dst, int sizeX, int sizeY, MIL_DOUBLE color, int fontIndex)//1
{
	Im::PIL_ID tempMilBuf = M_NULL;
	Im::PIL_ID tempDst = M_NULL;
	int offsetX = 0;
	int offsetY = 0;
	int width = 0;
	int height = 0;

	StrControl(M_FONT_INDEX(fontIndex),M_DRAW_BOX_MARGIN_X ,1);
	StrControl(M_FONT_INDEX(fontIndex),M_DRAW_BOX_MARGIN_Y ,1);
	GetFontImgSize(fontIndex, &offsetX, &offsetY, &width, &height);

	if(color == NULL)	
	{
		tempMilBuf = AllocBuff(width, height, 0);
		tempDst = AllocBuff(sizeX, sizeY, 0);
	}
	else
	{
		if(color == COLOR_BLACK)
		{
			tempMilBuf = AllocBuffColor(width, height, 255);	
			tempDst = AllocBuffColor(sizeX , sizeY, 255);	
		}
		else
		{
			tempMilBuf = AllocBuffColor(width, height, 0);	
			tempDst = AllocBuffColor(sizeX , sizeY, 0);	
		}		
	}


	MgraColor(M_DEFAULT, color);
	MstrDraw(M_DEFAULT, m_milStrContext, tempMilBuf, M_DRAW_CHAR, M_FONT_INDEX(fontIndex), M_NULL, M_DEFAULT); 


	double factor = GetResizeFactor(sizeX, sizeY, width, height);
	ResizeImg(tempMilBuf, tempDst, factor, TRUE);

	if(color != NULL)	
	{
		MbufGetColor(tempDst, M_PACKED+M_BGR24, M_ALL_BANDS, dst);
	}
	else
	{
		MbufGet(tempDst, dst);
	}

	FreeMilImageBuff(&tempMilBuf);
	FreeMilImageBuff(&tempDst);

	return eSTR_SUCCESS;
}
int CProcMil_OCR::GetBuffDrawDefineFont(UCHAR* dst, int sizeX, int sizeY, MIL_DOUBLE color, int fontIndex,CString OKFont)
{
//	MIL_TEXT_CHAR fontChar[MAX_STRLEN] ;
// 	for(int i = 0; i < OKFont.GetLength(); i++)
// 	{
// 		fontChar[i] = OKFont.GetAt(i);
// 		//font.Format(_T("%s"),fontChar);
// 	}
// 	fontChar[OKFont.GetLength()] = '\0';


	char fontChar[MAX_STRLEN];

	wcstombs_s(NULL, fontChar, MAX_STRLEN, OKFont, _TRUNCATE);

	Im::PIL_ID tempMilBuf = M_NULL;
	Im::PIL_ID tempDst = M_NULL;
	int offsetX = 0;
	int offsetY = 0;
	int width = 0;
	int height = 0;

	StrControl(M_FONT_INDEX(fontIndex),M_DRAW_BOX_MARGIN_X ,1);
	StrControl(M_FONT_INDEX(fontIndex),M_DRAW_BOX_MARGIN_Y ,1);
	GetFontImgSize(fontIndex, &offsetX, &offsetY, &width, &height);

	if(color == NULL)	
	{
		tempMilBuf = AllocBuff(width, height, 0);
		tempDst = AllocBuff(sizeX, sizeY, 0);
	}
	else
	{
		if(color == COLOR_BLACK)
		{
			tempMilBuf = AllocBuffColor(width, height, 255);	
			tempDst = AllocBuffColor(sizeX , sizeY, 255);	
		}
		else
		{
			tempMilBuf = AllocBuffColor(width, height, 0);	
			tempDst = AllocBuffColor(sizeX , sizeY, 0);	
		}		
	}

	MgraColor(M_DEFAULT, color);
	MstrDraw(M_DEFAULT, m_milStrContext, tempMilBuf, M_DRAW_CHAR, M_FONT_INDEX(fontIndex), fontChar, M_DEFAULT); 

	double factor = GetResizeFactor(sizeX, sizeY, width, height);
	ResizeImg(tempMilBuf, tempDst, factor, TRUE);

	if(color != NULL)	
	{
		MbufGetColor(tempDst, M_PACKED+M_BGR24, M_ALL_BANDS, dst);
	}
	else
	{
		MbufGet(tempDst, dst);
	}

	FreeMilImageBuff(&tempMilBuf);
	FreeMilImageBuff(&tempDst);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::DrawString(UCHAR* srcImg, int srcSizeX, int srcSizeY, UCHAR* dstImg)
{
	Im::PIL_ID milSrc = M_NULL;
	milSrc = AllocBuff(srcSizeX, srcSizeY);
	MbufPut(milSrc, srcImg);

	Im::PIL_ID milColorSrc = M_NULL;
	milColorSrc = AllocBuffColor(srcSizeX, srcSizeY, 0);
	MbufCopyColor(milSrc, milColorSrc, M_ALL_BANDS);
	FreeMilImageBuff(&milSrc);

	MgraColor(M_DEFAULT, M_COLOR_RED);
	MstrDraw(M_DEFAULT, m_milStrResult, milColorSrc, M_DRAW_STRING, M_ALL, M_NULL, M_DEFAULT);
	MgraColor(M_DEFAULT, M_COLOR_WHITE);

	SaveWorkImg(milColorSrc, _T("strImg.bmp"));

	FreeMilImageBuff(&milColorSrc);

	return eSTR_SUCCESS;
}


int CProcMil_OCR::GetBuffDrawString(Im::PIL_ID dst, MIL_DOUBLE color)
{
	MgraColor(M_DEFAULT, color);
	MstrDraw(M_DEFAULT, m_milStrResult, dst, M_DRAW_STRING, M_ALL, M_NULL, M_DEFAULT); 
	MgraColor(M_DEFAULT, M_COLOR_WHITE);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::GetBuffDrawString(void* milSrc, UCHAR* dst, int cx, int cy, int srcWidth, int srcHeight, int dstWidth, int dstHeight, double angle, MIL_DOUBLE color)
{
	Im::PIL_ID tempMilBuf = M_NULL;
	Im::PIL_ID tempMilsrc = M_NULL;
	Im::PIL_ID moveMilBuf = M_NULL;
	Im::PIL_ID tempDst = M_NULL;
	Im::PIL_ID srcClipBuf = M_NULL;
	Im::PIL_ID tempSingleBand = M_NULL;

	int margin = 50;
	int orgSrcX = srcWidth + margin;
	int orgSrcY = srcHeight + margin;
	int srcX = 0; 
	int srcY = 0;
	if(angle == 90 || angle == 270)
	{
		srcX = srcHeight + margin;
		srcY = srcWidth + margin;
	}
	else
	{
		srcX = srcWidth + margin;
		srcY = srcHeight + margin;
	}

	int dx = margin / 2;
	int dy = margin / 2;

	if(color == NULL)
	{
		tempMilBuf = AllocBuff(srcX , srcY, 0);
		moveMilBuf = AllocBuff(srcX , srcY, 0);
		tempDst = AllocBuff(dstWidth , dstHeight, 0);
	}
	else
	{
		if(color == COLOR_BLACK)
		{
			tempMilBuf = AllocBuffColor(srcX , srcY, 255);
			moveMilBuf = AllocBuffColor(srcX , srcY, 255);
			tempDst = AllocBuffColor(dstWidth , dstHeight, 255);
		}
		else
		{
			tempMilBuf = AllocBuffColor(srcX , srcY, 0);
			moveMilBuf = AllocBuffColor(srcX , srcY, 0);
			tempDst = AllocBuffColor(dstWidth , dstHeight, 0);
		}		
	}

	
	if(angle > 0)
	{
		srcClipBuf = AllocClipBuff((Im::PIL_ID)milSrc, cx, cy, orgSrcX, orgSrcY);
		tempMilsrc = AllocRotateBuff(srcClipBuf, angle);
		FreeMilImageBuff(&srcClipBuf);
	}
	else
	{
		tempMilsrc = AllocClipBuff((Im::PIL_ID)milSrc, cx, cy, orgSrcX, orgSrcY);	
	}

	MgraColor(M_DEFAULT, color);
	MstrDraw(M_DEFAULT, m_milStrResult, tempMilBuf, M_DRAW_STRING, M_ALL, M_NULL, M_DEFAULT); 
	MgraColor(M_DEFAULT, M_COLOR_WHITE);

	MbufCopyClip(tempMilBuf, moveMilBuf, (MIL_INT)dx, (MIL_INT)dy);
	

	tempSingleBand = AllocBuff(srcX, srcY, 0);

	MbufClear(tempSingleBand, 0);
	MbufCopyColor(moveMilBuf, tempSingleBand, M_RED);
	ArithImage(tempMilsrc, tempSingleBand, tempMilsrc, M_SUB);

	MbufClear(tempSingleBand, 0);
	MbufCopyColor(moveMilBuf, tempSingleBand, M_GREEN);
	ArithImage(tempMilsrc, tempSingleBand, tempMilsrc, M_SUB);

	MbufClear(tempSingleBand, 0);
	MbufCopyColor(moveMilBuf, tempSingleBand, M_BLUE);
	ArithImage(tempMilsrc, tempSingleBand, tempMilsrc, M_SUB);


	Im::PIL_ID tempColor = AllocBuffColor(srcX, srcY, 0);
	ArithImage(tempMilsrc, moveMilBuf, tempColor, M_OR);

	double factor;
	if(angle == 90 || angle == 270)
	{
		factor = GetResizeFactor(dstWidth, dstHeight, srcX, srcY, TRUE);
	}
	else
	{
		factor = GetResizeFactor(dstWidth, dstHeight, srcX, srcY, FALSE);
	}	
	ResizeImg(tempColor, tempDst, factor, TRUE);



	if(color != NULL)	
	{
		MbufGetColor(tempDst, M_PACKED + M_BGR24, M_ALL_BANDS, dst);
	}
	else
	{
		MbufGet(tempMilBuf, dst);
	}

	FreeMilImageBuff(&tempMilsrc);
	FreeMilImageBuff(&moveMilBuf);
	FreeMilImageBuff(&tempDst);
	FreeMilImageBuff(&tempMilBuf);
	FreeMilImageBuff(&tempSingleBand);
	FreeMilImageBuff(&tempColor);
	
	return eSTR_SUCCESS;
}

// int CProcMil_OCR::GetBuffDrawString(UCHAR* dst, int sizeX, int sizeY, MIL_DOUBLE color)
// {
// 	Im::PIL_ID tempMilBuf;
// 	Im::PIL_ID tempDst = M_NULL;
// 	int offsetX = 0;
// 	int offsetY = 0;
// 	int width = 0;
// 	int height = 0;
// 
// 	GetStringImgSize(&offsetX, &offsetY, &width, &height);
// 
// 	if(color == NULL)
// 	{
// 		tempMilBuf = AllocBuff(width , height, 0);
// 		tempDst = AllocBuff(sizeX , sizeY, 0);
// 	}
// 	else
// 	{
// 		if(color == COLOR_BLACK)
// 		{
// 			tempMilBuf = AllocBuffColor(width , height, 255);	
// 			tempDst = AllocBuffColor(sizeX , sizeY, 255);
// 		}
// 		else
// 		{
// 			tempMilBuf = AllocBuffColor(width , height, 0);	
// 			tempDst = AllocBuffColor(sizeX , sizeY, 0);
// 		}		
// 	}
// 
// 	MgraColor(M_DEFAULT, color);
// 	MstrDraw(M_DEFAULT, m_milStrResult, tempMilBuf, M_DRAW_STRING, M_ALL, M_NULL, M_DEFAULT); 
// 	
// 	MbufGetColor(tempMilBuf, M_PACKED + M_BGR24, M_ALL_BANDS, dst);
// 
// 	FreeMilImageBuff(&tempMilBuf);
// 	FreeMilImageBuff(&tempDst);
// 
// 	return eSTR_SUCCESS;
// }

int CProcMil_OCR::GetBuffDrawBox(Im::PIL_ID dst, MIL_DOUBLE color)
{
	MgraColor(M_DEFAULT, color);
	MstrDraw(M_DEFAULT, m_milStrResult, dst, M_DRAW_STRING_BOX, M_ALL, M_NULL, M_DEFAULT); 

	return eSTR_SUCCESS;
}

int CProcMil_OCR::GetBuffDrawBox(UCHAR* dst, int sizeX, int sizeY, MIL_DOUBLE color)
{
	Im::PIL_ID tempMilBuf;

	if(color == NULL)
	{
		tempMilBuf = AllocBuff(sizeX , sizeY, 0);
	}
	else
	{
		if(color == COLOR_BLACK)
		{
			tempMilBuf = AllocBuffColor(sizeX , sizeY, 255);	

		}
		else
		{
			tempMilBuf = AllocBuffColor(sizeX , sizeY, 0);	
		}		
	}



	MgraColor(M_DEFAULT, color);
	MstrDraw(M_DEFAULT, m_milStrResult, tempMilBuf, M_DRAW_STRING_BOX, M_ALL, M_NULL, M_DEFAULT); 

	MbufGetColor(tempMilBuf, M_PACKED + M_BGR24, M_ALL_BANDS, dst);

	FreeMilImageBuff(&tempMilBuf);

	return eSTR_SUCCESS;
}

/*
int CProcMil_OCR::GetThreshold(Im::PIL_ID milSrc)
{
	int temp[256] = {0,};
	int ret = 0;

	GetHistoValue(milSrc, temp);


	MIL_INT sizeX = (int)MbufInquire(milSrc,M_SIZE_X,M_NULL);
	MIL_INT sizeY = (int)MbufInquire(milSrc,M_SIZE_Y,M_NULL);


	int T = 0 ,Told = 0;
	float area = (float)((int)sizeX * (int)sizeY);
	float hist[256];
	float sum = 0.f;


	for(int i=0; i<256; i++)
	{
		hist[i] = temp[i]/area;
	}

	for(int i=0; i<256; i++)
	{
		sum += (i*hist[i]);
	}
	T = (int)sum;

	if(T<0)
	{
		return -1;
	}

	float a1,b1,u1,a2,b2,u2;

	do{
		Told = T;

		a1 = b1 = 0;

		for(int i=0; i<=Told; i++)
		{
			a1 += (i*hist[i]);
			b1 += hist[i];
		}

		u1 = a1/b1;

		a2 = b2 = 0;

		for(int i=Told+1; i<256; i++)
		{
			a2 += (i*hist[i]);
			b2 += hist[i];
		}
		u2 = a2/b2;

		if(b1 == 0)
			b1 = 1.f;
		if(b2 == 0)
			b2 = 1.f;

		T = (int)((u1+u2)/2);

		if(T<0)
		{
			return 150;
		}
	}while(T != Told);

	ret = T;

	if(ret<=0 || ret>=255)
		return -1;

	return ret;
}

*/

void CProcMil_OCR::SetDrawLine(void * milBuff, int lineCnt, int * linePos, int * lineWidth, int foreGround)
{
	if(lineCnt < 1)
		return;

	MIL_INT sizeY = MbufInquire((Im::PIL_ID)milBuff, M_SIZE_Y, M_NULL);

	if(foreGround == 0)
		MgraColor(M_DEFAULT, M_COLOR_WHITE);
	else		// 1
		MgraColor(M_DEFAULT, M_COLOR_BLACK);


	for(int n = 0; n < lineCnt; n++)
	{
		for(int m = 0; m < lineWidth[n]; m++)
		{
			MgraLine(M_DEFAULT, (Im::PIL_ID)milBuff, (MIL_DOUBLE )linePos[n]+m, (MIL_DOUBLE )0, (MIL_DOUBLE )linePos[n]+m, (MIL_DOUBLE )sizeY);
		}
	}
}

//test
void CProcMil_OCR::MakeImg(void* srcT_R, void* srcT_G, void* srcT_B, void* srcM_R, void* srcM_B, void* srcB_R, void* srcB_B)
{
	CSize size = GetImageSize((Im::PIL_ID)srcT_R);

	Im::PIL_ID milSrcTop_R = M_NULL;
	milSrcTop_R = AllocBuff(size.cx, size.cy, 0);
	MbufCopy((Im::PIL_ID)srcT_R, milSrcTop_R);
	SaveWorkImg(milSrcTop_R, _T("ocr\\_Top_Red.bmp"));

	Im::PIL_ID milSrcTop_G = M_NULL;
	milSrcTop_G = AllocBuff(size.cx, size.cy, 0);
	MbufCopy((Im::PIL_ID)srcT_G, milSrcTop_G);
	SaveWorkImg(milSrcTop_G, _T("ocr\\_Top_Green.bmp"));

	Im::PIL_ID milSrcTop_B = M_NULL;
	milSrcTop_B = AllocBuff(size.cx, size.cy, 0);
	MbufCopy((Im::PIL_ID)srcT_B, milSrcTop_B);
	SaveWorkImg(milSrcTop_B, _T("ocr\\_Top_Blue.bmp"));

	Im::PIL_ID milSrcMid_R = M_NULL;
	milSrcMid_R = AllocBuff(size.cx, size.cy, 0);
	MbufCopy((Im::PIL_ID)srcM_R, milSrcMid_R);
	SaveWorkImg(milSrcMid_R, _T("ocr\\_Mid_Red.bmp"));

	Im::PIL_ID milSrcMid_B = M_NULL;
	milSrcMid_B = AllocBuff(size.cx, size.cy, 0);
	MbufCopy((Im::PIL_ID)srcM_B, milSrcMid_B);
	SaveWorkImg(milSrcMid_B, _T("ocr\\_Mid_Blue.bmp"));

	Im::PIL_ID milSrcBot_R = M_NULL;
	milSrcBot_R = AllocBuff(size.cx, size.cy, 0);
	MbufCopy((Im::PIL_ID)srcB_R, milSrcBot_R);
	SaveWorkImg(milSrcBot_R, _T("ocr\\_Bot_Red.bmp"));

	Im::PIL_ID milSrcBot_B = M_NULL;
	milSrcBot_B = AllocBuff(size.cx, size.cy, 0);
	MbufCopy((Im::PIL_ID)srcB_B, milSrcBot_B);
	SaveWorkImg(milSrcBot_B, _T("ocr\\_Bot_Blue.bmp"));

	Im::PIL_ID milDst = M_NULL;
	milDst = AllocBuff(size.cx, size.cy, 0);

	ArithImage(milSrcMid_R, milSrcBot_R, milDst, M_ADD, TRUE);
	SaveWorkImg(milDst, _T("ocr\\1_MidR_BotR_ADD.bmp"));

	MbufClear(milDst, 0);
	ArithImage(milSrcTop_R, milSrcTop_G, milDst, M_ADD, TRUE);
	ArithImage(milDst, milSrcTop_B, milDst, M_ADD, TRUE);
	SaveWorkImg(milDst, _T("ocr\\2_TopR_TopG_TopB_ADD.bmp"));

	MbufClear(milDst, 0);
	ArithImage(milSrcMid_R, milSrcMid_B, milDst, M_ADD, TRUE);
	SaveWorkImg(milDst, _T("ocr\\3_MidR_MidB_ADD.bmp"));




	//////////////////////////////////////////////////////////////////////////
	FreeMilImageBuff(&milSrcTop_R);
	FreeMilImageBuff(&milSrcTop_G);
	FreeMilImageBuff(&milSrcTop_B);

	FreeMilImageBuff(&milSrcMid_R);
	FreeMilImageBuff(&milSrcMid_B);

	FreeMilImageBuff(&milSrcBot_R);
	FreeMilImageBuff(&milSrcBot_B);

	FreeMilImageBuff(&milDst);
}

//shk 2014/04/21
void CProcMil_OCR::TopHatFiltering(UCHAR* srcImg,UCHAR* dstImg,int sizeX,int sizeY,int foreGround)
{
	Im::PIL_ID tempMilBuf = M_NULL;
	tempMilBuf = AllocBuff(sizeX, sizeY);
	MbufPut(tempMilBuf, srcImg);

	Im::PIL_ID tempMilBuf2 = M_NULL;
	tempMilBuf2 = AllocBuff(sizeX, sizeY);

	MbufCopy(tempMilBuf,tempMilBuf2);

	//	SaveWorkImg(tempMilBuf2,_T("MimArith_before.bmp"));
	MimArith(tempMilBuf2,tempMilBuf2,tempMilBuf2,M_NEG);
	//	SaveWorkImg(tempMilBuf2,_T("MimArith_after.bmp"));

	int temp2ChildTop = 0;
	int temp2ChildHeight = 0;

	if(sizeY > 30)
		temp2ChildTop = (int)(sizeY /2)-(int)(sizeY * 0.3);

	if(sizeY > 30)
		temp2ChildHeight = (int)(sizeY * 0.6);
	else
		temp2ChildHeight = sizeY;


	Im::PIL_ID tempMilBuf2_C = M_NULL;
	MbufChild2d(tempMilBuf2,0,temp2ChildTop,sizeX,temp2ChildHeight,&tempMilBuf2_C);

	MimHistogramEqualize(tempMilBuf2_C, tempMilBuf2_C, M_UNIFORM , M_EXPONENTIAL, 0, 255) ;
	//SaveWorkImg(tempMilBuf2_C,_T("MimHistogramEqualize_after.bmp"));
	int thres = GetOtsuThreshold(tempMilBuf2_C);

	Binarize(tempMilBuf2_C,tempMilBuf2_C,thres,foreGround/*TRUE*/);
	//SaveWorkImg(tempMilBuf2_C,_T("Binarize_after.bmp"));
	CMilBlobResult* blobResult = NULL;
	//blobResult = new CMilBlobResult(1);

	MakeBlob2(tempMilBuf2_C,tempMilBuf2_C,!foreGround/*0*/,1,&blobResult);

	if(blobResult == NULL)
		return;

	if ( blobResult->left[0] == 0)
	{
		//delete blobResult;
		g_pMManager->pem_delete(blobResult, false);
		blobResult = NULL;

		MbufFree(tempMilBuf2_C);
		MbufFree(tempMilBuf);
		MbufFree(tempMilBuf2);
		return;
	}

	int nWidth = 0;
	int nHeight = 0;

	blobResult->left[0] += 3; //left margin +3
	blobResult->right[0] -= 3;

	nWidth = (int)(blobResult->right[0]) - (int)(blobResult->left[0]);

	if( blobResult->top[0] > 0)
	{
		blobResult->top[0] += 1;
		temp2ChildTop += (int)(blobResult->top[0]);
	}

	if(temp2ChildHeight > blobResult->bottom[0])
	{
		blobResult->bottom[0] -= 1;
		temp2ChildHeight = (int)(blobResult->bottom[0]);
	}

	nHeight = temp2ChildHeight;

	Im::PIL_ID tempMilBuf_C = M_NULL;
	MbufChild2d(tempMilBuf,(int)(blobResult->left[0]),temp2ChildTop,nWidth,nHeight,&tempMilBuf_C);

	Im::PIL_ID ExtremeResult  = M_NULL;
	//int* MinAndMax = new int[256];
	int* MinAndMax = g_pMManager->pem_new<int>(true, 256, (PCHAR)__FUNCTION__, __LINE__);

	GetHistoValue(tempMilBuf_C,MinAndMax);

	int minIdx = 0;
	int maxIdx = 0;

	GetMinMaxIdx(MinAndMax,256,&minIdx,&maxIdx);

	thres = maxIdx;
	MbufClear(tempMilBuf2,thres);
	//delete MinAndMax;
	g_pMManager->pem_delete(MinAndMax, false);
	MinAndMax = NULL;

	MbufCopyClip(tempMilBuf_C,tempMilBuf2,(int)(blobResult->left[0]),temp2ChildTop/*(int)(blobResult->top[0])*/);

	//delete blobResult;
	g_pMManager->pem_delete(blobResult, false);
	blobResult = NULL;

	MbufGet(tempMilBuf2,dstImg);

	MbufFree(tempMilBuf_C);
	MbufFree(tempMilBuf2_C);
	MbufFree(tempMilBuf);
	MbufFree(tempMilBuf2);

}

void CProcMil_OCR::MakeBlob2(Im::PIL_ID src, Im::PIL_ID dst, int foreground, int fontCount,CMilBlobResult** blobResult)
{
	SaveWorkImg(src,_T("tempMilBuf2_before.bmp"));

	if(foreground == 1)
		Im::Blob::blobControl(m_milBlobResult, M_FOREGROUND_VALUE, M_NONZERO);
	else
		Im::Blob::blobControl(m_milBlobResult, M_FOREGROUND_VALUE, M_ZERO);

	Im::Blob::blobCalculate(src, M_NULL, m_milBlobFeature, m_milBlobResult);

	MIL_INT count = 0;
	Im::Blob::blobGetNumber(m_milBlobResult,  &count);

	int areaThres = 0;
	if(count > 0 && fontCount > 0)
	{
		//double* area = new double[(int)count];
		double* area = g_pMManager->pem_new<double>(true, (int)count, (PCHAR)__FUNCTION__, __LINE__);
		Im::Blob::blobGetResult(m_milBlobResult, M_AREA + M_TYPE_DOUBLE, area);
		areaThres = (int)area[fontCount - 1];
		//delete area;
		g_pMManager->pem_delete(area, false);
	}

	//blob select
	Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_AREA, M_LESS, areaThres, M_NULL);
	Im::Blob::blobGetNumber(m_milBlobResult,  &count);

	if(*blobResult == NULL && count > 0)
	{
		*blobResult = new CMilBlobResult(count);
		g_pMManager->pem_new_check(*blobResult, (PCHAR)__FUNCTION__, __LINE__);
	}

	if(count > (MIL_INT)0)
	{

		Im::Blob::blobGetResult(m_milBlobResult, M_AREA + M_TYPE_DOUBLE, (*blobResult)->area);
		Im::Blob::blobGet_BOX(m_milBlobResult, (*blobResult)->left,(*blobResult)->top,(*blobResult)->right,(*blobResult)->bottom);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_X_MIN + M_TYPE_DOUBLE, (*blobResult)->left);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_X_MAX + M_TYPE_DOUBLE, (*blobResult)->right);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_Y_MIN + M_TYPE_DOUBLE, (*blobResult)->top);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_Y_MAX + M_TYPE_DOUBLE, (*blobResult)->bottom);
		Im::Blob::blobGet_Center(m_milBlobResult,(*blobResult)->cx, (*blobResult)->cy);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_CENTER_OF_GRAVITY_X + M_TYPE_DOUBLE, (*blobResult)->cx);	
// 		Im::Blob::blobGetResult(m_milBlobResult, M_CENTER_OF_GRAVITY_Y + M_TYPE_DOUBLE, (*blobResult)->cy);
	}

	SaveWorkImg(dst,_T("tempMilBuf2_after.bmp"));
}


void CProcMil_OCR::StrAllocResult()
{
	if(m_milStrResult != M_NULL)
	{
		MstrFree(m_milStrResult);
		m_milStrResult = M_NULL;
	}

	MstrAllocResult(*m_milSys, M_DEFAULT, &m_milStrResult);
}

int CProcMil_OCR::GetCharPos(double* x, double* y,double* width, double* height,double* angle, int index,bool m_bReRotate)
{
	double* strPosX = NULL;
	double* strPosY = NULL;
	double* strPosX2 = NULL;
	double* strPosY2 = NULL;
	double* strangle = NULL;

	int charCount = 0;
	GetCharCount(&charCount);

	if(charCount <= 0)
	{
		return eSTR_FAIL;
	}

	/*strPosX = new double[MAX_STRLEN];
	strPosY = new double[MAX_STRLEN];
	strPosX2 = new double[MAX_STRLEN];
	strPosY2 = new double[MAX_STRLEN];
	strangle = new double[MAX_STRLEN];*/
	strPosX = g_pMManager->pem_new<double>(true, MAX_STRLEN, (PCHAR)__FUNCTION__, __LINE__);
	strPosY = g_pMManager->pem_new<double>(true, MAX_STRLEN, (PCHAR)__FUNCTION__, __LINE__);
	strPosX2 = g_pMManager->pem_new<double>(true, MAX_STRLEN, (PCHAR)__FUNCTION__, __LINE__);
	strPosY2 = g_pMManager->pem_new<double>(true, MAX_STRLEN, (PCHAR)__FUNCTION__, __LINE__);
	strangle = g_pMManager->pem_new<double>(true, MAX_STRLEN, (PCHAR)__FUNCTION__, __LINE__);

	memcpy(strPosX, doubeZeroBuf, sizeof(double)*MAX_STRLEN);
	memcpy(strPosX, doubeZeroBuf, sizeof(double)*MAX_STRLEN);
	memcpy(strPosX2, doubeZeroBuf, sizeof(double)*MAX_STRLEN);
	memcpy(strPosX2, doubeZeroBuf, sizeof(double)*MAX_STRLEN);
	memcpy(strangle, doubeZeroBuf, sizeof(double)*MAX_STRLEN);

	MstrGetResult(m_milStrResult, index,   M_CHAR_BOX_UL_X  + M_TYPE_DOUBLE , strPosX);
	MstrGetResult(m_milStrResult, index,   M_CHAR_BOX_UL_Y  + M_TYPE_DOUBLE , strPosY);
	MstrGetResult(m_milStrResult, index,   M_CHAR_BOX_BR_X  + M_TYPE_DOUBLE , strPosX2);
	MstrGetResult(m_milStrResult, index,   M_CHAR_BOX_BR_Y  + M_TYPE_DOUBLE , strPosY2);
	MstrGetResult(m_milStrResult, index,   M_CHAR_ANGLE  + M_TYPE_DOUBLE , strangle);
	for(int i = 0;i<charCount;i++)
	{
		strPosX2[i] = strPosX2[i]-strPosX[i];
		strPosY2[i] = strPosY2[i]-strPosY[i];

		if(m_bReRotate)
		{
			strPosX[i] = ImgWidth -strPosX[i]-strPosX2[i];
			strPosY[i] = ImgHeight -strPosY[i]-strPosY2[i];
		}
	}

	memcpy(x, strPosX, sizeof(double)*MAX_STRLEN);
	memcpy(y, strPosY, sizeof(double)*MAX_STRLEN);
	memcpy(width, strPosX2, sizeof(double)*MAX_STRLEN);
	memcpy(height, strPosY2, sizeof(double)*MAX_STRLEN);
	memcpy(angle, strangle, sizeof(double)*MAX_STRLEN);

	/*delete strPosX;
	delete strPosY;
	delete strPosX2;
	delete strPosY2;
	delete strangle;*/
	g_pMManager->pem_delete(strPosX, false);
	g_pMManager->pem_delete(strPosY, false);
	g_pMManager->pem_delete(strPosX2, false);
	g_pMManager->pem_delete(strPosY2, false);
	g_pMManager->pem_delete(strangle, false);

	return eSTR_SUCCESS;
}

int CProcMil_OCR::GetCharSIMILARITY(double* score, int index)
{
	double* charScore = NULL;

	int charCount = 0;
	GetCharCount(&charCount);

	if(charCount <= 0)
	{
		return eSTR_FAIL;
	}

	//charScore = new double[MAX_STRLEN];
	charScore = g_pMManager->pem_new<double>(true, MAX_STRLEN, (PCHAR)__FUNCTION__, __LINE__);

	for(int i = 0; i<MAX_STRLEN; i++)
	{
		charScore[i] = -1.;
	}

	MstrGetResult(m_milStrResult, index,  M_CHAR_SIMILARITY_SCORE  + M_TYPE_DOUBLE , charScore);
	memcpy(score, charScore, sizeof(double)*MAX_STRLEN);

	//delete charScore;
	g_pMManager->pem_delete(charScore, false);

	return eSTR_SUCCESS;
}
double CProcMil_OCR::GetFontWidth()
{

	MIL_DOUBLE fontWidth = 0.0;
	StrControl(M_FONT_INDEX(M_ALL),M_SPACE_WIDTH,M_MAX_CHAR_WIDTH);
	MstrInquire(m_milStrContext, M_FONT_INDEX(0), M_SPACE_WIDTH_VALUE  + M_TYPE_DOUBLE, &fontWidth);
	
	return (double)fontWidth;
}

void CProcMil_OCR::SetImgSize(int width, int height)
{
	ImgWidth=width;
	ImgHeight=height;
}