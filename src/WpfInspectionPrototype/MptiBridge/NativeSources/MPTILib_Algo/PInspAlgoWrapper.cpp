#include "stdafx.h"
#include "PInspAlgoWrapper.h"
#include "ippi.h"
#include "ExtInspRoot.h"

PInspAlgoWrapper::PInspAlgoWrapper()
{

	PIAL::PInspAlgo_Lib::Factory();
	m_PInspAlgo = new PIAL::PInspAlgo;
	// m_PTeaching = new PIAL::PTeaching;
	// m_PInspWire = new PIAL::PInspAlgo_WireBonding;

	m_bFastImageCompose = true;
	PIAL::PInspAlgo_Lib::m_SaveTempLog = false;
	PIAL::PInspAlgo_Lib::m_Bump_NoCut = true;
}

PInspAlgoWrapper::~PInspAlgoWrapper()
{
	PIAL::PInspAlgo_Lib::Free();
	delete m_PInspAlgo;
	// delete m_PTeaching;
	// delete m_PInspWire;
}

void PInspAlgoWrapper::SetResolution(int fovWidth, int fovLength, double resolX, double resolY)
{
	PIAL::PInspAlgo_Lib::InitDevice(fovWidth, fovLength, resolX, resolY);
}

void PInspAlgoWrapper::SetCompositeLightMode(int LightMode, float compoBtmR, float compoBtmG, float compoBtmB)
{
	PIAL::PInspAlgo_Lib::SetCompoLightData(LightMode, compoBtmR, compoBtmG, compoBtmB);
}

void PInspAlgoWrapper::SetFittingValue(float r, float g, float b, float br, float bb)
{
	PIAL::PInspAlgo_Lib::SetFittingData(r, g, b, br, bb);
}

bool PInspAlgoWrapper::MakeBuffWnd(WndAlgoImg* sClipWndAlgoImg, WndInfo sWndInfo, PIAL::Insp_Image*& pImg_buf, bool bref)
{
	bool bResult = false;

	if (pImg_buf == nullptr)
		return bResult;

	int nImgWidth = sClipWndAlgoImg->m_nWidth;
	int nImgHeight = sClipWndAlgoImg->m_nHeight;

	if (sClipWndAlgoImg->m_ucArr2D)
	{
		if (pImg_buf->m_p2D)
			delete pImg_buf->m_p2D;

		pImg_buf->m_p2D = new PIAL::PI_Buff(sClipWndAlgoImg->m_ucArr2D, nImgWidth, nImgHeight, nImgWidth, 1, 1, bref);

	}

	if (sClipWndAlgoImg->m_fArr3D)
	{
		if (pImg_buf->inspWndImage->m_p3D)
			delete pImg_buf->inspWndImage->m_p3D;

		pImg_buf->inspWndImage->m_p3D = new PIAL::PI_Buff(sClipWndAlgoImg->m_fArr3D, nImgWidth, nImgHeight, bref);
	}

	pImg_buf->inspWndImage->nImgSizeX = nImgWidth;
	pImg_buf->inspWndImage->nImgSizeY = nImgHeight;


	pImg_buf->inspPartImage->nImgSizeX = sClipWndAlgoImg->m_nWidth3D;
	pImg_buf->inspPartImage->nImgSizeY = sClipWndAlgoImg->m_nHeight3D;

	return true;
}

bool PInspAlgoWrapper::MakeBuffWnd_Clip(WndAlgoImg* sWndAlgoImg, WndInfo sWndInfo, PIAL::Insp_Image*& pImg_buf)
{
	bool bResult = false;

	if (pImg_buf == nullptr)
		return bResult;


	if (sWndAlgoImg->m_ucArrCV)
	{
		int width = sWndAlgoImg->m_nWidth;
		int height = sWndAlgoImg->m_nHeight;
		if ((width <= 0) || (height <= 0))
			return bResult;

		int widthStep = width;

		int nSizeX = width % 4;
		if (nSizeX != 0)
			widthStep += 4 - nSizeX;


		pImg_buf->m_p2D = new PIAL::PI_Buff(sWndAlgoImg->m_ucArrCV, width, height, widthStep);
	}

	pImg_buf->inspPartImage->nImgSizeX = sWndAlgoImg->m_nWidth3D;
	pImg_buf->inspPartImage->nImgSizeY = sWndAlgoImg->m_nHeight3D;

	double dResolX = PIAL::PInspAlgo_Lib::m_resolX;
	double dResolY = PIAL::PInspAlgo_Lib::m_resolY;

	double dClipWidth = (float)(sWndInfo.dWidth / dResolX);
	double dClipHeight = (float)(sWndInfo.dLength / dResolY);
	if ((dClipWidth <= 0) || (dClipHeight <= 0))
		return bResult;

	pImg_buf->inspPartImage->m_Angle = sWndAlgoImg->dAngle;

	double dClipX2D = ((sWndInfo.m_dPartWidth / 2.) + (sWndInfo.dCenterX) - (sWndInfo.dWidth / 2.0)) / dResolX;
	double dClipY2D = ((sWndInfo.m_dPartHeight / 2.) - (sWndInfo.dCenterY) - (sWndInfo.dLength / 2.0)) / dResolY;
	double dClipX3D = ((sWndInfo.m_dPartWidth / 2.) + (sWndInfo.dCenterX) - (sWndInfo.dWidth / 2.0)) / dResolX;
	double dClipY3D = ((sWndInfo.m_dPartHeight / 2.) - (sWndInfo.dCenterY) - (sWndInfo.dLength / 2.0)) / dResolY;

	if (dClipX2D < 0)  dClipX2D = 0;
	if (dClipY2D < 0) dClipY2D = 0;

	if (dClipX3D <= 0) dClipX3D = 0;
	if (dClipY3D <= 0) dClipY3D = 0;

	// 20141008 SHW
	if (dClipWidth > sWndAlgoImg->m_nWidth) dClipX2D = sWndAlgoImg->m_nWidth / 2;
	if (dClipHeight > sWndAlgoImg->m_nHeight) dClipY2D = sWndAlgoImg->m_nHeight / 2;

	int nClipWidth = RounD(dClipWidth);
	int nClipHeight = RounD(dClipHeight);
	int nStartX2D = RounD(dClipX2D);
	int nStartY2D = RounD(dClipY2D);
	int nStartX3D = RounD(dClipX3D);
	int nStartY3D = RounD(dClipY3D);

	return true;
}

bool PInspAlgoWrapper::MakeROIImg(AlgoColorOpt* ptrAlgoColorOpt, PIAL::Insp_ROIImg*&  InspImageData)
{
	InspImageData->nImageSizeX = ptrAlgoColorOpt->m_sImgBuf.nImageSizeX;
	InspImageData->nImageSizeY = ptrAlgoColorOpt->m_sImgBuf.nImageSizeY;

	if (ptrAlgoColorOpt->m_sImgBuf.imgTop_R)
	{
		PIAL::PI_Buff* buff = new PIAL::PI_Buff(ptrAlgoColorOpt->m_sImgBuf.imgTop_R, InspImageData->nImageSizeX, InspImageData->nImageSizeY, InspImageData->nImageSizeX);
		InspImageData->AddBuff(PIAL::Top_R, buff);
	}
	if (ptrAlgoColorOpt->m_sImgBuf.imgTop_G)
	{
		PIAL::PI_Buff* buff = new PIAL::PI_Buff(ptrAlgoColorOpt->m_sImgBuf.imgTop_G, InspImageData->nImageSizeX, InspImageData->nImageSizeY, InspImageData->nImageSizeX);
		InspImageData->AddBuff(PIAL::Top_G, buff);
	}
	if (ptrAlgoColorOpt->m_sImgBuf.imgTop_B)
	{
		PIAL::PI_Buff* buff = new PIAL::PI_Buff(ptrAlgoColorOpt->m_sImgBuf.imgTop_B, InspImageData->nImageSizeX, InspImageData->nImageSizeY, InspImageData->nImageSizeX);
		InspImageData->AddBuff(PIAL::Top_B, buff);
	}
	if (ptrAlgoColorOpt->m_sImgBuf.imgTop_W)
	{
		PIAL::PI_Buff* buff = new PIAL::PI_Buff(ptrAlgoColorOpt->m_sImgBuf.imgTop_W, InspImageData->nImageSizeX, InspImageData->nImageSizeY, InspImageData->nImageSizeX);
		InspImageData->AddBuff(PIAL::Top_W, buff);
	}
	if (ptrAlgoColorOpt->m_sImgBuf.imgMiddle_R)
	{
		PIAL::PI_Buff* buff = new PIAL::PI_Buff(ptrAlgoColorOpt->m_sImgBuf.imgMiddle_R, InspImageData->nImageSizeX, InspImageData->nImageSizeY, InspImageData->nImageSizeX);
		InspImageData->AddBuff(PIAL::Middle_R, buff);
	}
	if (ptrAlgoColorOpt->m_sImgBuf.imgMiddle_B)
	{
		PIAL::PI_Buff* buff = new PIAL::PI_Buff(ptrAlgoColorOpt->m_sImgBuf.imgMiddle_B, InspImageData->nImageSizeX, InspImageData->nImageSizeY, InspImageData->nImageSizeX);
		InspImageData->AddBuff(PIAL::Middle_B, buff);
	}
	if (ptrAlgoColorOpt->m_sImgBuf.imgBottom_R)
	{
		PIAL::PI_Buff* buff = new PIAL::PI_Buff(ptrAlgoColorOpt->m_sImgBuf.imgBottom_R, InspImageData->nImageSizeX, InspImageData->nImageSizeY, InspImageData->nImageSizeX);
		InspImageData->AddBuff(PIAL::Bottom_R, buff);
	}
	if (ptrAlgoColorOpt->m_sImgBuf.imgBottom_B)
	{
		PIAL::PI_Buff* buff = new PIAL::PI_Buff(ptrAlgoColorOpt->m_sImgBuf.imgBottom_B, InspImageData->nImageSizeX, InspImageData->nImageSizeY, InspImageData->nImageSizeX);
		InspImageData->AddBuff(PIAL::Bottom_B, buff);
	}

	return true;
}

bool PInspAlgoWrapper::MakeROIImg(InspRoiImgBuf* ptrImgBuff, PIAL::Insp_ROIImg*&  InspImageData)
{
	//FOV
	InspImageData->nImageSizeX = ptrImgBuff->nImageSizeX;
	InspImageData->nImageSizeY = ptrImgBuff->nImageSizeY;

	if (ptrImgBuff->imgTop_R)
	{
		PIAL::PI_Buff* buff = new PIAL::PI_Buff(ptrImgBuff->imgTop_R, InspImageData->nImageSizeX, InspImageData->nImageSizeY, InspImageData->nImageSizeX);
		InspImageData->AddBuff(PIAL::Top_R, buff);
	}
	if (ptrImgBuff->imgTop_G)
	{
		PIAL::PI_Buff* buff = new PIAL::PI_Buff(ptrImgBuff->imgTop_G, InspImageData->nImageSizeX, InspImageData->nImageSizeY, InspImageData->nImageSizeX);
		InspImageData->AddBuff(PIAL::Top_G, buff);
	}
	if (ptrImgBuff->imgTop_B)
	{
		PIAL::PI_Buff* buff = new PIAL::PI_Buff(ptrImgBuff->imgTop_B, InspImageData->nImageSizeX, InspImageData->nImageSizeY, InspImageData->nImageSizeX);
		InspImageData->AddBuff(PIAL::Top_B, buff);
	}
	if (ptrImgBuff->imgTop_W)
	{
		PIAL::PI_Buff* buff = new PIAL::PI_Buff(ptrImgBuff->imgTop_W, InspImageData->nImageSizeX, InspImageData->nImageSizeY, InspImageData->nImageSizeX);
		InspImageData->AddBuff(PIAL::Top_W, buff);
	}
	if (ptrImgBuff->imgMiddle_R)
	{
		PIAL::PI_Buff* buff = new PIAL::PI_Buff(ptrImgBuff->imgMiddle_R, InspImageData->nImageSizeX, InspImageData->nImageSizeY, InspImageData->nImageSizeX);
		InspImageData->AddBuff(PIAL::Middle_R, buff);
	}
	if (ptrImgBuff->imgMiddle_B)
	{
		PIAL::PI_Buff* buff = new PIAL::PI_Buff(ptrImgBuff->imgMiddle_B, InspImageData->nImageSizeX, InspImageData->nImageSizeY, InspImageData->nImageSizeX);
		InspImageData->AddBuff(PIAL::Middle_B, buff);
	}
	if (ptrImgBuff->imgBottom_R)
	{
		PIAL::PI_Buff* buff = new PIAL::PI_Buff(ptrImgBuff->imgBottom_R, InspImageData->nImageSizeX, InspImageData->nImageSizeY, InspImageData->nImageSizeX);
		InspImageData->AddBuff(PIAL::Bottom_R, buff);
	}
	if (ptrImgBuff->imgBottom_B)
	{
		PIAL::PI_Buff* buff = new PIAL::PI_Buff(ptrImgBuff->imgBottom_B, InspImageData->nImageSizeX, InspImageData->nImageSizeY, InspImageData->nImageSizeX);
		InspImageData->AddBuff(PIAL::Bottom_B, buff);
	}

	return true;
}

bool PInspAlgoWrapper::ConvertAlgo(AlgoBlob* algo, PIAL::_AlgoBlob& algo2)
{
	//DLL 사용하지않으므로 Wrapper 작업 X 
	memset(&algo2, 0, sizeof(PIAL::_AlgoBlob));

	algo2.m_byIPCClass = algo->m_byIPCClass;
	algo2.m_bInvertCheck = algo->m_bInvertCheck;
	algo2.m_dAreaMin = algo->m_dAreaMin;
	algo2.m_dAreaMax = algo->m_dAreaMax;
	algo2.m_nTypeSelectBlob = algo->m_nTypeSelectBlob;
	algo2.m_bFillHole = algo->m_bFillHole;

	algo2.m_bFilterIsUse = algo->m_bFilterIsUse;
	algo2.m_nFilterStepNarrow = algo->m_nFilterStepNarrow;

	algo2.m_bInsp2D = algo->m_bInsp2D;
	algo2.m_nMinBinary = algo->m_nMinBinary;
	algo2.m_nMaxBinary = algo->m_nMaxBinary;
	algo2.m_nTypeRange2D = algo->m_nTypeRange2D;

	algo2.m_bInsp3D = algo->m_bInsp3D;
	algo2.m_dHeightRateMin = algo->m_dHeightRateMin;
	algo2.m_dHeightRateMax = algo->m_dHeightRateMax;
	algo2.m_fHeightAvg = algo->m_fHeightAvg;
	algo2.m_nTypeRange3D = algo->m_nTypeRange3D;

	algo2.m_dTechCenterX = algo->m_dTechCenterX;
	algo2.m_dTechCenterY = algo->m_dTechCenterY;

	algo2.m_bUseIPC = algo->m_bUseIPC;

	algo2.m_bTeachWidthUse = algo->m_bTeachWidthUse;
	algo2.m_dTeachWidth = algo->m_dTeachWidth;
	algo2.m_dTeachWidthRateMin = algo->m_dTeachWidthRateMin;
	algo2.m_dTeachWidthRateMax = algo->m_dTeachWidthRateMax;
	algo2.m_bTeachLengthUse = algo->m_bTeachLengthUse;
	algo2.m_dTeachLength = algo->m_dTeachLength;
	algo2.m_dTeachLengthRateMin = algo->m_dTeachLengthRateMin;
	algo2.m_dTeachLengthRateMax = algo->m_dTeachLengthRateMax;

	algo2.m_bShiftIsUse = algo->m_bShiftIsUse;
	algo2.m_bShiftXUse = algo->m_bShiftXUse;
	algo2.m_bShiftYUse = algo->m_bShiftYUse;

	algo2.m_dShiftX = algo->m_dShiftX;
	algo2.m_dShiftY = algo->m_dShiftY;
	algo2.m_dAreaCurrent = algo->m_dAreaCurrent;
	algo2.m_bAreaIsUse = algo->m_bAreaIsUse;

	algo2.m_bUseBlobNG = algo->m_bUseBlobNG;
	algo2.m_dBlobSizeWidth = algo->m_dBlobSizeWidth;
	algo2.m_dBlobSizeLength = algo->m_dBlobSizeLength;

	algo2.m_bUseHeight = algo->m_bUseHeight;
	algo2.m_dTeachHeight = algo->m_dTeachHeight;
	algo2.m_dTeachHeightMax = algo->m_dTeachHeightMax;
	algo2.m_dTeachHeightMin = algo->m_dTeachHeightMin;

	algo2.m_bIsModelSubImg = algo->m_bIsModelSubImg;
	algo2.m_bUsePattern = algo->m_bUsePattern;
	algo2.m_bUseFPBW = algo->m_bUseFPBW;
	algo2.m_byFPMargin = algo->m_byFPMargin;

	algo2.m_bUseBlobSizeDistance = algo->m_bUseBlobSizeDistance;
	algo2.m_dBlobSizeDistance = algo->m_dBlobSizeDistance;

	algo2.m_bCircleOpt = algo->m_bCircleOpt;
	algo2.m_bInspCoil = algo->m_bInspCoil;

	algo2.m_nCoilThMax = algo->m_nCoilThMax;
	algo2.m_nCoilThMin = algo->m_nCoilThMin;

	algo2.m_nBlobSizeDistanceType = algo->m_nBlobSizeDistanceType;

	algo2.m_bUseThick = algo->m_bUseThickMax;
	algo2.m_fThick = algo->m_fThickMax;
	return true;
}

bool PInspAlgoWrapper::ConvertAlgo(AlgoBlob* algo, PIAL::_AlgoBump& algo2)
{
	memset(&algo2, 0, sizeof(PIAL::_AlgoBump));

	algo2.m_dAreaMin = algo->m_dAreaMin;
	algo2.m_dAreaMax = algo->m_dAreaMax;
	algo2.m_nTypeSelectBlob = algo->m_nTypeSelectBlob;
	algo2.m_bFillHole = algo->m_bFillHole;

	algo2.m_bFilterIsUse = algo->m_bFilterIsUse;
	algo2.m_nFilterStepNarrow = algo->m_nFilterStepNarrow;

	algo2.m_bInsp2D = algo->m_bInsp2D;
	algo2.m_nMinBinary = algo->m_nMinBinary;
	algo2.m_nMaxBinary = algo->m_nMaxBinary;
	algo2.m_nTypeRange2D = algo->m_nTypeRange2D;

	algo2.m_bInsp3D = algo->m_bInsp3D;
	algo2.m_dHeightRateMin = algo->m_dHeightRateMin;
	algo2.m_dHeightRateMax = algo->m_dHeightRateMax;
	algo2.m_fHeightAvg = algo->m_fHeightAvg;
	algo2.m_nTypeRange3D = algo->m_nTypeRange3D;

	algo2.m_dTechCenterX = algo->m_dTechCenterX;
	algo2.m_dTechCenterY = algo->m_dTechCenterY;

	algo2.m_bTeachWidthUse = algo->m_bTeachWidthUse;
	algo2.m_dTeachWidth = algo->m_dTeachWidth;
	algo2.m_dTeachWidthRateMin = algo->m_dTeachWidthRateMin;
	algo2.m_dTeachWidthRateMax = algo->m_dTeachWidthRateMax;
	algo2.m_bTeachLengthUse = algo->m_bTeachLengthUse;
	algo2.m_dTeachLength = algo->m_dTeachLength;
	algo2.m_dTeachLengthRateMin = algo->m_dTeachLengthRateMin;
	algo2.m_dTeachLengthRateMax = algo->m_dTeachLengthRateMax;

	algo2.m_bShiftIsUse = algo->m_bShiftIsUse;
	algo2.m_bShiftXUse = algo->m_bShiftXUse;
	algo2.m_bShiftYUse = algo->m_bShiftYUse;

	algo2.m_dShiftX = algo->m_dShiftX;
	algo2.m_dShiftY = algo->m_dShiftY;
	algo2.m_dAreaCurrent = algo->m_dAreaCurrent;
	algo2.m_bAreaIsUse = algo->m_bAreaIsUse;

	algo2.m_bUseBlobNG = algo->m_bUseBlobNG;
	algo2.m_dBlobSizeWidth = algo->m_dBlobSizeWidth;
	algo2.m_dBlobSizeLength = algo->m_dBlobSizeLength;
	algo2.m_bUseBlobSizeLength = algo->m_bUseBlobSizeLength;
	algo2.m_bUseBlobSizeWidth = algo->m_bUseBlobSizeWidth;

	algo2.m_bUseHeight = algo->m_bUseHeight;
	algo2.m_dTeachHeight = algo->m_dTeachHeight;
	algo2.m_dTeachHeightMax = algo->m_dTeachHeightMax;
	algo2.m_dTeachHeightMin = algo->m_dTeachHeightMin;

	algo2.m_bCircleOpt = algo->m_bCircleOpt;

	algo2.m_bUseCircleRate = algo->m_bUseCircleRate;
	algo2.m_dCircleRate = algo->m_dCircleRate;

	algo2.m_bUseUnCoining = algo->m_bUseBlobSizeDistance;
	algo2.m_dReverseArea = algo->m_dBlobSizeDistance;

	return true;
}

bool PInspAlgoWrapper::ConvertAlgo(AlgoBump* algo, PIAL::_AlgoBump& algo2)
{
	memset(&algo2, 0, sizeof(PIAL::_AlgoBump));
	algo2.m_dAreaMin = algo->farrdata[Bump_F_AreaMin];			// AreaMin,Coining Min 티칭값
	algo2.m_dAreaMax = algo->farrdata[Bump_F_AreaMax];			// AreaMin,Coining Max(100) 티칭값
	algo2.m_nTypeSelectBlob = 3;								// BlobType (무조건 NearBlob일것같아서 3 넣어둠)
	algo2.m_bFillHole = algo->idata & Bump_Data_UseFillHole;	// Fillhole 사용유무
	algo2.m_bFilterIsUse = algo->idata&Bump_Data_Filter;		// Filter 사용유무
	algo2.m_nFilterStepNarrow = algo->narrdata[Bump_by_Filter];	// Filter Step
	algo2.m_bInsp2D = algo->idata&Bump_Data_Bin2D;				// 2D Threshold 사용유무
	algo2.m_nMinBinary = algo->narrdata[Bump_by_Min2D];			// 2D Threshold 최소값
	algo2.m_nMaxBinary = algo->narrdata[Bump_by_Max2D];			// 2D Threshold 최대값
	algo2.m_nTypeRange2D = algo->narrdata[Bump_by_Range2D];		// 2D Threshold RangeType
	algo2.m_bInsp3D = algo->idata&Bump_Data_Bin3D;				// 3D Threshold 사용유무
	algo2.m_dHeightRateMin = algo->farrdata[Bump_F_Min3D];		// 3D Threshold 최소값
	algo2.m_dHeightRateMax = algo->farrdata[Bump_F_Max3D];		// 3D Threshold 최대값
	algo2.m_fHeightAvg = algo->farrdata[Bump_F_TeachHeight];	// 이건잘모르겠음
	algo2.m_nTypeRange3D = algo->narrdata[Bump_by_Range3D];		// 3D Threshold RangeType
	algo2.m_dTechCenterX = algo->farrdata[Bump_F_TechCenterX];	// 범프의 TeachCenter 좌표
	algo2.m_dTechCenterY = algo->farrdata[Bump_F_TechCenterY];	// 범프의 TeachCenter 좌표
	algo2.m_bTeachWidthUse = algo->idata&Bump_Data_UseWidth;	// Width/단축 사용유무
	algo2.m_dTeachWidth = algo->farrdata[Bump_F_TeachWidth];	// TeachWidth값
	algo2.m_dTeachWidthRateMin = algo->farrdata[Bump_F_WidthRateMin];			// TeachWidthMin 값
	algo2.m_dTeachWidthRateMax = algo->farrdata[Bump_F_WidthRateMax];			// TeachWidthMax 값
	algo2.m_bTeachLengthUse = algo->idata&Bump_Data_UseLength;					// Length/장축 사용유무
	algo2.m_dTeachLength = algo->farrdata[Bump_F_TeachLength];					// TeachLength값
	algo2.m_dTeachLengthRateMin = algo->farrdata[Bump_F_LengthRateMin];			// TeachLengthMin값
	algo2.m_dTeachLengthRateMax = algo->farrdata[Bump_F_LengthRateMax];			// TeachLengthMax값
	algo2.m_bShiftIsUse = algo->idata&Bump_Data_UseShiftX;						// Shift 사용유무
	algo2.m_bShiftXUse = algo2.m_bShiftIsUse;			// ShiftX 사용유무(의미없음)
	algo2.m_bShiftYUse = algo2.m_bShiftIsUse;			// ShiftY 사용유무(의미없음)
	algo2.m_dShiftX = algo->farrdata[Bump_F_TeachShiftX];		// TeachShiftX값
	algo2.m_dShiftY = algo->farrdata[Bump_F_TeachShiftY];		// TeachShiftY값
	algo2.m_dAreaCurrent = algo->farrdata[Bump_F_Area];			// TeachArea 값
	algo2.m_bAreaIsUse = algo->idata&Bump_Data_UseArea;			// Area/Coining 검사유무
	algo2.m_bUseBlobNG = (algo->idata&Bump_Data_UseNGBumpL) | (algo->idata&Bump_Data_UseNGBumpW);		// NGBump 사용유무 
	algo2.m_dBlobSizeWidth = algo->farrdata[Bump_F_NGBumpW];			// NGBumpW 사용유무 
	algo2.m_dBlobSizeLength = algo->farrdata[Bump_F_NGBumpL];			//NGBumpL 사용유무 
	algo2.m_bUseBlobSizeLength = algo->idata&Bump_Data_UseNGBumpL;			// NGBumpW 티칭값
	algo2.m_bUseBlobSizeWidth = algo->idata&Bump_Data_UseNGBumpW;			// NGBumpL 티칭값
	algo2.m_bUseHeight = algo->idata&Bump_Data_UseHeight;			// Height 검사 사용유무
	algo2.m_dTeachHeight = algo->farrdata[Bump_F_TeachHeight];			// Teach Height  값
	algo2.m_dTeachHeightMax = algo->farrdata[Bump_F_HeightMax];			// TeachHeight Max 값
	algo2.m_dTeachHeightMin = algo->farrdata[Bump_F_HeightMin];			// TeachHeight Min 값
	algo2.m_bCircleOpt = 0;// 이건뭔지 모르겠음 참조찾아봤는데 안쓰이는것같아서 그냥 False 넣음
	algo2.m_bUseCircleRate = algo->idata&Bump_Data_UseCirclerate;			// CircleRate 사용유무
	algo2.m_dCircleRate = algo->farrdata[Bump_F_CircleRate];			// Teach Circle Rate 값
	algo2.m_bUseUnCoining = algo->idata&Bump_Data_UseUnCoining;			// Uncoining 사용여부
	algo2.m_dReverseArea = algo->farrdata[Bump_F_UncoiningValue];			// Teach Uncoining 값
	algo2.UseBridge = (algo->idata & Bump_Data_UseBridge) == Bump_Data_UseBridge;
	algo2.m_bUsePitch = (algo->idata & Bump_Data_UsePitch) == Bump_Data_UsePitch;
	algo2.m_dTolPitch = algo->farrdata[Bump_F_TolPitch]; 

	algo2.m_bUseShiftR = (algo->idata & Bump_Data_UseShiftR) == Bump_Data_UseShiftR;
	algo2.m_dShiftR = algo->farrdata[Bump_F_TolShiftR];

	algo2.m_bUseContrast = (algo->idata & Bump_Data_UseContrast) == Bump_Data_UseContrast;
	algo2.m_dTolContrast = algo->farrdata[Bump_F_TolContrast];
	algo2.m_bUseHeightDev = (algo->idata &Bump_Data_UseHeightDev) == Bump_Data_UseHeightDev;
	algo2.m_dHeightDev = algo->farrdata[Bump_F_HeightDev];
	algo2.m_HeightAreaType = algo->narrdata[Bump_by_HeightAreaType];
	algo2.m_HeightAreaType_COP = algo->narrdata[Bump_by_HeightAreaType_COP];
	algo2.m_bUseVolume = algo->idata&Bump_Data_UseVolume;			// Volume 사용여부
	algo2.m_dTeachVolume = algo->farrdata[Bump_F_TeachVolume];			// Teach Volume 값
	algo2.m_dTeachVolumeRateMin = algo->farrdata[Bump_F_VolumeRateMin];			// Teach Volume Min Rate 값
	algo2.m_dTeachVolumeRateMax = algo->farrdata[Bump_F_VolumeRateMax];			// Teach Volume Max Rate 값

	algo2.m_bUseAbsoluteHeight = (algo->idata & Bump_Data_UseAbsoluteHeight) == Bump_Data_UseAbsoluteHeight;
	return true;
}

bool PInspAlgoWrapper::ConvertAlgo(AlgoBGA* algo, PIAL::_AlgoBGA& algo2)
{
	memset(&algo2, 0, sizeof(PIAL::_AlgoBGA));

	//SetGolden_KTL(algo2);

	algo2.m_dAreaMin = algo->m_dAreaMin;
	algo2.m_dAreaMax = algo->m_dAreaMax;
	algo2.m_nTypeSelectBlob = algo->m_nTypeSelectBlob;
	algo2.m_bFillHole = algo->m_bFillHole;

	algo2.m_bFilterIsUse = algo->m_bFilterIsUse;
	algo2.m_nFilterStepNarrow = algo->m_nFilterStepNarrow;

	algo2.m_bInsp2D = algo->m_bInsp2D;
	algo2.m_nMinBinary = algo->m_nMinBinary;
	algo2.m_nMaxBinary = algo->m_nMaxBinary;
	algo2.m_nTypeRange2D = algo->m_nTypeRange2D;

	algo2.m_bInsp3D = algo->m_bInsp3D;
	algo2.m_dHeightRateMin = algo->m_dHeightRateMin;
	algo2.m_dHeightRateMax = algo->m_dHeightRateMax;
	algo2.m_nTypeRange3D = algo->m_nTypeRange3D;

	algo2.m_bTeachWidthUse = algo->m_bTeachWidthUse;
	algo2.m_dTeachWidth = algo->m_dTeachWidth;
	algo2.m_dTeachWidthRateMin = algo->m_dTeachWidthRateMin;
	algo2.m_dTeachWidthRateMax = algo->m_dTeachWidthRateMax;
	algo2.m_bTeachLengthUse = algo->m_bTeachLengthUse;
	algo2.m_dTeachLength = algo->m_dTeachLength;
	algo2.m_dTeachLengthRateMin = algo->m_dTeachLengthRateMin;
	algo2.m_dTeachLengthRateMax = algo->m_dTeachLengthRateMax;

	algo2.m_bShiftXUse = algo->m_bShiftXUse;
	algo2.m_bShiftYUse = algo->m_bShiftYUse;

	algo2.m_bUseHeight = algo->m_bUseHeight;
	algo2.m_dTeachHeight = algo->m_dTeachHeight;
	algo2.m_dTeachHeightMax = algo->m_dTeachHeightMax;
	algo2.m_dTeachHeightMin = algo->m_dTeachHeightMin;
	algo2.m_InspAreaType = algo->m_nInspType;

	algo2.m_bUseNGBump = algo->m_bUseNGBump;
	algo2.m_nNGBumpType = algo->m_nNGBumpType;  // 0: defalt  1: In Bump ,2 :Except Bump
	algo2.m_Blob_NGBump.m_bFillHole = algo->m_bUseNGBumpFillHole;
	algo2.m_Blob_NGBump.m_bFilterIsUse = algo->m_bNGBumpFillterUse;
	algo2.m_Blob_NGBump.m_bInsp2D = algo->m_InspNGBump2D;
	algo2.m_Blob_NGBump.m_bInsp3D = algo->m_InspNGBump3D;
	algo2.m_Blob_NGBump.m_dHeightRateMax = algo->m_NGBumpHeightRateMax;
	algo2.m_Blob_NGBump.m_dHeightRateMin = algo->m_NGBumpHeightRateMin;
	algo2.m_Blob_NGBump.m_nFilterStepNarrow = algo->m_nNGBumpFilterCnt;
	algo2.m_Blob_NGBump.m_nMaxBinary = algo->m_NGBumpMaxBinary;
	algo2.m_Blob_NGBump.m_nMinBinary = algo->m_NGBumpMinBinary;
	algo2.m_Blob_NGBump.m_nTypeRange2D = algo->m_TypeRangeNGBump2D;
	algo2.m_Blob_NGBump.m_nTypeRange3D = algo->m_TypeRangeNGBump3D;

	algo2.m_bUseUnCoining = algo->m_bUseUncoining;
	algo2.m_Blob_uncoining.m_bFillHole = algo->m_bUseUncoiningFillHole;
	algo2.m_Blob_uncoining.m_bFilterIsUse = algo->m_bUncoiningFillterUse;
	algo2.m_Blob_uncoining.m_bInsp2D = algo->m_InspUncoining2D;
	algo2.m_Blob_uncoining.m_bInsp3D = algo->m_InspUncoining3D;
	algo2.m_Blob_uncoining.m_dHeightRateMax = algo->m_UncoiningHeightRateMax;
	algo2.m_Blob_uncoining.m_dHeightRateMin = algo->m_UncoiningHeightRateMin;
	algo2.m_Blob_uncoining.m_nFilterStepNarrow = algo->m_nUncoiningFilterCnt;
	algo2.m_Blob_uncoining.m_nMaxBinary = algo->m_UncoiningMaxBinary;
	algo2.m_Blob_uncoining.m_nMinBinary = algo->m_UncoiningMinBinary;
	algo2.m_Blob_uncoining.m_nTypeRange2D = algo->m_TypeRangeUncoining2D;
	algo2.m_Blob_uncoining.m_nTypeRange3D = algo->m_TypeRangeUncoining3D;
// 
	algo2.m_bUseCoplanarity = algo->m_bUseCoplanarity;
	algo2.m_CoplanarMax = algo->m_CoplanarMax;
	algo2.m_CoplanarMin = algo->m_CoplanarMin;

	algo2.m_nCoplOption = algo->m_nCoplOption;


	algo2.m_HeightAreaType = algo->m_HeightAreaType;
	algo2.m_HeightAreaType_COP = algo->m_HeightAreaType_COP;
	algo2.m_SelectOffSetType = algo->m_SelectOffSetType;
	algo2.m_bUseAbsoluteHeight = algo->m_bUseAbsoluteHeight;
	algo2.m_bUsePitch = algo->m_bUsePitch;
	algo2.m_dTolPitch = algo->m_dTolPitch;

	ConvertColorBase(&algo->m_sAlgoColorBase, &algo2.m_sAlgoColorBase);
// 
// 	algo2.m_bUsePitch = algo->m_bUsePitch;
// 	algo2.m_dTolPitch = algo->m_dTolPitch;
// 	algo2.m_bUseContrast = algo->m_bUseContrast;
// 	algo2.m_dTolContrast = algo->m_dTolContrast;
// 
// 	algo2.m_bUseGridOffsetX = algo->m_bUseGridOffsetX;
// 	algo2.m_bUseGridOffsetY = algo->m_bUseGridOffsetY;
// 	algo2.m_dTolGridOffsetX = algo->m_dTolGridOffsetX;
// 	algo2.m_dTolGridOffsetY = algo->m_dTolGridOffsetY;
// 
// 	algo2.m_UseGridAlign = algo->m_UseGridAlign;
// 	algo2.m_HeightAreaType = algo->m_HeightAreaType;
// 	algo2.m_nCoplOption = algo->m_nCoplOption;
// 	algo2.m_bUseGoldenDevice = algo->m_bUseGoldenDevice;
// 	algo2.m_UseNormalizeBumpWidth = algo->m_UseNormalizeBumpWidth;

	if (algo2.m_bUseGoldenDevice)
	{
		if (SetGolden_KTL(algo2) == false)
		{
			algo2.m_nIndexTarget1Bump = 14;
			algo2.m_nIndexTarget2Bump = 2;
			algo2.m_nIndexCopleBump = 14;

			/*algo2.m_nIndexTarget1Bump = algo->m_nIndexTarget1Bump;
			algo2.m_nIndexTarget2Bump = algo->m_nIndexTarget2Bump;
			algo2.m_nIndexCopleBump = algo->m_nIndexCopleBump;*/
		}

	}

	return true;
}



bool PInspAlgoWrapper::ConvertAlgo(AlgoTilt* algo, PIAL::_AlgoTilt& algo2)
{
	/*if (sizeof(AlgoTilt) != sizeof(PIAL::_AlgoTilt))
		return false;*/
	memcpy(&algo2, algo, sizeof(PIAL::_AlgoTilt));
	return true;
}

bool PInspAlgoWrapper::ConvertAlgo(AlgoColor* algo, PIAL::_AlgoColor& algo2)
{
	memset(&algo2, 0, sizeof(PIAL::_AlgoColor));
	algo2.m_bAutoSearchROI = algo->m_bAutoSearchROI;
	algo2.m_bInvert = algo->m_bInvert;
	algo2.m_bUseColorMap2 = algo->m_bUseColorMap2;
	algo2.m_bUsePolarity = algo->m_bUsePolarity;
	algo2.m_bUseRangeBar = algo->m_bUseRangeBar;
	algo2.m_dRateResult = algo->m_dRateResult;
	algo2.m_dRateStd = algo->m_dRateStd;
	algo2.m_dWndAngle = algo->m_dWndAngle;
	algo2.m_fBlueFactorBtm = algo->m_fBlueFactorBtm;
	algo2.m_fFatorBlue = algo->m_fFatorBlue;
	algo2.m_fFatorGreen = algo->m_fFatorGreen;
	algo2.m_fFatorRed = algo->m_fFatorRed;
	algo2.m_fRebFactorBtm = algo->m_fRebFactorBtm;
	algo2.m_nCntHistoResult = algo->m_nCntHistoResult;
	algo2.m_nCntHistoStd = algo->m_nCntHistoStd;
	algo2.m_nCntPoint = algo->m_nCntPoint;
	algo2.m_nCntRect = algo->m_nCntRect;
	algo2.m_nInspTypeColor = algo->m_nInspTypeColor;
	algo2.m_nLeadPosition = algo->m_nLeadPosition;
	algo2.m_nLeadTipDirection = algo->m_nLeadTipDirection;
	algo2.m_nPolygonCnt = algo->m_nPolygonCnt;
	algo2.m_nRangeMax = algo->m_nRangeMax;
	algo2.m_nRangeMin = algo->m_nRangeMin;
	algo2.m_nRangeMode = algo->m_nRangeMode;
	algo2.m_nSizeXCIE = algo->m_nSizeXCIE;
	algo2.m_nSizeYCIE = algo->m_nSizeYCIE;
	algo2.m_nSolderLength = algo->m_nSolderLength;
	algo2.m_nTypeTab = algo->m_nTypeTab;
	algo2.m_nViewColor = algo->m_nViewColor;

	// algo2.m_bUseRGB[i] = algo->m_bUseRGB[i];
	// algo2.m_byMax[i] = algo->m_byMax[i];
	// algo2.m_byMin[i] = algo->m_byMin[i];
	// algo2.m_byRange[i] = algo->m_byRange[i];
	// algo2.m_ptArrPolygon = algo->m_ptArrPolygon;
	memcpy(algo2.m_bUseRGB, algo->m_bUseRGB, COLORALGO_POLYGON_CNTS);
	memcpy(algo2.m_byMax, algo->m_byMax, COLORALGO_POLYGON_CNTS);
	memcpy(algo2.m_byMin, algo->m_byMin, COLORALGO_POLYGON_CNTS);
	memcpy(algo2.m_byRange, algo->m_byRange, COLORALGO_POLYGON_CNTS);
	memcpy(algo2.m_ptArrPolygon, algo->m_ptArrPolygon, sizeof(POINT) * (COLORALGO_POLYGON_CNTS * POLYGON_POINT_CNTS));
	memcpy(algo2.m_rcArrTabRect, algo->m_rcArrTabRect, BRIEDGE_CNT);

	return true;
}

bool PInspAlgoWrapper::ConvertAlgo(AlgoBW* algo, PIAL::_AlgoBW& rstAlgo)
{
	memset(&rstAlgo, 0, sizeof(PIAL::_AlgoBW));
	rstAlgo.m_b2dCheck = algo->m_b2dCheck;
	rstAlgo.m_b3dCheck = algo->m_b3dCheck;
	rstAlgo.m_bChipTracking = algo->m_bChipTracking;
	rstAlgo.m_bInspBWArea = algo->m_bInspBWArea;
	rstAlgo.m_bInvertCheck = algo->m_bInvertCheck;
	rstAlgo.m_bUseHeightMean = algo->m_bUseHeightMean;
	rstAlgo.m_bUseMaxOK = algo->m_bUseMaxOK;
	rstAlgo.m_bUseTeachingRate = algo->m_bUseTeachingRate;
	rstAlgo.m_byDir = algo->m_byDir;
	rstAlgo.m_byInspectionArea = algo->m_byInspectionArea;
	rstAlgo.m_byUseHeightOpt = algo->m_byUseHeightOpt;
	rstAlgo.m_d3dAvgHeight = algo->m_d3dAvgHeight;
	rstAlgo.m_d3dHeightMax = algo->m_d3dHeightMax;
	rstAlgo.m_d3dHeightMin = algo->m_d3dHeightMin;
	rstAlgo.m_dAreaCurrent = algo->m_dAreaCurrent;
	rstAlgo.m_dPercentOK = algo->m_dPercentOK;
	rstAlgo.m_dTeachingArea = algo->m_dTeachingArea;
	rstAlgo.m_fHeightMeanMax = algo->m_fHeightMeanMax;
	rstAlgo.m_fHeightMeanMin = algo->m_fHeightMeanMin;
	rstAlgo.m_n3dRange = algo->m_n3dRange;
	rstAlgo.m_nChipTrackingGap = algo->m_nChipTrackingGap;
	rstAlgo.m_nMaxOKPer = algo->m_nMaxOKPer;
	rstAlgo.m_nMaxValue = algo->m_nMaxValue;
	rstAlgo.m_nMinValue = algo->m_nMinValue;
	rstAlgo.m_nRange = algo->m_nRange;
	rstAlgo.m_nStdOKArea = algo->m_nStdOKArea;
	ConvertColorBase(&algo->m_sAlgoColorBase, &rstAlgo.m_sAlgoColorBase);
	ConvertAngleColorBase(&algo->m_sAngleColorBase, &rstAlgo.m_sAngleColorBase);
	return true;
}
bool PInspAlgoWrapper::ConvertAlgo(AlgoPatternDiff* algo, PIAL::_AlgoPatternDiff& algo2)
{
	memset(&algo2, 0, sizeof(PIAL::_AlgoPatternDiff));
	algo2.m_nLayerCnt = algo->m_nLayerCnt;
	algo2.m_bUsePattern = algo->m_bUsePattern;
	algo2.m_rcMatchingArea = algo->m_rcMatchingArea;
	algo2.m_nModelAddCnt = algo->m_nModelAddCnt;
	algo2.m_dTheta = algo->m_dTheta;
	algo2.m_bUseDarkForeign = algo->m_bUseDarkForeign;
	algo2.nSelectedModel = algo->nSelectedModel;
	
	algo2.m_bUseExcPattern = algo->m_bUseExcPattern;
	algo2.m_nExcPatternScore = algo->m_nExcPatternScore;
	algo2.m_nExcModelAddCnt = algo->m_nExcModelAddCnt;
	algo2.m_bUseArrAlignArea = algo->m_bUseArrAlignArea;
	algo2.m_nAcceptAlignScore = algo->m_nAcceptAlignScore;

	memcpy(algo2.m_nArrLayerType, algo->m_nArrLayerType, _MAX_LAYERCNT * sizeof(int));
	memcpy(algo2.m_stPolygon, algo->m_stPolygon, MAX_LAYERCNT * sizeof(PIAL::_tagLayerPolygon));
	memcpy(algo2.m_stBlobBase, algo->m_stBlobBase, MAX_LAYERCNT * sizeof(PIAL::_tagAlgoBlobBase));
	memcpy(algo2.m_dTeachLength, algo->m_dTeachLength, MAX_LAYERCNT * sizeof(double));
	memcpy(algo2.m_dTeachWidth, algo->m_dTeachWidth, MAX_LAYERCNT * sizeof(double));
	memcpy(algo2.m_dTeachArea, algo->m_dTeachArea, MAX_LAYERCNT * sizeof(double));
	memcpy(algo2.m_bUseFilter, algo->m_bUseFilter, MAX_LAYERCNT * sizeof(BOOL));
	memcpy(algo2.m_bNGBlobMode, algo->m_bNGBlobMode, MAX_LAYERCNT * sizeof(BOOL));
	memcpy(algo2.m_sModelPath, algo->m_sModelPath, MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_stHistoBase, algo->m_stHistoBase, MAX_LAYERCNT * sizeof(PIAL::_tagHistoParamBase));
	memcpy(algo2.m_bUseSizeAnd, algo->m_bUseSizeAnd, MAX_LAYERCNT * sizeof(BOOL));

	//ModelPath
	memcpy(algo2.m_sModelPath1, algo->m_sModelPath1, MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_sModelPath2, algo->m_sModelPath2, MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_sModelPath3, algo->m_sModelPath3, MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_sModelPath4, algo->m_sModelPath4, MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_sModelPath5, algo->m_sModelPath5, MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_sModelPath6, algo->m_sModelPath6, MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_sModelPath7, algo->m_sModelPath7, MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_sModelPath8, algo->m_sModelPath8, MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_sModelPath9, algo->m_sModelPath9, MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_sModelPath10, algo->m_sModelPath10, MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_sModelPath11, algo->m_sModelPath11, MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_sModelPath12, algo->m_sModelPath12, MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_sModelPath13, algo->m_sModelPath13, MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_sModelPath14, algo->m_sModelPath14, MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_sModelPath15, algo->m_sModelPath15, MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_sModelPath16, algo->m_sModelPath16, MAX_STRLEN * sizeof(wchar_t));
	
	memcpy(&algo2.m_stExcPath, &algo->m_stExcPath, sizeof(PIAL::_tagModelPath));

	memcpy(algo2.m_stArrLightData, algo->m_stArrLightData, MAX_LAYERCNT * sizeof(PIAL::_lightData));
	
	memcpy(algo2.m_bArrUseMergeLayer, algo->m_bArrUseMergeLayer, MAX_LAYERCNT * sizeof(BOOL));

	memcpy(algo2.m_bUseEdgeFilter, algo->m_bUseEdgeFilter, MAX_LAYERCNT * sizeof(BOOL));

	memcpy(algo2.m_nFilterSize, algo->m_nFilterSize, MAX_LAYERCNT * sizeof(int));
	memcpy(algo2.m_nEdgeFilterLevel, algo->m_nEdgeFilterLevel, MAX_LAYERCNT * sizeof(int));
	memcpy(algo2.m_rcArrAlignArea, algo->m_rcArrAlignArea, MAX_ALIGNCNT * sizeof(RECT));
	algo2.m_bPatternMatching = algo->m_bPatternMatching;

	memcpy(algo2.m_bUseMaskBW, algo->m_bUseMaskBW, MAX_LAYERCNT * sizeof(BOOL));
	memcpy(&algo2.m_stBlobBase_Mask, &algo->m_stBlobBase_Mask, sizeof(PIAL::_tagAlgoBlobBase));
	algo2.m_UseFilter_Mask = algo->m_UseFilter_Mask;
	algo2.m_nFilterSize_Mask = algo->m_nFilterSize_Mask;

	return true;
}
bool PInspAlgoWrapper::ConvertAlgo(AlgoShapeX* algo, PIAL::_AlgoShapeX& algo2)
{
	algo2.dPartangle = algo->dPartangle;
	algo2.m_nLeadTipDirection = algo->m_nLeadTipDirection;

	// sjb++ 250403.[41] ~PInspAlgoWrapper.cpp~
	algo2.m_nTypeSelectBlob = algo->m_nTypeSelectBlob;
	algo2.m_nTypeSelectTarget = algo->m_nTypeSelectTarget;
	// esjb++ 250403.[41]

	algo2.ROICnt = algo->ROICnt;
	algo2.MatchSc = algo->MatchSc;
	algo2.nHist1 = algo->nHist1;
	algo2.nHistLow1 = algo->nHistLow1;
	algo2.nHistUp1 = algo->nHistUp1;
	algo2.nHist2 = algo->nHist2;
	algo2.nHistLow2 = algo->nHistLow2;
	algo2.nHistUp2 = algo->nHistUp2;
	PInspAlgoWrapper::ConvertBlobBase(&algo->m_sBlobBase_ShapeArea, &algo2.m_sBlobBase_ShapeArea);
	PInspAlgoWrapper::ConvertBlobBase(&algo->m_sBlobBase_NG, &algo2.m_sBlobBase_NG);
	PInspAlgoWrapper::ConvertBlobBase(&algo->m_sBlobBase_NG2, &algo2.m_sBlobBase_NG2);

	for (int i = 0; i < nShapeXCnt; i++)
	{
		ConvertShapeXROI(&algo->spROI[i], &algo2.spROI[i]);
	}
	memcpy(algo2.ArrPathModel, algo->ArrPathModel, MAX_STRLEN * sizeof(wchar_t));

	algo2.nInspOption = algo->nInspOption;
	algo2.Aspectratio = algo->Aspectratio;
	algo2.MinScarThickness = algo->MinScarThickness;
	algo2.Verticalmaxlength = algo->Verticalmaxlength;
	algo2.Horizontalmaxlength = algo->Horizontalmaxlength;
	algo2.MaxNGArea = algo->MaxNGArea;
	algo2.CriticalArea = algo->CriticalArea;
	algo2.MaxChippingLength = algo->MaxChippingLength;
	algo2.nStartIdx = algo->nStartIdx;
	algo2.fTieBarRate = algo->fTieBarRate;
	algo2.WrForeignCnt = algo->WrForeignCnt;
	algo2.WrForeignWidth = algo->WrForeignWidth;
	algo2.WrForeignLength = algo->WrForeignLength;
	algo2.NGGroupingMaxSize = algo->NGGroupingMaxSize;
	algo2.NGGroupingDistance = algo->NGGroupingDistance;
	algo2.ContrastValue = algo->ContrastValue;
	algo2.Check3DMaxVal = algo->Check3DMaxVal;
	algo2.Check3DMinVal = algo->Check3DMinVal;
	algo2.CheckSearchByShape = algo->CheckSearchByShape;
	algo2.nWndId = algo->nWndId;

	return true;
}
bool PInspAlgoWrapper::ConvertColorBase(tagAlgoColorBase* org, PIAL::_tagAlgoColorBase* rst)
{
	rst->m_bInvert = org->m_bInvert;
	rst->m_bUseAngleColor = org->m_bUseAngleColor;
	rst->m_bUseColor = org->m_bUseColor;
	rst->m_nPolygonCnt = org->m_nPolygonCnt;
	rst->m_byType = org->m_byType;
	rst->m_byColorLightType = org->m_byColorLightType;
	rst->m_bUseIntensity = org->m_bUseIntensity;
	rst->m_nRangeType = org->m_nRangeType;
	rst->m_nRangeMin = org->m_nRangeMin;
	rst->m_nRangeMax = org->m_nRangeMax;
	memcpy(rst->m_bUseRGB, org->m_bUseRGB, sizeof(BOOL) * _COLORALGO_POLYGON_CNTS);
	memcpy(rst->m_byMax, org->m_byMax, sizeof(BYTE) * _COLORALGO_POLYGON_CNTS);
	memcpy(rst->m_byMin, org->m_byMin, sizeof(BYTE) * _COLORALGO_POLYGON_CNTS);
	memcpy(rst->m_byRange, org->m_byRange, sizeof(BYTE) * _COLORALGO_POLYGON_CNTS);
	memcpy(rst->m_ptArrPolygon, org->m_ptArrPolygon, sizeof(POINT) * _COLORALGO_POLYGON_CNTS * _POLYGON_POINT_CNTS);
	return true;
}

bool PInspAlgoWrapper::ConvertAngleColorBase(tagAngleColorBase* org, PIAL::_tagAngleColorBase* rst)
{
	memcpy(rst->m_nArrInspAC, org->m_nArrInspAC, sizeof(int) * InspAC_T3_Total);
	memcpy(rst->m_byArrInspAC, org->m_byArrInspAC, sizeof(byte) * InspAC_T2_Total);
	memcpy(rst->m_fArrInspAC, org->m_fArrInspAC, sizeof(float) * InspAC_T_Total);
	return true;
}

bool PInspAlgoWrapper::ConvertShapeXROI(ShapeXROI* org, PIAL::_ShapeXROI* rst)
{
	rst->ROI.left = org->ROI.left;
	rst->ROI.top = org->ROI.top;
	rst->ROI.right = org->ROI.right;
	rst->ROI.bottom = org->ROI.bottom;

	rst->pCt.x = org->pCt.x;
	rst->pCt.y = org->pCt.y;

	rst->bUseShape = org->bUseShape;
	rst->bUseInner = org->bUseInner;
	rst->bUseExist = org->bUseExist;
	rst->bUseShift = org->bUseShift;
	rst->fShapeArea = org->fShapeArea;
	rst->fInnerArea = org->fInnerArea;
	rst->fExistArea = org->fExistArea;
	rst->fShiftX = org->fShiftX;
	rst->fShiftY = org->fShiftY;
	rst->nModelCnt = org->nModelCnt;
	for (int i = 0; i < rst->nModelCnt; i++)
		rst->ArrModel[i] = org->ArrModel[i];
		//rst->ArrModel[i] = (UINT16)org->ArrModel[i];

	rst->bUseExistShape = org->bUseExistShape;
	rst->fExistShapeArea = org->fExistShapeArea;

	// sjb++ 250409.[0] ~PInspAlgoWrapper~
	rst->fLocalLength = org->fLocalLength;
	rst->fLocalLength2 = org->fLocalLength2;
	rst->fDent = org->fDent;
	rst->fDentMin = org->fDentMin;
	rst->fDentMax = org->fDentMax;
	rst->bUseLocalLength = org->bUseLocalLength;
	rst->bUseDent = org->bUseDent;
	// esjb++ 250409.[0]

	rst->bUseForeignWidth = org->bUseForeignWidth;
	rst->bUseForeignLength = org->bUseForeignLength;
	rst->fForeignWidth = org->fForeignWidth;
	rst->fForeignLength = org->fForeignLength;

	rst->bUseForeignWLAnd = org->bUseForeignWLAnd;

	return true;
}
bool PInspAlgoWrapper::ConvertAlgo(AlgoNGBlob* algo, PIAL::_AlgoNGBlob& algo2)
{
	int nFMin = MIN(NGBlob_F_Total, _NGBlob_F_Total);
	for (int i = 0; i < nFMin; i++)
		algo2.farrdata[i] = algo->farrdata[i];
	algo2.idata = algo->idata;
	algo2.idata2 = algo->idata2;

	int nByMin = MIN(NGBlob_by_Total, _NGBlob_by_Total);
	for (int i = 0; i < nByMin; i++)
		algo2.narrdata[i] = algo->narrdata[i];
	//.7 주석처리 되어있음.
	for (int i = 0; i < TiltRectCnt; i++)
		algo2.m_rcHeightInRoi[i] = algo->m_rcHeightInRoi[i];
	ConvertColorBase(&algo->m_sAlgoColorBase, &algo2.m_sAlgoColorBase);

	if (algo->narrdata[NGBlob_by_inspType] == NGBlob_InspType_DetailWarpage) // detail Warpage 일 경우
	{
		algo2.SetModelPath(algo->GetModelPath(0), 0);
		algo2.SetModelPath(algo->GetModelPath(1), 1);
		return true;
	}
	algo2.m_nCntPatternPath = algo->m_nCntPatternPath;
	for (int i = 0; i < algo2.m_nCntPatternPath; i++)
		algo2.SetModelPath(algo->GetModelPath(i), i);

	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoBlob rst2, RstAlgoBlob* rst)
{
	memset(rst, 0, sizeof(RstAlgoBlob));

	rst->m_dRstArea = rst2.m_dRstArea;
	rst->m_dRstShiftX = rst2.m_dRstShiftX;
	rst->m_dRstShiftY = rst2.m_dRstShiftY;
	rst->m_dRstAreaRate = rst2.m_dRstAreaRate;
	rst->m_dRstHeightMean = rst2.m_dRstHeightMean;
	rst->m_bOKArea = rst2.m_bOKArea;
	rst->m_bOKShiftX = rst2.m_bOKShiftX;
	rst->m_bOKShiftY = rst2.m_bOKShiftY;
	rst->m_bOKHeight = rst2.m_bOKHeight;
	rst->m_bOKCoilRst = rst2.m_bOKCoilRst;

	rst->m_dRstWidth = rst2.m_dRstWidth;
	rst->m_dRstLength = rst2.m_dRstLength;
	rst->m_dRstDistance = rst2.m_dRstDistance;
	rst->m_bOKWidth = rst2.m_bOKWidth;
	rst->m_bOKLength = rst2.m_bOKLength;
	rst->m_bOKDistance = rst2.m_bOKDistance;
	rst->m_bOKArea_Reverse = rst2.m_bOKArea_Reverse;
	rst->m_dRstArea_Reverse = rst2.m_dRstArea_Reverse;
	rst->m_dTeachArea_Reverse = rst2.m_dTeachArea_Reverse;

	rst->m_bUseSizeWidth = rst2.m_bUseSizeWidth;
	rst->m_bUseSizeLength = rst2.m_bUseSizeLength;

	rst->m_nArrRectCnt = rst2.m_nArrRectCnt;
	rst->m_rcRectT = rst2.m_rcRectT;
	rst->m_rcRect_I = rst2.m_rcRect_I;
	memcpy(rst->m_rcArrRect, rst2.m_rcArrRect, BLOB_RECT_CNTS * sizeof(RECT));
	rst->m_bOKCircleRate = TRUE;

	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoBump rst2, RstAlgoBlob* rst)
{
	memset(rst, 0, sizeof(RstAlgoBlob));

	rst->m_dRstArea = rst2.m_dRstArea;
	rst->m_dRstShiftX = rst2.m_dRstShiftX;
	rst->m_dRstShiftY = rst2.m_dRstShiftY;
	rst->m_dRstHeightMean = rst2.m_dRstHeightMean;
	rst->m_bOKArea = rst2.m_bOKArea;
	rst->m_bOKShiftX = rst2.m_bOKShiftX;
	rst->m_bOKShiftY = rst2.m_bOKShiftY;
	rst->m_bOKHeight = rst2.m_bOKHeight;

	rst->m_dRstWidth = rst2.m_dRstWidth;
	rst->m_dRstLength = rst2.m_dRstLength;
	rst->m_dRstDistance = rst2.m_dRstDistance;
	rst->m_bOKWidth = rst2.m_bOKWidth;
	rst->m_bOKLength = rst2.m_bOKLength;
	rst->m_bOKDistance = rst2.m_bOKDistance;
	rst->m_bOKArea_Reverse = rst2.m_bOKArea_Reverse;
	rst->m_dRstArea_Reverse = rst2.m_dRstArea_Reverse;
	rst->m_dTeachArea_Reverse = rst2.m_dTeachArea_Reverse;


	rst->m_pShiftSt.x = rst2.m_bOKWidth_Reverse ? 1. : 0;
	rst->m_pShiftSt.y = rst2.m_bOKLength_Reverse ? 1. : 0;

	rst->m_pShiftEd.x = rst2.m_dRstWidth_Reverse;
	rst->m_pShiftEd.y = rst2.m_dRstLength_Reverse;

	rst->m_nArrRectCnt = rst2.m_nArrRectCnt;
	rst->m_rcRectT = rst2.m_rcRectT;
	rst->m_rcRect_I = rst2.m_rcRect_I;
	memcpy(rst->m_rcArrRect, rst2.m_rcArrRect, BLOB_RECT_CNTS * sizeof(RECT));

	rst->m_bOKCircleRate = rst2.m_bOKCircleRate;
	rst->m_dRstCircleRate = rst2.m_dRstCircleRate;

	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoBump rst2, RstAlgoBump* rst)
{
	memset(rst, 0, sizeof(RstAlgoBump));



	rst->fRstArea = rst2.m_dRstArea;
	rst->fRstShiftX = rst2.m_dRstShiftX;
	rst->fRstShiftY = rst2.m_dRstShiftY;
	rst->fRstHeight = rst2.m_dRstHeightMean;
	rst->bIsOKArea = rst2.m_bOKArea;
	rst->m_bOKShiftX = rst2.m_bOKShiftX;
	rst->m_bOKShiftY = rst2.m_bOKShiftY;
	rst->bIsOKHeight = rst2.m_bOKHeight;

	rst->fRstWidth = rst2.m_dRstWidth;
	rst->fRstLength = rst2.m_dRstLength;
	rst->fRstUncoining = rst2.m_dRstDistance;
	rst->bIsOKWidth = rst2.m_bOKWidth;
	rst->bIsOKLength = rst2.m_bOKLength;
	rst->bIsOKBridge = rst2.m_bOKDistance; // Bridge

	rst->bIsOKUnCoining = rst2.m_bOKArea_Reverse;
	rst->fRstUncoining = rst2.m_dRstArea_Reverse;

	//rst->m_dTeachArea_Reverse = rst2.m_dTeachArea_Reverse;
	//rst->m_bUseSizeWidth = rst2.m_bUseSizeWidth;
	//rst->m_bUseSizeLength = rst2.m_bUseSizeLength; NGBlob 에사용되는 변수같아서 주석처리함 NGBlob은 이제 Bump에서 안쓰임.

	rst->bIsOKNGBumpW = rst2.m_bOKWidth_Reverse ? 1. : 0;
	rst->bIsOKNGBumpL = rst2.m_bOKLength_Reverse ? 1. : 0;

	rst->fRstNGBumpW = rst2.m_dRstWidth_Reverse;
	rst->fRstNGBumpL = rst2.m_dRstLength_Reverse;

	rst->m_iFRectCnt = rst2.m_nArrRectCnt;
	//rst->m_rcRectT = rst2.m_rcRectT; // Teaching 영역 ?
	rst->m_rcRectI = rst2.m_rcRect_I;
	memcpy(rst->m_rcArrRectF, rst2.m_rcArrRect, BUMP_RECT_CNTS * sizeof(RECT));
	rst->bIsOKCircleRate = rst2.m_bOKCircleRate;
	rst->fRstCicleRate = rst2.m_dRstCircleRate;

	rst->bIsOKVolume = rst2.m_bOKVolume;
	rst->fRstVolume = rst2.m_dRstVolume;
	rst->fRstBottomHeight = rst2.m_dRstBottomHeight;

	rst->bIsOKPitch = rst2.m_bOKPitch;
	rst->fRstPitch = rst2.m_dRstPitch;

	rst->m_bOKShiftR = rst2.m_bOKShiftR;
	rst->fRstShiftR = rst2.m_dRstShiftR;

	rst->bIsOKContrast = rst2.m_bOKContrast;
	rst->fRstContrast = rst2.m_dRstContrast;
	rst->bIsOKHeightDev = rst2.m_bOKHeightDev;
	rst->fRstHeightDev = rst2.m_dRstHeightDev;
	rst->fRstCoplanarity = rst2.m_dRstCoplanarity;

	rst->m_rcRectT = rst2.m_rcRectT;
	rst->fRstBottomHeight = rst2.m_dRstBottomHeight;

	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoBGA rst2, RstAlgoBGA* rst)
{
	memset(rst, 0, sizeof(RstAlgoBGA));
	rst->m_bCoplanarityOK = rst2.m_bOKCoplanarity;
	rst->m_fCoplanarity = rst2.m_fRstCoplanarity;
	rst->m_rcRect_I = rst2.m_rcRect_I;

	rst->m_rcMinCoplanarity = rst2.m_rcMinCoplanarity;
	rst->m_rcMaxCoplanarity = rst2.m_rcMaxCoplanarity;

	memcpy(rst->m_fArrRstCoplan, rst2.m_fArrRstCoplan, sizeof(float)* eMMA_Total);
	memcpy(rst->m_fArrRstHeight, rst2.m_fArrRstHeight, sizeof(float)* eMMA_Total);
	memcpy(rst->m_fArrRstWidth, rst2.m_fArrRstWidth, sizeof(float)* eMMA_Total);
	
	/*
	rst->m_bOKGridOffsetX = rst2.m_bOKGridOffsetX;
	rst->m_bOKGridOffsetY = rst2.m_bOKGridOffsetY;

	rst->m_fRstGridOffsetX = rst2.m_fRstGridOffsetX;
	rst->m_fRstGridOffsetY = rst2.m_fRstGridOffsetY;
	rst->m_fRstTwist = rst2.m_fRstTwist;
	rst->m_bOKTwist = rst2.m_bOKTwist;*/

	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoNGBlob rst2, RstAlgoNGBlob* rst)
{
	memset(rst, 0, sizeof(RstAlgoNGBlob));
	rst->bIsOKArea = rst2.bIsOKArea;
	rst->bIsOKWidth = rst2.bIsOKWidth;
	rst->bIsOKLength = rst2.bIsOKLength;
	memcpy(rst->fRstWidth, rst2.fRstWidth, sizeof(float) * BLOB_RECT_CNTS);
	memcpy(rst->fRstLength, rst2.fRstLength, sizeof(float) * BLOB_RECT_CNTS);
	memcpy(rst->fRstArea, rst2.fRstArea, sizeof(float) * BLOB_RECT_CNTS);
	rst->blob_count = rst2.blob_count;
	rst->bIsOKExtraPer = rst2.bIsOKExtraPer;
	rst->bIsOKExtraWidth = rst2.bIsOKExtraWidth;
	memcpy(rst->fRstExtraPer, rst2.fRstExtraPer, sizeof(float) * BLOB_RECT_CNTS);
	memcpy(rst->fRstExtraWidth, rst2.fRstExtraWidth, sizeof(float) * BLOB_RECT_CNTS);
	memcpy(rst->m_rcArrRect, rst2.m_rcArrRect, BLOB_RECT_CNTS * sizeof(RECT));
	rst->bIsOKHeight = rst2.bIsOKHeight;
	memcpy(rst->fRstHeight, rst2.fRstHeight, BLOB_RECT_CNTS * sizeof(float));
	rst->bIsOKWarpage = rst2.bIsOKWarpage;
	rst->bIsOKWarpageX = rst2.bIsOKWarpageX;
	rst->bIsOKWarpageY = rst2.bIsOKWarpageY;
	rst->fRstWarpagedeviation = rst2.fRstWarpagedeviation;
	rst->fRstWarpagedev_X = rst2.fRstWarpagedev_X;
	rst->fRstWarpagedev_Y = rst2.fRstWarpagedev_Y;
	rst->bIsOKScratch = rst2.bIsOKScratch;
	memcpy(rst->fRstScDistance, rst2.fRstScDistance, sizeof(float) * BLOB_RECT_CNTS);
	memcpy(rst->fRstScWidth, rst2.fRstScWidth, sizeof(float) * BLOB_RECT_CNTS);
	rst->bIsOKChipping = rst2.bIsOKChipping;
	memcpy(rst->fRstChippingWid, rst2.fRstChippingWid, sizeof(float) * BLOB_RECT_CNTS);
	memcpy(rst->fRstChippingLen, rst2.fRstChippingLen, sizeof(float) * BLOB_RECT_CNTS);
	rst->m_BodyRect = rst2.m_BodyRect;
	memcpy(rst->OutlineChipping, rst2.OutlineChipping, sizeof(int) * 8);
	memcpy(rst->bIsOKWarningCount, rst2.bIsOKWarningCount, sizeof(bool) * BLOB_RECT_CNTS);
	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoTilt rst2, RstAlgoTilt* rst)
{
	/*if (sizeof(RstAlgoTilt) != sizeof(PIAL::_RstAlgoTilt))
		return false;*/
	memcpy(rst, &rst2, sizeof(PIAL::_RstAlgoTilt));
	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoColor rst, RstAlgoColor* org)
{
	rst.m_bOKPolarity = org->m_bOKPolarity;
	rst.m_bRateOK = org->m_bRateOK;
	rst.m_dRstRate = org->m_dRstRate;
	rst.m_rcRect_I = org->m_rcRect_I;
	// memcpy(org, &rst, sizeof(PIAL::_RstAlgoColor));
	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoBW rst, RstAlgoBlackWhite* org)
{
	rst.m_bNoSolder = org->m_bNoSolder;
	rst.m_bOKBW = org->m_bOKBW;
	rst.m_bOKHeightMean = org->m_bOKHeightMean;
	rst.m_bOKMax = org->m_bOKMax;
	rst.m_dRstHeightMean = org->m_dRstHeightMean;
	rst.m_dRstPercent = org->m_dRstPercent;

	memcpy(rst.m_sInspAC.m_fRstInspAC, org->m_sInspAC.m_fRstInspAC, sizeof(float) * _InspAC_R_Total);
	rst.m_sInspAC.m_nInspAC = org->m_sInspAC.m_nInspAC;
	rst.m_sInspAC.m_nOKInspAC = org->m_sInspAC.m_nOKInspAC;
	memcpy(rst.m_sInspAC.m_rcArrRect, org->m_sInspAC.m_rcArrRect, sizeof(RECT) * _InspAC_R_Rect_Total);

	// memcpy(org, &rst2, sizeof(PIAL::_RstAlgoBW));
	return true;
}
bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoShapeX* rst, RstAlgoShapeX* org)
{
	org->isInsp = rst->isinsp;
	org->isOK = rst->isOK;
	org->nROICnt = rst->nROICnt;
	org->nNGAreaRoiCnt = rst->nNGAreaRoiCnt;
	org->RstWrForeignCnt = rst->RstWrForeignCnt;
	org->RstOKWrForeignCnt = rst->RstOKWrForeignCnt;
	org->nShapeNGCnt = rst->nShapeNGCnt;
	org->StripeRectCnt = rst->StripeRectCnt;

	for (int i = 0; i < _nShapeXCnt; i++)
	{
		org->nMatchingIdx[i] = rst->nMatchingIdx[i];
		org->nMatchingSc[i] = rst->nMatchingSc[i];
		org->ArrROIOK[i] = rst->ArrROIOK[i];
		org->ArrOKShape[i] = rst->ArrOKShape[i];
		org->ArrOKExistShape[i] = rst->ArrOKExistShape[i];
		org->ArrOKInner[i] = rst->ArrOKInner[i];
		org->ArrOKExist[i] = rst->ArrOKExist[i];
		org->ArrOKShift[i] = rst->ArrOKShift[i];
		org->ArrOKForeignW[i] = rst->ArrOKForeignW[i];
		org->ArrOKForeignL[i] = rst->ArrOKForeignL[i];
		org->ArrShapeArea[i] = rst->ArrShapeArea[i];
		org->ArrExistShapeArea[i] = rst->ArrExistShapeArea[i];
		org->ArrInnerArea[i] = rst->ArrInnerArea[i];
		org->ArrExistArea[i] = rst->ArrExistArea[i];
		org->ArrShiftX[i] = rst->ArrShiftX[i];
		org->ArrShiftY[i] = rst->ArrShiftY[i];
		org->ArrForeignW[i] = rst->ArrForeignW[i];
		org->ArrForeignL[i] = rst->ArrForeignL[i];
		org->rcArrRect[i].left = rst->rcArrRect[i].left;
		org->rcArrRect[i].right = rst->rcArrRect[i].right;
		org->rcArrRect[i].top = rst->rcArrRect[i].top;
		org->rcArrRect[i].bottom = rst->rcArrRect[i].bottom;
		org->m_rcArrNGArrRectCnt[i] = rst->m_rcArrNGArrRectCnt[i];
		org->rcArrNGRect[i].left = rst->rcArrNGRect[i].left;
		org->rcArrNGRect[i].right = rst->rcArrNGRect[i].right;
		org->rcArrNGRect[i].top = rst->rcArrNGRect[i].top;
		org->rcArrNGRect[i].bottom = rst->rcArrNGRect[i].bottom;
		org->m_nArrMaxFreqValue1[i] = rst->m_nArrMaxFreqValue1[i];
		org->m_nArrMaxFreqValue2[i] = rst->m_nArrMaxFreqValue2[i];

		//추가변수
		org->ArrOKExistShape[i] = rst->ArrOKExistShape[i];
		org->ArrExistShapeArea[i] = rst->ArrExistShapeArea[i];

		// esjb++ 250408.[23] ~PInslAlgoWrapper.cpp~
		org->ArrLocalLength[i] = rst->ArrLocalLength[i];
		org->ArrDent[i] = rst->ArrDent[i];
		org->ArrDent2[i] = rst->ArrDent2[i];

		org->ArrOKLocalLength[i] = rst->ArrOKLocalLength[i];
		org->ArrOKDent[i] = rst->ArrOKDent[i];
		// ++sjb 250408.[23]

		org->poArrCenter[i] = rst->poArrCenter[i];
		org->bMatchOK[i] = rst->bMatchOK[i];

		org->ArrStartJointP[i] = rst->ArrStartJointP[i];
		org->ArrEndJointP[i] = rst->ArrEndJointP[i];

		org->ArrOKWrForeignW[i] = rst->ArrOKWrForeignW[i];
		org->ArrOKWrForeignL[i] = rst->ArrOKWrForeignL[i];
		org->ArrWrForeignCnt[i] = rst->ArrWrForeignCnt[i];
		org->ArrWrForeignW[i] = rst->ArrWrForeignW[i];
		org->ArrWrForeignL[i] = rst->ArrWrForeignL[i];

		org->rcArrShapeRect[i].left = rst->rcArrShapeRect[i].left;
		org->rcArrShapeRect[i].right = rst->rcArrShapeRect[i].right;
		org->rcArrShapeRect[i].top = rst->rcArrShapeRect[i].top;
		org->rcArrShapeRect[i].bottom = rst->rcArrShapeRect[i].bottom;
		org->m_rcArrShapeNGArrRectCnt[i] = rst->m_rcArrShapeNGArrRectCnt[i];

		org->rcArrStripeRect[i].left = rst->rcArrStripeRect[i].left;
		org->rcArrStripeRect[i].right = rst->rcArrStripeRect[i].right;
		org->rcArrStripeRect[i].top = rst->rcArrStripeRect[i].top;
		org->rcArrStripeRect[i].bottom = rst->rcArrStripeRect[i].bottom;
	}

	return true;
}

void PInspAlgoWrapper::RoiImageCompose_LT(UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR* ptrbyWhiteBuffer,
	int nFullImageSize, int nFullImageWidth, int nFullImageHeight,
	int nROIImageSize, double dROIX, double dROIY, int nROIWidth, int nROIHeight,
	int nRedValue, int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage)
{
	if (m_bFastImageCompose)
	{
		RoiImageCompose_LT_SIMD(ptrbyRedBuffer, ptrbyGreenBuffer, ptrbyBlueBuffer, ptrbyWhiteBuffer,
			nFullImageSize, nFullImageWidth, nFullImageHeight, nROIImageSize, dROIX, dROIY, nROIWidth, nROIHeight, nRedValue, nGreenValue, nBlueValue, nWhiteValue, ptrbyResultImage);
		return;
	}
	float fRedValue = (float)nRedValue / 100.0;
	float fGreenValue = (float)nGreenValue / 100.0;
	float fBlueValue = (float)nBlueValue / 100.0;
	float fWhiteValue = (float)nWhiteValue / 100.0;

	int nValueBuf = 0;
	float fRedImageValue = 0;
	float fGreenImageValue = 0;
	float fBlueImageValue = 0;
	float fWhiteImageValue = 0;

	int nROIIndex = 0;
	int nStartX = 0, nStartY = 0;
	if (nFullImageWidth > nROIWidth)
		nStartX = RounD(dROIX);
	if (nFullImageHeight > nROIHeight)
		nStartY = RounD(dROIY);
	if (nStartX < 0)
		nStartX = 0;
	if (nStartY < 0)
		nStartY = 0;

	int nMaxValue = nFullImageSize / (nFullImageWidth*nFullImageHeight);
	if (nMaxValue <= 0)	nMaxValue = 1;

	nRedValue = (int)(fRedValue * 128.0);
	nGreenValue = (int)(fGreenValue * 128.0);
	nBlueValue = (int)(fBlueValue * 128.0);
	nWhiteValue = (int)(fWhiteValue * 128.0);

	__m128i redBuf = _mm_set_epi16(nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue);
	__m128i greenBuf = _mm_set_epi16(nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue);
	__m128i blueBuf = _mm_set_epi16(nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue);
	__m128i whiteBuf = _mm_set_epi16(nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue);
	__m128i maxBuf = _mm_set_epi16(255, 255, 255, 255, 255, 255, 255, 255);
	__m128i zeroBuf = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 0);


	for (int i = 0; i < nMaxValue; i++)
	{
		for (int y = nStartY; y < nStartY + nROIHeight; y++)
		{
			UCHAR * iptrbyResultImage = (&ptrbyResultImage[(y - nStartY)*nROIWidth]);
			int x = 0;
			for (x = nStartX; (x + 16) < (nStartX + nROIWidth); x += 16)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				register __m128i fRedImageValue;
				register __m128i fGreenImageValue;
				register __m128i fBlueImageValue;
				register __m128i fWhiteImageValue;

				if (!ptrbyRedBuffer)
					fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
				else
					fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyRedBuffer[FImgIdx]);
				if (!ptrbyGreenBuffer)
					fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
				else
					fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyGreenBuffer[FImgIdx]);
				if (!ptrbyBlueBuffer)
					fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
				else
					fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyBlueBuffer[FImgIdx]);
				if (!ptrbyWhiteBuffer)
					fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
				else
					fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyWhiteBuffer[FImgIdx]);

				_mm_storeu_si128(
					(__m128i *)(&iptrbyResultImage[(x - nStartX)])
					,
					_mm_packus_epi16(
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						),
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						)
					)
				);
			}

			for (; x < (nStartX + nROIWidth); x++)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				if (!ptrbyRedBuffer)
					fRedImageValue = 0;
				else
					fRedImageValue = ptrbyRedBuffer[FImgIdx];
				if (!ptrbyGreenBuffer)
					fGreenImageValue = 0;
				else
					fGreenImageValue = ptrbyGreenBuffer[FImgIdx];
				if (!ptrbyBlueBuffer)
					fBlueImageValue = 0;
				else
					fBlueImageValue = ptrbyBlueBuffer[FImgIdx];
				if (!ptrbyWhiteBuffer)
					fWhiteImageValue = 0;
				else
					fWhiteImageValue = ptrbyWhiteBuffer[FImgIdx];

				nValueBuf = (int)((fRedValue * fRedImageValue) + (fGreenValue * fGreenImageValue) + (fBlueValue * fBlueImageValue) + (fWhiteValue * fWhiteImageValue));

				if (nValueBuf > 255)
					nValueBuf = 255;
				iptrbyResultImage[(x - nStartX)] = nValueBuf;
			}
		}
	}
}


void PInspAlgoWrapper::RoiImageCompose_LT(UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR* ptrbyWhiteBuffer,
	int nFullImageSize, int nFullImageWidth, int nFullImageHeight,
	int nROIImageSize, double dROIX, double dROIY, int nROIWidth, int nROIHeight,
	int nRedValue, int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage, double dAngle)
{
	if (dAngle == 0)
	{
		RoiImageCompose_LT(ptrbyRedBuffer, ptrbyGreenBuffer, ptrbyBlueBuffer, ptrbyWhiteBuffer,
			nFullImageSize, nFullImageWidth, nFullImageHeight,
			nROIImageSize, dROIX, dROIY, nROIWidth, nROIHeight,
			nRedValue, nGreenValue, nBlueValue, nWhiteValue, ptrbyResultImage);
		return;
	}
	float fRedValue = (float)nRedValue / 100.0;
	float fGreenValue = (float)nGreenValue / 100.0;
	float fBlueValue = (float)nBlueValue / 100.0;
	float fWhiteValue = (float)nWhiteValue / 100.0;

	int nValueBuf = 0;
	float fRedImageValue = 0;
	float fGreenImageValue = 0;
	float fBlueImageValue = 0;
	float fWhiteImageValue = 0;

	float radian = (float)(dAngle / 180.0f * M_PI);
	int RotateWidth = std::abs(nROIWidth * cos(radian)) + std::abs(nROIHeight * sin(radian));
	int RotateHeight = std::abs(nROIHeight * cos(radian)) + std::abs(nROIWidth * sin(radian));
	int nROIGapX = RotateWidth - nROIWidth;
	int nROIGapY = RotateHeight - nROIHeight;
	nROIWidth = RotateWidth;
	nROIHeight = RotateHeight;
	dROIX -= nROIGapX / 2;
	dROIY -= nROIGapY / 2;

	int nROIIndex = 0;
	int nStartX = 0, nStartY = 0;
	if (nFullImageWidth > nROIWidth)
		nStartX = RounD(dROIX);
	if (nFullImageHeight > nROIHeight)
		nStartY = RounD(dROIY);
	if (nStartX < 0)
		nStartX = 0;
	if (nStartY < 0)
		nStartY = 0;

	UCHAR* ptrbyResultImageRound = g_pMManager->pem_new<UCHAR>(true, RotateWidth*RotateHeight * sizeof(UCHAR), (PCHAR)__FUNCTION__, __LINE__);

	int nMaxValue = nFullImageSize / (nFullImageWidth*nFullImageHeight);
	if (nMaxValue <= 0)	nMaxValue = 1;

	nRedValue = (int)(fRedValue * 128.0);
	nGreenValue = (int)(fGreenValue * 128.0);
	nBlueValue = (int)(fBlueValue * 128.0);
	nWhiteValue = (int)(fWhiteValue * 128.0);

	__m128i redBuf = _mm_set_epi16(nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue);
	__m128i greenBuf = _mm_set_epi16(nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue);
	__m128i blueBuf = _mm_set_epi16(nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue);
	__m128i whiteBuf = _mm_set_epi16(nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue);
	__m128i maxBuf = _mm_set_epi16(255, 255, 255, 255, 255, 255, 255, 255);
	__m128i zeroBuf = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 0);


	for (int i = 0; i < nMaxValue; i++)
	{
		for (int y = nStartY; y < nStartY + nROIHeight; y++)
		{
			UCHAR * iptrbyResultImageRound = (&ptrbyResultImageRound[(y - nStartY)*nROIWidth]);
			int x = 0;
			for (x = nStartX; (x + 16) < (nStartX + nROIWidth); x += 16)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				register __m128i fRedImageValue;
				register __m128i fGreenImageValue;
				register __m128i fBlueImageValue;
				register __m128i fWhiteImageValue;

				if (!ptrbyRedBuffer)
					fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
				else
					fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyRedBuffer[FImgIdx]);
				if (!ptrbyGreenBuffer)
					fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
				else
					fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyGreenBuffer[FImgIdx]);
				if (!ptrbyBlueBuffer)
					fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
				else
					fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyBlueBuffer[FImgIdx]);
				if (!ptrbyWhiteBuffer)
					fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
				else
					fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyWhiteBuffer[FImgIdx]);

				_mm_storeu_si128(
					(__m128i *)(&iptrbyResultImageRound[(x - nStartX)])
					,
					_mm_packus_epi16(
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						),
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						)
					)
				);
			}

			for (; x < (nStartX + nROIWidth); x++)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				if (!ptrbyRedBuffer)
					fRedImageValue = 0;
				else
					fRedImageValue = ptrbyRedBuffer[FImgIdx];
				if (!ptrbyGreenBuffer)
					fGreenImageValue = 0;
				else
					fGreenImageValue = ptrbyGreenBuffer[FImgIdx];
				if (!ptrbyBlueBuffer)
					fBlueImageValue = 0;
				else
					fBlueImageValue = ptrbyBlueBuffer[FImgIdx];
				if (!ptrbyWhiteBuffer)
					fWhiteImageValue = 0;
				else
					fWhiteImageValue = ptrbyWhiteBuffer[FImgIdx];

				nValueBuf = (int)((fRedValue * fRedImageValue) + (fGreenValue * fGreenImageValue) + (fBlueValue * fBlueImageValue) + (fWhiteValue * fWhiteImageValue));

				if (nValueBuf > 255)
					nValueBuf = 255;
				iptrbyResultImageRound[(x - nStartX)] = nValueBuf;
			}
		}
	}


	UCHAR* ptrSrc = ptrbyResultImage;
	UCHAR* ptrbyBuffer_ro = NULL;
	int retDstSizeX(0), retDstSizeY(0);
	int GapX = 0;
	int GapY = 0;
	if (ptrbyResultImage != NULL)
	{
		PIAL::PAlgo::RotateImg_ipp(ptrbyResultImageRound, nROIWidth, nROIHeight, -dAngle, &ptrbyBuffer_ro, &retDstSizeX, &retDstSizeY);
		nROIWidth -= nROIGapX;
		nROIHeight -= nROIGapY;
		GapX = RounD((retDstSizeX - nROIWidth) / 2);
		GapY = RounD((retDstSizeY - nROIHeight) / 2);
		if ((GapX + nROIWidth) > retDstSizeX)
			GapX = 0;
		if ((GapY + nROIHeight) > retDstSizeY)
			GapY = 0;
		for (int r = 0; r < nROIHeight; r++)
		{
			UCHAR* srcPtr = &ptrbyBuffer_ro[(r + GapY)*retDstSizeX + GapX];
			UCHAR* dstPtr = &ptrSrc[r*nROIWidth];
			memcpy(dstPtr, srcPtr, sizeof(UCHAR)*nROIWidth);
		}

		cv::Mat Buffer_ro_Red = cv::Mat(retDstSizeY, retDstSizeX, CV_8UC1, ptrbyBuffer_ro);
		cv::Mat RedBuffer = cv::Mat(nROIHeight, nROIWidth, CV_8UC1, ptrSrc);

		if (ptrbyBuffer_ro != NULL)
			delete [] ptrbyBuffer_ro;
	}
	g_pMManager->pem_delete(ptrbyResultImageRound, true);
}

void PInspAlgoWrapper::ROIImageClaculCompose(const LightTypeBuf &sLightImg, UCHAR* ptrbyResultImage, double dAngle)
{
	if (dAngle == 0)
	{
		ROIImageClaculCompose(sLightImg, ptrbyResultImage);
		return;
	}
	int nFullImageWidth = sLightImg.m_nImgWidth;
	int nFullImageHeight = sLightImg.m_nImgHeight;
	int nROIWidth = sLightImg.m_nROIImgWidth;
	int nROIHeight = sLightImg.m_nROIImgHeight;
	int nImgCnt = sLightImg.m_nImgCnt;
	if ((nFullImageWidth <= 0) || (nFullImageHeight <= 0) || (nImgCnt <= 0) || (nROIWidth <= 0) || (nROIHeight <= 0))
		return;
	double dROIX = sLightImg.m_dROIX;
	double dROIY = sLightImg.m_dROIY;

	UCHAR* ptrbyTopRedBuffer = sLightImg.m_pucTRed;
	UCHAR* ptrbyTopGreenBuffer = sLightImg.m_pucTGreen;
	UCHAR* ptrbyTopBlueBuffer = sLightImg.m_pucTBlue;
	UCHAR* ptrbyTopWhiteBuffer = sLightImg.m_pucTWhite;

	UCHAR* ptrbyMidRedBuffer = sLightImg.m_pucMRed;
	UCHAR* ptrbyMidGreenBuffer = sLightImg.m_pucMGreen;
	UCHAR* ptrbyMidBlueBuffer = sLightImg.m_pucMBlue;
	UCHAR* ptrbyMidWhiteBuffer = sLightImg.m_pucMWhite;

	UCHAR* ptrbyBotRedBuffer = sLightImg.m_pucBRed;
	UCHAR* ptrbyBotGreenBuffer = sLightImg.m_pucBGreen;
	UCHAR* ptrbyBotBlueBuffer = sLightImg.m_pucBBlue;
	UCHAR* ptrbyBotWhiteBuffer = sLightImg.m_pucBWhite;

	int* pnRedValue = sLightImg.m_pnRedValue;
	int* pnGreenValue = sLightImg.m_pnGreenValue;
	int* pnBlueValue = sLightImg.m_pnBlueValue;
	int* pnWhiteValue = sLightImg.m_pnWhiteValue;

	int *ptrnPosition = sLightImg.m_pnPosition;
	int *nCalculation = sLightImg.m_pnCalculation;
	if ((nCalculation == NULL) || (ptrbyResultImage == NULL))
		return;

	int nValueBuf = 0;
	float fRedImageValue = 0.0;
	float fGreenImageValue = 0.0;
	float fBlueImageValue = 0.0;
	float fWhiteImageValue = 0.0;

	float fRedValue = 0.0;
	float fGreenValue = 0.0;
	float fBlueValue = 0.0;
	float fWhiteValue = 0.0;

	//shkim rotate Rect sz 예외처리
	float radian = (float)(dAngle / 180.0f * M_PI);
	int RotateWidth = std::abs(nROIWidth * cos(radian)) + std::abs(nROIHeight * sin(radian));
	int RotateHeight = std::abs(nROIHeight * cos(radian)) + std::abs(nROIWidth * sin(radian));
	int nROIGapX = RotateWidth - nROIWidth;
	int nROIGapY = RotateHeight - nROIHeight;
	nROIWidth = RotateWidth;
	nROIHeight = RotateHeight;
	dROIX -= nROIGapX / 2;
	dROIY -= nROIGapY / 2;

	int nROIIndex = 0;
	int nStartX = 0, nStartY = 0;
	if (nFullImageWidth > nROIWidth)
		nStartX = RounD(dROIX);
	if (nFullImageHeight > nROIHeight)
		nStartY = RounD(dROIY);
	if (nStartX < 0)
		nStartX = 0;
	if (nStartY < 0)
		nStartY = 0;

	UCHAR* ptrbyResultImageRound = g_pMManager->pem_new<UCHAR>(true, RotateWidth*RotateHeight * sizeof(UCHAR), (PCHAR)__FUNCTION__, __LINE__);

	int nImageSize = nROIWidth * nROIHeight;
	UCHAR** ppucComposeImageBuf = NULL;
	Make_2DArray(&ppucComposeImageBuf, nImgCnt, nImageSize);

	__m128i maxBuf = _mm_set_epi16(255, 255, 255, 255, 255, 255, 255, 255);
	__m128i zeroBuf = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 0);

	for (int a = 0; a < nImgCnt; a++)
	{
		nROIIndex = 0;
		nValueBuf = 0;

		fRedValue = (double)pnRedValue[a] / 100.0;
		fGreenValue = (double)pnGreenValue[a] / 100.0;
		fBlueValue = (double)pnBlueValue[a] / 100.0;
		fWhiteValue = (double)pnWhiteValue[a] / 100.0;

		int nRedValue = (int)(fRedValue * 128.0);
		int nGreenValue = (int)(fGreenValue * 128.0);
		int nBlueValue = (int)(fBlueValue * 128.0);
		int nWhiteValue = (int)(fWhiteValue * 128.0);

		__m128i redBuf = _mm_set_epi16(nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue);
		__m128i greenBuf = _mm_set_epi16(nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue);
		__m128i blueBuf = _mm_set_epi16(nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue);
		__m128i whiteBuf = _mm_set_epi16(nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue);

		for (int y = nStartY; y < nStartY + nROIHeight; y++)
		{
			UCHAR * ippucComposeImageBuf = (UCHAR *)(&ppucComposeImageBuf[a][(y - nStartY)*nROIWidth]);
			int x;
			for (x = nStartX; (x + 16) < (nStartX + nROIWidth); x += 16)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				register __m128i fRedImageValue = zeroBuf;
				register __m128i fGreenImageValue = zeroBuf;
				register __m128i fBlueImageValue = zeroBuf;
				register __m128i fWhiteImageValue = zeroBuf;

				if (ptrnPosition[a] == 0)
				{
					if (!ptrbyTopRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopRedBuffer[FImgIdx]);
					if (!ptrbyTopGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopGreenBuffer[FImgIdx]);
					if (!ptrbyTopBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopBlueBuffer[FImgIdx]);
					if (!ptrbyTopWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopWhiteBuffer[FImgIdx]);
				}
				else if (ptrnPosition[a] == 1)
				{
					if (!ptrbyMidRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidRedBuffer[FImgIdx]);
					if (!ptrbyMidGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidGreenBuffer[FImgIdx]);
					if (!ptrbyMidBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidBlueBuffer[FImgIdx]);
					if (!ptrbyMidWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidWhiteBuffer[FImgIdx]);
				}
				else if (ptrnPosition[a] == 2)
				{
					if (!ptrbyBotRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotRedBuffer[FImgIdx]);
					if (!ptrbyBotGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotGreenBuffer[FImgIdx]);
					if (!ptrbyBotBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotBlueBuffer[FImgIdx]);
					if (!ptrbyBotWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotWhiteBuffer[FImgIdx]);
				}
				else
				{
					if (ppucComposeImageBuf)
					{
						Delete_2DArray(&ppucComposeImageBuf, nImgCnt, nImageSize);
					}
					return;
				}

				_mm_storeu_si128(
					(__m128i *)(&ippucComposeImageBuf[(x - nStartX)])
					,
					_mm_packus_epi16(
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						),
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						)
					)
				);
			}

			for (; x < (nStartX + nROIWidth); x++)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				if (ptrnPosition[a] == 0)
				{
					if (!ptrbyTopRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyTopRedBuffer[FImgIdx];
					if (!ptrbyTopGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyTopGreenBuffer[FImgIdx];
					if (!ptrbyTopBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyTopBlueBuffer[FImgIdx];
					if (!ptrbyTopWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyTopWhiteBuffer[FImgIdx];
				}
				else if (ptrnPosition[a] == 1)
				{
					if (!ptrbyMidRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyMidRedBuffer[FImgIdx];
					if (!ptrbyMidGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyMidGreenBuffer[FImgIdx];
					if (!ptrbyMidBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyMidBlueBuffer[FImgIdx];
					if (!ptrbyMidWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyMidWhiteBuffer[FImgIdx];
				}
				else if (ptrnPosition[a] == 2)
				{
					if (!ptrbyBotRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyBotRedBuffer[FImgIdx];
					if (!ptrbyBotGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyBotGreenBuffer[FImgIdx];
					if (!ptrbyBotBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyBotBlueBuffer[FImgIdx];
					if (!ptrbyBotWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyBotWhiteBuffer[FImgIdx];
				}
				else
				{
					if (ppucComposeImageBuf)
					{
						Delete_2DArray(&ppucComposeImageBuf, nImgCnt, nImageSize);
					}
					return;
				}

				nValueBuf = (int)((fRedValue * fRedImageValue) + (fGreenValue * fGreenImageValue) + (fBlueValue * fBlueImageValue) + (fWhiteValue * fWhiteImageValue));

				if (nValueBuf > 255)
					nValueBuf = 255;
				if (nValueBuf < 0)
					nValueBuf = 0;
				ippucComposeImageBuf[x - nStartX] = nValueBuf;
			}
		}
	}

	UCHAR *pnImageValue = NULL;;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &pnImageValue, nImageSize);
	int nImgValue = 0;
	for (int a = 0; a < nImgCnt; a++)
	{
		for (int b = 0; b < nImageSize; b++)
		{
			if (a == 0)
				nImgValue = ppucComposeImageBuf[a][b];
			else
			{
				if (nCalculation[a - 1] == 1)
					nImgValue = pnImageValue[b] + ppucComposeImageBuf[a][b];
				else if (nCalculation[a - 1] == 2)
					nImgValue = pnImageValue[b] - ppucComposeImageBuf[a][b];
			}

			if (nImgValue > 255)
				nImgValue = 255;
			if (nImgValue < 0)
				nImgValue = 0;

			pnImageValue[b] = nImgValue;
		}
	}
	for (int i = 0; i < nImageSize; i++)
	{
		ptrbyResultImageRound[i] = pnImageValue[i];
	}

	if (pnImageValue)
	{
		Delete_1DArray(&pnImageValue);
	}
	if (ppucComposeImageBuf)
	{
		Delete_2DArray(&ppucComposeImageBuf, nImgCnt, nImageSize);
	}

	//shkim rotate
	UCHAR* ptrSrc = ptrbyResultImage;
	UCHAR* ptrbyBuffer_ro = NULL;
	int retDstSizeX(0), retDstSizeY(0);
	int GapX = 0;
	int GapY = 0;
	if (ptrbyResultImage != NULL)
	{
		PIAL::PAlgo::RotateImg_ipp(ptrbyResultImageRound, nROIWidth, nROIHeight, -dAngle, &ptrbyBuffer_ro, &retDstSizeX, &retDstSizeY);

		nROIWidth -= nROIGapX;
		nROIHeight -= nROIGapY;
		GapX = RounD((retDstSizeX - nROIWidth) / 2);
		GapY = RounD((retDstSizeY - nROIHeight) / 2);
		if ((GapX + nROIWidth) > retDstSizeX)
			GapX = 0;
		if ((GapY + nROIHeight) > retDstSizeY)
			GapY = 0;
		for (int r = 0; r < nROIHeight; r++)
		{
			UCHAR* srcPtr = &ptrbyBuffer_ro[(r + GapY)*retDstSizeX + GapX];
			UCHAR* dstPtr = &ptrSrc[r*nROIWidth];
			memcpy(dstPtr, srcPtr, sizeof(UCHAR)*nROIWidth);
		}

		if (ptrbyBuffer_ro != NULL)
			delete [] ptrbyBuffer_ro;
	}
	g_pMManager->pem_delete(ptrbyResultImageRound, true);
}

void PInspAlgoWrapper::ROIImageClaculCompose(const LightTypeBuf &sLightImg, UCHAR* ptrbyResultImage)
{
	if (m_bFastImageCompose)
	{
		ROIImageClaculCompose_SIMD(sLightImg, ptrbyResultImage);
		return;
	}

	int nFullImageWidth = sLightImg.m_nImgWidth;
	int nFullImageHeight = sLightImg.m_nImgHeight;
	int nROIWidth = sLightImg.m_nROIImgWidth;
	int nROIHeight = sLightImg.m_nROIImgHeight;
	int nImgCnt = sLightImg.m_nImgCnt;
	if ((nFullImageWidth <= 0) || (nFullImageHeight <= 0) || (nImgCnt <= 0) || (nROIWidth <= 0) || (nROIHeight <= 0))
		return;
	double dROIX = sLightImg.m_dROIX;
	double dROIY = sLightImg.m_dROIY;

	UCHAR* ptrbyTopRedBuffer = sLightImg.m_pucTRed;
	UCHAR* ptrbyTopGreenBuffer = sLightImg.m_pucTGreen;
	UCHAR* ptrbyTopBlueBuffer = sLightImg.m_pucTBlue;
	UCHAR* ptrbyTopWhiteBuffer = sLightImg.m_pucTWhite;

	UCHAR* ptrbyMidRedBuffer = sLightImg.m_pucMRed;
	UCHAR* ptrbyMidGreenBuffer = sLightImg.m_pucMGreen;
	UCHAR* ptrbyMidBlueBuffer = sLightImg.m_pucMBlue;
	UCHAR* ptrbyMidWhiteBuffer = sLightImg.m_pucMWhite;

	UCHAR* ptrbyBotRedBuffer = sLightImg.m_pucBRed;
	UCHAR* ptrbyBotGreenBuffer = sLightImg.m_pucBGreen;
	UCHAR* ptrbyBotBlueBuffer = sLightImg.m_pucBBlue;
	UCHAR* ptrbyBotWhiteBuffer = sLightImg.m_pucBWhite;

	int* pnRedValue = sLightImg.m_pnRedValue;
	int* pnGreenValue = sLightImg.m_pnGreenValue;
	int* pnBlueValue = sLightImg.m_pnBlueValue;
	int* pnWhiteValue = sLightImg.m_pnWhiteValue;

	int *ptrnPosition = sLightImg.m_pnPosition;
	int *nCalculation = sLightImg.m_pnCalculation;
	if ((nCalculation == NULL) || (ptrbyResultImage == NULL))
		return;

	int nValueBuf = 0;
	float fRedImageValue = 0.0;
	float fGreenImageValue = 0.0;
	float fBlueImageValue = 0.0;
	float fWhiteImageValue = 0.0;

	float fRedValue = 0.0;
	float fGreenValue = 0.0;
	float fBlueValue = 0.0;
	float fWhiteValue = 0.0;

	int nROIIndex = 0;
	int nStartX = 0, nStartY = 0;
	if (nFullImageWidth > nROIWidth)
		nStartX = RounD(dROIX);
	if (nFullImageHeight > nROIHeight)
		nStartY = RounD(dROIY);
	if (nStartX < 0)
		nStartX = 0;
	if (nStartY < 0)
		nStartY = 0;

	int nImageSize = nROIWidth * nROIHeight;
	UCHAR** ppucComposeImageBuf = NULL;
	Make_2DArray(&ppucComposeImageBuf, nImgCnt, nImageSize);

	__m128i maxBuf = _mm_set_epi16(255, 255, 255, 255, 255, 255, 255, 255);
	__m128i zeroBuf = _mm_set_epi16(0, 0, 0, 0, 0, 0, 0, 0);

	for (int a = 0; a < nImgCnt; a++)
	{
		nROIIndex = 0;
		nValueBuf = 0;

		fRedValue = (double)pnRedValue[a] / 100.0;
		fGreenValue = (double)pnGreenValue[a] / 100.0;
		fBlueValue = (double)pnBlueValue[a] / 100.0;
		fWhiteValue = (double)pnWhiteValue[a] / 100.0;

		int nRedValue = (int)(fRedValue * 128.0);
		int nGreenValue = (int)(fGreenValue * 128.0);
		int nBlueValue = (int)(fBlueValue * 128.0);
		int nWhiteValue = (int)(fWhiteValue * 128.0);

		__m128i redBuf = _mm_set_epi16(nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue, nRedValue);
		__m128i greenBuf = _mm_set_epi16(nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue, nGreenValue);
		__m128i blueBuf = _mm_set_epi16(nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue, nBlueValue);
		__m128i whiteBuf = _mm_set_epi16(nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue, nWhiteValue);

		for (int y = nStartY; y < nStartY + nROIHeight; y++)
		{
			UCHAR * ippucComposeImageBuf = (UCHAR *)(&ppucComposeImageBuf[a][(y - nStartY)*nROIWidth]);
			int x;
			for (x = nStartX; (x + 16) < (nStartX + nROIWidth); x += 16)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				register __m128i fRedImageValue = zeroBuf;
				register __m128i fGreenImageValue = zeroBuf;
				register __m128i fBlueImageValue = zeroBuf;
				register __m128i fWhiteImageValue = zeroBuf;

				if (ptrnPosition[a] == 0)
				{
					if (!ptrbyTopRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopRedBuffer[FImgIdx]);
					if (!ptrbyTopGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopGreenBuffer[FImgIdx]);
					if (!ptrbyTopBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopBlueBuffer[FImgIdx]);
					if (!ptrbyTopWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyTopWhiteBuffer[FImgIdx]);
				}
				else if (ptrnPosition[a] == 1)
				{
					if (!ptrbyMidRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidRedBuffer[FImgIdx]);
					if (!ptrbyMidGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidGreenBuffer[FImgIdx]);
					if (!ptrbyMidBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidBlueBuffer[FImgIdx]);
					if (!ptrbyMidWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyMidWhiteBuffer[FImgIdx]);
				}
				else if (ptrnPosition[a] == 2)
				{
					if (!ptrbyBotRedBuffer)
						fRedImageValue = zeroBuf;//_mm_xor_si128(fRedImageValue, fRedImageValue);
					else
						fRedImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotRedBuffer[FImgIdx]);
					if (!ptrbyBotGreenBuffer)
						fGreenImageValue = zeroBuf;//_mm_xor_si128(fGreenImageValue, fGreenImageValue);
					else
						fGreenImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotGreenBuffer[FImgIdx]);
					if (!ptrbyBotBlueBuffer)
						fBlueImageValue = zeroBuf;//_mm_xor_si128(fBlueImageValue, fBlueImageValue);
					else
						fBlueImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotBlueBuffer[FImgIdx]);
					if (!ptrbyBotWhiteBuffer)
						fWhiteImageValue = zeroBuf;//_mm_xor_si128(fWhiteImageValue, fWhiteImageValue);
					else
						fWhiteImageValue = _mm_loadu_si128((__m128i *)&ptrbyBotWhiteBuffer[FImgIdx]);
				}
				else
				{
					if (ppucComposeImageBuf)
					{
						Delete_2DArray(&ppucComposeImageBuf, nImgCnt, nImageSize);
					}
					return;
				}

				_mm_storeu_si128(
					(__m128i *)(&ippucComposeImageBuf[(x - nStartX)])
					,
					_mm_packus_epi16(
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpacklo_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						),
						_mm_min_epu16(
							_mm_srli_epi16(
								_mm_adds_epu16(
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fRedImageValue, zeroBuf), redBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fGreenImageValue, zeroBuf), greenBuf), 4)
									),
									_mm_adds_epu16(
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fBlueImageValue, zeroBuf), blueBuf), 4),
										_mm_srli_epi16(_mm_mullo_epi16(_mm_unpackhi_epi8(fWhiteImageValue, zeroBuf), whiteBuf), 4)
									)
								),
								3),
							maxBuf
						)
					)
				);
			}

			for (; x < (nStartX + nROIWidth); x++)
			{
				if (y >= nFullImageHeight || x >= nFullImageWidth)
					continue;

				int FImgIdx = y * nFullImageWidth + x;

				if (ptrnPosition[a] == 0)
				{
					if (!ptrbyTopRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyTopRedBuffer[FImgIdx];
					if (!ptrbyTopGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyTopGreenBuffer[FImgIdx];
					if (!ptrbyTopBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyTopBlueBuffer[FImgIdx];
					if (!ptrbyTopWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyTopWhiteBuffer[FImgIdx];
				}
				else if (ptrnPosition[a] == 1)
				{
					if (!ptrbyMidRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyMidRedBuffer[FImgIdx];
					if (!ptrbyMidGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyMidGreenBuffer[FImgIdx];
					if (!ptrbyMidBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyMidBlueBuffer[FImgIdx];
					if (!ptrbyMidWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyMidWhiteBuffer[FImgIdx];
				}
				else if (ptrnPosition[a] == 2)
				{
					if (!ptrbyBotRedBuffer)
						fRedImageValue = 0;
					else
						fRedImageValue = ptrbyBotRedBuffer[FImgIdx];
					if (!ptrbyBotGreenBuffer)
						fGreenImageValue = 0;
					else
						fGreenImageValue = ptrbyBotGreenBuffer[FImgIdx];
					if (!ptrbyBotBlueBuffer)
						fBlueImageValue = 0;
					else
						fBlueImageValue = ptrbyBotBlueBuffer[FImgIdx];
					if (!ptrbyBotWhiteBuffer)
						fWhiteImageValue = 0;
					else
						fWhiteImageValue = ptrbyBotWhiteBuffer[FImgIdx];
				}
				else
				{
					if (ppucComposeImageBuf)
					{
						Delete_2DArray(&ppucComposeImageBuf, nImgCnt, nImageSize);
					}
					return;
				}

				nValueBuf = (int)((fRedValue * fRedImageValue) + (fGreenValue * fGreenImageValue) + (fBlueValue * fBlueImageValue) + (fWhiteValue * fWhiteImageValue));

				if (nValueBuf > 255)
					nValueBuf = 255;
				if (nValueBuf < 0)
					nValueBuf = 0;
				ippucComposeImageBuf[x - nStartX] = nValueBuf;
			}

		}
		//	m_pProcMilAlgo->SaveWorkImg(ppucComposeImageBuf[a], nROIWidth, nROIHeight, _T("ComposeImageBuf.bmp"));
	}

	cv::Mat dstImg(nROIHeight, nROIWidth, CV_8UC1);
	int nImgValue = 0;
	for (int a = 0; a < nImgCnt; a++)
	{
		cv::Mat srcImg(nROIHeight, nROIWidth, CV_8UC1, ppucComposeImageBuf[a]);
		if (a == 0)
			srcImg.copyTo(dstImg);
		else
		{
			if (nCalculation[a - 1] == 1)
				cv::add(dstImg, srcImg, dstImg);
			else if (nCalculation[a - 1] == 2)
				cv::subtract(dstImg, srcImg, dstImg);
		}
	}

	cv::Mat resImg(nROIHeight, nROIWidth, CV_8UC1, ptrbyResultImage);
	dstImg.copyTo(resImg);

	if (ppucComposeImageBuf)
	{
		Delete_2DArray(&ppucComposeImageBuf, nImgCnt, nImageSize);
	}
}

bool PInspAlgoWrapper::NormalImageCompose(InspPartInfo* InspPartInfo, InspAlgoType nAlgoType, InspAlgo& sInspAlgo, PIAL::Insp_Image* pImg_buf, PIAL::PI_Buff* rstBuf, int width, int height, int wnd_w, int wnd_h, double dX, double dY, int nSideCameraIndex, AlignResult* ptrAlignRes, int nAlignCnt)
{
	// Basler Color 
	// CI - Basler ColorCam 사용에 대한 부분은 일단 제외됨.

	int nCameraType = PIAL::PInspAlgo_Lib::m_CameraType;
	UCHAR * ImgR = NULL, *ImgG = NULL, *ImgB = NULL, *ImgW = NULL;
	ImgW = InspPartInfo->partImgBuf.imgTop_W;
	LightTypeBuf sLightBuf;
	InspLightType lightType = sInspAlgo.m_eLightType;

	int nRedValue = sInspAlgo.m_nRedValue;
	int nGreenValue = sInspAlgo.m_nGreenValue;
	int nBlueValue = sInspAlgo.m_nBlueValue;
	int nWhiteValue = sInspAlgo.m_nWhiteValue;
	if (lightType == User_Light)
	{
		if (nCameraType == Basler_Color)
		{
			sLightBuf.m_pucTRed = InspPartInfo->partImgBuf.imgTop_W;
			sLightBuf.m_pucTGreen = InspPartInfo->partImgBuf.imgMiddle_R;
			sLightBuf.m_pucTBlue = InspPartInfo->partImgBuf.imgMiddle_B;
			sLightBuf.m_pucTWhite = NULL;
			sLightBuf.m_pucMRed = NULL;
			sLightBuf.m_pucMGreen = NULL;
			sLightBuf.m_pucMBlue = NULL;
			sLightBuf.m_pucMWhite = NULL;
			sLightBuf.m_pucBRed = InspPartInfo->partImgBuf.imgTop_R;
			sLightBuf.m_pucBGreen = NULL;
			sLightBuf.m_pucBBlue = InspPartInfo->partImgBuf.imgTop_G;
			sLightBuf.m_pucBWhite = NULL;
		}
		else
		{
			sLightBuf.m_pucTRed = InspPartInfo->partImgBuf.imgTop_R;
			sLightBuf.m_pucTGreen = InspPartInfo->partImgBuf.imgTop_G;
			sLightBuf.m_pucTBlue = InspPartInfo->partImgBuf.imgTop_B;
			sLightBuf.m_pucTWhite = InspPartInfo->partImgBuf.imgTop_W;

			sLightBuf.m_pucMRed = InspPartInfo->partImgBuf.imgMiddle_R;
			sLightBuf.m_pucMGreen = NULL;
			sLightBuf.m_pucMBlue = InspPartInfo->partImgBuf.imgMiddle_B;
			sLightBuf.m_pucMWhite = NULL;

			sLightBuf.m_pucBRed = InspPartInfo->partImgBuf.imgBottom_R;
			sLightBuf.m_pucBGreen = NULL;
			sLightBuf.m_pucBBlue = InspPartInfo->partImgBuf.imgBottom_B;
			sLightBuf.m_pucBWhite = NULL;
		}

		sLightBuf.m_nImgWidth = width;
		sLightBuf.m_nImgHeight = height;
		sLightBuf.m_nROIImgWidth = wnd_w;
		sLightBuf.m_nROIImgHeight = wnd_h;
		sLightBuf.m_dROIX = dX;
		sLightBuf.m_dROIY = dY;
		sLightBuf.m_nImgCnt = sInspAlgo.m_nLightCnt;
		sLightBuf.m_pnRedValue = sInspAlgo.m_nArrRedValue;
		sLightBuf.m_pnGreenValue = sInspAlgo.m_nArrGreenValue;
		sLightBuf.m_pnBlueValue = sInspAlgo.m_nArrBlueValue;
		sLightBuf.m_pnWhiteValue = sInspAlgo.m_nArrWhiteValue;
		sLightBuf.m_pnPosition = sInspAlgo.m_nArrLightPosition;
		sLightBuf.m_pnCalculation = sInspAlgo.m_nArrCalculation;

		ROIImageClaculCompose(sLightBuf, rstBuf->m_pData);
	}
	else
	{
		switch (lightType)
		{
		case Top_Light:
		{
			ImgR = InspPartInfo->partImgBuf.imgTop_R;
			ImgG = InspPartInfo->partImgBuf.imgTop_G;
			ImgB = InspPartInfo->partImgBuf.imgTop_B;
			break;
		}
		case Middle_Light:
		{
			ImgR = InspPartInfo->partImgBuf.imgMiddle_R;
			ImgB = InspPartInfo->partImgBuf.imgMiddle_B;
			break;
		}
		case Bottom_Light:
		{
			if (PIAL::PInspAlgo_Lib::m_CameraType == Basler_Color)
			{
				ImgR = InspPartInfo->partImgBuf.imgTop_R;
				ImgB = InspPartInfo->partImgBuf.imgTop_G;
			}
			else
			{
				ImgR = InspPartInfo->partImgBuf.imgBottom_R;
				ImgB = InspPartInfo->partImgBuf.imgBottom_B;
			}
			break;
		}
		case Side1_Light:
		case Side2_Light:
		case Side3_Light:
		case Side4_Light:
		{
			if (nSideCameraIndex == eMSCN_SIDECAM1)
			{
				ImgR = InspPartInfo->partImgBuf.imgSide1_R;
				ImgG = InspPartInfo->partImgBuf.imgSide1_G;
				ImgB = InspPartInfo->partImgBuf.imgSide1_B;
			}
			else if (nSideCameraIndex == eMSCN_SIDECAM2)
			{
				ImgR = InspPartInfo->partImgBuf.imgSide2_R;
				ImgG = InspPartInfo->partImgBuf.imgSide2_G;
				ImgB = InspPartInfo->partImgBuf.imgSide2_B;
			}
			else if (nSideCameraIndex == eMSCN_SIDECAM3)
			{
				ImgR = InspPartInfo->partImgBuf.imgSide3_R;
				ImgG = InspPartInfo->partImgBuf.imgSide3_G;
				ImgB = InspPartInfo->partImgBuf.imgSide3_B;
			}
			else if (nSideCameraIndex == eMSCN_SIDECAM4)
			{
				ImgR = InspPartInfo->partImgBuf.imgSide4_R;
				ImgG = InspPartInfo->partImgBuf.imgSide4_G;
				ImgB = InspPartInfo->partImgBuf.imgSide4_B;
			}
			else
			{
				ImgR = NULL;
				ImgG = NULL;
				ImgB = NULL;
			}
			break;
		}
		}

		bool bUserLight = false;
		double dAlignAngle = 0;
		if (ptrAlignRes != NULL && (sInspAlgo.m_eAlgoType == eAlgoBlob || sInspAlgo.m_eAlgoType == eAlgoPOCR || sInspAlgo.m_eAlgoType == eAlgoPattern))
		{
			for (int nAlIdx = 0; nAlIdx < nAlignCnt; nAlIdx++)
				dAlignAngle = ptrAlignRes[nAlIdx].theta;
		}
			

		if (!WindowRotateState())
			dAlignAngle = 0;

		if (lightType == Top_Light || lightType == Middle_Light || lightType == Bottom_Light)
		{
			RoiImageCompose_LT(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w * wnd_h, dX, dY, wnd_w, wnd_h, nRedValue, nGreenValue, nBlueValue, nWhiteValue, rstBuf->m_pData, dAlignAngle);
		}
		else if (lightType == Side1_Light || lightType == Side2_Light || lightType == Side3_Light || lightType == Side4_Light)
		{
			nWhiteValue = 0;
			RoiImageCompose_LT(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w * wnd_h, dX, dY, wnd_w, wnd_h, nRedValue, nGreenValue, nBlueValue, nWhiteValue, rstBuf->m_pData, dAlignAngle);
		}
		else
		{
			ROIImageClaculCompose(sLightBuf, rstBuf->m_pData, dAlignAngle);
		}
	}

	return true;
}

void PInspAlgoWrapper::ColorDataInput(InspAlgo sInspAlgo, PIAL::Insp_Image* pImg_buf, int nWidth, int nHeight, RECT rtClip, byte byIDX, int nSideCameraIndex)
{
	int nType = 0;
	InspAlgoType algoType = sInspAlgo.m_eAlgoType;
	if (algoType == eAlgoBW || algoType == eAlgoBlob || algoType == eAlgoLine || algoType == eAlgoEdge
		|| algoType == eAlgoColor || algoType == eAlgoLead_Color || algoType == eAlgoLead_Solder
		|| algoType == eAlgoGray_Mean || algoType == eAlgoHeight_Mean || algoType == eAlgoNGBlob)
		nType = 1;
	else if (algoType == eAlgoPadAlign || algoType == eAlgoAlign || algoType == eAlgoAlignEdge || algoType == eAlgoBody_Blob)
		nType = 2;
	else
		return;

	BOOL bUseAngleColor = FALSE;
	BOOL bAIData = FALSE;
	bool byColorLightType = 0;
	if (sInspAlgo.m_eAlgoType == eAlgoBW)
	{
		AlgoBW *pInspAlgo = (AlgoBW *)sInspAlgo.m_ptrInspAlgoParam;
		if (pInspAlgo->m_sAlgoColorBase.m_bUseColor == FALSE &&
			(pInspAlgo->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data] & eDefaultAC_Data_Use) != eDefaultAC_Data_Use)
			return;
		bUseAngleColor = pInspAlgo->m_sAlgoColorBase.m_bUseAngleColor ||
			((pInspAlgo->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data] & eDefaultAC_Data_Use) == eDefaultAC_Data_Use);
		byColorLightType = pInspAlgo->m_sAlgoColorBase.m_byColorLightType;
	}
	else if (sInspAlgo.m_eAlgoType == eAlgoBlob)
	{
		AlgoBlob *pInspAlgo = (AlgoBlob *)sInspAlgo.m_ptrInspAlgoParam;
		bAIData = pInspAlgo->m_bInspCoil;
		if (pInspAlgo->m_sAlgoColorBase.m_bUseColor == FALSE &&
			(pInspAlgo->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data] & eDefaultAC_Data_Use) != eDefaultAC_Data_Use)
			return;
		bUseAngleColor = pInspAlgo->m_sAlgoColorBase.m_bUseAngleColor ||
			((pInspAlgo->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data] & eDefaultAC_Data_Use) == eDefaultAC_Data_Use);
		byColorLightType = pInspAlgo->m_sAlgoColorBase.m_byColorLightType;
	}
	else if (sInspAlgo.m_eAlgoType == eAlgoLine)
	{
		AlgoLine *pInspAlgo = (AlgoLine *)sInspAlgo.m_ptrInspAlgoParam;
		if (pInspAlgo->m_sAlgoColorBase.m_bUseColor == FALSE)
			return;
		bUseAngleColor = pInspAlgo->m_sAlgoColorBase.m_bUseAngleColor;
		byColorLightType = pInspAlgo->m_sAlgoColorBase.m_byColorLightType;
	}
	else if (sInspAlgo.m_eAlgoType == eAlgoEdge)
	{
		AlgoEdge *pInspAlgo = (AlgoEdge *)sInspAlgo.m_ptrInspAlgoParam;
		if (pInspAlgo->m_sAlgoColorBase.m_bUseColor == FALSE)
			return;
		bUseAngleColor = pInspAlgo->m_sAlgoColorBase.m_bUseAngleColor;
		byColorLightType = pInspAlgo->m_sAlgoColorBase.m_byColorLightType;
	}
	else if (sInspAlgo.m_eAlgoType == eAlgoPadAlign)
	{
		AlgoPadAlign *pInspAlgo = (AlgoPadAlign *)sInspAlgo.m_ptrInspAlgoParam;
		if (pInspAlgo->m_sAlgoBW.m_sAlgoColorBase.m_bUseColor == FALSE)
			return;
		bUseAngleColor = pInspAlgo->m_sAlgoBW.m_sAlgoColorBase.m_bUseAngleColor;
		byColorLightType = pInspAlgo->m_sAlgoBW.m_sAlgoColorBase.m_byColorLightType;
	}
	else if (sInspAlgo.m_eAlgoType == eAlgoAlign)
	{
		AlgoAlign *pInspAlgo = (AlgoAlign *)sInspAlgo.m_ptrInspAlgoParam;
		if (pInspAlgo->m_sAlgoColorBase.m_bUseColor == FALSE)
			return;
		bUseAngleColor = pInspAlgo->m_sAlgoColorBase.m_bUseAngleColor;
		byColorLightType = pInspAlgo->m_sAlgoColorBase.m_byColorLightType;
	}
	else if (sInspAlgo.m_eAlgoType == eAlgoAlignEdge)
	{
		AlgoAlignEdge *pInspAlgo = (AlgoAlignEdge *)sInspAlgo.m_ptrInspAlgoParam;
		if (pInspAlgo->sArrAlgoEdge[byIDX].m_sAlgoColorBase.m_bUseColor == FALSE)
			return;
		bUseAngleColor = pInspAlgo->sArrAlgoEdge[byIDX].m_sAlgoColorBase.m_bUseAngleColor;
		byColorLightType = pInspAlgo->sArrAlgoEdge[byIDX].m_sAlgoColorBase.m_byColorLightType;
	}
	else if (sInspAlgo.m_eAlgoType == eAlgoBody_Blob)
	{
		AlgoBodyBlob *pInspAlgo = (AlgoBodyBlob *)sInspAlgo.m_ptrInspAlgoParam;
		if (pInspAlgo->m_sAlgoColorBase.m_bUseColor == FALSE)
			return;
		bUseAngleColor = pInspAlgo->m_sAlgoColorBase.m_bUseAngleColor;
		byColorLightType = pInspAlgo->m_sAlgoColorBase.m_byColorLightType;
	}
	else if (sInspAlgo.m_eAlgoType == eAlgoColor || sInspAlgo.m_eAlgoType == eAlgoLead_Color)
	{
		AlgoColor *pInspAlgo = (AlgoColor *)sInspAlgo.m_ptrInspAlgoParam;
		bUseAngleColor = pInspAlgo->m_bUseColorMap2;
		byColorLightType = pInspAlgo->m_byColorLightType;
	}
	else if (sInspAlgo.m_eAlgoType == eAlgoLead_Solder)
	{
		AlgoLeadSolder *pInspAlgo = (AlgoLeadSolder *)sInspAlgo.m_ptrInspAlgoParam;
		if ((pInspAlgo->m_sAngleColorBase.m_nArrInspAC[m_eInspAC_T3_Data] & eDefaultAC_Data_Use) != eDefaultAC_Data_Use)
			return;
		bUseAngleColor = true;
	}
	else if (sInspAlgo.m_eAlgoType == eAlgoGray_Mean)
	{
		AlgoGrayMean *pInspAlgo = (AlgoGrayMean *)sInspAlgo.m_ptrInspAlgoParam;
		if (pInspAlgo->m_sBlobBase.m_sAlgoColorBase.m_bUseColor == FALSE)
			return;
		bUseAngleColor = pInspAlgo->m_sBlobBase.m_sAlgoColorBase.m_bUseAngleColor;
		byColorLightType = pInspAlgo->m_sBlobBase.m_sAlgoColorBase.m_byColorLightType;
	}
	else if (sInspAlgo.m_eAlgoType == eAlgoHeight_Mean)
	{
		AlgoHeightMean *pInspAlgo = (AlgoHeightMean *)sInspAlgo.m_ptrInspAlgoParam;
		if (pInspAlgo->m_sBlobHighestBase.m_sAlgoColorBase.m_bUseColor == FALSE)
			return;
		bUseAngleColor = pInspAlgo->m_sBlobHighestBase.m_sAlgoColorBase.m_bUseAngleColor;
		byColorLightType = pInspAlgo->m_sBlobHighestBase.m_sAlgoColorBase.m_byColorLightType;
	}
	else if (sInspAlgo.m_eAlgoType == eAlgoNGBlob)
	{//.7 주석처리되어있음.
// 		AlgoNGBlob *pInspAlgo = (AlgoNGBlob *)sInspAlgo.m_ptrInspAlgoParam;
// 		if (pInspAlgo->m_sAlgoColorBase.m_bUseColor == FALSE)
// 			return;
// 		bUseAngleColor = pInspAlgo->m_sAlgoColorBase.m_bUseAngleColor;
// 		byColorLightType = pInspAlgo->m_sAlgoColorBase.m_byColorLightType;
	}

	SetColorData(bUseAngleColor, nWidth, nHeight, pImg_buf, nType, rtClip, nSideCameraIndex, bAIData, byColorLightType);
}

void PInspAlgoWrapper::SetColorData(BOOL bAngleColor, int nWidth, int nHeight, PIAL::Insp_Image* pImg_buf, int nType, RECT rtClip, int nSideCameraIndex, bool bUseAI, byte byColorLightType)
{
	PIAL::PI_Buff* buf_TR = nullptr;
	PIAL::PI_Buff* buf_TG = nullptr;
	PIAL::PI_Buff* buf_TB = nullptr;
	PIAL::PI_Buff* buf_TW = nullptr;
	PIAL::PI_Buff* buf_MR = nullptr;
	PIAL::PI_Buff* buf_MB = nullptr;
	PIAL::PI_Buff* buf_BR = nullptr;
	PIAL::PI_Buff* buf_BB = nullptr;

	PIAL::PI_Buff* buf_R_AC = nullptr;
	PIAL::PI_Buff* buf_G_AC = nullptr;
	PIAL::PI_Buff* buf_B_AC = nullptr;
	PIAL::PI_Buff* buf_BR_AC = nullptr;
	PIAL::PI_Buff* buf_BB_AC = nullptr;

	int nImageWidth = 0;
	int nImageHeight = 0;
	if (nType == 1)
	{
		nImageWidth = pImg_buf->inspWndImage->nImgSizeX;
		nImageHeight = pImg_buf->inspWndImage->nImgSizeY;
		//if (g_pMPTI->m_bSideOriginalSize == true)	// ?섎?移대찓??
		//{
		//  이거 사용 할 경우에는 여기서 할당후 Clip할것.
		//	m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide1_R, nWidth, nHeight, ucImg_TR, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		//	m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide1_G, nWidth, nHeight, ucImg_TG, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		//	m_procMil->GetClipImage_LT(m_pInspBoardInfo->partImgBuf.imgSide1_B, nWidth, nHeight, ucImg_TB, dX, dY, sInspImgBuf->nImageSizeX, sInspImgBuf->nImageSizeY);
		//}
		//else
		//{
		if (nSideCameraIndex == 0 && pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Side1_R) != NULL)
		{
			buf_TR = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Side1_R);
			buf_TG = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Side1_G);
			buf_TB = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Side1_B);
		}
		else if (nSideCameraIndex == 1 && pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Side2_R) != NULL)
		{
			buf_TR = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Side2_R);
			buf_TG = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Side2_G);
			buf_TB = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Side2_B);
		}
		else if (nSideCameraIndex == 2 && pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Side3_R) != NULL)
		{
			buf_TR = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Side3_R);
			buf_TG = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Side3_G);
			buf_TB = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Side3_B);
		}
		else if (nSideCameraIndex == 3 && pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Side4_R) != NULL)
		{
			buf_TR = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Side4_R);
			buf_TG = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Side4_G);
			buf_TB = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Side4_B);
		}
		else
		{
			buf_TR = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Top_R);
			buf_TG = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Top_G);
			buf_TB = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Top_B);
		}
	// }
		cv::Mat tr = buf_TR->Mat();
		cv::Mat tg = buf_TG->Mat();
		cv::Mat tb = buf_TB->Mat();

		buf_TW = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Top_W);

		buf_MR = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Middle_R);
		buf_MB = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Middle_B);

		buf_BR = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Bottom_R);
		buf_BB = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Bottom_B);
	}
	else if (nType == 2)
	{
		nImageWidth = pImg_buf->inspWndImage->m_ImageBuffer->nImageSizeX;
		nImageHeight = pImg_buf->inspWndImage->m_ImageBuffer->nImageSizeY;
		buf_TR = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Top_R);
		buf_TG = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Top_G);
		buf_TB = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Top_B);
		buf_TW = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Top_W);

		buf_MR = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Middle_R);
		buf_MB = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Middle_B);

		buf_BR = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Bottom_R);
		buf_BB = pImg_buf->inspWndImage->m_ImageBuffer->GetBuff(PIAL::Bottom_B);
	}

	if (bAngleColor == TRUE)
	{
		buf_R_AC = new PIAL::PI_Buff(nImageWidth, nImageHeight);
		buf_G_AC = new PIAL::PI_Buff(nImageWidth, nImageHeight);
		buf_B_AC = new PIAL::PI_Buff(nImageWidth, nImageHeight);

		LightTypeBuf sLightBuf;
		PIAL::PI_Buff* TRAC_temp = buf_TR->Clone();
		PIAL::PI_Buff* TGAC_temp = buf_TG->Clone();
		PIAL::PI_Buff* TBAC_temp = buf_TB->Clone();
		PIAL::PI_Buff* TWAC_temp = bUseAI ? NULL : buf_TW->Clone();
		PIAL::PI_Buff* MRAC_temp = buf_MR->Clone();
		PIAL::PI_Buff* MBAC_temp = buf_MB->Clone();
		PIAL::PI_Buff* BRAC_temp = buf_BR->Clone();
		PIAL::PI_Buff* BBAC_temp = buf_BB->Clone();

		sLightBuf.m_pucTRed = TRAC_temp->m_pData;
		sLightBuf.m_pucTGreen = TGAC_temp->m_pData;
		sLightBuf.m_pucTBlue = TBAC_temp->m_pData;
		sLightBuf.m_pucTWhite = TWAC_temp->m_pData;
		sLightBuf.m_pucMRed = MRAC_temp->m_pData;
		sLightBuf.m_pucMGreen = NULL;
		sLightBuf.m_pucMBlue = MBAC_temp->m_pData;
		sLightBuf.m_pucMWhite = NULL;
		sLightBuf.m_pucBRed = BRAC_temp->m_pData;
		sLightBuf.m_pucBGreen = NULL;
		sLightBuf.m_pucBBlue = BBAC_temp->m_pData;
		sLightBuf.m_pucBWhite = NULL;
		sLightBuf.m_nImgWidth = nImageWidth;
		sLightBuf.m_nImgHeight = nImageHeight;
		sLightBuf.m_nROIImgWidth = nImageWidth;
		sLightBuf.m_nROIImgHeight = nImageHeight;
		sLightBuf.m_dROIX = 0;
		sLightBuf.m_dROIY = 0;

		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &sLightBuf.m_pnRedValue, _LIGHT_CNT);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &sLightBuf.m_pnGreenValue, _LIGHT_CNT);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &sLightBuf.m_pnBlueValue, _LIGHT_CNT);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &sLightBuf.m_pnWhiteValue, _LIGHT_CNT);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &sLightBuf.m_pnPosition, _LIGHT_CNT);
		Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &sLightBuf.m_pnCalculation, _LIGHT_CNT);
		for (int a = 0; a < 3; a++)
		{
			sLightBuf.m_nImgCnt = PIAL::PInspAlgo_Lib::m_sLightData[a].m_byLightCnt;
			memset(sLightBuf.m_pnRedValue, 0, LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnGreenValue, 0, LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnBlueValue, 0, LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnWhiteValue, 0, LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnPosition, 0, LIGHT_CNT * sizeof(int));
			memset(sLightBuf.m_pnCalculation, 0, LIGHT_CNT * sizeof(int));
			for (int b = 0; b < LIGHT_CNT; b++)
			{
				sLightBuf.m_pnRedValue[b] = PIAL::PInspAlgo_Lib::m_sLightData[a].m_byArrLightData[eLightData_Red][b];
				sLightBuf.m_pnGreenValue[b] = PIAL::PInspAlgo_Lib::m_sLightData[a].m_byArrLightData[eLightData_Green][b];
				sLightBuf.m_pnBlueValue[b] = PIAL::PInspAlgo_Lib::m_sLightData[a].m_byArrLightData[eLightData_Blue][b];
				sLightBuf.m_pnWhiteValue[b] = PIAL::PInspAlgo_Lib::m_sLightData[a].m_byArrLightData[eLightData_White][b];
				sLightBuf.m_pnPosition[b] = PIAL::PInspAlgo_Lib::m_sLightData[a].m_byArrLightData[eLightData_Position][b];
				sLightBuf.m_pnCalculation[b] = PIAL::PInspAlgo_Lib::m_sLightData[a].m_byArrLightData[eLightData_Calculation][b];
			}
			if (a == 0)
				ROIImageClaculCompose(sLightBuf, buf_R_AC->m_pData);
			else if (a == 1)
				ROIImageClaculCompose(sLightBuf, buf_G_AC->m_pData);
			else if (a == 2)
				ROIImageClaculCompose(sLightBuf, buf_B_AC->m_pData);
		}

		pImg_buf->m_AngleColorImageBuffer->AddBuff(PIAL::Top_R, buf_R_AC);
		pImg_buf->m_AngleColorImageBuffer->AddBuff(PIAL::Top_G, buf_G_AC);
		pImg_buf->m_AngleColorImageBuffer->AddBuff(PIAL::Top_B, buf_B_AC);
		pImg_buf->m_AngleColorImageBuffer->AddBuff(PIAL::Bottom_R, BRAC_temp);
		pImg_buf->m_AngleColorImageBuffer->AddBuff(PIAL::Bottom_B, BBAC_temp);

		delete TRAC_temp;
		delete TGAC_temp;
		delete TBAC_temp;
		delete TWAC_temp;
		delete MRAC_temp;
		delete MBAC_temp;
		// delete BRAC_temp;
		// delete BBAC_temp;

		Delete_1DArray(&sLightBuf.m_pnRedValue);
		Delete_1DArray(&sLightBuf.m_pnGreenValue);
		Delete_1DArray(&sLightBuf.m_pnBlueValue);
		Delete_1DArray(&sLightBuf.m_pnWhiteValue);
		Delete_1DArray(&sLightBuf.m_pnPosition);
		Delete_1DArray(&sLightBuf.m_pnCalculation);
	}
	else
	{
		pImg_buf->m_AngleColorImageBuffer->AddBuff(PIAL::Top_R, buf_TR->Clone());
		pImg_buf->m_AngleColorImageBuffer->AddBuff(PIAL::Top_G, buf_TG->Clone());
		pImg_buf->m_AngleColorImageBuffer->AddBuff(PIAL::Top_B, buf_TB->Clone());
		pImg_buf->m_AngleColorImageBuffer->AddBuff(PIAL::Bottom_R, buf_BR->Clone());
		pImg_buf->m_AngleColorImageBuffer->AddBuff(PIAL::Bottom_B, buf_BB->Clone());
		if (byColorLightType == 1 && buf_MR != NULL && buf_MB != NULL)
		{
#if _DEBUG
			cv::Mat TopR = buf_TR->Mat().clone();
			cv::Mat TopG = buf_TG->Mat().clone();
			cv::Mat TopB = buf_TB->Mat().clone();
#endif
			cv::Mat imgG = m_PInspAlgo->GetGreenImage(buf_MR, buf_MB);
			for (int r = 0; r < buf_MR->Length(); r++)
			{
				UCHAR* ucImg_MR = &buf_MR->m_pData[buf_MR->Pitch()*r];
				UCHAR* ucImg_MB = &buf_MB->m_pData[buf_MR->Pitch()*r];
				UCHAR* ucImg_TopR = &buf_TR->m_pData[buf_MR->Pitch()*r];
				UCHAR* ucImg_TopG = &buf_TG->m_pData[buf_MR->Pitch()*r];
				UCHAR* ucImg_TopB = &buf_TB->m_pData[buf_MR->Pitch()*r];
				memcpy(ucImg_TopR, ucImg_MR, sizeof(UCHAR) * buf_MR->Width());
				memcpy(ucImg_TopB, ucImg_MB, sizeof(UCHAR) * buf_MR->Width());
				memcpy(ucImg_TopG, imgG.ptr(r), sizeof(UCHAR) * buf_MR->Width());
			}

#if _DEBUG
			cv::Mat NewTopR = buf_TR->Mat().clone();
			cv::Mat NewTopG = buf_TG->Mat().clone();
			cv::Mat NewTopB = buf_TB->Mat().clone();
			cv::Mat NewMidR = buf_MR->Mat().clone();
			cv::Mat NewMidB = buf_MB->Mat().clone();
#endif
		}
	}

	pImg_buf->m_AngleColorImageBuffer->nImageSizeX = nImageWidth;
	pImg_buf->m_AngleColorImageBuffer->nImageSizeY = nImageHeight;
}

bool PInspAlgoWrapper::SetWindowImage(InspPartInfo* InspPartInfo, PIAL::Insp_Image* pImg_buf, RECT rtWnd)
{
	// Window Size Clip Image 
	int nPartWidth = InspPartInfo->partImgBuf.nImageSizeX;
	int nPartHeight = InspPartInfo->partImgBuf.nImageSizeY;

	PIAL::PI_Buff* buf_TR = nullptr;
	PIAL::PI_Buff* buf_TG = nullptr;
	PIAL::PI_Buff* buf_TB = nullptr;
	PIAL::PI_Buff* buf_TW = nullptr;

	PIAL::PI_Buff* buf_MR = nullptr;
	PIAL::PI_Buff* buf_MB = nullptr;

	PIAL::PI_Buff* buf_BR = nullptr;
	PIAL::PI_Buff* buf_BB = nullptr;

	PIAL::PI_Buff* buf_S1R = nullptr;
	PIAL::PI_Buff* buf_S1G = nullptr;
	PIAL::PI_Buff* buf_S1B = nullptr;

	PIAL::PI_Buff* buf_S2R = nullptr;
	PIAL::PI_Buff* buf_S2G = nullptr;
	PIAL::PI_Buff* buf_S2B = nullptr;

	PIAL::PI_Buff* buf_S3R = nullptr;
	PIAL::PI_Buff* buf_S3G = nullptr;
	PIAL::PI_Buff* buf_S3B = nullptr;

	PIAL::PI_Buff* buf_S4R = nullptr;
	PIAL::PI_Buff* buf_S4G = nullptr;
	PIAL::PI_Buff* buf_S4B = nullptr;
#pragma region Top
	if (InspPartInfo->partImgBuf.imgTop_R != NULL)
	{
		buf_TR = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgTop_R, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		// cv::Mat tt = cv::Mat(nPartHeight, nPartWidth, CV_8UC1, InspPartInfo->partImgBuf.imgTop_R);
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Top_R, buf_TR);
	}

	if (InspPartInfo->partImgBuf.imgTop_G != NULL)
	{
		buf_TG = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgTop_G, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		// cv::Mat tg = buf_TG->Mat();
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Top_G, buf_TG);
	}

	if (InspPartInfo->partImgBuf.imgTop_B != NULL)
	{
		buf_TB = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgTop_B, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		// cv::Mat tb = buf_TB->Mat();
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Top_B, buf_TB);
	}

	if (InspPartInfo->partImgBuf.imgTop_W != NULL)
	{
		buf_TW = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgTop_W, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		// cv::Mat tw = buf_TW->Mat();
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Top_W, buf_TW);
	}
#pragma endregion

#pragma region Middle
	if (InspPartInfo->partImgBuf.imgMiddle_R != NULL)
	{
		buf_MR = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgMiddle_R, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		// cv::Mat mr = buf_MR->Mat();
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Middle_R, buf_MR);

	}

	if (InspPartInfo->partImgBuf.imgMiddle_B != NULL)
	{
		buf_MB = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgMiddle_B, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		// cv::Mat mb = buf_MB->Mat();
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Middle_B, buf_MB);
	}
#pragma endregion

#pragma region Bottom
	if (InspPartInfo->partImgBuf.imgBottom_R != NULL)
	{
		buf_BR = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgBottom_R, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		//cv::Mat br = buf_BR->Mat();
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Bottom_R, buf_BR);

	}

	if (InspPartInfo->partImgBuf.imgBottom_B != NULL)
	{
		buf_BB = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgBottom_B, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		// cv::Mat bb = buf_BB->Mat();
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Bottom_B, buf_BB);

	}
#pragma endregion

#pragma region Side1
	if (InspPartInfo->partImgBuf.imgSide1_R != NULL)
	{
		buf_S1R = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgSide1_R, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Side1_R, buf_S1R);
	}

	if (InspPartInfo->partImgBuf.imgSide1_G != NULL)
	{
		buf_S1G = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgSide1_G, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Side1_G, buf_S1G);
	}

	if (InspPartInfo->partImgBuf.imgSide1_B != NULL)
	{
		buf_S1B = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgSide1_B, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Side1_B, buf_S1B);
	}

#pragma endregion

#pragma region Side2
	if (InspPartInfo->partImgBuf.imgSide2_R != NULL)
	{
		buf_S2R = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgSide2_R, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Side2_R, buf_S2R);
	}

	if (InspPartInfo->partImgBuf.imgSide2_G != NULL)
	{
		buf_S2G = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgSide2_G, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Side2_G, buf_S2G);
	}

	if (InspPartInfo->partImgBuf.imgSide2_B != NULL)
	{
		buf_S2B = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgSide2_B, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Side2_B, buf_S2B);
	}
#pragma endregion

#pragma region Side3
	if (InspPartInfo->partImgBuf.imgSide3_R != NULL)
	{
		buf_S3R = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgSide3_R, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Side3_R, buf_S3R);
	}

	if (InspPartInfo->partImgBuf.imgSide3_G != NULL)
	{
		buf_S3G = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgSide3_G, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Side3_G, buf_S3G);
	}

	if (InspPartInfo->partImgBuf.imgSide3_B != NULL)
	{
		buf_S3B = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgSide3_B, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Side3_B, buf_S3B);
	}
#pragma endregion

#pragma region Side4
	if (InspPartInfo->partImgBuf.imgSide4_R != NULL)
	{
		buf_S4R = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgSide4_R, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Side4_R, buf_S4R);
	}

	if (InspPartInfo->partImgBuf.imgSide4_G != NULL)
	{
		buf_S4G = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgSide4_G, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Side4_G, buf_S4G);
	}

	if (InspPartInfo->partImgBuf.imgSide4_B != NULL)
	{
		buf_S4B = PIAL::PI_Buff(InspPartInfo->partImgBuf.imgSide4_B, nPartWidth, nPartHeight, nPartWidth).ClipBuff_LT(rtWnd);
		pImg_buf->inspWndImage->m_ImageBuffer->AddBuff(PIAL::Side4_B, buf_S4B);
	}
#pragma endregion
	return true;
}

bool PInspAlgoWrapper::GetAlgoImage(InspAlgo* sInspAlgo, InspPartInfo* InspPartInfo, PIAL::Insp_Image* pImg_buf, RECT rtWnd, int nSideCameraType)
{
	// 각 알고리즘 별 이미지 담는 함수	
	return true;
	// 일단 Return 처리 한다. 각 알고리즘별로는 따로 처리 할 것.

	InspAlgoType algoType = sInspAlgo->m_eAlgoType;

	int nWidth = InspPartInfo->partImgBuf.nImageSizeX;
	int nHeight = InspPartInfo->partImgBuf.nImageSizeY;

	if (algoType == eAlgoAlignEdge)
	{
	}

	switch (sInspAlgo->m_eAlgoType)
	{
		case eAlgoPattern:
		{
			break;
		}
		case eAlgoOCR:
		case eAlgoPOCR:
		case eAlgoAlignEdge:
		{
			break;
		}
		case eAlgoGrid:
		case eAlgoBlob:
		{
			break;
		}
		case eAlgoFoot:
		case eAlgoWire:
		{
			break;
		}


	}
	return true;
}

bool PInspAlgoWrapper::AlgoImageMixCompose(InspPartInfo* InspPartInfo, const InspAlgoLight &algoLight, UCHAR* ptr2D, int width, int height, int wnd_w, int wnd_h, double dX, double dY)
{
	UCHAR * ImgR = NULL, *ImgG = NULL, *ImgB = NULL, *ImgW = NULL;
	LightTypeBuf sLightBuf;
	ImgW = InspPartInfo->partImgBuf.imgTop_W;

	if (algoLight.m_eLightType == User_Light)
	{
		sLightBuf.m_pucTRed = InspPartInfo->partImgBuf.imgTop_R;
		sLightBuf.m_pucTGreen = InspPartInfo->partImgBuf.imgTop_G;
		sLightBuf.m_pucTBlue = InspPartInfo->partImgBuf.imgTop_B;
		sLightBuf.m_pucTWhite = InspPartInfo->partImgBuf.imgTop_W;

		sLightBuf.m_pucMRed = InspPartInfo->partImgBuf.imgMiddle_R;
		sLightBuf.m_pucMGreen = NULL;
		sLightBuf.m_pucMBlue = InspPartInfo->partImgBuf.imgMiddle_B;
		sLightBuf.m_pucMWhite = NULL;

		sLightBuf.m_pucBRed = InspPartInfo->partImgBuf.imgBottom_R;
		sLightBuf.m_pucBGreen = NULL;
		sLightBuf.m_pucBBlue = InspPartInfo->partImgBuf.imgBottom_B;
		sLightBuf.m_pucBWhite = NULL;

		sLightBuf.m_nImgWidth = width;
		sLightBuf.m_nImgHeight = height;
		sLightBuf.m_nROIImgWidth = wnd_w;
		sLightBuf.m_nROIImgHeight = wnd_h;
		sLightBuf.m_dROIX = dX;
		sLightBuf.m_dROIY = dY;
		sLightBuf.m_nImgCnt = algoLight.m_nLightCnt;
		sLightBuf.m_pnRedValue = algoLight.m_nArrRedValue;
		sLightBuf.m_pnGreenValue = algoLight.m_nArrGreenValue;
		sLightBuf.m_pnBlueValue = algoLight.m_nArrBlueValue;
		sLightBuf.m_pnWhiteValue = algoLight.m_nArrWhiteValue;
		sLightBuf.m_pnPosition = algoLight.m_nArrLightPosition;
		sLightBuf.m_pnCalculation = algoLight.m_nArrCalculation;

		ROIImageClaculCompose(sLightBuf, ptr2D);
	}
	else
	{
		switch (algoLight.m_eLightType)
		{
		case Top_Light:
			ImgR = InspPartInfo->partImgBuf.imgTop_R;
			ImgG = InspPartInfo->partImgBuf.imgTop_G;
			ImgB = InspPartInfo->partImgBuf.imgTop_B;
			break;
		case Middle_Light:
			ImgR = InspPartInfo->partImgBuf.imgMiddle_R;
			ImgB = InspPartInfo->partImgBuf.imgMiddle_B;
			break;
		case Bottom_Light:
			ImgR = InspPartInfo->partImgBuf.imgBottom_R;
			ImgB = InspPartInfo->partImgBuf.imgBottom_B;
			break;

		}

		RoiImageCompose_LT(ImgR, ImgG, ImgB, ImgW, width*height, width, height, wnd_w*wnd_h, dX, dY, wnd_w, wnd_h, algoLight.m_nRedValue, algoLight.m_nGreenValue, algoLight.m_nBlueValue, algoLight.m_nWhiteValue, ptr2D);
	}

	return true;
}

bool PInspAlgoWrapper::WndSizeChange(PIAL::Insp_Image* pImg_buf, InspAlgo sInspAlgo, AlgoCoordinate &coordinateAlgo, InspPartInfo* InspBoardInfo, InspPartParam* pParamArray, int nWndIndex, int &nWidth, int &nHeight, double &dX, double &dY, AlignResult* curAlignRes, int &nOffX_pix, int &nOffY_pix)
{
	bool bResult = FALSE;

	InspAlgoType eAlgoType = sInspAlgo.m_eAlgoType;
	bool bPassive = false;
	//if (eAlgoType == eAlgoBody_Blob)
	//	bPassive = ((AlgoBodyBlob *)sInspAlgo.m_ptrInspAlgoParam)->Passive;

	double dResolX = PIAL::PInspAlgo_Lib::m_resolX;
	double dResolY = PIAL::PInspAlgo_Lib::m_resolY;
	nOffX_pix = nOffY_pix = 0;
	float fPartRoundErrX = 0;
	float fPartRoundErrY = 0;
	float fStartX = 0.0;
	float fStartY = 0.0;

	float fPartWidth = InspBoardInfo->partWidth / dResolX;//m_resolX;
	float fPartHeight = InspBoardInfo->partHeight / dResolY;//m_resolY;
	float fWndWidth = (float)(pParamArray[nWndIndex].width / dResolX);//m_resolX);
	float fWndHeight = (float)(pParamArray[nWndIndex].length / dResolY);//m_resolY);

	int nWndWidth = RounD(fWndWidth);
	int nWndHeight = RounD(fWndHeight);

	if (eAlgoType != eAlgoShapeX) // ShapeX Algorithm 은 Part RoundErr 값 사용하지않도록 변경
	{
		fPartRoundErrX = InspBoardInfo->fPartRoundingErrX;
		fPartRoundErrY = InspBoardInfo->fPartRoundingErrX;
	}
	if (eAlgoType == eAlgoBody_Blob && !bPassive)
	{
		// 파트 Coordeinate가 필요.
		coordinateAlgo.dROICenterX = (fPartWidth / 2.);
		coordinateAlgo.dROICenterY = (fPartHeight / 2.);
		coordinateAlgo.dROIWidth = (fPartWidth);
		coordinateAlgo.dROILength = (fPartHeight);
		coordinateAlgo.dROIAngle = InspBoardInfo->angle;
	}
	else
	{
		coordinateAlgo.dROICenterX = fPartWidth / 2. + pParamArray[nWndIndex].cx / dResolX;
		coordinateAlgo.dROICenterY = fPartHeight / 2. - pParamArray[nWndIndex].cy / dResolY;

		coordinateAlgo.dROIWidth = fWndWidth;
		coordinateAlgo.dROILength = fWndHeight;// + 0.01;	
		coordinateAlgo.dROIAngle = InspBoardInfo->angle;
	}

	if ((eAlgoType == eAlgoAlign) || (eAlgoType == eAlgoGray_Diff) || (eAlgoType == eAlgoHeight_Diff) || ((eAlgoType == eAlgoBody_Blob && !bPassive))
		|| (eAlgoType == eAlgoTilt) || /*(eAlgoType == eAlgoLead_SideSolder) || */(eAlgoType == eAlgoAlignEdge) || (eAlgoType == eAlgoPadAlign) || (eAlgoType == eAlgoBodyEdge))
	{
		nWidth = RounD(fPartWidth);
		nHeight = RounD(fPartHeight);
		fStartX = 0;
		fStartY = 0;

		dX = fStartX;
		dY = fStartY;

		if (eAlgoType == eAlgoPadAlign)
		{
			nWndWidth = nWidth;
			nWndHeight = nHeight;
		}
	}
	else
	{
		nWidth = RounD(coordinateAlgo.dROIWidth);
		nHeight = RounD(coordinateAlgo.dROILength);

		double corr_x = 0, corr_y = 0;
		bool bBridgeOffset = true;

		if (curAlignRes && (bBridgeOffset == true) && (eAlgoType != eAlgoFoot))
		{
			if (false)
			{
			}
			else
			{
				double dAngle = 0.; 
				if (eAlgoType == eAlgoBarcode)
					dAngle = curAlignRes->theta; 
				PIAL::PAlgo::CorrectCoordinate(pParamArray[nWndIndex].cx, pParamArray[nWndIndex].cy, curAlignRes->centerX, curAlignRes->centerY, dAngle, curAlignRes->offsetX, curAlignRes->offsetY, &corr_x, &corr_y);
				//fStartX = RounD(fPartWidth) / 2. + corr_x / dResolX;//m_resolX;
				//fStartY = RounD(fPartHeight) / 2. - corr_y / dResolY;//m_resolY;
				//coordinateAlgo.dROICenterX = fStartX;
				//coordinateAlgo.dROICenterY = fStartY;
				//dX = RounD(fStartX - (nWidth / 2.));
				//dY = RounD(fStartY - (nHeight / 2.));

				fStartX = fPartWidth / 2. + corr_x / dResolX;
				fStartY = fPartHeight / 2. - corr_y / dResolY;

				coordinateAlgo.dROICenterX = fStartX - fPartRoundErrX;
				coordinateAlgo.dROICenterY = fStartY - fPartRoundErrY;

				//apply PartClip Rounding Err  NYJ 2020.12
				float fDX, fDY;
				fDX = fStartX - coordinateAlgo.dROIWidth / 2.;
				fDY = fStartY - coordinateAlgo.dROILength / 2.;

				fDX -= fPartRoundErrX;
				fDY -= fPartRoundErrY;

				dX = (float)RounD(fDX);
				dY = (float)RounD(fDY);

				//save WindowClip Round err
				InspBoardInfo->fWndRoundingErrX = dX - fDX;
				InspBoardInfo->fWndRoundingErrY = dY - fDY;

				//apply WindowClip Round Err 
				coordinateAlgo.dROICenterX += InspBoardInfo->fWndRoundingErrX;
				coordinateAlgo.dROICenterY += InspBoardInfo->fWndRoundingErrY;

				nOffX_pix = RounD((corr_x - pParamArray[nWndIndex].cx) / dResolX);	// Align결과에 의해 offset된 pixel수
				nOffY_pix = RounD((pParamArray[nWndIndex].cy - corr_y) / dResolY);
			}
		}
		else
		{
			//fStartX = fPartWidth / 2. + (pParamArray[nWndIndex].cx) / dResolX;//m_resolX;
			//fStartY = fPartHeight / 2. - (pParamArray[nWndIndex].cy) / dResolY;//m_resolY;
			//coordinateAlgo.dROICenterX = fStartX;
			//coordinateAlgo.dROICenterY = fStartY;
			//dX = RounD(fStartX - (nWidth / 2.));
			//dY = RounD(fStartY - (nHeight / 2.));


			fStartX = fPartWidth / 2. + pParamArray[nWndIndex].cx / dResolX;
			fStartY = fPartHeight / 2. - pParamArray[nWndIndex].cy / dResolY;

			//apply PartClip Rounding Err  NYJ 2020.12
			float fDX, fDY;
			fDX = fStartX - coordinateAlgo.dROIWidth / 2.;
			fDY = fStartY - coordinateAlgo.dROILength / 2.;

			fDX -= fPartRoundErrX;
			fDY -= fPartRoundErrY;

			dX = (float)RounD(fDX);
			dY = (float)RounD(fDY);

			//save WindowClip Round err
			InspBoardInfo->fWndRoundingErrX = dX - fDX;
			InspBoardInfo->fWndRoundingErrY = dY - fDY;

			//apply WindowClip Round Err 
			//coordinateAlgo.dROICenterX += InspBoardInfo->fWndRoundingErrX;
			//coordinateAlgo.dROICenterY += InspBoardInfo->fWndRoundingErrY;
		}
	}
	if (dX < 0) dX = 0;
	if (dY < 0) dY = 0;
	if (dX + nWidth > RounD(fPartWidth))	dX = RounD(fPartWidth - nWidth);
	if (dY + nHeight > RounD(fPartHeight))	dY = RounD(fPartHeight - nHeight);


	// Algorithm 함수에 넘길 Window Image 구조체
	pImg_buf->inspWndImage->nImgSizeX = nWidth;
	pImg_buf->inspWndImage->nImgSizeY = nHeight;
	pImg_buf->inspPartImage->nImgSizeX = InspBoardInfo->partImgBuf.nImageSizeX;
	pImg_buf->inspPartImage->nImgSizeY = InspBoardInfo->partImgBuf.nImageSizeY;
	pImg_buf->inspWndImage->nStartX = dX;
	pImg_buf->inspWndImage->nStartY = dY;

	pImg_buf->inspWndImage->fWndCX = coordinateAlgo.dROICenterX;
	pImg_buf->inspWndImage->fWndCY = coordinateAlgo.dROICenterY;

	pImg_buf->inspPartImage->m_fPartRoundingErrX = fPartRoundErrX;
	pImg_buf->inspPartImage->m_fPartRoundingErrY = fPartRoundErrY;

	pImg_buf->m_p2D = new PIAL::PI_Buff(nWidth, nHeight);

	//for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)	wndImg.m_ucArr2D_Mix[iLoopCount] = new UCHAR[wndImg.m_nWidth * wndImg.m_nHeight];	//LJH 합칠 버퍼 할당
	//wndImg.m_fArr3D = new float[wndImg.m_nWidth * wndImg.m_nHeight];
	pImg_buf->inspPartImage->m_Angle = coordinateAlgo.dROIAngle;

	double dAngle = 0;
	if (curAlignRes != NULL && eAlgoType == eAlgoBlob)
		dAngle = curAlignRes[0].theta;
	if (InspBoardInfo->partZmapData.data != NULL)
	{
		int nStartIndex = InspBoardInfo->partZmapData.zmapSizeX* dY + dX;
		pImg_buf->inspWndImage->m_p3D = new PIAL::PI_Buff(InspBoardInfo->partZmapData.data + nStartIndex, pImg_buf->inspWndImage->nImgSizeX, pImg_buf->inspWndImage->nImgSizeY, InspBoardInfo->partZmapData.zmapSizeX * 4, true);
	}

	return bResult = true;

}

bool PInspAlgoWrapper::WndSizeChange_ArrAlign(PIAL::Insp_Image* pImg_buf, InspAlgoType nAlgoType, AlgoCoordinate &coordinateAlgo, InspPartInfo* InspBoardInfo, InspPartParam* pParamArray
	, int nCurWndIndex, int &nWidth, int &nHeight, double &dX, double &dY, AlignResult * curAlignRes, int nWndinspType, InspAlgo sInspAlgo, int nAlignTotalCnt)
{
	bool bResult = false;

	InspAlgoType eAlgoType = nAlgoType;

	WndAlgoImg sWndAlgoImg;
	sWndAlgoImg.Destroy();
	dX = 0;
	dY = 0;
	double dCx = pParamArray[nCurWndIndex].cx;
	double dCy = pParamArray[nCurWndIndex].cy;
	double m_resolX = PIAL::PInspAlgo_Lib::m_resolX;
	double m_resolY = PIAL::PInspAlgo_Lib::m_resolY;
	for (int n = 0; n < nAlignTotalCnt; n++)
	{
		WndAlgoImg sWndAlgoImg_Buf;
		sWndAlgoImg_Buf.Destroy();
		int nWndIndex = n;
		float fStartX = 0.0;
		float fStartY = 0.0;
		float fPartWidth = InspBoardInfo->partWidth / m_resolX;
		float fPartHeight = InspBoardInfo->partHeight / m_resolY;
		float fWndWidth = (float)(pParamArray[nCurWndIndex].width / m_resolX);
		float fWndHeight = (float)(pParamArray[nCurWndIndex].length / m_resolY);

		double dAlignRes_centerX = curAlignRes[nWndIndex].centerX;
		double dAlignRes_centerY = curAlignRes[nWndIndex].centerY;
		double dAlignRes_theta = curAlignRes[nWndIndex].theta;
		double dAlignRes_offsetX = curAlignRes[nWndIndex].offsetX;
		double dAlignRes_offsetY = curAlignRes[nWndIndex].offsetY;
		int nAlignRes_Width = curAlignRes[nWndIndex].rcBodyRect.Width();
		int nAlignRes_Height = curAlignRes[nWndIndex].rcBodyRect.Height();
		LONG lAlignRes_left = curAlignRes[nWndIndex].rcBodyRect.left;
		LONG lAlignRes_right = curAlignRes[nWndIndex].rcBodyRect.right;
		LONG lAlignRes_top = curAlignRes[nWndIndex].rcBodyRect.top;
		LONG lAlignRes_bottom = curAlignRes[nWndIndex].rcBodyRect.bottom;

		int nWndWidth = RounD(fWndWidth);
		int nWndHeight = RounD(fWndHeight);

		if (eAlgoType == eAlgoBody_Blob)
		{
			coordinateAlgo.dROICenterX = (fPartWidth / 2.);
			coordinateAlgo.dROICenterY = (fPartHeight / 2.);
			coordinateAlgo.dROIWidth = (fPartWidth);
			coordinateAlgo.dROILength = (fPartHeight);
			coordinateAlgo.dROIAngle = InspBoardInfo->angle;
		}
		else
		{
			coordinateAlgo.dROICenterX = fPartWidth / 2. + dCx / m_resolX;
			coordinateAlgo.dROICenterY = fPartHeight / 2. - dCy / m_resolY;
			coordinateAlgo.dROIWidth = fWndWidth;// + 0.01;		// 임시
			coordinateAlgo.dROILength = fWndHeight;// + 0.01;	
			coordinateAlgo.dROIAngle = InspBoardInfo->angle;
		}

		if ((eAlgoType == eAlgoAlign) || (eAlgoType == eAlgoGray_Diff) || (eAlgoType == eAlgoHeight_Diff) || (eAlgoType == eAlgoBody_Blob)
			|| (eAlgoType == eAlgoTilt) || /*(eAlgoType == eAlgoLead_SideSolder) || */(eAlgoType == eAlgoAlignEdge) || (eAlgoType == eAlgoPadAlign))
		{
			nWidth = RounD(fPartWidth);
			nHeight = RounD(fPartHeight);
			fStartX = 0;
			fStartY = 0;

			dX = fStartX;
			dY = fStartY;

			if (eAlgoType == eAlgoPadAlign)
			{
				nWndWidth = nWidth;
				nWndHeight = nHeight;
			}
		}
		else
		{
			nWidth = RounD(coordinateAlgo.dROIWidth);
			nHeight = RounD(coordinateAlgo.dROILength);

			double corr_x = 0, corr_y = 0;
			bool bBridgeOffset = true;

			if (curAlignRes && (bBridgeOffset == true))
			{
				if (false) {}
				else
				{
					PIAL::PAlgo::CorrectCoordinate(dCx, dCy, dAlignRes_centerX, dAlignRes_centerY, dAlignRes_theta, dAlignRes_offsetX, dAlignRes_offsetY, &corr_x, &corr_y);
					dCx = corr_x;
					dCy = corr_y;
					fStartX = fPartWidth / 2. + corr_x / m_resolX;
					fStartY = fPartHeight / 2. - corr_y / m_resolY;
					coordinateAlgo.dROICenterX = fStartX;
					coordinateAlgo.dROICenterY = fStartY;
					dX = (float)RounD(fStartX - coordinateAlgo.dROIWidth / 2.);
					dY = (float)RounD(fStartY - coordinateAlgo.dROILength / 2.);
					if (dX < 0)
						dX = 0;
					if (dY < 0)
						dY = 0;
					if (dX > (float)RounD(fPartWidth - coordinateAlgo.dROIWidth))
						dX = (float)RounD(fPartWidth - coordinateAlgo.dROIWidth);
					if (dY > (float)RounD(fPartHeight - coordinateAlgo.dROILength))
						dY = (float)RounD(fPartHeight - coordinateAlgo.dROILength);
				}
			}
			else
			{
				fStartX = fPartWidth / 2. + (dCx) / m_resolX;
				fStartY = fPartHeight / 2. - (dCy) / m_resolY;
				dX = (float)RounD(fStartX - coordinateAlgo.dROIWidth / 2.);
				dY = (float)RounD(fStartY - coordinateAlgo.dROILength / 2.);
			}
		}
		// Algorithm 함수에 넘길 Window Image 구조체
		if (n == 0)
		{
			sWndAlgoImg.m_nWidth = nWidth;
			sWndAlgoImg.m_nHeight = nHeight;
			sWndAlgoImg.m_nWidth3D = nWndWidth;
			sWndAlgoImg.m_nHeight3D = nWndHeight;
			//sWndAlgoImg.m_ucArr2D = new UCHAR[sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight];
			//for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)	sWndAlgoImg.m_ucArr2D_Mix[iLoopCount] = new UCHAR[sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight];	//LJH 합칠 버퍼 할당
			//sWndAlgoImg.m_fArr3D = new float[sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight];
			sWndAlgoImg.dAngle = coordinateAlgo.dROIAngle;
		}
		sWndAlgoImg_Buf.m_nWidth = nWidth;
		sWndAlgoImg_Buf.m_nHeight = nHeight;
		sWndAlgoImg_Buf.m_nWidth3D = nWndWidth;
		sWndAlgoImg_Buf.m_nHeight3D = nWndHeight;
		//sWndAlgoImg_Buf.m_ucArr2D = new UCHAR[sWndAlgoImg_Buf.m_nWidth * sWndAlgoImg_Buf.m_nHeight];
		//for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)	sWndAlgoImg_Buf.m_ucArr2D_Mix[iLoopCount] = new UCHAR[sWndAlgoImg_Buf.m_nWidth * sWndAlgoImg_Buf.m_nHeight];	//LJH 합칠 버퍼 할당
		//sWndAlgoImg_Buf.m_fArr3D = new float[sWndAlgoImg_Buf.m_nWidth * sWndAlgoImg_Buf.m_nHeight];
		sWndAlgoImg_Buf.dAngle = coordinateAlgo.dROIAngle;

		// 	m_proc3d.GetClipZmapData(m_pInspBoardInfo->partZmapData.data, wndImg.m_fArr3D, m_pInspBoardInfo->partZmapData.zmapSizeX,
		// 		m_pInspBoardInfo->partZmapData.zmapSizeY, dCenterX, dCenterY, nWidth, nHeight);

		if (n == 0) {
			/*m_proc3d.GetCropZmap_LT(m_pInspBoardInfo->partZmapData.data, sWndAlgoImg.m_fArr3D, m_pInspBoardInfo->partZmapData.zmapSizeX,
				m_pInspBoardInfo->partZmapData.zmapSizeY, dX, dY, nWidth, nHeight);*/
		}
		else
		{
			/*m_proc3d.GetCropZmap_LT(sWndAlgoImg.m_fArr3D, sWndAlgoImg_Buf.m_fArr3D, sWndAlgoImg.m_nWidth,
				sWndAlgoImg.m_nHeight, dX, dY, nWidth, nHeight);*/

			sWndAlgoImg.Destroy();
			sWndAlgoImg.m_nWidth = nWidth;
			sWndAlgoImg.m_nHeight = nHeight;
			sWndAlgoImg.m_nWidth3D = nWndWidth;
			sWndAlgoImg.m_nHeight3D = nWndHeight;
			//sWndAlgoImg.m_ucArr2D = new UCHAR[sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight];
			//for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)	sWndAlgoImg.m_ucArr2D_Mix[iLoopCount] = new UCHAR[sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight];	//LJH 합칠 버퍼 할당
			//sWndAlgoImg.m_fArr3D = new float[sWndAlgoImg.m_nWidth * sWndAlgoImg.m_nHeight];
			sWndAlgoImg.dAngle = coordinateAlgo.dROIAngle;
		}

		sWndAlgoImg_Buf.Destroy();
	}


	sWndAlgoImg.Destroy();

	// Algorithm 함수에 넘길 Window Image 구조체
	pImg_buf->inspWndImage->nImgSizeX = nWidth;
	pImg_buf->inspWndImage->nImgSizeY = nHeight;
	pImg_buf->inspPartImage->nImgSizeX = InspBoardInfo->partImgBuf.nImageSizeX;
	pImg_buf->inspPartImage->nImgSizeY = InspBoardInfo->partImgBuf.nImageSizeY;
	pImg_buf->inspWndImage->nStartX = dX;
	pImg_buf->inspWndImage->nStartY = dY;

	pImg_buf->inspWndImage->fWndCX = pImg_buf->inspWndImage->nStartX + (nWidth / 2.);
	pImg_buf->inspWndImage->fWndCY = pImg_buf->inspWndImage->nStartY + (nHeight / 2.);
	pImg_buf->inspPartImage->m_fPartRoundingErrX = InspBoardInfo->fPartRoundingErrX;
	pImg_buf->inspPartImage->m_fPartRoundingErrY = InspBoardInfo->fPartRoundingErrY;

	pImg_buf->m_p2D = new PIAL::PI_Buff(nWidth, nHeight);

	//for (int iLoopCount = 0; iLoopCount < MAX_MIX_COUNT; ++iLoopCount)	wndImg.m_ucArr2D_Mix[iLoopCount] = new UCHAR[wndImg.m_nWidth * wndImg.m_nHeight];	//LJH 합칠 버퍼 할당
	//wndImg.m_fArr3D = new float[wndImg.m_nWidth * wndImg.m_nHeight];
	pImg_buf->inspPartImage->m_Angle = coordinateAlgo.dROIAngle;

	double dAngle = 0;
	if (curAlignRes != NULL && eAlgoType == eAlgoBlob)
		dAngle = curAlignRes[0].theta;
	if (InspBoardInfo->partZmapData.data != NULL)
	{
		int nStartIndex = InspBoardInfo->partZmapData.zmapSizeX* dY + dX;
		pImg_buf->inspWndImage->m_p3D = new PIAL::PI_Buff(InspBoardInfo->partZmapData.data + nStartIndex, pImg_buf->inspWndImage->nImgSizeX, pImg_buf->inspWndImage->nImgSizeY, InspBoardInfo->partZmapData.zmapSizeX * 4, true);
	}

	return bResult = true;
}

void PInspAlgoWrapper::SetPartImage(InspPartInfo partInfo, PIAL::Insp_Image* pImg_buf)
{
	int nPartWidth = partInfo.partImgBuf.nImageSizeX;
	int nPartHeight = partInfo.partImgBuf.nImageSizeY;
	pImg_buf->inspPartImage->m_fPartRoundingErrX = partInfo.fPartRoundingErrX;
	pImg_buf->inspPartImage->m_fPartRoundingErrY = partInfo.fPartRoundingErrY;
	pImg_buf->inspPartImage->m_ImageBuffer->nImageSizeX = nPartWidth;
	pImg_buf->inspPartImage->m_ImageBuffer->nImageSizeY = nPartHeight;
	pImg_buf->inspPartImage->nImgSizeX = nPartWidth;
	pImg_buf->inspPartImage->nImgSizeY = nPartHeight;
	pImg_buf->inspPartImage->m_p3D = new PIAL::PI_Buff(partInfo.partZmapData.data, nPartWidth, nPartHeight, nPartWidth);
	PIAL::PI_Buff* TopR = nullptr;		 PIAL::PI_Buff* TopG = nullptr;		 PIAL::PI_Buff* TopB = nullptr;		PIAL::PI_Buff* TopW = nullptr;
	PIAL::PI_Buff* MiddleR = nullptr;	 PIAL::PI_Buff* MiddleB = nullptr;
	PIAL::PI_Buff* BottomR = nullptr;	 PIAL::PI_Buff* BottomB = nullptr;
	PIAL::PI_Buff* Side1R = nullptr;	 PIAL::PI_Buff* Side1G = nullptr;	 PIAL::PI_Buff* Side1B = nullptr;
	PIAL::PI_Buff* Side2R = nullptr;	 PIAL::PI_Buff* Side2G = nullptr;	 PIAL::PI_Buff* Side2B = nullptr;
	PIAL::PI_Buff* Side3R = nullptr;	 PIAL::PI_Buff* Side3G = nullptr;	 PIAL::PI_Buff* Side3B = nullptr;
	PIAL::PI_Buff* Side4R = nullptr;	 PIAL::PI_Buff* Side4G = nullptr;	 PIAL::PI_Buff* Side4B = nullptr;

#pragma region Top
	if (partInfo.partImgBuf.imgTop_R != nullptr)
		TopR = new PIAL::PI_Buff(partInfo.partImgBuf.imgTop_R, nPartWidth, nPartHeight, nPartWidth);

	if (partInfo.partImgBuf.imgTop_G != nullptr)
		TopG = new PIAL::PI_Buff(partInfo.partImgBuf.imgTop_G, nPartWidth, nPartHeight, nPartWidth);

	if (partInfo.partImgBuf.imgTop_B != nullptr)
		TopB = new PIAL::PI_Buff(partInfo.partImgBuf.imgTop_B, nPartWidth, nPartHeight, nPartWidth);

	if (partInfo.partImgBuf.imgTop_W != nullptr)
		TopW = new PIAL::PI_Buff(partInfo.partImgBuf.imgTop_W, nPartWidth, nPartHeight, nPartWidth);
#pragma endregion

#pragma region Middle
	if (partInfo.partImgBuf.imgMiddle_R != nullptr)
		MiddleR = new PIAL::PI_Buff(partInfo.partImgBuf.imgMiddle_R, nPartWidth, nPartHeight, nPartWidth);

	if (partInfo.partImgBuf.imgMiddle_B != nullptr)
		MiddleB = new PIAL::PI_Buff(partInfo.partImgBuf.imgMiddle_B, nPartWidth, nPartHeight, nPartWidth);
#pragma endregion

#pragma region Bottom
	if (partInfo.partImgBuf.imgBottom_R != nullptr)
		BottomR = new PIAL::PI_Buff(partInfo.partImgBuf.imgBottom_R, nPartWidth, nPartHeight, nPartWidth);

	if (partInfo.partImgBuf.imgBottom_B != nullptr)
		BottomB = new PIAL::PI_Buff(partInfo.partImgBuf.imgBottom_B, nPartWidth, nPartHeight, nPartWidth);
#pragma endregion

#pragma region Side1
	if (partInfo.partImgBuf.imgSide1_R != nullptr)
		Side1R = new PIAL::PI_Buff(partInfo.partImgBuf.imgSide1_R, nPartWidth, nPartHeight, nPartWidth);

	if (partInfo.partImgBuf.imgSide1_G != nullptr)
		Side1G = new PIAL::PI_Buff(partInfo.partImgBuf.imgSide1_G, nPartWidth, nPartHeight, nPartWidth);

	if (partInfo.partImgBuf.imgSide1_B != nullptr)
		Side1B = new PIAL::PI_Buff(partInfo.partImgBuf.imgSide1_B, nPartWidth, nPartHeight, nPartWidth);
#pragma endregion

#pragma region Side2
	if (partInfo.partImgBuf.imgSide2_R != nullptr)
		Side2R = new PIAL::PI_Buff(partInfo.partImgBuf.imgSide2_R, nPartWidth, nPartHeight, nPartWidth);

	if (partInfo.partImgBuf.imgSide2_G != nullptr)
		Side2G = new PIAL::PI_Buff(partInfo.partImgBuf.imgSide2_G, nPartWidth, nPartHeight, nPartWidth);

	if (partInfo.partImgBuf.imgSide2_B != nullptr)
		Side2B = new PIAL::PI_Buff(partInfo.partImgBuf.imgSide2_B, nPartWidth, nPartHeight, nPartWidth);
#pragma endregion

#pragma region Side3
	if (partInfo.partImgBuf.imgSide3_R != nullptr)
		Side3R = new PIAL::PI_Buff(partInfo.partImgBuf.imgSide3_R, nPartWidth, nPartHeight, nPartWidth);

	if (partInfo.partImgBuf.imgSide3_G != nullptr)
		Side3G = new PIAL::PI_Buff(partInfo.partImgBuf.imgSide3_G, nPartWidth, nPartHeight, nPartWidth);

	if (partInfo.partImgBuf.imgSide3_B != nullptr)
		Side3B = new PIAL::PI_Buff(partInfo.partImgBuf.imgSide3_B, nPartWidth, nPartHeight, nPartWidth);
#pragma endregion

#pragma region Side4
	if (partInfo.partImgBuf.imgSide4_R != nullptr)
		Side4R = new PIAL::PI_Buff(partInfo.partImgBuf.imgSide4_R, nPartWidth, nPartHeight, nPartWidth);

	if (partInfo.partImgBuf.imgSide4_G != nullptr)
		Side4G = new PIAL::PI_Buff(partInfo.partImgBuf.imgSide4_G, nPartWidth, nPartHeight, nPartWidth);

	if (partInfo.partImgBuf.imgSide4_B != nullptr)
		Side4B = new PIAL::PI_Buff(partInfo.partImgBuf.imgSide4_B, nPartWidth, nPartHeight, nPartWidth);
#pragma endregion

	if (TopR != nullptr)		 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Top_R, TopR);
	if (TopG != nullptr)		 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Top_G, TopG);
	if (TopB != nullptr)		 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Top_B, TopB);
	if (TopW != nullptr)		 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Top_W, TopW);

	if (MiddleR != nullptr)	 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Middle_R, MiddleR);
	if (MiddleB != nullptr)	 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Middle_B, MiddleB);

	if (BottomR != nullptr)	 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Bottom_R, BottomR);
	if (BottomB != nullptr)	 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Bottom_B, BottomB);

	if (Side1R != nullptr)		 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Side1_R, Side1R);
	if (Side1G != nullptr)		 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Side1_G, Side1G);
	if (Side1B != nullptr)		 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Side1_B, Side1B);

	if (Side2R != nullptr)		 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Side2_R, Side2R);
	if (Side2G != nullptr)		 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Side2_G, Side2G);
	if (Side2B != nullptr)		 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Side2_B, Side2B);

	if (Side3R != nullptr)		 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Side3_R, Side3R);
	if (Side3G != nullptr)		 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Side3_G, Side3G);
	if (Side3B != nullptr)		 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Side3_B, Side3B);

	if (Side4R != nullptr)		 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Side4_R, Side4R);
	if (Side4G != nullptr)		 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Side4_G, Side4G);
	if (Side4B != nullptr)		 pImg_buf->inspPartImage->m_ImageBuffer->AddBuff(PIAL::Side4_B, Side4B);

}

PIAL::Img_Channel PInspAlgoWrapper::ConvertIntToChannel(int nLightType)
{
	PIAL::Img_Channel channel = PIAL::Top_R;
	switch (nLightType)
	{
	case 0:
		channel = PIAL::Bottom_B;	break;
	case 1:
		channel = PIAL::Bottom_R;	break;
	case 2:
		channel = PIAL::Middle_B;	break;
	case 3:
		channel = PIAL::Middle_R;	break;
	case 4:
		channel = PIAL::Top_B;	break;
	case 5:
		channel = PIAL::Top_R;	break;
	case 6:
		channel = PIAL::Top_G;	break;
	case 7:
		channel = PIAL::Top_W;	break;
	}
	return channel;
}

bool PInspAlgoWrapper::ConvertExceptROI(InspAlgo* org, PIAL::_tagTotalInspExceptArea* rst, float nAngle)
{
	// 통합 검사/제외 영역에 넣어준다
	rst->m_nUsedMaskingValue = org->m_nUsedMaskingValue;
	rst->m_rcArrMaskingROI.resize(org->m_nUsedMaskingValue);
	for (int i = 0; i < org->m_nUsedMaskingValue; i++)
		rst->m_rcArrMaskingROI[i] = org->m_rcArrMaskingROI[i];
	rst->m_nUsedInspPolygon = org->m_nUsedInspPolygon;
	rst->m_bConvetExceptROI = org->m_bConvetExceptROI;
	for (int i = 0; i < _MAX_INSP_AREA_COUNT; i++)
		rst->m_ptArrInspPolygon[i] = org->m_ptArrInspPolygon[i];
	rst->dAngle = nAngle;

	return 1;
}

void PInspAlgoWrapper::ConvertExceptROI(PIAL::_tagTotalInspExceptArea* exceptROI, InspPartInfo* InspBoardInfo, InspPartParam partParam, PIAL::PInspData* InspData)
{
	if (InspData == nullptr)
		return;

	double dResolX = PIAL::PInspAlgo_Lib::m_resolX;
	double dResolY = PIAL::PInspAlgo_Lib::m_resolY;

	float fPartWidth = InspBoardInfo->partWidth / dResolX;
	float fPartHeight = InspBoardInfo->partHeight / dResolY;

	float fWndWidth = partParam.width / dResolX;
	float fWndHeight = partParam.length / dResolY;

	float fWndCX = (fPartWidth / 2) + (partParam.cx / dResolX);
	float fWndCY = (fPartHeight / 2) - (partParam.cy / dResolY);

	int nWidth = RounD(fWndWidth);
	int nHeight = RounD(fWndHeight);

	//for (int i = 0; i < _MAX_MASKING_NUM; i++)
	for (int i = 0; i < exceptROI->m_nUsedMaskingValue; i++)
	{
		auto* newMaskData = new PIAL::MaskInfo();
		newMaskData->StartPos.x = fWndCX + exceptROI->m_rcArrMaskingROI[i].left;
		newMaskData->StartPos.y = fWndCY + exceptROI->m_rcArrMaskingROI[i].top;
		newMaskData->Width = exceptROI->m_rcArrMaskingROI[i].right - exceptROI->m_rcArrMaskingROI[i].left;
		newMaskData->Length = exceptROI->m_rcArrMaskingROI[i].bottom - exceptROI->m_rcArrMaskingROI[i].top;

		if (newMaskData->Width <= 0 || newMaskData->Length <= 0)
		{
			delete newMaskData;
			continue;
		}

		InspData->SetMaskData(newMaskData);
	}
}

bool PInspAlgoWrapper::ConvertWindowExceptROI(InspPartParam* partParam, int nWndindex, PIAL::_tagTotalInspExceptArea* rst)
{
	if (rst == nullptr) return false;

	rst->m_nUsedWndPolygon = partParam[nWndindex].m_nUsedWndPolygon;
	for (int i = 0; i < MAX_INSP_AREA_COUNT; i++)
		rst->m_ptArrWndPolygon[i] = partParam[nWndindex].m_ptArrWndPolygon[i];

	return true;
}

void PInspAlgoWrapper::ConvertAlgo(AlgoBW* org, PIAL::_AlgoBlackWhite* rst)
{
	rst->m_bInvertCheck = org->m_bInvertCheck;
	rst->m_dPercentOK = org->m_dPercentOK;
	rst->m_bChipTracking = org->m_bChipTracking;
	rst->m_b2dCheck = org->m_b2dCheck;
	rst->m_nRange = org->m_nRange;
	rst->m_nMinValue = org->m_nMinValue;
	rst->m_nMaxValue = org->m_nMaxValue;
	rst->m_b3dCheck = org->m_b3dCheck;
	rst->m_n3dRange = org->m_n3dRange;
	rst->m_d3dHeightMin = org->m_d3dHeightMin;
	rst->m_d3dHeightMax = org->m_d3dHeightMax;
	rst->m_bUseTeachingRate = org->m_bUseTeachingRate;
	rst->m_dAreaCurrent = org->m_dAreaCurrent;
	rst->m_dTeachingArea = org->m_dTeachingArea;
	rst->m_nStdOKArea = org->m_nStdOKArea;
	rst->m_nChipTrackingGap = org->m_nChipTrackingGap;
	rst->m_d3dAvgHeight = org->m_d3dAvgHeight;
	ConvertColorBase(&org->m_sAlgoColorBase, &rst->m_sAlgoColorBase);
}

void PInspAlgoWrapper::ConvertAlgo(AlgoFoot* org, PIAL::_AlgoFoot& rst)
{
	rst.m_bOffset = org->m_bOffset;
	rst.m_nUseOption = org->m_nUseOption;
	rst.m_nUseOption2 = org->m_nUseOption2;
	rst.m_nFindOption = org->m_nFindOption;
	rst.m_nFindOption2 = org->m_nFindOption2;
	rst.m_bUsePatternAngle = org->m_bUsePatternAngle;

	for (size_t i = 0; i < (int)m_eFootBin_Total; i++)
	{
		rst.m_sArrBin[i].Moph = org->m_sArrBin[i].Moph;
		rst.m_sArrBin[i].m_bIsSet = org->m_sArrBin[i].m_bIsSet;
		rst.m_sArrBin[i].m_nLightCnt = org->m_sArrBin[i].m_byArrValue[m_eBin_L_Cnt];
		int nLightCnt = rst.m_sArrBin[i].m_nLightCnt;

		if (nLightCnt == 0)
		{
			nLightCnt = 1;
		}

		//조명합성
		memcpy(rst.m_sArrBin[i].m_nArrLightPosition, org->m_sArrBin[i].m_byArrLightData[eLightData_Position], nLightCnt);
		memcpy(rst.m_sArrBin[i].m_nArrCalculation, org->m_sArrBin[i].m_byArrLightData[eLightData_Calculation], nLightCnt);
		memcpy(rst.m_sArrBin[i].m_nArrRedValue, org->m_sArrBin[i].m_byArrLightData[eLightData_Red], nLightCnt);
		memcpy(rst.m_sArrBin[i].m_nArrBlueValue, org->m_sArrBin[i].m_byArrLightData[eLightData_Blue], nLightCnt);
		memcpy(rst.m_sArrBin[i].m_nArrGreenValue, org->m_sArrBin[i].m_byArrLightData[eLightData_Green], nLightCnt);
		memcpy(rst.m_sArrBin[i].m_nArrWhiteValue, org->m_sArrBin[i].m_byArrLightData[eLightData_White], nLightCnt);

		//2D
		rst.m_sArrBin[i].m_bInsp2D = ((org->m_sArrBin[i].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Use) != 0);
		rst.m_sArrBin[i].m_nMinBinary = org->m_sArrBin[i].m_byArrValue[(int)m_eBin::m_eBin_Min2D];
		rst.m_sArrBin[i].m_nMaxBinary = org->m_sArrBin[i].m_byArrValue[(int)m_eBin::m_eBin_Max2D];
		rst.m_sArrBin[i].m_nTypeRange2D = 0;
		if ((org->m_sArrBin[i].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Out) != 0)
			rst.m_sArrBin[i].m_nTypeRange2D = 1;
		else if ((org->m_sArrBin[i].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Up) != 0)
			rst.m_sArrBin[i].m_nTypeRange2D = 2;
		else if ((org->m_sArrBin[i].m_byArrValue[(int)m_eBin::m_eBin_Data2D] & m_eBinData::m_eBinData_Range_Lo) != 0)
			rst.m_sArrBin[i].m_nTypeRange2D = 3;

		//3D
		rst.m_sArrBin[i].m_bInsp3D = ((org->m_sArrBin[i].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Use) != 0);
		rst.m_sArrBin[i].m_dHeightRateMin = org->m_sArrBin[i].m_fArrH[eMMD_Min];
		rst.m_sArrBin[i].m_dHeightRateMax = org->m_sArrBin[i].m_fArrH[eMMD_Max];
		rst.m_sArrBin[i].m_nTypeRange3D = 0;
		if ((org->m_sArrBin[i].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Out) != 0)
			rst.m_sArrBin[i].m_nTypeRange3D = 1;
		else if ((org->m_sArrBin[i].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Up) != 0)
			rst.m_sArrBin[i].m_nTypeRange3D = 2;
		else if ((org->m_sArrBin[i].m_byArrValue[(int)m_eBin::m_eBin_Data3D] & m_eBinData::m_eBinData_Range_Lo) != 0)
			rst.m_sArrBin[i].m_nTypeRange3D = 3;

	}

	rst.m_sPoint = org->m_sPoint;
	rst.m_rSpecRect = org->m_rSpecRect;
	memcpy(&rst.m_fArrOptionValue, &org->m_fArrOptionValue, sizeof(float)*(int)m_eFoot_Total*(int)eMMD_Total);
	rst.m_rInspRect = org->m_rInspRect;

	memcpy(&rst.m_nFindOptionOrder, &org->m_nFindOptionOrder, sizeof(int)*(int)m_eFoot_FindOption::total);
	memcpy(&rst.m_nFindOptionOrder2, &org->m_nFindOptionOrder2, sizeof(int)*(int)m_eFoot_FindOption::total);
	rst.m_nFootType = org->m_nFootType;
	rst.m_nFootAngle = org->m_nFootAngle;
	rst.m_fPadAreaAvgHeight = org->m_fPadAreaAvgHeight;
}

void PInspAlgoWrapper::ConvertAlgo(AlgoWire* org, PIAL::_AlgoWire& rst)
{
	rst.m_byFootCnt = org->m_byFootCnt;
	memcpy(rst.m_nArrData, org->m_nArrData, sizeof(int)* eWIRE2_Total);
	rst.m_sWireRect = org->m_sWireRect;
	memcpy(rst.m_sArrPoint, org->m_sArrPoint, sizeof(POINTF)* WIRE_DRAW_CNT);
	rst.m_fArrDistance = org->m_fArrDistance;
	memcpy(rst.m_fArrOptionValue, org->m_fArrOptionValue, sizeof(float)* eWIRE_Total*eMMD_Total);
	rst.m_nWireRefAreaOpt = org->m_nWireRefAreaOpt;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoFoot rst2, RstAlgoFoot* rst)
{
	rst->m_bOK = rst2.m_bOK;
	rst->m_nFindBin = rst2.m_nFindBin;
	rst->m_bFind = rst2.m_bFind;

	memcpy(rst->m_bArrOK, rst2.m_bArrOK, sizeof(BOOL)* m_eFoot_Total);
	memcpy(rst->m_fArrRst, rst2.m_fArrRst, sizeof(float)* m_eFoot_Total);
	rst->m_poWire_Wnd = rst2.m_poWire_Wnd;
	rst->m_rcRefArea = rst2.m_rcRefArea;
	rst->m_rcSearchArea = rst2.m_rcSearchArea;
	memcpy(rst->rePoinSt, rst2.rePoinSt, sizeof(POINT)* m_eFoot_Total);
	memcpy(rst->rePoinEd, rst2.rePoinEd, sizeof(POINT)* m_eFoot_Total);
	rst->m_nMatchModelNum = rst2.m_nMatchModelNum;
	rst->m_sPoint = rst2.m_sPoint;
	rst->m_fAreaHeight = rst2.m_fAreaHeight;
	rst->m_fAngle = rst2.m_fAngle;
	rst->m_sEnd = rst2.m_sEnd;
	memcpy(rst->m_fArrRst_WingArea, rst2.m_fArrRst_WingArea, sizeof(float) * 4);
	rst->m_stWingFunc = rst2.m_stWingFunc;
	rst->m_edWingFunc = rst2.m_edWingFunc;
	rst->m_nTeachRectLT = rst2.m_nTeachRectLT;
	rst->m_nTeachRectLB = rst2.m_nTeachRectLB;
	rst->m_nTeachRectRB = rst2.m_nTeachRectRB;
	rst->m_nTeachRectRT = rst2.m_nTeachRectRT;;
	rst->MatchingScore = rst2.MatchingScore;
	memcpy(rst->DispPoinSt, rst2.DispPoinSt, sizeof(POINT)* m_eFoot_Total);
	memcpy(rst->DispPoinEd, rst2.DispPoinEd, sizeof(POINT)* m_eFoot_Total);

	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoWire rst2, RstAlgoWire* rst)
{
	rst->m_nWireCnt = rst2.m_nWireCnt;
	rst->m_bOK = rst2.m_bOK;
	memcpy(rst->m_bArrOK, rst2.m_bArrOK, sizeof(BOOL)* eWIRE_Total);
	rst->m_bArrNGType = rst2.m_bArrNGType;
	rst->m_nArrCnt = rst2.m_nArrCnt;
	memcpy(rst->m_fArrRst, rst2.m_fArrRst, sizeof(float)* eWIRE_Total);
	memcpy(rst->m_fArrValue_H, rst2.m_fArrValue_H, sizeof(float)* WIRE_DOT_CNT);
	memcpy(rst->m_fArrValue_W, rst2.m_fArrValue_W, sizeof(float)* WIRE_DOT_CNT);
	memcpy(rst->m_fArrValue_H2, rst2.m_fArrValue_H2, sizeof(float)* WIRE_DOT_CNT);
	memcpy(rst->m_fArrValue_T, rst2.m_fArrValue_T, sizeof(float)* WIRE_DOT3_CNT);
	memcpy(rst->m_fArrValue_I, rst2.m_fArrValue_I, sizeof(float)* WIRE_DOT3_CNT);
	memcpy(rst->m_nArrX, rst2.m_nArrX, sizeof(int)* eDot_Total);
	memcpy(rst->m_nArrY, rst2.m_nArrY, sizeof(int)* eDot_Total);
	memcpy(rst->m_nArrX_T, rst2.m_nArrX_T, sizeof(int)* WIRE_DRAW_CNT);
	memcpy(rst->m_nArrY_T, rst2.m_nArrY_T, sizeof(int)* WIRE_DRAW_CNT);
	memcpy(rst->m_nArrX_I, rst2.m_nArrX_I, sizeof(int)* WIRE_DRAW_CNT);
	memcpy(rst->m_nArrY_I, rst2.m_nArrY_I, sizeof(int)* WIRE_DRAW_CNT);
	rst->m_fRefArea_H = rst2.m_fRefArea_H;

	rst->m_poWire_Wnd = rst2.m_poWire_Wnd;
	rst->m_poWire_TS = rst2.m_poWire_TS;
	rst->m_poWire_TE = rst2.m_poWire_TE;
	rst->m_poWire_RS = rst2.m_poWire_RS;
	rst->m_poWire_RE = rst2.m_poWire_RE;
	rst->m_rcWire = rst2.m_rcWire;
	rst->m_bArrHo = rst2.m_bArrHo;
	rst->m_poArrAB = rst2.m_poArrAB;
	rst->m_nBinCenterCnt = rst2.m_nBinCenterCnt;

	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoGrayMean rst2, RstAlgoGrayMean* rst)
{
	rst->m_dRstGrayMean = rst2.m_dRstGrayMean;
	rst->m_bRstStdChanged = rst2.m_bRstStdChanged;
	rst->m_nStdGrayMin = rst2.m_nStdGrayMin;
	rst->m_nStdGrayMax = rst2.m_nStdGrayMax;
	rst->m_bUseUV = rst2.m_bUseUV;
	rst->m_nRstStdGrayMin = rst2.m_nRstStdGrayMin;
	rst->m_nRstStdGrayMax = rst2.m_nRstStdGrayMax;

	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoHeightMean rst2, RstAlgoHeightMean* rst)
{
	rst->m_dRstHeightMean = rst2.m_dRstHeightMean;
	rst->m_dRstHeighestValue = rst2.m_dRstHeighestValue;
	rst->m_bOKHeightMean = rst2.m_bOKHeightMean;
	rst->m_bOKHeighest = rst2.m_bOKHeighest;
	rst->m_bNoSolder = rst2.m_bNoSolder;
	rst->m_bExcess = rst2.m_bExcess;

	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoGrayDiff rst2, RstAlgoGrayDiff* rst)
{
	rst->m_nRstGrayDiff = rst2.m_nRstGrayDiff;
	rst->m_bCheckPolarity = rst2.m_bCheckPolarity;
	rst->m_rcRect_I[0] = rst2.m_rcRect_I[0];
	rst->m_rcRect_I[1] = rst2.m_rcRect_I[1];

	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoHeightDiff rst2, RstAlgoHeightDiff* rst)
{
	rst->m_dRst_1 = rst2.m_dRst_1;
	rst->m_dRst_2 = rst2.m_dRst_2;
	rst->m_dRstHeightDiff = rst2.m_dRstHeightDiff;
	rst->m_bCheckPolarity = rst2.m_bCheckPolarity;
	rst->m_rcRect_I[0] = rst2.m_rcRect_I[0];
	rst->m_rcRect_I[1] = rst2.m_rcRect_I[1];
	rst->m_bOK;

	for (int i = 0; i < 4; i++)
		rst->m_rcRect_I[i] = rst2.m_rcRect_I[i];

	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoPadBW rst2, RstAlgoPadBW* rst)
{
	memset(rst, 0, sizeof(RstAlgoPadBW));

	rst->m_bOKShapeArea = rst2.m_bOKShapeArea;
	rst->m_bOKShapeShiftX = rst2.m_bOKShapeShiftX;
	rst->m_bOKShapeShiftY = rst2.m_bOKShapeShiftY;

	memcpy(rst->m_fArrRstShapeArea, rst2.m_fArrRstShapeArea, MAX_SHAPEDEFECT_CNT * sizeof(float));
	memcpy(rst->m_fArrRstShapeAreaRate, rst2.m_fArrRstShapeAreaRate, MAX_SHAPEDEFECT_CNT * sizeof(float));
	memcpy(rst->m_fArrRstShiftX, rst2.m_fArrRstShiftX, MAX_SHAPEDEFECT_CNT * sizeof(float));
	memcpy(rst->m_fArrRstShiftY, rst2.m_fArrRstShiftY, MAX_SHAPEDEFECT_CNT * sizeof(float));

	memcpy(rst->m_rcArrShapeRect_I, rst2.m_rcArrShapeRect_I, MAX_SHAPEDEFECT_CNT * sizeof(RECT));
	memcpy(rst->m_bArrShapeOK, rst2.m_bArrShapeOK, MAX_SHAPEDEFECT_CNT * sizeof(BOOL));
	rst->m_nArrShapeRectCnt = rst2.m_nArrShapeRectCnt;

 	memcpy(rst->m_fArrRstArea, rst2.m_fArrRstArea, MAX_DEFECT_CNT * sizeof(float));
 	memcpy(rst->m_fArrRstWidth, rst2.m_fArrRstWidth, MAX_DEFECT_CNT * sizeof(float));
 	memcpy(rst->m_fArrRstLength, rst2.m_fArrRstLength, MAX_DEFECT_CNT * sizeof(float));
 
	rst->m_bOKArea = rst2.m_bOKArea;
 	rst->m_bOKWidth = rst2.m_bOKWidth;
 	rst->m_bOKLength = rst2.m_bOKLength;

	rst->m_nArrRectCnt = rst2.m_nArrRectCnt;

 	memcpy(rst->m_rcArrRect_I, rst2.m_rcArrRect_I, MAX_DEFECT_CNT * sizeof(RECT));
	rst->m_bMaskLoadSuccess = rst2.m_bMaskLoadSuccess;
	rst->Option3DRangeOK = rst2.Option3DRangeOK;
	rst->m_dRstOption3DRange = rst2.m_dRstOption3DRange;
	rst->m_nArrOption3DRangeIdx = rst2.m_nArrOption3DRangeIdx;

	rst->Option3DMinMaxOK = rst2.Option3DMinMaxOK;
	rst->m_fRsOption3DMin = rst2.m_fRsOption3DMin;
	rst->m_fRsOption3DMax = rst2.m_fRsOption3DMax;
	rst->m_nArrHeightMinMaxIdx = rst2.m_nArrHeightMinMaxIdx;

	memcpy(rst->m_nArrRstHeightMin, rst2.m_nArrRstHeightMin, MAX_DEFECT_CNT * sizeof(float));
	memcpy(rst->m_nArrRstHeightMax, rst2.m_nArrRstHeightMax, MAX_DEFECT_CNT * sizeof(float));
	rst->m_dAlignResultTheta = rst2.m_dAlignResultTheta;

	memcpy(rst->m_fArrRstRelativeHeightMin, rst2.m_fArrRstRelativeHeightMin, MAX_DEFECT_CNT * sizeof(float));
	memcpy(rst->m_fArrRstRelativeHeightMax, rst2.m_fArrRstRelativeHeightMax, MAX_DEFECT_CNT * sizeof(float));
	memcpy(rst->m_fArrRstContrast, rst2.m_fArrRstContrast, MAX_DEFECT_CNT * sizeof(float));

 	return true;
 }

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoBodyBlob rst2, RstAlgoBodyBlob* rst)
{
	memset(rst, 0, sizeof(RstAlgoBodyBlob));	
	rst->m_dRstWidth = rst2.m_dRstWidth;
	rst->m_dRstLength = rst2.m_dRstLength;
	rst->m_dRstShiftX = rst2.m_dRstShiftX;
	rst->m_dRstShiftY = rst2.m_dRstShiftY;
	rst->m_fRstShiftMaxX = rst2.m_fRstShiftMaxX;
	rst->m_fRstShiftMaxY = rst2.m_fRstShiftMaxY;
	rst->m_dRstCenterX = rst2.m_dRstCenterX;
	rst->m_dRstCenterY = rst2.m_dRstCenterY;
	rst->m_dRstAngle = rst2.m_dRstAngle;
	rst->m_dRstArea = rst2.m_dRstArea;
	rst->m_dRstAreaRate = rst2.m_dRstAreaRate;
	rst->m_dRstHeightMean = rst2.m_dRstHeightMean;
	rst->m_dRstDamageRate = rst2.m_dRstDamageRate;
	rst->m_dRstDamageConditionRate = rst2.m_dRstDamageConditionRate;
	rst->m_dRstDamageRate_Per = rst2.m_dRstDamageRate_Per;
	rst->m_nShapeAreaCnt = rst2.m_nShapeAreaCnt;
	memcpy(rst->m_dRstShapeArea, rst2.m_dRstShapeArea, sizeof(double)*_dfBodyBlobShapeAreaCnt);
	memcpy(rst->m_dRstShapeArea_Per, rst2.m_dRstShapeArea_Per, sizeof(double)*_dfBodyBlobShapeAreaCnt);
	rst->m_dRstHeightMean_Per = rst2.m_dRstHeightMean_Per;

	rst->m_bOKWidth = rst2.m_bOKWidth;
	rst->m_bOKLength = rst2.m_bOKLength;
	rst->m_bOKShiftX = rst2.m_bOKShiftX;
	rst->m_bOKShiftY = rst2.m_bOKShiftY;
	rst->m_bOKShiftMaxX = rst2.m_bOKShiftMaxX;
	rst->m_bOKShiftMaxY = rst2.m_bOKShiftMaxY;
	rst->m_bOKAngle = rst2.m_bOKAngle;
	rst->m_bOKArea = rst2.m_bOKArea;
	rst->m_bOKHeight = rst2.m_bOKHeight;
	rst->m_bBillboarding = rst2.m_bBillboarding;
	rst->m_bTombstone = rst2.m_bTombstone;
	rst->m_bOKDamage = rst2.m_bOKDamage;
	rst->m_bOKShape = rst2.m_bOKShape;
	rst->m_bShowRect = rst2.m_bShowRect;

	rst->m_rcBodyRect = rst2.m_rcBodyRect;
	rst->m_rcInspBodyRect = rst2.m_rcInspBodyRect;
	rst->m_rcBodyDamageRect = rst2.m_rcBodyDamageRect;
	rst->m_rcRect_T = rst2.m_rcRect_T;
	memcpy(rst->m_rcBodyShapeRect, rst2.m_rcBodyShapeRect, sizeof(RECT)*_dfBodyBlobShapeAreaCnt);

	rst->m_poDrawCenter = rst2.m_poDrawCenter;

	rst->m_bOKOffsetDistance = rst2.m_bOKOffsetDistance;
	rst->m_fOffsetDistance = rst2.m_fOffsetDistance;
	return true;
}
bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoPatternDiff rst2, RstAlgoPatternDiff* rst)
{
	rst->m_bOK = rst2.m_bOK;

	int nDefectCnt = rst2.m_nRectCnt > BLOB_RECT_CNTS ? BLOB_RECT_CNTS : rst2.m_nRectCnt;

	rst->m_nRectCnt = nDefectCnt;

	memcpy(rst->m_bArrRstLayer, rst2.m_bArrRstLayer, MAX_LAYERCNT * sizeof(BOOL));
	memcpy(rst->m_nArrDefectLayerNum, rst2.m_nArrDefectLayerNum, BLOB_RECT_CNTS * sizeof(int));
	memcpy(rst->m_dRstLength, rst2.m_dRstLength, BLOB_RECT_CNTS * sizeof(double));
	memcpy(rst->m_dRstWidth, rst2.m_dRstWidth, BLOB_RECT_CNTS * sizeof(double));
	memcpy(rst->m_dRstArea, rst2.m_dRstArea, BLOB_RECT_CNTS * sizeof(double));
	memcpy(rst->m_dRstPatternScore, rst2.m_dRstPatternScore, BLOB_RECT_CNTS * sizeof(double));

	rst->m_nSelectedModelIdx = rst2.m_nSelectedModelIdx;
	rst->m_bRstChipping = rst2.m_bRstChipping;

	rst->m_ptRstShiftXY.x = rst2.m_ptRstShiftXY.x;
	rst->m_ptRstShiftXY.y = rst2.m_ptRstShiftXY.y;
	rst->m_dTheta = rst2.m_dTheta;


	for (int i = 0; i < nDefectCnt; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			rst->m_ptArrRstPtr[i][j].x = rst2.m_ptArrRstPtr[i][j].x;
			rst->m_ptArrRstPtr[i][j].y = rst2.m_ptArrRstPtr[i][j].y;
		}
	}
	rst->m_ptRstMatchingCenter.x = rst2.m_ptRstMatchingCenter.x;
	rst->m_ptRstMatchingCenter.y = rst2.m_ptRstMatchingCenter.y;

	rst->m_dAlignScore = rst2.m_dAlignScore;
	memcpy(rst->m_dMatchingScore, rst2.m_dMatchingScore, MAX_ALIGNCNT * sizeof(double));
	rst->m_bMatchingOK = rst2.m_bMatchingOK;

	return true;
}

bool PInspAlgoWrapper::WindowRotateState()
{
	return PIAL::PInspAlgo_Lib::m_WindowRotate;
}
void PInspAlgoWrapper::SetWindowRotateState(bool value)
{
	PIAL::PInspAlgo_Lib::m_WindowRotate = value;
}

bool PInspAlgoWrapper::ConvertRstAlgo(AlignResult* org, PIAL::_AlignResult* dst)
{
	if (org == nullptr)
		return false;
	dst->centerX = org->centerX;
	dst->centerY = org->centerY;
	dst->nWindowID = org->nWindowID;
	dst->offsetX = org->offsetX;
	dst->offsetY = org->offsetY;
	memcpy(dst->rcBodyRect, org->rcBodyRect, sizeof(CRect));
	dst->TeachCenterX = org->TeachCenterX;
	dst->TeachCenterY = org->TeachCenterY;
	dst->theta = org->theta;
	return true;
}

bool PInspAlgoWrapper::ConvertAlignResult(AlignResult* org, int nNum, std::vector<PIAL::_AlignResult>& vec)
{
	if (org == nullptr || nNum == 0) return false;

	for (size_t i = 0; i < nNum; i++)
	{
		PIAL::_AlignResult dst;

		dst.centerX = org[i].centerX;
		dst.centerY = org[i].centerY;
		dst.nWindowID = org[i].nWindowID;
		dst.offsetX = org[i].offsetX;
		dst.offsetY = org[i].offsetY;
		memcpy(dst.rcBodyRect, org[i].rcBodyRect, sizeof(CRect));
		dst.TeachCenterX = org[i].TeachCenterX;
		dst.TeachCenterY = org[i].TeachCenterY;
		dst.theta = org[i].theta;
		vec.emplace_back(dst);
	}

	return true;
}

/*bool PInspAlgoWrapper::InspWireShort(int nWireNum, WireRst* Wirerst, UCHAR *ucArrDstImg, int nWidth, int nlength)
{

	std::vector<PIAL::WireInterval*> pwires;
	pwires.reserve(nWireNum);
	PIAL::PI_Buff pBuff(ucArrDstImg, nWidth, nlength, nWidth);

	for (size_t i = 0; i < nWireNum; i++)
	{
		AlgoWire algoWire = *Wirerst[i].m_Wire;
		RstAlgoWire *sRstAlgo = Wirerst[i].m_RstWire;

		if (sRstAlgo == nullptr) continue;

		PIAL::WireInterval* pWire = new PIAL::WireInterval;

		pWire->m_bUse = ((algoWire.m_nArrData[eWIRE2_UseOption] & eWIRE_Data_Interval) == eWIRE_Data_Interval);
		pWire->m_WireID = sRstAlgo->m_nWireCnt;
		pWire->m_nOption_DivCnt = algoWire.m_nArrData[eWIRE2_DivCnt];
		pWire->m_bUseMax = (algoWire.m_nArrData[eWIRE2_UseOption2] & m_eWIRE_Data2_Max) == m_eWIRE_Data2_Max;

		pWire->m_bArrHo = sRstAlgo->m_bArrHo;
		pWire->m_poArrAB = sRstAlgo->m_poArrAB;

		pWire->m_fArrValue_I = sRstAlgo->m_fArrValue_I;  //Pointer만 참조
		pWire->m_nArrX_I = sRstAlgo->m_nArrX_I; //위치: Pointer만 참조  
		pWire->m_nArrY_I = sRstAlgo->m_nArrY_I; //위치: Pointer만 참조
		pWire->m_fArrRst = &sRstAlgo->m_fArrRst[eWIRE_Interval]; //결과: Pointer만 참조

		pwires.push_back(pWire);
	}

	m_PInspWire->InspInterval(pwires, &pBuff);

	//Result
	for (size_t i = 0; i < nWireNum; i++)
	{
		AlgoWire* pAlgo = Wirerst[i].m_Wire;
		RstAlgoWire *sRstAlgo = Wirerst[i].m_RstWire;

		//좌표 보정
		for (size_t j = 0; j < 2; j++)
		{
			if (sRstAlgo->m_nArrX_I[j] == 0 && sRstAlgo->m_nArrY_I[j] == 0)
				continue;

			sRstAlgo->m_nArrX_I[j] -= sRstAlgo->m_poWire_Wnd.x;
			sRstAlgo->m_nArrY_I[j] -= sRstAlgo->m_poWire_Wnd.y;
		}

		if (((pAlgo->m_nArrData[eWIRE2_UseOption] & eWIRE_Interval) == 0))
		{
			sRstAlgo->m_fArrRst[eWIRE_Interval] = -1.;
			sRstAlgo->m_bArrOK[eWIRE_Interval] = TRUE;
			continue;
		}

		sRstAlgo->m_bArrOK[eWIRE_Interval] = FALSE;

		if (sRstAlgo->m_nWireCnt == 0)
			continue;

		if (sRstAlgo->m_fArrRst[eWIRE_Interval] == nWidth * nlength)
		{
			//맨 오른쪽 Wire 검사시 
			sRstAlgo->m_fArrRst[eWIRE_Interval] = -1.;
			sRstAlgo->m_bArrOK[eWIRE_Interval] = TRUE;
			continue;
		}

		float fValue = sRstAlgo->m_fArrRst[eWIRE_Interval];
		if (fValue < 0) fValue *= -1;

		if (fValue >= pAlgo->m_fArrOptionValue[eWIRE_Interval][eMMD_Default])
			sRstAlgo->m_bArrOK[eWIRE_Interval] = TRUE;
		else
		{
			sRstAlgo->m_bOK = FALSE;
			sRstAlgo->m_bArrOK[eWIRE_Interval] = FALSE;
		}
	}

	for (size_t i = 0; i < pwires.size(); i++)
	{
		delete pwires[i];
	}
	pwires.clear();

	return true;
}*/

void PInspAlgoWrapper::ConvertAlgo(AlgoGrayMean* org, PIAL::_AlgoGrayMean& rst)
{
	memset(&rst, 0, sizeof(PIAL::_AlgoGrayMean));

	rst.m_fGrayAvg = org->m_fGrayAvg;
	rst.m_nGrayRateMax = org->m_nGrayRateMax;
	rst.m_nGrayRateMin = org->m_nGrayRateMin;
	ConvertBlobBase(&org->m_sBlobBase, &rst.m_sBlobBase);
}

bool PInspAlgoWrapper::ConvertBlobBase(tagAlgoBlobBase* org, PIAL::_tagAlgoBlobBase* rst)
{
	memset(rst->m_nArrValue, 0, _m_enBlobBase_Total * sizeof(int));
	memset(rst->m_fArrValue, 0, _m_efBlobBase_Total * sizeof(float));

	for (int i = 0; i < _m_enBlobBase_Total; i++)
		rst->m_nArrValue[i] = org->m_nArrValue[i];
	for (int i = 0; i < _m_efBlobBase_Total; i++)
		rst->m_fArrValue[i] = org->m_fArrValue[i];
	ConvertColorBase(&org->m_sAlgoColorBase, &rst->m_sAlgoColorBase);

	return true;
}

void PInspAlgoWrapper::ConvertAlgo(AlgoHeightMean* org, PIAL::_AlgoHeightMean& rst)
{
	memset(&rst, 0, sizeof(PIAL::_AlgoHeightMean));

	rst.m_bHeightUse = org->m_bHeightUse;
	rst.dHeightMin3D = org->dHeightMin3D;
	rst.dHeightMax3D = org->dHeightMax3D;
	rst.dAvgHeight3D = org->dAvgHeight3D;
	rst.m_bUseHighest = org->m_bUseHighest;
	rst.m_dHighestMin = org->m_dHighestMin;
	rst.m_dHighestMax = org->m_dHighestMax;
	rst.m_dHighestValue = org->m_dHighestValue;
	rst.m_bBWOption = org->m_bBWOption;
	rst.m_bUseHeightMin3D = org->m_bUseHeightMin3D;
	rst.m_bUseHeightMax3D = org->m_bUseHeightMax3D;
	rst.m_bUseSolderHighest = org->m_bUseSolderHighest;
	rst.m_fAddHeight = org->m_fAddHeight;
	rst.m_fAddHighest = org->m_fAddHighest;

	rst.m_b2dCheck = org->m_b2dCheck;
	rst.m_b3dCheck = org->m_b3dCheck;
	rst.m_bChipTracking = org->m_bChipTracking;
	rst.m_bInvertCheck = org->m_bInvertCheck;
	rst.m_d3dAvgHeight = org->m_d3dAvgHeight;
	rst.m_d3dHeightMax = org->m_d3dHeightMax;
	rst.m_d3dHeightMin = org->m_d3dHeightMin;
	rst.m_dPercentOK = org->m_dPercentOK;
	rst.m_n3dRange = org->m_n3dRange;
	rst.m_nMaxValue = org->m_nMaxValue;
	rst.m_nMinValue = org->m_nMinValue;
	rst.m_nRange = org->m_nRange;
	ConvertBlobBase(&org->m_sBlobHighestBase, &rst.m_sBlobBase);
}

void PInspAlgoWrapper::ConvertAlgo(AlgoGrayDiff* org, PIAL::_AlgoGrayDiff& rst)
{
	memset(&rst, 0, sizeof(PIAL::_AlgoGrayDiff));

	rst.m_bDiffLower = org->m_bDiffLower;
	rst.m_bDiffUpper = org->m_bDiffUpper;
	rst.m_bPolarity = org->m_bPolarity;
	rst.m_bSignInversion = org->m_bSignInversion;
	rst.nGrayDiff = org->nGrayDiff;
	rst.rcFirstROI = org->rcFirstROI;
	rst.rcSeccondROI = org->rcSeccondROI;
}

void PInspAlgoWrapper::ConvertAlgo(AlgoPadBW* org, PIAL::_AlgoPadBW& rst)
{
	memset(&rst, 0, sizeof(PIAL::_AlgoPadBW));

	rst.m_nTotLightCnt = org->m_nTotLightCnt;
	memcpy(&rst.sDefaultPad, &org->sDefaultPad, sizeof(PIAL::_PadBin));
	memcpy(&rst.sArrInspPad, &org->sArrInspPad, sizeof(PIAL::_stPadBin) * org->m_nTotLightCnt);

	rst.bTeachAreaUse = org->bTeachAreaUse;
	rst.dTeachArea = org->dTeachArea;
	rst.dTeachAreaRateMin = org->dTeachAreaRateMin;
	rst.dTeachAreaRateMax = org->dTeachAreaRateMax;

	rst.bUseShift = org->bUseShift;
	rst.dTeachShiftX = org->dTeachShiftX;
	rst.dTeachShiftY = org->dTeachShiftY;

	memcpy(&rst.strGBMaskName, &org->strGBMaskName, _MAX_STRLEN);

	rst.bUseBlobWidth = org->bUseBlobWidth;
	rst.dBlobSizeWidth = org->dBlobSizeWidth;
	rst.bUseBlobLength = org->bUseBlobLength;
	rst.dBlobSizeLength = org->dBlobSizeLength;
	rst.bUseBlobArea = org->bUseBlobArea;
	rst.dBlobArea = org->dBlobArea;
	rst.nMaskShape = org->nMaskShape;

	rst.nFilterLevel = org->nFilterLevel;


	rst.nOrder = org->nOrder;

	rst.UseOption3DMinMax = org->UseOption3DMinMax;
	rst.Option3DMin = org->Option3DMin;
	rst.Option3DMax = org->Option3DMax;
	rst.Option3DThickMin = org->Option3DThickMin;

	rst.UseOption3DRange = org->UseOption3DRange;
	rst.fOption3DRange = org->fOption3DRange;
	rst.nSelectBlobType = org->nSelectBlobType;
	rst.nSelectBlobNum = org->nSelectBlobNum;

	rst.UseOptionRelativeHeight = org->UseOptionRelativeHeight;
	rst.fRelativeHeight_Min = org->fRelativeHeight_Min;
	rst.fRelativeHeight_Max = org->fRelativeHeight_Max;

	rst.UseShadeFix = org->UseShadeFix;
	rst.bUseBlobAnd = org->bUseBlobAnd;
	rst.bUseContrastGV = org->bUseContrastGV;
	rst.nContrastGV = org->nContrastGV;

	rst.bUseNGGrouping = org->bUseNGGrouping;
	rst.fNGGroupingMaxSize = org->fNGGroupingMaxSize;
	rst.fNGGroupingDistance = org->fNGGroupingDistance;

	rst.buseDIrection = org->buseDIrection;
	rst.nDirection = org->nDirection;
	rst.dDIrectionLength = org->dDIrectionLength;

	rst.bUseHoleAlign = org->bUseHoleAlign;
	rst.bUseWarningWidth = org->bUseWarningWidth;
	rst.dWarningWidth = org->dWarningWidth;
	rst.bUseWarningLength = org->bUseWarningLength;
	rst.dWarningLength = org->dWarningLength;
	rst.bUseWarningArea = org->bUseWarningArea;
	rst.dWarningArea = org->dWarningArea;
	rst.nWarningCount = org->nWarningCount;
	rst.bWarningAnd = org->bWarningAnd;

	rst.bUseImgAnd = org->bUseImgAnd;
	rst.bUseAI_Segmentation = org->bUseAI_Segmentation;
	rst.nMaskCurrentFilterSize = org->nMaskCurrentFilterSize;
	rst.nPrevMasktype = org->nPrevMasktype;
	rst.nPrevFilterSize = org->nPrevFilterSize;
	rst.nForeignImgType = org->nForeignImgType;
	rst.nPrevDefaultAlgoID = org->nPrevDefaultAlgoID;

	rst.m_bUseBlobSizeWidthNCritical = org->m_bUseBlobSizeWidthNCritical;
	rst.m_dBlobSizeWidthNCritical = org->m_dBlobSizeWidthNCritical;
	rst.m_bUseBlobSizeLengthNCritical = org->m_bUseBlobSizeLengthNCritical;
	rst.m_dBlobSizeLengthNCritical = org->m_dBlobSizeLengthNCritical;
	rst.m_bUseBlobSizeAreaNCritical = org->m_bUseBlobSizeAreaNCritical;
	rst.m_dBlobSizeAreaNCritical = org->m_dBlobSizeAreaNCritical;
	memcpy(&rst.stBlobLabelInfo, &org->stBlobLabelInfo, sizeof(PIAL::_tagBlobLabelInfo));

}
void PInspAlgoWrapper::ConvertAlgo(AlgoHeightDiff* org, PIAL::_AlgoHeightDiff& rst)
{
	memset(&rst, 0, sizeof(PIAL::_AlgoHeightDiff));

	rst.dHeightDiff3D = org->dHeightDiff3D;
	rst.dHeightDiff3DMax = org->dHeightDiff3DMax;
	rst.m_bDiffLower = org->m_bDiffLower;
	rst.m_bDiffUpper = org->m_bDiffUpper;
	rst.m_bPolarity = org->m_bPolarity;
	rst.m_bSignInversion = org->m_bSignInversion;
	rst.m_fAddHeight = org->m_fAddHeight;
	rst.m_nMinMaxflag = org->m_nMinMaxflag;
	rst.rcFirstROI = org->rcFirstROI;
	rst.rcSeccondROI = org->rcSeccondROI;
	ConvertBlobBase(&org->m_sBlobBase, &rst.m_sBlobBase);
	rst.m_bUseErodeFilter = org->m_bUseErodeFilter ;
	rst.m_nErodeFilter = org->m_nErodeFilter;
}
bool PInspAlgoWrapper::ConvertAlgo(AlgoBodyBlob* algo, PIAL::_AlgoBodyBlob& algo2)
{
	memset(&algo2, 0, sizeof(PIAL::_AlgoBodyBlob));
	algo2.Passive = FALSE; // AOI 기준
	algo2.PassiveMargin = 0.1f; // AOI 기준

	algo2.m_bUseIPC = algo->m_bUseIPC;
	algo2.m_byIPCClass = algo->m_byIPCClass;
	algo2.m_bInvertCheck = algo->m_bInvertCheck;

	algo2.m_bFilterIsUse = algo->m_bFilterIsUse;
	algo2.m_nFilterStepNarrow = algo->m_nFilterStepNarrow;

	algo2.m_bShiftIsUse = algo->m_bShiftIsUse;
	algo2.m_bShiftXUse = algo->m_bShiftXUse;
	algo2.m_bShiftYUse = algo->m_bShiftYUse;

	algo2.m_bAreaIsUse = algo->m_bAreaIsUse;
	algo2.m_dAreaMin = algo->m_dAreaMin;
	algo2.m_dAreaMax = algo->m_dAreaMax;
	algo2.m_dAreaCurrent = algo->m_dAreaCurrent;

	algo2.m_nTypeSelectBlob = algo->m_nTypeSelectBlob;
	algo2.m_bFillHole = algo->m_bFillHole;

	algo2.m_bInsp2D = algo->m_bInsp2D;
	algo2.m_nMinBinary = algo->m_nMinBinary;
	algo2.m_nMaxBinary = algo->m_nMaxBinary;
	algo2.m_nTypeRange2D = algo->m_nTypeRange2D;

	algo2.m_bInsp3D = algo->m_bInsp3D;
	algo2.m_dHeightRateMin = algo->m_dHeightRateMin;
	algo2.m_dHeightRateMax = algo->m_dHeightRateMax;
	algo2.m_fHeightAvg = algo->m_fHeightAvg;

	algo2.m_nTypeRange3D = algo->m_nTypeRange3D;

	algo2.m_dTechCenterX = algo->m_dTechCenterX;
	algo2.m_dTechCenterY = algo->m_dTechCenterY;


	algo2.m_bTeachWidthUse = algo->m_bTeachWidthUse;
	algo2.m_dTeachWidth = algo->m_dTeachWidth;
	algo2.m_dTeachWidthRateMin = algo->m_dTeachWidthRateMin;
	algo2.m_dTeachWidthRateMax = algo->m_dTeachWidthRateMax;
	algo2.m_dTeachWidthMin = algo->m_dTeachWidth * (algo->m_dTeachWidthRateMin / 100); //AOI 기준
	algo2.m_dTeachWidthMax = algo->m_dTeachWidth * (algo->m_dTeachWidthRateMax / 100);

	algo2.m_bTeachLengthUse = algo->m_bTeachLengthUse;
	algo2.m_dTeachLength = algo->m_dTeachLength;
	algo2.m_dTeachLengthRateMin = algo->m_dTeachLengthRateMin;
	algo2.m_dTeachLengthRateMax = algo->m_dTeachLengthRateMax;
	algo2.m_dTeachLengthMin = algo->m_dTeachLength * (algo->m_dTeachLengthRateMin / 100);//AOI 기준
	algo2.m_dTeachLengthMax = algo->m_dTeachLength * (algo->m_dTeachLengthRateMax / 100); 

	algo2.m_dShiftX = algo->m_dShiftX;
	algo2.m_dShiftY = algo->m_dShiftY;

	algo2.m_bUseBlobNG = algo->m_bUseBlobNG;
	algo2.m_dBlobSizeWidth = algo->m_dBlobSizeWidth;
	algo2.m_dBlobSizeLength = algo->m_dBlobSizeLength;
	algo2.m_bUseBlobSizeWidth = algo->m_bUseBlobSizeWidth;
	algo2.m_bUseBlobSizeLength = algo->m_bUseBlobSizeLength;

	algo2.m_bUseHeight = algo->m_bUseHeight;
	algo2.m_dTeachHeight = algo->m_dTeachHeight;
	algo2.m_dTeachHeightMax = algo->m_dTeachHeightMax;
	algo2.m_dTeachHeightMin = algo->m_dTeachHeightMin;

	ConvertColorBase(&algo->m_sAlgoColorBase, &algo2.m_sAlgoColorBase);


	algo2.m_bIsModelSubImg = algo->m_bIsModelSubImg;
	algo2.m_bUsePattern = algo->m_bUsePattern;

	algo2.m_nExtractRange = algo->m_nExtractRange;
	algo2.m_dSearchMargin = algo->m_dSearchMargin;

	algo2.m_dTeachRotate = algo->m_dTeachRotate;
	algo2.m_dStandardRotate = algo->m_dStandardRotate;

	algo2.m_bUseAngle = algo->m_bUseAngle;

	algo2.m_bUseDamage = algo->m_bUseDamage;
	algo2.m_dDamageHeight = algo->m_dDamageHeight;
	algo2.m_dDamageArea = algo->m_dDamageArea;
	algo2.m_bShiftMaxUse = algo->m_bShiftMaxUse;

	// Body Tip
	algo2.m_bUseBodyTip = algo->m_bUseBodyTip;
	algo2.m_bOnlyBodyTip = algo->m_bOnlyBodyTip;
	algo2.m_bTip2dCheck = algo->m_bTip2dCheck;
	algo2.m_nTip2dRange = algo->m_nTip2dRange;
	algo2.m_nTip2dMinValue = algo->m_nTip2dMinValue;
	algo2.m_nTip2dMaxValue = algo->m_nTip2dMaxValue;
	algo2.m_bTip3dCheck = algo->m_bTip3dCheck;
	algo2.m_nTip3dRange = algo->m_nTip3dRange;
	algo2.m_dTip3dHeightMin = algo->m_dTip3dHeightMin;
	algo2.m_dTip3dHeightMax = algo->m_dTip3dHeightMax;
	algo2.m_bIsHorizon = algo->m_bIsHorizon; // true : 가로(극성 양옆), false : 세로(극성 위아래)

	algo2.m_bUseShape = algo->m_bUseShape;
	algo2.m_fTeachShapeHeightMax = algo->m_fTeachShapeHeightMax;
	algo2.m_fTeachShapeArea = algo->m_fTeachShapeArea;

	algo2.m_bUseDamage2D = algo->m_bUseDamage2D;

	// Body Per
	algo2.m_b3dPerCheck = algo->m_b3dPerCheck;
	algo2.m_n3dPerRange = algo->m_n3dPerRange;
	algo2.m_f3dPerHeightMin = algo->m_f3dPerHeightMin;
	algo2.m_f3dPerHeightMax = algo->m_f3dPerHeightMax;
	algo2.m_f3dPerHeightAvg = algo->m_f3dPerHeightAvg;

	algo2.m_bRChipWithOCR = algo->m_bRChipWithOCR;
	algo2.m_fSearchDefault = algo->m_fSearchDefault;
	algo2.m_fSearchBodyTip = algo->m_fSearchBodyTip;
	algo2.m_bNGOffset = algo->m_bNGOffset;

	algo2.m_bUseOffsetDistance = algo->m_bUseOffsetDistance;
	algo2.m_fOffsetDistance = algo->m_fOffsetDistance;
	return true;
}

bool PInspAlgoWrapper::ConvertAlgo(AlgoAlign* algo, PIAL::_AlgoAlign& algo2)
{
	memset(&algo2, 0, sizeof(PIAL::_AlgoAlign));

	algo2.m_bInvertCheck = algo->m_InvertCheck;
	// 2D
	algo2.m_bInsp2D = algo->m_bInsp2D;		// 2D 적용 여부
	algo2.m_nMinBinary = algo->m_nMinBinary;
	algo2.m_nMaxBinary = algo->m_nMaxBinary;
	algo2.m_nTypeRange2D = algo->m_nTypeRange2D;
	// 3D
	algo2.m_bInsp3D = algo->m_bInsp3D;		// 3D 적용 여부
	algo2.m_dHeightRateMin = algo->m_dHeightRateMin;
	algo2.m_dHeightRateMax = algo->m_dHeightRateMax;
	algo2.m_dHeightAvg = algo->m_dHeightAvg;
	algo2.m_nTypeRange3D = algo->m_nTypeRange3D;

	algo2.m_nSearchNum = algo->m_nSearchNum;				// Search 영역 개수
	memcpy(algo2.m_sArrSearchPoint, algo->m_sArrSearchPoint, sizeof(POINTF) * 4); // Search 위치 좌표 (Part 기준 pixel 좌표)

	memcpy(algo2.m_sArrSearchSize, algo->m_sArrSearchSize, sizeof(SIZE) * 4);		// Search 영역 사이즈 (pixel 단위)
	algo2.m_nSearchMargin = algo->m_nSearchMargin;			// Search 영역 Margin +- 적용 (pixel 단위)	

	algo2.m_bUseShift = algo->m_bUseShift;		// Shift NG 검사 유무
	algo2.m_dShiftX = algo->m_dShiftX;
	algo2.m_dShiftY = algo->m_dShiftY;
	algo2.m_bUseAngle = algo->m_bUseAngle;		// Angle NG 검사 유무
	algo2.m_dAngle = algo->m_dAngle;

	algo2.m_nMinBlobArea = algo->m_nMinBlobArea;

	ConvertColorBase(&algo->m_sAlgoColorBase, &algo2.m_sAlgoColorBase);
	algo2.m_bFillHole = algo->m_bFillHole;
	algo2.m_byInspOPT = algo->m_byInspOPT;
	algo2.m_bSameSize = algo->m_bSameSize;

	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoAlign rst2, RstAlgoAlign* rst)
{
	memset(rst, 0, sizeof(RstAlgoAlign));
	rst->m_dOffset_x = rst2.m_dOffset_x;
	rst->m_dOffset_y = rst2.m_dOffset_y;
	rst->m_dTheta = rst2.m_dTheta;
	rst->m_bOKShiftX = rst2.m_bOKShiftX;
	rst->m_bOKShiftY = rst2.m_bOKShiftY;
	rst->m_bOKAngle = rst2.m_bOKAngle;
	rst->m_rcBodyRect = rst2.m_rcBodyRect;

	memcpy(rst->m_rcRect_T, rst2.m_rcRect_T, sizeof(RECT) * 4);
	memcpy(rst->m_rcRect_I, rst2.m_rcRect_I, sizeof(RECT) * 4);

	rst->m_poDrawCenter = rst2.m_poDrawCenter;
	return true;
}

bool PInspAlgoWrapper::ConvertAlgo(AlgoEdge* algo, PIAL::_AlgoEdge& algo2)
{
	memset(&algo2, 0, sizeof(PIAL::_AlgoEdge));

	algo2.m_bUseIPC = algo->m_bUseIPC;
	algo2.m_byIPCClass = algo->m_byIPCClass;
	algo2.m_bInvertCheck = algo->m_bInvertCheck;

	algo2.m_bFilterIsUse = algo->m_bFilterIsUse;
	algo2.m_nFilterStepNarrow = algo->m_nFilterStepNarrow;

	algo2.m_bShiftIsUse = algo->m_bShiftIsUse;
	algo2.m_dShiftX = algo->m_dShiftX;
	algo2.m_dShiftY = algo->m_dShiftY;

	algo2.m_bAreaIsUse = algo->m_bAreaIsUse;
	algo2.m_dAreaMin = algo->m_dAreaMin;
	algo2.m_dAreaMax = algo->m_dAreaMax;
	algo2.m_dAreaCurrent = algo->m_dAreaCurrent;				//PercentOK

	algo2.m_nTypeSelectBlob = algo->m_nTypeSelectBlob;
	// 2D
	algo2.m_bInsp2D = algo->m_bInsp2D;
	algo2.m_nMinBinary = algo->m_nMinBinary;
	algo2.m_nMaxBinary = algo->m_nMaxBinary;
	algo2.m_nTypeRange2D = algo->m_nTypeRange2D;
	// 3D
	algo2.m_bInsp3D = algo->m_bInsp3D;
	algo2.m_dHeightRateMin = algo->m_dHeightRateMin;
	algo2.m_dHeightRateMax = algo->m_dHeightRateMax;
	algo2.m_fHeightAvg = algo->m_fHeightAvg;

	algo2.m_nTypeRange3D = algo->m_nTypeRange3D;	// in 0, out 1, upper 2, lower 3

	algo2.m_dTechCenterX = algo->m_dTechCenterX;
	algo2.m_dTechCenterY = algo->m_dTechCenterY;

	algo2.m_bTeachWidthUse = algo->m_bTeachWidthUse;
	algo2.m_dTeachWidth = algo->m_dTeachWidth;
	algo2.m_dTeachWidthRateMin = algo->m_dTeachWidthRateMin;
	algo2.m_dTeachWidthRateMax = algo->m_dTeachWidthRateMax;

	algo2.m_bTeachLengthUse = algo->m_bTeachLengthUse;
	algo2.m_dTeachLength = algo->m_dTeachLength;
	algo2.m_dTeachLengthRateMin = algo->m_dTeachLengthRateMin;
	algo2.m_dTeachLengthRateMax = algo->m_dTeachLengthRateMax;

	algo2.m_bShiftXUse = algo->m_bShiftXUse;
	algo2.m_bShiftYUse = algo->m_bShiftYUse;

	algo2.m_bUseBlobNG = algo->m_bUseBlobNG;
	algo2.m_dBlobSizeWidth = algo->m_dBlobSizeWidth;
	algo2.m_dBlobSizeLength = algo->m_dBlobSizeLength;
	algo2.m_bUseBlobSizeWidth = algo->m_bUseBlobSizeWidth;
	algo2.m_bUseBlobSizeLength = algo->m_bUseBlobSizeLength;

	algo2.m_bUseHeight = algo->m_bUseHeight;
	algo2.m_dTeachHeight = algo->m_dTeachHeight;
	algo2.m_dTeachHeightMax = algo->m_dTeachHeightMax;
	algo2.m_dTeachHeightMin = algo->m_dTeachHeightMin;

	ConvertColorBase(&algo->m_sAlgoColorBase, &algo2.m_sAlgoColorBase);

	algo2.m_nSetLineCnt = algo->m_nSetLineCnt;
	algo2.m_bGroup = algo->m_bGroup;

	memcpy(algo2.m_bArrIsHorizon, algo->m_bArrIsHorizon, sizeof(BOOL) * EdgeLineTotalCnt);
	memcpy(algo2.m_nArrMeasureDirection, algo->m_nArrMeasureDirection, sizeof(int) * EdgeLineTotalCnt);
	memcpy(algo2.m_nArrSetInspCondition, algo->m_nArrSetInspCondition, sizeof(int) * EdgeInspCnt);

	memcpy(algo2.m_poArrSetTeachCenter, algo->m_poArrSetTeachCenter, sizeof(POINTF) * EdgeLineTotalCnt);
	memcpy(algo2.m_dArrTeachLength, algo->m_dArrTeachLength, sizeof(double) * EdgeLineTotalCnt);

	algo2.m_bUseAngle = algo->m_bUseAngle;						// Angle NG 검사 유무
	algo2.m_dTeachRotate = algo->m_dTeachRotate;

	algo2.m_nLineFindType = algo->m_nLineFindType;
	algo2.m_dLineFindRate = algo->m_dLineFindRate;

	algo2.m_bDistanceX = algo->m_bDistanceX;
	algo2.m_dTeachDistanceX = algo->m_dTeachDistanceX;
	algo2.m_dTeachDistanceXRateMin = algo->m_dTeachDistanceXRateMin;
	algo2.m_dTeachDistanceXRateMax = algo->m_dTeachDistanceXRateMax;
	algo2.m_bDistanceY = algo->m_bDistanceY;
	algo2.m_dTeachDistanceY = algo->m_dTeachDistanceY;
	algo2.m_dTeachDistanceYRateMin = algo->m_dTeachDistanceYRateMin;
	algo2.m_dTeachDistanceYRateMax = algo->m_dTeachDistanceYRateMax;
	algo2.m_bFindCenter = algo->m_bFindCenter;
	algo2.m_bFillHole = algo->m_bFillHole;

	algo2.m_dTeach1_3PtDis = algo->m_dTeach1_3PtDis;
	algo2.m_dTeach1_3PtDisMin = algo->m_dTeach1_3PtDisMin;
	algo2.m_dTeach1_3PtDisMax = algo->m_dTeach1_3PtDisMax;
	algo2.m_dTeach1_3PtDisOff = algo->m_dTeach1_3PtDisOff;
	algo2.m_dTeach3_3PtDis = algo->m_dTeach3_3PtDis;
	algo2.m_dTeach3_3PtDisMin = algo->m_dTeach3_3PtDisMin;
	algo2.m_dTeach3_3PtDisMax = algo->m_dTeach3_3PtDisMax;
	algo2.m_dTeach3_3PtDisOff = algo->m_dTeach3_3PtDisOff;

	algo2.m_bUseCrossCenter = algo->m_bUseCrossCenter;
	algo2.m_dStdTeachRotate = algo->m_dStdTeachRotate;

	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoEdge rst2, RstAlgoEdge* rst)
{
	memset(rst, 0, sizeof(RstAlgoEdge));
	rst->m_dRstShiftX = rst2.m_dRstShiftX;
	rst->m_dRstShiftY = rst2.m_dRstShiftY;
	rst->m_dRstRealAngle = rst2.m_dRstRealAngle;
	rst->m_dRstAngle = rst2.m_dRstAngle;
	memcpy(rst->m_dRstLength, rst2.m_dRstLength, sizeof(double) * EdgeLineTotalCnt);
	rst->m_dRstDistance = rst2.m_dRstDistance;
	rst->m_dRstDistanceX = rst2.m_dRstDistanceX;
	rst->m_dRstDistanceY = rst2.m_dRstDistanceY;

	memcpy(rst->m_poDrawLine, rst2.m_poDrawLine, sizeof(POINTF) * EdgeLineTotalCnt);
	memcpy(rst->m_poDrawLine_Sec, rst2.m_poDrawLine_Sec, sizeof(POINTF) * EdgeLineTotalCnt);

	// OK 판정 //
	rst->m_bOKShiftX = rst2.m_bOKShiftX;
	rst->m_bOKShiftY = rst2.m_bOKShiftY;
	rst->m_bOKAngle = rst2.m_bOKAngle;
	rst->m_bMissing = rst2.m_bMissing;
	rst->m_bOKLength = rst2.m_bOKLength;
	memcpy(rst->m_bArrOKLength, rst2.m_bArrOKLength, sizeof(BOOL) * EdgeLineTotalCnt);
	rst->m_bDistance = rst2.m_bDistance;
	rst->m_bDistanceX = rst2.m_bDistanceX;
	rst->m_bDistanceY = rst2.m_bDistanceY;
	memcpy(rst->m_poDrawLine_T, rst2.m_poDrawLine_T, sizeof(POINTF) * EdgeLineTotalCnt);
	memcpy(rst->m_poDrawLine_T2, rst2.m_poDrawLine_T2, sizeof(POINTF) * EdgeLineTotalCnt);
	//POINTF m_poDrawLine_T3[2];
	rst->m_poDrawCenter = rst2.m_poDrawCenter;

	return true;
}

bool PInspAlgoWrapper::ConvertAlgo(AlgoLine* algo, PIAL::_AlgoLine& algo2)
{
	memset(&algo2, 0, sizeof(PIAL::_AlgoLine));
	algo2.m_bUseIPC = algo->m_bUseIPC;
	algo2.m_byIPCClass = algo->m_byIPCClass;
	algo2.m_bInvertCheck = algo->m_bInvertCheck;

	algo2.m_bFilterIsUse = algo->m_bFilterIsUse;
	algo2.m_nFilterStepNarrow = algo->m_nFilterStepNarrow;

	algo2.m_bShiftIsUse = algo->m_bShiftIsUse;
	algo2.m_dShiftX = algo->m_dShiftX;
	algo2.m_dShiftY = algo->m_dShiftY;

	algo2.m_bAreaIsUse = algo->m_bAreaIsUse;
	algo2.m_dAreaMin = algo->m_dAreaMin;
	algo2.m_dAreaMax = algo->m_dAreaMax;
	algo2.m_dAreaCurrent = algo->m_dAreaCurrent;				//PercentOK

	algo2.m_nTypeSelectBlob = algo->m_nTypeSelectBlob;
	algo2.m_bFillHole = algo->m_bFillHole;
	// 2D
	algo2.m_bInsp2D = algo->m_bInsp2D;
	algo2.m_nMinBinary = algo->m_nMinBinary;
	algo2.m_nMaxBinary = algo->m_nMaxBinary;
	algo2.m_nTypeRange2D = algo->m_nTypeRange2D;
	// 3D
	algo2.m_bInsp3D = algo->m_bInsp3D;
	algo2.m_dHeightRateMin = algo->m_dHeightRateMin;
	algo2.m_dHeightRateMax = algo->m_dHeightRateMax;
	algo2.m_fHeightAvg = algo->m_fHeightAvg;

	algo2.m_nTypeRange3D = algo->m_nTypeRange3D;	// in 0, out 1, upper 2, lower 3

	algo2.m_dTechCenterX = algo->m_dTechCenterX;
	algo2.m_dTechCenterY = algo->m_dTechCenterY;

	algo2.m_bTeachWidthUse = algo->m_bTeachWidthUse;
	algo2.m_dTeachWidth = algo->m_dTeachWidth;
	algo2.m_dTeachWidthRateMin = algo->m_dTeachWidthRateMin;
	algo2.m_dTeachWidthRateMax = algo->m_dTeachWidthRateMax;

	algo2.m_bTeachLengthUse = algo->m_bTeachLengthUse;
	algo2.m_dTeachLength = algo->m_dTeachLength;
	algo2.m_dTeachLengthRateMin = algo->m_dTeachLengthRateMin;
	algo2.m_dTeachLengthRateMax = algo->m_dTeachLengthRateMax;

	algo2.m_bShiftXUse = algo->m_bShiftXUse;
	algo2.m_bShiftYUse = algo->m_bShiftYUse;

	algo2.m_bUseBlobNG = algo->m_bUseBlobNG;
	algo2.m_dBlobSizeWidth = algo->m_dBlobSizeWidth;
	algo2.m_dBlobSizeLength = algo->m_dBlobSizeLength;
	algo2.m_bUseBlobSizeWidth = algo->m_bUseBlobSizeWidth;
	algo2.m_bUseBlobSizeLength = algo->m_bUseBlobSizeLength;

	algo2.m_bUseHeight = algo->m_bUseHeight;
	algo2.m_dTeachHeight = algo->m_dTeachHeight;
	algo2.m_dTeachHeightMax = algo->m_dTeachHeightMax;
	algo2.m_dTeachHeightMin = algo->m_dTeachHeightMin;

	ConvertColorBase(&algo->m_sAlgoColorBase, &algo2.m_sAlgoColorBase);

	algo2.m_bIsHorizon = algo->m_bIsHorizon;
	algo2.m_nMeasureDirection = algo->m_nMeasureDirection;

	algo2.m_bUseAngle = algo->m_bUseAngle;						// Angle NG 검사 유무
	algo2.m_dTeachRotate = algo->m_dTeachRotate;

	algo2.m_bUseFix = algo->m_bUseFix;
	algo2.m_nCrossOpt = algo->m_nCrossOpt;

	memcpy(algo2.m_byLineFindType, algo->m_byLineFindType, sizeof(byte) * 2);

	algo2.m_bUseEndPos = algo->m_bUseEndPos;
	algo2.m_nLineData = algo->m_nLineData;

	memcpy(algo2.m_fArrPerpendicular, algo->m_fArrPerpendicular, sizeof(float)*eMMD_Total);

	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoLine rst2, RstAlgoLine* rst)
{
	memset(rst, 0, sizeof(RstAlgoLine));
	rst->m_dRstShiftX = rst2.m_dRstShiftX;
	rst->m_dRstShiftY = rst2.m_dRstShiftY;
	rst->m_dRstAngle = rst2.m_dRstAngle;
	rst->m_dRstLength = rst2.m_dRstLength;
	//POINTF m_poDrawLine[2];
	memcpy(rst->m_poDrawLine, rst2.m_poDrawLine, sizeof(POINTF) * 2);

	// OK 판정 //
	rst->m_bOKShiftX = rst2.m_bOKShiftX;
	rst->m_bOKShiftY = rst2.m_bOKShiftY;
	rst->m_bOKAngle = rst2.m_bOKAngle;
	rst->m_bMissing = rst2.m_bMissing;
	rst->m_bOKLength = rst2.m_bOKLength;

	//POINTF m_poDrawLine_T[2];
	memcpy(rst->m_poDrawLine_T, rst2.m_poDrawLine_T, sizeof(POINTF) * 2);
	rst->m_poDrawCenter = rst2.m_poDrawCenter;

	rst->m_bOKWidth = rst2.m_bOKWidth;
	rst->m_byWidthCnt = rst2.m_byWidthCnt;
	/*double m_dRstCuttinhW[50];
	double m_dRstCuttinhL[50];
	RECT m_rcDrawWidth[50];*/
	memcpy(rst->m_dRstCuttinhW, rst2.m_dRstCuttinhW, sizeof(double) * 50);
	memcpy(rst->m_dRstCuttinhL, rst2.m_dRstCuttinhL, sizeof(double) * 50);
	memcpy(rst->m_rcDrawWidth, rst2.m_rcDrawWidth, sizeof(RECT) * 50);

	rst->m_bOKPerpendicular = rst2.m_bOKPerpendicular;
	rst->m_fRstPerpendicular = rst2.m_fRstPerpendicular;
	//POINTF m_poDrawLine_2[2];
	memcpy(rst->m_poDrawLine_2, rst2.m_poDrawLine_2, sizeof(POINTF) * 2);

	return true;
}

bool PInspAlgoWrapper::ConvertAlgo(AlgoAlignEdge* algo, PIAL::_AlgoAlignEdge& algo2)
{
	memset(&algo2, 0, sizeof(PIAL::_AlgoAlignEdge));
	for (size_t i = 0; i < ALIGNEDGE_AREA_CNTS; i++)
	{
		ConvertAlgo(&algo->sArrAlgoEdge[i], algo2.sArrAlgoEdge[i]);
	}

	//POINTF m_sArrSearchPoint[ALIGNEDGE_AREA_CNTS];	// Search 위치 좌표 (Part 기준 pixel 좌표)
	//SIZE m_sArrSearchSize[ALIGNEDGE_AREA_CNTS];		// Search 영역 사이즈 (pixel 단위)
	memcpy(algo2.m_sArrSearchPoint, algo->m_sArrSearchPoint, sizeof(POINTF) * ALIGNEDGE_AREA_CNTS);
	memcpy(algo2.m_sArrSearchSize, algo->m_sArrSearchSize, sizeof(SIZE) * ALIGNEDGE_AREA_CNTS);

	algo2.m_nAreaCnt = algo->m_nAreaCnt;
	algo2.m_bCicleOpt = algo->m_bCicleOpt;
	algo2.m_nOPT = algo->m_nOPT;
	algo2.m_bUseAnchor = algo->m_bUseAnchor; // align window와 검출 center의 좌표 거리 반환 옵션
	algo2.m_bUseTwoAnchor = algo->m_bUseTwoAnchor;
	//algo2.m_sAlignWindow = algo->m_sAlignWindow;


	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoAlignEdge rst2, RstAlgoAlignEdge* rst)
{
	memset(rst, 0, sizeof(RstAlgoAlignEdge));
	rst->m_dRstShiftX = rst2.m_dRstShiftX;
	rst->m_dRstShiftY = rst2.m_dRstShiftY;
	rst->m_dTheta = rst2.m_dTheta;
	rst->m_dDistance = rst2.m_dDistance;
	rst->m_dDistanceX = rst2.m_dDistanceX;
	rst->m_dDistanceY = rst2.m_dDistanceY;
	rst->m_d3PtDis1 = rst2.m_d3PtDis1;
	rst->m_d3PtDis3 = rst2.m_d3PtDis3;
	//double m_dArea[EdgeInspCnt];
	memcpy(rst->m_dArea, rst2.m_dArea, sizeof(double) * EdgeInspCnt);
	// OK 판정 //
	rst->m_bOKShiftX = rst2.m_bOKShiftX;
	rst->m_bOKShiftY = rst2.m_bOKShiftY;
	rst->m_bOKAngle = rst2.m_bOKAngle;
	rst->m_bOKDistance = rst2.m_bOKDistance;
	rst->m_bOKDistanceX = rst2.m_bOKDistanceX;
	rst->m_bOKDistanceY = rst2.m_bOKDistanceY;
	rst->m_bOK3PtDis1 = rst2.m_bOK3PtDis1;
	rst->m_bOK3PtDis3 = rst2.m_bOK3PtDis3;

	rst->m_bMissing_1 = rst2.m_bMissing_1;
	rst->m_bMissing_2 = rst2.m_bMissing_2;
	rst->m_bMissing_3 = rst2.m_bMissing_3;

	/*POINTF m_poDrawLine[EdgeLineTotalCnt];
	POINTF m_poDrawLine_Sec[EdgeLineTotalCnt];
	POINTF m_poDrawLine_1[EdgeLineTotalCnt];
	POINTF m_poDrawLine_Sec_1[EdgeLineTotalCnt];
	POINTF m_poDrawLine_2[EdgeLineTotalCnt];
	POINTF m_poDrawLine_Sec_2[EdgeLineTotalCnt];
	RECT m_rcRect_I[3];
	POINTF m_poDrawLine_T[EdgeLineTotalCnt];
	POINTF m_poDrawLine_T2[EdgeLineTotalCnt];
	POINTF m_poDrawLine_T21[EdgeLineTotalCnt];
	POINTF m_poDrawLine_T22[EdgeLineTotalCnt];
	POINTF m_poDrawLine_T31[EdgeLineTotalCnt];
	POINTF m_poDrawLine_T32[EdgeLineTotalCnt];*/

	memcpy(rst->m_poDrawLine, rst2.m_poDrawLine, sizeof(POINTF) * EdgeLineTotalCnt);
	memcpy(rst->m_poDrawLine_Sec, rst2.m_poDrawLine_Sec, sizeof(POINTF) * EdgeLineTotalCnt);
	memcpy(rst->m_poDrawLine_1, rst2.m_poDrawLine_1, sizeof(POINTF) * EdgeLineTotalCnt);
	memcpy(rst->m_poDrawLine_Sec_1, rst2.m_poDrawLine_Sec_1, sizeof(POINTF) * EdgeLineTotalCnt);
	memcpy(rst->m_poDrawLine_2, rst2.m_poDrawLine_2, sizeof(POINTF) * EdgeLineTotalCnt);
	memcpy(rst->m_poDrawLine_Sec_2, rst2.m_poDrawLine_Sec_2, sizeof(POINTF) * EdgeLineTotalCnt);
	memcpy(rst->m_rcRect_I, rst2.m_rcRect_I, sizeof(RECT) * EdgeLineTotalCnt);
	memcpy(rst->m_poDrawLine_T, rst2.m_poDrawLine_T, sizeof(POINTF) * EdgeLineTotalCnt);
	memcpy(rst->m_poDrawLine_T2, rst2.m_poDrawLine_T2, sizeof(POINTF) * EdgeLineTotalCnt);
	memcpy(rst->m_poDrawLine_T21, rst2.m_poDrawLine_T21, sizeof(POINTF) * EdgeLineTotalCnt);
	memcpy(rst->m_poDrawLine_T22, rst2.m_poDrawLine_T22, sizeof(POINTF) * EdgeLineTotalCnt);
	memcpy(rst->m_poDrawLine_T31, rst2.m_poDrawLine_T31, sizeof(POINTF) * EdgeLineTotalCnt);
	memcpy(rst->m_poDrawLine_T32, rst2.m_poDrawLine_T32, sizeof(POINTF) * EdgeLineTotalCnt);

	rst->m_poDrawCenter = rst2.m_poDrawCenter;
	//for teaching display
	/*RECT m_rcFindCircle[3];
	POINTF m_pHPoint[4];
	POINTF m_poRstCenter[EdgeLineTotalCnt];*/
	memcpy(rst->m_rcFindCircle, rst2.m_rcFindCircle, sizeof(RECT) * 3);
	memcpy(rst->m_pHPoint, rst2.m_pHPoint, sizeof(POINTF) * 4);
	memcpy(rst->m_poRstCenter, rst2.m_poRstCenter, sizeof(POINTF) * EdgeLineTotalCnt);




	return true;
}

void PInspAlgoWrapper::SetBGAOffSet(float fBallHeightOffSet, float fBallWidthOffSet, float fBumpXOffSet, float fBumpYOffSet, float fCoplOffSet)
{
	PIAL::PInspAlgo_Lib::m_BumpHeightOffset = fBallHeightOffSet;
// 	PIAL::PInspAlgo_Lib::m_BumpWidth = fBallWidthOffSet;
// 	PIAL::PInspAlgo_Lib::m_BumpOffsetX = fBumpXOffSet;
// 	PIAL::PInspAlgo_Lib::m_BumpOffsetY = fBumpYOffSet;
// 	PIAL::PInspAlgo_Lib::m_BumpCoplOffset = fCoplOffSet;
}

void PInspAlgoWrapper::SetBodyBlobValue(double dSerachPer_Default, double dSerachPer_BodyTip, int nPoint, int nBodyBlobOPT)
{
	PIAL::PInspAlgo_Lib::m_dBodyBlobSearchPer_Default = dSerachPer_Default;
	PIAL::PInspAlgo_Lib::m_dBodyBlobSearchPer_BodyTip = dSerachPer_BodyTip;
	PIAL::PInspAlgo_Lib::m_nAlgoPoint = nPoint;
	PIAL::PInspAlgo_Lib::m_nBodyBlobOPT = nBodyBlobOPT;
}


bool PInspAlgoWrapper::ConvertAlgo(AlgoBodyEdge* algo, PIAL::_AlgoBodyEdge& algo2)
{
	int nDataFlag = algo->narrdata[BodyEdge_N_Data];

	//이진화
	//2D
	algo2.m_bInsp2D = (nDataFlag & BodyEdge_Data_Bin2D) == BodyEdge_Data_Bin2D;
	algo2.m_nMinBinary = algo->narrdata[BodyEdge_N_Min2D];
	algo2.m_nMaxBinary = algo->narrdata[BodyEdge_N_Max2D];
	algo2.m_nTypeRange2D = algo->narrdata[BodyEdge_N_Range2D];
	//3D
	algo2.m_bInsp3D = (nDataFlag & BodyEdge_Data_Bin3D) == BodyEdge_Data_Bin3D;
	algo2.m_dHeightRateMin = algo->farrdata[BodyEdge_F_Min3D];
	algo2.m_dHeightRateMax = algo->farrdata[BodyEdge_F_Max3D];
	algo2.m_nTypeRange3D= algo->narrdata[BodyEdge_N_Range3D];	// in 0, out 1, upper 2, lower 3
	algo2.m_bInvertCheck = FALSE;

	//Filter
	////Mopology
	algo2.m_bFilterIsUse = (nDataFlag & BodyEdge_Data_Filter) == BodyEdge_Data_Filter;
	algo2.m_nFilterStepNarrow = algo->narrdata[BodyEdge_N_Filter];

	//Fill Holl
	algo2.m_bFillHole = (nDataFlag & BodyEdge_Data_FillHoll) == BodyEdge_Data_FillHoll;

	//Blob
	//Select Blob  
	algo2.m_nTypeSelectBlob= algo->narrdata[BodyEdge_N_SelectBlob];// Big 0, All 1, center 2, position 3

	//Teaching Condition
	algo2.m_dTechCenterX = algo->farrdata[BodyEdge_F_TechCenterX];
	algo2.m_dTechCenterY = algo->farrdata[BodyEdge_F_TechCenterY];

	algo2.m_bUseShift = (nDataFlag & BodyEdge_Data_UseShift) == BodyEdge_Data_UseShift;
	algo2.m_bUseShiftX = (nDataFlag & BodyEdge_Data_UseShift) == BodyEdge_Data_UseShift;	// UI상에 Shift 사용 설정이 하나만 있어서 UseShift값을 X,Y에 다 넣어줌
	algo2.m_bUseShiftY = (nDataFlag & BodyEdge_Data_UseShift) == BodyEdge_Data_UseShift;
	algo2.m_dShiftX = algo->farrdata[BodyEdge_F_ShiftX];
	algo2.m_dShiftY = algo->farrdata[BodyEdge_F_ShiftY];

	algo2.m_bUseAngle = (nDataFlag & BodyEdge_Data_UseAngle) == BodyEdge_Data_UseAngle;
	algo2.m_dAngle = algo->farrdata[BodyEdge_F_Angle];
	algo2.m_dStandardAngle = algo->farrdata[BodyEdge_F_StandardAngle]; //Teaching시에 Angle

	algo2.m_bUseWidth = (nDataFlag & BodyEdge_Data_UseWidth) == BodyEdge_Data_UseWidth;
	algo2.m_dWidth = algo->farrdata[BodyEdge_F_Width];
	algo2.m_dWidthRateMin = algo->farrdata[BodyEdge_F_WidthRateMin];
	algo2.m_dWidthRateMax = algo->farrdata[BodyEdge_F_WidthRateMax];

	algo2.m_bUseLength = (nDataFlag & BodyEdge_Data_UseLength) == BodyEdge_Data_UseLength;
	algo2.m_dLength = algo->farrdata[BodyEdge_F_Length];
	algo2.m_dLengthRateMin = algo->farrdata[BodyEdge_F_LengthRateMin];
	algo2.m_dLengthRateMax = algo->farrdata[BodyEdge_F_LengthRateMax];

	algo2.m_nAreaNum = algo->narrdata[BodyEdge_N_AreaNum];
	//algo2.m_bSelectBodyCenterBlob = (nDataFlag & BodyEdge_Data_UseWndCenterBlob) == BodyEdge_Data_UseWndCenterBlob;

	memcpy(algo2.m_rcArea_T, algo->m_rcRoi, sizeof(RECT) * BODY_EDGE_RECT_CNTS);
	memcpy(algo2.m_nAreaDirection, algo->m_nArrDirection, sizeof(int) * BODY_EDGE_RECT_CNTS);
	memcpy(algo2.m_ptTeachEdge, algo->m_sArrTeachPoint, sizeof(POINTF) * 4);

	algo2.m_nSelectIdxAnchorMode = algo->m_nSelectIdxAnchorMode;
	algo2.m_nSelectIdxWindow = algo->m_nSelectIdxWindow;

	algo2.m_bUseDistance = (nDataFlag & BodyEdge_Data_UseDistance) == BodyEdge_Data_UseDistance;
	algo2.m_dTeachDistanceX = algo->farrdata[BodyEdge_F_TeachDistanceX];
	algo2.m_dTeachDistanceY = algo->farrdata[BodyEdge_F_TeachDistanceY];
	algo2.m_dTeachToleranceX = algo->farrdata[BodyEdge_F_TeachToleranceX];
	algo2.m_dTeachToleranceY = algo->farrdata[BodyEdge_F_TeachToleranceY];

		//subLine
	algo2.m_nSubLineMin2D = algo->narrdata[BodyEdge_N_SubLineMin2D];
	algo2.m_nSubLineMax2D = algo->narrdata[BodyEdge_N_SubLineMax2D];
	algo2.m_nSubLineRange2D = algo->narrdata[BodyEdge_N_SubLineRange2D];
	algo2.m_nSubLineRange3D = algo->narrdata[BodyEdge_N_SubLineRange3D];

	algo2.m_dSubLineMin3D = algo->farrdata[BodyEdge_F_SubLineMin3D];
	algo2.m_dSubLineMax3D = algo->farrdata[BodyEdge_F_SubLineMax3D];
	// 	BodyEdge_F_SubLineMin3D,
	// 		BodyEdge_F_SubLineMax3D,
	// 		BodyEdge_N_SubLineMin2D,
	// 		BodyEdge_N_SubLineMax2D,
	// 		BodyEdge_N_SubLineRange2D,
	// 		BodyEdge_N_SubLineRange3D,
	//algo2.m_nPreAlign_Type = algo->narrdata[BodyEdge_N_PreAlignType];

	//algo2.m_fSubLProp = algo->farrdata[BodyEdge_F_SubLProp] / 100;
	return true;

}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoBodyEdge rst2, RstAlgoBodyEdge* rst)
{
	rst->m_dRstOffset_x = rst2.m_dRstOffset_x;
	rst->m_dRstOffset_y = rst2.m_dRstOffset_y;
	rst->m_dRstTheta = rst2.m_dRstTheta;
	rst->m_dRstRealTheta = rst2.m_dRstRealTheta;
	rst->m_dRstWidth = rst2.m_dRstWidth;
	rst->m_dRstLength = rst2.m_dRstLength;
	rst->m_dRstCenterX = rst2.m_dRstCenterX;
	rst->m_dRstCenterY = rst2.m_dRstCenterY;
	rst->m_dRstDistanceX = rst2.m_dRstDistanceX;
	rst->m_dRstDistanceY = rst2.m_dRstDistanceY;

	// OK 판정 //
	rst->m_bOKShiftX = rst2.m_bOKShiftX;
	rst->m_bOKShiftY = rst2.m_bOKShiftY;
	rst->m_bOKAngle = rst2.m_bOKAngle;
	rst->m_bOKWidth = rst2.m_bOKWidth;
	rst->m_bOKLength = rst2.m_bOKLength;
	rst->m_bOKDistanceX = rst2.m_bOKDistanceX;
	rst->m_bOKDistanceY = rst2.m_bOKDistanceY;

	rst->m_nAreaNum = rst2.m_nAreaNum;
	//RECT m_rcArea_T[16];
	memcpy(rst->m_rcArea_T, rst2.m_rcArea_T, sizeof(RECT) * BODY_EDGE_RECT_CNTS);

	//RECT m_rcArea_I[16];
	memcpy(rst->m_rcArea_I, rst2.m_rcArea_I, sizeof(RECT) * BODY_EDGE_RECT_CNTS);

	//RECT m_rcBodyRect = rst2.m_rcBodyRect;
	memcpy(&rst->m_rcBodyRect, &rst2.m_rcBodyRect, sizeof(RECT));

	//POINTF m_ptInspEdge[4]; //좌상, 우상, 우하, 좌하
	memcpy(rst->m_ptInspEdge, rst2.m_ptInspEdge, sizeof(POINTF) * 4);
	
	rst->m_poDrawCenter = rst2.m_poDrawCenter;
// 	rst->m_bOKEdgeStraightness = rst2.m_bOKEdgeStraightness;
// 	rst->m_fRstEdgeStraightness = rst2.m_fRstEdgeStraightness;
// 	rst->m_fRstES_Above = rst2.m_fRstES_Above;
// 	rst->m_fRstES_Below = rst2.m_fRstES_Below;
// 	rst->m_poES_Above = rst2.m_poES_Above;
// 	rst->m_poES_Below = rst2.m_poES_Below;

	return true;
}

void PInspAlgoWrapper::RoiImageCompose_LT_SIMD(UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR* ptrbyWhiteBuffer,
	int nFullImageSize, int nFullImageWidth, int nFullImageHeight,
	int nROIImageSize, double dROIX, double dROIY, int nROIWidth, int nROIHeight,
	int nRedValue, int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage)
{

	if (nROIWidth == 0)
		nROIWidth = nFullImageWidth;

	if (nROIHeight == 0)
		nROIHeight = nFullImageHeight;

	float fRedValue = (float)nRedValue / 100.0;
	float fGreenValue = (float)nGreenValue / 100.0;
	float fBlueValue = (float)nBlueValue / 100.0;
	float fWhiteValue = (float)nWhiteValue / 100.0;

	int nValueBuf = 0;
	float fRedImageValue = 0;
	float fGreenImageValue = 0;
	float fBlueImageValue = 0;
	float fWhiteImageValue = 0;

	int nROIIndex = 0;
	int nStartX = 0, nStartY = 0;
	if (nFullImageWidth > nROIWidth)
		nStartX = RounD(dROIX);
	if (nFullImageHeight > nROIHeight)
		nStartY = RounD(dROIY);
	if (nStartX < 0)
		nStartX = 0;
	if (nStartY < 0)
		nStartY = 0;

	std::vector<UCHAR*> vecPtr(4, nullptr);
	std::vector<float> vecfValue(4, 0);
	int nMerge = 0;

	if (ptrbyRedBuffer && fRedValue > 0)
	{
		vecPtr[nMerge] = ptrbyRedBuffer;
		vecfValue[nMerge++] = fRedValue;

	}
	if (ptrbyGreenBuffer && fGreenValue > 0)
	{
		vecPtr[nMerge] = ptrbyGreenBuffer;
		vecfValue[nMerge++] = fGreenValue;
	}
	if (ptrbyBlueBuffer && fBlueValue > 0)
	{
		vecPtr[nMerge] = ptrbyBlueBuffer;
		vecfValue[nMerge++] = fBlueValue;
	}
	if (ptrbyWhiteBuffer && fWhiteValue > 0)
	{
		vecPtr[nMerge] = ptrbyWhiteBuffer;
		vecfValue[nMerge++] = fWhiteValue;
	}
	 PIAL::compositingImage::ROICompose_SSE(nMerge, vecPtr, vecfValue,
		nFullImageWidth, nFullImageHeight, nStartX, nStartY,
		nROIWidth, nROIHeight, ptrbyResultImage);
	//ROICompose_AVX2(nMerge, vecPtr, vecfValue,
	//	nFullImageWidth, nFullImageHeight, nStartX, nStartY,
	//	nROIWidth, nROIHeight, ptrbyResultImage);

}

void PInspAlgoWrapper::ROIImageClaculCompose_SIMD(const LightTypeBuf &sLightImg, UCHAR* ptrbyResultImage)
{
	int nFullImageWidth = sLightImg.m_nImgWidth;
	int nFullImageHeight = sLightImg.m_nImgHeight;
	int nROIWidth = sLightImg.m_nROIImgWidth;
	int nROIHeight = sLightImg.m_nROIImgHeight;
	if (nROIWidth == 0)
		nROIWidth = nFullImageWidth;

	if (nROIHeight == 0)
		nROIHeight = nFullImageHeight;

	int nImgCnt = sLightImg.m_nImgCnt;
	if ((nFullImageWidth <= 0) || (nFullImageHeight <= 0) || (nImgCnt <= 0) || (nROIWidth <= 0) || (nROIHeight <= 0))
		return;
	double dROIX = sLightImg.m_dROIX;
	double dROIY = sLightImg.m_dROIY;

	UCHAR* ptrbyTopRedBuffer = sLightImg.m_pucTRed;
	UCHAR* ptrbyTopGreenBuffer = sLightImg.m_pucTGreen;
	UCHAR* ptrbyTopBlueBuffer = sLightImg.m_pucTBlue;
	UCHAR* ptrbyTopWhiteBuffer = sLightImg.m_pucTWhite;

	UCHAR* ptrbyMidRedBuffer = sLightImg.m_pucMRed;
	UCHAR* ptrbyMidGreenBuffer = sLightImg.m_pucMGreen;
	UCHAR* ptrbyMidBlueBuffer = sLightImg.m_pucMBlue;
	UCHAR* ptrbyMidWhiteBuffer = sLightImg.m_pucMWhite;

	UCHAR* ptrbyBotRedBuffer = sLightImg.m_pucBRed;
	UCHAR* ptrbyBotGreenBuffer = sLightImg.m_pucBGreen;
	UCHAR* ptrbyBotBlueBuffer = sLightImg.m_pucBBlue;
	UCHAR* ptrbyBotWhiteBuffer = sLightImg.m_pucBWhite;

	int* pnRedValue = sLightImg.m_pnRedValue;
	int* pnGreenValue = sLightImg.m_pnGreenValue;
	int* pnBlueValue = sLightImg.m_pnBlueValue;
	int* pnWhiteValue = sLightImg.m_pnWhiteValue;

	int *ptrnPosition = sLightImg.m_pnPosition;
	int *nCalculation = sLightImg.m_pnCalculation;
	if ((nCalculation == NULL) || (ptrbyResultImage == NULL))
		return;

	float fRedValue = 0.0;
	float fGreenValue = 0.0;
	float fBlueValue = 0.0;
	float fWhiteValue = 0.0;

	int nStartX = 0, nStartY = 0;
	if (nFullImageWidth > nROIWidth)
		nStartX = RounD(dROIX);
	if (nFullImageHeight > nROIHeight)
		nStartY = RounD(dROIY);
	if (nStartX < 0)
		nStartX = 0;
	if (nStartY < 0)
		nStartY = 0;

	int nImageSize = nROIWidth * nROIHeight;

	UCHAR* ptrBufferCompose = NULL;
	Make_1DArray((PCHAR)__FUNCTION__, __LINE__, &ptrBufferCompose, nROIWidth*  nROIHeight);
	cv::Mat dstImg(nROIHeight, nROIWidth, CV_8UC1, ptrbyResultImage);
	cv::Mat srcImg(nROIHeight, nROIWidth, CV_8UC1, ptrBufferCompose);

	for (int a = 0; a < nImgCnt; a++)
	{
		UCHAR* ptrDst = ptrBufferCompose;
		if (a == 0)
			ptrDst = ptrbyResultImage;

		fRedValue = (double)pnRedValue[a] / 100.0;
		fGreenValue = (double)pnGreenValue[a] / 100.0;
		fBlueValue = (double)pnBlueValue[a] / 100.0;
		fWhiteValue = (double)pnWhiteValue[a] / 100.0;

		std::vector<UCHAR*> vecPtr(4, nullptr);
		std::vector<float> vecfValue(4, 0);
		int nMerge = 0;
		if (ptrnPosition[a] == 0)
		{
			if (ptrbyTopRedBuffer && fRedValue > 0)
			{
				vecPtr[nMerge] = ptrbyTopRedBuffer;
				vecfValue[nMerge++] = fRedValue;

			}
			if (ptrbyTopGreenBuffer && fGreenValue > 0)
			{
				vecPtr[nMerge] = ptrbyTopGreenBuffer;
				vecfValue[nMerge++] = fGreenValue;
			}
			if (ptrbyTopBlueBuffer && fBlueValue > 0)
			{
				vecPtr[nMerge] = ptrbyTopBlueBuffer;
				vecfValue[nMerge++] = fBlueValue;
			}
			if (ptrbyTopWhiteBuffer && fWhiteValue > 0)
			{
				vecPtr[nMerge] = ptrbyTopWhiteBuffer;
				vecfValue[nMerge++] = fWhiteValue;
			}
		}
		else if (ptrnPosition[a] == 1)
		{
			if (ptrbyMidRedBuffer && fRedValue > 0)
			{
				vecPtr[nMerge] = ptrbyMidRedBuffer;
				vecfValue[nMerge++] = fRedValue;
			}
			if (ptrbyMidGreenBuffer && fGreenValue > 0)
			{
				vecPtr[nMerge] = ptrbyMidGreenBuffer;
				vecfValue[nMerge++] = fGreenValue;
			}
			if (ptrbyMidBlueBuffer && fBlueValue > 0)
			{
				vecPtr[nMerge] = ptrbyMidBlueBuffer;
				vecfValue[nMerge++] = fBlueValue;
			}
			if (ptrbyMidWhiteBuffer && fWhiteValue > 0)
			{
				vecPtr[nMerge] = ptrbyMidWhiteBuffer;
				vecfValue[nMerge++] = fWhiteValue;
			}
		}
		else if (ptrnPosition[a] == 2)
		{
			if (ptrbyBotRedBuffer && fRedValue > 0)
			{
				vecPtr[nMerge] = ptrbyBotRedBuffer;
				vecfValue[nMerge++] = fRedValue;
			}
			if (ptrbyBotGreenBuffer && fGreenValue > 0)
			{
				vecPtr[nMerge] = ptrbyBotGreenBuffer;
				vecfValue[nMerge++] = fGreenValue;
			}
			if (ptrbyBotBlueBuffer && fBlueValue > 0)
			{
				vecPtr[nMerge] = ptrbyBotBlueBuffer;
				vecfValue[nMerge++] = fBlueValue;
			}
			if (ptrbyBotWhiteBuffer && fWhiteValue > 0)
			{
				vecPtr[nMerge] = ptrbyBotWhiteBuffer;
				vecfValue[nMerge++] = fWhiteValue;
			}
		}
		else
		{
			Delete_1DArray(&ptrBufferCompose);
			return;
		}
		PIAL::compositingImage::ROICompose_SSE(nMerge, vecPtr, vecfValue,
			nFullImageWidth, nFullImageHeight, nStartX, nStartY,
			nROIWidth, nROIHeight, ptrDst);
		/*	ROICompose_AVX2(nMerge, vecPtr, vecfValue,
				 nFullImageWidth,  nFullImageHeight,  nStartX,  nStartY,
				 nROIWidth,  nROIHeight, ptrDst);*/

		if (a == 0)
			continue;

		if (nMerge == 0)
			continue;

		if (nCalculation[a - 1] == 1)
			cv::add(dstImg, srcImg, dstImg);
		else if (nCalculation[a - 1] == 2)
			cv::subtract(dstImg, srcImg, dstImg);

	}

	if (ptrBufferCompose)
		Delete_1DArray(&ptrBufferCompose);

}

// bool PInspAlgoWrapper::ConvertAlgo(AlgoPackageThickness* algo, PIAL::_AlgoPackageThickness& algo2)
// {
// 	memcpy(algo2.m_ArrDataN, algo->m_ArrDataN, sizeof(int) * _m_ePackageThicknessN_Total);
// 	memcpy(algo2.m_ArrDataF, algo->m_ArrDataF, sizeof(float) * _m_ePackageThicknessF_Total);
// 	return true;
// }
// 
// bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoPackageThickness rst2, RstAlgoPackageThickness* rst)
// {	
// 	rst->m_bOKThickness = rst2.m_bOKThickness;
// 	rst->m_dRstThickness = rst2.m_dRstThickness;
// 	memcpy(&rst->m_RstRect, &rst2.m_RstRect, sizeof(RECT));
// 	return true;
// }

// bool PInspAlgoWrapper::ConvertAlgo(AlgoGWire* algo, PIAL::_AlgoGWire& algo2)
// {
// 	algo2.m_bInsp2D = (algo->narrdata[GWire_N_Data] & GWire_Data_Use2D) == GWire_Data_Use2D;
// 	algo2.m_bInsp3D = (algo->narrdata[GWire_N_Data] & GWire_Data_Use3D) == GWire_Data_Use3D;
// 	algo2.m_bFilterIsUse = (algo->narrdata[GWire_N_Data] & GWire_Data_Filter) == GWire_Data_Filter;
// 	algo2.m_bFillHole = (algo->narrdata[GWire_N_Data] & GWire_Data_FillHoll) == GWire_Data_FillHoll;
// 	algo2.m_bUseCrossFilter = (algo->narrdata[GWire_N_Data] & GWire_Data_CrossFilter) == GWire_Data_CrossFilter;
// 	algo2.m_bUseCenterMargin = (algo->narrdata[GWire_N_Data] & GWire_Data_UseCenterMargin) == GWire_Data_UseCenterMargin;
// 	algo2.m_nTypeRange2D = algo->narrdata[GWire_N_Range2D];
// 	algo2.m_nTypeRange3D = algo->narrdata[GWire_N_Range3D];
// 	algo2.m_nMinBinary = algo->narrdata[GWire_N_Min2D];
// 	algo2.m_nMaxBinary = algo->narrdata[GWire_N_Max2D];
// 	algo2.m_nFilterStepNarrow = algo->narrdata[GWire_N_Filter];
// 	algo2.m_nType = algo->narrdata[GWire_N_Type];
// 	algo2.m_nDirection = algo->narrdata[GWire_N_Direction];
// 	algo2.m_nWireCnt = algo->narrdata[GWire_N_WireCount];
// 	algo2.m_nCenterMargin = algo->narrdata[GWire_N_CenterMargin];
// 	algo2.m_dHeightRateMin = algo->farrdata[GWire_F_Min3D];
// 	algo2.m_dHeightRateMax = algo->farrdata[GWire_F_Max3D];
// 	algo2.m_dWireThickness = algo->farrdata[GWire_F_WireThickness];
// 	algo2.m_nTypeSelectBlob = _ETypeBlob::_eSelectMix;
// 
// 	return true;
// }
// 
// bool PInspAlgoWrapper::ConvertAlgo(AlgoGWire* algo, PIAL::_AlgoBodyEdge& algo2)
// {
// 	algo2.m_bInsp2D = (algo->narrdata[GWire_N_Data] & GWire_Data_Use2D) == GWire_Data_Use2D;
// 	algo2.m_bInsp3D = (algo->narrdata[GWire_N_Data] & GWire_Data_Use3D) == GWire_Data_Use3D;
// 	algo2.m_bFilterIsUse = (algo->narrdata[GWire_N_Data] & GWire_Data_Filter) == GWire_Data_Filter;
// 	algo2.m_bFillHole = (algo->narrdata[GWire_N_Data] & GWire_Data_FillHoll) == GWire_Data_FillHoll;
// 	algo2.m_nTypeRange2D = algo->narrdata[GWire_N_Range2D];
// 	algo2.m_nTypeRange3D = algo->narrdata[GWire_N_Range3D];
// 	algo2.m_nMinBinary = algo->narrdata[GWire_N_Min2D];
// 	algo2.m_nMaxBinary = algo->narrdata[GWire_N_Max2D];
// 	algo2.m_nFilterStepNarrow = algo->narrdata[GWire_N_Filter];
// 	algo2.m_dHeightRateMin = algo->farrdata[GWire_F_Min3D];
// 	algo2.m_dHeightRateMax = algo->farrdata[GWire_F_Max3D];
// 	algo2.m_nTypeSelectBlob = _ETypeBlob::_eSelectMix;
// 
// 	return true;
// }

// bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoGWire rst2, RstAlgoGWire* rst)
// {
// 	rst->m_bCntOK = rst2.m_bCntOK;
// 	rst->m_nRstCnt = rst2.m_nRstCnt;
// 
// 	rst->m_nCenterMarginX = rst2.m_nCenterMarginX;
// 	rst->m_nCenterMarginY = rst2.m_nCenterMarginY;
// 	rst->m_nCenterMarginW = rst2.m_nCenterMarginW;
// 	rst->m_nCenterMarginH = rst2.m_nCenterMarginH;
// 
// 	return true;
// }


bool PInspAlgoWrapper::SetGolden_KTL(PIAL::_AlgoBGA& algo2)
{
	CString ProcessName = TEXT("Eagle3D");
	CString strPath = TEXT("C:\\") + ProcessName + TEXT("_64x\\Init\\GoldenDevice_KTL.ini");

	if (PathFileExists(strPath) == false)
		return false;

	TCHAR buff[255];
	GetPrivateProfileString(_T("GOLDEN"), _T("NUM"), _T("0"), buff, sizeof(buff), (LPCTSTR)strPath);

	int nBumpNum = _tstoi(buff);
	int Target1 = -1;
	int Target2 = -1;
	int CopleTarget = -1;
	GetPrivateProfileString(_T("GOLDEN"), _T("Target1"), _T("-1"), buff, sizeof(buff), (LPCTSTR)strPath);
	Target1 = _tstoi(buff);
	GetPrivateProfileString(_T("GOLDEN"), _T("Target2"), _T("-1"), buff, sizeof(buff), (LPCTSTR)strPath);
	Target2 = _tstoi(buff);
	GetPrivateProfileString(_T("GOLDEN"), _T("CopleTarget"), _T("-1"), buff, sizeof(buff), (LPCTSTR)strPath);
	CopleTarget = _tstoi(buff);

	if (Target1 != -1)
		algo2.m_nIndexTarget1Bump = Target1;
	if (Target2 != -1)
		algo2.m_nIndexTarget2Bump = Target2;
	if (Target2 != -1)
		algo2.m_nIndexCopleBump = CopleTarget;
	/*
	for (size_t i = 0; i < nBumpNum + 1; i++)
	{
		CString strIndex;      //CString형 선언
		strIndex.Format(_T("%d"), i); //int 를 CStirng으로 변환
		GetPrivateProfileString(strIndex, _T("BALL_COP"), NULL, buff, sizeof(buff), (LPCTSTR)strPath);
		if (buff == NULL)continue;
		float BALL_COP = _tstoi(buff);
		GetPrivateProfileString(strIndex, _T("BALL_OFFSETX"), NULL, buff, sizeof(buff), (LPCTSTR)strPath);
		if (buff == NULL)continue;
		float BALL_OFFSETX = _tstoi(buff);
		GetPrivateProfileString(strIndex, _T("BALL_OFFSETY"), NULL, buff, sizeof(buff), (LPCTSTR)strPath);
		if (buff == NULL)continue;
		float BALL_OFFSETY = _tstoi(buff);
		GetPrivateProfileString(strIndex, _T("BALL_WIDTH"), NULL, buff, sizeof(buff), (LPCTSTR)strPath);
		if (buff == NULL)continue;
		float BALL_WIDTH = _tstoi(buff);

		if (BALL_COP == 0 && BALL_OFFSETX == 0 && BALL_OFFSETY == 0 && BALL_WIDTH == 0)
			continue;
	}
	*/
	return true;
}

//bool PInspAlgoWrapper::ConvertAlgo(AlgoGWire* algo, PIAL::_AlgoGWire& algo2)
//{
//	algo2.m_bInsp2D = (algo->narrdata[m_eGWireN_Data] & m_eGWireData_Use2D) == m_eGWireData_Use2D;
//	algo2.m_bInsp3D = (algo->narrdata[m_eGWireN_Data] & m_eGWireData_Use3D) == m_eGWireData_Use3D;
//	algo2.m_bFilterIsUse = (algo->narrdata[m_eGWireN_Data] & m_eGWireData_Filter) == m_eGWireData_Filter;
//	algo2.m_bFillHole = (algo->narrdata[m_eGWireN_Data] & m_eGWireData_FillHoll) == m_eGWireData_FillHoll;
//	algo2.m_bUseCrossFilter = (algo->narrdata[m_eGWireN_Data] & m_eGWireData_CrossFilter) == m_eGWireData_CrossFilter;
//	algo2.m_bUseCenterMargin = (algo->narrdata[m_eGWireN_Data] & m_eGWireData_UseCenterMargin) == m_eGWireData_UseCenterMargin;
//	algo2.m_nTypeRange2D = algo->narrdata[m_eGWireN_Range2D];
//	algo2.m_nTypeRange3D = algo->narrdata[m_eGWireN_Range3D];
//	algo2.m_nMinBinary = algo->narrdata[m_eGWireN_Min2D];
//	algo2.m_nMaxBinary = algo->narrdata[m_eGWireN_Max2D];
//	algo2.m_nType = algo->narrdata[m_eGWireN_Type];
//	algo2.m_nDirection = algo->narrdata[m_eGWireN_Direction];
//	algo2.m_nWireCnt = algo->narrdata[m_eGWireN_WireCount];
//	algo2.m_nFilterStepNarrow = algo->narrdata[m_eGWireN_Filter];
//	algo2.m_nCenterMargin = algo->narrdata[m_eGWireN_CenterMargin];
//	algo2.m_dHeightRateMin = algo->farrdata[m_eGWireF_Min3D];
//	algo2.m_dHeightRateMax = algo->farrdata[m_eGWireF_Max3D];
//	algo2.m_dWireThickness = algo->farrdata[m_eGWireF_WireThickness];
//	algo2.m_nTypeSelectBlob = _ETypeBlob::_eSelectMix;
//
//	return true;
//}

//bool PInspAlgoWrapper::ConvertAlgo(AlgoGWire* algo, PIAL::_AlgoBodyEdge& algo2)
//{
//	algo2.m_bInsp2D = (algo->narrdata[m_eGWireN_Data] & m_eGWireData_Use2D) == m_eGWireData_Use2D;
//	algo2.m_bInsp3D = (algo->narrdata[m_eGWireN_Data] & m_eGWireData_Use3D) == m_eGWireData_Use3D;
//	algo2.m_bFilterIsUse = (algo->narrdata[m_eGWireN_Data] & m_eGWireData_Filter) == m_eGWireData_Filter;
//	algo2.m_bFillHole = (algo->narrdata[m_eGWireN_Data] & m_eGWireData_FillHoll) == m_eGWireData_FillHoll;
//	algo2.m_nTypeRange2D = algo->narrdata[m_eGWireN_Range2D];
//	algo2.m_nTypeRange3D = algo->narrdata[m_eGWireN_Range3D];
//	algo2.m_nMinBinary = algo->narrdata[m_eGWireN_Min2D];
//	algo2.m_nMaxBinary = algo->narrdata[m_eGWireN_Max2D];
//	algo2.m_nFilterStepNarrow = algo->narrdata[m_eGWireN_Filter];
//	algo2.m_dHeightRateMin = algo->farrdata[m_eGWireF_Min3D];
//	algo2.m_dHeightRateMax = algo->farrdata[m_eGWireF_Max3D];
//	algo2.m_nTypeSelectBlob = _ETypeBlob::_eSelectMix;
//
//	return true;
//}

//bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoGWire rst2, RstAlgoGWire* rst)
//{
//	rst->m_bCntOK = rst2.m_bCntOK;
//	rst->m_nRstCnt = rst2.m_nRstCnt;
//
//	rst->m_nCenterMarginX = rst2.m_nCenterMarginX;
//	rst->m_nCenterMarginY = rst2.m_nCenterMarginY;
//	rst->m_nCenterMarginW = rst2.m_nCenterMarginW;
//	rst->m_nCenterMarginH = rst2.m_nCenterMarginH;
//
//	return true;
//}


bool PInspAlgoWrapper::ConvertAlgo(AlgoPOCR* algo, PIAL::_AlgoPOCR& algo2)
{
	algo2.m_dWndAngle = algo->m_dWndAngle;
	algo2.m_dFontAngle = algo->m_dFontAngle;
	algo2.m_nClrForeGround = algo->m_nClrForeGround;
	algo2.m_nClrResultFont = algo->m_nClrResultFont;
	algo2.m_nThreshVal = algo->m_nThreshVal;
	algo2.m_nThreshMode = algo->m_nThreshMode;
	algo2.m_bUsePolarity = algo->m_bUsePolarity;
	algo2.m_nCharMaxCount = algo->m_nCharMaxCount;
	algo2.m_bUsePOCRNG = algo->m_bUsePOCRNG;
	algo2.m_bUseLib = algo->m_bUseLib;
	algo2.m_bUseLowerSpecial = algo->m_bUseLowerSpecial;
	algo2.m_bUseCharSpace = algo->m_bUseCharSpace;
	algo2.m_bUseWndNameInWindow = algo->m_bUseWndNameInWindow;
	algo2.m_bUseOCV = algo->m_bUseOCV;
	algo2.m_dOCVScore = algo->m_dOCVScore;

	memcpy(algo2.m_sWindName, algo->m_sWindName, _MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_sFontPath, algo->m_sFontPath, _MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_sTargetFont, algo->m_sTargetFont, _MAX_STRLEN * sizeof(wchar_t));
	memcpy(algo2.m_dStdCharScore, algo->m_dStdCharScore, _MAX_STRLEN * sizeof(double));

	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoPOCR rst2, RstAlgoPOCR* rst)
{
	rst->m_nCharCount = rst2.m_nCharCount;
	rst->m_dStringScore = rst2.m_dStringScore;

	rst->m_bOKString = rst2.m_bOKString;
	rst->m_bOKScore = rst2.m_bOKScore;
	rst->m_bOKPolarity = rst2.m_bOKPolarity;

	rst->ModelX = rst2.ModelX;
	rst->ModelY = rst2.ModelY;
	rst->ModelWidth = rst2.ModelWidth;
	rst->ModelHeight = rst2.ModelHeight;

	rst->Insp_Ro = rst2.Insp_Ro;
	rst->ModelX_Ro = rst2.ModelX_Ro;
	rst->ModelY_Ro = rst2.ModelY_Ro;
	rst->ModelWidth_Ro = rst2.ModelWidth_Ro;
	rst->ModelHeight_Ro = rst2.ModelHeight_Ro;

	rst->bAIOK = rst2.bAIOK;

	rst->bRstShiftX = rst2.bRstShiftX;
	rst->dRstShiftX = rst2.dRstShiftX;
	rst->bRstShiftY = rst2.bRstShiftY;
	rst->dRstShiftY = rst2.dRstShiftY;

	memcpy(rst->m_dArrCharScore, rst2.m_dArrCharScore, _MAX_STRLEN * sizeof(double));
	memcpy(rst->m_nCharScoreResult, rst2.m_nCharScoreResult, _MAX_STRLEN * sizeof(int));
	memcpy(rst->m_cArrStr, rst2.m_cArrStr, _MAX_STRLEN * sizeof(wchar_t));
	memcpy(rst->charX, rst2.charX, _MAX_STRLEN * sizeof(double));
	memcpy(rst->charY, rst2.charY, _MAX_STRLEN * sizeof(double));
	memcpy(rst->charWidth, rst2.charWidth, _MAX_STRLEN * sizeof(double));
	memcpy(rst->charHeight, rst2.charHeight, _MAX_STRLEN * sizeof(double));
	memcpy(rst->charAngle, rst2.charAngle, _MAX_STRLEN * sizeof(double));
	memcpy(rst->m_nCharScoreResult_Ro, rst2.m_nCharScoreResult_Ro, _MAX_STRLEN * sizeof(int));
	memcpy(rst->m_dArrCharScore_Ro, rst2.m_dArrCharScore_Ro, _MAX_STRLEN * sizeof(double));
	memcpy(rst->charX_Ro, rst2.charX_Ro, _MAX_STRLEN * sizeof(double));
	memcpy(rst->charY_Ro, rst2.charY_Ro, _MAX_STRLEN * sizeof(double));
	memcpy(rst->charWidth_Ro, rst2.charWidth_Ro, _MAX_STRLEN * sizeof(double));
	memcpy(rst->charHeight_Ro, rst2.charHeight_Ro, _MAX_STRLEN * sizeof(double));
	memcpy(rst->charAngle_Ro, rst2.charAngle_Ro, _MAX_STRLEN * sizeof(double));
	memcpy(rst->nAIFontOK, rst2.nAIFontOK, _MAX_STRLEN * sizeof(int));
	memcpy(rst->sFont_AI, rst2.sFont_AI, _MAX_STRLEN * sizeof(wchar_t));
	memcpy(rst->Score_AI, rst2.Score_AI, _MAX_STRLEN * sizeof(double));
	memcpy(rst->Score_AI_Sec, rst2.Score_AI_Sec, _MAX_STRLEN * sizeof(double));
	memcpy(rst->m_cArrTeachStr, rst2.m_cArrTeachStr, _MAX_STRLEN * sizeof(wchar_t));
	return true;
}

bool PInspAlgoWrapper::ConvertAlgo(AlgoPadArray* algo, PIAL::_AlgoPadArray& algo2)
{
	int nFMin = MIN(PadArray_F_Total, _PadArray_F_Total);
	for (int i = 0; i < nFMin; i++)
		algo2.farrdata[i] = algo->farrdata[i];
	algo2.idata = algo->idata;

	int nByMin = MIN(PadArray_by_Total, _PadArray_by_Total);
	for (int i = 0; i < nByMin; i++)
		algo2.narrdata[i] = algo->narrdata[i];

	for (int i = 0; i < 6; i++)
	{
		algo2.rcFirstROI[i] = algo->rcFirstROI[i];
		algo2.rcSeccondROI[i] = algo->rcSecondROI[i];
	}
	return true;
}

bool PInspAlgoWrapper::ConvertRstAlgo(PIAL::_RstAlgoPadArray rst2, RstAlgoPadArray* rst)
{
	memcpy(rst, &rst2, sizeof(PIAL::_RstAlgoPadArray));

	return true;
}

bool PInspAlgoWrapper::ConvertAlgo(PIAL::_InspRstPolyAlgo* pRstPoly, InspRstPolyAlgo* pRstPoly2)
{
	if (pRstPoly == nullptr || pRstPoly2 == nullptr)
		return false;

	pRstPoly2->m_ndX = pRstPoly->m_ndX;
	pRstPoly2->m_ndY = pRstPoly->m_ndY;
	pRstPoly2->m_nWindID = pRstPoly->m_nWindID;
	pRstPoly2->m_nAlgoID = pRstPoly->m_nAlgoID;
	pRstPoly2->m_nRoiID = pRstPoly->m_nRoiID;
	pRstPoly2->m_nCount = pRstPoly->m_nCount;
	pRstPoly2->m_ptCenter = pRstPoly->m_ptCenter;

	pRstPoly2->m_vPolyPoints.resize(pRstPoly->m_vPolyPoints.size());
	std::copy(pRstPoly->m_vPolyPoints.begin(), pRstPoly->m_vPolyPoints.end(), pRstPoly2->m_vPolyPoints.begin());

	return true;
}