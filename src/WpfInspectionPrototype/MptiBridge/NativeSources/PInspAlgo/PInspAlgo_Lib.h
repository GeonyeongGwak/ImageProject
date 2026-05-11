#pragma once

#include "PInspalgo_Def.h"
#include "Insp_Wnd.h"

constexpr float PI__ = 3.141592f;


namespace PIAL
{
	class ResolMap;
	class PINSPALGO_API PInspAlgo_Lib
	{
	public:
		static int m_fovWidth;
		static int m_fovLength;
		static double m_resolX;
		static double m_resolY;

		static int m_MachineType;
		static int m_CameraType;
		static bool m_WindowRotate;
		static bool m_bSideOriginalSize;

		static int m_nCompositeLightMode;
		static float m_compoBtmR;
		static float m_compoBtmG;
		static float m_compoBtmB;

		static float m_fit_R;
		static float m_fit_G;
		static float m_fit_B;
		static float m_fit_BR;
		static float m_fit_BB;

		static UCHAR* CIEMapOriginal;
		static UCHAR* ModifiedCIEMap;
		static POINT* CIEXY;

		static byte m_byDefaultAC[2];
		static _lightData* m_sLightData;	   // AngleColor 조명 값
		// static Insp_ROIImg* PartImageBuffer; // 파트 이미지 버퍼를 가지고 있음.

		// AOI Insp Option
		static double m_dBodyBlobSearchPer_Default;
		static double m_dBodyBlobSearchPer_BodyTip;
		static int m_nAlgoPoint;
		static int m_nBodyBlobOPT;

		static float m_BumpHeightOffset; //micron
		static float m_BumpHeightOffset_FlatBall;
		static float m_BumpHeightOffset_Blob;
		static float m_BumpCoplOffset; //micron
		static float m_BumpOffsetX; //micron
		static float m_BumpOffsetY; //micron
		static float m_BumpWidth; //micron

		static bool m_BumpRecoveryCenter;
		static bool m_Bump_NoCut;
		static bool m_Bump_Length_NO_Diameter; //Stemtec True, Length에 기준 false: diameter, true: length

		static bool m_NGBlob_ExROI_Rotate;

		static int m_ntempOption1; //현재는 Hynix 임시 옵션1
		static int m_ntempOption2; //현재는 Hynix 임시 옵션2
		static int m_ntempOption3; //현재는 Hynix 임시 옵션3
		static int m_ntempOption4; //현재는 Hynix 임시 옵션3

		static bool m_SaveTempLog;

		static void Factory();
		static void Free();

		static void AddLog(CString sMessge);
		static void AddLog_Dev(CString sMessge);
		static void InitDevice(int fovWidth, int fovLength, double resolX, double resolY);

		static void SetCompoLightData(int nCompositeLightMode, float compoBtmR, float compoBtmG, float compoBtmB);
		static void SetFittingData(float r, float g, float b, float br, float bb);
		static void GetColorBuffer_Window(UCHAR** img_R, UCHAR** img_G, UCHAR** img_B, Insp_Wnd* wndBuffer, bool bTeach, int nInspAC = 0, int nSideCam = -1);
		static void GetColorBuffer_Part(Insp_Image* pImg_buf, UCHAR** img_R, UCHAR** img_G, UCHAR** img_B, int nWidth, int nHeight, bool bTeach, int nInspAC = 0, int nSideCam = -1);
		static void GetColorBuffer_Window_Bottom(UCHAR** img_BR, UCHAR** img_BB, Insp_Wnd* wndBuffer);
		static void ROIImageClaculCompose(const _LightTypeBuf &sLightImg, UCHAR* ptrbyResultImage);
		static int RounD(double val);

		static cv::Mat BytesToMat(UCHAR* src, int nWidth, int nHeight, int nType);
		static int nCalcWidthStep(bool bUseWidthStep, int OrgImgWidth);
		static void ROIAnglePointChange(double dAngle, const POINTF rcSrc, POINTF *rcDst);

		//LocalResol
		static double m_resolX_Local;
		static double m_resolY_Local;
		static bool m_bUse_LocalResol;
		static ResolMap*  m_ResolMap;
		static bool ReadLocalResol(CString cPath);
		static void UseLocalResol(bool bUse);
		static void SetLocalResol(float fovX, float FovY);
		static bool GetLocalResol(float fovX, float FovY ,double& resolX, double& resolY );

		//public image bufer
		static PI_Buff* m_pImgBuffer[4];
		static bool m_bAllocImgBuffer;
		static int m_nBufferSizeX;
		static int m_nBufferSizeY;
		static void SetImgBuffer(int nWidth, int nLength);
	};

}