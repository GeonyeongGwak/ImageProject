#pragma once
#include "ExtInspRoot.h"
#include "InspParamDef.h"
#include "PInsp_Algo/InspParamDef_Algo.h"
#include "./opencv2/opencv.hpp"
namespace ext {
namespace Integ {

	typedef struct _tagParamHead
	{
		QWORD zero;
		QWORD qwOffset;
		QWORD sizebytes;
		_tagParamHead()
		{
			zero = qwOffset = sizebytes = 0;
		}
	}tagParamHead;

	typedef struct _tagPartHead
	{
		QWORD zero;
		QWORD sizebytes;
		_tagPartHead()
		{
			zero = sizebytes = 0;
		}
	}tagPartHead;

	typedef struct _tagImgHead
	{
		int nImgWidth;
		int nImgHeight;
		_tagImgHead()
		{
			nImgWidth = 0;
			nImgHeight = 0;
		}
	}tagImgHead;

	class aAlgo
	{
	public:
		aAlgo();
		virtual bool Init();
		virtual void Exit();

		QWORD GetSize(InspPartInfo * PartData);
		void Add(InspPartInfo * PartData, BYTE * pBuf);
		void FromWnd(InspPartInfo * PartData, BYTE * pBuf);
		virtual void Clear();

	protected:
		int _AlgoClone(InspAlgoType eAlgoType, LPVOID lpSource, LPVOID& lpTarget);
		int _AlgoSize(InspAlgoType eAlgoType);
	};
	class Windows
	{
	public:
		Windows();

		virtual bool Init();
		QWORD GetSize(InspPartInfo * PartData);
		void FromPart(InspPartInfo * PartData, BYTE * pBuf);

		void Add(InspPartInfo * PartData, BYTE * pBuf);

		aAlgo _Algos;

	protected:

	};
	class Part
	{
	public:
		Part();
		virtual bool Init();
		virtual QWORD GetSize(InspPartInfo * PartData);
		QWORD AddPart(InspPartInfo * PartData, InspPartParam * pWind, int nWndCnt);

		Windows _Windows;

	protected:
		
		QWORD Add(InspPartInfo * PartData);
		CSharedMemory _stream;
		QWORD _qwOffsetBytes;
	};


	class PartBuff
	{
	public:
		PartBuff();
		virtual bool Init();
		void AddPart(InspPartInfo * PartData, InspPartParam * pWind, int nWndCnt);
		void GetPart(InspPartInfo * PartData, InspPartParam * pWind);
		void SetBuffIdx(int nIdx) { nBuffIdx = nIdx; };
		int AlgoSize(InspAlgoType eAlgoType);
		int AlgoClone(InspAlgoType eAlgoType, LPVOID& lpSource, BYTE* dst);
	protected:

		bool Add(InspPartInfo * PartData);
		CSharedMemory _stream;
		QWORD _qwCurrentPos;
		int nBuffIdx;
	};
	class RstBuff
	{
	public:
		RstBuff();
		virtual bool Init();
		void AddRst(InspectionResult * PartData, int nWndCnt);
		void GetRst(InspectionResult * PartData, int nWndCnt);
		void SetBuffIdx(int nIdx) { nBuffIdx = nIdx; };
	protected:
		void AddRstAlgo(InspAlgoResult* AlgoData);
		CSharedMemory _stream;
		QWORD _qwCurrentPos;
		int nBuffIdx;
	};
	class ImgBuff	//(8 channel + 3D)
	{
	public:
		ImgBuff();
		virtual bool Init();
		bool AddImage(InspRoiImgBuf * partImgBuf, ZmapData*  partZmapData);
		bool GetImage(InspRoiImgBuf * partImgBuf, ZmapData*  partZmapData);

		UCHAR GetPartImage(InspRoiImgBuf* partImgBuf, ZmapData*  partZmapData);
		void SetBuffIdx(int nIdx) { nBuffIdx = nIdx; };
	protected:

		CSharedMemory _stream;
		QWORD _qwCurrentPos;
		int nBuffIdx;
	};
	class LightBuff
	{
	public:
		LightBuff();
		virtual bool Init();
		bool AddLight(InspPartParam * pParamArray, int nWndCnt);
		bool GetLight(InspPartParam * pParamArray, int nWndCnt);
		void SetBuffIdx(int nIdx) { nBuffIdx = nIdx; };


	protected:
		ExtAlgoLight Add(InspAlgo * pAlgo, BYTE* ptr, int WndID, int AlgoID);
		void Get(InspAlgo * pAlgo);
		CSharedMemory _stream;
		QWORD _qwCurrentPos;
		int nBuffIdx;
		ExtAlgoLight stCurLightBuffer;
	};

	class InspBuffer	//All buffer 
	{
	public:
		InspBuffer();
		virtual bool Init();
		void SetBuffIdx() 
		{
			for (int i = 0; i < EXT_BUFFER_CNT; i++)
			{
				_part[i].SetBuffIdx(i);
				_img[i].SetBuffIdx(i);
				_rst[i].SetBuffIdx(i);
				_light[i].SetBuffIdx(i);
			}
		};
	protected:
		PartBuff _part[EXT_BUFFER_CNT];
		ImgBuff _img[EXT_BUFFER_CNT];
		RstBuff _rst[EXT_BUFFER_CNT];
		LightBuff _light[EXT_BUFFER_CNT];
		int nBuffIdx;
	};

	class ClientCtrl
	{
	public:
		ClientCtrl();

		bool Init(int nTool_id);
		void Exit();

		void Execute();


		Ctrlee  _ctrl;

		InspBuffer _inspbuffer;

//		std::vector<PartBuff> vPartBuff;
//		std::vector<ImgBuff> vImgBuff;
//		std::vector<RstBuff> vRstBuff;
//;

		int _nTool_id;

	protected:
	};
}
}