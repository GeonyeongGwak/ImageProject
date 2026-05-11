#pragma once


namespace csml
{
	class AFX_CLASS_EXPORT npps
	{
	public:
		static int set(float src, float * dst, int dstlength);
		static int set(int src, int * dst, int dstlength);
		static int set(BYTE src, BYTE * dst, int dstlength);
		static int set(short src, short * dst, int dstlength);

		static int mulc(float src, float * dst, int dstlength);
		static int mulc(int src, int * dst, int dstlength);
		static int mulc(BYTE src, BYTE * dst, int dstlength);
		static int mulc(short src, short * dst, int dstlength);

		static int divc(float src, float * dst, int dstlength);
		static int divc(int src, int * dst, int dstlength);
		static int divc(BYTE src, BYTE * dst, int dstlength);
		static int divc(short src, short * dst, int dstlength);

		static int addc(float src, float * dst, int dstlength);
		static int addc(int src, int * dst, int dstlength);
		static int addc(BYTE src, BYTE * dst, int dstlength);
		static int addc(short src, short * dst, int dstlength);

		static int subc(float src, float * dst, int dstlength);
		static int subc(BYTE src, BYTE * dst, int dstlength);
		static int subc(short src, short * dst, int dstlength);


	};
}

