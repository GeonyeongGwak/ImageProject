#include "StdAfx.h"
#include "Binarize.h"

namespace jsl
{
	namespace Binarize
	{
		Less::Less(cv::Mat& src, cv::Mat& des, int th)
		{
			cv::threshold(src, des, th-1, 255, cv::THRESH_BINARY_INV);
		}

		LessEqual::LessEqual(cv::Mat& src, cv::Mat& des, int th)
		{
			cv::threshold(src, des, th, 255, cv::THRESH_BINARY_INV);
		}

		Greater::Greater(cv::Mat& src, cv::Mat& des, int th)
		{
			cv::threshold(src, des, th, 255, cv::THRESH_BINARY);
		}

		GreaterEqual::GreaterEqual(cv::Mat& src, cv::Mat& des, int th)

		{
			cv::threshold(src, des, th-1, 255, cv::THRESH_BINARY);
		}

		InRange::InRange(cv::Mat& src, cv::Mat& des, int thL, int thH)
		{
			cv::Mat tmp(src.rows, src.cols, src.type());
			cv::threshold(src, tmp, thL-1, 255, cv::THRESH_BINARY);
			cv::threshold(src, des, thH, 255, cv::THRESH_BINARY_INV);
			cv::bitwise_and(tmp, des, des);
		}

		OutRange::OutRange(cv::Mat&src, cv::Mat& des, int thL, int thH)
		{
			cv::Mat tmp(src.rows, src.cols, src.type());
			cv::threshold(src, tmp, thL-1, 255, cv::THRESH_BINARY_INV);
			cv::threshold(src, des, thH, 255, cv::THRESH_BINARY);
			cv::bitwise_or(tmp, des, des);
		}
	}
}
