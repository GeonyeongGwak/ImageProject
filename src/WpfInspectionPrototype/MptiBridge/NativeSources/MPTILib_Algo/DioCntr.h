#pragma once

enum IO_RESULT
{
	ePIR_NOT_SUPPORTED = -2,
	ePIR_UNKNOWN = -1,
	ePIR_SUCCESS = 0,
	ePIR_FAIL    = 1
};

class CDioCntr
{
public:
	CDioCntr(void) {};
	virtual ~CDioCntr(void) {};
public:
	virtual int InitDevice()=0;
	virtual int CloseDevice()=0;
	virtual int DistributeChannel()=0;
	virtual int DistributeChannel(int numInChannel, int numInModule, int *pArrNumChannelInModule, int *pArrIdxInModule, int numOutChannel, int numOutModule, int *pArrNumChannelOutModule, int *pArrIdxOutModule)=0;
	virtual int DisposeChannel()=0;
	virtual int GetCountInChannel(int *numChannel)=0;
	virtual int GetCountOutChannel(int *numChannel)=0;
	virtual int ReadInChannel(int idxChannel, bool *stat)=0;
	virtual int ReadInChannelAll(bool *pArray)=0;
	virtual int ReadOutChannel(int idxChannel, bool *stat)=0;
	virtual int ReadOutChannelAll(bool *pArray)=0;
	virtual int WriteOutChannel(int idxChannel, bool onOff)=0;
	virtual int WriteOutChannelAll(bool *pArray)=0;
};

