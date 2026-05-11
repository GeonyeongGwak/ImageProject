#include "stdafx.h"
#include "BilinearInterpolation.h"
#include "MemoryManager.h"

BilinearInterpolation::BilinearInterpolation()
{
	ptReference = nullptr; //예상좌표
	ptObject= nullptr; //실제 좌표

	x_num = 0;
	y_num = 0;
}


BilinearInterpolation::~BilinearInterpolation()
{
	Clear();
}

void BilinearInterpolation::Clear()
{
	if (ptReference)
	{
		for (int i = 0; i<y_num; i++)
		{
			//delete[](ptReference[i]);
			g_pMManager->pem_delete(ptReference[i], true);
		}
		//delete[] ptReference;
		g_pMManager->pem_delete(ptReference, true);
		ptReference = nullptr;
	}

	if (ptObject)
	{
		for (int i = 0; i<y_num; i++)
		{
			//delete[](ptObject[i]);
			g_pMManager->pem_delete(ptObject[i], true);
		}
		//delete[] ptObject;
		g_pMManager->pem_delete(ptObject, true);
		ptObject = nullptr;
	}

	x_num = 0;
	y_num = 0;
}

bool BilinearInterpolation::HasData()
{
	if (ptReference == nullptr || ptObject == nullptr)
		return false;

	if (x_num == 0 || y_num == 0)
		return false;


	return true;
}

bool BilinearInterpolation::ReadFile(const CString& filePath, int mode)
{
	if (_taccess(filePath, 0) == -1)
	{
		return false;
	}

	CStdioFile file;
	if (file.Open(filePath, CFile::modeRead | CFile::typeText)) {
		CString  strTemp = _T("");
		TCHAR cTemp;
		int m_nTableCode = 0;
		int i = 0;

		while (file.ReadString(strTemp)) {
			strTemp;

			cTemp = strTemp.GetAt(0);
			if (cTemp == '@')
			{
				if (strTemp == _T("@HEADER"))
				{
					m_nTableCode = 0;
				}
				else if (strTemp == _T("@BODY X"))
				{
					m_nTableCode = 1;
				}
				else if (strTemp == _T("@BODY Y"))
				{
					m_nTableCode = 2;
				}
			}
			else
			{
				if (m_nTableCode == 0)
				{
					Set_CalibHeader(strTemp.GetBuffer(), mode);
				}
				else if (m_nTableCode == 1)
				{
					Set_Calib_BodyX(strTemp.GetBuffer(), i, mode);
				}
				else if (m_nTableCode == 2)
				{
					Set_Calib_BodyY(strTemp.GetBuffer(), i, mode);
				}
			}


			i++;
		}

		file.Close();
	}

	
	return true;
}
void BilinearInterpolation::Set_CalibHeader(TCHAR * strData, int mode)
{
	if (strData == NULL) return;
	TCHAR seps[] = _T(" =\n");
	TCHAR* next_token = NULL;

	CString type;
	CString data;

	type = _tcstok_s(strData, seps, &next_token);
	data = _tcstok_s(NULL, seps, &next_token);

	CString strQuery1, strQuery2;
	if (type == _T("X"))
	{
		x_num = _tstoi(data);
	}
	else if (type == _T("Y"))
	{
		y_num = _tstoi(data);

		if (mode == 0)
		{
			if (ptReference)
			{
				for (int i = 0; i<y_num; i++)
					//delete[](ptReference[i]);
					g_pMManager->pem_delete(ptReference[i], true);
				
				//delete[] ptReference;
				g_pMManager->pem_delete(ptReference, true);
				ptReference = nullptr;
			}

			//ptReference = new POINT_64F*[y_num];
			ptReference = g_pMManager->pem_new<POINT_64F*>(true, y_num, (PCHAR)__FUNCTION__, __LINE__);

			for (size_t i = 0; i < y_num; i++)
				//ptReference[i] = new POINT_64F[x_num]; 
				ptReference[i] = g_pMManager->pem_new<POINT_64F>(true, x_num, (PCHAR)__FUNCTION__, __LINE__);
			
		}			
		else if (mode == 1)
		{
			if (ptObject)
			{
				for (int i = 0; i<y_num; i++)
					//delete[](ptObject[i]);
					g_pMManager->pem_delete(ptObject[i], true);
				
				//delete[] ptObject;
				g_pMManager->pem_delete(ptObject, true);
				ptObject = nullptr;
			}

			//ptObject = new POINT_64F*[y_num];
			ptObject = g_pMManager->pem_new<POINT_64F*>(true, y_num, (PCHAR)__FUNCTION__, __LINE__);

			for (size_t i = 0; i < y_num; i++)
				//ptObject[i] = new POINT_64F[x_num];
				ptObject[i] = g_pMManager->pem_new<POINT_64F>(true, x_num, (PCHAR)__FUNCTION__, __LINE__);

		}
			
	}
}
void BilinearInterpolation::Set_Calib_BodyX(TCHAR * strData, int i, int mode)
{
	if (strData == NULL) return;
	TCHAR seps[] = _T(" \t");
	TCHAR* next_token = NULL;

	CString data = _tcstok_s(strData, seps, &next_token);
	int m = i - 4;
	int n = 0;
	if (mode == 0)
	{
		while (data != _T(""))
		{
			ptReference[m][n].x = _tstof(data);
			data = _tcstok_s(NULL, seps, &next_token);
			n++;
		}
	}
	else if (mode == 1)
	{
		while (data != _T(""))
		{
			ptObject[m][n].x = _tstof(data);
			data = _tcstok_s(NULL, seps, &next_token);
			n++;
		}
	}
}
void BilinearInterpolation::Set_Calib_BodyY(TCHAR * strData, int i, int mode)
{
	if (strData == NULL) return;
	TCHAR seps[] = _T(" \t");
	TCHAR* next_token = NULL;

	CString data = _tcstok_s(strData, seps, &next_token);
	int m = i - (5 + y_num);
	int n = 0;

	if (mode == 0)
	{
		while (data != _T(""))
		{
			ptReference[m][n].y = _tstof(data);
			data = _tcstok_s(NULL, seps, &next_token);
			n++;
		}
	}
	else if (mode == 1)
	{
		while (data != _T(""))
		{
			ptObject[m][n].y = _tstof(data);
			data = _tcstok_s(NULL, seps, &next_token);
			n++;
		}
	}
}


bool BilinearInterpolation::Load_Reference(const CString& filePath)
{
	return ReadFile(filePath, 0);
}
bool BilinearInterpolation::Load_Object(const CString& filePath)
{
	return ReadFile(filePath, 1);
}



#define SWAP(a,b) {temp=(a);(a)=(b);(b)=temp;}

void gaussj2(float a[][4], float b[])
{
	int i, icol, irow, j, k, l, ll;
	float big, dum, pivinv, temp;

	int indxr[] = { 0,0,0,0 };
	int indxc[] = { 0,0,0,0 };
	int ipiv[] = { 0,0,0,0 };

	for (i = 0; i < 4; i++)
	{	//This is the _tmain loop over the columns to be
		//reduced. 
		big = 0.0;
		for (j = 0; j < 4; j++)	//This is the outer loop of the search for a pivot
								//element. 
		{
			if (ipiv[j] != 1)
			{
				for (k = 0; k < 4; k++)
				{
					if (ipiv[k] == 0)
					{
						if (fabs(a[j][k]) >= big)
						{
							big = (float)fabs(a[j][k]);
							irow = j;
							icol = k;
						}
					}
					else if (ipiv[k] > 1) AfxMessageBox(_T("gaussj: Singular Matrix-1"));
				}
			}
		}
		++(ipiv[icol]);

		if (irow != icol)
		{
			for (l = 0; l < 4; l++) SWAP(a[irow][l], a[icol][l])
				/*for (l = 0; l < 1; l++)*/ SWAP(b[irow], b[icol])
		}

		indxr[i] = irow;					//We are now ready to divide the pivot row by the
											//pivot element, located at irow and icol. 
		indxc[i] = icol;

		if (a[icol][icol] == 0.0) AfxMessageBox(_T("gaussj: Singular Matrix-2"));
		pivinv = (float)(1.0 / a[icol][icol]);
		a[icol][icol] = 1.0;
		for (l = 0; l < 4; l++) a[icol][l] *= pivinv;
		/*for (l = 0; l < 1; l++)*/ b[icol] *= pivinv;
		for (ll = 0; ll < 4; ll++)			//Next, we reduce the rows...
		{
			if (ll != icol)
			{			//...except for the pivot one, of course.
				dum = a[ll][icol];
				a[ll][icol] = 0.0;
				for (l = 0; l < 4; l++) a[ll][l] -= a[icol][l] * dum;
				/*for (l = 0; l < 1; l++)*/ b[ll] -= b[icol] * dum;
			}
		}
	}
}

bool IsPointInPolygon(POINT_64F p, POINT_64F* PointList, int size =4)
{
	int sides = size;
	int j = sides - 1;
	bool pointStatus = false;
	for (int i = 0; i < sides; i++)
	{
		if (PointList[i].y < p.y && PointList[j].y >= p.y || PointList[j].y < p.y && PointList[i].y >= p.y)
		{
			if (PointList[i].x + (p.y - PointList[i].y) / (PointList[j].y - PointList[i].y) * (PointList[j].x - PointList[i].x) < p.x)
			{
				pointStatus = !pointStatus;
			}
		}
		j = i;
	}
	return pointStatus;
}

bool BilinearInterpolation::IsContains(POINT_64F**& pts, int minx, int miny, POINT_64F p)
{
	POINT_64F points[4];
	points[0] = pts[miny][minx];
	points[1] = pts[miny][minx + 1];
	points[2] = pts[miny + 1][minx + 1];
	points[3] = pts[miny + 1][minx];
	return IsPointInPolygon(p, points);
}

bool BilinearInterpolation::FindMinIndex(POINT_64F**& souce, POINT_64F pt, int& minX, int& minY)
{
	if (x_num == 0 || y_num == 0)
		return false;

	float fMinX = (float)souce[1][1].x;
	float fMaxX = (float)souce[1][x_num - 2].x;
	float fMinY = (float)souce[1][1].y;
	float fMaxY = (float)souce[y_num - 2][ 1].y;

	float fPitchX = (fMaxX - fMinX) / (x_num - 2);
	float fPitchY = (fMaxY - fMinY) / (y_num - 2);

	int minXIndex = 0;
	int minYIndex = 0;

	bool beXternel = false;

	if (pt.x < fMinX)
	{
		minXIndex = 0;
		beXternel = true;
	}
	else if (pt.x > fMaxX)
	{
		minXIndex = x_num - 2;
		beXternel = true;
	}
	else
	{
		minXIndex = (int)((pt.x - fMinX) / fPitchX);
		minXIndex++;

		if (minXIndex >= x_num - 1)
			minXIndex = x_num - 2;
	}

	if (pt.y < fMinY)
	{
		minYIndex = 0;
		beXternel = true;
	}
	else if (pt.y > fMaxY)
	{
		minYIndex = y_num - 2;
		beXternel = true;
	}
	else
	{
		minYIndex = (int)((pt.y - fMinY) / fPitchY);
		minYIndex++;

		if (minYIndex >= y_num - 1)
			minYIndex = y_num - 2;

	}

	if (beXternel == false)
	{
		int maxYIndex = minYIndex + 1;

		if (maxYIndex > y_num - 1)
			maxYIndex = y_num - 1;

		int maxXindex = minXIndex + 1;

		if (maxXindex > x_num - 1)
			maxXindex = x_num - 1;

		bool bFind = false;

		//찾은 중심에서 3x3 사이즈로 검색
		for (int y = minYIndex - 1; y < maxYIndex; y++)
		{
			for (int x = minXIndex - 1; x < maxXindex; x++)
			{
				if (IsContains(souce, x, y, pt))
				{
					minX = x;
					minY = y;

					bFind = true;
					break;
				}

			}
			if (bFind) break;
		}

		if (bFind == false)
		{
			//못 찾으면 전체에서 검색

			for (int y = 0; y < y_num - 1; y++)
			{
				for (int x = 0; x < x_num - 1; x++)
				{
					if (IsContains(souce, x, y, pt))
					{
						minX = x;
						minY = y;

						bFind = true;
						break;
					}

				}

				if (bFind) break;
			}
		}

		if (bFind == false)
			return false;
		
	}
	else
	{  //외부영역
		/*minX = minXIndex;
		minY = minYIndex;*/
		for (int y = 0; y < y_num - 1; y++)
		{
			for (int x = 0; x < x_num - 1; x++)
			{
				if (IsContains(souce, x, y, pt))
				{
					minX = x;
					minY = y;

					return true;
				}

			}

		}
		return false;
	}


	return true;

}

POINT_64F BilinearInterpolation::Interpolate(POINT_64F**& matA, POINT_64F**& MatB, int minX, int minY, int maxX, int maxY, POINT_64F pt)
{
	float fInterpolation_x[4][4];
	float fInterpolation_y[4][4];
	float h_x[4];
	float h_y[4];

	fInterpolation_x[0][0] = fInterpolation_y[0][0] = 1.;
	fInterpolation_x[0][1] = fInterpolation_y[0][1] = (float)matA[minY][minX].x;
	fInterpolation_x[0][2] = fInterpolation_y[0][2] = (float)matA[minY][minX].y;
	fInterpolation_x[0][3] = fInterpolation_y[0][3] = (float)(matA[minY][minX].x * matA[minY][ minX].y);

	fInterpolation_x[1][0] = fInterpolation_y[1][0] = 1.;
	fInterpolation_x[1][1] = fInterpolation_y[1][1] = (float)matA[maxY][minX].x;
	fInterpolation_x[1][2] = fInterpolation_y[1][2] = (float)matA[maxY][minX].y;
	fInterpolation_x[1][3] = fInterpolation_y[1][3] = (float)(matA[maxY][minX].x * matA[maxY][minX].y);

	fInterpolation_x[2][0] = fInterpolation_y[2][0] = 1.;
	fInterpolation_x[2][1] = fInterpolation_y[2][1] = (float)matA[maxY][maxX].x;
	fInterpolation_x[2][2] = fInterpolation_y[2][2] = (float)matA[maxY][maxX].y;
	fInterpolation_x[2][3] = fInterpolation_y[2][3] = (float)(matA[maxY][maxX].x * matA[maxY][maxX].y);

	fInterpolation_x[3][0] = fInterpolation_y[3][0] = 1.;
	fInterpolation_x[3][1] = fInterpolation_y[3][1] = (float)matA[minY][maxX].x;
	fInterpolation_x[3][2] = fInterpolation_y[3][2] = (float)matA[minY][maxX].y;
	fInterpolation_x[3][3] = fInterpolation_y[3][3] = (float)(matA[minY][maxX].x * matA[minY][ maxX].y);

	h_x[0] = (float)MatB[minY][ minX].x;
	h_x[1] = (float)MatB[maxY][ minX].x;
	h_x[2] = (float)MatB[maxY][ maxX].x;
	h_x[3] = (float)MatB[minY][ maxX].x;

	h_y[0] = (float)MatB[minY][ minX].y;
	h_y[1] = (float)MatB[maxY][ minX].y;
	h_y[2] = (float)MatB[maxY][ maxX].y;
	h_y[3] = (float)MatB[minY][ maxX].y;

	gaussj2(fInterpolation_x, h_x);
	gaussj2(fInterpolation_y, h_y);

	POINT_64F newpt;
	newpt.x = h_x[0] + h_x[1] * pt.x + h_x[2] * pt.y + h_x[3] * pt.x * pt.y;
	newpt.y = h_y[0] + h_y[1] * pt.x + h_y[2] * pt.y + h_y[3] * pt.x * pt.y;

	return newpt;
}

POINT_64F BilinearInterpolation::UndistortPoint(POINT_64F pt)
{
	int minX = 0, minY = 0;
	if(!HasData())
		return pt;
	if (!FindMinIndex(ptReference, pt,  minX,  minY))
		return pt;

	int maxX = minX + 1, maxY = minY + 1;
	return Interpolate(ptReference, ptObject, minX, minY, maxX, maxY, pt);
}

//Motion Calib
POINT_64F BilinearInterpolation::UndistortPoint2(POINT_64F pt)
{
	int minX = 0, minY = 0;
	
	double ref_offsetX;
	double ref_offsetY;

	if (ptReference[1][1].x >= 0)ref_offsetX = ptReference[1][1].x;
	else ref_offsetX = ptReference[1][1].x -1;

	if (ptReference[1][1].y >= 0)ref_offsetY = ptReference[1][1].y;
	else ref_offsetY = ptReference[1][1].y - 1;

	if (pt.x - ref_offsetX > 0.)
		minX = (int)floor((pt.x - ref_offsetX) / 10) + 1;
	if (pt.y - ref_offsetY > 0.)
		minY = (int)floor((pt.y - ref_offsetY) / 10) + 1;

	if (minX<0)
		minX = 0;
	else if (minX >= x_num - 1)
		minX = x_num - 2;

	if (minY<0)
		minY = 0;
	else if (minY >= y_num - 1)
		minY = y_num - 2;

	int maxX = minX + 1, maxY = minY + 1;
	return Interpolate(ptReference, ptObject, minX, minY, maxX, maxY, pt);

}

POINT_64F BilinearInterpolation::DistortPoint(POINT_64F pt)
{
	int minX = 0, minY = 0;
	if(!HasData())
		return pt;
	if (!FindMinIndex(ptObject, pt,  minX,  minY))
		return pt;

	int maxX = minX + 1, maxY = minY + 1;
	return Interpolate(ptObject, ptReference, minX, minY, maxX, maxY, pt);
}