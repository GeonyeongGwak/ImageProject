#include "stdafx.h"
#include "InspRstPoly.h"
InspRstPolyAlgo::InspRstPolyAlgo(int ndX, int ndY, int nWindID, int nAlgoID, int nRoiID, int nCount, POINTF ptCenter, vector<POINTF> vPolyPoints)
{
	m_ndX = ndX;
	m_ndY = ndY;
	m_nWindID = nWindID;
	m_nAlgoID = nAlgoID;
	m_nRoiID = nRoiID;
	m_nCount = nCount;
	m_ptCenter = ptCenter;
	m_vPolyPoints = vPolyPoints;
}
DSIDefect::DSIDefect() 
{
	m_nID1 = 0; 
	m_nID2 = 0; 
	m_nID3 = 0; 
	m_nID4 = 0; 
	m_nDefectIdx = 0;
	m_nCount = 0;
	m_nDefectLocation = 0;
}
DSIDefect::~DSIDefect() 
{
	return;
}
void DSIDefect::WriteDSI(ofstream* dsiWirter) 
{
	dsiWirter->write((char*)this, sizeof(*this));
}
void DSIDefectShape::WriteDSI(ofstream* dsiWirter)
{
	return;
}
void DSIDefectShape::CalDSISize()
{
	return;
}
DSIDefectShape::DSIDefectShape()
{
	m_ptfCenter.x = 0;
	m_ptfCenter.y = 0;
	m_nDSSize = 0;
}
DSIDefectShape ::~DSIDefectShape()
{
	return;
}

DSIDefectShape_Polygon::DSIDefectShape_Polygon()
{
	m_ShapeType = e_Polygon;
}
DSIDefectShape_Polygon::~DSIDefectShape_Polygon()
{
	return;
}
void DSIDefectShape_Polygon::WriteDSI(ofstream* dsiWirter)
{
	dsiWirter->write((char*)&m_ShapeType, sizeof(m_ShapeType));
	dsiWirter->write((char*)&m_ptfCenter.x, sizeof(m_ptfCenter.x));
	dsiWirter->write((char*)&m_ptfCenter.y, sizeof(m_ptfCenter.y));
	dsiWirter->write((char*)&m_nDSSize, sizeof(m_nDSSize));
	int nPolyLength = m_vPoygons.size();
	dsiWirter->write((char*)&nPolyLength, sizeof(nPolyLength));
	for (int idxPoly = 0; idxPoly < nPolyLength; idxPoly++) 
	{
		dsiWirter->write((char*)&m_vPoygons[idxPoly].x, sizeof(m_vPoygons[idxPoly].x));
		dsiWirter->write((char*)&m_vPoygons[idxPoly].y, sizeof(m_vPoygons[idxPoly].y));
	}
}
void DSIDefectShape_Polygon::CalDSISize()
{
	m_nDSSize = 4 + m_vPoygons.size() * 8;
}
DSIHeader::DSIHeader() 
{
	m_nDSNum = 0;
	int nSizeHeader = sizeof(*this);
	m_nFileSize = nSizeHeader;
	m_nHeadLocation = nSizeHeader;
	m_nImageWidth = 0;
	m_nImageHeight = 0;
}
void DSIHeader::WriteDSI(ofstream* dsiWirter) 
{
	dsiWirter->write((char*)this, sizeof(*this));
}
DSI::DSI() 
{
	m_nDefectHaderSize = 0;
}
DSI::~DSI()
{
	for (auto& atDs : m_vDefectShape) 
		g_pMManager->pem_delete(atDs, false);
	for (auto& atDf : m_vDefect)
		g_pMManager->pem_delete(atDf, false);
}
void DSI::CreateDir(CString Path)
{
	CString strPath = Path;
	CString strTemp[20];
	CString strFile;
	int nLength = strPath.GetLength();
	int j = 0;
	for (int i = 0; i < 20; i++)
		strTemp[i].Empty();
	for (int i = 0; i < nLength; i++)
	{
		if (strPath.Mid(i, 1) == '\\')
			j++;
		strTemp[j] = strTemp[j] + strPath.Mid(i, 1);
	}
	strFile = strTemp[0];
	int i = 1;
	while (strTemp[i] != "")
	{
		strFile = strFile + strTemp[i];
		CreateDirectory(strFile, NULL);
		i++;
	}
}
void DSI::SetDSIData(int nPartID, int nPartImageWidth, int nPartImageHeight, vector<InspRstPolyAlgo>* vpInspRstPoly, bool bInspInTeach)
{
	int nSize = vpInspRstPoly->size();
	if (nSize > 0) 
	{
		m_disHeader.m_nImageWidth = nPartImageWidth;
		m_disHeader.m_nImageHeight = nPartImageHeight;
		set<tuple<int, int, int, int>> setID; // ID1, ID2, ID3, ID4 -> 중복체크를 위한 변수
		int nDefectIdx = 0;
		for (InspRstPolyAlgo& inspRstPolyAlgo : *vpInspRstPoly)
		{
			int nWindID = inspRstPolyAlgo.m_nWindID;
			int nAlgoID = inspRstPolyAlgo.m_nAlgoID;
			int nRoiID = inspRstPolyAlgo.m_nRoiID;
			int nPolySize = inspRstPolyAlgo.m_vPolyPoints.size();
			if (nPolySize > 0)
			{
				DSIDefectShape_Polygon* pDsPoly = g_pMManager->pem_new<DSIDefectShape_Polygon>(false, 0, (PCHAR)__FUNCTION__, __LINE__); // 소멸자에서 데이터 해제
				pDsPoly->m_ptfCenter.x = inspRstPolyAlgo.m_ptCenter.x;
				pDsPoly->m_ptfCenter.y = inspRstPolyAlgo.m_ptCenter.y;
				if (bInspInTeach) 
				{
					pDsPoly->m_ptfCenter.x -= inspRstPolyAlgo.m_ndX;
					pDsPoly->m_ptfCenter.y -= inspRstPolyAlgo.m_ndY;
				}
				for (POINTF& ptfWindPoint : inspRstPolyAlgo.m_vPolyPoints)
				{
					POINTF ptfPartPoint;
					ptfPartPoint.x = ptfWindPoint.x - inspRstPolyAlgo.m_ptCenter.x;// Center 좌표 중심으로 그리므로 빼줌
					ptfPartPoint.y = ptfWindPoint.y - inspRstPolyAlgo.m_ptCenter.y;// Center 좌표 중심으로 그리므로 빼줌
					/*if (bInspInTeach) 
					{
						ptfPartPoint.x -= inspRstPolyAlgo.m_ndX;
						ptfPartPoint.y -= inspRstPolyAlgo.m_ndY;
					}*/
					pDsPoly->m_vPoygons.push_back(ptfPartPoint);
				}
				int nDfSize;
				tuple<int, int, int, int> tupleID = make_tuple(nPartID, nWindID, nAlgoID, nRoiID);
				if (setID.find(tupleID) == setID.end())
				{
					setID.insert(tupleID);
					DSIDefect* dsiDf = g_pMManager->pem_new<DSIDefect>(false, 0, (PCHAR)__FUNCTION__, __LINE__); // 소멸자에서 데이터 해제
					dsiDf->m_nID1 = nPartID;
					dsiDf->m_nID2 = nWindID;
					dsiDf->m_nID3 = nAlgoID;
					dsiDf->m_nID4 = nRoiID;
					dsiDf->m_nDefectIdx = nDefectIdx++;
					dsiDf->m_nCount = inspRstPolyAlgo.m_nCount;
					dsiDf->m_nDefectLocation = m_disHeader.m_nHeadLocation;
					m_vDefect.push_back(dsiDf);
					nDfSize = sizeof(*dsiDf);//32 // (4 * 8) // ID(1, 2, 3, 4), m_nDefectIdx, m_nCount, m_nDefectLocation
				}
				else
					nDfSize = 0;
				pDsPoly->CalDSISize();// 꼭 해줘야함
				int nDSLineSize = 16 + pDsPoly->m_nDSSize; // type, CenterX, CenterY, Size 각각 4Byte씩 16을 더해준다
				m_disHeader.m_nFileSize += (nDSLineSize + nDfSize); // nDSLineSize + DSIDefect 안에 들어있는 Byte 값 
				m_disHeader.m_nHeadLocation += nDSLineSize;
				m_vDefectShape.push_back(pDsPoly);
			}
		}
		m_disHeader.m_nDSNum = m_vDefectShape.size();
		m_nDefectHaderSize = m_vDefect.size();
		m_disHeader.m_nFileSize += 4; // Defect 길이를 하나 더 써줌으로 4를 더해줘야함
	}
}
void DSI::SetDSIData_new(int nPartID, int nPartImageWidth, int nPartImageHeight, vector<InspRstPolyAlgo>* vpInspRstPoly, bool bInspInTeach)
{
	int nSize = vpInspRstPoly->size();
	if (nSize > 0)
	{
		m_disHeader.m_nImageWidth = nPartImageWidth;
		m_disHeader.m_nImageHeight = nPartImageHeight;
		set<tuple<int, int, int, int>> setID; // ID1, ID2, ID3, ID4 -> 중복체크를 위한 변수
		int nDefectIdx = 0;
		for (InspRstPolyAlgo& inspRstPolyAlgo : *vpInspRstPoly)
		{
			int nWindID = inspRstPolyAlgo.m_nWindID;
			int nAlgoID = inspRstPolyAlgo.m_nAlgoID;
			int nRoiID = inspRstPolyAlgo.m_nRoiID;
			int nPolySize = inspRstPolyAlgo.m_vPolyPoints.size();
			if (nPolySize > 0)
			{
				DSIDefectShape_Polygon* pDsPoly = g_pMManager->pem_new<DSIDefectShape_Polygon>(false, 0, (PCHAR)__FUNCTION__, __LINE__); // 소멸자에서 데이터 해제
				pDsPoly->m_ptfCenter.x = inspRstPolyAlgo.m_ptCenter.x;
				pDsPoly->m_ptfCenter.y = inspRstPolyAlgo.m_ptCenter.y;
				if (bInspInTeach)
				{
					pDsPoly->m_ptfCenter.x -= inspRstPolyAlgo.m_ndX;
					pDsPoly->m_ptfCenter.y -= inspRstPolyAlgo.m_ndY;
				}
				for (POINTF& ptfWindPoint : inspRstPolyAlgo.m_vPolyPoints)
				{
					POINTF ptfPartPoint;
					ptfPartPoint.x = ptfWindPoint.x - inspRstPolyAlgo.m_ptCenter.x;// Center 좌표 중심으로 그리므로 빼줌
					ptfPartPoint.y = ptfWindPoint.y - inspRstPolyAlgo.m_ptCenter.y;// Center 좌표 중심으로 그리므로 빼줌
					pDsPoly->m_vPoygons.push_back(ptfPartPoint);
				}
				int nDfSize;
				tuple<int, int, int, int> tupleID = make_tuple(nPartID, nWindID, nAlgoID, nRoiID);
				setID.insert(tupleID);
				DSIDefect* dsiDf = g_pMManager->pem_new<DSIDefect>(false, 0, (PCHAR)__FUNCTION__, __LINE__); // 소멸자에서 데이터 해제
				dsiDf->m_nID1 = nPartID;
				dsiDf->m_nID2 = nWindID;
				dsiDf->m_nID3 = nAlgoID;
				dsiDf->m_nID4 = nRoiID;
				dsiDf->m_nDefectIdx = nDefectIdx++;
				dsiDf->m_nCount = inspRstPolyAlgo.m_nCount;
				dsiDf->m_nDefectLocation = m_disHeader.m_nHeadLocation;
				m_vDefect.push_back(dsiDf);
				nDfSize = sizeof(*dsiDf);//32 // (4 * 8) // ID(1, 2, 3, 4), m_nDefectIdx, m_nCount, m_nDefectLocation
				pDsPoly->CalDSISize();// 꼭 해줘야함
				int nDSLineSize = 16 + pDsPoly->m_nDSSize; // type, CenterX, CenterY, Size 각각 4Byte씩 16을 더해준다
				m_disHeader.m_nFileSize += (nDSLineSize + nDfSize); // nDSLineSize + DSIDefect 안에 들어있는 Byte 값 
				m_disHeader.m_nHeadLocation += nDSLineSize;
				m_vDefectShape.push_back(pDsPoly);
			}
		}
		m_disHeader.m_nDSNum = m_vDefectShape.size();
		m_nDefectHaderSize = m_vDefect.size();
		m_disHeader.m_nFileSize += 4; // Defect 길이를 하나 더 써줌으로 4를 더해줘야함
	}
}
void DSI::WriteDSIFile(CString sFullName)
{
	ofstream dsiWirter(sFullName, ios::out | ios::binary);
	if (dsiWirter) 
	{
		CreateDir(sFullName);
		m_disHeader.WriteDSI(&dsiWirter);
		//m_vDefectShape
		for (int idxDs = 0; idxDs < m_disHeader.m_nDSNum; idxDs++)
			(*m_vDefectShape[idxDs]).WriteDSI(&dsiWirter);
		//m_vDefect
		dsiWirter.write((char*)&m_nDefectHaderSize, sizeof(m_nDefectHaderSize));
		for (int idxDf = 0; idxDf < m_nDefectHaderSize; idxDf++)
			(*m_vDefect[idxDf]).WriteDSI(&dsiWirter);
		dsiWirter.close();
	}
}
