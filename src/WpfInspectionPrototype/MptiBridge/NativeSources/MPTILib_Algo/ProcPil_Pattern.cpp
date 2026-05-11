#include "StdAfx.h"
#include "ProcPil_Pattern.h"
//#include "GeoMatch.h"
#include "MPTI.h"

#define PAT_ACCURACY	M_HIGH//M_MEDIUM
#define PAT_SPEED		M_HIGH//M_MEDIUM 
#define  PAT_ATCSCORE		0.0

#ifdef _DEBUG
	#define    _PROC_IMG_SAVE
#endif


void Log_PILPatModel(CString msg)
{
	//g_pMPTI->AddLog_OCR(msg);
}

CProcPil_Pattern::CProcPil_Pattern(void)
{
	m_className = _T("CProcPil_Pattern");

	m_ModelMng = std::shared_ptr<CMModelManager<ModelFile_Pat>>(new CMModelManager<ModelFile_Pat>(eAlgoPattern, _T(".ppd"), Log_PILPatModel));
	ImgProc = std::make_shared<ImgProcessing>();
	ImgProc_E1 = std::make_shared<ImgProcessing>();
	ImgProc_E2 = std::make_shared<ImgProcessing>();
	ImgProc_E3 = std::make_shared<ImgProcessing>();
	ImgProc_D1 = std::make_shared<ImgProcessing>();
	ImgProc_D2 = std::make_shared<ImgProcessing>();
	ImgProc_D3 = std::make_shared<ImgProcessing>();
}

CProcPil_Pattern::~CProcPil_Pattern(void)
{
}

int CProcPil_Pattern::InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib)
{
	if(milApp == M_NULL || milSys == M_NULL)
	{
		return eMPAT_FAIL;	
	}

	CProcMil::InitMil(milApp, milSys, bUseImagePilLib);

	m_milPatResult = std::make_shared<GeoResult_Pat>();
	m_milPatResult->Alloc(1);
}

int CProcPil_Pattern::FreeMil()
{
#ifndef _DISABLE_MIL	
	CProcMil :: FreeMil();
#endif
	if (m_milPatResult != nullptr)
		m_milPatResult.reset();

	return eMPAT_SUCCESS;
}

void CProcPil_Pattern::DeleteModelBuf()
{
	m_milPatModel.reset();
	m_milPatModel_Rotate.reset();
}

int CProcPil_Pattern::AllocPatModel(void* milSrc, int cx, int cy, int width, int height)
{
	int ret  = eMPAT_SUCCESS;

	if(milSrc == NULL)
		return eMPAT_FAIL;

	DeleteModelBuf();


	m_milPatModel_Temp = std::shared_ptr<GeoModel_Pattern>(new GeoModel_Pattern());

	int Wid = Im::Buf::Inquire((Im::PIL_ID)milSrc, M_SIZE_X, M_NULL);
	int Len = Im::Buf::Inquire((Im::PIL_ID)milSrc, M_SIZE_Y, M_NULL);
	int Pitch = Im::Buf::Inquire((Im::PIL_ID)milSrc, M_PITCH_BYTE, M_NULL);
	uchar * pData = (uchar *)Im::Buf::Inquire((Im::PIL_ID)milSrc, M_HOST_ADDRESS, M_NULL);
	cv::Mat srcImg(Len, Wid, CV_8UC1);
	for (int y=0; y<Len; y++)
		memcpy(srcImg.ptr(y), &pData[y*Pitch], Wid);

	m_milPatModel_Temp->Alloc(srcImg);
	m_milPatModel = m_milPatModel_Temp;

	return ret;
}
void CProcPil_Pattern::SetnChannel(int nChannel)
{
	m_nChannel = nChannel;
}
int CProcPil_Pattern::GetnChannel()
{
	return m_nChannel;
}
void CProcPil_Pattern::GetnStep(int step)
{
	m_nStep = step;
}
void CProcPil_Pattern::GetClipModelImg(cv::Mat ClipModelImg)
{
	m_ClipModelImg = ClipModelImg;
}
int CProcPil_Pattern::GetModelAlgo()
{
	return m_milPatModel->getMatchAlgo();
}

int CProcPil_Pattern::AllocPatModel(UCHAR* userSrc, int width, int height, double rotateAngle, int rotateW, int rotateH)
{
	int ret  = eMPAT_SUCCESS;

	if(userSrc == nullptr)
		return eMPAT_FAIL;

	Im::PIL_ID milSrc = NULL;

	if(m_nChannel != 3) 
		m_nChannel=1;

	if(m_nChannel == 3)
	{
		milSrc = AllocBuffColor(width, height);
		MIL_INT M_width = Im::Buf::Inquire(milSrc, M_SIZE_X, M_NULL);
		MIL_INT M_height = Im::Buf::Inquire(milSrc, M_SIZE_Y, M_NULL);
		Im::Buf::PutColor2d(milSrc, M_PACKED + M_BGR24, M_ALL_BANDS,0,0, M_width, M_height, userSrc);
		//Im::Buf::PutColor(milSrc, M_PACKED + M_BGR24, M_ALL_BANDS, userSrc);
	}
	else
	{
		milSrc = AllocBuff(width, height);
		Im::Buf::Put(milSrc, userSrc);
	}
	DeleteModelBuf();

	m_milPatModel_Temp = std::shared_ptr<GeoModel_Pattern>(new GeoModel_Pattern());

	cv::Mat srcImg(height, width, CV_MAKETYPE(CV_8U,m_nChannel));

	if(m_nChannel == 3)
	{
		for (int y=0; y<height; y++)
			memcpy(srcImg.ptr(y), &userSrc[y*(width*3)], width*3);
	}
	else
	{
		for (int y=0; y<height; y++)
			memcpy(srcImg.ptr(y), &userSrc[y*width], width);
	}
	m_milPatModel_Temp->Alloc(srcImg);

	if(rotateAngle > 0)
	{
		Im::PIL_ID rotateImg = NULL;

		if(m_nChannel == 3)
		{
			Im::PIL_ID rotateImg;
			if(rotateAngle == 90 || rotateAngle == 270)
				Im::Buf::AllocColor(*m_milSys, 3, (MIL_INT)height, (MIL_INT)width, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &rotateImg);	
			else
				Im::Buf::AllocColor(*m_milSys, 3, (MIL_INT)width, (MIL_INT)height, 8 + M_UNSIGNED, M_IMAGE + M_PROC, &rotateImg);

			Im::im::Rotate(milSrc, rotateImg, (MIL_DOUBLE)rotateAngle, M_DEFAULT , M_DEFAULT , M_DEFAULT, M_DEFAULT, M_BILINEAR);

			int Wid = Im::Buf::Inquire((Im::PIL_ID)rotateImg, M_SIZE_X, M_NULL);
			int Len = Im::Buf::Inquire((Im::PIL_ID)rotateImg, M_SIZE_Y, M_NULL);
			
			cv::Mat rotImg = cv::Mat::zeros(Len, Wid, CV_8UC3);
			Im::Buf::GetColor2d(rotateImg, M_PACKED + M_BGR24, M_ALL_BANDS,0,0, Wid, Len, rotImg.ptr());

			m_milPatModel_Rotate = std::shared_ptr<GeoModel_Pattern>(new GeoModel_Pattern());
			m_milPatModel_Rotate->Alloc(rotImg);
			FreeMilImageBuff(&rotateImg);
		}
		else 
		{
			Im::PIL_ID rotateImg = AllocRotateBuff(milSrc, rotateAngle);
			int Wid = Im::Buf::Inquire((Im::PIL_ID)rotateImg, M_SIZE_X, M_NULL);
			int Len = Im::Buf::Inquire((Im::PIL_ID)rotateImg, M_SIZE_Y, M_NULL);
			int Pitch = Im::Buf::Inquire((Im::PIL_ID)rotateImg, M_PITCH_BYTE, M_NULL);
			uchar * pData = (uchar *)Im::Buf::Inquire((Im::PIL_ID)rotateImg, M_HOST_ADDRESS, M_NULL);

			cv::Mat rotImg(Len, Wid, CV_8UC1);
			for (int y=0; y<Len; y++)
				memcpy(rotImg.ptr(y), &pData[y*Pitch], Wid);

			m_milPatModel_Rotate = std::shared_ptr<GeoModel_Pattern>(new GeoModel_Pattern());
			m_milPatModel_Rotate->Alloc(rotImg);
			FreeMilImageBuff(&rotateImg);
		}
	}

	m_milPatModel = m_milPatModel_Temp;

	FreeMilImageBuff(&milSrc);
	srcImg.release();
	return ret;
}

int CProcPil_Pattern::SearchPattern(UCHAR* userSrc, int width, int height)
{
	Im::PIL_ID milTemp = M_NULL;

	if(m_milPatModel == nullptr)
		return 0;

#ifdef _PROC_IMG_SAVE
 	CString sPatName;
 	sPatName.Format(_T("SearchPatternpat_%.0f.bmp"), m_milPatModel->getImageAngle());
	SaveWorkImg(m_nChannel==1 ? m_milPatModel->Image() : m_milPatModel->color_Image(), sPatName);
#endif
	

	cv::Mat srcImg;
	srcImg = cv::Mat::zeros(height, width, CV_MAKETYPE(CV_8U,m_nChannel));

	if(m_nChannel == 3)
	{
		for (int y=0; y<height; y++)
			memcpy(srcImg.ptr(y), &userSrc[y*(width*3)], width*3);
	}
	else
	{
		for (int y=0; y<height; y++)
			memcpy(srcImg.ptr(y), &userSrc[y*width], width);
	}

#ifdef _PROC_IMG_SAVE
	CString sSrcName;
	sSrcName.Format(_T("SearchPatternsrc_%.0f.bmp"), m_milPatModel->getImageAngle());
	SaveWorkImg(srcImg, sSrcName);
#endif

	m_milPatResult->Free();
	m_milPatResult->Alloc(1);

	gm.SetImgProcess(ImgProc,ImgProc_E1,ImgProc_E2,ImgProc_E3,ImgProc_D1,ImgProc_D2,ImgProc_D3);
	if(m_SearchParam==nullptr)
		gm.FindModel(*m_milPatModel, *m_milPatResult,false);
	else
		gm.FindModel_Div(*m_milPatModel, *m_SearchParam, *m_milPatResult);

//	EdgeAlgoCheckback();
#ifdef _PROC_IMG_SAVE
	CString sName, sName1;
	sName.Format(_T("SearchPatterndraw_%.0f.bmp"), m_milPatModel->getImageAngle());
	sName1.Format(_T("SearchPatterndraw1_%.0f.bmp"), m_milPatModel->getImageAngle());
	cv::Mat draw, draw1;
	srcImg.copyTo(draw);
	draw1 = cv::Mat::zeros(srcImg.rows, srcImg.cols, srcImg.type());
	gm.DrawContours(draw, *m_milPatModel, *m_milPatResult, cv::Scalar(255), 1);
	gm.DrawContours(draw1, *m_milPatModel, *m_milPatResult, cv::Scalar(255), 1);
	SaveWorkImg(draw, sName);
	SaveWorkImg(draw1, sName1);
#endif
	srcImg.release();
	return m_milPatResult->getResCnt();
}

int CProcPil_Pattern::SearchPattern_Draw(UCHAR* userSrc, int width, int height, UCHAR* userDst,int Channel)
{
	Im::PIL_ID milTemp = M_NULL;
	//cv::Mat srcImg(height, width, CV_8UC1);
	cv::Mat srcImg(height, width, CV_MAKETYPE(CV_8U,Channel));
	if (m_milPatModel == nullptr)
		return 0;
	if(m_milPatModel->getMatchAlgo() == GeoModel::agImage)
		return 0;

	if(Channel==3)
	{
		for(int y=0; y< height ; y++)
			memcpy(&srcImg.data[y*srcImg.step], &userSrc[y*(width * 3)], width * 3);
	}
	else
	{
		for (int y=0; y<height; y++)
			memcpy(srcImg.ptr(y), &userSrc[y*width], width);
	}
	double dAngleRotate = 180.0;
	//double angleTemp = dAngleRotate + m_milPatModel->getImageAngle();
// 	if(angleTemp >= 360)
// 		angleTemp = angleTemp - 360;

	std::shared_ptr<GeoModel_Pattern> rotModel = std::make_shared<GeoModel_Pattern>();
	rotModel->CopyOf(*m_milPatModel, dAngleRotate);

	std::shared_ptr<GeoResult_Pat> res = std::make_shared<GeoResult_Pat>();
	std::shared_ptr<GeoResult_Pat> rotRes = std::make_shared<GeoResult_Pat>();
	res->Alloc(1);
	rotRes->Alloc(1);

	ImgProc = std::make_shared<ImgProcessing>();
	ImgProc->_ImgProcess(srcImg,*m_milPatModel);

	gm.SetImgProcess(ImgProc,ImgProc_E1,ImgProc_E2,ImgProc_E3,ImgProc_D1,ImgProc_D2,ImgProc_D3);

	double stdScore(HUGE_VAL),rotScore(HUGE_VAL);
	if(m_SearchParam==nullptr)
	{
		gm.FindModel( *m_milPatModel, *res,false);
		gm.FindModel( *rotModel, *rotRes,false);
		stdScore = res->_Equality[0];
		rotScore = rotRes->_Equality[0];

	}
	else
	{
		gm.FindModel_Div(*m_milPatModel, *m_SearchParam, *res);
		gm.FindModel_Div(*rotModel, *m_SearchParam, *rotRes);
		stdScore = res->_Equality[0];
		rotScore = rotRes->_Equality[0];
		//if(m_SearchParam->_nAreaCnt_C>1)
		{
			for (int x=0; x<m_SearchParam->_nAreaCnt_C; x++)
			{
				int nEdgeCnt(0);
				for (int c=0; c<m_SearchParam->_nCharAreaCnt; c++)
				{
					nEdgeCnt = res->_Score[0].Count[x][c];
					if(res->_Score[0].Count[x][c]>5 && stdScore > res->_Score[0].Score[x][c])
						stdScore = res->_Score[0].Score[x][c];
					if(rotRes->_Score[0].Count[x][c]>5 && rotScore > rotRes->_Score[0].Score[x][c])
						rotScore = rotRes->_Score[0].Score[x][c];
				}
			}
		}

		if(m_SearchParam->_nAreaCnt_R>1)
		{
			for (int x=0; x<m_SearchParam->_nAreaCnt_R; x++)
			{
				int nEdgeCnt(0);
				for (int c=0; c<m_SearchParam->_nCharAreaCnt; c++)
				{
					nEdgeCnt = res->_Score[0].Count[x][c];
					if(res->_Score[0].Count[x][c]>5 && stdScore > res->_Score[0].Score[x][c])
						stdScore = res->_Score[0].Score[x][c];
					nEdgeCnt = res->_Score[0].Count[x][c];
					if(rotRes->_Score[0].Count[x][c]>5 && rotScore > rotRes->_Score[0].Score[x][c])
						rotScore = rotRes->_Score[0].Score[x][c];
				}
			}
		}

			if(stdScore == HUGE_VAL)
				stdScore = 0;

			if(stdScore<0.0)
				stdScore = 0;

			if(rotScore == HUGE_VAL)
				rotScore = 0;

			if(rotScore<0.0)
				rotScore = 0;
	}
// 	gm.FindModel(*m_milPatModel, res);
// 	gm.FindModel(*rotModel, rotRes);
	m_milPatResult = res;
//	EdgeAlgoCheckback();

	cv::Mat draw(height, width, CV_MAKETYPE(CV_8U,Channel), userDst, /*width*/srcImg.step);

	double dScore = stdScore >= rotScore ? stdScore : rotScore;
	if(dScore > 50.0)
	{
		if(stdScore >= rotScore)
			gm.DrawContours(draw, *m_milPatModel, *res, cv::Scalar(255), 1);
		else
			gm.DrawContours(draw, *rotModel, *rotRes, cv::Scalar(255), 1);
	}
	else
		draw.setTo(0);

	ImgProc->Free();
	rotRes->Free();
	res->Free();
	rotModel.reset();

#ifdef _PROC_IMG_SAVE
	CString sName;
	sName.Format(_T("SearchPatterndraw_%.0f.bmp"), m_milPatModel->getImageAngle());
	SaveWorkImg(draw, sName);
#endif

	return stdScore >= rotScore ? res->getResCnt() : rotRes->getResCnt();
}



int CProcPil_Pattern::SearchPattern(void* milSrc, void* milSrc_color, int cx, int cy, int width, int height)
{	
	if(m_milPatModel == nullptr)
		return 0;

	gm.SetImgProcess(ImgProc,ImgProc_E1,ImgProc_E2,ImgProc_E3,ImgProc_D1,ImgProc_D2,ImgProc_D3);
	if(m_SearchParam==nullptr)
		gm.FindModel( *m_milPatModel, *m_milPatResult,m_SearchParam->_nAreaCnt_C<2);
	else
		gm.FindModel_Div(*m_milPatModel, *m_SearchParam, *m_milPatResult);

//	EdgeAlgoCheckback();
#ifdef _PROC_IMG_SAVE
#endif
	//FreeMilImageBuff(&milTemp);
	ImgProc->Free();
	if (m_milPatResult == nullptr)
		return 0;
	else
		return m_milPatResult->getResCnt();
}

void CProcPil_Pattern::PreprocModel(UCHAR* userSrc, int sizeX, int sizeY)
{
	Im::PIL_ID milTemp = AllocBuff(sizeX, sizeY); 
	Im::Buf::Put(milTemp, userSrc);

	int Wid = Im::Buf::Inquire((Im::PIL_ID)userSrc, M_SIZE_X, M_NULL);
	int Len = Im::Buf::Inquire((Im::PIL_ID)userSrc, M_SIZE_Y, M_NULL);
	int Pitch = Im::Buf::Inquire((Im::PIL_ID)userSrc, M_PITCH_BYTE, M_NULL);
	uchar * pData = (uchar *)Im::Buf::Inquire((Im::PIL_ID)userSrc, M_HOST_ADDRESS, M_NULL);

	cv::Mat srcImg(Len, Wid, CV_MAKETYPE(CV_8U,m_nChannel), pData, Pitch);
	
	m_milPatModel->Alloc(srcImg);
	m_milPatModel->Preprocess();

	FreeMilImageBuff(&milTemp);
}

void CProcPil_Pattern::PreprocModel(void* milSrc, BOOL rotMode, int use_algo, int nLight_Number)
{
	m_milPatModel->setMatchAlgo(use_algo);
	m_milPatModel->setLightbt(nLight_Number);
	m_milPatModel->setModelFilterType(getModelFilterType());
	m_milPatModel->Preprocess();
	
	if(rotMode == TRUE)
	{
		m_milPatModel_Rotate->setLightbt(nLight_Number);
		m_milPatModel_Rotate->setMatchAlgo(use_algo);
		m_milPatModel_Rotate->Preprocess();
	}
}

int CProcPil_Pattern::GetPatResult(double* retScore, double* retAngle, double* retPosX, double* retPosY)
{
	int ret = eMPAT_SUCCESS;
	if (m_milPatResult == nullptr)
	{
		*retScore = 0;
		*retAngle = 0;
		*retPosX = 0;
		*retPosY = 0;

		return eMPAT_FAIL;
	}
	int count = m_milPatResult->getResCnt();

	if(count == 1)
	{
		float posX = m_milPatResult->_Center_y[0];
		float posY = m_milPatResult->_Center_x[0];
		float angle = m_milPatResult->_Angle[0];
		float score = m_milPatResult->_Equality[0];

		*retScore = score;
		*retAngle = angle;
		*retPosX = posX;
		*retPosY = posY;
	}
	else if((int)count > 1)
	{
		/*float*  posX = new float[count];
		float*  posY = new float[count];
		float*  angle = new float[count];
		float*  score = new float[count];*/
		float*  posX = g_pMManager->pem_new<float>(true, count, (PCHAR)__FUNCTION__, __LINE__);
		float*  posY = g_pMManager->pem_new<float>(true, count, (PCHAR)__FUNCTION__, __LINE__);
		float*  angle = g_pMManager->pem_new<float>(true, count, (PCHAR)__FUNCTION__, __LINE__);
		float*  score = g_pMManager->pem_new<float>(true, count, (PCHAR)__FUNCTION__, __LINE__);

		memcpy(posX, &m_milPatResult->_Center_y[0], sizeof(float)*count);
		memcpy(posY, &m_milPatResult->_Center_x[0], sizeof(float)*count);
		memcpy(angle, &m_milPatResult->_Angle[0], sizeof(float)*count);
		memcpy(score, &m_milPatResult->_Equality[0], sizeof(float)*count);

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

void CProcPil_Pattern::GetPatSubScore(double* retScore, int Pitch)
{
	if(m_SearchParam==nullptr || m_milPatResult == nullptr)
		return;

	int count = m_milPatResult->getResCnt();

	bool bCheckChDiv = m_SearchParam->_bUseCharDiv;

	if(count == 1)
	{
		float posX = m_milPatResult->_Center_y[0];
		float posY = m_milPatResult->_Center_x[0];
		float angle = m_milPatResult->_Angle[0];
		float score = m_milPatResult->_Equality[0];

		
		//if(m_SearchParam->_nAreaCnt_C>1)
		for (int x=0; x<m_SearchParam->_nAreaCnt_C; x++)
		{
			int nEdgeCnt(0);
			double dMinScore(HUGE_VAL);
			for (int c=0; c<m_SearchParam->_nCharAreaCnt; c++)
			{
				nEdgeCnt = m_milPatResult->_Score[0].Count[x][c];
				
				if(bCheckChDiv == true)
				{
					if(m_milPatResult->_Score[0].Count[x][c]>5 && dMinScore > m_milPatResult->_Score[0].Score[x][c])
						dMinScore = m_milPatResult->_Score[0].Score[x][c];
				}
				else
				{
					if(dMinScore > m_milPatResult->_Score[0].Score[x][c])
						dMinScore = m_milPatResult->_Score[0].Score[x][c];
				}
				
			}

			if(dMinScore == HUGE_VAL)
				dMinScore = 0;

			if(dMinScore<0.0)
				dMinScore = 0;

			retScore[x] = dMinScore;
		}
		
		//row
		if(m_SearchParam->_nAreaCnt_R>1)
		for (int x=0; x<m_SearchParam->_nAreaCnt_R; x++)
		{
			int nEdgeCnt(0);
			double dMinScore(HUGE_VAL);
			for (int c=0; c<m_SearchParam->_nCharAreaCnt; c++)
			{
				nEdgeCnt = m_milPatResult->_Score[0].Count[x][c];
				if(bCheckChDiv == true)
				{
					if(m_milPatResult->_Score[0].Count[x][c]>5 && dMinScore > m_milPatResult->_Score[0].Score[x][c])
						dMinScore = m_milPatResult->_Score[0].Score[x][c];
				}
				else
				{
					if(dMinScore > m_milPatResult->_Score[0].Score[x][c])
						dMinScore = m_milPatResult->_Score[0].Score[x][c];
				}
				
			}

			if(dMinScore == HUGE_VAL)
				dMinScore = 0;

			if(dMinScore<0.0)
				dMinScore = 0;

			retScore[x] = dMinScore;
		}
		else
		{
			int nEdgeCnt(0);
			double dMinScore(HUGE_VAL);
			for (int c = 0; c < m_SearchParam->_nCharAreaCnt; c++)
			{
				nEdgeCnt = m_milPatResult->_Score[0].Count[0][c];

				if (bCheckChDiv == true)
				{
					if (m_milPatResult->_Score[0].Count[0][c] > 5 && dMinScore > m_milPatResult->_Score[0].Score[0][c])
						dMinScore = m_milPatResult->_Score[0].Score[0][c];
				}
				else
				{
					if (dMinScore > m_milPatResult->_Score[0].Score[0][c])
						dMinScore = m_milPatResult->_Score[0].Score[0][c];
				}

			}

			if (dMinScore == HUGE_VAL)
				dMinScore = 0;

			if (dMinScore < 0.0)
				dMinScore = 0;

			retScore[0] = dMinScore;

		}
	}
	else if((int)count > 1)
	{
		/*float*  posX = new float[count];
		float*  posY = new float[count];
		float*  angle = new float[count];
		float*  score = new float[count];*/
		float*  posX = g_pMManager->pem_new<float>(true, count, (PCHAR)__FUNCTION__, __LINE__);
		float*  posY = g_pMManager->pem_new<float>(true, count, (PCHAR)__FUNCTION__, __LINE__);
		float*  angle = g_pMManager->pem_new<float>(true, count, (PCHAR)__FUNCTION__, __LINE__);
		float*  score = g_pMManager->pem_new<float>(true, count, (PCHAR)__FUNCTION__, __LINE__);

		memcpy(posX, &m_milPatResult->_Center_y[0], sizeof(float)*count);
		memcpy(posY, &m_milPatResult->_Center_x[0], sizeof(float)*count);
		memcpy(angle, &m_milPatResult->_Angle[0], sizeof(float)*count);
		memcpy(score, &m_milPatResult->_Equality[0], sizeof(float)*count);

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


		if(m_SearchParam->_nAreaCnt_C>1)
		for (int x=0; x<m_SearchParam->_nAreaCnt_C; x++)
		{
			double dMinScore(HUGE_VAL);
			for (int c=0; c<m_SearchParam->_nCharAreaCnt; c++)
			{
				if(bCheckChDiv == true)
				{
					if(m_milPatResult->_Score[0].Count[x][c]>5 && dMinScore > m_milPatResult->_Score[maxIndex].Score[x][c])
						dMinScore = m_milPatResult->_Score[maxIndex].Score[x][c];
				}
				else
				{
					if(dMinScore > m_milPatResult->_Score[maxIndex].Score[x][c])
						dMinScore = m_milPatResult->_Score[maxIndex].Score[x][c];
				}
			}

			if(dMinScore == HUGE_VAL)
				dMinScore = 0.0;

			if(dMinScore<0.0)
				dMinScore = 0;

			retScore[x] = dMinScore;
		}

		//row
		if(m_SearchParam->_nAreaCnt_R>1)
		for (int x=0; x<m_SearchParam->_nAreaCnt_R; x++)
		{
			double dMinScore(HUGE_VAL);
			for (int c=0; c<m_SearchParam->_nCharAreaCnt; c++)
			{
				if(bCheckChDiv == true)
				{
					if(m_milPatResult->_Score[0].Count[x][c]>5 && dMinScore > m_milPatResult->_Score[maxIndex].Score[x][c])
						dMinScore = m_milPatResult->_Score[maxIndex].Score[x][c];
				}
				else
				{
					if(dMinScore > m_milPatResult->_Score[maxIndex].Score[x][c])
						dMinScore = m_milPatResult->_Score[maxIndex].Score[x][c];
				}
			}

			if(dMinScore == HUGE_VAL)
				dMinScore = 0.0;

			if(dMinScore<0.0)
				dMinScore = 0;

			retScore[x] = dMinScore;
		}

		else
		{
			int nEdgeCnt(0);
			double dMinScore(HUGE_VAL);
			for (int c = 0; c < m_SearchParam->_nCharAreaCnt; c++)
			{
				nEdgeCnt = m_milPatResult->_Score[0].Count[0][c];
				if (bCheckChDiv == true)
				{
					if (m_milPatResult->_Score[0].Count[0][c] > 5 && dMinScore > m_milPatResult->_Score[0].Score[0][c])
						dMinScore = m_milPatResult->_Score[0].Score[0][c];
				}
				else
				{
					if (dMinScore > m_milPatResult->_Score[0].Score[0][c])
						dMinScore = m_milPatResult->_Score[0].Score[0][c];
				}

			}

			if (dMinScore == HUGE_VAL)
				dMinScore = 0;

			if (dMinScore < 0.0)
				dMinScore = 0;

			retScore[0] = dMinScore;

		}
		/*delete [] posX;
		delete [] posY;
		delete [] angle;
		delete [] score;*/
		g_pMManager->pem_delete(posX, true);
		g_pMManager->pem_delete(posY, true);
		g_pMManager->pem_delete(angle, true);
		g_pMManager->pem_delete(score, true);
	}
}

int CProcPil_Pattern::WritePatternModel(void* milSrc, int cx, int cy, int width, int height, CString strPath)
{
	int ret = eMPAT_SUCCESS;
	int stX = 0;
	int stY = 0;

	if(m_milPatModel == nullptr)
		return eMPAT_FAIL;

	stX = (int)(cx - (width / 2.0));
	stY = (int)(cy - (height / 2.0));

	CString strImagePath = _T("");
	strImagePath = strPath + _T(".tif");

	ModelSave(strPath);
	SaveClipImg((Im::PIL_ID)milSrc, 1, cx, cy, width, height, strImagePath);

	return ret;
}

int CProcPil_Pattern::WritePatternModel(UCHAR* userSrc, int width, int height, double rotateAngle, CString strPath, CString strExt)
{
	int ret = eMPAT_SUCCESS;

	if(userSrc == NULL || width < 0 || height < 0)
		return eMPAT_FAIL;
	Im::PIL_ID milSrc = NULL;

	/*if(g_pMPTI->m_LogLevel == 8)
		g_pMPTI->AddLog_OCR(_T("ProcPill_Pattern::WritePatternModel Start"));*/

	if(m_nChannel ==3 )
	{
		milSrc = AllocBuffColor(width, height);
		MIL_INT M_width = Im::Buf::Inquire(milSrc, M_SIZE_X, M_NULL);
		MIL_INT M_height = Im::Buf::Inquire(milSrc, M_SIZE_Y, M_NULL);
		Im::Buf::PutColor2d(milSrc, M_PACKED + M_BGR24, M_ALL_BANDS,0,0, M_width, M_height, userSrc);
	}
	else
	{
		milSrc = AllocBuff(width, height);
		Im::Buf::Put(milSrc, userSrc);
	}

	/*if(g_pMPTI->m_LogLevel == 8)
		g_pMPTI->AddLog_OCR(_T("ProcPill_Pattern::WritePatternModel AllocEnd"));*/

	if(m_milPatModel == nullptr)
		return eMPAT_FAIL;

	CString strImagePath = _T("");
	//strImagePath = strPath;// +_T(".bmp");
	strImagePath.Format(_T("%s%s"), strPath,strExt);
	
	if(rotateAngle == 0)
	{ 
		if(m_milPatModel != nullptr)
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
		if(m_milPatModel_Rotate != nullptr)
		{


			Im::PIL_ID rotateImg = M_NULL;

			if(m_nChannel ==3 )
				rotateImg = AllocRotateBuff_Color(milSrc, rotateAngle);
			else
				rotateImg = AllocRotateBuff(milSrc, rotateAngle);

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

	/*if(g_pMPTI->m_LogLevel == 8)
		g_pMPTI->AddLog_OCR(_T("ProcPill_Pattern::WritePatternModel End"));*/
	
	return ret;
}

int CProcPil_Pattern::WritePatternModel(CString strPath)
{
	int ret = eMPAT_SUCCESS;

	if(m_milPatModel == nullptr)
		return eMPAT_FAIL;

	ModelSave(strPath);

	return ret;
}

void CProcPil_Pattern::ModelSave(CString strPath)
{
	if(m_milPatModel == nullptr)
		return;

	CString strModelPath_mod = strPath + _T(".mod");
	DeleteFile(strModelPath_mod);

	m_milPatModel->setRes(m_resolX,m_resolY);
	m_milPatModel->SaveFile(strPath + _T(".ppd"), false);
}


void CProcPil_Pattern::ModelSave(std::shared_ptr<GeoModel_Pattern> milModel, CString strPath)
{
	if(milModel == nullptr)
		return;

	CString strModelPath_mod = strPath + _T(".mod");

	CFileFind fn;
	int ret  = fn.FindFile(strModelPath_mod);
	
	if(ret == 1)
		DeleteFile(strModelPath_mod);

	milModel->setRes(m_resolX,m_resolY);
	milModel->setModelFilterType(getModelFilterType());
	milModel->SaveFile(strPath + _T(".ppd"), false);
}

// int CProcPil_Pattern::ModelList_Load(CString strPath)
// {
// 	int ret = eMPAT_SUCCESS;
// 
// 	if (!m_ModelMng->LoadFileList(strPath))
// 		ret = eMPAT_FAIL;
// 
// 	return ret;
// }
int CProcPil_Pattern::ExtModelList_Load()
{
	int ret = eMPAT_SUCCESS;

	if (!m_ModelMng->ExtLoadFileList_P(m_resolX, m_resolY))
		ret = eMPAT_FAIL;

	return ret;
}

int CProcPil_Pattern::ModelList_Load(fileAlgoPath* strPath, int cnt)
{
	int ret = eMPAT_SUCCESS;

// 	if (!m_ModelMng->LoadFileList(strPath,cnt,m_resolX,m_resolY))
// 		ret = eMPAT_FAIL;
	ret = m_ModelMng->LoadFileList(strPath,cnt,m_resolX,m_resolY);

	return ret;
}

int CProcPil_Pattern::ExtModelList_Check()
{
	int ret = eMPAT_SUCCESS;

	if (!m_ModelMng->ExtCheckFileList(m_resolX, m_resolY , 1))
		ret = eMPAT_FAIL;

	return ret;
}

int CProcPil_Pattern::ModelList_Check(fileAlgoPath* strPath, int cnt)
{
	int ret = eMPAT_SUCCESS;

	// 	if (!m_ModelMng->LoadFileList(strPath,cnt,m_resolX,m_resolY))
	// 		ret = eMPAT_FAIL;
	ret = m_ModelMng->CheckFileList(strPath, cnt, m_resolX, m_resolY);
	
	int nF = m_ModelMng->GetModelCount(false);
	int nR = m_ModelMng->GetModelCount(true);

		CString str;
		str.Format(_T("ForntListCnt: %d, RearListCnt: %d"),nF,nR);
		ext::Log::add(str);



	return ret;
}

int CProcPil_Pattern::ModelList_Clear(int nLane)
{
	int ret = eMPAT_SUCCESS;

	ret = m_ModelMng->Clear_File(nLane);

	return ret;
}

int CProcPil_Pattern::ModelLoad(CString strPath)
{
	int ret = eMPAT_SUCCESS;

	DeleteModelBuf();

	m_ModelFile = m_ModelMng->GetModel(strPath);

	

	if (m_ModelFile == NULL)
	{
		CString strLog;
		strLog.Format(_T("Add Pattern Load Model  = %s "), strPath);
		ext::Log::add(strLog);
		ext::Log::add(_T("ModelLoad Faile "));
		ret = eMPAT_FAIL;
	}
	else
		SetnChannel(m_ModelFile->milModel->getModelnChannel());
// 
// 	m_milPatModel = std::make_shared<GeoModel_Pattern>();
// 	m_milPatModel->LoadFile(strPath);
// 
// 	if(m_milPatModel == NULL)
// 	{
// 		ret = eMPAT_FAIL;
// 	}
// 	else
// 		SetnChannel(m_milPatModel->getModelnChannel());
	return ret;
}
int CProcPil_Pattern::CodeSearchModelLoad(CString strPath)
{
	int ret = eMPAT_SUCCESS;

	DeleteModelBuf();

	m_ModelFile = m_ModelMng->GetPartSearchModel(strPath);

	if(m_ModelFile == NULL)
	{
		ret = eMPAT_FAIL;
	}

	return ret;
}

int CProcPil_Pattern::SimilarModelListLoad(CString Parcode, bool rear)
{
	DeleteModelBuf();


	m_SimilarModelFile.reserve( m_ModelMng->GetModelCount(rear) );

	int ret = m_ModelMng->GetSimilarModel(Parcode,rear, m_SimilarModelFile);

	return m_SimilarModelFile.size();
}

void CProcPil_Pattern::SimilarCntReset()
{
	m_SimilarModelFile.clear();
}

int CProcPil_Pattern::SimilarModelLoad(int index)
{
	int ret = eMPAT_SUCCESS;

	m_ModelFile = m_SimilarModelFile[index];

	if(m_ModelFile == NULL)
	{
		ret = eMPAT_FAIL;
	}
	else
		SetnChannel(m_ModelFile->milModel->getModelnChannel());

	return ret;
}

int CProcPil_Pattern::ModelFileLoad(CString strPath)
{
	int ret = eMPAT_SUCCESS;

	DeleteModelBuf();

	CString strModelPath = _T("");
	strModelPath = strPath + _T(".mod");
	CString strModelPath_ppd = strPath + _T(".ppd");
	
	if(FileExists(strModelPath))
	{
		m_ModelFile = std::shared_ptr<ModelFile_Pat>(new ModelFile_Pat(eAlgoPattern));
		m_ModelFile->LoadFile(strModelPath,m_resolX,m_resolY);
		if(m_ModelFile->milModel == nullptr)
			ret = eMPAT_FAIL;
		
	}
	else if(FileExists(strModelPath_ppd))
	{
		m_ModelFile = std::shared_ptr<ModelFile_Pat>(new ModelFile_Pat(eAlgoPattern));
		m_ModelFile->LoadFile(strModelPath_ppd,m_resolX,m_resolY);
		if (m_ModelFile->milModel == nullptr)
			return eMPAT_FAIL;
		setPilLightbt(m_ModelFile->milModel->getLightbt());
		setPilMatchAlgo(m_ModelFile->milModel->getMatchAlgo());
		setPilModelnChannel(m_ModelFile->milModel->getModelnChannel());
		setModelFilterType(m_ModelFile->milModel->getModelFilterType());
		if((m_ModelFile->milModel->getMatchAlgo() == GeoModel::agImage) && m_ModelFile->milModel->color_Image().empty()==false) SetnChannel(3);
		else SetnChannel(1);
	}
	else
	{
		ret = eMPAT_FAIL;
		CString sLog;
		sLog.Format(_T("File Not Excist!! Path: %s"), strModelPath);
		ext::Log::add(sLog);

	
	}

	return ret;
}

void CProcPil_Pattern::ModelPreproc(double dAngle,int ModelPreproc)
{
	if(m_ModelFile != nullptr)
		m_milPatModel = m_ModelFile->GetAngleModel(dAngle);
	else
		m_milPatModel.reset();
}

int CProcPil_Pattern::RotateImage(CString sPathImageOld, CString sPathImageNew, double dAngRotate)
{
	int ret = ModelFileLoad(sPathImageOld);
	if(ret == eMPAT_FAIL)
		return ret;

	cv::Mat srcMat = m_ModelFile->milModel->Image().clone();
	int w = srcMat.cols;
	int h = srcMat.rows;
	int ch =CV_8UC1;
	if(m_ModelFile->milModel->getModelnChannel() == 3)
		ch = CV_8UC3;

	//UCHAR* dst = new UCHAR[srcMat.cols*srcMat.rows];
	UCHAR* dst = g_pMManager->pem_new<UCHAR>(true, srcMat.cols*srcMat.rows, (PCHAR)__FUNCTION__, __LINE__);

	RotateImg_ipp(srcMat.data,srcMat.cols,srcMat.rows,dAngRotate,&dst,&w,&h);

	cv::Mat dstMat(h,w,ch,dst);
//	cv::imwrite("D:\\rotateImag.bmp",dstMat);
	std::shared_ptr<GeoModel_Pattern> tmpPatModel = std::make_shared<GeoModel_Pattern>();
	tmpPatModel->Alloc(dstMat);
	tmpPatModel->setAcceptance( m_ModelFile->milModel->getAcceptance() );
	tmpPatModel->setAccuracy( m_ModelFile->milModel->getAccuracy() );

	bool bEn(false);
	float fdNeg(0.0f), fdPos(0.0f), fdStep(0.0f);
	m_ModelFile->milModel->getAngleRange(bEn, fdNeg, fdPos, fdStep);
	tmpPatModel->setAngleRange(bEn, fdNeg, fdPos, fdStep);

	tmpPatModel->setAngle( 0 );
	tmpPatModel->Preprocess();

	tmpPatModel->setRes(m_resolX,m_resolY);
	tmpPatModel->SaveFile(sPathImageNew, false);

	tmpPatModel.reset();

	if(dst != NULL)
	{
		//delete [] dst;
		g_pMManager->pem_delete(dst, true);
		dst = NULL;
	}
}

void CProcPil_Pattern::SetAccuracy(int speedFactor, int accuracy, BOOL rotMode)
{
	GeoModel::enmSpeed spd;
	GeoModel::enmAccuracy acc;

	switch(speedFactor)
	{
	case M_LOW:
		spd = GeoModel::enmSpeed::spLow;
		break;
	case M_MEDIUM:
		spd = GeoModel::enmSpeed::spMedium;
		break;
	case M_HIGH:
		spd = GeoModel::enmSpeed::spHigh;
		break;
	}

	switch(accuracy)
	{
	case M_LOW:
		acc = GeoModel::enmAccuracy::acLow;
		break;
	case M_MEDIUM:
		acc = GeoModel::enmAccuracy::acMedium;
		break;
	case M_HIGH:
		acc = GeoModel::enmAccuracy::acHigh;
		break;
	}

	m_milPatModel->setSpeed(spd);
	m_milPatModel->setAccuracy(acc);

	if(rotMode == TRUE || m_milPatModel_Rotate != M_NULL)
	{
		m_milPatModel_Rotate->setSpeed(spd);
		m_milPatModel_Rotate->setAccuracy(acc);
	}
}

void CProcPil_Pattern::SetAngleMode(double posLimit, double negLimit, double accuracy, BOOL rotMode, bool bAngleModel)
{
	SetAccuracy(PAT_SPEED, PAT_ACCURACY, rotMode);

	m_milPatModel->setAngleRange(bAngleModel, negLimit, posLimit, (posLimit+negLimit)/5);
	m_milPatModel->setAcceptance(PAT_ATCSCORE);	
	

	if(rotMode == TRUE || m_milPatModel_Rotate != M_NULL)
	{
		m_milPatModel_Rotate->setAngleRange(bAngleModel, negLimit, posLimit, (posLimit+negLimit)/5);
		m_milPatModel_Rotate->setAcceptance(PAT_ATCSCORE);
	}
}

void CProcPil_Pattern::SetSearchStartAngle(double angle)
{
	m_milPatModel->setAngle(angle);
}
void CProcPil_Pattern::SetFindSubPixel(BOOL rotMode,bool bFindModel)
{
	m_milPatModel->setFindSubPixel(bFindModel);
	if(rotMode == TRUE || m_milPatModel_Rotate != M_NULL)
	{
		m_milPatModel_Rotate->setFindSubPixel(bFindModel);
	}
}

bool CProcPil_Pattern::SetParam_DivArea(BOOL bUseCharDiv, int* DiviLnC, int* DiviLnR, double ** GapLnC, double ** GapLnR, double ** DiviScore, bool bDetailSearch, RECT modelRect)
{
	if(m_ModelFile == nullptr)
		return false;

	m_SearchParam = std::shared_ptr<GeoMatchParam>(new GeoMatchParam);
	if(m_ModelFile->milModel->_DParam.m_bsetData)
	{
		if (m_ModelFile->milModel->_DParam.DiviLnC != 0 && m_ModelFile->milModel->_DParam.DiviLnC != 1)
		{
			int zero = m_ModelFile->milModel->_DParam.DiviLnC;
			double* GapC = m_ModelFile->milModel->_DParam.GapLnC;
			std::sort(GapC ,GapC + zero);

		} else if(m_ModelFile->milModel->_DParam.DiviLnR != 0 && m_ModelFile->milModel->_DParam.DiviLnR != 1)
		{
			int zero = m_ModelFile->milModel->_DParam.DiviLnR;
			double* GapR = m_ModelFile->milModel->_DParam.GapLnR;
			std::sort(GapR ,GapR + zero);
		}

		m_SearchParam->SetParam(m_ModelFile->milModel,m_ModelFile->milModel->_DParam.bUseCharDiv, m_ModelFile->milModel->_DParam.DiviLnC, m_ModelFile->milModel->_DParam.DiviLnR, m_ModelFile->milModel->_DParam.GapLnC, m_ModelFile->milModel->_DParam.GapLnR, m_ModelFile->milModel->_DParam.DiviScore, (CNT_PATTERN_DIVISION_C + 1), m_ModelFile->milModel->_DParam.DetailSearch, modelRect);
	}
	else
	{
		if(DiviLnC==0 && DiviLnR==0)
		{
			m_SearchParam.reset();
			return true;
		}

		m_SearchParam->SetParam(m_ModelFile->milModel, bUseCharDiv, *DiviLnC, *DiviLnR,*GapLnC, *GapLnR, *DiviScore, (CNT_PATTERN_DIVISION_C + 1), bDetailSearch, modelRect);
	}
	return true;
}

void CProcPil_Pattern::GetParam_DivArea(BOOL& bUseCharDiv, int & DiviLnC, int & DiviLnR, double * GapLnC, double * GapLnR, double * DiviScore, BOOL & bDetailSearch, RECT & modelRect)
{
	if(m_ModelFile == nullptr)
		return;

	m_SearchParam = std::shared_ptr<GeoMatchParam>(new GeoMatchParam);
	bUseCharDiv = m_ModelFile->milModel->_DParam.bUseCharDiv;
	DiviLnC = m_ModelFile->milModel->_DParam.DiviLnC;
	DiviLnR = m_ModelFile->milModel->_DParam.DiviLnR;
	memcpy(GapLnC, m_ModelFile->milModel->_DParam.GapLnC, sizeof(m_ModelFile->milModel->_DParam.GapLnC));
	memcpy(GapLnR, m_ModelFile->milModel->_DParam.GapLnR, sizeof(m_ModelFile->milModel->_DParam.GapLnR));
	memcpy(DiviScore, m_ModelFile->milModel->_DParam.DiviScore, sizeof(m_ModelFile->milModel->_DParam.DiviScore));
	bDetailSearch = m_ModelFile->milModel->_DParam.DetailSearch;

	m_SearchParam->SetParam(m_ModelFile->milModel, bUseCharDiv, DiviLnC, DiviLnR,GapLnC, GapLnR, DiviScore, (CNT_PATTERN_DIVISION_C + 1), bDetailSearch, modelRect);
}

int CProcPil_Pattern::GetMaskImage(CString sPath, UCHAR * Img, int Wid, int Len, int Pitch)
{
	if(ModelFileLoad(sPath) != eMPAT_SUCCESS)
		return eMPAT_FAIL;

	if(m_ModelFile->milModel == nullptr)
		return eMPAT_FAIL;
	
	cv::Mat Mask = m_ModelFile->milModel->GetMaskImage();

	int CpyWid = cv::min(Wid,Mask.cols);
	int CpyLen = cv::min(Len,Mask.rows);

	cv::Mat Dest(Len, Wid, CV_8UC1, Img, Pitch);

	for (int y=0; y<CpyLen; y++)
		memcpy(Dest.ptr(y), Mask.ptr(y), CpyWid);

	SaveWorkImg(Dest, _T("ppd_mask_dst_img.bmp"));

	return eMPAT_FAIL;
}

int CProcPil_Pattern::SetMaskImage(UCHAR * Img, int Wid, int Len, int Pitch, double dAngle)
{
	if(m_ModelFile==nullptr || m_ModelFile->milModel == nullptr)
		return eMPAT_FAIL;

	cv::Mat Src(Len, Wid, CV_8UC1, Img);
	cv::Mat Dest = GetAngleImg(Src, dAngle, true);
	SaveWorkImg(Dest, _T("ppd_mask_src_img.bmp"));

	m_ModelFile->milModel->SetMaskImage(Dest);
	m_ModelFile->milModel->setRes(m_resolX,m_resolY);
	m_ModelFile->milModel->SaveFile(m_ModelFile->milModel->getModelPath(), false);

	return eMPAT_FAIL;

}

bool CProcPil_Pattern::IsDivInspAvailable()
{
	return true;
}

void CProcPil_Pattern::ImgProcess(void* milSrc, void* milSrc_color, int cx, int cy, int width, int height,std::shared_ptr<ImgProcessing> Proc_Img)
{

	//CSize size = GetImageSize((Im::PIL_ID)milSrc);
	Im::PIL_ID milTemp = NULL;
	ImgProc = Proc_Img;
	if(m_milPatModel == nullptr)
		return;

	if(m_nChannel != 3)
	{
		milTemp = (Im::PIL_ID)milSrc;
	}
	else  milTemp = (Im::PIL_ID)milSrc_color;

	int Wid = Im::Buf::Inquire((Im::PIL_ID)milTemp, M_SIZE_X, M_NULL);
	int Len = Im::Buf::Inquire((Im::PIL_ID)milTemp, M_SIZE_Y, M_NULL);

	if(ImgProc->bIsImgProcessed)
	{

		int matchAlgo = m_milPatModel->getMatchAlgo();
		if(matchAlgo == GeoModel::enmMatchAlgo::agIntaglio || matchAlgo == GeoModel::enmMatchAlgo::agRelief || matchAlgo == GeoModel::enmMatchAlgo::agValue)
		{
			if(ImgProc->_ProcBuf[GeoModel::enmDepth::spOrg].Sdl.cols > 0  && ImgProc->_ProcBuf[GeoModel::enmDepth::spOrg].Sdl.rows > 0
				&&(ImgProc->IntaglioModelStep == m_milPatModel->_pyrDownStep))
				return;
		}
		else if(matchAlgo == GeoModel::enmMatchAlgo::agDefault || matchAlgo == GeoModel::enmMatchAlgo::agLowContrast || matchAlgo == GeoModel::enmMatchAlgo::agOriginal)
		{
			if(ImgProc->_ProcBuf[GeoModel::enmDepth::spOrg].Sdx.cols > 0 && ImgProc->_ProcBuf[GeoModel::enmDepth::spOrg].Sdx.rows > 0
				&&(ImgProc->edgeModelStep == m_milPatModel->_pyrDownStep))
				return;
		}
		else
		{
			if(ImgProc->_ProcBuf[GeoModel::enmDepth::spOrg].Integral.cols>0  && ImgProc->_ProcBuf[GeoModel::enmDepth::spOrg].Integral.rows>0
				&& ImgProc->_ProcBuf[GeoModel::enmDepth::spOrg].Integral.channels() == m_nChannel)
				return;
		}
	}


	UCHAR * pData = NULL;

	int Pitch = Wid * m_nChannel;
	//int nSizeX = Pitch % 4;
	//if (nSizeX!=0)
	//	Pitch += 4 - nSizeX;

	Pitch = g_pMPTI->nCalcWidthStep(false, Pitch);

	if(m_nChannel != 3)	Pitch = Im::Buf::Inquire(milTemp, M_PITCH_BYTE, M_NULL);

	if(m_nChannel == 3)
	{
		//pData = new UCHAR[Wid*Len*m_nChannel];
		pData = g_pMManager->pem_new<UCHAR>(true, Wid*Len*m_nChannel, (PCHAR)__FUNCTION__, __LINE__);
		Im::Buf::GetColor(milTemp, M_PACKED + M_BGR24, M_ALL_BANDS, pData);
	}
	else
	{
		pData = (uchar *)Im::Buf::Inquire(milTemp, M_HOST_ADDRESS, M_NULL);
	}

	cv::Mat srcImg(Len, Wid, CV_MAKETYPE(CV_8U,m_nChannel));

	if(m_nChannel==3)
	{
		for(int y=0; y< height ; y++)
			memcpy(&srcImg.data[y*srcImg.step], &pData[y*(width * 3)], width * 3);
	}
	else
	{
		cv::Mat srcImg_1(Len, Wid, /*CV_8UC1*/CV_MAKETYPE(CV_8U,m_nChannel), pData, Pitch);
		srcImg = srcImg_1;
	}
	//cv::Mat srcImg(Len, Wid, /*CV_8UC1*/CV_MAKETYPE(8,m_nChannel), pData, Pitch);
	//if(pData && m_nChannel==3) delete [] pData; pData = NULL;
	if (pData && m_nChannel == 3) g_pMManager->pem_delete(pData, true); pData = NULL;

#ifdef _PROC_IMG_SAVE
	CString sSrcName;
	sSrcName.Format(_T("SearchPatternsrc_%.0f.bmp"), m_milPatModel->getImageAngle());
	SaveWorkImg(srcImg, sSrcName);

	CString sPatName;
	sPatName.Format(_T("SearchPatternpat_%.0f.bmp"), m_milPatModel->getImageAngle());

	SaveWorkImg(m_nChannel==1 ? m_milPatModel->Image() : m_milPatModel->color_Image(), sPatName);
#endif

	ImgProc->_ImgProcess(srcImg,*m_milPatModel);
	if(!m_SearchParam->_bDivDetailSearch)
	{

		int k=1;
		cv::Mat MorphImg;
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1+k*2, 1+k*2), cv::Point(2, 2) );
		cv::erode(srcImg,MorphImg,kernel);
		ImgProc_E1->_ImgProcess(MorphImg,*m_milPatModel);
		cv::dilate(srcImg,MorphImg,kernel);
		ImgProc_D1->_ImgProcess(MorphImg,*m_milPatModel);
		k++;

		kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1+k*2, 1+k*2), cv::Point(2, 2) );
		cv::erode(srcImg,MorphImg,kernel);
		ImgProc_E2->_ImgProcess(MorphImg,*m_milPatModel);
		cv::dilate(srcImg,MorphImg,kernel);
		ImgProc_D2->_ImgProcess(MorphImg,*m_milPatModel);
		k++;

		kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1+k*2, 1+k*2), cv::Point(2, 2) );
		cv::erode(srcImg,MorphImg,kernel);
		ImgProc_E3->_ImgProcess(MorphImg,*m_milPatModel);
		cv::dilate(srcImg,MorphImg,kernel);
		ImgProc_D3->_ImgProcess(MorphImg,*m_milPatModel);
	}
}

void CProcPil_Pattern::ImgProcess(UCHAR* userSrc, int width, int height,std::shared_ptr<ImgProcessing> Proc_Img)
{

	Im::PIL_ID milTemp = M_NULL;
	ImgProc = Proc_Img;

	if(m_milPatModel == nullptr)
		return ;

	if(ImgProc->bIsImgProcessed)
	{

		int matchAlgo = m_milPatModel->getMatchAlgo();
		if(matchAlgo == GeoModel::enmMatchAlgo::agIntaglio || matchAlgo == GeoModel::enmMatchAlgo::agRelief || matchAlgo == GeoModel::enmMatchAlgo::agValue)
		{
			if(ImgProc->_ProcBuf[GeoModel::enmDepth::spOrg].Sdl.cols>0  && ImgProc->_ProcBuf[GeoModel::enmDepth::spOrg].Sdl.rows>0
				&&(ImgProc->IntaglioModelStep == m_milPatModel->_pyrDownStep))
				return;
		}
		else if(matchAlgo == GeoModel::enmMatchAlgo::agDefault || matchAlgo == GeoModel::enmMatchAlgo::agLowContrast || matchAlgo == GeoModel::enmMatchAlgo::agOriginal)
		{
			if(ImgProc->_ProcBuf[GeoModel::enmDepth::spOrg].Sdx.cols>0  && ImgProc->_ProcBuf[GeoModel::enmDepth::spOrg].Sdx.rows>0
				&&(ImgProc->edgeModelStep == m_milPatModel->_pyrDownStep))
				return;
		}
		else
		{
			if(ImgProc->_ProcBuf[GeoModel::enmDepth::spOrg].Integral.cols>0  && ImgProc->_ProcBuf[GeoModel::enmDepth::spOrg].Integral.rows>0)
				return;
		}
	}
#ifdef _PROC_IMG_SAVE
	CString sPatName;
	sPatName.Format(_T("SearchPatternpat_%.0f.bmp"), m_milPatModel->getImageAngle());
	SaveWorkImg(m_nChannel==1 ? m_milPatModel->Image() : m_milPatModel->color_Image(), sPatName);
#endif


	cv::Mat srcImg;
	srcImg = cv::Mat::zeros(height, width, CV_MAKETYPE(CV_8U,m_nChannel));

	if(m_nChannel == 3)
	{
		for (int y=0; y<height; y++)
			memcpy(srcImg.ptr(y), &userSrc[y*(width*3)], width*3);
	}
	else
	{
		for (int y=0; y<height; y++)
			memcpy(srcImg.ptr(y), &userSrc[y*width], width);
	}

#ifdef _PROC_IMG_SAVE
	CString sSrcName;
	sSrcName.Format(_T("SearchPatternsrc_%.0f.bmp"), m_milPatModel->getImageAngle());
	SaveWorkImg(srcImg, sSrcName);
#endif

	ImgProc->_ImgProcess(srcImg,*m_milPatModel);
}
cv::Mat CProcPil_Pattern::GetAngleImg(cv::Mat & src, float angle, bool bInterpolation)
{
	//cv::Point2f center(src.cols/2.0, src.rows/2.0);
	//cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);

	//float radian = angle / 180.0f * M_PI;
	//float sinth = sin(radian);
	//float costh = cos(radian);

	//float lt_x = (-center.x) * costh + ( center.y) * sinth;
	//float lt_y =-(-center.x) * sinth + ( center.y) * costh;

	//float lb_x = (-center.x) * costh + (-center.y) * sinth;
	//float lb_y =-(-center.x) * sinth + (-center.y) * costh;

	//float rt_x = ( center.x) * costh + ( center.y) * sinth;
	//float rt_y =-( center.x) * sinth + ( center.y) * costh;

	//float rb_x = ( center.x) * costh + (-center.y) * sinth;
	//float rb_y =-( center.x) * sinth + (-center.y) * costh;

	//float min_x = std::min(std::min(std::min(lt_x, lb_x), rt_x), rb_x);
	//float max_x = std::max(std::max(std::max(lt_x, lb_x), rt_x), rb_x);
	//float min_y = std::min(std::min(std::min(lt_y, lb_y), rt_y), rb_y);
	//float max_y = std::max(std::max(std::max(lt_y, lb_y), rt_y), rb_y);
	//float sz_x = max_x - min_x;
	//float sz_y = max_y - min_y;

	//rot.at<double>(0,2) += sz_x/2.0 - center.x;
	//rot.at<double>(1,2) += sz_y/2.0 - center.y;

	cv::Mat dst;
	
	GeoModel gm;
	gm.RotateImg_ipp(src, angle, &dst, bInterpolation);	
	
	return dst;
}
#if 0
void CProcPil_Pattern::EdgeAlgoCheckback()
{
	int nLine = __LINE__;
	try
	{
		cv::Mat img = *ImgProc->_OrgImage;
		if(m_milPatModel->getMatchAlgo()<=GeoModel::enmMatchAlgo::agLowContrast)
		{
			cv::Mat scoreMat;
			if(img.cols<m_milPatModel->Image().cols  || img.rows< m_milPatModel->Image().rows)
				return;
			int stx;
			int sty;
			int rew;
			int reh;
			double Min, Max;
			cv::Point	ptMin, ptMax;
			for (int i=0; i<m_milPatResult->getResCnt(); i++)
			{
				rew = m_milPatModel->Image().cols;
				reh = m_milPatModel->Image().rows;
				stx = (int)(m_milPatResult->_Center_y[i]+0.49f - rew/2);
				sty = (int)(m_milPatResult->_Center_x[i]+0.49f - reh/2);
				if(stx<0)
					stx = 0;
				if(sty<0)
					sty = 0;
				if(stx+rew>img.cols-1)
					stx = img.cols - rew;
				if(sty+reh>img.rows-1)
					sty = img.rows - reh;
				cv::Mat MatchImg = img(cv::Rect(stx,sty,rew,reh));
				cv::matchTemplate(MatchImg, m_milPatModel->Image(), scoreMat, cv::TM_CCOEFF_NORMED);
				cv::minMaxLoc(scoreMat, &Min, &Max, &ptMin, &ptMax);
				Max *=100.f;
				if(Max < 50 && Max < m_milPatResult->_Equality[i])
				{
					m_milPatResult->_Equality[i] = Max;
					if(m_SearchParam->_nAreaCnt_C>1)
					{
						for (int x=0; x<m_SearchParam->_nAreaCnt_C; x++)
						{
							for (int c=0; c<m_SearchParam->_nCharAreaCnt; c++)
							{
								m_milPatResult->_Score[0].Score[x][c] = Max;
							}
						}
					}
				}
			}

		}
	}
	catch (CMemoryException* e)
	{
		CString msg;
		msg.Format(_T("EdgeAlgoCheckback CMemoryException Line: %d"),nLine);
		g_pMPTI->AddLog_OCR(msg);
	}
	catch (CException* e)
	{
		CString msg;
		msg.Format(_T("EdgeAlgoCheckback CException Line: %d"),nLine);
		g_pMPTI->AddLog_OCR(msg);
	}
	catch (...)
	{
		CString msg;
		msg.Format(_T("EdgeAlgoCheckback Exception Line: %d"),nLine);
		g_pMPTI->AddLog_OCR(msg);
	}

}
#endif

int CProcPil_Pattern::AllocModePatlexceptMask(UCHAR* userSrc, int width, int height, double rotateAngle, int rotateW, int rotateH)
{
	int ret  = eMPAT_SUCCESS;

	if(userSrc == nullptr)
		return eMPAT_FAIL;

	Im::PIL_ID milSrc = NULL;

	milSrc = AllocBuff(width, height);
	Im::Buf::Put(milSrc, userSrc);

	if(m_milPatModel == nullptr)
		m_milPatModel = std::shared_ptr<GeoModel_Pattern>(new GeoModel_Pattern());

	cv::Mat srcImg(height, width, CV_8UC1);

	for (int y=0; y<height; y++)
		memcpy(srcImg.ptr(y), &userSrc[y*width], width);

	cv::bitwise_not(srcImg,srcImg);

	m_milPatModel->SetMaskImage(srcImg);

	if(rotateAngle > 0)
	{
		Im::PIL_ID rotateImg = AllocRotateBuff(milSrc, -rotateAngle);
		int Wid = Im::Buf::Inquire((Im::PIL_ID)rotateImg, M_SIZE_X, M_NULL);
		int Len = Im::Buf::Inquire((Im::PIL_ID)rotateImg, M_SIZE_Y, M_NULL);
		int Pitch = Im::Buf::Inquire((Im::PIL_ID)rotateImg, M_PITCH_BYTE, M_NULL);
		uchar * pData = (uchar *)Im::Buf::Inquire((Im::PIL_ID)rotateImg, M_HOST_ADDRESS, M_NULL);

		cv::Mat rotImg(Len, Wid, CV_8UC1);
		for (int y=0; y<Len; y++)
			memcpy(rotImg.ptr(y), &pData[y*Pitch], Wid);

		cv::bitwise_not(rotImg,rotImg);
		if(m_milPatModel_Rotate == nullptr)
			m_milPatModel_Rotate = std::shared_ptr<GeoModel_Pattern>(new GeoModel_Pattern());
		m_milPatModel_Rotate->SetMaskImage(rotImg);
		FreeMilImageBuff(&rotateImg);
	}

	FreeMilImageBuff(&milSrc);
	srcImg.release();
	return ret;
}
SIZE CProcPil_Pattern::getModelSz()
{
	SIZE rst;
	rst.cx = 0;
	rst.cy = 0;
	if (m_milPatModel == nullptr)
		return rst;
	rst.cx = m_milPatModel->Image().cols;
	rst.cy = m_milPatModel->Image().rows;
	return rst;
}