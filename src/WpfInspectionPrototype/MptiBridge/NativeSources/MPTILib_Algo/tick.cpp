#include "stdafx.h"
#include "tick.h"

tick::tick()
{
	QueryPerformanceFrequency(&_freq);
	_result = 0.0;
}

tick::~tick()
{
}

void tick::start()
{
	QueryPerformanceCounter(&_start);
}

double tick::current()
{
	QueryPerformanceCounter(&_end);
	return (double)(_end.QuadPart - _start.QuadPart) / ((double)(_freq.QuadPart) * 0.001);
}

double tick::end()
{
	QueryPerformanceCounter(&_end);
	_result = (double)(_end.QuadPart - _start.QuadPart) / ((double)(_freq.QuadPart) * 0.001);

	return _result;
}

