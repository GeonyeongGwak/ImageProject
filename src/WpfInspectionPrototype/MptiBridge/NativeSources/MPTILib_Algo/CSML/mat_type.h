#pragma once

#include <memory>
#include <vector>
#include "_array.h"


namespace csml
{
	class AFX_EXT_CLASS Mat //: public array<float>
	{
// 	public:
// 		typedef std::shared_ptr<Mat> Ptr;
// 
// 		typedef enum
// 		{
// 			DATA_FLOAT = 0,
// 			DATA_DOUBLE = 1,
// 			DATA_HALF = 2,
// 			DATA_INT8 = 3,
// 			DATA_INT32 = 4
// 		} DataType_t;
// 
// 		typedef enum {
// 			OP_N = 0,
// 			OP_T = 1,
// 			OP_C = 2
// 		} Operation_t;
// 		
// 	public:
// 		virtual void create(Ptr lhs) = 0;
// 		virtual bool create(size_t n, size_t c, size_t h, size_t w)
// 		{
// 			return array<float>::create(n, c, h, w);
// 		}
// 		virtual void release()
// 		{
// 			return array<float>::release();
// 		}
// 
// 		virtual Ptr clone() = 0;
// 		virtual void copyTo(Ptr & lhs) = 0;
// 
// 		virtual void Serialize(archive& ar) = 0;
// 
// 		virtual int rows() = 0;
// 		virtual int cols() = 0;
// 
// 		virtual Ptr roi(int n, int size) = 0;
// 		virtual Ptr roi(int n, int c, int size) = 0;
// 		virtual Ptr roi(int n, int c, int h, int size) = 0;
// 
// 		virtual void * descriptor() = 0;
// 		virtual DataType_t type() = 0;
// 
// 		virtual float operator =(float val) = 0;
// 		virtual void operator *=(float val) = 0;
// 		virtual void operator +=(float val) = 0;
// 		virtual void operator -=(float val) = 0;
// 		virtual void operator /=(float val) = 0;
// 
// 	protected:
// 		virtual void init()
// 		{
// 			return array<float>::init();
// 		}


	public:
		typedef enum {
			OP_N = 0,
			OP_T = 1,
			OP_C = 2
		} Operation_t;
		typedef enum OP_MODE
		{
			ADD = 0,
			MUL = 1,
			MIN = 2,
			MAX = 3,
			SQRT = 4,
			NOT = 5
		}OP_MODE;
	public:
		static bool mul(const cuFloat::ptr & x, const cuFloat::ptr & y, cuFloat::ptr & z);
		static bool gemm(const cuFloat::ptr & A, Operation_t opa, float a, const cuFloat::ptr & B, Operation_t opb, cuFloat::ptr & C, float b, cuFloat::eAccess acc = cuFloat::Device);
		static bool geam(const cuFloat::ptr & A, Operation_t opa, float a, const cuFloat::ptr & B, Operation_t opb, cuFloat::ptr & C, float b);
		static bool gemv(const cuFloat::ptr & A, Operation_t opa, float a, const cuFloat::ptr & x, cuFloat::ptr & y, float b);
		static bool axpy(const cuFloat::ptr & x, float a, cuFloat::ptr & y);
		static bool sqrt(const cuFloat::ptr & x, cuFloat::ptr & y);
		static bool reciprocal(const cuFloat::ptr & x, cuFloat::ptr & y);

		static bool img2mat(cuByte::ptr src, cuFloat::ptr & des, int cIdx, int RoiWid, int RoiLen, int cntX, int cntY);
		static bool fovNpts2mat(cuByte::ptr src, int pitch, cuFloat::ptr & des, int dci, cuInt::ptr pt_x, cuInt::ptr pt_y, int RoiWid, int RoiLen, cuByte::ptr workspace, int preprocess_chk = 0);
		static bool fov2mat(cuByte::ptr src, int pitch, cuFloat::ptr & des, int n, int c, int offset_x, int offset_y);
		static bool mat2fov(const cuFloat::ptr src, int ImgWid, int ImgLen, int ImgPitch, int gap_x, int gap_y, cuByte::ptr & dst, int dh, int dw, cuFloat::ptr workspace, int binary_threshold = 240);
		static bool sumofarray(const cuFloat::ptr src, int size, const cuFloat::ptr dst);

		static bool mul_s(const cuShort::ptr & x, const cuShort::ptr & y, cuShort::ptr & z);
		static bool gemm_s(const cuShort::ptr & A, Operation_t opa, short a, const cuShort::ptr & B, Operation_t opb, cuShort::ptr & C, short b, cuShort::eAccess acc = cuShort::Device);
		static bool geam_s(const cuShort::ptr & A, Operation_t opa, short a, const cuShort::ptr & B, Operation_t opb, cuShort::ptr & C, short b);
		static bool gemv_s(const cuShort::ptr & A, Operation_t opa, short a, const cuShort::ptr & x, cuShort::ptr & y, short b);
		static bool axpy_s(const cuShort::ptr & x, short a, cuShort::ptr & y);
		static bool sqrt_s(const cuShort::ptr & x, cuShort::ptr & y);
		static bool reciprocal_s(const cuShort::ptr & x, cuShort::ptr & y);

		static bool img2mat_s(cuByte::ptr src, cuShort::ptr & des, int cIdx, int RoiWid, int RoiLen, int cntX, int cntY);
		static bool fovNpts2mat_s(cuByte::ptr src, int pitch, cuShort::ptr & des, int dci, cuInt::ptr pt_x, cuInt::ptr pt_y, int RoiWid, int RoiLen, cuByte::ptr workspace, int preprocess_chk = 0);
		static bool fov2mat_s(const cuByte::ptr src, int pitch, cuShort::ptr & des, int n, int c, int offset_x, int offset_y);
		static bool mat2fov_s(const cuShort::ptr src, int ImgWid, int ImgLen, int ImgPitch, int gap_x, int gap_y, cuByte::ptr & dst, int dh, int dw, cuShort::ptr workspace, int binary_threshold = 240);

		static void opTensor(void* in1, void* in2, void* out, Mat::OP_MODE opMode, int n, int c, int h, int w );
	};
}