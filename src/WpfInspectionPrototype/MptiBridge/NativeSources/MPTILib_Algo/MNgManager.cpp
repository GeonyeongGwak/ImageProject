#include "StdAfx.h"
#include "MPTI.h"
#include "MNgManager.h"
#include "Proc3D.h"
#include "alp_alloc.h"

#include "SharedMemory.h" // SHKang 2017/03/13
#include <io.h>


#define NGPATH     _T("d:\\ngimage\\")

// SHKang 2017/03/13
#define	LMTI(i)	(i < 0.f) ? 0.f : ((i > 255.f) ? 255.f : i)
#define MPTT(r,g,b,rf,gf,bf)	( ((int)(LMTI((float)r*rf)) << 16) | ((int)(LMTI((float)g*gf)) << 8) | ((int)(LMTI((float)b*bf))) )

CMNgManager* g_pNgMng = NULL;


extern int m_defectCode;

CMNgManager::CMNgManager(void)
{
	g_pNgMng = this;
	m_milProc = NULL;

	temp_R = M_NULL;
	temp_G = M_NULL;
	temp_B = M_NULL;

	m_pInspBoardInfo = NULL;
	m_pInspBoardInfo_Foreign = NULL;
	m_pParamArray = NULL;
	m_nParamArraySize = 0;

	CopyPVImage = NULL;
	CopyPVImage_Foreign = NULL;
	//PVSize = new int [3];
	PVSize = g_pMManager->pem_new<int>(true, 3, (PCHAR)__FUNCTION__, __LINE__);
	PVSize_Foreign = g_pMManager->pem_new<int>(true, 3, (PCHAR)__FUNCTION__, __LINE__);
	ImgProcessing[0] = &CMNgManager::Processing_DS;
	ImgProcessing[1] = &CMNgManager::Processing_DSGray;
}


CMNgManager::~CMNgManager(void)
{
	g_pNgMng = NULL;

	if(CopyPVImage != NULL)
	{
		//delete [] CopyPVImage;
		g_pMManager->pem_delete(CopyPVImage, true);
		CopyPVImage = NULL;
	}
	if (CopyPVImage_Foreign != NULL)
	{
		//delete [] CopyPVImage;
		g_pMManager->pem_delete(CopyPVImage_Foreign, true);
		CopyPVImage_Foreign = NULL;
	}
	if(PVSize != NULL)
	{
		//delete [] PVSize;
		g_pMManager->pem_delete(PVSize, true);
		PVSize = NULL;
	}
	if (PVSize_Foreign != NULL)
	{
		//delete [] PVSize;
		g_pMManager->pem_delete(PVSize_Foreign, true);
		PVSize_Foreign = NULL;
	}
}

int CMNgManager::Init(/*Im::PIL_ID* milApp, Im::PIL_ID* milSys*/)
{
	int ret = TRUE;

// 	m_milApp = milApp;	// mil application
// 	m_milSys = milSys;	// mil system
// 
// 
// 	if(m_milProc == NULL)
// 	{
// 		//m_milProc = new CProcMil();
// 		m_milProc = g_pMManager->pem_new<CProcMil>(false, 0, (PCHAR)__FUNCTION__, __LINE__, true);
// 	}
// 
// 	m_milProc->InitMil(m_milApp, m_milSys,g_pMPTI->isUseImagePilLib());
// 	m_milProc->SetResol(m_resolX, m_resolY, m_fovWidth);	

	return ret;
}

void CMNgManager::SetPath(CString path)
{
	m_ngPath = path;
}

void CMNgManager::SetZmapMargin(int margin)
{
	m_roiMargin = margin;
}

int CMNgManager::Exit()
{
	int ret = TRUE;
// 
// 	if(m_milProc != NULL)
// 	{
// 		//JACKY 2013/09/27
// 		m_milProc->FreeMil();
// 		
// 		//delete m_milProc;
// 		g_pMManager->pem_delete(m_milProc, false);
// 		m_milProc = NULL;
// 	}


	if(temp_R != M_NULL)
	{	
		MbufFree(temp_R);
		temp_R = M_NULL;
	}
	if(temp_G != M_NULL)
	{	
		MbufFree(temp_G);
		temp_G = M_NULL;
	}
	if(temp_B != M_NULL)
	{	
		MbufFree(temp_B);
		temp_B = M_NULL;
	}

	return ret;
}

void CMNgManager::SetResolution(int fovWidth, int fovLength, double resolX, double resolY)
{
	m_fovWidth = fovWidth;
	m_fovLength = fovLength;
	m_resolX = resolX;
	m_resolY = resolY;
}

void CMNgManager::SetPartParam(InspPartInfo* boardInfo, InspPartParam *paramArray, int paramArraySize)
{
	m_pInspBoardInfo = boardInfo;
	m_nParamArraySize = paramArraySize;
	m_pParamArray = paramArray;
}

void CMNgManager::SetPartParam(InspPartInfo* boardInfo)
{
	m_pInspBoardInfo = boardInfo;
}
void CMNgManager::SetPartParam_Foreign(InspPartInfo* boardInfo)
{
	m_pInspBoardInfo_Foreign = boardInfo;
}

void CMNgManager::SetInspItemInfo(int* inspItemCnts,	int** inspItemID, int** inspWndOrder)
{
	m_inspItemCnts = inspItemCnts;
	m_inspItemID = inspItemID;
}

void CMNgManager::SetInspGroupInfo(int* groupIndexCnts, int** groupIndex, int* groupID, int* groupWndCnts, int** groupWndID)
{
	m_groupIndexCnts = groupIndexCnts;
	m_groupIndex = groupIndex;

	m_groupID = groupID;
	m_groupWndCnts = groupWndCnts;
	m_groupWndID = groupWndID;
}


void CMNgManager::SetNgParam(NgParam param)
{
	m_ngParam = param;
}

BOOL CMNgManager::IsZmapUse(int index)
{
	BOOL ret = FALSE;

	if(m_pParamArray == NULL || index >= m_nParamArraySize || m_nParamArraySize < 1)
		return ret;

	return TRUE;

// 	if(index < 0)
// 	{
// 		for(int i = 0; i < m_nParamArraySize; i++)
// 		{
// 			if(m_pParamArray[i].useZmap == TRUE)
// 			{
// 				ret = TRUE;
// 				break;
// 			}
// 		}
// 	}
// 	else
// 	{
// 		if(m_pParamArray[index].useZmap == TRUE)
// 			ret = TRUE;
// 	}
// 	return ret;	
}

// SHKang 2017/03/13 : return 1일경우 성공
// 3d image 를 shared memory 방식으로 저장한다.
// retColorImg 가 NULL 이 아니면 B,G,R 값을 채운다.
// bSavePtt == false 이면 colorImg 만 채운다.
// SHKang 2017/07/20 : retColorImg 는 widthStep 반영된다.
// SHKang 2018/03/02 : UCHAR *unAngleColor 추가 - 정리는 나중에 하자.
int CMNgManager::SaveInspPartImage_SharedMemory(CString fullPath, UCHAR* retColorImg, UCHAR *ucAngleColor, int nWidthStep, bool bSavePtt)
	{
	if(!m_pInspBoardInfo)
		return -1;

	UCHAR* imgAngleR =  m_pInspBoardInfo->partImgColorBuf.img_R;
	UCHAR* imgAngleG =  m_pInspBoardInfo->partImgColorBuf.img_G;
	UCHAR* imgAngleB =  m_pInspBoardInfo->partImgColorBuf.img_B;

	UCHAR* imgR = m_pInspBoardInfo->partImgBuf.imgTop_R;
	UCHAR* imgG = m_pInspBoardInfo->partImgBuf.imgTop_G;
	UCHAR* imgB = m_pInspBoardInfo->partImgBuf.imgTop_B;

	int roiSizeX = m_pInspBoardInfo->partImgBuf.nImageSizeX;
	int roiSizeY = m_pInspBoardInfo->partImgBuf.nImageSizeY;

	if(bSavePtt==false)
	{
		if(ucAngleColor == nullptr)
		{
			#pragma omp parallel for
			for (int y=0 ; y<roiSizeY ; y++)
			{
				for (int x=0 ; x<roiSizeX ; x++)
				{
					int nIdx = (y * roiSizeX) + x;

					UCHAR dataR = LMTI((float)imgR[nIdx]*m_ngParam.redFator);
					UCHAR dataG = LMTI((float)imgG[nIdx]*m_ngParam.greenFator);
					UCHAR dataB = LMTI((float)imgB[nIdx]*m_ngParam.blueFator);

					// jpg 를 위한 24 bit
					int nIndex = (y * nWidthStep) + x;	// SHKang 2017/07/20

					int idxRoiDst = nIndex*3;
					retColorImg[idxRoiDst] = dataB;
					retColorImg[idxRoiDst+1] = dataG;
					retColorImg[idxRoiDst+2] = dataR;
				}
			}
		}
		else
		{
			
			#pragma omp parallel for
			for (int y=0 ; y<roiSizeY ; y++)
			{
				for (int x=0 ; x<roiSizeX ; x++)
				{
					int nIdx = (y * roiSizeX) + x;

					UCHAR dataR = LMTI((float)imgR[nIdx]*m_ngParam.redFator);
					UCHAR dataG = LMTI((float)imgG[nIdx]*m_ngParam.greenFator);
					UCHAR dataB = LMTI((float)imgB[nIdx]*m_ngParam.blueFator);

					// jpg 를 위한 24 bit

					int nIndex = (y * nWidthStep) + x;	// SHKang 2017/07/20

					int idxRoiDst = nIndex*3;
					retColorImg[idxRoiDst] = dataB;
					retColorImg[idxRoiDst+1] = dataG;
					retColorImg[idxRoiDst+2] = dataR;

					ucAngleColor[idxRoiDst] = imgAngleB[nIdx];
					ucAngleColor[idxRoiDst+1] = imgAngleG[nIdx];
					ucAngleColor[idxRoiDst+2] = imgAngleR[nIdx];
				}
			}
		}

		return 0;
	}

	CString tmpPath;
	tmpPath = fullPath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".ptt")) == FALSE)
	{
		fullPath += _T(".ptt");
	}

	int roiArea = (int)(roiSizeX * roiSizeY);
	RECT roi = {0, 0, m_pInspBoardInfo->partImgBuf.nImageSizeX, m_pInspBoardInfo->partImgBuf.nImageSizeY};

	int sizeX = m_pInspBoardInfo->partZmapData.zmapSizeX;
	int sizeY = m_pInspBoardInfo->partZmapData.zmapSizeY;

	QWORD ImgSz = sizeof(HEADER_PTT) + sizeof(short) * sizeX * sizeY + sizeof(unsigned int) * sizeX * sizeY;

	CSharedMemory shared;
	shared.AllocFile(fullPath, ImgSz, _T(""));
	shared.MapToWrite(0, ImgSz);
	BYTE * Buffer = shared.Byte();
	int nWritten=0;
	int nWritten2dBody = ImgSz -  sizeof(unsigned int) * sizeX * sizeY;  // 2d Written body index 는 다음과 같다.

	//save zmap (ptt)
	HEADER_PTT hd;

	hd.uiNumRow = sizeX;
	hd.uiNumCol = sizeY;
	hd.fNumRow = (float)m_pInspBoardInfo->partZmapData.zmapSizeX;
	hd.fNumCol = (float)m_pInspBoardInfo->partZmapData.zmapSizeY;
	hd.pixelResX = (float)_mm2micron(m_resolX);
	hd.pixelResY = (float)_mm2micron(m_resolY);
	hd.zResolX = (float)_mm2micron(m_resolX);
	hd.zResolY = (float)_mm2micron(m_resolY);
	// 3d header 저장
	memcpy(&Buffer[nWritten], &hd.uiNumCol, sizeof(hd.uiNumCol));			nWritten += sizeof(hd.uiNumCol);
	memcpy(&Buffer[nWritten], &hd.uiNumRow, sizeof(hd.uiNumRow));		nWritten += sizeof(hd.uiNumRow);
	memcpy(&Buffer[nWritten], &hd.zResolX, sizeof(hd.zResolX));			nWritten += sizeof(hd.zResolX);
	memcpy(&Buffer[nWritten], &hd.zResolY, sizeof(hd.zResolY));			nWritten += sizeof(hd.zResolY);
	memcpy(&Buffer[nWritten], &hd.sizeBit, sizeof(hd.sizeBit));			nWritten += sizeof(hd.sizeBit);
	memcpy(&Buffer[nWritten], &hd.gabX, sizeof(hd.gabX));		nWritten += sizeof(hd.gabX);
	memcpy(&Buffer[nWritten], &hd.gabY, sizeof(hd.gabY));		nWritten += sizeof(hd.gabY);
	memcpy(&Buffer[nWritten], &hd.sizeUnit, sizeof(hd.sizeUnit));		nWritten += sizeof(hd.sizeUnit);

	// body 저장.
	for(int y=0 ; y<sizeY ; y++)
	{
		for(int x=0 ; x<sizeX ; x++)
		{
			short sHeight = (short)m_pInspBoardInfo->partZmapData.data[x+(y*sizeX)];
			memcpy(&Buffer[nWritten], &sHeight, sizeof(sHeight));		nWritten += sizeof(sHeight);
		}
	}

	// 2d header 저장
	memcpy(&Buffer[nWritten], &hd.fNumCol, sizeof(hd.uiNumCol));			nWritten += sizeof(hd.uiNumCol);
	memcpy(&Buffer[nWritten], &hd.fNumRow, sizeof(hd.uiNumRow));		nWritten += sizeof(hd.uiNumRow);
	memcpy(&Buffer[nWritten], &hd.targetStartX, sizeof(hd.targetStartX));			nWritten += sizeof(hd.targetStartX);
	memcpy(&Buffer[nWritten], &hd.targetStartY, sizeof(hd.targetStartY));			nWritten += sizeof(hd.targetStartY);
	memcpy(&Buffer[nWritten], &hd.targetShiftX, sizeof(hd.targetShiftX));			nWritten += sizeof(hd.targetShiftX);
	memcpy(&Buffer[nWritten], &hd.targetShiftY, sizeof(hd.targetShiftY));		nWritten += sizeof(hd.targetShiftY);
	memcpy(&Buffer[nWritten], &hd.targetWidth, sizeof(hd.targetWidth));		nWritten += sizeof(hd.targetWidth);
	memcpy(&Buffer[nWritten], &hd.targetHeight, sizeof(hd.targetHeight));		nWritten += sizeof(hd.targetHeight);
	memcpy(&Buffer[nWritten], &hd.pixelResX, sizeof(hd.pixelResX));		nWritten += sizeof(hd.pixelResX);
	memcpy(&Buffer[nWritten], &hd.pixelResY, sizeof(hd.pixelResY));		nWritten += sizeof(hd.pixelResY);
	memcpy(&Buffer[nWritten], &hd.depthBit, sizeof(hd.depthBit));		nWritten += sizeof(hd.depthBit);
	if(retColorImg == NULL)
	{
		int nIdx = 0;
		for (int y=0 ; y<roiSizeY ; y++)
		{
			for (int x=0 ; x<roiSizeX ; x++)
			{
				nIdx = (y * roiSizeX) + x;
				unsigned int unRGB = (unsigned int)(MPTT(imgR[nIdx], imgG[nIdx], imgB[nIdx], m_ngParam.redFator, m_ngParam.greenFator, m_ngParam.blueFator));
				memcpy(&Buffer[nWritten], &unRGB, sizeof(unsigned int));		nWritten += sizeof(unsigned int);

			}
		}
	}
	else
	{
		UCHAR dataR = 0;
		UCHAR dataG = 0;
		UCHAR dataB = 0;

		/*if(ucAngleColor != nullptr)// */  //ucAngleColor값이 없어서 임시
		{
			int idxRoiDst = 0;
			int nIdx = 0;
			for (int y=0 ; y<roiSizeY ; y++)
			{
				for (int x=0 ; x<roiSizeX ; x++)
				{
					nIdx = (y * roiSizeX) + x;

					dataR = LMTI((float)imgR[nIdx]*m_ngParam.redFator);
					dataG = LMTI((float)imgG[nIdx]*m_ngParam.greenFator);
					dataB = LMTI((float)imgB[nIdx]*m_ngParam.blueFator);

					unsigned int unRGB =  ((UINT)dataR << 16) | ((UINT)dataG << 8) | dataB;
					memcpy(&Buffer[nWritten], &unRGB, sizeof(unsigned int));		nWritten += sizeof(unsigned int);

					// jpg 를 위한 24 bit
					nIdx = (y * nWidthStep) + x;
					idxRoiDst = nIdx*3;
					retColorImg[idxRoiDst] = dataB;
					retColorImg[idxRoiDst+1] = dataG;
					retColorImg[idxRoiDst+2] = dataR;
				}
			}
		}
// 		else
// 		{
// 			int idxRoiDst = 0;
// 			int nIdx = 0;
// 			for (int y=0 ; y<roiSizeY ; y++)
// 			{
// 				for (int x=0 ; x<roiSizeX ; x++)
// 				{
// 					nIdx = (y * roiSizeX) + x;
// 
// 					dataR = LMTI((float)imgR[nIdx]*m_ngParam.redFator);
// 					dataG = LMTI((float)imgG[nIdx]*m_ngParam.greenFator);
// 					dataB = LMTI((float)imgB[nIdx]*m_ngParam.blueFator);
// 
// 					unsigned int unRGB =  ((UINT)dataR << 16) | ((UINT)dataG << 8) | dataB;
// 					memcpy(&Buffer[nWritten], &unRGB, sizeof(unsigned int));		nWritten += sizeof(unsigned int);
// 
// 					// jpg 를 위한 24 bit
// 					int nIndex = (y * nWidthStep) + x;
// 					idxRoiDst = nIndex*3;
// 					retColorImg[idxRoiDst] = dataB;
// 					retColorImg[idxRoiDst+1] = dataG;
// 					retColorImg[idxRoiDst+2] = dataR;
// 
// 					ucAngleColor[idxRoiDst] = imgAngleB[nIdx];
// 					ucAngleColor[idxRoiDst+1] = imgAngleG[nIdx];
// 					ucAngleColor[idxRoiDst+2] = imgAngleR[nIdx];
// 				}
// 			}
// 		}

	}
	// 
	// 	if(retColorImg != NULL)
	// 		alpf_make_img24n32f(roi, roiSizeX, imgR, imgG, imgB, m_ngParam.redFator, m_ngParam.greenFator, m_ngParam.blueFator, roiSizeX, NULL, retColorImg);

	return 1;
}
int CMNgManager::SaveInspPartImage_SharedMemory_Multi(CString fullPath, UCHAR* retColorImg, UCHAR *ucAngleColor, int nWidthStep, bool bSavePtt, RoiColorBuf partImgColorBuf, InspRoiImgBuf partImgBuf,ZmapData partZmapData)
{
	//if (partImgColorBuf == NULL || partImgBuf == NULL)
	//	return -1;

	UCHAR* imgAngleR = partImgColorBuf.img_R;
	UCHAR* imgAngleG = partImgColorBuf.img_G;
	UCHAR* imgAngleB = partImgColorBuf.img_B;

	UCHAR* imgR = partImgBuf.imgTop_R;
	UCHAR* imgG = partImgBuf.imgTop_G;
	UCHAR* imgB = partImgBuf.imgTop_B;

	int roiSizeX = partImgBuf.nImageSizeX;
	int roiSizeY = partImgBuf.nImageSizeY;

	if (bSavePtt == false)
	{
		if (ucAngleColor == nullptr)
		{
#pragma omp parallel for
			for (int y = 0; y < roiSizeY; y++)
			{
				for (int x = 0; x < roiSizeX; x++)
				{
					int nIdx = (y * roiSizeX) + x;

					UCHAR dataR = LMTI((float)imgR[nIdx] * m_ngParam.redFator);
					UCHAR dataG = LMTI((float)imgG[nIdx] * m_ngParam.greenFator);
					UCHAR dataB = LMTI((float)imgB[nIdx] * m_ngParam.blueFator);

					// jpg 를 위한 24 bit
					int nIndex = (y * nWidthStep) + x;	// SHKang 2017/07/20

					int idxRoiDst = nIndex * 3;
					retColorImg[idxRoiDst] = dataB;
					retColorImg[idxRoiDst + 1] = dataG;
					retColorImg[idxRoiDst + 2] = dataR;
				}
			}
		}
		else
		{

#pragma omp parallel for
			for (int y = 0; y < roiSizeY; y++)
			{
				for (int x = 0; x < roiSizeX; x++)
				{
					int nIdx = (y * roiSizeX) + x;

					UCHAR dataR = LMTI((float)imgR[nIdx] * m_ngParam.redFator);
					UCHAR dataG = LMTI((float)imgG[nIdx] * m_ngParam.greenFator);
					UCHAR dataB = LMTI((float)imgB[nIdx] * m_ngParam.blueFator);

					// jpg 를 위한 24 bit

					int nIndex = (y * nWidthStep) + x;	// SHKang 2017/07/20

					int idxRoiDst = nIndex * 3;
					retColorImg[idxRoiDst] = dataB;
					retColorImg[idxRoiDst + 1] = dataG;
					retColorImg[idxRoiDst + 2] = dataR;

					ucAngleColor[idxRoiDst] = imgAngleB[nIdx];
					ucAngleColor[idxRoiDst + 1] = imgAngleG[nIdx];
					ucAngleColor[idxRoiDst + 2] = imgAngleR[nIdx];
				}
			}
		}

		return 0;
	}

	CString tmpPath;
	tmpPath = fullPath;
	tmpPath.MakeLower();
	if ((tmpPath.Right(4) == _T(".ptt")) == FALSE)
	{
		fullPath += _T(".ptt");
	}

	int roiArea = (int)(roiSizeX * roiSizeY);
	RECT roi = { 0, 0, partImgBuf.nImageSizeX, partImgBuf.nImageSizeY };

	int sizeX = partZmapData.zmapSizeX;
	int sizeY = partZmapData.zmapSizeY;

	QWORD ImgSz = sizeof(HEADER_PTT) + sizeof(short) * sizeX * sizeY + sizeof(unsigned int) * sizeX * sizeY;

	CSharedMemory shared;
	shared.AllocFile(fullPath, ImgSz, _T(""));
	shared.MapToWrite(0, ImgSz);
	BYTE * Buffer = shared.Byte();
	int nWritten = 0;
	int nWritten2dBody = ImgSz - sizeof(unsigned int) * sizeX * sizeY;  // 2d Written body index 는 다음과 같다.

	//save zmap (ptt)
	HEADER_PTT hd;

	hd.uiNumRow = sizeX;
	hd.uiNumCol = sizeY;
	hd.fNumRow = (float)partZmapData.zmapSizeX;
	hd.fNumCol = (float)partZmapData.zmapSizeY;
	hd.pixelResX = (float)_mm2micron(m_resolX);
	hd.pixelResY = (float)_mm2micron(m_resolY);
	hd.zResolX = (float)_mm2micron(m_resolX);
	hd.zResolY = (float)_mm2micron(m_resolY);
	// 3d header 저장
	memcpy(&Buffer[nWritten], &hd.uiNumCol, sizeof(hd.uiNumCol));			nWritten += sizeof(hd.uiNumCol);
	memcpy(&Buffer[nWritten], &hd.uiNumRow, sizeof(hd.uiNumRow));		nWritten += sizeof(hd.uiNumRow);
	memcpy(&Buffer[nWritten], &hd.zResolX, sizeof(hd.zResolX));			nWritten += sizeof(hd.zResolX);
	memcpy(&Buffer[nWritten], &hd.zResolY, sizeof(hd.zResolY));			nWritten += sizeof(hd.zResolY);
	memcpy(&Buffer[nWritten], &hd.sizeBit, sizeof(hd.sizeBit));			nWritten += sizeof(hd.sizeBit);
	memcpy(&Buffer[nWritten], &hd.gabX, sizeof(hd.gabX));		nWritten += sizeof(hd.gabX);
	memcpy(&Buffer[nWritten], &hd.gabY, sizeof(hd.gabY));		nWritten += sizeof(hd.gabY);
	memcpy(&Buffer[nWritten], &hd.sizeUnit, sizeof(hd.sizeUnit));		nWritten += sizeof(hd.sizeUnit);

	// body 저장.
	for (int y = 0; y < sizeY; y++)
	{
		for (int x = 0; x < sizeX; x++)
		{
			short sHeight = (short)partZmapData.data[x + (y*sizeX)];
			memcpy(&Buffer[nWritten], &sHeight, sizeof(sHeight));		nWritten += sizeof(sHeight);
		}
	}

	// 2d header 저장
	memcpy(&Buffer[nWritten], &hd.fNumCol, sizeof(hd.uiNumCol));			nWritten += sizeof(hd.uiNumCol);
	memcpy(&Buffer[nWritten], &hd.fNumRow, sizeof(hd.uiNumRow));		nWritten += sizeof(hd.uiNumRow);
	memcpy(&Buffer[nWritten], &hd.targetStartX, sizeof(hd.targetStartX));			nWritten += sizeof(hd.targetStartX);
	memcpy(&Buffer[nWritten], &hd.targetStartY, sizeof(hd.targetStartY));			nWritten += sizeof(hd.targetStartY);
	memcpy(&Buffer[nWritten], &hd.targetShiftX, sizeof(hd.targetShiftX));			nWritten += sizeof(hd.targetShiftX);
	memcpy(&Buffer[nWritten], &hd.targetShiftY, sizeof(hd.targetShiftY));		nWritten += sizeof(hd.targetShiftY);
	memcpy(&Buffer[nWritten], &hd.targetWidth, sizeof(hd.targetWidth));		nWritten += sizeof(hd.targetWidth);
	memcpy(&Buffer[nWritten], &hd.targetHeight, sizeof(hd.targetHeight));		nWritten += sizeof(hd.targetHeight);
	memcpy(&Buffer[nWritten], &hd.pixelResX, sizeof(hd.pixelResX));		nWritten += sizeof(hd.pixelResX);
	memcpy(&Buffer[nWritten], &hd.pixelResY, sizeof(hd.pixelResY));		nWritten += sizeof(hd.pixelResY);
	memcpy(&Buffer[nWritten], &hd.depthBit, sizeof(hd.depthBit));		nWritten += sizeof(hd.depthBit);
	if (retColorImg == NULL)
	{
		int nIdx = 0;
		for (int y = 0; y < roiSizeY; y++)
		{
			for (int x = 0; x < roiSizeX; x++)
			{
				nIdx = (y * roiSizeX) + x;
				unsigned int unRGB = (unsigned int)(MPTT(imgR[nIdx], imgG[nIdx], imgB[nIdx], m_ngParam.redFator, m_ngParam.greenFator, m_ngParam.blueFator));
				memcpy(&Buffer[nWritten], &unRGB, sizeof(unsigned int));		nWritten += sizeof(unsigned int);

			}
		}
	}
	else
	{
		UCHAR dataR = 0;
		UCHAR dataG = 0;
		UCHAR dataB = 0;

		/*if(ucAngleColor != nullptr)// */  //ucAngleColor값이 없어서 임시
		{
			int idxRoiDst = 0;
			int nIdx = 0;
			for (int y = 0; y < roiSizeY; y++)
			{
				for (int x = 0; x < roiSizeX; x++)
				{
					nIdx = (y * roiSizeX) + x;

					dataR = LMTI((float)imgR[nIdx] * m_ngParam.redFator);
					dataG = LMTI((float)imgG[nIdx] * m_ngParam.greenFator);
					dataB = LMTI((float)imgB[nIdx] * m_ngParam.blueFator);

					unsigned int unRGB = ((UINT)dataR << 16) | ((UINT)dataG << 8) | dataB;
					memcpy(&Buffer[nWritten], &unRGB, sizeof(unsigned int));		nWritten += sizeof(unsigned int);

					// jpg 를 위한 24 bit
					nIdx = (y * nWidthStep) + x;
					idxRoiDst = nIdx * 3;
					retColorImg[idxRoiDst] = dataB;
					retColorImg[idxRoiDst + 1] = dataG;
					retColorImg[idxRoiDst + 2] = dataR;
				}
			}
		}
	}
	return 1;

}
int CMNgManager::SaveInspPartImage_AngleColor(UCHAR *ucAngleColor, int nWidthStep)
{
	if (!m_pInspBoardInfo || ucAngleColor == nullptr)
		return -1;

	UCHAR* imgAngleR = m_pInspBoardInfo->partImgColorBuf.img_R;
	UCHAR* imgAngleG = m_pInspBoardInfo->partImgColorBuf.img_G;
	UCHAR* imgAngleB = m_pInspBoardInfo->partImgColorBuf.img_B;
	//UCHAR* imgAngleR = m_pInspBoardInfo->partImgBuf.imgTop_R;
	//UCHAR* imgAngleG = m_pInspBoardInfo->partImgBuf.imgTop_G;
	//UCHAR* imgAngleB = m_pInspBoardInfo->partImgBuf.imgTop_B;
	
	int roiSizeX = m_pInspBoardInfo->partImgBuf.nImageSizeX;
	int roiSizeY = m_pInspBoardInfo->partImgBuf.nImageSizeY;

#pragma omp parallel for
	for (int y = 0; y < roiSizeY; y++)
	{
		for (int x = 0; x < roiSizeX; x++)
		{
			int nIdx = (y * roiSizeX) + x;
			int nIndex = (y * nWidthStep) + x;	// SHKang 2017/07/20
			int idxRoiDst = nIndex * 3;
			ucAngleColor[idxRoiDst] = imgAngleB[nIdx];
			ucAngleColor[idxRoiDst + 1] = imgAngleG[nIdx];
			ucAngleColor[idxRoiDst + 2] = imgAngleR[nIdx];
		}
	}
	return 1;
}
// void CMNgManager::SaveNgImage(void* srcMilImg, CRect roi, CString fullPath)
// {
// 	if(m_fovWidth <= 0 || m_fovLength <= 0 || roi.top < 0 || roi.bottom < 0 || roi.left < 0 || roi.right < 0 || roi.Width() <= 0 || roi.Height() <= 0)
// 		return;
// 
// 	int area = roi.Width() * roi.Height();
// 	CPoint ct = roi.CenterPoint();
// 
// 	//save image
// 	m_milProc->SaveClipImg_void(srcMilImg, 1, (int)ct.x, (int)ct.y, roi.Width(), roi.Height(), fullPath + ProcTypeTIF);
// }

void CMNgManager::ProcSaveDisplaynForeignNg(CString dispPath, CString partName, AForeignResult vforeign, int nMesIndex, CString UserPath, CString Filename, int extension, int nDivide, int nGapX, int nGapY, float* pfOff3D)
{
	CString strDispFileName = _T("");
	CString strDispFullPath = _T("");
	strDispFullPath.Format(_T("%s\\%s"), dispPath, partName);
	CreateDir(dispPath);
	int nImgW = m_pInspBoardInfo_Foreign->zmapData.zmapSizeX / nDivide - 1;
	int nImgH = m_pInspBoardInfo_Foreign->zmapData.zmapSizeY / nDivide - 1;
	if (pfOff3D)
	{
		if (m_pInspBoardInfo == NULL)
			return;
		nImgW = m_pInspBoardInfo->zmapData.zmapSizeX / nDivide - 1;
		nImgH = m_pInspBoardInfo->zmapData.zmapSizeY / nDivide - 1;
	}

	if (nGapX < 0) nGapX = 0;
	if (nGapY < 0) nGapY = 0;
	if (nGapX > 100) nGapX = 100;
	if (nGapY > 100) nGapY = 100;
	int nL = vforeign.m_rcRect.left - nGapX;
	int nR = vforeign.m_rcRect.right + nGapX;
	int nT = vforeign.m_rcRect.top - nGapY;
	int nB = vforeign.m_rcRect.bottom + nGapY;
	if (nL < 0) nL = 0;
	if (nR < 0) nR = 0;
	if (nT < 0) nT = 0;
	if (nB < 0) nB = 0;
	if (nL > nImgW) nL = nImgW;
	if (nR > nImgW) nR = nImgW;
	if (nT > nImgH) nT = nImgH;
	if (nB > nImgH) nB = nImgH;
	if (nR <= nL || nB <= nT) return;
	int nW = nR - nL;
	int nH = nB - nT;
	RECT rtForeignRoi = {nL, nT, nR, nB};
	UCHAR* pUcColorImg = g_pMManager->pem_new<UCHAR>(true, nW * nH * 3, (PCHAR)__FUNCTION__, __LINE__);
//	SaveDispImage_Foreign_new(strDispFullPath, TRUE, pUcColorImg, nGapX, nGapY, vforeign, nDivide, pfOff3D);
	CopyPVImage_Foreign = SavePVImage_Foreign(pUcColorImg, nW, nH);

#if _DEBUG
	cv::Mat imgSrc(nH, nW, CV_8UC3, pUcColorImg);
	cv::Mat imgDst(nH, nW, CV_8UC3, CopyPVImage_Foreign);
#endif
	if(nMesIndex == 1)
	{
		CString strpath =_T("");
		CString Extension = _T("");
		strpath.Format(_T("%s\\%s"), UserPath, Filename);
		CreateDir(UserPath);
		Extension = _T("tif");
		if(extension == 0)		Extension = _T(".jpg");
		else if(extension == 1)	Extension = _T(".png");
		m_milProc->SaveImage(pUcColorImg, nW, nH, 3, strpath + Extension);
	}
	Delete_1DArray(&pUcColorImg);
}

// SHKang 2017/05/25 : PTT, POT, PST 를 저장하고 JPG 형(width step 이 고려된 IC_BGR)의 메모리(UCHAR * CopyPVImage)를 채운다.  
void CMNgManager::ProcSaveDisplaynNg(CString dispPath, CString moduleName, CString partName, int *pArrUseSideImg, CString LinkagePath, bool bLinkage, CString RefID, int nAngleColor, int exportSaveOption)
{
	//comon
	CRect roi_3D = GetRectRoi();
	int roiSizeX = (int)(roi_3D.right - roi_3D.left);
	int roiSizeY = (int)(roi_3D.bottom - roi_3D.top);
	if(m_pInspBoardInfo->partImgBuf.nImageSizeX != roiSizeX)
	{
		roi_3D.right = roi_3D.left + m_pInspBoardInfo->partImgBuf.nImageSizeX;
		roiSizeX = m_pInspBoardInfo->partImgBuf.nImageSizeX;
	}
	if(m_pInspBoardInfo->partImgBuf.nImageSizeY != roiSizeY)
	{
		roi_3D.bottom = roi_3D.top + m_pInspBoardInfo->partImgBuf.nImageSizeY;
		roiSizeY = m_pInspBoardInfo->partImgBuf.nImageSizeY;
	}
	int roiArea = (int)(roiSizeX * roiSizeY);

	//display
	CString dispFileName = MakeFilename_Disp(moduleName, partName);
	CString dispFullPath = _T("");
	CString dispFullPathRGB = _T("");
	dispFullPath.Format(_T("%s\\%s"), dispPath, dispFileName);
	dispFullPathRGB.Format(_T("%s\\%s_RGB"), dispPath, dispFileName);
	CreateDir(dispPath);
	//BOOL zmapFlag = IsZmapUse();	// PTT 저장에서 IsZmapUse 와 상관없이 m_pInspBoardInfo 가 NULL 이아니면 저장 Part Window Enable 이 하나도 없으면 IsZmapUse 가 False 리턴하여 PTT 저장 안하게됨.
	BOOL zmapFlag = TRUE;
	// SHKang 2017/07/20 : widthStep 반영
	int nWidthStep = roiSizeX;
	/*int nSizeX = roiSizeX % 4;
	if (nSizeX != 0) 	nWidthStep += 4 - nSizeX;*/
	nWidthStep = g_pMPTI->nCalcWidthStep(false, roiSizeX);

	int nImgSize = nWidthStep * roiSizeY;
	UCHAR* colorImg = g_pMManager->pem_new<UCHAR>(true, nImgSize * 3, (PCHAR)__FUNCTION__, __LINE__);
	// PTT 이미지 파일을 저장하고 JPEG Color 배열(BGR배열)의 buffer 를 받아온다. 
// 	if(g_pMPTI->IsMachineTypeUV())
// 	{
// 		SaveDispImage(LinkagePath, zmapFlag, colorImg, nWidthStep, exportSaveOption, TRUE);
// 		if (bLinkage) //SHS 2017/03/27
// 		{
// 			CString sLinkagedispFileName = _T("");	//SHS 2017/03/27
// 			CString sLinkageFilePath = _T("");	//SHS 2017/03/27
// 			sLinkagedispFileName = MakeFilename_Disp(moduleName, RefID);
// 			sLinkageFilePath.Format(_T("%s\\%s"), LinkagePath, sLinkagedispFileName);
// 			SaveDispImage(sLinkageFilePath, zmapFlag, colorImg, nWidthStep, exportSaveOption, TRUE);
// 		}
// 		SavePutImage(dispFullPath, roiArea);	// .put (uv image)
// 	}
// 	else
	{
		SaveDispImage(dispFullPath, zmapFlag, colorImg, nWidthStep, exportSaveOption);
		if (bLinkage) //SHS 2017/03/27
		{
			CString sLinkagedispFileName = _T("");	//SHS 2017/03/27
			CString sLinkageFilePath = _T("");	//SHS 2017/03/27
			sLinkagedispFileName = MakeFilename_Disp(moduleName, RefID);
			sLinkageFilePath.Format(_T("%s\\%s"), LinkagePath, sLinkagedispFileName);
			SaveDispImage(sLinkageFilePath, zmapFlag, colorImg, nWidthStep, exportSaveOption);
		}
	}	
	if (nAngleColor == 1)
	{
		int widthStep = 0;
		int roiArea = GetROISize(roiSizeX, roiSizeY, widthStep);
		SaveColorMap2UseImage(dispFullPath, widthStep);
	}
	// POT 를 저장한다.
	SavePotImage(dispFullPath, roiArea);
	// PST	// Side Camera R.G.B Image
	int *pUseSideImg = NULL;
	alpa_make_array_1d(&pUseSideImg, 4);					// 4 개의 배열을 만들고 0 으로 초기화한다.
	CopyMemory(pUseSideImg, pArrUseSideImg, sizeof(int)*4); // 메모리 카피한다.

	for(int  i = 0; i < 4; i++)
	{
		if(pUseSideImg[i] == 1)
			SavePstImage(dispFullPath, roiArea, i);
	}
	// UCHAR* rstColorImg = NULL;	// SHKang 2017/05/25 : 별도로 변수 선언해서 사용하는 이유가 없으므로 변수, 포인터 맞추는것 제거... rstColorImg = colorImg 이도록 아래코드 수정한다.
	double wndAngle = m_pInspBoardInfo->angle;
	BOOL isAnyAngle = IsAnyAngle(wndAngle);
	int w = roiSizeX;
	int h = roiSizeY;
	int marginX = 0;
	int marginY = 0;

	// rstColorImg = colorImg;
	marginX = 0;
	marginY = 0;
	CopyPVImage = SavePVImage(colorImg, w, h);	// width step 을 고려한 메모리에 저장하기... open cv 를 이용한 저장 구문때문으로 보임.
	if(pUseSideImg != NULL)
	{
		g_pMManager->pem_delete(pUseSideImg, true);
		pUseSideImg = NULL;
	}
	g_pMManager->pem_delete(colorImg, true);
	colorImg = NULL;
}

//LDH, JPG AuAa¿e image buf copy
UCHAR* CMNgManager::SavePVImage(void* ColorImage, int width, int height)
{
	Delete_1DArray(&CopyPVImage);
	int widthStep = g_pMPTI->nCalcWidthStep(true, width);
	//int nSizeX = width % 4;
	/*if (nSizeX != 0)
		widthStep += 4 - nSizeX;*/
	//CopyPVImage = new UCHAR[widthStep*height*3];
	CopyPVImage = g_pMManager->pem_new<UCHAR>(true, widthStep*height * 3, (PCHAR)__FUNCTION__, __LINE__);
	IppStatus sts;
	IppiSize iSize = {width, height};
	sts =  ippiCopy_8u_C3R((UCHAR*)ColorImage, width * 3, CopyPVImage, widthStep * 3, iSize);
	PVSize[0] = widthStep;
	PVSize[1] = height;
	PVSize[2] = width;
	return CopyPVImage;
}
UCHAR* CMNgManager::SavePVImage_Foreign(void* ColorImage, int width, int height)
{
	Delete_1DArray(&CopyPVImage_Foreign);
	int widthStep = g_pMPTI->nCalcWidthStep(true, width);
	//int nSizeX = width % 4;
	/*if (nSizeX != 0)
		widthStep += 4 - nSizeX;*/
	//CopyPVImage = new UCHAR[widthStep*height*3];
	CopyPVImage_Foreign = g_pMManager->pem_new<UCHAR>(true, widthStep*height * 3, (PCHAR)__FUNCTION__, __LINE__);
	IppStatus sts;
	IppiSize iSize = { width, height };
	sts = ippiCopy_8u_C3R((UCHAR*)ColorImage, width * 3, CopyPVImage_Foreign, widthStep * 3, iSize);
	PVSize_Foreign[0] = widthStep;
	PVSize_Foreign[1] = height;
	PVSize_Foreign[2] = width;

#if _DEBUG
	cv::Mat img(height, widthStep, CV_8UC3, CopyPVImage_Foreign);
#endif

	return CopyPVImage_Foreign;
}

// SHKang 2017/05/26
int CMNgManager::CreateJpgBuffer(UCHAR * srcImage, int srcWidth, int srcHeight, int & dstWdith, int & dstHeight, UCHAR ** dstImage)
{
	if(*dstImage != NULL)
	{
		//delete [] *dstImage;
		g_pMManager->pem_delete(*dstImage, true);
		*dstImage = NULL;
	}

	int widthStep = g_pMPTI->nCalcWidthStep(false, srcWidth);

	//int nSizeX = srcWidth % 4;
	//if (nSizeX != 0)
	//{
	//	widthStep += 4 - nSizeX;
	//}

	//*dstImage = new UCHAR[widthStep*srcHeight*3]; 
	*dstImage = g_pMManager->pem_new<UCHAR>(true, widthStep*srcHeight * 3, (PCHAR)__FUNCTION__, __LINE__);
	
	IppStatus sts;
	IppiSize iSize = {srcWidth, srcHeight};
	sts =  ippiCopy_8u_C3R(srcImage, srcWidth * 3, *dstImage, widthStep * 3, iSize);

	PVSize[0] = dstWdith = widthStep;
	PVSize[1] = dstHeight = srcHeight;
	PVSize[2] = srcWidth;		// ¿ø·¡ AI¹IAo Width

	return 0;
}

UCHAR* CMNgManager::GetPVImage(int* nWidthStep, int* nHeight, int* nWidth)
{
	*nWidthStep = PVSize[0];
	*nHeight = PVSize[1];
	*nWidth = PVSize[2];

	return CopyPVImage;
}
UCHAR* CMNgManager::GetPVImage_Foreign(int* nWidthStep, int* nHeight, int* nWidth)
{
	*nWidthStep = PVSize_Foreign[0];
	*nHeight = PVSize_Foreign[1];
	*nWidth = PVSize_Foreign[2];

#if _DEBUG
	cv::Mat img(PVSize_Foreign[1], PVSize_Foreign[0], CV_8UC3, CopyPVImage_Foreign);
#endif
	return CopyPVImage_Foreign;
}

void CMNgManager::SaveTifImage(UCHAR *uImgBuff, CString strFullPath, int nWidth, int nHeight, int nBand)
{
	//m_milProc->SaveImage(uImgBuff, nWidth, nHeight, nBand, strFullPath + ProcTypeTIF);
}

void CMNgManager::SavePotImage(CString strFullPath, int roiArea)
{
	if(!m_pInspBoardInfo)
		return;

	UCHAR* pImgBB = NULL;
	UCHAR* pImgBR = NULL;
	UCHAR* pImgMB = NULL;
	UCHAR* pImgMR = NULL;
	UCHAR* pImgTW = NULL;

	// SHKang 2017/05/24 : 이미지 buffer 를 만들어서 복사하는 이유가 없어 보임.
// 	pImgBB = new UCHAR[roiArea];
// 	pImgBR = new UCHAR[roiArea];
// 	pImgMB = new UCHAR[roiArea];
// 	pImgMR = new UCHAR[roiArea];
// 	pImgTW = new UCHAR[roiArea];
// 
// 	memcpy_s(pImgBB, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgBottom_B, sizeof(UCHAR) * roiArea);
// 	memcpy_s(pImgBR, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgBottom_R, sizeof(UCHAR) * roiArea);
// 	memcpy_s(pImgMB, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgMiddle_B, sizeof(UCHAR) * roiArea);
// 	memcpy_s(pImgMR, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgMiddle_R, sizeof(UCHAR) * roiArea);
// 	memcpy_s(pImgTW, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgTop_W, sizeof(UCHAR) * roiArea);

 	pImgBB = m_pInspBoardInfo->partImgBuf.imgBottom_B;
 	pImgBR = m_pInspBoardInfo->partImgBuf.imgBottom_R;
 	pImgMB = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
 	pImgMR = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
 	pImgTW = m_pInspBoardInfo->partImgBuf.imgTop_W;

	float* pResX = NULL;
	float* pResY = NULL;

	HEADER_POT hd;

	hd.fNumCol = (float)m_pInspBoardInfo->partImgBuf.nImageSizeX;
	hd.fNumRow = (float)m_pInspBoardInfo->partImgBuf.nImageSizeY;
	hd.fPixelResX = (float)m_resolX;
	hd.fPixelResY = (float)m_resolY;
	hd.fDepthBit = 8;
		
	alpf_save_pot(strFullPath, &hd, pImgBB, pImgBR, pImgMB, pImgMR, pImgTW);

	// Debug // SHKang 2017/05/24 : 필요없는 구문 주석처리
// 	int w = (int)hd.fNumCol;
// 	int h = (int)hd.fNumRow;

	// SHKang 2017/05/24 : 이미지 버퍼를 새로 생성할 필요 없어 보임.
// 	if(pImgBB != NULL)
// 	{
// 		delete [] pImgBB;
// 		pImgBB = NULL;
// 	}
// 	if(pImgBR != NULL)
// 	{
// 		delete [] pImgBR;
// 		pImgBR = NULL;
// 	}
// 	if(pImgMB != NULL)
// 	{
// 		delete [] pImgMB;
// 		pImgMB = NULL;
// 	}
// 	if(pImgMR != NULL)
// 	{
// 		delete [] pImgMR;
// 		pImgMR = NULL;
// 	}
// 	if(pImgTW != NULL)
// 	{
// 		delete [] pImgTW;
// 		pImgTW = NULL;
//  	}
}

void CMNgManager::SavePotImage_Multi(CString strFullPath, int roiArea, InspRoiImgBuf partImgBuf)
{
	//if (!InspBoardInfo)
	//	return;

	UCHAR* pImgBB = NULL;
	UCHAR* pImgBR = NULL;
	UCHAR* pImgMB = NULL;
	UCHAR* pImgMR = NULL;
	UCHAR* pImgTW = NULL;


	pImgBB = partImgBuf.imgBottom_B;
	pImgBR = partImgBuf.imgBottom_R;
	pImgMB = partImgBuf.imgMiddle_B;
	pImgMR = partImgBuf.imgMiddle_R;
	pImgTW = partImgBuf.imgTop_W;

	float* pResX = NULL;
	float* pResY = NULL;

	HEADER_POT hd;

	hd.fNumCol = (float)partImgBuf.nImageSizeX;
	hd.fNumRow = (float)partImgBuf.nImageSizeY;
	hd.fPixelResX = (float)m_resolX;
	hd.fPixelResY = (float)m_resolY;
	hd.fDepthBit = 8;

	alpf_save_pot(strFullPath, &hd, pImgBB, pImgBR, pImgMB, pImgMR, pImgTW);

	
}

// SHKang 2017/03/13
int CMNgManager::SavePotImage_SharedMemory(CString fullPath, int roiArea)
{
	int ret = 0;
	if(!m_pInspBoardInfo)
		return ret;

	CString tmpPath;
	tmpPath = fullPath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".pot")) == FALSE)
	{
		fullPath += _T(".pot");
	}

	UCHAR* pImgBB = m_pInspBoardInfo->partImgBuf.imgBottom_B;
	UCHAR* pImgBR = m_pInspBoardInfo->partImgBuf.imgBottom_R;
	UCHAR* pImgMB = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
	UCHAR* pImgMR = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
	UCHAR* pImgTW = m_pInspBoardInfo->partImgBuf.imgTop_W;

	int sizeX = m_pInspBoardInfo->partImgBuf.nImageSizeX;
	int sizeY = m_pInspBoardInfo->partImgBuf.nImageSizeY;

	QWORD ImgSz = sizeof(HEADER_POT) + sizeof(UCHAR) * sizeX * sizeY * 5;

	CSharedMemory shared;
	shared.AllocFile(fullPath, ImgSz, _T(""));
	shared.MapToWrite(0, ImgSz);
	BYTE * Buffer = shared.Byte();
	int nWritten=0;

	float* pResX = NULL;
	float* pResY = NULL;

	HEADER_POT hd;

	hd.fNumCol = (float)m_pInspBoardInfo->partImgBuf.nImageSizeX;
	hd.fNumRow = (float)m_pInspBoardInfo->partImgBuf.nImageSizeY;
	hd.fPixelResX = (float)m_resolX;
	hd.fPixelResY = (float)m_resolY;
	hd.fDepthBit = 8;

	memcpy(&Buffer[nWritten], &hd.fNumCol, sizeof(hd.fNumCol));			nWritten += sizeof(hd.fNumCol);
	memcpy(&Buffer[nWritten], &hd.fNumRow, sizeof(hd.fNumRow));		nWritten += sizeof(hd.fNumRow);
	memcpy(&Buffer[nWritten], &hd.fPixelResX, sizeof(hd.fPixelResX));			nWritten += sizeof(hd.fPixelResX);
	memcpy(&Buffer[nWritten], &hd.fPixelResY, sizeof(hd.fPixelResY));			nWritten += sizeof(hd.fPixelResY);
	memcpy(&Buffer[nWritten], &hd.fDepthBit, sizeof(hd.fDepthBit));			nWritten += sizeof(hd.fDepthBit);

	int y = 0;
	int x = 0;

	int nSize = sizeof(UCHAR) * sizeX * sizeY;
	memcpy(&Buffer[nWritten], pImgBB, nSize); nWritten += nSize;
	memcpy(&Buffer[nWritten], pImgBR, nSize); nWritten += nSize;
	memcpy(&Buffer[nWritten], pImgMB, nSize); nWritten += nSize;
	memcpy(&Buffer[nWritten], pImgMR, nSize); nWritten += nSize;
	memcpy(&Buffer[nWritten], pImgTW, nSize); nWritten += nSize;

// 	for(y=0 ; y<sizeY ; y++)
// 	{
// 		for(x=0 ; x<sizeX ; x++)
// 		{
// 			memcpy(&Buffer[nWritten], &pImgBB[x+(y*sizeX)], sizeof(UCHAR));			nWritten += sizeof(UCHAR);
// 		}
// 	}
// 	for(y=0 ; y<sizeY ; y++)
// 	{
// 		for(x=0 ; x<sizeX ; x++)
// 		{
// 			memcpy(&Buffer[nWritten], &pImgBR[x+(y*sizeX)], sizeof(UCHAR));			nWritten += sizeof(UCHAR);
// 		}
// 	}
// 	for(y=0 ; y<sizeY ; y++)
// 	{
// 		for(x=0 ; x<sizeX ; x++)
// 		{
// 			memcpy(&Buffer[nWritten], &pImgMB[x+(y*sizeX)], sizeof(UCHAR));			nWritten += sizeof(UCHAR);
// 		}
// 	}
// 	for(y=0 ; y<sizeY ; y++)
// 	{
// 		for(x=0 ; x<sizeX ; x++)
// 		{
// 			memcpy(&Buffer[nWritten], &pImgMR[x+(y*sizeX)], sizeof(UCHAR));			nWritten += sizeof(UCHAR);
// 		}
// 	}
// 	for(y=0 ; y<sizeY ; y++)
// 	{
// 		for(x=0 ; x<sizeX ; x++)
// 		{
// 			memcpy(&Buffer[nWritten], &pImgTW[x+(y*sizeX)], sizeof(UCHAR));			nWritten += sizeof(UCHAR);
// 		}
// 	}
	
	ret = 1;
	return ret;
}
int CMNgManager::SavePotImage_SharedMemory_Multi(CString fullPath, int roiArea, InspRoiImgBuf partImgBuf)
{
	int ret = 0;
	CString tmpPath;
	tmpPath = fullPath;
	tmpPath.MakeLower();
	if ((tmpPath.Right(4) == _T(".pot")) == FALSE)
	{
		fullPath += _T(".pot");
	}

	UCHAR* pImgBB = partImgBuf.imgBottom_B;
	UCHAR* pImgBR = partImgBuf.imgBottom_R;
	UCHAR* pImgMB = partImgBuf.imgMiddle_B;
	UCHAR* pImgMR = partImgBuf.imgMiddle_R;
	UCHAR* pImgTW = partImgBuf.imgTop_W;

	int sizeX = partImgBuf.nImageSizeX;
	int sizeY = partImgBuf.nImageSizeY;

	QWORD ImgSz = sizeof(HEADER_POT) + sizeof(UCHAR) * sizeX * sizeY * 5;

	CSharedMemory shared;
	shared.AllocFile(fullPath, ImgSz, _T(""));
	shared.MapToWrite(0, ImgSz);
	BYTE * Buffer = shared.Byte();
	int nWritten = 0;

	float* pResX = NULL;
	float* pResY = NULL;

	HEADER_POT hd;

	hd.fNumCol = (float)partImgBuf.nImageSizeX;
	hd.fNumRow = (float)partImgBuf.nImageSizeY;
	hd.fPixelResX = (float)m_resolX;
	hd.fPixelResY = (float)m_resolY;
	hd.fDepthBit = 8;

	memcpy(&Buffer[nWritten], &hd.fNumCol, sizeof(hd.fNumCol));			nWritten += sizeof(hd.fNumCol);
	memcpy(&Buffer[nWritten], &hd.fNumRow, sizeof(hd.fNumRow));		nWritten += sizeof(hd.fNumRow);
	memcpy(&Buffer[nWritten], &hd.fPixelResX, sizeof(hd.fPixelResX));			nWritten += sizeof(hd.fPixelResX);
	memcpy(&Buffer[nWritten], &hd.fPixelResY, sizeof(hd.fPixelResY));			nWritten += sizeof(hd.fPixelResY);
	memcpy(&Buffer[nWritten], &hd.fDepthBit, sizeof(hd.fDepthBit));			nWritten += sizeof(hd.fDepthBit);

	int y = 0;
	int x = 0;

	int nSize = sizeof(UCHAR) * sizeX * sizeY;
	memcpy(&Buffer[nWritten], pImgBB, nSize); nWritten += nSize;
	memcpy(&Buffer[nWritten], pImgBR, nSize); nWritten += nSize;
	memcpy(&Buffer[nWritten], pImgMB, nSize); nWritten += nSize;
	memcpy(&Buffer[nWritten], pImgMR, nSize); nWritten += nSize;
	memcpy(&Buffer[nWritten], pImgTW, nSize); nWritten += nSize;

	ret = 1;
	return ret;
}
void CMNgManager::SavePutImage(CString strFullPath, int roiArea)
{
	if(!m_pInspBoardInfo)
		return;

	BOOL bUseUV2 = FALSE;

// 	if(g_pMPTI->IsMachineTypeUV() && g_pMPTI->GetDiffGrabMode() == DiffGrabUV2)
// 		bUseUV2 = TRUE;

	UCHAR* pImgTR = NULL;
	UCHAR* pImgTG = NULL;
	UCHAR* pImgTB = NULL;
	UCHAR* pImgBB = NULL;	// UV Light
	UCHAR* pImgBR = NULL;	// UV2 Light

// 	pImgTR = new UCHAR[roiArea];
// 	pImgTG = new UCHAR[roiArea];
// 	pImgTB = new UCHAR[roiArea];
// 	pImgBB = new UCHAR[roiArea];
// 
// 	memcpy_s(pImgTR, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgTop_R, sizeof(UCHAR) * roiArea);
// 	memcpy_s(pImgTG, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgTop_G, sizeof(UCHAR) * roiArea);
// 	memcpy_s(pImgTB, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgTop_B, sizeof(UCHAR) * roiArea);
// 	memcpy_s(pImgBB, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgBottom_B, sizeof(UCHAR) * roiArea);

// 	if(g_pMPTI->m_nCameraType == (int)Basler_Color)
// 	{
// 		pImgTR = m_pInspBoardInfo->partImgBuf.imgTop_W;
// 		pImgTG = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
// 		pImgTB = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
// 		pImgBB = m_pInspBoardInfo->partImgBuf.imgBottom_B;
// 	}
// 	else
	{
		pImgTR = m_pInspBoardInfo->partImgBuf.imgTop_R;
		pImgTG = m_pInspBoardInfo->partImgBuf.imgTop_G;
		pImgTB = m_pInspBoardInfo->partImgBuf.imgTop_B;
		pImgBB = m_pInspBoardInfo->partImgBuf.imgBottom_B;
	}

	if(bUseUV2)
		pImgBR = m_pInspBoardInfo->partImgBuf.imgBottom_R;
	
	float* pResX = NULL;
	float* pResY = NULL;

	HEADER_POT hd;

	hd.fNumCol = (float)m_pInspBoardInfo->partImgBuf.nImageSizeX;
	hd.fNumRow = (float)m_pInspBoardInfo->partImgBuf.nImageSizeY;
	hd.fPixelResX = (float)m_resolX;
	hd.fPixelResY = (float)m_resolY;
	hd.fDepthBit = 8;

	alpf_save_put(strFullPath, &hd, pImgTR, pImgTG, pImgTB, pImgBB, pImgBR);

	// Debug
	int w = (int)hd.fNumCol;
	int h = (int)hd.fNumRow;

	// 	m_milProc->SaveImage(pImgTR, w, h, 1, strFullPath + _T("TR") +ProcTypeTIF);
	// 	m_milProc->SaveImage(pImgTG, w, h, 1, strFullPath + _T("TG")+ ProcTypeTIF);
	// 	m_milProc->SaveImage(pImgTB, w, h, 1, strFullPath + _T("TB")+ ProcTypeTIF);
	// 	m_milProc->SaveImage(pImgBB, w, h, 1, strFullPath + _T("BB")+ ProcTypeTIF);
	
// 	if(pImgTR != NULL)
// 	{
// 		delete [] pImgTR;
// 		pImgTR = NULL;
// 	}
// 	if(pImgTG != NULL)
// 	{
// 		delete [] pImgTG;
// 		pImgTG = NULL;
// 	}
// 	if(pImgTB != NULL)
// 	{
// 		delete [] pImgTB;
// 		pImgTB = NULL;
// 	}
// 	if(pImgBB != NULL)
// 	{
// 		delete [] pImgBB;
// 		pImgBB = NULL;
// 	}
}
void CMNgManager::SavePutImage_Multi(CString strFullPath, int roiArea, InspRoiImgBuf partImgBuf)
{


	BOOL bUseUV2 = FALSE;

	UCHAR* pImgTR = NULL;
	UCHAR* pImgTG = NULL;
	UCHAR* pImgTB = NULL;
	UCHAR* pImgBB = NULL;	// UV Light
	UCHAR* pImgBR = NULL;	// UV2 Light

	{
		pImgTR = partImgBuf.imgTop_R;
		pImgTG = partImgBuf.imgTop_G;
		pImgTB = partImgBuf.imgTop_B;
		pImgBB = partImgBuf.imgBottom_B;
	}

	if (bUseUV2)
		pImgBR = partImgBuf.imgBottom_R;

	float* pResX = NULL;
	float* pResY = NULL;

	HEADER_POT hd;

	hd.fNumCol = (float)partImgBuf.nImageSizeX;
	hd.fNumRow = (float)partImgBuf.nImageSizeY;
	hd.fPixelResX = (float)m_resolX;
	hd.fPixelResY = (float)m_resolY;
	hd.fDepthBit = 8;

	alpf_save_put(strFullPath, &hd, pImgTR, pImgTG, pImgTB, pImgBB, pImgBR);

	int w = (int)hd.fNumCol;
	int h = (int)hd.fNumRow;


}
void CMNgManager::SavePstImage(CString strFullPath, int roiArea, int nSideCamNum)
{
	if(!m_pInspBoardInfo)
		return;

	UCHAR* pImgSR = NULL;
	UCHAR* pImgSG = NULL;
	UCHAR* pImgSB = NULL;

	bool bIsSavePst = false;
	
// 	pImgSR = new UCHAR[roiArea];
// 	pImgSG = new UCHAR[roiArea];
// 	pImgSB = new UCHAR[roiArea];

	switch(nSideCamNum)
	{
	case 0:
		if(m_pInspBoardInfo->partImgBuf.imgSide1_R != NULL && m_pInspBoardInfo->partImgBuf.imgSide1_G != NULL && m_pInspBoardInfo->partImgBuf.imgSide1_B != NULL)
		{
			// SHKang 2017/05/24 : 메모리를 다시 생성할 필요가 있을까?
			pImgSR = m_pInspBoardInfo->partImgBuf.imgSide1_R;
			pImgSG = m_pInspBoardInfo->partImgBuf.imgSide1_G;
			pImgSB = m_pInspBoardInfo->partImgBuf.imgSide1_B;

// 			pImgSR = new UCHAR[roiArea];
// 			pImgSG = new UCHAR[roiArea];
// 			pImgSB = new UCHAR[roiArea];
// 
// 			memcpy_s(pImgSR, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgSide1_R, sizeof(UCHAR) * roiArea);
// 			memcpy_s(pImgSG, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgSide1_G, sizeof(UCHAR) * roiArea);
// 			memcpy_s(pImgSB, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgSide1_B, sizeof(UCHAR) * roiArea);

			bIsSavePst = true;
		}
		
		break;
	case 1:
		if(m_pInspBoardInfo->partImgBuf.imgSide2_R != NULL && m_pInspBoardInfo->partImgBuf.imgSide2_G != NULL && m_pInspBoardInfo->partImgBuf.imgSide2_B != NULL)
		{
			// SHKang 2017/05/24 : 메모리를 다시 생성할 필요가 있을까?
			pImgSR = m_pInspBoardInfo->partImgBuf.imgSide2_R;
			pImgSG = m_pInspBoardInfo->partImgBuf.imgSide2_G;
			pImgSB = m_pInspBoardInfo->partImgBuf.imgSide2_B;

// 			pImgSR = new UCHAR[roiArea];
// 			pImgSG = new UCHAR[roiArea];
// 			pImgSB = new UCHAR[roiArea];
// 
// 			memcpy_s(pImgSR, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgSide2_R, sizeof(UCHAR) * roiArea);
// 			memcpy_s(pImgSG, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgSide2_G, sizeof(UCHAR) * roiArea);
// 			memcpy_s(pImgSB, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgSide2_B, sizeof(UCHAR) * roiArea);

			bIsSavePst = true;
		}
		break;
	case 2:
		if(m_pInspBoardInfo->partImgBuf.imgSide3_R != NULL && m_pInspBoardInfo->partImgBuf.imgSide3_G != NULL && m_pInspBoardInfo->partImgBuf.imgSide3_B != NULL)
		{
			// SHKang 2017/05/24 : 메모리를 다시 생성할 필요가 있을까?
			pImgSR = m_pInspBoardInfo->partImgBuf.imgSide3_R;
			pImgSG = m_pInspBoardInfo->partImgBuf.imgSide3_G;
			pImgSB = m_pInspBoardInfo->partImgBuf.imgSide3_B;

// 			pImgSR = new UCHAR[roiArea];
// 			pImgSG = new UCHAR[roiArea];
// 			pImgSB = new UCHAR[roiArea];
// 
// 			memcpy_s(pImgSR, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgSide3_R, sizeof(UCHAR) * roiArea);
// 			memcpy_s(pImgSG, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgSide3_G, sizeof(UCHAR) * roiArea);
// 			memcpy_s(pImgSB, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgSide3_B, sizeof(UCHAR) * roiArea);

			bIsSavePst = true;
		}
		break;
	case 3:
		if(m_pInspBoardInfo->partImgBuf.imgSide4_R != NULL && m_pInspBoardInfo->partImgBuf.imgSide4_G != NULL || m_pInspBoardInfo->partImgBuf.imgSide4_B != NULL)
		{
			// SHKang 2017/05/24 : 메모리를 다시 생성할 필요가 있을까?
			pImgSR = m_pInspBoardInfo->partImgBuf.imgSide4_R;
			pImgSG = m_pInspBoardInfo->partImgBuf.imgSide4_G;
			pImgSB = m_pInspBoardInfo->partImgBuf.imgSide4_B;

// 			pImgSR = new UCHAR[roiArea];
// 			pImgSG = new UCHAR[roiArea];
// 			pImgSB = new UCHAR[roiArea];
// 
// 			memcpy_s(pImgSR, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgSide4_R, sizeof(UCHAR) * roiArea);
// 			memcpy_s(pImgSG, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgSide4_G, sizeof(UCHAR) * roiArea);
// 			memcpy_s(pImgSB, sizeof(UCHAR) * roiArea, m_pInspBoardInfo->partImgBuf.imgSide4_B, sizeof(UCHAR) * roiArea);

			bIsSavePst = true;
		}
		break;
	}

	float* pResX = NULL;
	float* pResY = NULL;

	HEADER_POT hd;
	CString strPstFileName;

	if(bIsSavePst)
	{
		hd.fNumCol = (float)m_pInspBoardInfo->partImgBuf.nImageSizeX;
		hd.fNumRow = (float)m_pInspBoardInfo->partImgBuf.nImageSizeY;
		hd.fPixelResX = (float)m_resolX;
		hd.fPixelResY = (float)m_resolY;
		hd.fDepthBit = 8;


		strPstFileName.Format(_T("%s@S%d"),strFullPath, nSideCamNum+1);

		alpf_save_pst(strPstFileName, &hd, pImgSR, pImgSG, pImgSB);
	}

	// Debug
// 	int w = (int)hd.fNumCol;
// 	int h = (int)hd.fNumRow;
// 
// 	 	m_milProc->SaveImage(pImgSR, w, h, 1, strPstFileName + _T("SR") + ProcTypeTIF);
// 	 	m_milProc->SaveImage(pImgSG, w, h, 1, strPstFileName + _T("SG") +ProcTypeTIF);
// 	 	m_milProc->SaveImage(pImgSB, w, h, 1, strPstFileName + _T("SB") +ProcTypeTIF);

	// SHKang 2017/05/24 : 메모리를 다시 생성할 필요가 있을까? 생성하지 않으므로 해제해서도 안된다.
// 	if(pImgSR != NULL)
// 	{
// 		delete [] pImgSR;
// 		pImgSR = NULL;
// 	}
// 	if(pImgSG != NULL)
// 	{
// 		delete [] pImgSG;
// 		pImgSG = NULL;
// 	}
// 	if(pImgSB != NULL)
// 	{
// 		delete [] pImgSB;
// 		pImgSB = NULL;
// 	}
}
void CMNgManager::SavePstImage_Multi(CString strFullPath, int roiArea, int nSideCamNum , InspRoiImgBuf partImgBuf)
{
	UCHAR* pImgSR = NULL;
	UCHAR* pImgSG = NULL;
	UCHAR* pImgSB = NULL;

	bool bIsSavePst = false;


	switch (nSideCamNum)
	{
	case 0:
		if (partImgBuf.imgSide1_R != NULL && partImgBuf.imgSide1_G != NULL && partImgBuf.imgSide1_B != NULL)
		{
			// SHKang 2017/05/24 : 메모리를 다시 생성할 필요가 있을까?
			pImgSR = partImgBuf.imgSide1_R;
			pImgSG = partImgBuf.imgSide1_G;
			pImgSB = partImgBuf.imgSide1_B;

			bIsSavePst = true;
		}

		break;
	case 1:
		if (partImgBuf.imgSide2_R != NULL && partImgBuf.imgSide2_G != NULL && partImgBuf.imgSide2_B != NULL)
		{
			// SHKang 2017/05/24 : 메모리를 다시 생성할 필요가 있을까?
			pImgSR = partImgBuf.imgSide2_R;
			pImgSG = partImgBuf.imgSide2_G;
			pImgSB = partImgBuf.imgSide2_B;

			bIsSavePst = true;
		}
		break;
	case 2:
		if (partImgBuf.imgSide3_R != NULL && partImgBuf.imgSide3_G != NULL && partImgBuf.imgSide3_B != NULL)
		{
			// SHKang 2017/05/24 : 메모리를 다시 생성할 필요가 있을까?
			pImgSR = partImgBuf.imgSide3_R;
			pImgSG = partImgBuf.imgSide3_G;
			pImgSB = partImgBuf.imgSide3_B;

			bIsSavePst = true;
		}
		break;
	case 3:
		if (partImgBuf.imgSide4_R != NULL && partImgBuf.imgSide4_G != NULL || partImgBuf.imgSide4_B != NULL)
		{
			// SHKang 2017/05/24 : 메모리를 다시 생성할 필요가 있을까?
			pImgSR = partImgBuf.imgSide4_R;
			pImgSG = partImgBuf.imgSide4_G;
			pImgSB = partImgBuf.imgSide4_B;

			bIsSavePst = true;
		}
		break;
	}

	float* pResX = NULL;
	float* pResY = NULL;

	HEADER_POT hd;
	CString strPstFileName;

	if (bIsSavePst)
	{
		hd.fNumCol = (float)partImgBuf.nImageSizeX;
		hd.fNumRow = (float)partImgBuf.nImageSizeY;
		hd.fPixelResX = (float)m_resolX;
		hd.fPixelResY = (float)m_resolY;
		hd.fDepthBit = 8;


		strPstFileName.Format(_T("%s@S%d"), strFullPath, nSideCamNum + 1);

		alpf_save_pst(strPstFileName, &hd, pImgSR, pImgSG, pImgSB);
	}

}
// 
// void CMNgManager::SaveDispImage_Foreign(CString fullPath, BOOL zmapUse, UCHAR* retColorImg, int nGapX, int nGapY, AForeignResult vforeign, int nDivide)
// {
// 	if (!m_pInspBoardInfo_Foreign)
// 		return;
// 	int nFOVW = m_pInspBoardInfo_Foreign->zmapData.zmapSizeX;
// 	int nFOVH = m_pInspBoardInfo_Foreign->zmapData.zmapSizeY;
// 	int nImgW = nFOVW / nDivide - 1;
// 	int nImgH = nFOVH / nDivide - 1;
// 	int nL = vforeign.m_rcRect.left - nGapX;
// 	int nR = vforeign.m_rcRect.right + nGapX;
// 	int nT = vforeign.m_rcRect.top - nGapY;
// 	int nB = vforeign.m_rcRect.bottom + nGapY;
// 	if (nL < 0) nL = 0;
// 	if (nR < 0) nR = 0;
// 	if (nT < 0) nT = 0;
// 	if (nB < 0) nB = 0;
// 	if (nL > nImgW) nL = nImgW;
// 	if (nR > nImgW) nR = nImgW;
// 	if (nT > nImgH) nT = nImgH;
// 	if (nB > nImgH) nB = nImgH;
// 	if (nR <= nL || nB <= nT) return;
// 	int nW = nR - nL;
// 	int nH = nB - nT;
// 	int nCX = nL + (nW / 2);
// 	int nCY = nT + (nH / 2);
// 	/*UCHAR* ConvertImgR = new UCHAR[nFOVW * nFOVH];
// 	UCHAR* ConvertImgG = new UCHAR[nFOVW * nFOVH];
// 	UCHAR* ConvertImgB = new UCHAR[nFOVW * nFOVH];
// 	UCHAR* ConvertImgBB = new UCHAR[nFOVW * nFOVH];
// 	UCHAR* ConvertImgBR = new UCHAR[nFOVW * nFOVH];*/
// 	UCHAR* ConvertImgR = g_pMManager->pem_new<UCHAR>(true, nFOVW * nFOVH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* ConvertImgG = g_pMManager->pem_new<UCHAR>(true, nFOVW * nFOVH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* ConvertImgB = g_pMManager->pem_new<UCHAR>(true, nFOVW * nFOVH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* ConvertImgBB = g_pMManager->pem_new<UCHAR>(true, nFOVW * nFOVH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* ConvertImgBR = g_pMManager->pem_new<UCHAR>(true, nFOVW * nFOVH, (PCHAR)__FUNCTION__, __LINE__);
// 	memset(ConvertImgR, 0, sizeof(UCHAR) * nFOVW * nFOVH);
// 	memset(ConvertImgG, 0, sizeof(UCHAR) * nFOVW * nFOVH);
// 	memset(ConvertImgB, 0, sizeof(UCHAR) * nFOVW * nFOVH);
// 	memset(ConvertImgBB, 0, sizeof(UCHAR) * nFOVW * nFOVH);
// 	memset(ConvertImgBR, 0, sizeof(UCHAR) * nFOVW * nFOVH);
// 
// 	if (g_pMPTI->m_nCameraType == Basler_Color)
// 	{
// 		int nFOVSizeX = 2040;
// 		int nFOVSizeY = 2040;
// 
// 		m_milProc->GetClipImage((UCHAR*)m_pInspBoardInfo_Foreign->fovImgBuf.imgTop_W, nFOVSizeX, nFOVSizeY, ConvertImgR, nImgW, nImgH, nFOVW, nFOVH);
// 		m_milProc->GetClipImage((UCHAR*)m_pInspBoardInfo_Foreign->fovImgBuf.imgMiddle_R, nFOVSizeX, nFOVSizeY, ConvertImgG, nImgW, nImgH, nFOVW, nFOVH);
// 		m_milProc->GetClipImage((UCHAR*)m_pInspBoardInfo_Foreign->fovImgBuf.imgMiddle_B, nFOVSizeX, nFOVSizeY, ConvertImgB, nImgW, nImgH, nFOVW, nFOVH);
// 		m_milProc->GetClipImage((UCHAR*)m_pInspBoardInfo_Foreign->fovImgBuf.imgBottom_B, nFOVSizeX, nFOVSizeY, ConvertImgBB, nImgW, nImgH, nFOVW, nFOVH);
// 		if (vforeign.m_nForeignDefect == eGrayBubForeign && vforeign.m_nUVColorSelect > 2) // For Bubble Inspection
// 		{
// 			g_pMPTI->ConvertSubBB = ((UCHAR*)g_pMPTI->GetGrabOutputBuf_Color(m_pInspBoardInfo_Foreign->fovIndex, 2, 3));
// 			m_milProc->GetClipImage((UCHAR*)g_pMPTI->ConvertSubBB, nFOVSizeX, nFOVSizeY, ConvertImgBB, nFOVW / 2, nFOVH / 2, nFOVW, nFOVH);
// 		}
// 		else
// 			m_milProc->GetClipImage((UCHAR*)m_pInspBoardInfo_Foreign->fovImgBuf.imgBottom_B, nFOVSizeX, nFOVSizeY, ConvertImgBB, nFOVW / 2, nFOVH / 2, nFOVW, nFOVH);
// 	}
// 	else
// 	{
// 		if (m_pInspBoardInfo_Foreign->fovImgBuf.imgTop_R != NULL)
// 			m_milProc->GetClipBuff(m_pInspBoardInfo_Foreign->fovImgBuf.imgTop_R, ConvertImgR, nImgW, nImgH, nFOVW, nFOVH);
// 		if (m_pInspBoardInfo_Foreign->fovImgBuf.imgTop_G != NULL)
// 			m_milProc->GetClipBuff(m_pInspBoardInfo_Foreign->fovImgBuf.imgTop_G, ConvertImgG, nImgW, nImgH, nFOVW, nFOVH);
// 		if (m_pInspBoardInfo_Foreign->fovImgBuf.imgTop_B != NULL)
// 			m_milProc->GetClipBuff(m_pInspBoardInfo_Foreign->fovImgBuf.imgTop_B, ConvertImgB, nImgW, nImgH, nFOVW, nFOVH);
// 		if (m_pInspBoardInfo_Foreign->fovImgBuf.imgBottom_B != NULL)
// 			m_milProc->GetClipBuff(m_pInspBoardInfo_Foreign->fovImgBuf.imgBottom_B, ConvertImgBB, nImgW, nImgH, nFOVW, nFOVH);
// 		if (m_pInspBoardInfo_Foreign->fovImgBuf.imgBottom_R != NULL)
// 			m_milProc->GetClipBuff(m_pInspBoardInfo_Foreign->fovImgBuf.imgBottom_R, ConvertImgBR, nImgW, nImgH, nFOVW, nFOVH);
// 		m_milProc->SaveWorkImg(ConvertImgR, nFOVW, nFOVH, _T("Foreign_test_milTochar.bmp"));
// 		m_milProc->SaveWorkImg(ConvertImgBB, nFOVW, nFOVH, _T("Foreign_test_milTochar_BB.bmp"));
// 	}
// 
// 	// uchar* -> resizing
// 	/*UCHAR* ResizingImgR = new UCHAR[nImgW * nImgH];
// 	UCHAR* ResizingImgG = new UCHAR[nImgW * nImgH];
// 	UCHAR* ResizingImgB = new UCHAR[nImgW * nImgH];
// 	UCHAR* ResizingImgBB = new UCHAR[nImgW * nImgH];
// 	UCHAR* ResizingImgBR = new UCHAR[nImgW * nImgH];*/
// 	UCHAR* ResizingImgR = g_pMManager->pem_new<UCHAR>(true, nImgW * nImgH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* ResizingImgG = g_pMManager->pem_new<UCHAR>(true, nImgW * nImgH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* ResizingImgB = g_pMManager->pem_new<UCHAR>(true, nImgW * nImgH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* ResizingImgBB = g_pMManager->pem_new<UCHAR>(true, nImgW * nImgH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* ResizingImgBR = g_pMManager->pem_new<UCHAR>(true, nImgW * nImgH, (PCHAR)__FUNCTION__, __LINE__);
// 	memset(ResizingImgR, 0, sizeof(UCHAR) * nImgW * nImgH);
// 	memset(ResizingImgG, 0, sizeof(UCHAR) * nImgW * nImgH);
// 	memset(ResizingImgB, 0, sizeof(UCHAR) * nImgW * nImgH);
// 	memset(ResizingImgBB, 0, sizeof(UCHAR) * nImgW * nImgH);
// 	memset(ResizingImgBR, 0, sizeof(UCHAR) * nImgW * nImgH);
// 	double factor = m_milProc->GetResizeFactor(nImgW, nImgH, nFOVW, nFOVH);
// 	m_milProc->ResizeImg(ConvertImgR, ResizingImgR, nFOVW, nFOVH, nImgW, nImgH, 1, factor);
// 	m_milProc->ResizeImg(ConvertImgG, ResizingImgG, nFOVW, nFOVH, nImgW, nImgH, 1, factor);
// 	m_milProc->ResizeImg(ConvertImgB, ResizingImgB, nFOVW, nFOVH, nImgW, nImgH, 1, factor);
// 	m_milProc->ResizeImg(ConvertImgBB, ResizingImgBB, nFOVW, nFOVH, nImgW, nImgH, 1, factor);
// 	m_milProc->ResizeImg(ConvertImgBR, ResizingImgBR, nFOVW, nFOVH, nImgW, nImgH, 1, factor);
// 	m_milProc->SaveWorkImg(ResizingImgR, nImgW, nImgH, _T("Foreign_test_resizing.bmp"));
// 	m_milProc->SaveWorkImg(ResizingImgBB, nImgW, nImgH, _T("Foreign_test_resizing_BB.bmp"));
// 	/*float* img32 = new float[nW * nH];
// 	float* clipzmap = new float[nW * nH];
// 	UCHAR* clipImgR = new UCHAR[nW * nH];
// 	UCHAR* clipImgG = new UCHAR[nW * nH];
// 	UCHAR* clipImgB = new UCHAR[nW * nH];
// 	UCHAR* clipImgBB = new UCHAR[nW * nH];
// 	UCHAR* clipImgBR = new UCHAR[nW * nH];*/
// 	float* img32 = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
// 	float* clipzmap = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* clipImgR = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* clipImgG = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* clipImgB = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* clipImgBB = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* clipImgBR = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
// 	memset(img32, 0, sizeof(float) * nW * nH);
// 	memset(clipzmap, 0, sizeof(float) * nW * nH);
// 	memset(clipImgR, 0, sizeof(UCHAR) * nW * nH);
// 	memset(clipImgG, 0, sizeof(UCHAR) * nW * nH);
// 	memset(clipImgB, 0, sizeof(UCHAR) * nW * nH);
// 	memset(clipImgBB, 0, sizeof(UCHAR) * nW * nH);
// 	memset(clipImgBR, 0, sizeof(UCHAR) * nW * nH);
// 	m_milProc->GetClipImage(ResizingImgR, nImgW, nImgH, clipImgR, nCX, nCY, nW, nH);
// 	m_milProc->GetClipImage(ResizingImgG, nImgW, nImgH, clipImgG, nCX, nCY, nW, nH);
// 	m_milProc->GetClipImage(ResizingImgB, nImgW, nImgH, clipImgB, nCX, nCY, nW, nH);
// 	m_milProc->GetClipImage(ResizingImgBB, nImgW, nImgH, clipImgBB, nCX, nCY, nW, nH);
// 	m_milProc->GetClipImage(ResizingImgBR, nImgW, nImgH, clipImgBR, nCX, nCY, nW, nH);
// 	m_milProc->SaveWorkImg(clipImgR, nW, nH, _T("Foreign_test_ClipImg.bmp"));
// 	m_milProc->SaveWorkImg(clipImgBB, nW, nH, _T("Foreign_test_ClipImg_BB.bmp"));
// 	RECT rectZerotofull = { 0,0, nW, nH };
// 	if (g_pMPTI->GetCompositeLightMode() == 0)
// 	{
// 		if (vforeign.m_nForeignDefect == eGrayBubForeign)
// 		{
// 			alpf_make_img24n32f(rectZerotofull, nW, clipImgBB, clipImgBB, clipImgBB, 1, 1, 1, nW, img32, retColorImg);
// 			HEADER_POT hd;
// 			hd.fNumCol = (float)nW;
// 			hd.fNumRow = (float)nH;
// 			hd.fPixelResX = (float)m_resolX;
// 			hd.fPixelResY = (float)m_resolY;
// 			hd.fDepthBit = 8;
// 			alpf_save_put(fullPath, &hd, clipImgR, clipImgG, clipImgB, clipImgBB, clipImgBR);
// 		}
// 		else
// 			alpf_make_img24n32f(rectZerotofull, nW, clipImgR, clipImgG, clipImgB, m_ngParam.redFator, m_ngParam.greenFator, m_ngParam.blueFator, nW, img32, retColorImg);
// 	}
// 	else
// 	{
// 		alpf_make_img24n32f_CompositeBtm(rectZerotofull, nW, clipImgR, clipImgG, clipImgB, clipImgBR, clipImgBB,
// 			m_ngParam.redFator, m_ngParam.greenFator, m_ngParam.blueFator, m_ngParam.btmRedfactor, m_ngParam.btmBluefactor, g_pMPTI->m_fCompoBtmR, g_pMPTI->m_fCompoBtmG, g_pMPTI->m_fCompoBtmB,
// 			nW, img32, retColorImg);
// 	}
// 	if (g_pMPTI->IsForeignEnable() && zmapUse == TRUE)
// 	{
// 		int nCX3D = nCX;
// 		int nCY3D = nCY;
// 		m_milProc->GetCropZmap(m_pInspBoardInfo_Foreign->zmapForeignData.data, clipzmap, nFOVW, nFOVH, nCX3D, nCY3D, nW, nH);
// 		HEADER_PTT hd;
// 		hd.uiNumRow = nW;
// 		hd.uiNumCol = nH;
// 		hd.fNumRow = (float)nW;
// 		hd.fNumCol = (float)nH;
// 		hd.pixelResX = (float)_mm2micron(m_resolX);
// 		hd.pixelResY = (float)_mm2micron(m_resolY);
// 		hd.zResolX = (float)_mm2micron(m_resolX);
// 		hd.zResolY = (float)_mm2micron(m_resolY);
// 		alpf_save_ptt(fullPath, &hd, clipzmap, img32);
// 	}
// 	Delete_1DArray(&ConvertImgR);
// 	Delete_1DArray(&ConvertImgG);
// 	Delete_1DArray(&ConvertImgB);
// 	Delete_1DArray(&ConvertImgBB);
// 	Delete_1DArray(&ConvertImgBR);
// 
// 	Delete_1DArray(&ResizingImgR);
// 	Delete_1DArray(&ResizingImgG);
// 	Delete_1DArray(&ResizingImgB);
// 	Delete_1DArray(&ResizingImgBR);
// 	Delete_1DArray(&ResizingImgBB);
// 
// 	Delete_1DArray(&img32);
// 	Delete_1DArray(&clipzmap);
// 	Delete_1DArray(&clipImgR);
// 	Delete_1DArray(&clipImgG);
// 	Delete_1DArray(&clipImgB);
// 	Delete_1DArray(&clipImgBB);
// 	Delete_1DArray(&clipImgBR);
// }
// void CMNgManager::SaveDispImage_Foreign_new(CString fullPath, BOOL zmapUse, UCHAR* retColorImg, int nGapX, int nGapY, AForeignResult vforeign, int nDivide, float* pfOff3D)
// {
// 	if (!m_pInspBoardInfo_Foreign)
// 		return;
// 
// 	float* pf3D = m_pInspBoardInfo_Foreign->zmapForeignData.data;
// 	UCHAR* pucTR = m_pInspBoardInfo_Foreign->partImgBuf.imgTop_R;
// 	UCHAR* pucTG = m_pInspBoardInfo_Foreign->partImgBuf.imgTop_G;
// 	UCHAR* pucTB = m_pInspBoardInfo_Foreign->partImgBuf.imgTop_B;
// 	UCHAR* pucBR = m_pInspBoardInfo_Foreign->partImgBuf.imgBottom_R;
// 	UCHAR* pucBB = m_pInspBoardInfo_Foreign->partImgBuf.imgBottom_B;
// 	int nFov2DW = m_pInspBoardInfo_Foreign->partImgBuf.nImageSizeX;
// 	int nFov2DH = m_pInspBoardInfo_Foreign->partImgBuf.nImageSizeY;
// 	int nFOVW = m_pInspBoardInfo_Foreign->zmapData.zmapSizeX;
// 	int nFOVH = m_pInspBoardInfo_Foreign->zmapData.zmapSizeY;
// 	int nImgW = nFOVW / nDivide - 1;
// 	int nImgH = nFOVH / nDivide - 1;
// 	cv::Mat imgR, imgG, imgB;
// 	if (pfOff3D)
// 	{
// 		if (m_pInspBoardInfo == NULL)
// 			return;
// 		pf3D = pfOff3D;
// 		nFov2DW = m_pInspBoardInfo->partImgBuf.nImageSizeX;
// 		nFov2DH = m_pInspBoardInfo->partImgBuf.nImageSizeY;
// 		nFOVW = m_pInspBoardInfo->zmapData.zmapSizeX;
// 		nFOVH = m_pInspBoardInfo->zmapData.zmapSizeY;
// 		nImgW = nFOVW / nDivide - 1;
// 		nImgH = nFOVH / nDivide - 1;
// 		
// 		cv::Mat imgROrg = cv::Mat(nFOVH, nFOVW, CV_8UC1, cv::Scalar(0));
// 		cv::Mat imgGOrg = cv::Mat(nFOVH, nFOVW, CV_8UC1, cv::Scalar(0));
// 		cv::Mat imgBOrg = cv::Mat(nFOVH, nFOVW, CV_8UC1, cv::Scalar(0));
// 		m_milProc->GetClipBuff(m_pInspBoardInfo->fovImgBuf.imgTop_R, imgROrg.data, nFOVW / 2, nFOVH / 2, nFOVW, nFOVH);
// 		m_milProc->GetClipBuff(m_pInspBoardInfo->fovImgBuf.imgTop_G, imgGOrg.data, nFOVW / 2, nFOVH / 2, nFOVW, nFOVH);
// 		m_milProc->GetClipBuff(m_pInspBoardInfo->fovImgBuf.imgTop_B, imgBOrg.data, nFOVW / 2, nFOVH / 2, nFOVW, nFOVH);
// 
// 		nImgW = nFOVW / nDivide - 1;
// 		nImgH = nFOVH / nDivide - 1;
// 		cv::resize(imgROrg, imgR, cv::Size(nImgW, nImgH));
// 		cv::resize(imgGOrg, imgG, cv::Size(nImgW, nImgH));
// 		cv::resize(imgBOrg, imgB, cv::Size(nImgW, nImgH));
// 		pucTR = imgR.ptr<UCHAR>();
// 		pucTG = imgG.ptr<UCHAR>();
// 		pucTB = imgB.ptr<UCHAR>();
// 		pucBR = NULL;
// 		pucBB = NULL;
// 	}
// 
// 
// 	//사이즈 다를시, 바슬러 컬러카메라(CI)일때 기존 시퀀스 타도록 수정
// 	if (nImgH != nFov2DH || nImgW != nFov2DW || g_pMPTI->m_nCameraType == Basler_Color)
// 		SaveDispImage_Foreign(fullPath, zmapUse, retColorImg, nGapX, nGapY, vforeign, nDivide);
// 
// 	int nL = vforeign.m_rcRect.left - nGapX;
// 	int nR = vforeign.m_rcRect.right + nGapX;
// 	int nT = vforeign.m_rcRect.top - nGapY;
// 	int nB = vforeign.m_rcRect.bottom + nGapY;
// 	if (nL < 0) nL = 0;
// 	if (nR < 0) nR = 0;
// 	if (nT < 0) nT = 0;
// 	if (nB < 0) nB = 0;
// 	if (nL > nImgW) nL = nImgW;
// 	if (nR > nImgW) nR = nImgW;
// 	if (nT > nImgH) nT = nImgH;
// 	if (nB > nImgH) nB = nImgH;
// 	if (nR <= nL || nB <= nT) return;
// 	int nW = nR - nL;
// 	int nH = nB - nT;
// 	int nCX = nL + (nW / 2);
// 	int nCY = nT + (nH / 2);
// 
// #if _DEBUG
// 	cv::Mat imgSrc(nH, nW, CV_8UC3, retColorImg);
// #endif
// 
// 	double factor = m_milProc->GetResizeFactor(nImgW, nImgH, nFOVW, nFOVH);
// 
// 	float* img32 = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
// 	float* clipzmap = g_pMManager->pem_new<float>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* clipImgR = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* clipImgG = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* clipImgB = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* clipImgBB = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
// 	UCHAR* clipImgBR = g_pMManager->pem_new<UCHAR>(true, nW * nH, (PCHAR)__FUNCTION__, __LINE__);
// 	memset(img32, 0, sizeof(float) * nW * nH);
// 	memset(clipzmap, 0, sizeof(float) * nW * nH);
// 	memset(clipImgR, 0, sizeof(UCHAR) * nW * nH);
// 	memset(clipImgG, 0, sizeof(UCHAR) * nW * nH);
// 	memset(clipImgB, 0, sizeof(UCHAR) * nW * nH);
// 	memset(clipImgBB, 0, sizeof(UCHAR) * nW * nH);
// 	memset(clipImgBR, 0, sizeof(UCHAR) * nW * nH);
// 	if (pucTR) m_milProc->GetClipImage(pucTR, nImgW, nImgH, clipImgR, nCX, nCY, nW, nH);
// 	if (pucTG) m_milProc->GetClipImage(pucTG, nImgW, nImgH, clipImgG, nCX, nCY, nW, nH);
// 	if (pucTB) m_milProc->GetClipImage(pucTB, nImgW, nImgH, clipImgB, nCX, nCY, nW, nH);
// 	if (pucBB) m_milProc->GetClipImage(pucBB, nImgW, nImgH, clipImgBB, nCX, nCY, nW, nH);
// 	if (pucBR) m_milProc->GetClipImage(pucBR, nImgW, nImgH, clipImgBR, nCX, nCY, nW, nH);
// 	m_milProc->SaveWorkImg(clipImgR, nW, nH, _T("Foreign_test_ClipImg.bmp"));
// 	m_milProc->SaveWorkImg(clipImgBB, nW, nH, _T("Foreign_test_ClipImg_BB.bmp"));
// 	RECT rectZerotofull = { 0,0, nW, nH };
// 	if (g_pMPTI->GetCompositeLightMode() == 0)
// 	{
// 		if (vforeign.m_nForeignDefect == eGrayBubForeign)
// 		{
// 			alpf_make_img24n32f(rectZerotofull, nW, clipImgBB, clipImgBB, clipImgBB, 1, 1, 1, nW, img32, retColorImg);
// 			HEADER_POT hd;
// 			hd.fNumCol = (float)nW;
// 			hd.fNumRow = (float)nH;
// 			hd.fPixelResX = (float)m_resolX;
// 			hd.fPixelResY = (float)m_resolY;
// 			hd.fDepthBit = 8;
// 			alpf_save_put(fullPath, &hd, clipImgR, clipImgG, clipImgB, clipImgBB, clipImgBR);
// 		}
// 		else
// 			alpf_make_img24n32f(rectZerotofull, nW, clipImgR, clipImgG, clipImgB, m_ngParam.redFator, m_ngParam.greenFator, m_ngParam.blueFator, nW, img32, retColorImg);
// 	}
// 	else
// 	{
// 		alpf_make_img24n32f_CompositeBtm(rectZerotofull, nW, clipImgR, clipImgG, clipImgB, clipImgBR, clipImgBB,
// 			m_ngParam.redFator, m_ngParam.greenFator, m_ngParam.blueFator, m_ngParam.btmRedfactor, m_ngParam.btmBluefactor, g_pMPTI->m_fCompoBtmR, g_pMPTI->m_fCompoBtmG, g_pMPTI->m_fCompoBtmB,
// 			nW, img32, retColorImg);
// 	}
// 	if (g_pMPTI->IsForeignEnable() && zmapUse == TRUE)
// 	{
// 		int nCX3D = nCX;
// 		int nCY3D = nCY;
// 		m_milProc->GetCropZmap(pf3D, clipzmap, nFOVW, nFOVH, nCX3D, nCY3D, nW, nH);
// 		HEADER_PTT hd;
// 		hd.uiNumRow = nW;
// 		hd.uiNumCol = nH;
// 		hd.fNumRow = (float)nW;
// 		hd.fNumCol = (float)nH;
// 		hd.pixelResX = (float)_mm2micron(m_resolX);
// 		hd.pixelResY = (float)_mm2micron(m_resolY);
// 		hd.zResolX = (float)_mm2micron(m_resolX);
// 		hd.zResolY = (float)_mm2micron(m_resolY);
// 		alpf_save_ptt(fullPath, &hd, clipzmap, img32);
// 	}
// 
// 	Delete_1DArray(&img32);
// 	Delete_1DArray(&clipzmap);
// 	Delete_1DArray(&clipImgR);
// 	Delete_1DArray(&clipImgG);
// 	Delete_1DArray(&clipImgB);
// 	Delete_1DArray(&clipImgBB);
// 	Delete_1DArray(&clipImgBR);
// }

// jpg color image buffer 정보를 채우고 ptt 파일로 저장(fullPath)한다.
// bisMixImage == TRUE 일때 Bottom red, blue image 를 mixing 하여 color 로 만든다.
// SHKang 2017/07/20 : retColorImg 는 nWidthStep 이 반영된 메모리로 변경한다.
void CMNgManager::SaveDispImage(CString fullPath, BOOL zmapUse, UCHAR* retColorImg, int nWIdthStep, int exportSaveOption, BOOL bIsSideBtm, BOOL bIsMixImage)
{
	if(!m_pInspBoardInfo)
		return;
	int roiSizeX = m_pInspBoardInfo->partImgBuf.nImageSizeX;
	int roiSizeY = m_pInspBoardInfo->partImgBuf.nImageSizeY;
	int roiArea = (int)(roiSizeX * roiSizeY);
	RECT roi = {0, 0, m_pInspBoardInfo->partImgBuf.nImageSizeX, m_pInspBoardInfo->partImgBuf.nImageSizeY};
	UCHAR* imgR = m_pInspBoardInfo->partImgBuf.imgTop_R;
	UCHAR* imgG = m_pInspBoardInfo->partImgBuf.imgTop_G;
	UCHAR* imgB = m_pInspBoardInfo->partImgBuf.imgTop_B;
// 	if(bIsSideBtm)
// 	{
// 		imgR = m_pInspBoardInfo->partImgBuf.imgSide1_R;
// 		imgG = m_pInspBoardInfo->partImgBuf.imgSide1_G;
// 		imgB = m_pInspBoardInfo->partImgBuf.imgSide1_B;
// 	}
// 	else if(g_pMPTI->m_nCameraType == (int)Basler_Color)
// 	{
// 		imgR = m_pInspBoardInfo->partImgBuf.imgTop_W;	// Top R
// 		imgG = m_pInspBoardInfo->partImgBuf.imgMiddle_R;	// Top G
// 		imgB = m_pInspBoardInfo->partImgBuf.imgMiddle_B;	// Top B
// 	}

	float* img32 = NULL;
	//img32 = new float[roiArea];
	img32 = g_pMManager->pem_new<float>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);

	int nCompositeMode = 0;// g_pMPTI->GetCompositeLightMode();

	if(nCompositeMode == 0)
	{
	//	if(g_pMPTI->IsMachineTypeUV() == FALSE)
		{
			alpf_make_img24n32f(roi, roiSizeX, imgR, imgG, imgB, m_ngParam.redFator, m_ngParam.greenFator, m_ngParam.blueFator, roiSizeX, img32, retColorImg, nWIdthStep); // SHKang 2017/07/20 : dst image 의 widthStep 을 넣는다.
		}
// 		else	// for UV
// 		{
// 			if(m_nSave_ImgType == 0)//if(!bIsMixImage)
// 			{
// 				alpf_make_img24(roi, roiSizeX, imgR, imgG, imgB, m_ngParam.redFator, m_ngParam.greenFator, m_ngParam.blueFator, nWIdthStep, retColorImg, nWIdthStep); // SHKang 2017/07/20 : dst image 의 widthStep 을 넣는다.
// 			}
// 			else if(m_nSave_ImgType == 1)
// 			{
// 				// 	g_pMPTI->m_factorB
// 				float factorUVR;
// 				float factorUVG;
// 				float factorUVB;
// 				g_pMPTI->GetUVFactorRGB(&factorUVR, &factorUVG, &factorUVB);
// 
// 				UCHAR* imgBUV = m_pInspBoardInfo->partImgBuf.imgBottom_B;
// 				alpf_make_Miximg24_S4(roi, roiSizeX, imgR, imgG, imgB, imgBUV, factorUVR, factorUVG, factorUVB, nWIdthStep, retColorImg, nWIdthStep); // SHKang 2017/07/20 : dst image 의 widthStep 을 넣는다.
// 			}
// 			else if(m_nSave_ImgType == 2)
// 			{
// 				float factorUVR;
// 				float factorUVG;
// 				float factorUVB;
// 				g_pMPTI->GetUVFactorRGB(&factorUVR, &factorUVG, &factorUVB);
// 
// 				UCHAR* imgBUV = m_pInspBoardInfo->partImgBuf.imgBottom_B;
// 				alpf_make_img24(roi, roiSizeX, imgR, imgR, imgR, 1, 1, 1, nWIdthStep, retColorImg, nWIdthStep); // SHKang 2017/07/20 : dst image 의 widthStep 을 넣는다.
// 			}
// 		}
	}
	else
	{
		UCHAR* imgBR = m_pInspBoardInfo->partImgBuf.imgBottom_R;
		UCHAR* imgBB = m_pInspBoardInfo->partImgBuf.imgBottom_B;

		alpf_make_img24n32f_CompositeBtm(roi, roiSizeX, imgR, imgG, imgB, imgBR, imgBB, m_ngParam.redFator, m_ngParam.greenFator, m_ngParam.blueFator, m_ngParam.btmRedfactor, m_ngParam.btmBluefactor,
			g_pMPTI->m_fCompoBtmR, g_pMPTI->m_fCompoBtmG, g_pMPTI->m_fCompoBtmB, roiSizeX, img32, retColorImg, nWIdthStep); // SHKang 2017/07/20 : dst image 의 widthStep 을 넣는다.
	}

	//save zmap (ptt)
// 	if(zmapUse == TRUE && g_pMPTI->IsMachineTypeUV() == FALSE)
// 	{		
		HEADER_PTT hd;

		hd.uiNumRow = m_pInspBoardInfo->partZmapData.zmapSizeX;
		hd.uiNumCol = m_pInspBoardInfo->partZmapData.zmapSizeY;
		hd.fNumRow = (float)m_pInspBoardInfo->partZmapData.zmapSizeX;
		hd.fNumCol = (float)m_pInspBoardInfo->partZmapData.zmapSizeY;
		hd.pixelResX = (float)_mm2micron(m_resolX);
		hd.pixelResY = (float)_mm2micron(m_resolY);
		hd.zResolX = (float)_mm2micron(m_resolX);
		hd.zResolY = (float)_mm2micron(m_resolY);
		Ipp32f fMin = 0.0f, fMax = 0.0f;
		ippsMinMax_32f(m_pInspBoardInfo->partZmapData.data, (int)m_pInspBoardInfo->partZmapData.zmapSizeX * (int)m_pInspBoardInfo->partZmapData.zmapSizeY, &fMin, &fMax);
		if (exportSaveOption == 0)
			hd.sizeBit = (fMax >= 32000) ? 32 : 16;
		else if (exportSaveOption == 1)
			hd.sizeBit = 32;
		//hd.sizeBit = 32 ;
		alpf_save_ptt(fullPath, &hd, m_pInspBoardInfo->partZmapData.data, img32);		
	/*}*/
	
	//delete[] img32;
	g_pMManager->pem_delete(img32, true);
	img32 = NULL;
}
void CMNgManager::SaveDispImage_Multi(CString fullPath, BOOL zmapUse, UCHAR* retColorImg, int nWIdthStep, int exportSaveOption, BOOL bIsSideBtm, BOOL bIsMixImage , RoiColorBuf partImgColorBuf, InspRoiImgBuf partImgBuf, ZmapData partZmapData)
{
	if (!m_pInspBoardInfo)
		return;
	int roiSizeX = partImgBuf.nImageSizeX;
	int roiSizeY = partImgBuf.nImageSizeY;
	int roiArea = (int)(roiSizeX * roiSizeY);
	RECT roi = { 0, 0, partImgBuf.nImageSizeX, partImgBuf.nImageSizeY };
	UCHAR* imgR = partImgBuf.imgTop_R;
	UCHAR* imgG = partImgBuf.imgTop_G;
	UCHAR* imgB = partImgBuf.imgTop_B;

	float* img32 = NULL;
	img32 = g_pMManager->pem_new<float>(true, roiArea, (PCHAR)__FUNCTION__, __LINE__);

	int nCompositeMode = 0;// g_pMPTI->GetCompositeLightMode();

	if (nCompositeMode == 0)
	{
		{
			alpf_make_img24n32f(roi, roiSizeX, imgR, imgG, imgB, m_ngParam.redFator, m_ngParam.greenFator, m_ngParam.blueFator, roiSizeX, img32, retColorImg, nWIdthStep); // SHKang 2017/07/20 : dst image 의 widthStep 을 넣는다.
		}

	}
	else
	{
		UCHAR* imgBR = partImgBuf.imgBottom_R;
		UCHAR* imgBB = partImgBuf.imgBottom_B;

		alpf_make_img24n32f_CompositeBtm(roi, roiSizeX, imgR, imgG, imgB, imgBR, imgBB, m_ngParam.redFator, m_ngParam.greenFator, m_ngParam.blueFator, m_ngParam.btmRedfactor, m_ngParam.btmBluefactor,
			g_pMPTI->m_fCompoBtmR, g_pMPTI->m_fCompoBtmG, g_pMPTI->m_fCompoBtmB, roiSizeX, img32, retColorImg, nWIdthStep); // SHKang 2017/07/20 : dst image 의 widthStep 을 넣는다.
	}

	//save zmap (ptt)
// 	if(zmapUse == TRUE && g_pMPTI->IsMachineTypeUV() == FALSE)
// 	{		
	HEADER_PTT hd;

	hd.uiNumRow = partZmapData.zmapSizeX;
	hd.uiNumCol = partZmapData.zmapSizeY;
	hd.fNumRow = (float)partZmapData.zmapSizeX;
	hd.fNumCol = (float)partZmapData.zmapSizeY;
	hd.pixelResX = (float)_mm2micron(m_resolX);
	hd.pixelResY = (float)_mm2micron(m_resolY);
	hd.zResolX = (float)_mm2micron(m_resolX);
	hd.zResolY = (float)_mm2micron(m_resolY);
	Ipp32f fMin = 0.0f, fMax = 0.0f;
	ippsMinMax_32f(partZmapData.data, (int)partZmapData.zmapSizeX * (int)partZmapData.zmapSizeY, &fMin, &fMax);
	if (exportSaveOption == 0)
		hd.sizeBit = (fMax >= 32000) ? 32 : 16;
	else if (exportSaveOption == 1)
		hd.sizeBit = 32;
	//hd.sizeBit = 32 ;
	alpf_save_ptt(fullPath, &hd, partZmapData.data, img32);
	/*}*/

	//delete[] img32;
	g_pMManager->pem_delete(img32, true);
	img32 = NULL;
}
void CMNgManager::SaveColorMap2UseImage(CString fullPath, int nWidthStep)
{
	if (!m_pInspBoardInfo)
		return;

	int roiSizeX = 0;
	int roiSizeY = 0;
	int widthStep = 0;
	int roiArea = GetROISize(roiSizeX, roiSizeY, widthStep);
	roiArea = widthStep * roiSizeY;

	CString strAngleColorFileName = _T("");
	strAngleColorFileName.Format(_T("%s_AC.jpg"), fullPath);
	UCHAR * ucAngleColor = g_pMManager->pem_new<UCHAR>(true, roiArea * 3, (PCHAR)__FUNCTION__, __LINE__);

	UCHAR* imgAngleR = m_pInspBoardInfo->partImgColorBuf.img_R;
	UCHAR* imgAngleG = m_pInspBoardInfo->partImgColorBuf.img_G;
	UCHAR* imgAngleB = m_pInspBoardInfo->partImgColorBuf.img_B;
	int nBufroiSizeX = m_pInspBoardInfo->partImgBuf.nImageSizeX;
	int nBufroiSizeY = m_pInspBoardInfo->partImgBuf.nImageSizeY;
	for (int y = 0; y < nBufroiSizeY; y++)
	{
		for (int x = 0; x < nBufroiSizeX; x++)
		{
			int nIdx = (y * nBufroiSizeX) + x;
			// jpg 를 위한 24 bit
			int nIndex = y * nWidthStep + x;
			int idxRoiDst = nIndex * 3;
			ucAngleColor[idxRoiDst] = imgAngleB[nIdx];
			ucAngleColor[idxRoiDst + 1] = imgAngleG[nIdx];
			ucAngleColor[idxRoiDst + 2] = imgAngleR[nIdx];
		}
	}
	Save_JpegImage(ucAngleColor, roiSizeX, roiSizeY, 3, IC_BGR, strAngleColorFileName, 75);
}
#if 0
void CMNgManager::ProcSaveNgImg(CString path, CString moduleName, CString partName, CRect roi, double angle, BOOL isLeadNg, CRect* leadNgRect, int leadNgCount, CRect* solderNgRect, int solderNgCount, CRect* gapNgRect, int gapNgCount, CRect* leadCountNgRect, int leadCountNgCount)
{
	CString fileName = _T("");
	CString fullPath = _T("");
	BOOL flag = FALSE;

	int lightCnt = 10;
	int* lightTemp = new int[lightCnt];
	memset(lightTemp, 0, sizeof(int) * lightCnt);

	if(m_defectCode != eDFT_NOTHING_DEFECT)
	{
		for(int i = 0; i < m_nParamArraySize; i++)
		{
			int lightType = m_pParamArray[i].lightType_main;			
			void* srcMilImg = m_pParamArray[i].targetImg;

			CString wndName = _T("");
			wndName.Format(_T("window%d"),  m_pParamArray[i].wndIndex);

			if(lightType < lightCnt && lightTemp[lightType] == 0)
			{
				fileName = MakeFilename_Ng(moduleName, partName, wndName, lightType);	
				fullPath.Format(_T("%s\\%s"), path, fileName);
				CreateDir(path);

				//SaveNgImage(srcMilImg, roi, fullPath);
				//////////////////////////////////////////////////////////////////////////
				int sizeX = (int)(roi.right - roi.left);
				int sizeY = (int)(roi.bottom - roi.top);
				int roiArea = (int)(sizeX * sizeY);
				CPoint center = roi.CenterPoint();
				UCHAR* userSrcImg = new UCHAR[sizeX * sizeY];

				m_milProc->GetClipBuff(srcMilImg, userSrcImg, center.x, center.y, sizeX, sizeY);
				m_milProc->SaveWorkImg(userSrcImg, sizeX, sizeY, _T("test.bmp"));
				UCHAR* procImg = NULL;
				double wndAngle = angle;
				BOOL isAnyAngle = IsAnyAngle(wndAngle);
				int w = sizeX;
				int h = sizeY;
				int marginX = 0;
				int marginY = 0;
				if(isAnyAngle == TRUE)
				{
					double rotAngle = CalcRotateAngle(wndAngle);		
					m_milProc->ProcAnyAngle_Img(userSrcImg, &procImg, sizeX, sizeY, rotAngle, 1, &w, &h);

					marginX = (int)((w - sizeX) / 2.0);
					marginY = (int)((h - sizeY) / 2.0);
				}
				else
				{
					procImg = userSrcImg;
					marginX = 0;
					marginY = 0;
				}

				UCHAR* rstColorImg = NULL;
				if(isLeadNg == TRUE)
				{				
					rstColorImg = new UCHAR[w * h * 3];

					if(leadCountNgRect != NULL && leadCountNgCount > 0)
						m_milProc->DrawLeadNgRect(procImg, rstColorImg, w, h, 1, roi, leadCountNgRect, leadCountNgCount, marginX, marginY, 1);

					if(leadNgRect != NULL && leadNgCount > 0)
						m_milProc->DrawLeadNgRect(procImg, rstColorImg, w, h, 1, roi, leadNgRect, leadNgCount, marginX, marginY, 1);

					if(solderNgRect != NULL && solderNgCount > 0)
						m_milProc->DrawLeadNgRect(procImg, rstColorImg, w, h, 1, roi, solderNgRect, solderNgCount, marginX, marginY, 1);

					if(gapNgRect != NULL && gapNgCount > 0)
						m_milProc->DrawLeadNgRect(procImg, rstColorImg, w, h, 1, roi, gapNgRect, gapNgCount, marginX, marginY, 1);	
				
				}
				else
				{  
					rstColorImg = procImg;
				}


				if(isAnyAngle == TRUE)
				{
					UCHAR* anyAngleColorImg = NULL;
					UCHAR* anyAngleColorImg_Temp = NULL;
					m_milProc->ProcAnyAngle_Img(rstColorImg, &anyAngleColorImg_Temp, w, h, wndAngle, 3, &w, &h);
					//m_milProc->SaveWorkImg(anyAngleColorImg_Temp, w, h, _T("test3.bmp"), 3);

					int tempCx = (int)(w / 2.0);
					int tempCy = (int)(h / 2.0);

					anyAngleColorImg = new UCHAR[roiArea * 3];
					m_milProc->GetClipImage(anyAngleColorImg_Temp, w, h, anyAngleColorImg, tempCx, tempCy, sizeX, sizeY, 3);
					delete [] anyAngleColorImg_Temp; anyAngleColorImg_Temp = NULL; // SHKang 2017/08/07 : 동적배열 해제.

					if(isLeadNg == TRUE)
					{
						m_milProc->SaveImage(anyAngleColorImg, sizeX, sizeY, 3, fullPath + FileType);
					}
					else
					{
						m_milProc->SaveImage(rstColorImg, sizeX, sizeY, 1, fullPath + FileType);
					}


					delete [] anyAngleColorImg; anyAngleColorImg = NULL;	// SHKang 2017/08/07 : 배열메모리 해제부 수정.
				}
				else
				{
					if(isLeadNg == TRUE)		
						m_milProc->SaveImage(rstColorImg, sizeX, sizeY, 3, fullPath + FileType);
					else
						m_milProc->SaveImage(rstColorImg, sizeX, sizeY, 1, fullPath + FileType);
				}
					
				delete [] userSrcImg; userSrcImg = NULL; // SHKang 2017/08/07 : 배열메모리 해제부 수정.

				if(isAnyAngle == TRUE)
				{	delete [] procImg; procImg = NULL; } // SHKang 2017/08/07 : 배열메모리 해제부 수정.

				if(isLeadNg == TRUE)
				{	delete [] rstColorImg; rstColorImg = NULL; } // SHKang 2017/08/07 :배열메모리 해제부 수정.
				//////////////////////////////////////////////////////////////////////////
				lightTemp[lightType]++;
			}
			
			flag = TRUE;

		}
	}

	delete [] lightTemp; lightTemp = NULL; // SHKang 2017/08/07 : 배열메모리 해제부 수정.

}
#endif

CString CMNgManager::MakeDirname()
{
	CString ret = _T("");
	CString modelName = _T("");
	CString day = _T("");
	CString time = _T("");


	modelName = m_pInspBoardInfo->modelName;

	CTime ct;
	ct = CTime::GetCurrentTime();

	day.Format(_T("%04d%02d%02d"), ct.GetYear(), ct.GetMonth(), ct.GetDay());
	time.Format(_T("%02d%02d%02d"), ct.GetHour(), ct.GetMinute(), ct.GetSecond());

	ret.Format(_T("%s\\%s\\%s\\"), modelName, day, time);

	return ret;
}

CString CMNgManager::MakeFilename_Disp(CString moduleName, CString partName)
{
	CString ret = _T("");
	CString InspName = _T("");


//	ret.Format(_T("%s@%s@%s"), moduleName, partName, GetDefectCodeName(defectCode));
	//ret.Format(_T("%s@%s@%d"), moduleName, partName, defectCode);
	ret.Format(_T("%s@%s"), moduleName, partName);

	return ret;
}

CString CMNgManager::MakeFilename_Ng(CString moduleName, CString partName, CString wndName, int lightNum)
{
	CString ret = _T("");
	CString InspName = _T("");

	ret.Format(_T("%s@%s@%s@%d"), moduleName, partName, wndName, lightNum);

	return ret;
}

CRect CMNgManager::GetRectRoi(InspPartParam param)
{
	CRect ret;
	Coordinate coordi;
	coordi = SetCoordinate(m_pInspBoardInfo, param);

	if(param.inspType == eINSP_MOUNT)
	{
		ret.left = (LONG)(coordi.bdrCx - (coordi.bdrWidth / 2));
		ret.right = (LONG)(coordi.bdrCx + (coordi.bdrWidth / 2));
		ret.top = (LONG)(coordi.bdrCy - (coordi.bdrLength / 2));
		ret.bottom = (LONG)(coordi.bdrCy + (coordi.bdrLength / 2));
	}
	else
	{
		ret.left = (LONG)(coordi.cx - (coordi.width / 2));
		ret.right = (LONG)(coordi.cx + (coordi.width / 2));
		ret.top = (LONG)(coordi.cy - (coordi.length / 2));
		ret.bottom = (LONG)(coordi.cy + (coordi.length / 2));
	}

// 	int margin = 20;
// 	ret.left = ret.left - (margin / 2);
// 	ret.right = ret.right + (margin / 2);
// 	ret.top= ret.top - (margin / 2);
// 	ret.bottom = ret.bottom + (margin / 2);


	return ret;
}

BOOL CMNgManager::IsInMount(int* retIndex)
{
	BOOL ret = FALSE;
	
	int index = 0;
	for(int i = 0; i < m_nParamArraySize; i++)
	{
		if(m_pParamArray[i].inspType == eINSP_MOUNT)
		{
			ret = TRUE;
			index = i;
		}
	}

	if(retIndex != NULL)
		*retIndex = index;

	return ret;
}

CRect CMNgManager::GetRectRoi()
{
	CRect ret;
	double rotAngle = m_pInspBoardInfo->angle;
	BOOL isAnyAngle = IsAnyAngle(rotAngle);
	if(isAnyAngle)
		return GetRectBoundaryRoi();

	double TempWidh = 0;
	double TempHeight = 0;
	double tempCx = 0;
	double tempCy = 0;

	BOOL flag = FALSE;//IsInMount(&index);

	if(flag)
	{
		int index = 0;
		TempWidh = m_pParamArray[index].bdrWidth;
		TempHeight =  m_pParamArray[index].bdrLength;
		tempCx =  m_pParamArray[index].bdrCx;
		tempCy =  m_pParamArray[index].bdrCy;
	}
	else
	{
		TempWidh = m_pInspBoardInfo->partWidth;
		TempHeight = m_pInspBoardInfo->partHeight;
		tempCx = m_pInspBoardInfo->partCx;
		tempCy = m_pInspBoardInfo->partCy;
	}


	int width = (int)(float)(TempWidh / m_resolX + 0.00001);		// 임시 (좌표 round로 통일 시키기 전에..)
	int height = (int)(float)(TempHeight / m_resolY + 0.00001);
	POINT ct =  CvtBoradToPixel(m_pInspBoardInfo->fovCx, m_pInspBoardInfo->fovCy, tempCx, tempCy, tempCx, tempCy, m_pInspBoardInfo);

	ret.left = (LONG)(ct.x - (width / 2));
	ret.top = (LONG)(ct.y - (height / 2));
	ret.right = (LONG)(ret.left + width);
	ret.bottom = (LONG)(ret.top + height);

	return ret;
}

CRect CMNgManager::GetRectBoundaryRoi()
{
	CRect ret;
	double TempWidh = 0;
	double TempHeight = 0;
	double tempCx = 0;
	double tempCy = 0;

	//int index = 0;
	//for(int i = 0; i < m_nParamArraySize; i++)
	//{
	//
	//	if(m_pParamArray != NULL && m_pParamArray[i].inspType == eINSP_LEADSOLDER)
	//	{
	//		index = i;
	//		break;
	//	}
	//}

	TempWidh = m_pInspBoardInfo->anyAngleWidth;
	TempHeight =  m_pInspBoardInfo->anyAngleLength;
	tempCx =  m_pInspBoardInfo->anyAngleCx;
	tempCy =  m_pInspBoardInfo->anyAngleCy;

	int width = (int)((float)TempWidh / m_resolX);
	int height = (int)((float)TempHeight / m_resolY);
	POINT ct =  CvtBoradToPixel(m_pInspBoardInfo->fovCx, m_pInspBoardInfo->fovCy, tempCx, tempCy, tempCx, tempCy, m_pInspBoardInfo);

	ret.left = (LONG)(ct.x - (width / 2));
	ret.right = (LONG)(ct.x + (width / 2));
	ret.top = (LONG)(ct.y - (height / 2));
	ret.bottom = (LONG)(ct.y + (height / 2));

	return ret;
}

CRect CMNgManager::GetRectRoi_3D()
{
	CRect ret;
	double TempWidh = 0;
	double TempHeight = 0;
	double tempCx = 0;
	double tempCy = 0;

	int index = 0;
	BOOL flag = IsInMount(&index);
	if(flag)
	{
		TempWidh = m_pParamArray[index].bdrWidth;
		TempHeight =  m_pParamArray[index].bdrLength;
		tempCx =  m_pParamArray[index].bdrCx;
		tempCy =  m_pParamArray[index].bdrCy;
	}
	else
	{
		TempWidh = m_pInspBoardInfo->partWidth;
		TempHeight = m_pInspBoardInfo->partHeight;
		tempCx = m_pInspBoardInfo->partCx;
		tempCy = m_pInspBoardInfo->partCy;
	}

	int width = (int)((float)(TempWidh + (m_roiMargin * 2.0)) / m_resolX);
	int height = (int)((float)(TempHeight + (m_roiMargin * 2.0)) / m_resolY);
	POINT ct =  CvtBoradToPixel(m_pInspBoardInfo->fovCx, m_pInspBoardInfo->fovCy, tempCx, tempCy, tempCx, tempCy, m_pInspBoardInfo);

	ret.left = (LONG)(ct.x - (width / 2));
	ret.right = (LONG)(ct.x + (width / 2));
	ret.top = (LONG)(ct.y - (height / 2));
	ret.bottom = (LONG)(ct.y + (height / 2));

	return ret;
}

void CMNgManager::CreateDir(CString Path)
{
	CString strPath = Path;
	CString strTemp[20];
	CString strFile;
	int nLength = strPath.GetLength();
	int j = 0;

	for(int i = 0 ; i<20 ; i++)
		strTemp[i].Empty();


	for(int i = 0; i < nLength; i++)
	{
		if(strPath.Mid(i,1) == '\\')
		{
			j++;
		}
		strTemp[j] = strTemp[j] + strPath.Mid(i,1);

	}

	strFile = strTemp[0];
	int i = 1;

	while(strTemp[i] != "")
	{
		strFile = strFile+strTemp[i];
		CreateDirectory(strFile,NULL);
		i++;
	}
}

// SHKang 2017/07/06
// SHKang 2017/07/20 : widthStep 으로 넘겨준다.
int CMNgManager::GetROISize(int & nSizeX, int & nSizeY, int & nWidthStep)
{
	int roiArea = 0;
	CRect roi_3D;

	roi_3D = GetRectRoi();

	int roiSizeX = (int)(roi_3D.right - roi_3D.left);
	int roiSizeY = (int)(roi_3D.bottom - roi_3D.top);

	if(m_pInspBoardInfo->partImgBuf.nImageSizeX != roiSizeX)
	{
		//m_pInspBoardInfo->partImgBuf.nImageSizeX = roiSizeX;
		roi_3D.right = roi_3D.left + m_pInspBoardInfo->partImgBuf.nImageSizeX;
		roiSizeX = m_pInspBoardInfo->partImgBuf.nImageSizeX;
	}
	if(m_pInspBoardInfo->partImgBuf.nImageSizeY != roiSizeY)
	{
		roi_3D.bottom = roi_3D.top + m_pInspBoardInfo->partImgBuf.nImageSizeY;
		roiSizeY = m_pInspBoardInfo->partImgBuf.nImageSizeY;
	}

	//Image 저장시에는 Align 필요
	int nStepTemp = 4 - roiSizeX % 4;
	//if(roiSizeX % 4 == 0) nStepTemp = 0;
	//nWidthStep = roiSizeX + nStepTemp;

	nWidthStep = g_pMPTI->nCalcWidthStep(true, roiSizeX);

	nSizeX = roiSizeX; 
	nSizeY = roiSizeY;

	roiArea = nSizeX * nSizeY;
	return roiArea;
}

// SHKang 2017/03/13
enum { eSaveShared_3d = 0x01, eSaveShared_Jpg = 0x02, eSaveCaption_Jpg = 0x04 };
int CMNgManager::SaveInspPartImage(SaveImgParamData tgParam, UCHAR * ucColorImg, bool bImageSave, UCHAR * ucColorImg_AC, bool bLeadOK, bool bSaveUserLight, int exportSaveOption, InspPartInfo* pInsp, InspectionResult* inspectionResult)
{
	m_nSave_ImgType = tgParam.m_nSave_ImgType;
	int ret = 0;
	if (bImageSave == false)
		tgParam.nSave_2d = 0;
	if (tgParam.nSave_3d == 0 && tgParam.nSave_2d == 0 && tgParam.nSave_put == 0 && tgParam.nMSPtImg == 0 && bLeadOK == false && tgParam.m_nSave_Color == 0)
		return ret;

	InspPartInfo* pBoard = NULL;
	if (pInsp)
		pBoard = pInsp;
	else if (m_pInspBoardInfo)
		pBoard = m_pInspBoardInfo;
	if (pBoard == NULL)
		return ret;
	if (tgParam.nSave_3d != 0) CreateDir(tgParam.arr3dFilePath);

	int roiSizeX = 0;
	int roiSizeY = 0;
	int widthStep = 0;
	int roiArea = GetROISize(roiSizeX, roiSizeY,widthStep);
	roiArea = widthStep * roiSizeY;

	UCHAR* colorImg = ucColorImg;
	bool bOutputColorImg = true;
	if(colorImg==nullptr)
	{
		//colorImg = new UCHAR[roiArea * 3];
		colorImg = g_pMManager->pem_new<UCHAR>(true, roiArea * 3, (PCHAR)__FUNCTION__, __LINE__);
		bOutputColorImg = false;
	}

	// SHKang 2018/03/02
	UCHAR * ucAngleColor = nullptr;	

	CString str3dLeadOKPartFullName;
	str3dLeadOKPartFullName.Format(_T("%s\\%s"), tgParam.arr3dLeadOKFilePath, tgParam.arr3dLeadOKFileName);

	CString str3dFullName;
	str3dFullName.Format(_T("%s\\%s"), tgParam.arr3dFilePath, tgParam.arr3dFileName);
	
	CString strLinkageFullName;
	strLinkageFullName.Format(_T("%s\\%s"), tgParam.arrAOILinkFilePath, tgParam.arrAOILinkFileName);

	CString strNextMCFullName;
	strNextMCFullName.Format(_T("%s\\%s"), tgParam.arrNextMCFilePath, tgParam.arrNextMCFileName);

	CString str2dMESFullName = _T("");
	str2dMESFullName.Format(_T("%s\\%s"), tgParam.arrMesFilePath, tgParam.arrMesFileName);

	CString str2dAPCFullName = _T("");
	str2dAPCFullName.Format(_T("%s\\%s"), tgParam.arrAPCFilePath, tgParam.arrAPCFileName);

	CString str3dRawFullName;
	str3dRawFullName.Format(_T("%s\\%s"), tgParam.arr3dRawFilePath, tgParam.arr3dRawFileName);

	CString str3dLeadFullFileName = _T("");
	if (bLeadOK == true)
	{
		CreateDir(tgParam.arr3dLeadOKFilePath);
		str3dLeadFullFileName.Format(_T("%s.ptt"), str3dLeadOKPartFullName);
	}
	CString str2dFullFileName = _T("");
	CString strMSPtImgFullFileName = _T("");
	CString str3dFullFileName = _T("");
	CString strAngleColorFileName = _T("");
	CString str3dRawFullFileName = _T("");
	{
		if(tgParam.nSave_3d != 0) 
		{
			CreateDir(tgParam.arr3dFilePath);
			str3dFullFileName.Format(_T("%s.ptt"), str3dFullName);

			if(tgParam.m_nSave_Color == 1)
			{
				//ucAngleColor = new UCHAR[roiArea * 3];
				ucAngleColor = g_pMManager->pem_new<UCHAR>(true, roiArea * 3, (PCHAR)__FUNCTION__, __LINE__);
				strAngleColorFileName.Format(_T("%s_AC.jpg"), str3dFullName);	// 이름은 나중에 바꾸자.
			}
		}
		else if(tgParam.nSave_AOILink == 1)
		{
			CreateDir(tgParam.arrAOILinkFilePath);
			str3dFullFileName.Format(_T("%s.ptt"), strLinkageFullName);
		}
		
		if(tgParam.nSave_2d == 1)
		{
			CreateDir(tgParam.arrJpgFilePath);
			str2dFullFileName.Format(_T("%s\\%s.jpg"), tgParam.arrJpgFilePath, tgParam.arrJpgFileName);
		}
		else if(tgParam.nSave_2d == 2)
		{
			CString str2dPath = tgParam.arr3dFilePath;
			CreateDir(str2dPath);
			str2dFullFileName.Format(_T("%s.jpg"), str3dFullName);
		}
		else if(tgParam.nSave_AOILink == 2)
		{
			CreateDir(tgParam.arrAOILinkFilePath);
			str2dFullFileName.Format(_T("%s.jpg"), strLinkageFullName);
		}
		else if(tgParam.nSave_NextMC == 2)
		{
			CreateDir(tgParam.arrNextMCFilePath);
			str2dFullFileName.Format(_T("%s.jpg"), strNextMCFullName);
		}
		else if(tgParam.m_nSave_MES != 0)
		{
			CreateDir(tgParam.arrMesFilePath);
			str2dFullFileName.Format(_T("%s.jpg"), str2dMESFullName);
		}
		else if(tgParam.m_nSave_APC != 0)
		{
			CreateDir(tgParam.arrAPCFilePath);
			str2dFullFileName.Format(_T("%s.png"), str2dAPCFullName);
		}
		if (tgParam.nMSPtImg == 1 && tgParam.nSave_2d == 0)
		{
			CreateDir(tgParam.arrJpgFilePath);
			strMSPtImgFullFileName.Format(_T("%s\\%s.jpg"), tgParam.arrJpgFilePath, tgParam.arrJpgFileName);
		}
		BOOL bZmapFlag = FALSE;
		BOOL bIsSideBtm = FALSE;
		if(tgParam.pArrSideImg && tgParam.pArrSideImg[0] != 0 /*&& g_pMPTI->GetDiffMachineType() != DiffMachineUV*/)
		{
			bIsSideBtm = TRUE;
		}

		if(str3dFullFileName != "") bZmapFlag = TRUE;
		if (str3dLeadFullFileName != "") bZmapFlag = TRUE;
		if (bLeadOK == true)//if(tgParam.unNGType == 100 && bLeadOK == true)
			SaveInspPartImage_SharedMemory(str3dLeadFullFileName, colorImg, ucAngleColor, widthStep, bZmapFlag);
		if ((tgParam.unSaveOption & eSaveShared_3d) == eSaveShared_3d)
			ret = SaveInspPartImage_SharedMemory(str3dFullFileName, colorImg, ucAngleColor, widthStep, bZmapFlag);
		else
			SaveDispImage(str3dFullFileName, bZmapFlag, colorImg, widthStep, exportSaveOption, bIsSideBtm, false/*g_pMPTI->GetDiffMachineType() == DiffMachineUV*/);

		int nWidth=0, nHeight=0;
		CreateJpgBuffer(colorImg, widthStep, roiSizeY, nWidth, nHeight, &CopyPVImage);

		if (str2dFullFileName != "")
		{
			if (bIsSideBtm == FALSE && bSaveUserLight == true)
				Check2DImage(colorImg, widthStep, roiSizeY);
			Save_JpegImage(colorImg, roiSizeX, roiSizeY, 3, IC_BGR, str2dFullFileName, tgParam.m_n2DSaveQuality);
		}
		if (strMSPtImgFullFileName != "")
			Save_JpegImage(colorImg, roiSizeX, roiSizeY, 3, IC_BGR, strMSPtImgFullFileName, 75);

		if (ucColorImg && (tgParam.m_nColorCheck & 0x01) == 0x01)
		{
			memcpy(ucColorImg, colorImg, roiArea * 3 * sizeof(UCHAR));
		}
		if (colorImg && pBoard != NULL && inspectionResult != NULL)
		{
			DefectImgCrop_AngleColor(pBoard, inspectionResult, roiSizeX, roiSizeY, colorImg, widthStep, tgParam.m_nMachineCode, tgParam.ColorMode, tgParam.arrJobPath);
		}
		if (colorImg)
		{
			g_pMManager->pem_delete(colorImg, true);
			colorImg = NULL;
		}

		if (ucAngleColor != nullptr && strAngleColorFileName != "")
		{
			SaveInspPartImage_AngleColor(ucAngleColor, widthStep);
			//if (pBoard != NULL && inspectionResult != NULL)
			//{
			//	DefectImgCrop_AngleColor(pBoard, inspectionResult, roiSizeX, roiSizeY, ucAngleColor, widthStep, tgParam.m_nMachineCode, tgParam.arrJobPath);
			//}
			//else
			{
				Save_JpegImage(ucAngleColor, roiSizeX, roiSizeY, 3, IC_BGR, strAngleColorFileName, 75);
				if (tgParam.nSave_3d != 0 && tgParam.m_nSave_Color != 0 && (tgParam.m_nColorCheck & 0x02) == 0x02)
				{
					if (ucColorImg_AC) memcpy(ucColorImg_AC, ucAngleColor, roiArea * 3 * sizeof(UCHAR));
				}
			}
		}

		if(str3dFullFileName != "")
		{
			if(tgParam.nSave_AOILink == 1)
			{
				CString strFileName = _T("");
				strFileName.Format(_T("%s.ptt"), strLinkageFullName);
				if(str3dFullFileName != strFileName) 
				{
					CreateDir(tgParam.arrAOILinkFilePath);
					CopyFile(str3dFullFileName, strFileName, false);
				}
			}
		}

		if(str2dFullFileName != "")
		{
			if(tgParam.nSave_2d == 2)
			{
				CString strFileName = _T("");
				strFileName.Format(_T("%s.jpg"), str3dFullName);
				if(str2dFullFileName != strFileName)
				{
					CreateDir(tgParam.arr3dFilePath);
					CopyFile(str2dFullFileName, strFileName, false);
				}
			}

			if(tgParam.nSave_AOILink == 2)
			{
				CString strFileName = _T("");
				strFileName.Format(_T("%s.jpg"), strLinkageFullName);
				if(str2dFullFileName != strFileName) 
				{
					CreateDir(tgParam.arrAOILinkFilePath);
					CopyFile(str2dFullFileName, strFileName, false);
				}
			}

			if(tgParam.nSave_NextMC == 2)
			{
				CString strFileName = _T("");
				strFileName.Format(_T("%s.jpg"), strNextMCFullName);
				if(str2dFullFileName != strFileName) 
				{
					CreateDir(tgParam.arrNextMCFilePath);
					CopyFile(str2dFullFileName, strFileName, false);
				}
			}

			if(tgParam.m_nSave_MES != 0)
			{
				CString strFileName = _T("");
				strFileName.Format(_T("%s.jpg"), str2dMESFullName);
				if(str2dFullFileName != strFileName) 
				{
					CreateDir(tgParam.arrMesFilePath);
					CopyFile(str2dFullFileName, strFileName, false);
				}
			}

			if(tgParam.m_nSave_APC != 0)
			{
				CString strFileName = _T("");
				strFileName.Format(_T("%s.png"), str2dAPCFullName);
				if(str2dFullFileName != strFileName) 
				{
					CreateDir(tgParam.arrAPCFilePath);
					CopyFile(str2dFullFileName, strFileName, false);
				}
			}
		}

	}

	if(colorImg && bOutputColorImg==false)
	{
		//delete [] colorImg;
		g_pMManager->pem_delete(colorImg, true);
 		colorImg = NULL;
	}

	if(ucAngleColor != nullptr)
	{
		//delete [] ucAngleColor;
		g_pMManager->pem_delete(ucAngleColor, true);
		ucAngleColor = NULL;
	}

	if(tgParam.pArrSideImg)		// CI put Return 때문에 위로 올림.
	{
		for(int  i = 0; i < 4; i++)
		{
			if(tgParam.pArrSideImg[i] != 0)
				SavePstImage(str3dFullName, roiArea, i);
		}
	}

	//if(tgParam.nSave_3d == 0 && tgParam.nSave_put == 0) return 0; //SHS 2021/04/29 ptt 파일 저장 안할 수도 있어 주석 처리함(bump검사 시는 ptt저장 안함)

	if(tgParam.nSave_put!=0)
	{
		CString strPutFullName;
		strPutFullName.Format(_T("%s\\%s.put"), tgParam.arrPutFilePath, tgParam.arrPutFileName);
		SavePutImage(strPutFullName, roiArea);
		return 0;
	}

	if (tgParam.nSave_3d != 0)
	{
		if (tgParam.unSaveOption & eSaveShared_3d)// if(tgParam.unSaveOption & eSaveShared_3d)
			ret = SavePotImage_SharedMemory(str3dFullName, roiArea);	// pot 는 무조건 SharedMemory 로 저장한다.
		else
			SavePotImage(str3dFullName, roiArea);
	}

	if(tgParam.m_nSave_3DRaw != 0) 
	{
		CreateDir(tgParam.arr3dRawFilePath);
		str3dRawFullFileName.Format(_T("%s.p3d"), str3dRawFullName);

		SaveP3DImage_SharedMemory(str3dRawFullFileName);
	}

	//auto it = g_pMPTI->m_vAIPart2DBuf.find(pBoard->nPartIDOrg);
	if (pBoard->nUseAISegmentation > 0)
	{
		CString sAIPath;
		sAIPath.Format(_T("%s\\%s"), tgParam.arrJpgFilePath, tgParam.arrJpgFileName);
		SaveImage_Segmentation_AI(pBoard, sAIPath);
	}

	//2D Color 영상이 기본값 세팅이 아닐 경우 합성 작업
	//m_pInspBoardInfo->bSync2DLgtPosWithJobConfig
	if (pBoard->bSync2DLgtPosWithJobConfig == true)
	{
		//2D Color 영상이 기본값 세팅이 아닐 경우 합성 작업
		if (g_pMPTI->m_nRedLgtPos_2DImg != (int)eColor2DImg_Light_Top_R
			|| g_pMPTI->m_nGreenLgtPos_2DImg != (int)eColor2DImg_Light_Top_G || g_pMPTI->m_nBlueLgtPos_2DImg != (int)eColor2DImg_Light_Top_B)
		{
			CString sSaveImgPath_2DColor;
			sSaveImgPath_2DColor = str2dFullFileName;
			SaveImage_2DColor(pBoard, sSaveImgPath_2DColor);
		}
	}
	

	// Compress contents to save disk space
	//for (int i = 0; i < 2; i++)
	//{
	//	// get file path
	//	CString sName = _T("");
	//	if (i == 0)
	//		sName = "ptt";
	//	else
	//		sName = "pot";
	//
	//	CString csFile = _T("");
	//	csFile.Format(_T("%s.%s"), str3dFullName, sName);
	//	string sFile = CT2CA(csFile); // CString to string
	//	char chCompressfile[MAX_PATH] = { 0, };
	//	strcpy(chCompressfile, sFile.c_str()); // string to char[]
	//	if (_access(chCompressfile, 0) == 0) // check to exist the file
	//	{
	//		char szCompactCommand[MAX_PATH] = { 0, };
	//		sprintf(szCompactCommand, "compact /c %s", chCompressfile); // compress
	//		WinExec(szCompactCommand, SW_HIDE);
	//	}
	//}

	return ret;
}

enum
{
	eInspPartInfo = 0,
	eSaveImgParamData,
	eMultiStructData,
	eSaveOption,
	eBufferIndex,
	eTupleStructMax

};
int CMNgManager::SaveMultiInspPartImage(std::tuple<InspPartInfo *, SaveImgParamData*, MultiStruct*, bool ,int> TupleSaveImages)
{
	int ret = 0;
	InspPartInfo* pBoard = std::get<eInspPartInfo>(TupleSaveImages);
	SaveImgParamData  tgParam = *std::get<eSaveImgParamData>(TupleSaveImages);
	MultiStruct multiData = *std::get<eMultiStructData>(TupleSaveImages);
	bool bImageSave = std::get<eSaveOption>(TupleSaveImages);
	
	UCHAR * ucColorImg = nullptr; 
	UCHAR * ucColorImg_AC = nullptr;
	bool bLeadOK = false;
	bool bSaveUserLight = false;
	int exportSaveOption = 0;
	


	m_nSave_ImgType = tgParam.m_nSave_ImgType;
	
	if (bImageSave == false)
		tgParam.nSave_2d = 0;
	if (tgParam.nSave_3d == 0 && tgParam.nSave_2d == 0 && tgParam.nSave_put == 0 && tgParam.nMSPtImg == 0 && bLeadOK == false)
		return ret;

	if (tgParam.nSave_3d != 0) CreateDir(tgParam.arr3dFilePath);

	int roiSizeX = multiData.nRoiSizeX;
	int roiSizeY = multiData.nRoiSizeY;
	int widthStep = multiData.nWidthStep;
	int roiArea = multiData.nRoiArea;//GetROISize(roiSizeX, roiSizeY, widthStep);
	roiArea = widthStep * roiSizeY;

	UCHAR* colorImg = ucColorImg;
	bool bOutputColorImg = true;
	if (colorImg == nullptr)
	{
		//colorImg = new UCHAR[roiArea * 3];
		colorImg = g_pMManager->pem_new<UCHAR>(true, roiArea * 3, (PCHAR)__FUNCTION__, __LINE__);
		bOutputColorImg = false;
	}

	// SHKang 2018/03/02
	UCHAR * ucAngleColor = nullptr;

	CString str3dLeadOKPartFullName;
	str3dLeadOKPartFullName.Format(_T("%s\\%s"), tgParam.arr3dLeadOKFilePath, tgParam.arr3dLeadOKFileName);

	CString str3dFullName;
	str3dFullName.Format(_T("%s\\%s"), tgParam.arr3dFilePath, tgParam.arr3dFileName);

	CString strLinkageFullName;
	strLinkageFullName.Format(_T("%s\\%s"), tgParam.arrAOILinkFilePath, tgParam.arrAOILinkFileName);

	CString strNextMCFullName;
	strNextMCFullName.Format(_T("%s\\%s"), tgParam.arrNextMCFilePath, tgParam.arrNextMCFileName);

	CString str2dMESFullName = _T("");
	str2dMESFullName.Format(_T("%s\\%s"), tgParam.arrMesFilePath, tgParam.arrMesFileName);

	CString str2dAPCFullName = _T("");
	str2dAPCFullName.Format(_T("%s\\%s"), tgParam.arrAPCFilePath, tgParam.arrAPCFileName);

	CString str3dRawFullName;
	str3dRawFullName.Format(_T("%s\\%s"), tgParam.arr3dRawFilePath, tgParam.arr3dRawFileName);

	CString str3dLeadFullFileName = _T("");
	if (bLeadOK == true)
	{
		CreateDir(tgParam.arr3dLeadOKFilePath);
		str3dLeadFullFileName.Format(_T("%s.ptt"), str3dLeadOKPartFullName);
	}
	CString str2dFullFileName = _T("");
	CString strMSPtImgFullFileName = _T("");
	CString str3dFullFileName = _T("");
	CString strAngleColorFileName = _T("");
	CString str3dRawFullFileName = _T("");
	{
		if (tgParam.nSave_3d != 0)
		{
			CreateDir(tgParam.arr3dFilePath);
			str3dFullFileName.Format(_T("%s.ptt"), str3dFullName);

			if (tgParam.m_nSave_Color == 1)
			{
				//ucAngleColor = new UCHAR[roiArea * 3];
				ucAngleColor = g_pMManager->pem_new<UCHAR>(true, roiArea * 3, (PCHAR)__FUNCTION__, __LINE__);
				strAngleColorFileName.Format(_T("%s_AC.jpg"), str3dFullName);	// 이름은 나중에 바꾸자.
			}
		}
		else if (tgParam.nSave_AOILink == 1)
		{
			CreateDir(tgParam.arrAOILinkFilePath);
			str3dFullFileName.Format(_T("%s.ptt"), strLinkageFullName);
		}

		if (tgParam.nSave_2d == 1)
		{
			CreateDir(tgParam.arrJpgFilePath);
			str2dFullFileName.Format(_T("%s\\%s.jpg"), tgParam.arrJpgFilePath, tgParam.arrJpgFileName);
		}
		else if (tgParam.nSave_2d == 2)
		{
			CString str2dPath = tgParam.arr3dFilePath;
			CreateDir(str2dPath);
			str2dFullFileName.Format(_T("%s.jpg"), str3dFullName);
		}
		else if (tgParam.nSave_AOILink == 2)
		{
			CreateDir(tgParam.arrAOILinkFilePath);
			str2dFullFileName.Format(_T("%s.jpg"), strLinkageFullName);
		}
		else if (tgParam.nSave_NextMC == 2)
		{
			CreateDir(tgParam.arrNextMCFilePath);
			str2dFullFileName.Format(_T("%s.jpg"), strNextMCFullName);
		}
		else if (tgParam.m_nSave_MES != 0)
		{
			CreateDir(tgParam.arrMesFilePath);
			str2dFullFileName.Format(_T("%s.jpg"), str2dMESFullName);
		}
		else if (tgParam.m_nSave_APC != 0)
		{
			CreateDir(tgParam.arrAPCFilePath);
			str2dFullFileName.Format(_T("%s.png"), str2dAPCFullName);
		}
		if (tgParam.nMSPtImg == 1 && tgParam.nSave_2d == 0)
		{
			CreateDir(tgParam.arrJpgFilePath);
			strMSPtImgFullFileName.Format(_T("%s\\%s.jpg"), tgParam.arrJpgFilePath, tgParam.arrJpgFileName);
		}
		BOOL bZmapFlag = FALSE;
		BOOL bIsSideBtm = FALSE;
		if (tgParam.pArrSideImg && tgParam.pArrSideImg[0] != 0 /*&& g_pMPTI->GetDiffMachineType() != DiffMachineUV*/)
		{
			bIsSideBtm = TRUE;
		}

		if (str3dFullFileName != "") bZmapFlag = TRUE;
		if (str3dLeadFullFileName != "") bZmapFlag = TRUE;
		if (bLeadOK == true)//if(tgParam.unNGType == 100 && bLeadOK == true)
			SaveInspPartImage_SharedMemory_Multi(str3dLeadFullFileName, colorImg, ucAngleColor, widthStep, bZmapFlag, pBoard->partImgColorBuf, pBoard->partImgBuf, pBoard->partZmapData); //RoiColorBuf partImgColorBuf, InspRoiImgBuf partImgBuf,ZmapData partZmapData
		if ((tgParam.unSaveOption & eSaveShared_3d) == eSaveShared_3d)
			ret = SaveInspPartImage_SharedMemory_Multi(str3dFullFileName, colorImg, ucAngleColor, widthStep, bZmapFlag, pBoard->partImgColorBuf, pBoard->partImgBuf, pBoard->partZmapData); //RoiColorBuf partImgColorBuf, InspRoiImgBuf partImgBuf,ZmapData partZmapData
		else
			SaveDispImage_Multi(str3dFullFileName, bZmapFlag, colorImg, widthStep, exportSaveOption, bIsSideBtm, FALSE, pBoard->partImgColorBuf, pBoard->partImgBuf, pBoard->partZmapData);

		int nWidth = 0, nHeight = 0;
		CreateJpgBuffer(colorImg, widthStep, roiSizeY, nWidth, nHeight, &CopyPVImage);

		if (str2dFullFileName != "")
		{
			if (bIsSideBtm == FALSE && bSaveUserLight == true)
				Check2DImage_Multi(colorImg, widthStep, roiSizeY, pBoard->partImgBuf);
			Save_JpegImage(colorImg, roiSizeX, roiSizeY, 3, IC_BGR, str2dFullFileName, tgParam.m_n2DSaveQuality);
		}
		if (strMSPtImgFullFileName != "")
			Save_JpegImage(colorImg, roiSizeX, roiSizeY, 3, IC_BGR, strMSPtImgFullFileName, 75);

		// 		if (ucAngleColor != nullptr && strAngleColorFileName != "")
		// 		{
		// 			SaveInspPartImage_AngleColor(ucAngleColor, widthStep);
		// 			Save_JpegImage(ucAngleColor, roiSizeX, roiSizeY, 3, IC_BGR, strAngleColorFileName, 75);
		// 			if (ucColorImg_AC) memcpy(ucColorImg_AC, ucAngleColor, roiArea * 3 * sizeof(UCHAR));
		// 		}

		if (str3dFullFileName != "")
		{
			if (tgParam.nSave_AOILink == 1)
			{
				CString strFileName = _T("");
				strFileName.Format(_T("%s.ptt"), strLinkageFullName);
				if (str3dFullFileName != strFileName)
				{
					CreateDir(tgParam.arrAOILinkFilePath);
					CopyFile(str3dFullFileName, strFileName, false);
				}
			}
		}

		if (str2dFullFileName != "")
		{
			if (tgParam.nSave_2d == 2)
			{
				CString strFileName = _T("");
				strFileName.Format(_T("%s.jpg"), str3dFullName);
				if (str2dFullFileName != strFileName)
				{
					CreateDir(tgParam.arr3dFilePath);
					CopyFile(str2dFullFileName, strFileName, false);
				}
			}

			if (tgParam.nSave_AOILink == 2)
			{
				CString strFileName = _T("");
				strFileName.Format(_T("%s.jpg"), strLinkageFullName);
				if (str2dFullFileName != strFileName)
				{
					CreateDir(tgParam.arrAOILinkFilePath);
					CopyFile(str2dFullFileName, strFileName, false);
				}
			}

			if (tgParam.nSave_NextMC == 2)
			{
				CString strFileName = _T("");
				strFileName.Format(_T("%s.jpg"), strNextMCFullName);
				if (str2dFullFileName != strFileName)
				{
					CreateDir(tgParam.arrNextMCFilePath);
					CopyFile(str2dFullFileName, strFileName, false);
				}
			}

			if (tgParam.m_nSave_MES != 0)
			{
				CString strFileName = _T("");
				strFileName.Format(_T("%s.jpg"), str2dMESFullName);
				if (str2dFullFileName != strFileName)
				{
					CreateDir(tgParam.arrMesFilePath);
					CopyFile(str2dFullFileName, strFileName, false);
				}
			}

			if (tgParam.m_nSave_APC != 0)
			{
				CString strFileName = _T("");
				strFileName.Format(_T("%s.png"), str2dAPCFullName);
				if (str2dFullFileName != strFileName)
				{
					CreateDir(tgParam.arrAPCFilePath);
					CopyFile(str2dFullFileName, strFileName, false);
				}
			}
		}

	}

	if (colorImg && bOutputColorImg == false)
	{
		//delete [] colorImg;
		g_pMManager->pem_delete(colorImg, true);
		colorImg = NULL;
	}

	if (ucAngleColor != nullptr)
	{
		//delete [] ucAngleColor;
		g_pMManager->pem_delete(ucAngleColor, true);
		ucAngleColor = NULL;
	}

	if (tgParam.pArrSideImg)		// CI put Return 때문에 위로 올림.
	{
		for (int i = 0; i < 4; i++)
		{
			if (tgParam.pArrSideImg[i] != 0)
				SavePstImage_Multi(str3dFullName, roiArea, i, pBoard->partImgBuf); // InspRoiImgBuf partImgBuf
		}
	}

	//if(tgParam.nSave_3d == 0 && tgParam.nSave_put == 0) return 0; //SHS 2021/04/29 ptt 파일 저장 안할 수도 있어 주석 처리함(bump검사 시는 ptt저장 안함)

	if (tgParam.nSave_put != 0)
	{
		CString strPutFullName;
		strPutFullName.Format(_T("%s\\%s.put"), tgParam.arrPutFilePath, tgParam.arrPutFileName);
		SavePutImage_Multi(strPutFullName, roiArea, pBoard->partImgBuf); // InspRoiImgBuf partImgBuf
		return 0;
	}

	if (tgParam.nSave_3d != 0)
	{
		if (tgParam.unSaveOption & eSaveShared_3d)// if(tgParam.unSaveOption & eSaveShared_3d)
			ret = SavePotImage_SharedMemory_Multi(str3dFullName, roiArea, pBoard->partImgBuf);	//InspRoiImgBuf partImgBuf// pot 는 무조건 SharedMemory 로 저장한다.
		else
			SavePotImage_Multi(str3dFullName, roiArea, pBoard->partImgBuf); // InspRoiImgBuf partImgBuf
	}

	if (tgParam.m_nSave_3DRaw != 0)
	{
		CreateDir(tgParam.arr3dRawFilePath);
		str3dRawFullFileName.Format(_T("%s.p3d"), str3dRawFullName);

		SaveP3DImage_SharedMemory_Multi(str3dRawFullFileName, pBoard->Save3DRawData); // RoiByteBuf Save3DRawData
	}
	//auto it = g_pMPTI->m_vAIPart2DBuf.find(pBoard->nPartIDOrg);
	if (pBoard->nUseAISegmentation > 0)
	{
		CString sAIPath;
		sAIPath.Format(_T("%s\\%s"), tgParam.arrJpgFilePath, tgParam.arrJpgFileName);
		SaveImage_Segmentation_AI(pBoard, sAIPath);
	}

	//2D Color 영상이 기본값 세팅이 아닐 경우 합성 작업
	//m_pInspBoardInfo->bSync2DLgtPosWithJobConfig
	if (pBoard->bSync2DLgtPosWithJobConfig == true)
	{
		//2D Color 영상이 기본값 세팅이 아닐 경우 합성 작업
		if (g_pMPTI->m_nRedLgtPos_2DImg != (int)eColor2DImg_Light_Top_R
			|| g_pMPTI->m_nGreenLgtPos_2DImg != (int)eColor2DImg_Light_Top_G || g_pMPTI->m_nBlueLgtPos_2DImg != (int)eColor2DImg_Light_Top_B)
		{
			CString sSaveImgPath_2DColor;
			sSaveImgPath_2DColor = str2dFullFileName;
			SaveImage_2DColor(pBoard, sSaveImgPath_2DColor);
		}
	}
	return ret;
}
int CMNgManager::SaveImageParamDelete(std::tuple<InspPartInfo *, SaveImgParamData*, MultiStruct*, bool,int  >TupleSaveImages)
{
	//}
	bool bOffData = true;
	//while (bOffData)
	//{
	//	bOffData = !ext::ClientCtrl::get()->_SndCtrl[std::get<eBufferIndex>(TupleSaveImages)].GetFlag(ext::BufferFlag::IF_OFF);
	//	Sleep(1);
	//}

	//std::get<eInspPartInfo>(TupleSaveImages)->DeleteData();
	/*delete std::get<eInspPartInfo>(TupleSaveImages);
	std::get<eInspPartInfo>(TupleSaveImages) = nullptr;*/
	delete std::get<eSaveImgParamData>(TupleSaveImages);
	std::get<eSaveImgParamData>(TupleSaveImages) = nullptr;
	delete std::get<eMultiStructData>(TupleSaveImages);
	std::get<eMultiStructData>(TupleSaveImages) = nullptr;
	ext::ClientCtrl::get()->_SndCtrl[std::get<eBufferIndex>(TupleSaveImages)].SetFlag(ext::BufferFlag::IF_OFF, true);
	return 0; 
}
void CMNgManager::Check2DImage(UCHAR* ucImg2D, int nW, int nH)
{
	if (!m_pInspBoardInfo /*|| g_pMPTI->m_nCameraType == (int)Basler_Color ||
		g_pMPTI->IsMachineTypeUV() == TRUE || g_pMPTI->GetDiffMachineType() == DiffMachineUV*/)
		return;
// 
// 	CPInsp_Color* pColorInsp = g_pInspMng->GetColorInsp();
// 	if (pColorInsp == NULL)
// 		return;

	// ColorMap...?
	int nUserMap = 0;//g_pMPTI->m_InspMng->m_nUserMap;

	int nImgW = m_pInspBoardInfo->partImgBuf.nImageSizeX;
	int nImgH = m_pInspBoardInfo->partImgBuf.nImageSizeY;

	cv::Mat imgRGB[3];
	if (nUserMap == m_eLightPosition_MIDDLE)
	{
		imgRGB[2] = cv::Mat(nImgH, nImgW, CV_8UC1, m_pInspBoardInfo->partImgBuf.imgMiddle_R);
		imgRGB[0] = cv::Mat(nImgH, nImgW, CV_8UC1, m_pInspBoardInfo->partImgBuf.imgMiddle_B);
		float fGR = m_fFatorMGR;
		float fGB = m_fFatorMGB;
		float fitMR = fGR <= 0.0 ? 0.4f : fGR;
		float fitMB = fGB <= 0.0 ? 0.8f : fGB;
		cv::addWeighted(imgRGB[2], fitMR, imgRGB[0], fitMB, 1, imgRGB[1]);//pColorInsp->GetGreenImage(imgRGB[2].data, imgRGB[0].data, nImgW, nImgH);

	}
	else if (nUserMap == m_eLightPosition_BOTTOM)
	{
		imgRGB[2] = cv::Mat(nImgH, nImgW, CV_8UC1, m_pInspBoardInfo->partImgBuf.imgBottom_R);
		imgRGB[0] = cv::Mat(nImgH, nImgW, CV_8UC1, m_pInspBoardInfo->partImgBuf.imgBottom_B);
		float fGR = m_fFatorBGR;
		float fGB = m_fFatorBGB;
		float fitMR = fGR <= 0.0 ? 0.4f : fGR;
		float fitMB = fGB <= 0.0 ? 0.8f : fGB;
		cv::addWeighted(imgRGB[2], fitMR, imgRGB[0], fitMB, 1, imgRGB[1]);//pColorInsp->GetGreenImage(imgRGB[2].data, imgRGB[0].data, nImgW, nImgH);imgRGB[1] = pColorInsp->GetGreenImage(imgRGB[2].data, imgRGB[0].data, nImgW, nImgH, 2);
	}
	else if (nUserMap == m_eLightPosition_UserMap)
	{
		imgRGB[2] = cv::Mat(nImgH, nImgW, CV_8UC1, cv::Scalar(0));
		imgRGB[1] = cv::Mat(nImgH, nImgW, CV_8UC1, cv::Scalar(0));
		imgRGB[0] = cv::Mat(nImgH, nImgW, CV_8UC1, cv::Scalar(0));
		GetUserMapImage(m_pInspBoardInfo->partImgBuf, imgRGB[2].data, 0);
		GetUserMapImage(m_pInspBoardInfo->partImgBuf, imgRGB[1].data, 1);
		GetUserMapImage(m_pInspBoardInfo->partImgBuf, imgRGB[0].data, 2);
	}
	else
		return;

	cv::Mat imgMerge;
	cv::merge(imgRGB, 3, imgMerge);
	if (nW == nImgW && nH == nImgH)
		memcpy(ucImg2D, imgMerge.data, sizeof(UCHAR) * nW * nH * 3);
	else
	{
		cv::Mat imgRe;
		cv::resize(imgMerge, imgRe, cv::Size(nW, nH));
		memcpy(ucImg2D, imgRe.data, sizeof(UCHAR) * nW * nH * 3);
	}
}

void CMNgManager::Check2DImage_Multi(UCHAR* ucImg2D, int nW, int nH , InspRoiImgBuf partImgBuf)
{

	int nUserMap = 0;//g_pMPTI->m_InspMng->m_nUserMap;

	int nImgW = partImgBuf.nImageSizeX;
	int nImgH = partImgBuf.nImageSizeY;

	cv::Mat imgRGB[3];
	if (nUserMap == m_eLightPosition_MIDDLE)
	{
		imgRGB[2] = cv::Mat(nImgH, nImgW, CV_8UC1, partImgBuf.imgMiddle_R);
		imgRGB[0] = cv::Mat(nImgH, nImgW, CV_8UC1, partImgBuf.imgMiddle_B);
		float fGR = m_fFatorMGR;
		float fGB = m_fFatorMGB;
		float fitMR = fGR <= 0.0 ? 0.4f : fGR;
		float fitMB = fGB <= 0.0 ? 0.8f : fGB;
		cv::addWeighted(imgRGB[2], fitMR, imgRGB[0], fitMB, 1, imgRGB[1]);//pColorInsp->GetGreenImage(imgRGB[2].data, imgRGB[0].data, nImgW, nImgH);

	}
	else if (nUserMap == m_eLightPosition_BOTTOM)
	{
		imgRGB[2] = cv::Mat(nImgH, nImgW, CV_8UC1, partImgBuf.imgBottom_R);
		imgRGB[0] = cv::Mat(nImgH, nImgW, CV_8UC1, partImgBuf.imgBottom_B);
		float fGR = m_fFatorBGR;
		float fGB = m_fFatorBGB;
		float fitMR = fGR <= 0.0 ? 0.4f : fGR;
		float fitMB = fGB <= 0.0 ? 0.8f : fGB;
		cv::addWeighted(imgRGB[2], fitMR, imgRGB[0], fitMB, 1, imgRGB[1]);//pColorInsp->GetGreenImage(imgRGB[2].data, imgRGB[0].data, nImgW, nImgH);imgRGB[1] = pColorInsp->GetGreenImage(imgRGB[2].data, imgRGB[0].data, nImgW, nImgH, 2);
	}
	else if (nUserMap == m_eLightPosition_UserMap)
	{
		imgRGB[2] = cv::Mat(nImgH, nImgW, CV_8UC1, cv::Scalar(0));
		imgRGB[1] = cv::Mat(nImgH, nImgW, CV_8UC1, cv::Scalar(0));
		imgRGB[0] = cv::Mat(nImgH, nImgW, CV_8UC1, cv::Scalar(0));
		GetUserMapImage(partImgBuf, imgRGB[2].data, 0);
		GetUserMapImage(partImgBuf, imgRGB[1].data, 1);
		GetUserMapImage(partImgBuf, imgRGB[0].data, 2);
	}
	else
		return;

	cv::Mat imgMerge;
	cv::merge(imgRGB, 3, imgMerge);
	if (nW == nImgW && nH == nImgH)
		memcpy(ucImg2D, imgMerge.data, sizeof(UCHAR) * nW * nH * 3);
	else
	{
		cv::Mat imgRe;
		cv::resize(imgMerge, imgRe, cv::Size(nW, nH));
		memcpy(ucImg2D, imgRe.data, sizeof(UCHAR) * nW * nH * 3);
	}
}

float * CMNgManager::Save3DImage(CString fullPath, int * nSizeX, int * nSizeY, int exportSaveOption)
{
	CRect roi_3D;
	CRect bdryRoi;

	roi_3D = GetRectRoi();

	int roiSizeX = (int)(roi_3D.right - roi_3D.left);
	int roiSizeY = (int)(roi_3D.bottom - roi_3D.top);
	if(m_pInspBoardInfo->partImgBuf.nImageSizeX != roiSizeX)
	{
		roi_3D.right = roi_3D.left + m_pInspBoardInfo->partImgBuf.nImageSizeX;
		roiSizeX = m_pInspBoardInfo->partImgBuf.nImageSizeX;
	}
	if(m_pInspBoardInfo->partImgBuf.nImageSizeY != roiSizeY)
	{
		roi_3D.bottom = roi_3D.top + m_pInspBoardInfo->partImgBuf.nImageSizeY;
		roiSizeY = m_pInspBoardInfo->partImgBuf.nImageSizeY;
	}

	// SHKang 2017/07/20 : widthStep 반영
	int nWidthStep = roiSizeX;
	/*int nTempX = roiSizeX % 4;
	if (nTempX != 0) 	nWidthStep += 4 - nTempX;*/
	nWidthStep = g_pMPTI->nCalcWidthStep(false, roiSizeX);

	int nImgSize = nWidthStep * roiSizeY;

	UCHAR* colorImg = NULL;	
	//colorImg = new UCHAR[nImgSize * 3];
	colorImg = g_pMManager->pem_new<UCHAR>(true, nImgSize * 3, (PCHAR)__FUNCTION__, __LINE__);
	SaveDispImage(fullPath, true, colorImg, nWidthStep, exportSaveOption);
	//delete [] colorImg; 
	g_pMManager->pem_delete(colorImg, true); colorImg = NULL;	// SHKang 2017/08/07 : NULL 로 확인하게.
	if(nSizeX)
		*nSizeX = m_pInspBoardInfo->partZmapData.zmapSizeX;
	if(nSizeY)
		*nSizeY = m_pInspBoardInfo->partZmapData.zmapSizeY;
	return m_pInspBoardInfo->partZmapData.data;
}

//#####################################################################################################################
// SHKang 2017/04/04
/*
void CMNgManager::Set_JpegImage(jpeg_formatType type, jpeg_formatQuality quality)
{
	m_Jpeg.init_Param(type, quality);
}

int CMNgManager::Save_JpegImage_1ch(UCHAR * ucSrc, int width, int height, CString file_path)
{
	return m_Jpeg.PIL_Save_Jpeg_8(ucSrc, width, height, file_path);
}

int CMNgManager::Save_JpegImage_3ch(UCHAR * srcR, UCHAR * srcG, UCHAR * srcB, int width, int height, CString  file_path)
{
	return m_Jpeg.PIL_Save_Jpeg_Color(srcR, srcG, srcB, width, height, file_path);
}
*/

int CMNgManager::Save_JpegImage(UCHAR * ucSrc, int width, int height, int nChannel, _IM_COLOR imColor, CString  file_path, int nJpegQuality)
{
	return m_Jpeg.PIL_Save_Jpeg(ucSrc, width, height, nChannel, imColor, file_path, nJpegQuality);
}


//#####################################################################################################################

int CMNgManager::SaveP3DImage_SharedMemory(CString fullPath)
{
	int ret = 0;
	if(!m_pInspBoardInfo)
		return ret;
	if(!m_pInspBoardInfo->Save3DRawData.pBuffer || m_pInspBoardInfo->Save3DRawData.szSize <= 0)
		return ret;

	CString tmpPath;
	tmpPath = fullPath;
	tmpPath.MakeLower();
	if((tmpPath.Right(4) == _T(".p3d")) == FALSE)
	{
		fullPath += _T(".p3d");
	}

	BYTE* pImg = m_pInspBoardInfo->Save3DRawData.pBuffer;

	CSharedMemory shared;
	shared.AllocFile(fullPath, m_pInspBoardInfo->Save3DRawData.szSize, _T(""));
	shared.MapToWrite(0, m_pInspBoardInfo->Save3DRawData.szSize);
	BYTE * Buffer = shared.Byte();

	memcpy(Buffer, m_pInspBoardInfo->Save3DRawData.pBuffer, m_pInspBoardInfo->Save3DRawData.szSize);

	ret = 1;
	return ret;
}
int CMNgManager::DefectImgCrop_AngleColor(InspPartInfo* pBoard, InspectionResult* inspectionResult, int roiSizeX, int roiSizeY, UCHAR* ucAngleColor, int roistep, int nMachineCode, int ColorMode, CString sJobPath)
{
	int ret = eMR_SUCCESS;
	CString sFolderPath = pBoard->s2DImagePath;
#if _DEBUG
	if (sFolderPath == _T(""))
	{
		sFolderPath.Format(_T("D:\\testimage\\"));
	}
#endif
	if (sFolderPath == _T("") || nMachineCode < 0)
		return ret;
	CString sOKImgPath = _T("");
	cv::Mat OKImg;
	bool OkImgSave = false;
	if (pBoard != NULL && sJobPath != _T(""))
	{
		sOKImgPath.Format(_T("%s\\PartImage\\%s.jpg"), sJobPath, pBoard->modelName);

		TCHAR szExistCrashFile[MAX_PATH] = { 0, };
		_stprintf(szExistCrashFile, _T("%s"), sOKImgPath);
		if ((_taccess(szExistCrashFile, 0)) == -1)
		{
			sOKImgPath.Format(_T("%s\\PartImage\\%s.tif"), sJobPath, pBoard->modelName);
			if ((_taccess(szExistCrashFile, 0)) != -1)
			{
				OKImg = cv::imread(std::string(CT2A(sOKImgPath)), -1);
				if (OKImg.cols >= roiSizeX && OKImg.rows >= roiSizeY)
					OkImgSave = true;
			}
		}
		else
		{
			OKImg = cv::imread(std::string(CT2A(sOKImgPath)), -1);
			if (OKImg.cols >= roiSizeX && OKImg.rows >= roiSizeY)
				OkImgSave = true;
		}
	}
	if (pBoard != NULL && inspectionResult != NULL && inspectionResult->nDefectType != NG_Type::eOK && sFolderPath != _T(""))
	{
		cv::Mat PartACImg(roiSizeY, roiSizeX, CV_8UC3, ucAngleColor, roistep * 3);
		SIZE boarSz = g_pMPTI->GetSizeBoard();
		float fIMGSz = 1.1f;
		int nIMGSzX = fIMGSz / m_resolX;
		int nIMGSzY = fIMGSz / m_resolY;
		CPoint PartLT;
		PartLT.x = (pBoard->partCx - (pBoard->partWidth / 2)) / m_resolX;
		//PartLT.y = (boarSz.cy - (pBoard->partCy + (pBoard->partHeight / 2))) / m_resolY;
		PartLT.y = (pBoard->partCy - (pBoard->partHeight / 2)) / m_resolY;

		for (int wType = 0; wType < eINSP_Total; wType++)
		{
			InspWndResult* InspwndRst = inspectionResult->GetWndResult((insp_type)wType);
			int wSize = inspectionResult->GetWndArraySize((insp_type)wType);
			for (int w = 0; w < wSize; w++)
			{
				InspAlgoResult* InspalgoRst = InspwndRst[w].m_vArrRstInspAlgo;
				InspParamTemp InspParamtmp = inspectionResult->GetWndParam(eINSP_MOUNT)[w];
				for (int a = 0; a < InspwndRst[w].m_nAlgorithmCnt; a++)
				{
					if (InspalgoRst[a].m_bOk == 0)
					{
						if (InspalgoRst[a].m_nAlgoType == InspAlgoType::eAlgoPatternDiff)
						{
							RstAlgoPatternDiff* RstPatternDiff = (RstAlgoPatternDiff*)InspalgoRst[a].m_vRstInspAlgo;
							cv::Mat AlgoGray;
							BasicAlgo_CalcROICompose(InspParamtmp.vArrAlgoParam[a], AlgoGray);
							for (int d = 0; d < RstPatternDiff->m_nRectCnt; d++)
							{
								int nStx = RstPatternDiff->m_ptArrRstPtr[d][2].x < RstPatternDiff->m_ptArrRstPtr[d][3].x ? RstPatternDiff->m_ptArrRstPtr[d][2].x : RstPatternDiff->m_ptArrRstPtr[d][3].x;
								int nStR = RstPatternDiff->m_ptArrRstPtr[d][0].x > RstPatternDiff->m_ptArrRstPtr[d][1].x ? RstPatternDiff->m_ptArrRstPtr[d][0].x : RstPatternDiff->m_ptArrRstPtr[d][1].x;
								int nSty = RstPatternDiff->m_ptArrRstPtr[d][0].y < RstPatternDiff->m_ptArrRstPtr[d][3].y ? RstPatternDiff->m_ptArrRstPtr[d][0].y : RstPatternDiff->m_ptArrRstPtr[d][3].y;
								int nStB = RstPatternDiff->m_ptArrRstPtr[d][1].y > RstPatternDiff->m_ptArrRstPtr[d][2].y ? RstPatternDiff->m_ptArrRstPtr[d][1].y : RstPatternDiff->m_ptArrRstPtr[d][2].y;
								int nClipW = nStR - nStx + 1;
								int nClipH = nStB - nSty + 1;
								int nDeW = nIMGSzX;
								int nDeH = nIMGSzY;
								cv::Rect DefectRect(0, 0, nClipW - 1, nClipH - 1);
								//cv::Mat defectImg = WndROI(DefectRect);
								float fCx = nStx + (nClipW / 2);
								float fCy = nSty + (nClipH / 2);
								cv::Rect ClipRect;
								DefectImgCropRect(DefectRect, ClipRect, PartACImg, nIMGSzX, nIMGSzY, fCx, fCy, nClipW, nClipH, nStx, nSty, nDeW, nDeH);

								cv::Mat defectImg(nDeH, nDeW, CV_8UC3);
								defectImg.setTo(0);
								cv::Mat defectClipImg = defectImg(DefectRect);
								PartACImg(ClipRect).copyTo(defectClipImg);
								(this->*ImgProcessing[ColorMode])(AlgoGray(ClipRect), defectClipImg, defectClipImg, true);
#if _DEBUG
								CString defectPath_Debug;
								defectPath_Debug.Format(_T("D:\\testimage\\DefectSave_PatternDiff_%d_%d_%d_%d.jpg"), pBoard->nPartID, w, a, d);
								cv::imwrite(std::string(CT2A(defectPath_Debug)), defectImg);
#endif
								//shkim 2024.09.02 좌하단 (0,0)으로 수정 및 X,Y좌표 반전(해성DS요청)
								CString defectPath;
								defectPath.Format(_T("%s [%d] size_x=%.3f size_y=%.3f size_z=%.3f 좌표 X=%d Y=%d %d DEF.png"), sFolderPath, InspwndRst[w].m_nDefectCode, RstPatternDiff->m_dRstWidth[d], RstPatternDiff->m_dRstLength[d], 0, (int)std::roundf(PartLT.y + fCy), (int)std::roundf(PartLT.x + fCx), nMachineCode);
								cv::imwrite(std::string(CT2A(defectPath)), defectImg);
								if (OkImgSave)
								{
									cv::Mat OKSaveImg(nDeH, nDeW, CV_8UC3);
									OKSaveImg.setTo(0);
									cv::Mat OKCropImg = OKSaveImg(DefectRect);
									OKImg(ClipRect).copyTo(OKCropImg);
									cv::Mat Img100(ClipRect.height, ClipRect.width, CV_8UC1);
									Img100.setTo(80);
									(this->*ImgProcessing[ColorMode])(Img100, OKCropImg, OKCropImg, false);

									CString OKImagePath;
									OKImagePath.Format(_T("%s [%d] size_x=%.3f size_y=%.3f size_z=%.3f 좌표 X=%d Y=%d %d REF.png"), sFolderPath, InspwndRst[w].m_nDefectCode, RstPatternDiff->m_dRstWidth[d], RstPatternDiff->m_dRstLength[d], 0, (int)std::roundf(PartLT.y + fCy), (int)std::roundf(PartLT.x + fCx), nMachineCode);
									cv::imwrite(std::string(CT2A(OKImagePath)), OKSaveImg);
								}
							}
						}
						else if (InspalgoRst[a].m_nAlgoType == InspAlgoType::eAlgoPadBW)
						{
							AlgoPadBW* pAlgoPadBW = (AlgoPadBW*)InspParamtmp.vArrAlgoParam[a].m_ptrInspAlgoParam;
							PIAL::PI_Buff* RstCompose = new PIAL::PI_Buff(m_pInspBoardInfo->partImgBuf.nImageSizeX, m_pInspBoardInfo->partImgBuf.nImageSizeY);
							PadBW_CalcRoiCompose(*pAlgoPadBW, 0, 0, RstCompose);

							cv::Mat AlgoGray = RstCompose->Mat();
							RstAlgoPadBW* RstPadBW = (RstAlgoPadBW*)InspalgoRst[a].m_vRstInspAlgo;
							for (int d = 0; d < RstPadBW->m_nArrRectCnt; d++)
							{
								int nStx = RstPadBW->m_rcArrRect_I[d].left;
								int nSty = RstPadBW->m_rcArrRect_I[d].top;
								int nClipW = RstPadBW->m_rcArrRect_I[d].right - RstPadBW->m_rcArrRect_I[d].left + 1;
								int nClipH = RstPadBW->m_rcArrRect_I[d].bottom - RstPadBW->m_rcArrRect_I[d].top + 1;
								int nDeW = nIMGSzX;
								int nDeH = nIMGSzY;
								cv::Rect DefectRect(0, 0, nClipW - 1, nClipH - 1);
								//cv::Mat defectImg = WndROI(DefectRect);
								float fCx = nStx + (nClipW / 2);
								float fCy = nSty + (nClipH / 2);

								cv::Rect ClipRect;
								DefectImgCropRect(DefectRect, ClipRect, PartACImg, nIMGSzX, nIMGSzY, fCx, fCy, nClipW, nClipH, nStx, nSty, nDeW, nDeH);

								cv::Mat defectImg(nDeH, nDeW, CV_8UC3);
								defectImg.setTo(0);
								cv::Mat defectClipImg = defectImg(DefectRect);
								PartACImg(ClipRect).copyTo(defectClipImg);
								//cv::Mat AlgoDefectGray = AlgoGray(ClipRect).clone();
								(this->*ImgProcessing[ColorMode])(AlgoGray(ClipRect), defectClipImg, defectClipImg, true);
#if _DEBUG
								CString defectPath_Debug;
								defectPath_Debug.Format(_T("D:\\testimage\\DefectSave_PadBW_%d_%d_%d_%d.jpg"), pBoard->nPartID, w, a, d);
								cv::imwrite(std::string(CT2A(defectPath_Debug)), defectImg);
#endif
								//shkim 2024.09.02 좌하단 (0,0)으로 수정 및 X,Y좌표 반전(해성DS요청)
								CString defectPath;
								defectPath.Format(_T("%s [%d] size_x=%.3f size_y=%.3f size_z=%.3f 좌표 X=%d Y=%d %d DEF.png"), sFolderPath, InspwndRst[w].m_nDefectCode, RstPadBW->m_fArrRstWidth[d], RstPadBW->m_fArrRstLength[d], RstPadBW->m_nArrRstHeightMax[d], (int)std::roundf(PartLT.y + fCy), (int)std::roundf(PartLT.x + fCx), nMachineCode);
								cv::imwrite(std::string(CT2A(defectPath)), defectImg);
								if (OkImgSave)
								{
									cv::Mat OKSaveImg(nDeH, nDeW, CV_8UC3);
									OKSaveImg.setTo(0);
									cv::Mat OKCropImg = OKSaveImg(DefectRect);
									OKImg(ClipRect).copyTo(OKCropImg);
									cv::Mat Img100(ClipRect.height, ClipRect.width, CV_8UC1);
									Img100.setTo(80);
									(this->*ImgProcessing[ColorMode])(Img100, OKCropImg, OKCropImg, false);
									CString OKImagePath;
									OKImagePath.Format(_T("%s [%d] size_x=%.3f size_y=%.3f size_z=%.3f 좌표 X=%d Y=%d %d REF.png"), sFolderPath, InspwndRst[w].m_nDefectCode, RstPadBW->m_fArrRstWidth[d], RstPadBW->m_fArrRstLength[d], RstPadBW->m_nArrRstHeightMax[d], (int)std::roundf(PartLT.y + fCy), (int)std::roundf(PartLT.x + fCx), nMachineCode);
									cv::imwrite(std::string(CT2A(OKImagePath)), OKSaveImg);
								}
							}
						}
						else if (InspalgoRst[a].m_nAlgoType == InspAlgoType::eAlgoBlob)
						{
							RstAlgoBlob* RstBlob = (RstAlgoBlob*)InspalgoRst[a].m_vRstInspAlgo;
							cv::Mat AlgoGray;
							BasicAlgo_CalcROICompose(InspParamtmp.vArrAlgoParam[a], AlgoGray);
							for (int d = 0; d < RstBlob->m_nArrRectCnt; d++)
							{
								int nStx = RstBlob->m_rcArrRect[d].left;
								int nSty = RstBlob->m_rcArrRect[d].top;
								int nClipW = RstBlob->m_rcArrRect[d].right - RstBlob->m_rcArrRect[d].left + 1;
								int nClipH = RstBlob->m_rcArrRect[d].bottom - RstBlob->m_rcArrRect[d].top + 1;
								int nDeW = nIMGSzX;
								int nDeH = nIMGSzY;
								cv::Rect DefectRect(0, 0, nClipW - 1, nClipH - 1);
								//cv::Mat defectImg = WndROI(DefectRect);
								float fCx = nStx + (nClipW / 2);
								float fCy = nSty + (nClipH / 2);

								cv::Rect ClipRect;
								DefectImgCropRect(DefectRect, ClipRect, PartACImg, nIMGSzX, nIMGSzY, fCx, fCy, nClipW, nClipH, nStx, nSty, nDeW, nDeH);

								cv::Mat defectImg(nDeH, nDeW, CV_8UC3);
								defectImg.setTo(0);
								cv::Mat defectClipImg = defectImg(DefectRect);
								PartACImg(ClipRect).copyTo(defectClipImg);

								(this->*ImgProcessing[ColorMode])(AlgoGray(ClipRect), defectClipImg, defectClipImg, true);
#if _DEBUG
								CString defectPath_Debug;
								defectPath_Debug.Format(_T("D:\\testimage\\DefectSave_blob_%d_%d_%d_%d.jpg"), pBoard->nPartID, w, a, d);
								cv::imwrite(std::string(CT2A(defectPath_Debug)), defectImg);
#endif
								float fW = RstBlob->m_fArrRstW[d];
								float fL = RstBlob->m_fArrRstL[d];
								fW = fW == 0 ? RstBlob->m_dRstWidth : fW;
								fL = fL == 0 ? RstBlob->m_dRstLength : fL;
								//shkim 2024.09.02 좌하단 (0,0)으로 수정 및 X,Y좌표 반전(해성DS요청)
								CString defectPath;
								defectPath.Format(_T("%s [%d] size_x=%.3f size_y=%.3f size_z=%.3f 좌표 X=%d Y=%d %d DEF.png"), sFolderPath, InspwndRst[w].m_nDefectCode, fW, fL, 0, (int)std::roundf(PartLT.y + fCy), (int)std::roundf(PartLT.x + fCx), nMachineCode);
								cv::imwrite(std::string(CT2A(defectPath)), defectImg);
								if (OkImgSave)
								{
									cv::Mat OKSaveImg(nDeH, nDeW, CV_8UC3);
									OKSaveImg.setTo(0);
									cv::Mat OKCropImg = OKSaveImg(DefectRect);
									OKImg(ClipRect).copyTo(OKCropImg);
									cv::Mat Img100(ClipRect.height, ClipRect.width, CV_8UC1);
									Img100.setTo(80);
									(this->*ImgProcessing[ColorMode])(Img100, OKCropImg, OKCropImg, false);

									CString OKImagePath;
									OKImagePath.Format(_T("%s [%d] size_x=%.3f size_y=%.3f size_z=%.3f 좌표 X=%d Y=%d %d REF.png"), sFolderPath, InspwndRst[w].m_nDefectCode, fW, fL, 0, (int)std::roundf(PartLT.y + fCy), (int)std::roundf(PartLT.x + fCx), nMachineCode);
									cv::imwrite(std::string(CT2A(OKImagePath)), OKSaveImg);
								}
							}
						}
						else if (InspalgoRst[a].m_nAlgoType == InspAlgoType::eAlgoHeight_Diff)
						{
							RstAlgoHeightDiff* RstHeightDiff = (RstAlgoHeightDiff*)InspalgoRst[a].m_vRstInspAlgo;
							for (int d = 0; d < 2; d++)
							{
								int nStx = RstHeightDiff->m_rcRect_I[d].left;
								int nSty = RstHeightDiff->m_rcRect_I[d].top;
								int nClipW = RstHeightDiff->m_rcRect_I[d].right - RstHeightDiff->m_rcRect_I[d].left + 1;
								int nClipH = RstHeightDiff->m_rcRect_I[d].bottom - RstHeightDiff->m_rcRect_I[d].top + 1;
								int nDeW = nIMGSzX;
								int nDeH = nIMGSzY;
								cv::Rect DefectRect(0, 0, nClipW - 1, nClipH - 1);
								//cv::Mat defectImg = WndROI(DefectRect);
								float fCx = nStx + (nClipW / 2);
								float fCy = nSty + (nClipH / 2);


								cv::Rect ClipRect;
								DefectImgCropRect(DefectRect, ClipRect, PartACImg, nIMGSzX, nIMGSzY, fCx, fCy, nClipW, nClipH, nStx, nSty, nDeW, nDeH);

								cv::Mat defectImg(nDeH, nDeW, CV_8UC3);
								defectImg.setTo(0);
								cv::Mat defectClipImg = defectImg(DefectRect);
								PartACImg(ClipRect).copyTo(defectClipImg);
#if _DEBUG
								CString defectPath_Debug;
								defectPath_Debug.Format(_T("D:\\testimage\\DefectSave_blob_%d_%d_%d_%d.jpg"), pBoard->nPartID, w, a, d);
								cv::imwrite(std::string(CT2A(defectPath_Debug)), defectImg);
#endif
								float fW = (RstHeightDiff->m_rcRect_I[d].right - RstHeightDiff->m_rcRect_I[d].left + 1)*m_resolX;
								float fL = (RstHeightDiff->m_rcRect_I[d].bottom - RstHeightDiff->m_rcRect_I[d].top + 1)*m_resolY;
								//shkim 2024.09.02 좌하단 (0,0)으로 수정 및 X,Y좌표 반전(해성DS요청)
								CString defectPath;
								defectPath.Format(_T("%s [%d] size_x=%.3f size_y=%.3f size_z=%.3f 좌표 X=%d Y=%d %d DEF.png"), sFolderPath, InspwndRst[w].m_nDefectCode, fW, fL, RstHeightDiff->m_dRstHeightDiff, (int)std::roundf(PartLT.y + fCy), (int)std::roundf(PartLT.x + fCx), nMachineCode);
								cv::imwrite(std::string(CT2A(defectPath)), defectImg);
								if (OkImgSave)
								{
									cv::Mat OKSaveImg(nDeH, nDeW, CV_8UC3);
									OKSaveImg.setTo(0);
									cv::Mat OKCropImg = OKSaveImg(DefectRect);
									OKImg(ClipRect).copyTo(OKCropImg);

									CString OKImagePath;
									OKImagePath.Format(_T("%s [%d] size_x=%.3f size_y=%.3f size_z=%.3f 좌표 X=%d Y=%d %d REF.png"), sFolderPath, InspwndRst[w].m_nDefectCode, fW, fL, RstHeightDiff->m_dRstHeightDiff, (int)std::roundf(PartLT.y + fCy), (int)std::roundf(PartLT.x + fCx), nMachineCode);
									cv::imwrite(std::string(CT2A(OKImagePath)), OKSaveImg);
								}
							}
						}
					}
				}
			}
		}
	}

	return ret;
}

int CMNgManager::DefectImgCropRect(cv::Rect& DefectRect, cv::Rect& ClipRect, cv::Mat& PartACImg, int nIMGSzX, int nIMGSzY, float fCx, float fCy, int &nClipW, int &nClipH, int& nStx, int& nSty, int& nDeW, int& nDeH)
{
	if (nClipW < nIMGSzX)
	{
		nClipW = nIMGSzX;
		nStx = fCx - (nClipW / 2);
		if (nStx < 0)
			nStx = 0;
		DefectRect.x = (nIMGSzX - nClipW) / 2;
		DefectRect.width = nClipW < PartACImg.cols ? nClipW : PartACImg.cols - 1;
	}
	else if (nClipW > nIMGSzX)
	{
		nDeW = nClipW;
		DefectRect.x = 0;
		DefectRect.width = nDeW;
	}
	else
	{
		nDeW = nClipW;
		DefectRect.x = 0;
		DefectRect.width = nDeW;
	}
	if (nClipH < nIMGSzY)
	{
		nClipH = nIMGSzY;
		nSty = fCy - (nClipH / 2);
		if (nSty < 0)
			nSty = 0;
		DefectRect.y = (nIMGSzY - nClipH) / 2;
		DefectRect.height = nClipH < PartACImg.rows ? nClipH : PartACImg.rows - 1;
	}
	else if (nClipW > nIMGSzX)
	{
		nDeH = nClipH;
		DefectRect.y = 0;
		DefectRect.height = nDeH;
	}
	else
	{
		nDeH = nClipH;
		DefectRect.y = 0;
		DefectRect.height = nDeH;
	}

	if (nStx + nClipW >= PartACImg.cols)
	{
		nClipW = PartACImg.cols - nStx - 1;
		if (DefectRect.width != nClipW)
		{
			DefectRect.x = std::abs((nClipW - DefectRect.width) / 2);
			DefectRect.width = nClipW;
		}
	}
	if (nSty + nClipH >= PartACImg.rows)
	{
		nClipH = PartACImg.rows - nSty - 1;
		if (DefectRect.height != nClipH)
		{
			DefectRect.y = std::abs((nClipH - DefectRect.height) / 2);
			DefectRect.height = nClipH;
		}
	}
	ClipRect.x = nStx;
	ClipRect.y = nSty;
	ClipRect.width = nClipW;
	ClipRect.height = nClipH;

	return 0;
}

void CMNgManager::BasicAlgo_CalcROICompose(InspAlgo& sInspAlgo, cv::Mat& dst)
{
	UCHAR * ImgR = NULL, *ImgG = NULL, *ImgB = NULL, *ImgW = NULL;
	LightTypeBuf sLightBuf;
	switch (sInspAlgo.m_eLightType)
	{
	case Top_Light:
		ImgR = m_pInspBoardInfo->partImgBuf.imgTop_R;
		ImgG = m_pInspBoardInfo->partImgBuf.imgTop_G;
		ImgB = m_pInspBoardInfo->partImgBuf.imgTop_B;
		break;
	case Middle_Light:
		ImgR = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
		ImgB = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
		break;
	case Bottom_Light:
		if (g_pMPTI->m_nCameraType == Basler_Color)
		{
			ImgR = m_pInspBoardInfo->partImgBuf.imgBottom_R;
			ImgB = m_pInspBoardInfo->partImgBuf.imgBottom_B;
		}
		else
		{
			ImgR = m_pInspBoardInfo->partImgBuf.imgBottom_R;
			ImgB = m_pInspBoardInfo->partImgBuf.imgBottom_B;
		}
		break;
	case  User_Light:
		if (g_pMPTI->m_nCameraType == Basler_Color)
		{
			sLightBuf.m_pucTRed = m_pInspBoardInfo->partImgBuf.imgTop_W;
			sLightBuf.m_pucTGreen = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
			sLightBuf.m_pucTBlue = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
			sLightBuf.m_pucTWhite = NULL;
			sLightBuf.m_pucMRed = NULL;
			sLightBuf.m_pucMGreen = NULL;
			sLightBuf.m_pucMBlue = NULL;
			sLightBuf.m_pucMWhite = NULL;
			sLightBuf.m_pucBRed = m_pInspBoardInfo->partImgBuf.imgBottom_R;
			sLightBuf.m_pucBGreen = NULL;
			sLightBuf.m_pucBBlue = m_pInspBoardInfo->partImgBuf.imgBottom_B;
			sLightBuf.m_pucBWhite = NULL;
		}
		else
		{
			sLightBuf.m_pucTRed = m_pInspBoardInfo->partImgBuf.imgTop_R;
			sLightBuf.m_pucTGreen = m_pInspBoardInfo->partImgBuf.imgTop_G;
			sLightBuf.m_pucTBlue = m_pInspBoardInfo->partImgBuf.imgTop_B;
			sLightBuf.m_pucTWhite = m_pInspBoardInfo->partImgBuf.imgTop_W;

			sLightBuf.m_pucMRed = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
			sLightBuf.m_pucMGreen = NULL;
			sLightBuf.m_pucMBlue = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
			sLightBuf.m_pucMWhite = NULL;

			sLightBuf.m_pucBRed = m_pInspBoardInfo->partImgBuf.imgBottom_R;
			sLightBuf.m_pucBGreen = NULL;
			sLightBuf.m_pucBBlue = m_pInspBoardInfo->partImgBuf.imgBottom_B;
			sLightBuf.m_pucBWhite = NULL;
		}

		sLightBuf.m_nImgWidth = m_pInspBoardInfo->partImgBuf.nImageSizeX;
		sLightBuf.m_nImgHeight = m_pInspBoardInfo->partImgBuf.nImageSizeY;
		sLightBuf.m_nROIImgWidth = m_pInspBoardInfo->partImgBuf.nImageSizeX;
		sLightBuf.m_nROIImgHeight = m_pInspBoardInfo->partImgBuf.nImageSizeY;
		sLightBuf.m_dROIX = 0;
		sLightBuf.m_dROIY = 0;
		sLightBuf.m_nImgCnt = sInspAlgo.m_nLightCnt;
		sLightBuf.m_pnRedValue = sInspAlgo.m_nArrRedValue;
		sLightBuf.m_pnGreenValue = sInspAlgo.m_nArrGreenValue;
		sLightBuf.m_pnBlueValue = sInspAlgo.m_nArrBlueValue;
		sLightBuf.m_pnWhiteValue = sInspAlgo.m_nArrWhiteValue;
		sLightBuf.m_pnPosition = sInspAlgo.m_nArrLightPosition;
		sLightBuf.m_pnCalculation = sInspAlgo.m_nArrCalculation;
	}
	int width = m_pInspBoardInfo->partImgBuf.nImageSizeX;
	int height = m_pInspBoardInfo->partImgBuf.nImageSizeY;
	dst = cv::Mat(m_pInspBoardInfo->partImgBuf.nImageSizeY, m_pInspBoardInfo->partImgBuf.nImageSizeX, CV_8UC1);
	dst.setTo(0);
	CPInsp_Algo* m_InspAlgo = g_pInspMng->GetPtrInspAlgo();
	if ((sInspAlgo.m_eLightType == Top_Light) || (sInspAlgo.m_eLightType == Middle_Light) || (sInspAlgo.m_eLightType == Bottom_Light))
	{
		//m_InspAlgo.RoiImageCompose(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w*wnd_h, cx, cy, wnd_w, wnd_h, sInspAlgo.m_nRedValue, sInspAlgo.m_nGreenValue, sInspAlgo.m_nBlueValue, sInspAlgo.m_nWhiteValue, wndImg.m_ucArr2D);
		m_InspAlgo->RoiImageCompose_LT(ImgR, ImgG, ImgB, ImgW, width*height, width, height, width*height, 0, 0, width, height,
			sInspAlgo.m_nRedValue, sInspAlgo.m_nGreenValue, sInspAlgo.m_nBlueValue, sInspAlgo.m_nWhiteValue,
			dst.data, 0);

	}
	else
	{
		m_InspAlgo->ROIImageClaculCompose(sLightBuf, dst.data, 0);
	}
}
BOOL CMNgManager::PadBW_CalcRoiCompose(AlgoPadBW &pAlgoPadBW, int nSelectedLightIdx, BOOL bInspMode, PIAL::PI_Buff* RstCompose)
{
	BOOL bRet = FALSE;


	if (RstCompose == nullptr)
		return bRet;

	lightData _LightData = pAlgoPadBW.sArrInspPad[nSelectedLightIdx].stInspAlgoLightsMix;

	PIAL::_LightTypeBuf stSubLight;
	stSubLight.m_pnRedValue = new int[_LIGHT_CNT] {true, };
	stSubLight.m_pnGreenValue = new int[_LIGHT_CNT] {true, };
	stSubLight.m_pnBlueValue = new int[_LIGHT_CNT] {true, };
	stSubLight.m_pnWhiteValue = new int[_LIGHT_CNT] {true, };
	stSubLight.m_pnPosition = new int[_LIGHT_CNT] {true, };
	stSubLight.m_pnCalculation = new int[_LIGHT_CNT] {true, };

	memset(stSubLight.m_pnRedValue, 0, _LIGHT_CNT * sizeof(int));
	memset(stSubLight.m_pnGreenValue, 0, _LIGHT_CNT * sizeof(int));
	memset(stSubLight.m_pnBlueValue, 0, _LIGHT_CNT * sizeof(int));
	memset(stSubLight.m_pnWhiteValue, 0, _LIGHT_CNT * sizeof(int));
	memset(stSubLight.m_pnPosition, 0, _LIGHT_CNT * sizeof(int));
	memset(stSubLight.m_pnCalculation, 0, _LIGHT_CNT * sizeof(int));

	//Different Image step
	if (bInspMode == 1)
	{
		//Teaching
		//Window ptr Data
		stSubLight.m_nImgWidth = m_pInspBoardInfo->partImgBuf.nImageSizeX;
		stSubLight.m_nImgHeight = m_pInspBoardInfo->partImgBuf.nImageSizeY;
	}
	else
	{
		//Auto inspection
		//Part ptr Data
		stSubLight.m_nImgWidth = m_pInspBoardInfo->partImgBuf.nImageSizeX;
		stSubLight.m_nImgHeight = m_pInspBoardInfo->partImgBuf.nImageSizeY;
	}

	stSubLight.m_nROIImgWidth = m_pInspBoardInfo->partImgBuf.nImageSizeX;
	stSubLight.m_nROIImgHeight = m_pInspBoardInfo->partImgBuf.nImageSizeY;

	//no startXY, in AlgorithmTool
	//스탭만 파트, 윈도우로 상황마다 다를뿐이지 Window -> Window 데이터이기 때문에 Roi는 무조건 0이다.
	stSubLight.m_dROIX = 0;
	stSubLight.m_dROIY = 0;

	stSubLight.m_pucTRed = m_pInspBoardInfo->partImgBuf.imgTop_R;
	stSubLight.m_pucTGreen = m_pInspBoardInfo->partImgBuf.imgTop_G;
	stSubLight.m_pucTBlue = m_pInspBoardInfo->partImgBuf.imgTop_B;
	stSubLight.m_pucTWhite = m_pInspBoardInfo->partImgBuf.imgTop_W;
	stSubLight.m_pucMRed = m_pInspBoardInfo->partImgBuf.imgMiddle_R;
	stSubLight.m_pucMGreen = NULL;
	stSubLight.m_pucMBlue = m_pInspBoardInfo->partImgBuf.imgMiddle_B;
	stSubLight.m_pucMWhite = NULL;

	stSubLight.m_pucBRed = m_pInspBoardInfo->partImgBuf.imgBottom_R;
	stSubLight.m_pucBGreen = NULL;
	stSubLight.m_pucBBlue = m_pInspBoardInfo->partImgBuf.imgBottom_B;
	stSubLight.m_pucBWhite = NULL;

	int nPlusFlag = 1;
	if (_LightData.m_byLightCnt == 1) //sInspAlgo.m_eLightType != _InspLightType::_User_Light)
	{
		//single light
		stSubLight.m_nImgCnt = 1;
		stSubLight.m_pnRedValue[0] = (int)_LightData.m_byArrLightData[_m_eLightData::_eLightData_Red][0];
		stSubLight.m_pnGreenValue[0] = (int)_LightData.m_byArrLightData[_m_eLightData::_eLightData_Green][0];
		stSubLight.m_pnBlueValue[0] = (int)_LightData.m_byArrLightData[_m_eLightData::_eLightData_Blue][0];
		stSubLight.m_pnWhiteValue[0] = (int)_LightData.m_byArrLightData[_m_eLightData::_eLightData_White][0];
		stSubLight.m_pnPosition[0] = (int)_InspLightType::_Top_Light;	//top;
		stSubLight.m_pnCalculation[0] = nPlusFlag;
		switch (_LightData.m_byArrLightData[_m_eLightData::_eLightData_Position][0])
		{
		case _InspLightType::_Middle_Light:
			stSubLight.m_pnPosition[0] = (int)_InspLightType::_Middle_Light;
			break;
		case _InspLightType::_Bottom_Light:
			stSubLight.m_pnPosition[0] = (int)_InspLightType::_Bottom_Light;
			break;
		}
	}
	else
	{
		//user light
		stSubLight.m_nImgCnt = (int)_LightData.m_byLightCnt;
		for (int i = 0; i < stSubLight.m_nImgCnt; i++)
		{
			stSubLight.m_pnPosition[i] = (int)_LightData.m_byArrLightData[_m_eLightData::_eLightData_Position][i];
			stSubLight.m_pnCalculation[i] = (int)_LightData.m_byArrLightData[_m_eLightData::_eLightData_Calculation][i];
			stSubLight.m_pnRedValue[i] = (int)_LightData.m_byArrLightData[_m_eLightData::_eLightData_Red][i];
			stSubLight.m_pnGreenValue[i] = (int)_LightData.m_byArrLightData[_m_eLightData::_eLightData_Green][i];
			stSubLight.m_pnBlueValue[i] = (int)_LightData.m_byArrLightData[_m_eLightData::_eLightData_Blue][i];
			stSubLight.m_pnWhiteValue[i] = (int)_LightData.m_byArrLightData[_m_eLightData::_eLightData_White][i];
		}
	}

	//if (pImg_buf.inspPartImage->m_Angle == 0)
	{
		PIAL::PAlgo::RoiCalcImageCompose(stSubLight, RstCompose->m_pData);
	}
	//else
	//{
	//	PIAL::PI_Buff composeimgOrg(RstCompose->Width(), RstCompose->Length());
	//	PIAL::PAlgo::RoiCalcImageCompose(stSubLight, composeimgOrg.m_pData);
	//	PIAL::PAlgo::RotateImg_ipp2020_2(composeimgOrg.m_pData, RstCompose->m_pData, -pImg_buf.inspPartImage->m_Angle, RstCompose->Width(), RstCompose->Length(), NULL, NULL, false, true, true);
	//}
	//Release
	delete[] stSubLight.m_pnRedValue;
	delete[] stSubLight.m_pnGreenValue;
	delete[] stSubLight.m_pnBlueValue;
	delete[] stSubLight.m_pnWhiteValue;
	delete[] stSubLight.m_pnCalculation;
	delete[] stSubLight.m_pnPosition;

	return TRUE;
}
void CMNgManager::Processing_DS(cv::Mat& gray, cv::Mat& Color, cv::Mat& dst, bool bNomalize)
{
	cv::Mat fGray, Gray100;
	if (bNomalize)
		cv::normalize(gray, Gray100, 60, 100, cv::NORM_MINMAX);
	else
		Gray100 = gray.clone();

	cv::Mat arrHSV[3];
	cv::Mat mHSV;
	cv::cvtColor(Color, mHSV, cv::COLOR_BGR2HSV);
	cv::split(mHSV, arrHSV);
	for (int r = 0; r < Gray100.rows; r++)
	{
		UCHAR* vPtr = arrHSV[2].ptr(r);
		UCHAR* gPtr = Gray100.ptr(r);
		for (int c = 0; c < gray.cols; c++)
		{
			//float ff = (vPtr[c]) * (gPtr[c] / 100.0);//W
			float ff = (vPtr[c]) * ((100 + (60 - gPtr[c])) / 100.0);//black
			uchar uu = (uchar)(ff);
			vPtr[c] = uu;
			if (ff > 0)
				vPtr[c] = uu;
		}
	}
	cv::merge(arrHSV, 3, dst);
	cv::cvtColor(dst, dst, cv::COLOR_HSV2BGR);
}
void CMNgManager::Processing_DSGray(cv::Mat& gray, cv::Mat& Color, cv::Mat& dst, bool bNomalize)
{
	cv::cvtColor(gray, dst, cv::COLOR_GRAY2BGR);
}

int CMNgManager::SaveP3DImage_SharedMemory_Multi(CString fullPath, RoiByteBuf Save3DRawData)
{
	int ret = 0;
	if (!Save3DRawData.pBuffer || Save3DRawData.szSize <= 0)
		return ret;

	CString tmpPath;
	tmpPath = fullPath;
	tmpPath.MakeLower();
	if ((tmpPath.Right(4) == _T(".p3d")) == FALSE)
	{
		fullPath += _T(".p3d");
	}

	BYTE* pImg = Save3DRawData.pBuffer;

	CSharedMemory shared;
	shared.AllocFile(fullPath, Save3DRawData.szSize, _T(""));
	shared.MapToWrite(0, Save3DRawData.szSize);
	BYTE * Buffer = shared.Byte();

	memcpy(Buffer, Save3DRawData.pBuffer, Save3DRawData.szSize);

	ret = 1;
	return ret;
}
void CMNgManager::SaveImage_Segmentation_AI(InspPartInfo* pInspBoardInfo, CString sPath)
{
	CString sInputPath = sPath + L"_AI_I.jpg";
	CString sOutputPath = sPath + L"_AI_O.jpg";

	int roiSizeX = pInspBoardInfo->partImgBuf.nImageSizeX;
	int roiSizeY = pInspBoardInfo->partImgBuf.nImageSizeY;

	cv::Mat cvPartAI_tw((int)roiSizeY, (int)roiSizeX, CV_8UC1, pInspBoardInfo->partImgBuf.imgTop_W);
	cv::Mat cvPartAI_mr((int)roiSizeY, (int)roiSizeX, CV_8UC1, pInspBoardInfo->partImgBuf.imgMiddle_R);
	cv::Mat cvPartAI_mb((int)roiSizeY, (int)roiSizeX, CV_8UC1, pInspBoardInfo->partImgBuf.imgMiddle_B);

	// Gray → 3채널로 복제
	cv::Mat ImgMerge[3];

	//이미지 Inline 실제 테스트용
	//R: bottomRed, G: bMiddleBlue, B: TopGreen
	ImgMerge[0] = cvPartAI_mb;//B
	ImgMerge[1] = cvPartAI_mr;//G
	ImgMerge[2] = cvPartAI_tw;//R

	//auto it = g_pMPTI->m_vAIPart2DBuf.find(pInspBoardInfo->nPartIDOrg);
	cv::Mat ImgInput;
	cv::merge(ImgMerge, 3, ImgInput);   // CV_8UC3
	cv::Mat ImgOutput((int)roiSizeY, (int)roiSizeX, CV_8UC1, pInspBoardInfo->partImgBuf.imgAI);

	cv::imwrite(std::string(CT2A(sInputPath)), ImgInput);
	cv::imwrite(std::string(CT2A(sOutputPath)), ImgOutput);
}
UCHAR* GetLightBufPtr(InspPartInfo* pInspBoardInfo, int lightIdx)
{
	switch (lightIdx)
	{
	case eColor2DImg_Light_Top_R: return pInspBoardInfo->partImgBuf.imgTop_R;
	case eColor2DImg_Light_Top_G: return pInspBoardInfo->partImgBuf.imgTop_G;
	case eColor2DImg_Light_Top_B: return pInspBoardInfo->partImgBuf.imgTop_B;
	case eColor2DImg_Light_Top_W: return pInspBoardInfo->partImgBuf.imgTop_W;
	case eColor2DImg_Light_Mid_R: return pInspBoardInfo->partImgBuf.imgMiddle_R;
	case eColor2DImg_Light_Mid_B: return pInspBoardInfo->partImgBuf.imgMiddle_B;
	case eColor2DImg_Light_Btm_R: return pInspBoardInfo->partImgBuf.imgBottom_R;
	case eColor2DImg_Light_Btm_B: return pInspBoardInfo->partImgBuf.imgBottom_B;
	default: return nullptr;
	}
}

void CMNgManager::SaveImage_2DColor(InspPartInfo* pInspBoardInfo, CString sPath)
{
	int roiSizeX = pInspBoardInfo->partImgBuf.nImageSizeX;
	int roiSizeY = pInspBoardInfo->partImgBuf.nImageSizeY;

	cv::Mat cvPart2D_Red;
	cv::Mat cvPart2D_Green;
	cv::Mat cvPart2D_Blue;

	std::vector<cv::Mat> ImgMerge(3);

	int rIdx = g_pMPTI->m_nRedLgtPos_2DImg;
	int gIdx = g_pMPTI->m_nGreenLgtPos_2DImg;
	int bIdx = g_pMPTI->m_nBlueLgtPos_2DImg;

	// 범위 체크 + 기본값(R=0,G=1,B=2)
	if (rIdx < 0 || rIdx >= 8) rIdx = eColor2DImg_Light_Top_R;
	if (gIdx < 0 || gIdx >= 8) gIdx = eColor2DImg_Light_Top_G;
	if (bIdx < 0 || bIdx >= 8) bIdx = eColor2DImg_Light_Top_B;

	UCHAR* pR = GetLightBufPtr(pInspBoardInfo, rIdx);
	UCHAR* pG = GetLightBufPtr(pInspBoardInfo, gIdx);
	UCHAR* pB = GetLightBufPtr(pInspBoardInfo, bIdx);

	if (!pR || !pG || !pB)
	{
		return;
	}

	const int w = (int)roiSizeX;   // ROI/이미지 크기
	const int h = (int)roiSizeY;

	cvPart2D_Red = cv::Mat(h, w, CV_8UC1, pR);
	cvPart2D_Green = cv::Mat(h, w, CV_8UC1, pG);
	cvPart2D_Blue = cv::Mat(h, w, CV_8UC1, pB);

	ImgMerge[0] = cvPart2D_Blue;
	ImgMerge[1] = cvPart2D_Green;
	ImgMerge[2] = cvPart2D_Red;

	cv::Mat outBGR;
	cv::merge(ImgMerge, outBGR);

	cv::imwrite(std::string(CT2A(sPath)), outBGR);

	return;
}

#pragma region exposure_implementation

int  MPTI_SetNgPath(wchar_t* path)
{
	if (g_pNgMng == NULL)
		return eMR_FAIL;

	g_pNgMng->SetPath(path);

	return eMR_SUCCESS;
}

float * MPTI_Save3DImage(wchar_t* path, int * nSizeX, int * nSizeY, int exportSaveOption)
{
	if (g_pNgMng == NULL)
		return NULL;

	return g_pNgMng->Save3DImage(path, nSizeX, nSizeY, exportSaveOption);
}

UCHAR*  MPTI_SavePVImageNG(int* nWidth, int* nHeight, int* nWidthSrc)
{	
	return g_pNgMng->GetPVImage(nWidth, nHeight, nWidthSrc);
}
UCHAR*  MPTI_SavePVImageNG_Foreign(int* nWidth, int* nHeight, int* nWidthSrc)
{
	return g_pNgMng->GetPVImage_Foreign(nWidth, nHeight, nWidthSrc);
}

/*
// SHKang 2017/04/05
int MPTI_Set_JpegImage(jpeg_formatType type, jpeg_formatQuality quality)
{
	if (g_pNgMng == NULL)
		return eMR_FAIL;
	g_pNgMng->Set_JpegImage(type, quality);

	return eMR_SUCCESS;
}


// SHKang 2017/04/05
int MPTI_Save_JpegImage_1ch(UCHAR * ucSrc, int width, int height, wchar_t* file_path)
{
	if (g_pNgMng == NULL)
		return eMR_FAIL;
	int nRet = g_pNgMng->Save_JpegImage_1ch(ucSrc, width, height, file_path);

	return eMR_SUCCESS;
}
// SHKang 2017/04/05
int MPTI_Save_JpegImage_3ch(UCHAR * srcR, UCHAR * srcG, UCHAR * srcB, int width, int height, wchar_t* file_path)
{
	if (g_pNgMng == NULL)
		return eMR_FAIL;
	int nRet = g_pNgMng->Save_JpegImage_3ch(srcR, srcG, srcB, width, height, file_path);

	return eMR_SUCCESS;
}
*/
// SHKang 2017/04/05
int MPTI_Save_JpegImage(UCHAR * ucSrc, int width, int height, int nChannel, _IM_COLOR imColor, wchar_t* file_path, int nJpegQuality)
{
	if (g_pNgMng == NULL)
		return eMR_FAIL;

	int nRet = g_pNgMng->Save_JpegImage(ucSrc, width, height, nChannel, imColor, file_path, nJpegQuality);

	return eMR_SUCCESS;
}

void MPTI_SetData(UCHAR * ucSrc, UCHAR * ucDst, int width, int height, int nWidthStep, int nBand)
{
	if (g_pNgMng == NULL || ucSrc == NULL || ucDst == NULL || (nBand != 1 && nBand != 3))
		return;
	if (nWidthStep == width)
		memcpy(ucDst, ucSrc, width * height * nBand * sizeof(UCHAR));
	else // CJY 2021/3/22 수정하면서 else 아래단 타지 않을 것임 혹시 모를 예외처리 구문 남겨툼 (이구문을 타면 이미지 오른쪽에 쓰레기 값이 쌓인체로 들어갈 것 )
	{
		IppStatus sts;
		IppiSize iSize = { width, height };
		if (nBand == 3)
			sts = ippiCopy_8u_C3R(ucSrc, width * 3, ucDst, nWidthStep * 3, iSize);
		else if (nBand == 1)
			sts = ippiCopy_8u_C1R(ucSrc, width, ucDst, nWidthStep, iSize);
	}
}

void CMNgManager::GetUserMapImage(InspRoiImgBuf partImgBuf, UCHAR* ptrbyDst, int nType)
{
	int nImgW = partImgBuf.nImageSizeX;
	int nImgH = partImgBuf.nImageSizeY;

	UCHAR* ucimgTop_R = partImgBuf.imgTop_R;
	UCHAR* ucimgTop_G = partImgBuf.imgTop_G;
	UCHAR* ucimgTop_B = partImgBuf.imgTop_B;
	UCHAR* ucimgTop_W = partImgBuf.imgTop_W;

	UCHAR* ucimgMiddle_R = partImgBuf.imgMiddle_R;
	UCHAR* ucimgMiddle_B = partImgBuf.imgMiddle_B;
	float fGR = m_fFatorMGR;
	float fGB = m_fFatorMGB;
	float fitMR = fGR <= 0.0 ? 0.4f : fGR;
	float fitMB = fGB <= 0.0 ? 0.8f : fGB;
	cv::Mat imgR(partImgBuf.nImageSizeY, partImgBuf.nImageSizeX, CV_8UC1, ucimgTop_R);
	cv::Mat imgB(partImgBuf.nImageSizeY, partImgBuf.nImageSizeX, CV_8UC1, ucimgTop_R);
	cv::Mat imgMG;
	cv::addWeighted(imgR, fitMR, imgB, fitMB, 1, imgMG);
	 //= GetGreenImage(ucimgMiddle_R, ucimgMiddle_B, nImgW, nImgH);
	UCHAR* ucimgMiddle_G = imgMG.data;

	UCHAR* ucimgBottom_R = partImgBuf.imgBottom_R;
	UCHAR* ucimgBottom_B = partImgBuf.imgBottom_B;

	if (ucimgTop_R == NULL || ucimgTop_G == NULL || ucimgTop_B == NULL || ucimgTop_W == NULL ||
		ucimgMiddle_R == NULL || ucimgMiddle_G == NULL || ucimgMiddle_B == NULL || ucimgBottom_R == NULL || ucimgBottom_B == NULL ||
		nImgW <= 0 || nImgH <= 0)
		return;

	GetUserMapImage(ucimgTop_R, ucimgTop_G, ucimgTop_B, ucimgTop_W,
		ucimgMiddle_R, ucimgMiddle_G, ucimgMiddle_B, ucimgBottom_R, ucimgBottom_B,
		ptrbyDst, nType, nImgW, nImgH);

#if _DEBUG

	cv::Mat ImgTR(nImgH, nImgW, CV_8UC1, ucimgTop_R);
	cv::Mat ImgTG(nImgH, nImgW, CV_8UC1, ucimgTop_G);
	cv::Mat ImgTB(nImgH, nImgW, CV_8UC1, ucimgTop_B);
	cv::Mat ImgTW(nImgH, nImgW, CV_8UC1, ucimgTop_W);

	cv::Mat ImgMR(nImgH, nImgW, CV_8UC1, ucimgMiddle_R);
	cv::Mat ImgMB(nImgH, nImgW, CV_8UC1, ucimgMiddle_B);

	cv::Mat ImgBR(nImgH, nImgW, CV_8UC1, ucimgBottom_R);
	cv::Mat ImgBB(nImgH, nImgW, CV_8UC1, ucimgBottom_B);

	cv::Mat ImgRst(nImgH, nImgW, CV_8UC1, ptrbyDst);

#endif
}
void CMNgManager::GetUserMapImage(UCHAR* ptrbyTopRedBuffer, UCHAR* ptrbyTopGreenBuffer, UCHAR* ptrbyTopBlueBuffer, UCHAR* ptrbyTopWhiteBuffer,
	UCHAR* ptrbyMidRedBuffer, UCHAR* ptrbyMidGreenBuffer, UCHAR* ptrbyMidBlueBuffer, UCHAR* ptrbyBotRedBuffer, UCHAR* ptrbyBotBlueBuffer,
	UCHAR* ptrbyDst, int nType, int nWidth, int nHeight)
{
	LightTypeBuf sLightBuf;
	sLightBuf.m_pucTRed = ptrbyTopRedBuffer;
	sLightBuf.m_pucTGreen = ptrbyTopGreenBuffer;
	sLightBuf.m_pucTBlue = ptrbyTopBlueBuffer;
	sLightBuf.m_pucTWhite = ptrbyTopWhiteBuffer;
	sLightBuf.m_pucMRed = ptrbyMidRedBuffer;
	sLightBuf.m_pucMGreen = ptrbyMidGreenBuffer;
	sLightBuf.m_pucMBlue = ptrbyMidBlueBuffer;
	sLightBuf.m_pucMWhite = NULL;
	sLightBuf.m_pucBRed = ptrbyBotRedBuffer;
	sLightBuf.m_pucBGreen = NULL;
	sLightBuf.m_pucBBlue = ptrbyBotBlueBuffer;
	sLightBuf.m_pucBWhite = NULL;
	sLightBuf.m_nImgWidth = nWidth;
	sLightBuf.m_nImgHeight = nHeight;
	sLightBuf.m_nROIImgWidth = nWidth;
	sLightBuf.m_nROIImgHeight = nHeight;
	sLightBuf.m_dROIX = 0;
	sLightBuf.m_dROIY = 0;
	sLightBuf.m_pnRedValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
	sLightBuf.m_pnGreenValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
	sLightBuf.m_pnBlueValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
	sLightBuf.m_pnWhiteValue = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
	sLightBuf.m_pnPosition = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
	sLightBuf.m_pnCalculation = g_pMManager->pem_new<int>(true, LIGHT_CNT, (PCHAR)__FUNCTION__, __LINE__);
	sLightBuf.m_nImgCnt = 1;// m_sUserLightData[nType].m_byLightCnt;
	memset(sLightBuf.m_pnRedValue, 0, LIGHT_CNT * sizeof(int));
	memset(sLightBuf.m_pnGreenValue, 0, LIGHT_CNT * sizeof(int));
	memset(sLightBuf.m_pnBlueValue, 0, LIGHT_CNT * sizeof(int));
	memset(sLightBuf.m_pnWhiteValue, 0, LIGHT_CNT * sizeof(int));
	memset(sLightBuf.m_pnPosition, 0, LIGHT_CNT * sizeof(int));
	memset(sLightBuf.m_pnCalculation, 0, LIGHT_CNT * sizeof(int));
	for (int b = 0; b < LIGHT_CNT; b++)
	{
		sLightBuf.m_pnRedValue[b] = m_sUserLightData[nType].m_byArrLightData[eLightData_Red][b];
		sLightBuf.m_pnGreenValue[b] = m_sUserLightData[nType].m_byArrLightData[eLightData_Green][b];
		sLightBuf.m_pnBlueValue[b] = m_sUserLightData[nType].m_byArrLightData[eLightData_Blue][b];
		sLightBuf.m_pnWhiteValue[b] = m_sUserLightData[nType].m_byArrLightData[eLightData_White][b];
		sLightBuf.m_pnPosition[b] = m_sUserLightData[nType].m_byArrLightData[eLightData_Position][b];
		sLightBuf.m_pnCalculation[b] = m_sUserLightData[nType].m_byArrLightData[eLightData_Calculation][b];
	}
// 	m_procMil->SaveWorkImg(sLightBuf.m_pucTRed, sLightBuf.m_nImgWidth, sLightBuf.m_nImgHeight, _T("Src_ImageTR.bmp"));
// 	m_procMil->SaveWorkImg(sLightBuf.m_pucTGreen, sLightBuf.m_nImgWidth, sLightBuf.m_nImgHeight, _T("Src_ImageTG.bmp"));
// 	m_procMil->SaveWorkImg(sLightBuf.m_pucTBlue, sLightBuf.m_nImgWidth, sLightBuf.m_nImgHeight, _T("Src_ImageTB.bmp"));
// 	m_procMil->SaveWorkImg(sLightBuf.m_pucTWhite, sLightBuf.m_nImgWidth, sLightBuf.m_nImgHeight, _T("Src_ImageTW.bmp"));
// 	m_procMil->SaveWorkImg(sLightBuf.m_pucMRed, sLightBuf.m_nImgWidth, sLightBuf.m_nImgHeight, _T("Src_ImageMR.bmp"));
// 	m_procMil->SaveWorkImg(sLightBuf.m_pucMBlue, sLightBuf.m_nImgWidth, sLightBuf.m_nImgHeight, _T("Src_ImageMB.bmp"));
// 	m_procMil->SaveWorkImg(sLightBuf.m_pucBRed, sLightBuf.m_nImgWidth, sLightBuf.m_nImgHeight, _T("Src_ImageBR.bmp"));
// 	m_procMil->SaveWorkImg(sLightBuf.m_pucBBlue, sLightBuf.m_nImgWidth, sLightBuf.m_nImgHeight, _T("Src_ImageBB.bmp"));

	g_pMPTI->m_InspMng->m_InspAlgo.ROIImageClaculCompose(sLightBuf, ptrbyDst);

/*	m_procMil->SaveWorkImg(ptrbyDst, sLightBuf.m_nImgWidth, sLightBuf.m_nImgHeight, _T("Rst_Image.bmp"));*/
	Delete_1DArray(&sLightBuf.m_pnRedValue);
	Delete_1DArray(&sLightBuf.m_pnGreenValue);
	Delete_1DArray(&sLightBuf.m_pnBlueValue);
	Delete_1DArray(&sLightBuf.m_pnWhiteValue);
	Delete_1DArray(&sLightBuf.m_pnPosition);
	Delete_1DArray(&sLightBuf.m_pnCalculation);
}

BOOL CMNgManager::UserMapSet(int nUserMap, lightData* pRedLightData)
{
	BOOL bRet = FALSE;

	m_nUserMap = nUserMap;
	for (int n = 0; n < 3; n++)
	{
		m_sUserLightData[n].m_byLightCnt = pRedLightData[n].m_byLightCnt;
		for (int a = 0; a < LIGHT_DATA_CNT; a++)
		{
			for (int b = 0; b < LIGHT_CNT; b++)
			{
				byte byData = pRedLightData[n].m_byArrLightData[a][b];
				m_sUserLightData[n].m_byArrLightData[a][b] = byData;
			}
		}
	}
	bRet = TRUE;
	if (g_pInspMng->m_bNewInspection)
	{
		BOOL bRet = FALSE;
		for (int n = 0; n < 3; n++)
		{
			PIAL::PInspAlgo_Lib::m_sLightData[n].m_byLightCnt = pRedLightData[n].m_byLightCnt;
			for (int a = 0; a < LIGHT_DATA_CNT; a++)
			{
				for (int b = 0; b < LIGHT_CNT; b++)
				{
					byte byData = pRedLightData[n].m_byArrLightData[a][b];
					PIAL::PInspAlgo_Lib::m_sLightData[n].m_byArrLightData[a][b] = byData;
				}
			}
		}
		bRet = TRUE;
	}
	return bRet;
}

#pragma endregion exposure_implementation