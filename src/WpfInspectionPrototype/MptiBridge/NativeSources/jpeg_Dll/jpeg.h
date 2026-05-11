/*
//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//    Copyright (c) 2001-2012 Intel Corporation. All Rights Reserved.
//
*/

#pragma once

#include "jpgHeader.h"

# ifdef JPG_EXPORTS
#  define JPGIPP_API __declspec(dllexport)
# else 
#  define JPGIPP_API __declspec(dllimport)
# endif

#define MAX_JPEG_COMMENT_BUF 128

// SHKang 2017/04/05
enum jpeg_formatType
{ 
	BaseLine = 1, 
	LossLess = 4 
};

enum jpeg_formatQuality
{ 
	BELOW_NORMAL = 90, 
	NORMAL = 95, 
	ABOVE_NORMAL = 100
};

typedef struct _PARAMS_JPEG
{
	int           nthreads;
	int           quality;
	int           restart_interval;
	int           huffman_opt;
	int           point_transform;
	int           predictor;
	int           use_qdct;
	int           dct_scale;
	int           tmode;
	int           mode;
	IM_COLOR      color;
	IM_SAMPLING   sampling;
	int           comment_size;
	UCHAR         comment[MAX_JPEG_COMMENT_BUF];


	_PARAMS_JPEG()
	{
		nthreads = 0;
		restart_interval = 0;
		huffman_opt = 0;
		point_transform = 0;
		predictor = 0;
		use_qdct = 0;
		dct_scale = 0;
		tmode = 0;
		quality = 75;
		color = IC_GRAY;
		mode = BaseLine;
		sampling = IS_444;
		comment_size = MAX_JPEG_COMMENT_BUF;
		for (int i = 0; i < MAX_JPEG_COMMENT_BUF; i++)
			comment[i] = 0;

	}

} PARAMS_JPEG;


typedef enum _PRO_ERROR
{
	PROCESS_OK = 1,
	PROCESS_SIZE = -1,	// uData size or image size to error
	PROCESS_PITCH = -2,	// udata size and pitch size to error 
	PROCESS_LOAD_PATH = -3,	// udata size and pitch size to error 
	PROCESS_INIT = -4	//init Error
} PRO_ERROR;

class JPGIPP_API CJPEG
{
public:

	CJPEG();
	~CJPEG();

	PRO_ERROR PIL_Save_Jpeg(UCHAR * ucSrc, int width, int height, int nChannel, _IM_COLOR imColor, CString  file_path, int nJpegQuality);

	PRO_ERROR PIL_Save_Jpeg_8(UCHAR * ucSrc, int width, int height, CString  file_path);
	PRO_ERROR PIL_Save_Jpeg_Color(UCHAR * uData_R, UCHAR * uData_G,  UCHAR * uData_B, int width, int height, CString  file_path);

	void init_Param(jpeg_formatType type, jpeg_formatQuality quality);
	PRO_ERROR PIL_Load_JPEG_8(UCHAR * uData, CString  file_path, int width, int height);
	PRO_ERROR PIL_Load_JPEG_Color(UCHAR * uData_R, UCHAR * uData_G, UCHAR * uData_B, CString  file_path, int width, int height);

	PARAMS_JPEG params;

};

