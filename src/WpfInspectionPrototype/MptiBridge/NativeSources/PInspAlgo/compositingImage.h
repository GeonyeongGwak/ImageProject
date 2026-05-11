#pragma once

namespace PIAL
{
	class PINSPALGO_API compositingImage
	{
	public:
		compositingImage();
		~compositingImage();


		static void ROICompose_SSE(int nMerge, std::vector<UCHAR*>& vecPtr, std::vector<float>& vecfValue,
			int nFullImageWidth, int nFullImageHeight, int nStartX, int nStartY,
			int nROIWidth, int nROIHeight, UCHAR* ptrbyResultImage);

		static void ROICompose_AVX2(int nMerge, std::vector<UCHAR*>& vecPtr, std::vector<float>& vecfValue,
			int nFullImageWidth, int nFullImageHeight, int nStartX, int nStartY,
			int nROIWidth, int nROIHeight, UCHAR* ptrbyResultImage);

		static void ROICompose_AVX512(int nMerge, std::vector<UCHAR*>& vecPtr, std::vector<float>& vecfValue,
			int nFullImageWidth, int nFullImageHeight, int nStartX, int nStartY,
			int nROIWidth, int nROIHeight, UCHAR* ptrbyResultImage);
	};

}