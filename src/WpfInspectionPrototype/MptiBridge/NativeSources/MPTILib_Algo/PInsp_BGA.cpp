#include "StdAfx.h"
#include "PInsp_BGA.h"
#include <math.h>

#define SAMPLE_RECT_SIZE	5       //5*5



CPInsp_BGA::CPInsp_BGA(void)
{
	m_className = _T("CPInsp_BGA");

	m_inspParam = NULL;
	m_teachParam = NULL;

	m_fovImage_insp = NULL;
	m_fovImage_teach = NULL;

	m_fovImage_T = NULL;
	m_fovImage_M = NULL;
	m_fovImage_B = NULL;

	InitResultStruct(&m_inspResult);
}


CPInsp_BGA::~CPInsp_BGA(void)
{
	CloseDevice();
}

int CPInsp_BGA::InitDevice(Im::PIL_ID* milApp, Im::PIL_ID* milSys, int fovWidth, int fovLength, double resolX, double resolY, bool bUseImagePilLib)
{
	m_fovWidth = fovWidth;
	m_fovLength = fovLength;
	m_resolX = resolX;
	m_resolY = resolY;

	//m_procMil = new CProcMil_BGA();
	m_procMil = g_pMManager->pem_new<CProcMil_BGA>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
	m_procMil->InitMil(milApp, milSys,bUseImagePilLib);
	m_procMil->SetResol(m_resolX, m_resolY, m_fovWidth);

	CPInsp::InitDevice(NULL, NULL, fovWidth, fovLength, resolX, resolY, bUseImagePilLib);

	return ePART_SUCCESS;
}

int CPInsp_BGA::CloseDevice()
{

	if(m_procMil != NULL)
	{
		m_procMil->FreeMil();
		
		//delete m_procMil;
		g_pMManager->pem_delete(m_procMil, false);
		m_procMil = NULL;
	}

	CPInsp::CloseDevice();

	return ePART_SUCCESS;	
}

int CPInsp_BGA::SetInspParam(void* itemParam, void* targetImg, ZmapData zmap, Coordinate cdn)
{
	m_inspParam = (InspParamBGA*)itemParam;
	m_fovImage_insp = targetImg;
	m_inspZmapData =  zmap;
	m_inspCoordinate = cdn;

	return ePART_SUCCESS;
}

int CPInsp_BGA::SetInspParam(void* itemParam, InspImgBuf img, ZmapData zmap, Coordinate cdn)
{
	m_inspParam = (InspParamBGA*)itemParam;
	m_inspZmapData =  zmap;
	m_inspCoordinate = cdn;

	m_fovImage_T = img.imgTop_R;
	m_fovImage_M = img.imgMiddle_R;
	m_fovImage_B = img.imgBottom_R;

	return ePART_SUCCESS;
}

int CPInsp_BGA::SetTeachParam(void* itemParam, void* targetImg, int teachType, ZmapData zmap, Coordinate cdn)
{
	m_teachParam = (TeachParamBGA*) itemParam;
	m_fovImage_teach = targetImg;
	m_teachZmapData = zmap;
	m_teachCoordinate = cdn;

	return ePART_SUCCESS;
}

int CPInsp_BGA::SetTeachParam(void* itemParam, InspImgBuf img, int teachType, ZmapData zmap, Coordinate cdn)
{
	m_teachParam = (TeachParamBGA*) itemParam;
	m_teachZmapData = zmap;
	m_teachCoordinate = cdn;

	m_fovImage_T = img.imgTop_R;
	m_fovImage_M = img.imgMiddle_R;
	m_fovImage_B = img.imgBottom_R;

	return ePART_SUCCESS;
}

int CPInsp_BGA::InspProc_BGA()
{
	int ret = 0;

	ret = ProcAuto();

	return ret;
}

int CPInsp_BGA::ProcAuto()
{
	int ret = e_OK;

	int stepID = 0;
	int maxStepCnts = eBGAStepID_COUNTS;

	//step1(eBGAStepID_HEIGHT) : Height
	//step2(eBGAStepID_ECCENTRICITY) : eccentricity
	//step3(eBGAStepID_SIZE) : size (height X)


	InitResultStruct(&m_inspResult);
	InitResultStruct(&m_inspParam->retInspResult);

	int tempRst = e_OK;
	for(int i = 0; i < maxStepCnts; i++)
	{
		tempRst = ProcStep(i);

		if(tempRst != e_OK)
			ret = tempRst;
	}

	return ret;
}


int CPInsp_BGA::ProcStep(int stepID)
{
	int ret = e_OK;

	switch(stepID)
	{
	case eBGAStepID_HEIGHT:
		{
			ret = BGA_Calculate();
		}
		break;
	case eBGAStepID_ECCENTRICITY:
		{
			ret = Eccentricity();
		}
		break;
	case eBGAStepID_SIZE:
		{
			ret = ComponentSize();
		}
		break;
	}

	return ret;
}

int CPInsp_BGA::BGA_Calculate()
{
	int ret = e_OK;

	int cx = m_inspCoordinate.cx;
	int cy = m_inspCoordinate.cy;
	int roiSizeX = m_inspCoordinate.width;
	int roiSizeY = m_inspCoordinate.length;
	float* zmapData = m_inspZmapData.data;
	int zmapSizeX = m_inspZmapData.zmapSizeX;
	int zmapSizeY = m_inspZmapData.zmapSizeY;

	int lowerLimit_ball = m_inspParam->lowerLimit_ball;
	int lowerLimit_board = m_inspParam->lowerLimit_board;
	double stdHeight_Abs = m_inspParam->stdH_Abs;
	double stdHeight_Rel = m_inspParam->stdH_Rel;
	double range_hMax = m_inspParam->range_MaxHeight;
	double range_hMin = m_inspParam->range_MinHeight;

	int roiArea = roiSizeX * roiSizeY;

	if(roiSizeX < 1 || roiSizeY < 1 || roiArea < 1 || zmapData == NULL)
		return e_NG;

	//////////////////////////////////////////////////////////////////////////
	//roi영역만큼 추출
	//float* zmapRoiData = new float[roiArea];
	float* zmapRoiData = g_pMManager->pem_new<float>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
	//memset(zmapRoiData, 0, sizeof(float) * roiArea);
	m_procMil->ClipZmap(zmapData, zmapRoiData, zmapSizeX, zmapSizeY, cx, cy, roiSizeX, roiSizeY);

	//UCHAR* sumImg = new UCHAR[zmapSizeX * zmapSizeY];
	UCHAR* sumImg = g_pMManager->pem_new<UCHAR>(true, zmapSizeX * zmapSizeY, (PCHAR)__FUNCTION__, __LINE__);
	m_procMil->MakeImage_Sum(m_fovImage_T, m_fovImage_M, m_fovImage_B, sumImg);
	m_procMil->SaveWorkImg(sumImg, zmapSizeX, zmapSizeY, _T("sumImg.bmp"));

	//UCHAR* fovImg = new UCHAR[roiArea];
	UCHAR* fovImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
	//memset(fovImg, 0, sizeof(UCHAR) * roiArea);
	m_procMil->GetClipImage(sumImg, zmapSizeX, zmapSizeY, fovImg, cx, cy, roiSizeX, roiSizeY);
	//m_procMil->GetClipBuff(m_fovImage_teach, fovImg, cx, cy, roiSizeX, roiSizeY);
	//delete sumImg;
	g_pMManager->pem_delete(sumImg, false);

	//////////////////////////////////////////////////////////////////////////
	//m_procMil->SaveWorkImg(fovImg, roiSizeX, roiSizeY, _T("bgaSrcImg.bmp"));

	/*UCHAR* blobImg = new UCHAR[roiArea];
	UCHAR* lutImg = new UCHAR[roiArea];*/
	UCHAR* blobImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
	UCHAR* lutImg = g_pMManager->pem_new<UCHAR>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);
	//memset(blobImg, 0, sizeof(UCHAR) * roiArea);
	int blobCnt = m_procMil->BlobProc(fovImg, roiSizeX, roiSizeY, blobImg, lutImg);
	m_procMil->SaveWorkImg(blobImg, roiSizeX, roiSizeY, _T("finalBlob.bmp"));
	m_procMil->SaveWorkImg(lutImg, roiSizeX, roiSizeY, _T("lutImg.bmp"));

	CMilBlobResult* retRst = NULL;
	double cogX_board = 0.0;
	double cogY_board = 0.0; 
	double cogX_pixel = 0.0;
	double cogY_pixel = 0.0;
	double diameter = 0.0;
	double blobWidth = 0.0;
	double blobHeight = 0.0;
	float absH = 0;
	float relH = 0;
	if(blobCnt > 0)
	{
		//2d
		//retRst = new CMilBlobResult(blobCnt);
		retRst = new CMilBlobResult(blobCnt);
		g_pMManager->pem_new_check(retRst, (PCHAR)__FUNCTION__, __LINE__);

		m_procMil->GetBlobResult(retRst);

		diameter = retRst->diameter[0];
		blobWidth = retRst->width[0];
		blobHeight = retRst->length[0];

		cogX_pixel = (cx - (roiSizeX / 2.0)) + retRst->cx[0];
		cogY_pixel = (cy - (roiSizeY / 2.0)) + retRst->cy[0];		
		CvtPixelToBoard(m_inspCoordinate.fovCx, m_inspCoordinate.fovCy, cogX_pixel, cogY_pixel, &cogX_board, &cogY_board);	

		//3d
	
		//int lutCnt = m_procMil->MakeLUT(fovImg, roiSizeX, roiSizeY, lutImg);
		GetHeight2(zmapRoiData, blobImg, lutImg, roiSizeX, roiSizeY, (int)retRst->cx[0], (int)retRst->cy[0], SAMPLE_RECT_SIZE, lowerLimit_ball, &absH, &relH);
		//GetHeight(zmapRoiData, blobImg, lutImg, roiSizeX, roiSizeY, (int)retRst->cx[0], (int)retRst->cy[0], SAMPLE_RECT_SIZE, 5, &absH, &relH);
	}
	else
	{
		ret = e_NG;
	}

	m_inspResult.dia = _pixel2mm_x(diameter);
	m_inspResult.cogX = cogX_board;
	m_inspResult.cogY = cogY_board;
	m_inspResult.cogX_pixel = cogX_pixel;
	m_inspResult.cogY_pixel = cogY_pixel;
	m_inspResult.height_Abs = _micron2mm(absH);
	m_inspResult.height_Rel = _micron2mm(relH);
	m_inspResult.width = _pixel2mm_x(blobWidth);
	m_inspResult.height = _pixel2mm_y(blobHeight);

	//////////////////////////////////////////////////////////////////////////
	double hMax = stdHeight_Abs * (range_hMax / 100.0);
	double hMin = stdHeight_Abs * (range_hMin / 100.0);
	if( m_inspResult.height_Abs < 0 || m_inspResult.height_Abs > hMax || m_inspResult.height_Abs < hMin)  //   range_hMin% <  avr  < range_hMax%    wrong
	{
		ret = e_NG;
		m_inspResult.ok = FALSE;

		m_inspResult.list.heightOk = FALSE;
	}
	else
	{
		ret = e_OK;
		m_inspResult.ok = TRUE;
		m_inspResult.list.heightOk = TRUE;
	}


	if(ret == e_NG)	
		m_inspResult.ok = FALSE;

	m_inspResult.isInsp = TRUE;


	//////////////////////////////////////////////////////////////////////////
	/*delete zmapRoiData;
	delete fovImg;
	delete blobImg;*/
	g_pMManager->pem_delete(zmapRoiData, false);
	g_pMManager->pem_delete(fovImg, false);
	g_pMManager->pem_delete(blobImg, false);
	

	
	if(lutImg != NULL)
		//delete lutImg;
		g_pMManager->pem_delete(lutImg, false);
	if(retRst != NULL)
		//delete retRst;
		g_pMManager->pem_delete(retRst, false);

	return ret;
}

int CPInsp_BGA::Eccentricity()
{
	int ret = e_OK;

	double stdCogX = m_inspParam->stdCogX; //mm
	double stdCogY = m_inspParam->stdCogY; //mm

	double range = m_inspParam->range_offset; //mm

	double curCogX = m_inspResult.cogX; //mm
	double curCogY = m_inspResult.cogY; //mm


	double offsetX;  //mm
	double offsetY;  //mm
	offsetX = curCogX - stdCogX;	
	offsetY = curCogY - stdCogY;



	//////////////////////////////////////////////////////////////////////////
	if(abs(offsetX) > range || abs(offsetY) > range)
	{
		ret = e_NG;
		m_inspResult.ok = FALSE;


		//defect viewer....
		if(abs(offsetX) > range)
			m_inspResult.list.offsetXOk = FALSE;

		if(abs(offsetY) > range)
			m_inspResult.list.offsetYOk = FALSE;
	}
	else
	{
		ret = e_OK;

		if(m_inspResult.ok != FALSE)
			m_inspResult.ok = TRUE;
	}


	m_inspResult.offsetX = offsetX;
	m_inspResult.offsetY = offsetY;



	return ret;
}

int CPInsp_BGA::ComponentSize()
{
	int ret = e_OK;

	double stdDiameter = m_inspParam->stdDia;
	double range_MaxDia = m_inspParam->range_MaxDia;
	double range_MinDia = m_inspParam->range_MinDia;

	double curDiameter = m_inspResult.dia;

	double diaMax = stdDiameter * (range_MaxDia / 100.0);
	double diaMin = stdDiameter * (range_MinDia / 100.0);

	if( curDiameter < 0 || curDiameter > diaMax || curDiameter < diaMin)  //   range_hMin% <  avr  < range_hMax%    wrong
	{
		ret = e_NG;
		m_inspResult.ok = FALSE;

		m_inspResult.list.diameterOk = FALSE;
	}
	else
	{
		ret = e_OK;

		if(m_inspResult.ok != FALSE)
			m_inspResult.ok = TRUE;
	}
	


	return ret;
}

void CPInsp_BGA::GetHeight(float* zmapData, UCHAR* blobImg, UCHAR* lutImg, int sizeX, int sizeY, int blobCx, int blobCy, int sampleRectSize, double hPercent, float* retAbsH, float* retRelH)
{
	float ballHeight = 0;
	float boardHeight = 0;
	double sum = 0;
	int count = 0;
	int imgSize = sizeX * sizeY;

	if(imgSize < 1)
		return;
	
	//////////////////////////////////////////////////////////////////////////
	//1. blob 중심에서 sampleRectSize * sampleRectSize 사이즈 만큼 추출하여 평균..
	int sampleSize = sampleRectSize * sampleRectSize;
	//float* clipZmap = new float[sampleSize];
	float* clipZmap = g_pMManager->pem_new<float>(true, sampleSize, (PCHAR)__FUNCTION__, __LINE__);
	m_procMil->ClipZmap(zmapData, clipZmap, sizeX, sizeY, blobCx, blobCy, sampleRectSize, sampleRectSize);

	
	for(int i = 0; i < sampleSize; i++)
	{
		sum += clipZmap[i];
		count++;
	}
	ballHeight = (float)(sum / count);
	//delete clipZmap;
	g_pMManager->pem_delete(clipZmap, false);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//2. blob전체의 높이를 오름차순으로 정리하여 상위 hPercent만큼 추출하여 평균
/*	float* hData = new float[imgSize];
	memset(hData, 0, sizeof(float) * imgSize);
	//블랍 부분만 높이 골라 내기
	sum = 0;
	count = 0;
	for(int i = 0; i < imgSize; i++)
	{
		if(blobImg[i] > 0)
		{
			hData[count] = zmapData[i];
			count++;
		}
	}
	if(count < 1)
		return;
	//골라낸 높이를 오름 차순으로 소팅
	float* sortData = new float[count];
	memset(sortData, 0, sizeof(float) * count);
	DataSort(hData, count, sortData);

	//소팅된 데이터에서 필요한 데이터만 더해서 평균..
	int topCount = (int)(count * (hPercent / 100.0));
	for(int i = 0; i < topCount; i++)
	{
		sum += sortData[i];
	}
	ballHeight = (float)(sum / topCount);

	delete hData;*/
	//////////////////////////////////////////////////////////////////////////

	
	sum = 0;
	count = 0;
	for(int i = 0; i < imgSize; i++)
	{
		if(lutImg[i] == 0)
		{
			sum += zmapData[i];
			count++;
		}
	}
	boardHeight = (float)(sum / count);

	*retAbsH = ballHeight;
	*retRelH = ballHeight - boardHeight;
}

void CPInsp_BGA::GetHeight2(float* zmapData, UCHAR* blobImg, UCHAR* lutImg, int sizeX, int sizeY, int blobCx, int blobCy, int sampleRectSize, double hPercent, float* retAbsH, float* retRelH)
{
	float ballHeight = 0;
	float boardHeight = 0;
	double sum = 0;
	int count = 0;
	int imgSize = sizeX * sizeY;

	if(imgSize < 1)
		return;
	
	//////////////////////////////////////////////////////////////////////////
	//1. blob 중심에서 sampleRectSize * sampleRectSize 사이즈 만큼 추출하여 평균..
	int sampleSize = sampleRectSize * sampleRectSize;
	//float* clipZmap = new float[sampleSize];
	float* clipZmap = g_pMManager->pem_new<float>(true, sampleSize, (PCHAR)__FUNCTION__, __LINE__);
	m_procMil->ClipZmap(zmapData, clipZmap, sizeX, sizeY, blobCx, blobCy, sampleRectSize, sampleRectSize);

	
	for(int i = 0; i < sampleSize; i++)
	{
		sum += clipZmap[i];
		count++;
	}
	ballHeight = (float)(sum / count);
	//delete clipZmap;
	g_pMManager->pem_delete(clipZmap, false);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//2. blob전체의 높이를 오름차순으로 정리하여 상위 hPercent만큼 추출하여 평균
/*	float* hData = new float[imgSize];
	memset(hData, 0, sizeof(float) * imgSize);
	//블랍 부분만 높이 골라 내기
	sum = 0;
	count = 0;
	for(int i = 0; i < imgSize; i++)
	{
		if(blobImg[i] > 0)
		{
			hData[count] = zmapData[i];
			count++;
		}
	}
	if(count < 1)
		return;
	//골라낸 높이를 오름 차순으로 소팅
	float* sortData = new float[count];
	memset(sortData, 0, sizeof(float) * count);
	DataSort(hData, count, sortData);

	//소팅된 데이터에서 필요한 데이터만 더해서 평균..
	int topCount = (int)(count * (hPercent / 100.0));
	for(int i = 0; i < topCount; i++)
	{
		sum += sortData[i];
	}
	ballHeight = (float)(sum / topCount);

	delete hData;*/
	//////////////////////////////////////////////////////////////////////////

	
	sum = 0;
	count = 0;
	float avrTemp = 0;
	for(int i = 0; i < imgSize; i++)
	{
		if(lutImg[i] == 0)
		{
			sum += zmapData[i];
			count++;
		}
	}
	avrTemp = (float)(sum / count);

	sum = 0;
	count = 0;
	for(int i = 0; i < imgSize; i++)
	{
		if(lutImg[i] == 0)
		{
			if(zmapData[i] > avrTemp)
			{
				sum += zmapData[i];
				count++;
			}
		}
	}
	boardHeight = (float)(sum / count);

	*retAbsH = ballHeight;
	*retRelH = ballHeight - boardHeight;
}

void CPInsp_BGA::DataSort(float* srcData, int dataCnt, float* dstData)
{
	if(dataCnt < 1)
		return;

	//float* dataTemp = new float[dataCnt];
	float* dataTemp = g_pMManager->pem_new<float>(true, dataCnt, (PCHAR)__FUNCTION__, __LINE__);
	//memset(dataTemp, 0, sizeof(float) * dataCnt);
	memcpy_s(dataTemp, sizeof(float) * dataCnt, srcData, sizeof(float) * dataCnt );

	float temp = 0;
	for(int i = 0; i < dataCnt; i++)
	{
		for(int j = i+1; j < dataCnt; j++)
		{
			if(dataTemp[i] < dataTemp[j])
			{
				temp = dataTemp[j];
				dataTemp[j] = dataTemp[i];
				dataTemp[i] = temp;
			}
		}
	}

	if(dstData != NULL)
		memcpy_s(dstData, sizeof(float) * dataCnt, dataTemp, sizeof(float) * dataCnt );

	//delete dataTemp;
	g_pMManager->pem_delete(dataTemp, false);
}


float CPInsp_BGA::GetHeight_REL(float* zmapData, int sizeX, int sizeY, UCHAR* blobImg)
{
	float height = 0;
	float boardHeight = 0;

	int size = sizeX * sizeY;
	



	return height;
}

void CPInsp_BGA::CalcCog(double angle, int cx, int cy, int sizeX, int sizeY, double blobCx, double blobCy, double* retCogX, double* retCogY)
{
	double cogX = 0.0;
	double cogY = 0.0;

	if(angle == 90.0 || angle || 270.0)
	{
		cogX = (cx - (sizeY / 2.0)) + blobCy;
		cogY = (cy - (sizeX / 2.0)) + blobCx;
	}
	else
	{
		cogX = (cx - (sizeX / 2.0)) + blobCx;
		cogY = (cy - (sizeY / 2.0)) + blobCy;
	}

	*retCogX = cogX;
	*retCogY = cogY;
}


void CPInsp_BGA::InitResultStruct(RstInspBGA_Wnd* rst)
{
	//RstInspBGA_Wnd* temp = new RstInspBGA_Wnd;
	RstInspBGA_Wnd* temp = g_pMManager->pem_new<RstInspBGA_Wnd>(false, 0, (PCHAR)__FUNCTION__, __LINE__);
	memset(temp, 0, sizeof(RstInspBGA_Wnd));

	temp->ok = -1;
	temp->isInsp = FALSE;

	temp->list.offsetXOk = TRUE;
	temp->list.offsetYOk = TRUE;
	temp->list.diameterOk = TRUE;
	temp->list.heightOk = TRUE;


	*rst = *temp;
	//delete temp;
	g_pMManager->pem_delete(temp, false);
}


void CPInsp_BGA::GetInspRst(RstInspBGA_Wnd* dst)
{
	*dst = m_inspResult;
}
