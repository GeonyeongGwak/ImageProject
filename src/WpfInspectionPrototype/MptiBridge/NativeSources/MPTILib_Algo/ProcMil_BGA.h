#pragma once
#include "procmil.h"

enum milprocbga_result
{
	eMBGA_SUCCESS = 0,
	eMBGA_FAIL
};

class CProcMil_BGA : public CProcMil
{
public:
	CProcMil_BGA(void);
	virtual ~CProcMil_BGA(void);


// private:
// 	Im::PIL_ID m_milBlobFeature;
// 	Im::PIL_ID m_milBlobResult;

public:
	int InitMil(Im::PIL_ID* milApp, Im::PIL_ID* milSys, bool bUseImagePilLib);
	int FreeMil();


	int CalcBlobM(Im::PIL_ID milBinSrc, int minArea, int maxArea, BOOL eraseBorderBlob, BOOL fillHole, Im::PIL_ID milDst);
	int CalcBlobU(UCHAR* userBinSrc, int sizeX, int sizeY, int minArea, int maxArea , BOOL eraseBorderBlob, BOOL fillHole, UCHAR* userDst);
	void GetBlobResult(CMilBlobResult* retResult);

	int BlobProc(UCHAR* userSrc, int sizeX, int sizeY, UCHAR* retUserDst, UCHAR* retUserLut);
	int SelectBlob_MaxArea(int blobNum,  Im::PIL_ID milDst);

	int MakeLUT(UCHAR* userSrc, int sizeX, int sizeY, UCHAR* retUserDst);



	void MakeImage_Sum(void* milImgT, void* milImgM, void* milImgB, UCHAR* userDst);
};

