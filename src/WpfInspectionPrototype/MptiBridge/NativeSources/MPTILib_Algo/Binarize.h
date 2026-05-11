#pragma once

#include <opencv2/opencv.hpp>

namespace jsl
{
	namespace Binarize
	{
		class Less
		{
		public:
			Less(cv::Mat& src, cv::Mat& des, int th);
			virtual ~Less() {}
		};

		class LessEqual
		{
		public:
			LessEqual(cv::Mat& src, cv::Mat& des, int th);
			virtual ~LessEqual() {}
		};

		class Greater
		{
		public:
			Greater(cv::Mat& src, cv::Mat& des, int th);
			virtual ~Greater() {}
		};

		class GreaterEqual
		{
		public:
			GreaterEqual(cv::Mat& src, cv::Mat& des, int th);
			virtual ~GreaterEqual() {}
		};

		class InRange
		{
		public:
			InRange(cv::Mat& src, cv::Mat& des, int thL, int thH);
			virtual ~InRange() {}
		};

		class OutRange
		{
		public:
			OutRange(cv::Mat& src, cv::Mat& des, int thL, int thH);
			virtual ~OutRange() {}
		};
	}
}

