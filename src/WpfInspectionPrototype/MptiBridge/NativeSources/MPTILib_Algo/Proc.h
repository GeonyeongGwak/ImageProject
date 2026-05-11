#pragma once


#define _center2start(cpt, length)    (cpt - (length / 2))    

//image save path
#define WORK_IMAGE_PATH    _T("d:\\testimage\\workimage")
#define WORK_IMAGE_PATH2   _T("d:\\log_temp")
//Image Type
//#define ProcTypeTIF	_T(".tif")
#define ProcTypeTIF	_T(".jpg")		//jpg 읽어오기

//////////////////////////////////////////////////////////////////////////
//max string length
#define MAX_STRLEN   256


//////////////////////////////////////////////////////////////////////////
//color ref
#define	COLOR_BLACK					M_COLOR_BLACK      
#define	COLOR_RED					M_COLOR_RED
#define	COLOR_GREEN					M_COLOR_GREEN
#define	COLOR_BLUE					M_COLOR_BLUE
#define	COLOR_YELLOW				M_COLOR_YELLOW
#define	COLOR_MAGENTA				M_COLOR_MAGENTA                          
#define	COLOR_CYAN					M_COLOR_CYAN                           
#define	COLOR_WHITE					M_COLOR_WHITE                             
#define	COLOR_GRAY					M_COLOR_GRAY                                
#define	COLOR_BRIGHT_GRAY			M_COLOR_BRIGHT_GRAY			                   
#define	COLOR_LIGHT_GRAY			M_COLOR_LIGHT_GRAY                        
#define	COLOR_LIGHT_GREEN			M_COLOR_LIGHT_GREEN                        
#define	COLOR_LIGHT_BLUE			M_COLOR_LIGHT_BLUE                        
#define	COLOR_LIGHT_WHITE			M_COLOR_LIGHT_WHITE                        
#define	COLOR_DARK_RED				M_COLOR_DARK_RED                             
#define	COLOR_DARK_GREEN			M_COLOR_DARK_GREEN                          
#define	COLOR_DARK_BLUE				M_COLOR_DARK_BLUE                        
#define	COLOR_DARK_YELLOW			M_COLOR_DARK_YELLOW                     
#define	COLOR_DARK_MAGENTA			M_COLOR_DARK_MAGENTA                     
#define	COLOR_DARK_CYAN				M_COLOR_DARK_CYAN
#define COLOR_ORANGE				M_RGB888(255,144,0)
//////////////////////////////////////////////////////////////////////////

#define	_mm2micron(val)    (val * 1000.0)
#define	_micron2mm(val)    (val / 1000.0)
// #define _pixel2mm(val)	   (val * 0.02)
// #define _mm2pixel(val)	   (val / 0.02)
#define _radian2degree(val)			(val * (180.0 / 3.14))
#define _degree2radian(val)			(val * (3.14 / 180.0))

#define _round(val,pos)		((int)((val + pow(0.1,pos)/2.0) * pow(10.0,pos))) /  pow(10.0,pos);

enum direction
{
	e_LEFT = 0,
	e_RIGHT ,
	e_TOP ,
	e_BOTTOM 
};



class CProc
{
public:
	CProc(void);
	virtual ~CProc(void);

	double m_resolX;
	double m_resolY;
	int m_fovWidth;

	CString m_className;

public:
	UCHAR* AllocUserBuff(int sizeX, int sizeY, int band = 1);
	void DeleteUserBuff(UCHAR** userBuf);

	virtual void SetResol(double resolX, double resolY, int fovWidth);

 
	double	_pixel2mm_x(double val);	 
	double	_mm2pixel_x(double val);	
	double	_pixel2mm_y(double val);	 
	double	_mm2pixel_y(double val);


	BOOL FileExists(CString strFilePath);
	
	int CorrectCoordinate(double pos_x, double pos_y, double ct_x, double ct_y, double theta, double delta_x, double delta_y, double *corr_x, double *corr_y);
	int _CorrectCoordinate(double pos_x, double pos_y, double ct_x, double ct_y, double theta, double delta_x, double delta_y, double *corr_x, double *corr_y);

	void CopyBuffer(UCHAR* &pUcDstBuffer, int nDstSizeX, int nDstSizeY, UCHAR* pUcSrcBuffer, int nSrcSizeX, int nSrcSizeY, int nOffsetX, int nOffsetY);
	void CopyBuffer_OFOV(UCHAR* &pUcDstBuffer, int nDstSizeX, int nDstSizeY, UCHAR* pUcSrcBuffer, int nOverlapSizeX, int nOverlapSizeY, int nSrcSizeX, int nSrcSizeY, int nOffsetX, int nOffsetY);
	void CopyBuffer(float* &pUcDstBuffer, int nDstSizeX, int nDstSizeY, float* pUcSrcBuffer, int nSrcSizeX, int nSrcSizeY, int nOffsetX, int nOffsetY);

	void CopyCropBuffer( UCHAR* &pUcDstBuffer, int nDstSizeX, int nDstSizeY, int nOffsetX, int nOffsetY, UCHAR* pUcSrcBuffer, int nSrcSizeX, int nSrcSttX, int nSrcSttY, int nSrcEndX, int nSrcEndY );
	void CopyCropBuffer( float* &pUcDstBuffer, int nDstSizeX, int nDstSizeY, int nOffsetX, int nOffsetY, float* pUcSrcBuffer, int nSrcSizeX, int nSrcSttX, int nSrcSttY, int nSrcEndX, int nSrcEndY );
	void CopyCropBuffer(UCHAR* &pUcDstBuffer, int nDstStX, int nDstStY, int nDstSizeX, int nDstSizeY, UCHAR* pUcSrcBuffer, int nSrcSizeX, int nSrcSizeY, int nOffsetX, int nOffsetY);
	void CopyCropBuffer(float* &pUcDstBuffer, int nDstStX, int nDstStY, int nDstSizeX, int nDstSizeY, float* pUcSrcBuffer, int nSrcSizeX, int nSrcSizeY, int nOffsetX, int nOffsetY);
	void CopyBuffer_Portion(float* &pUcDstBuffer, int nDstSizeX, int nDstSizeY, float* pUcSrcBuffer, int nCutLeftSize, int nCutTopSize, int nSrcSizeX, int nSrcSizeY, int nOffsetX, int nOffsetY, bool bLastMerge);
	void CopyBuffer_Portion(UCHAR* &pUcDstBuffer, int nDstSizeX, int nDstSizeY, UCHAR* pUcSrcBuffer, int nCutLeftSize, int nCutTopSize, int nSrcSizeX, int nSrcSizeY, int nOffsetX, int nOffsetY);
};

