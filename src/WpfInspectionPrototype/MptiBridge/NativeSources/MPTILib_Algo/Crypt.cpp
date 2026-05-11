#include "StdAfx.h"
#include "Crypt.h"

const INT C1 = 52845;
const INT C2 = 22719;
const INT KEY = 78695;

CCrypt::CCrypt(void)
{

}



CCrypt::~CCrypt(void)
{

}

BOOL CCrypt::Encrypt(BYTE *source, BYTE *destination, DWORD length)
{

	DWORD i;
	INT key= KEY;

	if(!source || !destination || length <= 0)
	{
		return FALSE;
	}

	for(i=0; i<length; i++)

	{

		destination[i] = source[i]^key>>8;

		key = (destination[i]+key) * C1 + C2;

	}

	return TRUE;
}

BOOL CCrypt::Encrypt(UCHAR *source, UCHAR *destination, DWORD width, DWORD length)
{
	DWORD i;
	LONG64 key= KEY;

	if(!source || !destination || length <= 0)
	{
		return FALSE;
	}

	for(DWORD i=0; i< length; i++)
	{
		for(DWORD j = 0; j < width; j ++)
		{
			destination[i * width + j] = source[i * width + j]^key>>8;

			key = (destination[i * width + j]+key) * C1 + C2;
		}		

	}

	return TRUE;
}

BOOL CCrypt::Decrypt(BYTE *source, BYTE *destination, DWORD length)
{

	DWORD i;
	BYTE previousBlock;
	INT key = KEY;

	if(!source || !destination || length <= 0)
	{
		return FALSE;
	}	

	for(i=0; i<length; i++)
	{
		previousBlock = source[i];

		destination[i] = source[i]^key>>8;

		key = (previousBlock + key) * C1 + C2;
	}

	return TRUE;
}

BOOL CCrypt::Decrypt(UCHAR *source, UCHAR *destination, DWORD width, DWORD length)
{
	DWORD i;
	BYTE previousBlock;
	LONG64 key = KEY;

	if(!source || !destination || length <= 0)
	{
		return FALSE;
	}

	for(DWORD i =0; i<width * length; i++)
	{		
		previousBlock = source[i];

		destination[i] = source[i]^key>>8;

		key = (previousBlock + key) * C1 + C2;
	}

	return TRUE;
}

BOOL CCrypt::CheckDecrypt(UCHAR *source, UCHAR *destination, DWORD width, DWORD length)
{
	BOOL bRet = TRUE;

	for(DWORD i =0; i<width * length; i++)
	{
		if(destination[i] != source[i])
		{
			bRet = FALSE;
		}
	}

	return bRet;
}

