#pragma once
#include "opencv2/opencv.hpp"

namespace PIAL
{
	class PINSPALGO_API PI_Buff
	{
	public:
		PI_Buff();

		PI_Buff(int nWidth, int nLength, int nElementSize =1,int Channel = 1);
		PI_Buff(void* pData, int nWidth, int nLength, int nPitch,int nElementSize =1, int Channel = 1, bool bref = true);
		PI_Buff(float* pData, int nWidth, int nLength, bool bref = true);
		PI_Buff(float* pData, int nWidth, int nLength, int nPitch, bool bref = true);
		PI_Buff(const cv::Mat& mat, bool bref = true);
		PI_Buff(const PI_Buff* Buff,  RECT& roi);
		~PI_Buff();

		UCHAR* m_pData;
		
		inline int Width() {return m_Width;}
		inline int Length() { return m_Length; }
		inline int Pitch() { return m_Pitch; }
		inline int Element() { return m_Element; }
		inline int Element_Byte() { return m_Element_Byte; }
		inline bool SubImage() { return m_SubImage; }
		inline bool IsRef() { return m_IsRef; }
		inline int Channel() { return m_Channel; }

		//Reference Mat ¹ÝÈ¯
		cv::Mat Mat();

		PI_Buff* Clone();
		PI_Buff* Ref();

		bool CopyTo(PI_Buff* dst);
		bool CopyTo(PI_Buff* dst, int nStartX, int nStartY);

		PI_Buff* ClipBuff_LT(int nSX, int nSY, int roiSizeX, int roiSizeY, bool bClone = false, bool bref = true);
		PI_Buff* ClipBuff_LT(RECT rect, bool bClone = false, bool bref = true);
		PI_Buff* ClipBuff(int cx, int cy, int roiSizeX, int roisizeY, bool bref = true);
		PI_Buff Clip_Ref(RECT rect);

		void Clear();

		static bool SaveImage(PI_Buff* Buff, CString strImgPath);
		static PI_Buff* Load_Image(CString strImgPath);
	protected:
		int m_Width;
		int m_Length;

		int m_Channel;

		int m_Pitch;

		int m_Element;
		int m_Element_Byte;

		bool m_SubImage;
		bool m_IsRef;

	};

	class PINSPALGO_API PI_Buff_Float: PI_Buff
	{
	public:
		PI_Buff_Float();
		PI_Buff_Float(int nWidth, int nLength, int Channel = 1);
		PI_Buff_Float(float* pData, int nWidth, int nLength, bool bref = true);
		PI_Buff_Float(float* pData, int nWidth, int nLength, int nPitch, bool bref = true);
		~PI_Buff_Float();

	};


}

