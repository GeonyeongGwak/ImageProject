#include "StdAfx.h"
#include "ProcMil_Color.h"
#include "MemoryManager.h"

CProcMil_Color::CProcMil_Color(void)
{
	m_className = _T("CProcMil_Color");
	m_milBlobFeature = M_NULL;
	m_milBlobResult = M_NULL;

}


CProcMil_Color::~CProcMil_Color(void)
{

}


int CProcMil_Color::InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib)
{
	if(milApp == M_NULL || milSys == M_NULL)
	{
		return eMCOL_FAIL;	
	}

	CProcMil :: InitMil(milApp, milSys,bUseImagePilLib);
	InitMilAlgoBlob();

// 	Im::Blob::blobAllocFeatureList(*m_milSys, &m_milBlobFeature);   //Allocate blob feature list.
// 	Im::Blob::blobAllocResult(*m_milSys, &m_milBlobResult);         //Allocate blob result.
// 
// 	//add feature list.
// 	Im::Blob::blobSelectFeature( m_milBlobFeature, M_AREA);
// 	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BOX_X_MIN);
// 	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BOX_X_MAX);
// 	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BOX_Y_MIN);
// 	Im::Blob::blobSelectFeature( m_milBlobFeature, M_BOX_Y_MAX);

	return eMCOL_SUCCESS;
}

int CProcMil_Color::FreeMil()
{
#ifndef _DISABLE_MIL	
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
	FreeMilAlgoBlob();

	CProcMil :: FreeMil();
#endif

	return eMCOL_SUCCESS;
}


void CProcMil_Color::GetPtrClipBuf(void* milBuf, UCHAR* userDst, int cx, int cy, int width, int height)
{
	Im::PIL_ID milTemp = M_NULL;

	milTemp = AllocClipBuff((Im::PIL_ID)milBuf, cx, cy, width, height);
	Im::Buf::Get(milTemp, userDst);

	FreeMilImageBuff(&milTemp);
}

void CProcMil_Color::CvtGrayToColor(UCHAR* userSrc, UCHAR* userDst, int width, int height, double color)
{
	/*Im::PIL_ID milTempColor = M_NULL;
	Im::PIL_ID milTempSrc = M_NULL;
	Im::PIL_ID milTempDest = M_NULL;

	milTempDest = AllocBuffColor(width, height, M_COLOR_BLACK);
    milTempColor = AllocBuffColor(width, height, color);

	milTempSrc = AllocBuff(width, height, 0);
	Im::Buf::Put(milTempSrc, userSrc);

	ArithImage(milTempColor, milTempSrc, milTempDest, M_AND);

	Im::Buf::GetColor(milTempDest, M_PACKED + M_BGR24, M_ALL_BANDS, userDst);

	FreeMilImageBuff(&milTempDest);
	FreeMilImageBuff(&milTempColor);
	FreeMilImageBuff(&milTempSrc);*/

	Im::PIL_ID milTempColor = M_NULL;
	Im::PIL_ID milTempSrc = M_NULL;
	Im::PIL_ID milTemp = M_NULL;



	milTemp = AllocBuffColor(width, height, M_COLOR_BLACK);
	milTempColor = AllocBuffColor(width, height, color);

	milTempSrc = AllocBuff(width, height);
	Im::Buf::Put(milTempSrc, userSrc);

	ArithImage(milTempColor, milTempSrc, milTemp, M_AND);

	//UCHAR* userTemp = new UCHAR[width * height * 3];
	UCHAR* userTemp = g_pMManager->pem_new<UCHAR>(true, width * height * 3, (PCHAR)__FUNCTION__, __LINE__);
	Im::Buf::GetColor(milTemp, M_PACKED + M_BGR24, M_ALL_BANDS, userTemp);

	UCHAR data = 0;
	BOOL flag = TRUE;
	UCHAR* srcTemp = userTemp;
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			flag = TRUE;
			for(int i = 0; i < 3; i++)
			{
				data = *srcTemp++;//userTemp[(y * (width * 3)) + ((3 * x) + i)];

				if(data != 0)
					flag = FALSE;
			}
			
 			if(flag)
 			{
				srcTemp -= 3;
				for(int i = 0; i < 3; i++)
				{
					//userTemp[(y * (width * 3)) + ((3 * x) + i)] = 255;
					
					*srcTemp++ = 255;
				}
			}
		}
	}

	memcpy_s(userDst, (sizeof(UCHAR) * width * height * 3), userTemp, (sizeof(UCHAR) * width * height * 3));
	//delete userTemp;
	g_pMManager->pem_delete(userTemp, false);

	FreeMilImageBuff(&milTempColor);
	FreeMilImageBuff(&milTempSrc);
	FreeMilImageBuff(&milTemp);
	
}

void CProcMil_Color::SumImageToColor(UCHAR* userSrc1, UCHAR* userSrc2, UCHAR* userDst, int width, int height, double color1 , double color2)
{
	if(userSrc1  == NULL || userSrc2 == NULL || userDst == NULL || width < 0 || height < 0)
		return;

	UCHAR* tempSrc1 = NULL;
	UCHAR* tempSrc2 = NULL;
	Im::PIL_ID tempMil1 = M_NULL;
	Im::PIL_ID tempMil2 = M_NULL;
	Im::PIL_ID tempMilResult = M_NULL;
	
	//tempSrc1 = new UCHAR[width * height * 3];
	tempSrc1 = g_pMManager->pem_new<UCHAR>(true, width * height * 3, (PCHAR)__FUNCTION__, __LINE__);
	//memset(tempSrc1, 0, (sizeof(UCHAR) * width * height * 3));

	//tempSrc2 = new UCHAR[width * height * 3];
	tempSrc2 = g_pMManager->pem_new<UCHAR>(true, width * height * 3, (PCHAR)__FUNCTION__, __LINE__);
	//memset(tempSrc2, 0, (sizeof(UCHAR) * width * height * 3));

	EdgeDetect(userSrc2, userSrc2, width, height);

	CvtGrayToColor(userSrc1, tempSrc1, width, height, color1);
	CvtGrayToColor(userSrc2, tempSrc2, width, height, color2);

	tempMil1 = AllocBuffColor(width, height);
	tempMil2 = AllocBuffColor(width, height);
	tempMilResult = AllocBuffColor(width, height, M_COLOR_BLACK);

	Im::Buf::Put(tempMil1, tempSrc1);
	Im::Buf::Put(tempMil2, tempSrc2);

	ArithImage(tempMil1, tempMil2, tempMilResult, M_ADD );

	Im::Buf::GetColor(tempMilResult, M_PACKED + M_BGR24, M_ALL_BANDS, userDst);

	FreeMilImageBuff(&tempMil1);
	FreeMilImageBuff(&tempMil2);
	FreeMilImageBuff(&tempMilResult);
	/*delete tempSrc1;
	delete tempSrc2;*/
	g_pMManager->pem_delete(tempSrc1, false);
	g_pMManager->pem_delete(tempSrc2, false);
}








int CProcMil_Color::CalcBlob(Im::PIL_ID milSrc, Im::PIL_ID milDst, int minArea)
{
	MIL_INT cnts = 0;

	Im::Blob::blobControl(m_milBlobResult, M_FOREGROUND_VALUE, M_NONZERO);
	Im::Blob::blobCalculate(milSrc, M_NULL, m_milBlobFeature, m_milBlobResult);

	//blob select
	Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_AREA, M_LESS, minArea, M_NULL);

	Im::Blob::blobFill(m_milBlobResult, milDst, M_EXCLUDED_BLOBS, 0);   //redraw excluded blob (value : 0) -> erase
	Im::Blob::blobFill(m_milBlobResult, milDst, M_INCLUDED_BLOBS, 255);   //redraw included blob (value : 255) -> draw

	Im::Blob::blobGetNumber(m_milBlobResult, &cnts);

	return (int)cnts;
}

CRect CProcMil_Color::GetBlobBox(Im::PIL_ID milSrc, int minArea)
{
	MIL_INT cnts = 0;
	long* pLeft = NULL;
	long* pRight = NULL;
	long* pTop = NULL;
	long* pBottom = NULL;
	CRect ret(0,0,0,0);

	Im::Blob::blobControl(m_milBlobResult, M_FOREGROUND_VALUE, M_NONZERO);
	Im::Blob::blobCalculate(milSrc, M_NULL, m_milBlobFeature, m_milBlobResult);
	//blob select
	Im::Blob::blobSelect(m_milBlobResult, M_EXCLUDE, M_AREA, M_LESS, minArea, M_NULL);
	Im::Blob::blobGetNumber(m_milBlobResult, &cnts);

	int index = GetBlobIndex_MaxArea((int)cnts);

	if(cnts > 0)
	{
		/*pLeft = new long[cnts];
		pRight = new long[cnts];
		pTop = new long[cnts];
		pBottom = new long[cnts];*/
		pLeft = g_pMManager->pem_new<long>(true, cnts, (PCHAR)__FUNCTION__, __LINE__);
		pRight = g_pMManager->pem_new<long>(true, cnts, (PCHAR)__FUNCTION__, __LINE__);
		pTop = g_pMManager->pem_new<long>(true, cnts, (PCHAR)__FUNCTION__, __LINE__);
		pBottom = g_pMManager->pem_new<long>(true, cnts, (PCHAR)__FUNCTION__, __LINE__);

		Im::Blob::blobGet_BOX(m_milBlobResult, pLeft,pTop,pRight,pBottom);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_X_MIN + M_TYPE_LONG , pLeft);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_X_MAX + M_TYPE_LONG , pRight);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_Y_MIN + M_TYPE_LONG , pTop);
// 		Im::Blob::blobGetResult(m_milBlobResult, M_BOX_Y_MAX + M_TYPE_LONG , pBottom);

		ret.left = pLeft[index];
		ret.right = pRight[index];
		ret.top = pTop[index];
		ret.bottom = pBottom[index];

		/*delete pLeft;
		delete pRight;
		delete pTop;
		delete pBottom;*/
		g_pMManager->pem_delete(pLeft, true);
		g_pMManager->pem_delete(pRight, true);
		g_pMManager->pem_delete(pTop, true);
		g_pMManager->pem_delete(pBottom, true);
	}
	
	return ret;
}

int CProcMil_Color::GetBlobIndex_MaxArea(int cnts)
{
	double * area;
	//area = (double *)malloc(sizeof( double) * cnts);
	area = (double *)g_pMManager->pem_malloc(sizeof(double) * cnts, (PCHAR)__FUNCTION__, __LINE__);

	Im::Blob::blobGetResult(m_milBlobResult, M_AREA + M_TYPE_DOUBLE, area);

	double max = area[0];
	int index = 0;
	for( int i = 1; i < cnts ; i ++)
	{
		if( max < area[i])
		{
			index = i;
			max = area[i];
		}
	}

	//free(area);
	g_pMManager->pem_free(area);

	return index;
}

void CProcMil_Color::MakeBlobImg(UCHAR* src, UCHAR* dst, int width, int height, int iteration)
{
	Im::PIL_ID srcTemp = M_NULL;
	int blobCnts = 0;

	srcTemp = AllocBuff(width, height);
	Im::Buf::Put(srcTemp, src);

	//
	Binarize(srcTemp, srcTemp, 1, FALSE);

	//
	MorClose(srcTemp, srcTemp, 3);
	FillHoleBlob(srcTemp, srcTemp, m_milBlobResult);
	blobCnts = CalcBlob(srcTemp, srcTemp, 10);

	MorDilate(srcTemp, srcTemp, iteration);
	FillHoleBlob(srcTemp, srcTemp, m_milBlobResult);

	Im::Buf::Get(srcTemp, dst);

	FreeMilImageBuff(&srcTemp);
}

void CProcMil_Color::GetPolygonCoordi(UCHAR* src, int width, int height, int margin, POINT* retPt)
{
	Im::PIL_ID srcTemp = M_NULL;
	CRect rect(0,0,0,0);
	POINT pt[5];

	srcTemp = AllocBuff(width, height);
	Im::Buf::Put(srcTemp, src);

	rect = GetBlobBox(srcTemp, 1);

	rect.InflateRect(margin*3, margin);

	//   *00
	//   0 0
	pt[0].x = rect.left;
	pt[0].y = rect.top;


	//   000
	//   * 0
	pt[1].x = rect.left;
	pt[1].y = rect.bottom;


	//   000
	//   0 *
	pt[2].x = rect.right;
	pt[2].y = rect.bottom;

	//   00*
	//   0 0
	pt[3].x = rect.right;
	pt[3].y = rect.top;

	//   0*0
	//   0 0
	pt[4].x = (pt[0].x + pt[3].x)/2;
	pt[4].y = (pt[0].y + pt[3].y)/2;


	memcpy_s(retPt, (sizeof(POINT) * 5), pt, (sizeof(POINT) * 5));

	FreeMilImageBuff(&srcTemp);
}

void CProcMil_Color::GetPolygonCoordi_Pt7(UCHAR* src, int width, int height, int margin, POINT* retPt)
{
	Im::PIL_ID srcTemp = M_NULL;
	CRect rect(0,0,0,0);
	POINT pt[7];

	srcTemp = AllocBuff(width, height);
	Im::Buf::Put(srcTemp, src);

	rect = GetBlobBox(srcTemp, 1);

	rect.InflateRect(margin * 3, margin);

	//   *00
	//     0
	//   000
	pt[0].x = rect.left;
	pt[0].y = rect.top;

	//   000
	//     0
	//   * 0
	pt[1].x = rect.left;
	pt[1].y = rect.bottom;

	//   000
	//     0
	//   0*0
	pt[2].x = (rect.right + rect.left) / 2;
	pt[2].y = rect.bottom;

	//   000
	//     0
	//   00*
	pt[3].x = rect.right;
	pt[3].y = rect.bottom;

	//   000
	//     *
	//   000
	pt[4].x = rect.right;
	pt[4].y = (rect.top + rect.bottom) / 2;

	//   00*
	//     0
	//   000
	pt[5].x = rect.right;
	pt[5].y = rect.top;

	//   0*0
	//     0
	//   000
	pt[6].x = (rect.right + rect.left) / 2;
	pt[6].y = rect.top;

	memcpy_s(retPt, (sizeof(POINT) * 7), pt, (sizeof(POINT) * 7));

	FreeMilImageBuff(&srcTemp);
}

void CProcMil_Color::GetPolygonCoordi_CIE(UCHAR* src, int width, int height, int offsetX, int offsetY, POINT* retPt)
{
	Im::PIL_ID srcTemp = M_NULL;
	CRect rect(0,0,0,0);
	POINT pt[5];

	srcTemp = AllocBuff(width, height);
	Im::Buf::Put(srcTemp, src);

	rect = GetBlobBox(srcTemp, 1);

	int margin = 10; //pixel  

	//   *00
	//   0 0
	pt[0].x = (rect.left - offsetX) - margin;
	pt[0].y = (rect.top - offsetY) - margin;


	//   000
	//   * 0
	pt[1].x = (rect.left - offsetX) - margin;
	pt[1].y = (rect.bottom - offsetY) + margin;


	//   000
	//   0 *
	pt[2].x = (rect.right - offsetX) + margin;
	pt[2].y = (rect.bottom - offsetY) + margin;

	//   00*
	//   0 0
	pt[3].x = (rect.right - offsetX) + margin;
	pt[3].y = (rect.top - offsetY) - margin;

	//   0*0
	//   0 0
	pt[4].x = (rect.left + (rect.Width()/2) - offsetX);
	pt[4].y = (rect.top - offsetY) - margin;

	memcpy_s(retPt, (sizeof(POINT) * 5), pt, (sizeof(POINT) * 5));
	// 	for(int i = 0; i<5; i++)
	// 	{
	// 		retPt[i].x = pt[i].x;
	// 		retPt[i].y = pt[i].y;
	// 	}

	FreeMilImageBuff(&srcTemp);
}


void CProcMil_Color::ImageClip_CIE(UCHAR* src, int orgSizeX, int orgSizeY, int band, UCHAR* dst, int cx, int cy, int dstSizeX, int dstSizeY)
{
	Im::PIL_ID milSrc = M_NULL;
	Im::PIL_ID milClip = M_NULL;
	if(band == 3)
	{
		milSrc = AllocBuffColor(orgSizeX, orgSizeY, 0);
		Im::Buf::PutColor(milSrc, M_PACKED + M_BGR24, M_ALL_BANDS, src);
	}

	milClip = AllocClipBuffColor(milSrc, cx, cy, dstSizeX, dstSizeY);
	Im::Buf::GetColor(milClip, M_PACKED + M_BGR24, M_ALL_BANDS, dst);

	FreeMilImageBuff(&milSrc);
	FreeMilImageBuff(&milClip);
}
