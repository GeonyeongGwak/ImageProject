#pragma once
#include <fstream>
#include <vector>
#include <set>
#include <tuple>
/*
작성자 : 최준영 
작성일 : 2022/07/04
작성내용 : DSI 파일을 쓰는 부분, 읽는부분은 C# 단에 서함
특이사항 : 변수 추가 혹은 제거시 파일 쓰고 읽히는거 필히 확인 해야함, C# 단과 싱크 맞춰줘야함
*/
class InspRstPolyAlgo
{
public:
	int m_ndX; // Teaching 창에서 검사시 빼주기 위해 가지고있음
	int m_ndY; // Teaching 창에서 검사시 빼주기 위해 가지고있음
	int m_nWindID;
	int m_nAlgoID;
	int m_nRoiID;
	int m_nCount; // ROI내에 Blob 몇개 잡혔는지 나타내는 Count
	POINTF m_ptCenter;
	vector<POINTF> m_vPolyPoints;
public:
	InspRstPolyAlgo(int ndX, int ndY, int nWindID, int nAlgoID, int nRoiID, int nCount, POINTF ptCenter, vector<POINTF> vPolyPoints);
	InspRstPolyAlgo() {};
};
enum DSIType
{
	e_Polygon = 0, //일단을 Polygon만
	e_Rectangle = 1,
	e_Circle = 2,
};
class DSIDefect 
{
public:
	int m_nID1; //PartID
	int m_nID2; //WindowID
	int m_nID3; //AlgoID
	int m_nID4; //ROIID
	int m_nDefectIdx;
	int m_nCount;// ROI내에 Blob 몇개 잡혔는지 나타내는 Count
	INT64 m_nDefectLocation;
public:
	DSIDefect();
	~DSIDefect();
	void WriteDSI(ofstream* dsiWirter);
};
class DSIDefectShape 
{
public:
	int m_ShapeType; // DSIType
	POINTF m_ptfCenter;
	int m_nDSSize;
public:
	virtual void WriteDSI(ofstream* dsiWirter);
	virtual void CalDSISize();
	DSIDefectShape();
	virtual ~DSIDefectShape();
};
class DSIDefectShape_Polygon : public DSIDefectShape
{
public:
	vector<POINTF> m_vPoygons;
public:
	DSIDefectShape_Polygon();
	~DSIDefectShape_Polygon();
	void WriteDSI(ofstream* dsiWirter);
	void CalDSISize();
};

class DSIHeader 
{
private:
	const char m_arrChars[4] = { 'D', 'S', 'I', ' ' };
public:
	int m_nDSNum; // Defec Shape Count
	INT64 m_nFileSize; // TotalFileSize
	INT64 m_nHeadLocation; // Defect Starting Point
	int m_nImageWidth;
	int m_nImageHeight;
private:
	const int m_nDumy1 = 0;
	const int m_nDumy2 = 0;
public:
	DSIHeader();
	void WriteDSI(ofstream* dsiWirter);
};
class DSI 
{
public:
	// Header 부분
	DSIHeader m_disHeader;
	// Defect Shape 부분 
	vector<DSIDefectShape*> m_vDefectShape;
	// Defect 부분
	int m_nDefectHaderSize;
	vector<DSIDefect*> m_vDefect;
public:
	DSI();	// 생성자
	~DSI();	// 소멸자
public:
	void CreateDir(CString sPath);
	void WriteDSIFile(CString sFullName);
	void SetDSIData(int nPartID, int nPartImageWidth, int nPartImageHeight, vector<InspRstPolyAlgo>* vpInspRstPoly, bool bInspInTeach);
	void SetDSIData_new(int nPartID, int nPartImageWidth, int nPartImageHeight, vector<InspRstPolyAlgo>* vpInspRstPoly, bool bInspInTeach);
};

