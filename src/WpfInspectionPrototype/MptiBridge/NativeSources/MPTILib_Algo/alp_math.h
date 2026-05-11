//////////////////////////////////////////////////////////////////////////
//
//	alp_math.h/cpp
//	- create by ATW
//
//	< history >
//	01. ATW 2013/09/23 create below
//		int alpm_lsm_line(int order, int len, float *points, float *values, float *coef);
//		int alpm_lsm_plane(int order, int len, float* ax, float* ay, float* az, float* coef);
//		bool alpm_pt_in_polygon(POINT pt, const POINT *pts, int ptNum);
//		int gauss_elimination(const float* matrixA, const float* matrixI, float* matrixB, int len);
//
//////////////////////////////////////////////////////////////////////////

#pragma once


//////////////////////////////////////////////////////////////////////////
//
// pm_lsm_line : Least square method for calculate line equation coefficients.
// Made by ATW. 2013/09/12
// coefficients index
//	: [0]x^n + [1]x^[n-1] + ... + [n-1]x^0
//
//////////////////////////////////////////////////////////////////////////
int alpm_lsm_line(int order, int len, float *points, float *values, float *coef);

//////////////////////////////////////////////////////////////////////////
//
// pm_lsm_plane : Least square method for calculate plane equation coefficients.
// Made by ATW. 2013/09/12
// coefficients index
//	: [0](x^n * y^(order-n)) + [1](x^(n-1) * y^(order-(n-1))) + ...
//
//////////////////////////////////////////////////////////////////////////
int alpm_lsm_plane(int order, int len, float* ax, float* ay, float* az, float* coef);


bool alpm_pt_in_polygon(POINT pt, const POINT *pts, int ptNum);