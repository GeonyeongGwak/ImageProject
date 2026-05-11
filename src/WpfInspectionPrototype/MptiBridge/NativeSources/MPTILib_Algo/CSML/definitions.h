#pragma once

// Visual Studio 버전 호환용 함수 정의

#ifdef _MSC_VER < 1700

#include <math.h>

#define isnan(x) _isnan(x)
#define isinf(x) (!_finite(x))
#define isfinite(x) _finite(x)

#endif

// 버퍼 생성 부 파라메터 정의
// Maximum GPU 개수 - cudnn Handle / cublas Handle / memoryMng _buffer
#define _maxGpuNum 8

// Temp Memory 개수 - memoryMng _buffer
#define _maxTempMem 10

// Maximum Thread 개수
#define _maxThreadNum 3