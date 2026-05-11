//////////////////////////////////////////////////////////////////////////
//
//	alp_alloc.h/cpp
//	- create by ATW
//
//	< history >
//	01. 01. ATW 2013/09/23 create below
//		<template> alpa_make(delete)_array_1d/2d/3d
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#define _ALP_ALLOC_H_


template <class T>
void alpa_make_array_3d(T**** array, int x, int y, int z, int iv=0)
{
	*array = new T**[x];

	for(int i=0; i<x; i++)
	{
		(*array)[i] = new T*[y];

		for(int j=0; j<y; j++)
		{
			(*array)[i][j] = new T[z];
			memset(***array, iv, sizeof(T)*z);
		}
	}
}

template <class T>
void alpa_delete_array_3d(T**** array, int x, int y)
{
	if(*array)
	{           
		for(int i=0; i<x; i++ )
		{
			for(int j=0; j<y; j++)
			{
				delete []((*array)[i][j]);
			}        
		}

		for(int i=0; i<x; i++)
		{
			delete []((*array)[i]);
		}

		delete [] (*array);
		array = NULL;
	}
}

template <class T>
void alpa_make_array_2d(T*** array, int x, int y, int iv=0)
{
	*array = new T*[x];

	for(int i=0; i<x; i++)
	{
		(*array)[i] = new T[y];
		memset(**array, iv, sizeof(T)*y);
	}
}

template <class T>
void alpa_delete_array_2d(T*** array, int x)
{
	if(*array)
	{           
		for(int i=0; i<x; i++)
		{
			delete []((*array)[i]);
		}

		delete [] (*array);
		*array = NULL;
	}
}

template <class T>
void alpa_make_array_1d(T** array, int x, int iv=0)
{
	*array = new T[x];
	memset(*array, iv, sizeof(T)*x);
}

template <class T>
void alpa_delete_array_1d(T** array)
{
	if(*array)
	{
		delete [] (*array);
		*array = NULL;
	}
}
