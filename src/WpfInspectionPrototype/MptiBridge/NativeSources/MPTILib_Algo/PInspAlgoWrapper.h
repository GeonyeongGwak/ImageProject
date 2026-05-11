#pragma once

#include "InspParamDef.h"
#include "../PInspAlgo/PInspalgo_Def.h"
#include "../PInspAlgo/PInspAlgo_Lib.h"
#include "../PInspAlgo/PInspAlgo.h"
#include "../PInspAlgo/PAlgo.h"
//#include "../PInspAlgo/PI_Buff.h"
#include "../PInspAlgo/PInspData.h"
#include "../PInspAlgo/FiducialCoord.h"
// #include "../PInspAlgo/PInspalgo_Def.h"
// #include "../PInspAlgo/PInspAlgo_Lib.h"
// #include "../PInspAlgo/PI_Buff.h"
// #include "../PInspAlgo/PAlgo.h"
 
//#include "../PInspalgo/PTeaching.h"
//#include "../PInspAlgo/PInspAlgo_WireBonding.h"
// #include "../PInspAlgo/PI_Geo.h"
// #include "../PInspAlgo/PInspData.h"
#include "../PInspAlgo/compositingImage.h"
#include "New_Barcode/Barcode_Def.h"
#ifdef _DEBUG
#pragma comment(lib, "../PInspAlgo/DEBUG/PInspAlgoD.lib")
#pragma comment(lib, "New_Barcode/Debug/Barcode_x64d.lib")
#else
#pragma comment(lib, "../PInspAlgo/Release/PInspAlgo.lib")
#pragma comment(lib, "New_Barcode/Release/Barcode_x64.lib")
#endif 
#define MAX(a, b)     ( (a) > (b) ? (a) : (b) )
#define MIN(a, b)     ( (a) < (b) ? (a) : (b) )
#define ABS(a)        ( (a) > 0 ? (a) : (-(a)) )
#define SWAP(a,b)     ( (a) ^= (b), (b) ^= (a), (a) ^= (b) )

#define ASCII2HEX(a)  ( (a) - ((a) > '9' ? ('A' - 10) : '0') )
#define HEX2ASCII(b)  ( (b) + ((b) >  9  ? ('A' - 10) : '0') )
#define UPPERCASE(c)  ( (c) - ( ('a'<=(c) && (c) <='z') ? ('a' - 'A') : 0 ) )     
#define LOWERCASE(c)  ( (c) + ( ('A'<=(c) && (c) <='Z') ? ('a' - 'A') : 0 ) )

#define ARRAY_SIZE(array)   ( sizeof(array) / sizeof(array[0]) )

#define devide_hbyte(uchar_data) (uchar_data & 0xf0) >> 4
#define devide_lbyte(uchar_data) (uchar_data & 0x0f)

#define make_hbyte(uchar_data) (uchar_data & 0xf0) >> 4
#define make_lbyte(uchar_data) (uchar_data & 0x0f)

#define make_h2byte(word_data) (word_data & 0xff00) >> 8
#define make_l2byte(word_data) (word_data & 0x00ff)

#define char_to_hex(cdata) (cdata > '9') ? (cdata-'A'+0x0A) : (cdata-'0')		// 'A' => 0x0A
#define hex_to_char(hdata) (hdata > 9) ? (hdata+('A'-0x0A)) : (hdata+'0')		// 0x0A => 'A'

class PInspAlgoWrapper
{
public:
	PInspAlgoWrapper();
	~PInspAlgoWrapper();

private:

public:
	int m_fovWidth;
	int m_fovLength;
	//double m_resolX;
	//double m_resolY;
	int m_nCompositeLightMode;
	bool m_bFastImageCompose;

	PIAL::PInspAlgo* m_PInspAlgo;
	// PIAL::PTeaching* m_PTeaching;
	// PIAL::PInspAlgo_WireBonding* m_PInspWire;

	void SetResolution(int fovWidth, int fovLength, double resolX, double resolY);
	void SetCompositeLightMode(int LightMode, float compoBtmR, float compoBtmG, float compoBtmB);
	void SetFittingValue(float r, float g, float b, float br, float bb);
	void SetPartImage(InspPartInfo partInfo, PIAL::Insp_Image* pImg_buf);
	PIAL::Img_Channel ConvertIntToChannel(int nLightType);


	bool MakeBuffWnd(WndAlgoImg* sClipWndAlgoImg, WndInfo sWndInfo, PIAL::Insp_Image*& pImg_buf, bool bref = true);
    bool MakeBuffWnd_Clip(WndAlgoImg* sWndAlgoImg, WndInfo sWndInfo, PIAL::Insp_Image*& pImg_buf);
	bool MakeROIImg(InspRoiImgBuf* ptrAlgoColorOpt, PIAL::Insp_ROIImg*&  InspImageData); //Wire
	bool MakeROIImg(AlgoColorOpt* ptrAlgoColorOpt, PIAL::Insp_ROIImg*&  InspImageData);

	bool ConvertAlgo(AlgoBlob* algo, PIAL::_AlgoBlob& algo2);
	bool ConvertAlgo(AlgoBlob* algo, PIAL::_AlgoBump& algo2);
	bool ConvertAlgo(AlgoBump* algo, PIAL::_AlgoBump& algo2);
	bool ConvertAlgo(AlgoBGA* algo, PIAL::_AlgoBGA& algo2);

	bool ConvertAlgo(AlgoNGBlob* algo, PIAL::_AlgoNGBlob& algo2);
	bool ConvertAlgo(AlgoTilt* algo, PIAL::_AlgoTilt& algo2);
	bool ConvertAlgo(AlgoColor* algo, PIAL::_AlgoColor& algo2);

	bool ConvertAlgo(AlgoBW* algo, PIAL::_AlgoBW& rstAlgo);
	void ConvertAlgo(AlgoBW* org, PIAL::_AlgoBlackWhite* rst);
	void ConvertAlgo(AlgoFoot* org, PIAL::_AlgoFoot& rst);
	void ConvertAlgo(AlgoWire* org, PIAL::_AlgoWire& rst);
	void ConvertAlgo(AlgoGrayMean* org, PIAL::_AlgoGrayMean& rst);
	void ConvertAlgo(AlgoHeightMean* org, PIAL::_AlgoHeightMean& rst);
	void ConvertAlgo(AlgoGrayDiff* org, PIAL::_AlgoGrayDiff& rst);
	void ConvertAlgo(AlgoPadBW* algo, PIAL::_AlgoPadBW& algo2);		//NYJ 2021/04/21
	void ConvertAlgo(AlgoHeightDiff* org, PIAL::_AlgoHeightDiff& rst);
	bool ConvertAlgo(AlgoBodyBlob* algo, PIAL::_AlgoBodyBlob& algo2);
	bool ConvertAlgo(AlgoPatternDiff* algo, PIAL::_AlgoPatternDiff& algo2);
	bool ConvertAlgo(AlgoShapeX* algo, PIAL::_AlgoShapeX& algo2);
	bool ConvertAlgo(AlgoPadArray* algo, PIAL::_AlgoPadArray& algo2);
	//bool ConvertAlgo(AlgoPackageThickness* algo, PIAL::_AlgoPackageThickness& algo2);

	bool ConvertRstAlgo(PIAL::_RstAlgoBlob rst2, RstAlgoBlob* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoBump rst2, RstAlgoBlob* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoBump rst2, RstAlgoBump* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoBGA rst2, RstAlgoBGA* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoNGBlob rst2, RstAlgoNGBlob* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoTilt rst2, RstAlgoTilt* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoColor rst2, RstAlgoColor* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoBW rst2, RstAlgoBlackWhite* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoFoot rst2, RstAlgoFoot* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoWire rst2, RstAlgoWire* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoGrayMean rst2, RstAlgoGrayMean* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoHeightMean rst2, RstAlgoHeightMean* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoGrayDiff rst2, RstAlgoGrayDiff* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoHeightDiff rst2, RstAlgoHeightDiff* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoPadBW rst2, RstAlgoPadBW* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoBodyBlob rst2, RstAlgoBodyBlob* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoPatternDiff rst2, RstAlgoPatternDiff* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoPadArray rst2, RstAlgoPadArray* rst);
	bool ConvertRstAlgo(PIAL::_RstAlgoShapeX* rst, RstAlgoShapeX* org);
	//bool ConvertRstAlgo(PIAL::_RstAlgoPackageThickness rst2, RstAlgoPackageThickness* rst);

	bool ConvertAlgo(AlgoAlign* algo, PIAL::_AlgoAlign& algo2);
	bool ConvertRstAlgo(PIAL::_RstAlgoAlign rst2, RstAlgoAlign* rst);
	bool ConvertAlgo(AlgoEdge* algo, PIAL::_AlgoEdge& algo2);
	bool ConvertRstAlgo(PIAL::_RstAlgoEdge rst2, RstAlgoEdge* rst);
	bool ConvertAlgo(AlgoLine* algo, PIAL::_AlgoLine& algo2);
	bool ConvertRstAlgo(PIAL::_RstAlgoLine rst2, RstAlgoLine* rst);
	bool ConvertAlgo(AlgoAlignEdge* algo, PIAL::_AlgoAlignEdge& algo2);
	bool ConvertRstAlgo(PIAL::_RstAlgoAlignEdge rst2, RstAlgoAlignEdge* rst);

	bool ConvertAlgo(AlgoBodyEdge* algo, PIAL::_AlgoBodyEdge& algo2);
	bool ConvertRstAlgo(PIAL::_RstAlgoBodyEdge rst2, RstAlgoBodyEdge* rst);

	bool ConvertRstAlgo(AlignResult* org, PIAL::_AlignResult* dst);

//  	bool ConvertAlgo(AlgoGWire* algo, PIAL::_AlgoGWire& algo2);
//  	bool ConvertAlgo(AlgoGWire* algo, PIAL::_AlgoBodyEdge& algo2);
//  	bool ConvertRstAlgo(PIAL::_RstAlgoGWire rst2, RstAlgoGWire* rst);

	bool ConvertAlgo(AlgoPOCR* algo, PIAL::_AlgoPOCR& algo2);
	bool ConvertRstAlgo(PIAL::_RstAlgoPOCR rst2, RstAlgoPOCR* rst);

	//bool ConvertAlgo(AlgoPatternDiff* algo, PIAL::_AlgoPatternDiff& algo2);
	//bool ConvertRstAlgo(PIAL::_RstAlgoPatternDiff rst2, RstAlgoPatternDiff* rst);

	bool ConvertAlignResult(AlignResult* org, int nNum,std::vector<PIAL::_AlignResult>& dst);

	bool ConvertColorBase(tagAlgoColorBase* org, PIAL::_tagAlgoColorBase* rst);
	bool ConvertAngleColorBase(tagAngleColorBase* org, PIAL::_tagAngleColorBase* rst);

	bool ConvertBlobBase(tagAlgoBlobBase* org, PIAL::_tagAlgoBlobBase* rst);
	bool ConvertExceptROI(InspAlgo* org, PIAL::_tagTotalInspExceptArea* rst, float nAngle);
	void ConvertExceptROI(PIAL::_tagTotalInspExceptArea* exceptROI, InspPartInfo* InspBoardInfo, InspPartParam partParam, PIAL::PInspData* InspData = nullptr);

	bool ConvertWindowExceptROI(InspPartParam* partParam, int nWndindex, PIAL::_tagTotalInspExceptArea* rst);
	bool ConvertShapeXROI(ShapeXROI* org, PIAL::_ShapeXROI* rst);


	void RoiImageCompose_LT(UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR* ptrbyWhiteBuffer,
		int nFullImageSize, int nFullImageWidth, int nFullImageHeight,
		int nROIImageSize, double dROIX, double dROIY, int nROIWidth, int nROIHeight,
		int nRedValue, int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage);

	void RoiImageCompose_LT(UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR* ptrbyWhiteBuffer,
		int nFullImageSize, int nFullImageWidth, int nFullImageHeight, int nROIImageSize,
		double dROIStartX, double dROIStartY, int nROIWidth, int nROIHeight, int nRedValue,
		int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage, double dAngle);

	void ROIImageClaculCompose(const LightTypeBuf &sLightImg, UCHAR* ptrbyResultImage, double dAngle);

	void ROIImageClaculCompose(const LightTypeBuf &sLightImg, UCHAR* ptrbyResultImage);

	bool AlgoImageCompose(InspPartInfo* InspPartInfo, InspAlgoType nAlgoType, InspAlgo& sInspAlgo,UCHAR* ptr2D, int width, int height, int wnd_w, int wnd_h, double dX, double dY);

	bool NormalImageCompose(InspPartInfo* InspPartInfo, InspAlgoType nAlgoType, InspAlgo& sInspAlgo, PIAL::Insp_Image* pImg_buf, PIAL::PI_Buff* rstBuf, int width, int height, int wnd_w, int wnd_h, double dX, double dY, int nSideCameraIndex, AlignResult* ptrAlignRes, int nAlignCnt);
	bool AlgoImageMixCompose(InspPartInfo* InspPartInfo, const InspAlgoLight &algoLight, UCHAR* ptr2D, int width, int height, int wnd_w, int wnd_h, double dX, double dY);

	bool WndSizeChange(PIAL::Insp_Image* pImg_buf, InspAlgo sInspAlgo, /*InspAlgoType nAlgoType,*/ AlgoCoordinate &coordinateAlgo, InspPartInfo* InspBoardInfo, InspPartParam* pParamArray, int nWndIndex, int &nWidth, int &nHeight, double &dX, double &dY, AlignResult * curAlignRes, int &nOffX_pix, int &nOffY_pix);

	void ColorDataInput(InspAlgo sInspAlgo, PIAL::Insp_Image* pImg_buf, int nWidth, int nHeight, RECT rtClip, byte byIDX, int nSideCameraIndex);

	void SetColorData(BOOL bAngleColor, int nWidth, int nHeight, PIAL::Insp_Image* pImg_buf, int nType, RECT rtClip, int nSideCameraIndex, bool bUseAI, byte byColorLightType);

	bool SetWindowImage(InspPartInfo* InspPartInfo, PIAL::Insp_Image* pImg_buf, RECT rtWnd);

	bool GetAlgoImage(InspAlgo* sInspAlgo, InspPartInfo* InspPartInfo, PIAL::Insp_Image* pImg_buf, RECT rtWnd, int nSideCameraType);



	bool WndSizeChange_ArrAlign(PIAL::Insp_Image* pImg_buf, InspAlgoType nAlgoType, AlgoCoordinate &coordinateAlgo, InspPartInfo* InspBoardInfo, InspPartParam* pParamArray
		, int nCurWndIndex, int &nWidth, int &nHeight, double &dX, double &dY, AlignResult * curAlignRes, int nWndinspType, InspAlgo sInspAlgo, int nAlignTotalCnt);

	bool WindowRotateState();
	void SetWindowRotateState(bool value);

	void SetBodyBlobValue(double dSerachPer_Default, double dSerachPer_BodyTip, int nPoint, int nBodyBlobOPT);

	void RoiImageCompose_LT_SIMD(UCHAR* ptrbyRedBuffer, UCHAR* ptrbyGreenBuffer, UCHAR* ptrbyBlueBuffer, UCHAR* ptrbyWhiteBuffer,
		int nFullImageSize, int nFullImageWidth, int nFullImageHeight,
		int nROIImageSize, double dROIX, double dROIY, int nROIWidth, int nROIHeight,
		int nRedValue, int nGreenValue, int nBlueValue, int nWhiteValue, UCHAR* ptrbyResultImage);

	void ROIImageClaculCompose_SIMD(const LightTypeBuf &sLightImg, UCHAR* ptrbyResultImage);

	void SetBGAOffSet(float fBallHeightOffSet, float fBallWidthOffSet, float fBumpXOffSet, float fBumpYOffSet, float fCoplOffSet);

	bool SetGolden_KTL(PIAL::_AlgoBGA& algo2);
	bool ConvertAlgo(PIAL::_InspRstPolyAlgo* pRstPoly, InspRstPolyAlgo* pRstPoly2);
};

