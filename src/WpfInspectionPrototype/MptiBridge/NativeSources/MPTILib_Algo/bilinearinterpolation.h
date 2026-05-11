#pragma once

#include "MCoordi.h"

//typedef struct tagPOINT_64F {
//	double x;
//	double y;
//} POINT_64F;

class BilinearInterpolation
{
public:
	BilinearInterpolation();
	~BilinearInterpolation();

private:
	POINT_64F** ptReference; //¿¹»ó ÁÂÇ¥
	POINT_64F** ptObject; //½ÇÁ¦ ÁÂÇ¥

	int x_num;
	int y_num;

	void Set_CalibHeader(TCHAR * strData, int mode);
	void Set_Calib_BodyX(TCHAR * strData, int i, int mode);
	void Set_Calib_BodyY(TCHAR * strData, int i, int mode);
	bool ReadFile(const CString& filePath, int mode);

	bool IsContains(POINT_64F**& pts, int minx, int miny, POINT_64F p);
    bool FindMinIndex(POINT_64F**& souce, POINT_64F pt,  int& minX, int& minY);
	POINT_64F Interpolate(POINT_64F**& matA, POINT_64F**& MatB, int minX, int minY, int maxX, int maxY, POINT_64F pt);

public:
	
	bool Load_Reference(const CString& filePath);
	bool Load_Object(const CString& filePath);
	void Clear();
	bool HasData();

	//ref to Obj
	POINT_64F BilinearInterpolation::UndistortPoint(POINT_64F pt);

	POINT_64F BilinearInterpolation::UndistortPoint2(POINT_64F pt);

	//obj to ref
 	POINT_64F BilinearInterpolation::DistortPoint(POINT_64F pt);


};

