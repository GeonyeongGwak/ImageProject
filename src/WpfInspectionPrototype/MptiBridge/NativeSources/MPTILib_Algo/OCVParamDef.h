#pragma once

#define MAX_POCR_DIVISITION_CNTS	16
#define MAX_POCR_PARAM_CNT 6
enum POCRFont
{
	_0, _1, _2,
	_3, _4, _5,
	_6, _7, _8, _9,
	A, B, C, D, E, F,
	G, H, I, J, K, L,
	M, N, O, P, Q, R,
	S, T, U, V, W, X,
	Y, Z,
	a, b, c, d, e, f,
	g ,h ,i ,j ,k ,l,
	m, n, o, p, q, r,
	s, t, u, v, w, x,
	y, z,
	SMall,
	Num
};

enum POCRFontType
{
	eVerdana,
	eHaettenschweiler,
	eSegoe_UI_Light,
	eSegoe_Print,
	eSegoe_WP_Black,
	ePalatino_Linotype,
	eDokChampa,
	eNum,
};

enum DivisionArea
{
	eHeight_2 = 0,
	eWidth_2,
	eLattice_4,
	eLattice_9,
	eLattice_6,
	eWidth_3,
	eHeight_3,
	eLatticeH_6,
	None,
};

enum POCRAlgoType
{
	eDefault,
	eLowContrast,
	eIntagrio,
	eRelief,
	eImage,
	eImageMin,
};

enum RequiredType
{
	RequiredType_eNone,	// 일반 영역
	RequiredType_eOK,	// 필수검사 OK면 OK
	RequiredType_eNG,	// 미검영역
};


typedef struct tagPOCRInspParam
{
public:
	POCRFontType eFontType;

	DivisionArea eDivisionArea;
	int nDivisionCnt;
	int nRequiredSocre;

	POCRAlgoType ePOCRAlgoType[MAX_POCR_DIVISITION_CNTS];
	int nDivisionScore[MAX_POCR_DIVISITION_CNTS];
	RequiredType eRequiredArea[MAX_POCR_DIVISITION_CNTS];

	tagPOCRInspParam()
	{
		eFontType = eVerdana;

		eDivisionArea = eHeight_2;
		nDivisionCnt = 2;
		nRequiredSocre = 0;

		for (int i = 0; i < MAX_POCR_DIVISITION_CNTS; i++)
		{
			ePOCRAlgoType[i] = eDefault;
			nDivisionScore[i] = 0;
			eRequiredArea[i] = RequiredType_eNone;
		}
	}
}POCRInspParam;

class cPOCRInspParam : public tagPOCRInspParam
{
public:
	cPOCRInspParam()
	{
	}
	cPOCRInspParam(cPOCRInspParam & lhs)
	{
		init();
		alloc();
		copy(&lhs);
	}
	cPOCRInspParam(tagPOCRInspParam & lhs)
	{
		init();
		alloc();
		copy(&lhs);
	}
	virtual ~cPOCRInspParam()
	{
		free();
	}
	const cPOCRInspParam & operator=(const cPOCRInspParam & lhs)
	{
		alloc();
		copy(&lhs);

		return *this;
	}
	const cPOCRInspParam & operator=(const tagPOCRInspParam & lhs)
	{
		alloc();
		copy(&lhs);

		return *this;
	}
	void copy(const tagPOCRInspParam * lhs)
	{
		eFontType = lhs->eFontType;

		eDivisionArea = lhs->eDivisionArea;
		nDivisionCnt = lhs->nDivisionCnt;
		nRequiredSocre = lhs->nRequiredSocre;

		for (int i = 0; i < MAX_POCR_DIVISITION_CNTS; i++)
		{
			ePOCRAlgoType[i] = lhs->ePOCRAlgoType[i];
			nDivisionScore[i] = lhs->nDivisionScore[i];
			eRequiredArea[i] = lhs->eRequiredArea[i];
		}
	}
	void alloc()
	{
	}
	void free()
	{
	}
	void init()
	{
		eFontType = eVerdana;

		eDivisionArea = eHeight_2;
		nDivisionCnt = 2;
		nRequiredSocre = 0;

		for (int i = 0; i < MAX_POCR_DIVISITION_CNTS; i++)
		{
			ePOCRAlgoType[i] = eDefault;
			nDivisionScore[i] = 0;
			eRequiredArea[i] = RequiredType_eNone;
		}
	}
};

typedef struct tagPOCRRstInspTeach
{
	int nRstScore;

	int nDivisionScore[MAX_POCR_DIVISITION_CNTS];

	tagPOCRRstInspTeach()
	{
		nRstScore = 0;
		for (int i = 0; i < MAX_POCR_DIVISITION_CNTS; i++)
			nDivisionScore[i] = 0;
	}
}POCRRstInspTeach;

typedef struct tagPOCRInspResult
{
	int nInspCnt;

	POCRRstInspTeach* vPOCRRstInspTeach;

	tagPOCRInspResult()
	{
		nInspCnt = 0;
		vPOCRRstInspTeach = NULL;
	}

	~tagPOCRInspResult()
	{
		Destroy();
	}

	void Destroy()
	{
		if (vPOCRRstInspTeach)
		{
			delete[] vPOCRRstInspTeach;
			//g_pMManager->pem_delete(vPOCRRstInspTeach, true);
			vPOCRRstInspTeach = NULL;
		}
		nInspCnt = 0;
	}
}POCRInspResult;

typedef struct tagPOCRFontFile
{
	UCHAR * m_FontData;

	int nFileSize;
}POCRFontFile;

typedef struct tagPOCRRstPtr
{
	POCRRstInspTeach* vPOCRRstPtr;
}POCRRstPtr;

typedef struct tagPOCRInspPtr
{
	POCRInspParam* vPOCRInspPtr;
}POCRInspPtr;

class cPOCRInspPtr : public tagPOCRInspPtr
{
public:
	cPOCRInspPtr()
	{
		init();
	}
	cPOCRInspPtr(cPOCRInspPtr & lhs)
	{
		init();
		alloc();
		copy(&lhs);
	}
	cPOCRInspPtr(tagPOCRInspPtr & lhs)
	{
		init();
		alloc();
		copy(&lhs);
	}
	virtual ~cPOCRInspPtr()
	{
		free();
	}
	const cPOCRInspPtr & operator=(const cPOCRInspPtr & lhs)
	{
		alloc();
		copy(&lhs);

		return *this;
	}
	const cPOCRInspPtr & operator=(const tagPOCRInspPtr & lhs)
	{
		alloc();
		copy(&lhs);

		return *this;
	}
	void copy(const tagPOCRInspPtr * lhs)
	{
		*_vPOCRInspPtr = *lhs->vPOCRInspPtr;
		vPOCRInspPtr = _vPOCRInspPtr;
	}
	void alloc()
	{
		free();
		_vPOCRInspPtr = new cPOCRInspParam;
		//_vPOCRInspPtr = g_pMManager->pem_new<cPOCRInspParam>(false, 0, (PCHAR)__FUNCTION__, __LINE__);

		vPOCRInspPtr = _vPOCRInspPtr;
	}
	void free()
	{
		if(_vPOCRInspPtr)
			delete _vPOCRInspPtr;
			//g_pMManager->pem_delete(_vPOCRInspPtr, false);
		_vPOCRInspPtr = nullptr;
		vPOCRInspPtr = nullptr;
	}
	void init()
	{
		_vPOCRInspPtr = nullptr;
		vPOCRInspPtr = nullptr;
	}

	cPOCRInspParam * _vPOCRInspPtr;
};

typedef struct tagPOCRInfoParam
{
	POCRFont eFontKey;

	int nPOCRInspCnt;
	POCRInspPtr* vPOCRInspPtr;
}POCRInfoParam;

class cPOCRInfoParam : public tagPOCRInfoParam
{
public:
	cPOCRInfoParam()
	{
		init();
	}
	cPOCRInfoParam(cPOCRInfoParam & lhs)
	{
		init();
		alloc(lhs.nPOCRInspCnt);
		copy(&lhs);
	}
	cPOCRInfoParam(tagPOCRInfoParam & lhs)
	{
		init();
		alloc(lhs.nPOCRInspCnt);
		copy(&lhs);
	}
	virtual ~cPOCRInfoParam()
	{
		free();
	}
	const cPOCRInfoParam & operator=(const cPOCRInfoParam & lhs)
	{
		alloc(lhs.nPOCRInspCnt);
		copy(&lhs);

		return *this;
	}
	const cPOCRInfoParam & operator=(const tagPOCRInfoParam & lhs)
	{
		alloc(lhs.nPOCRInspCnt);
		copy(&lhs);

		return *this;
	}
	void copy(const tagPOCRInfoParam * lhs)
	{
		eFontKey = lhs->eFontKey;

		nPOCRInspCnt = lhs->nPOCRInspCnt;
		for (int i=0; i<nPOCRInspCnt; i++)
			_vPOCRInspPtr[i] = lhs->vPOCRInspPtr[i];
	}
	void alloc(int cnt)
	{
		free();
		_vPOCRInspPtr = new cPOCRInspPtr[cnt];
		//_vPOCRInspPtr = g_pMManager->pem_new<cPOCRInspPtr>(true, cnt, (PCHAR)__FUNCTION__, __LINE__);
		vPOCRInspPtr = _vPOCRInspPtr;
		nPOCRInspCnt = cnt;
	}
	void free()
	{
		if(_vPOCRInspPtr)
			delete [] _vPOCRInspPtr;
			//g_pMManager->pem_delete(_vPOCRInspPtr, true);
		_vPOCRInspPtr = nullptr;
		vPOCRInspPtr = nullptr;
		nPOCRInspCnt=0;
	}
	void init()
	{
		_vPOCRInspPtr = nullptr;
		vPOCRInspPtr = nullptr;
		nPOCRInspCnt = 0;
		eFontKey = POCRFont::Num;
	}

	cPOCRInspPtr* _vPOCRInspPtr;
};

typedef struct tagPOCRDataParam
{
	POCRInfoParam* vFontParam;
}POCRDataParam;
