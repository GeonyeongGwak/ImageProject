#pragma once
#include <map>
#include <vector>
#include "opencv2/opencv.hpp"

enum eFilterMethod
{
	FILTER_BLUR=1,
	FILTER_GAUSSIAN,
};
namespace csml
{
	class AFX_EXT_CLASS ImageDeformation
	{

	public:
		ImageDeformation(void);
		~ImageDeformation(void);

		struct DeformProperty
		{
			// 함수 항상 적용여부
			bool shift_apply;
			bool rotation_apply;
			bool shear_apply;
			bool scale_apply;
			bool flip_horizontal_apply;
			bool flip_vertical_apply;
			bool contrast_deformation_apply;
			bool brightness_deformation_apply;
			bool saltandpepper_apply;
			bool gaussian_apply;
			bool smooth_apply;
			bool sharpening_apply;
			bool contrast_noise_apply;
			bool brightness_noise_apply;

			// 랜덤함수의 사용여부
			bool shift_apply_rand;
			bool rotation_apply_rand;
			bool shear_apply_rand;
			bool scale_apply_rand;
			bool flip_horizontal_rand;
			bool flip_vertical_rand;
			bool contrast_deformation_apply_rand;
			bool brightness_deformation_apply_rand;
			bool saltandpepper_apply_rand;
			bool gaussian_apply_rand;
			bool smooth_apply_rand;
			bool sharpening_apply_rand;
			bool contrast_noise_apply_rand;
			bool brightness_noise_apply_rand;

			// Noise
			int brightness_min;
			int brightness_max;
			double saltandpepper_min;
			double saltandpepper_max;
			double contrast_alpha_min;
			double contrast_alpha_max;
			double gaussian_mean_min;
			double gaussian_mean_max;
			double gaussian_variance_min;
			double gaussian_variance_max;
			double gaussian_alpha_min;
			double gaussian_alpha_max;
			double gaussian_beta_min;
			double gaussian_beta_max;
			double gaussian_gamma_min;
			double gaussian_gamma_max;
			int smoothType_min;
			int smoothType_max;
			int smoothFilter_min;
			int smoothFilter_max;
			int sharpeningType_min;
			int sharpeningType_max;
			int brightnesstype_min;
			int brightnesstype_max;
			int contrastnoise_min;
			int contrastnoise_max;

			// Affine Transformation
			int rotation_min;
			int rotation_max;
			float scale_x_axis_min;
			float scale_x_axis_max;
			float scale_y_axis_min;
			float scale_y_axis_max;
			float shear_x_axis_min;
			float shear_x_axis_max;
			float shear_y_axis_min;
			float shear_y_axis_max;
			float shift_x_axis_min;
			float shift_x_axis_max;
			float shift_y_axis_min;
			float shift_y_axis_max;
		};	

		std::vector<CString> targetString;
		std::vector<CString> replaceString;

		CString FrontCharacter;

		//bool roi_range_chk;  // 이미지 변환 후 Label 영역 존재여부 검사 (label 영역이 적거나 없는 경우, 변환된 이미지를 저장하지 않음)
		int erasedSample;  // Random Function으로 변형 후 NG 영역이 없어져 제거된 영상 수

		bool unsaved_Img_chk; // NG 영역이 가장자리에 있거나 없어진 영상의 저장여부
		int margin;
		int min_area_of_bubble;

		bool file_exist_chk; // 변형된 절대주소 상의 이미지 존재여부 체크

		int maxSamplingCnt_ng_roi;
		int maxSamplingCnt_good_roi;
		int maxSamplingCnt_good_fov;

	protected:
		std::vector<cv::Point2f> extremePoints;  // shear된 이미지 코너점들 (Rotation 변환 시 Crop된 영역 추출에 쓰임)
		std::vector<double> saltandpepperNoise;
		std::map<int, std::vector<double> > gaussianNoise;
		std::vector<double> contrastDeformation;
		std::vector<int> brightnessDeformation;
		std::map<int, std::vector<float> > scaleTransform;
		std::map<int, std::vector<float> > shearTransform;
		std::vector<int> rotationTransform;
		std::map<int, std::vector<int> > smooth;
		std::vector<int> sharpening;
		std::vector<int> brightnessNoise;
		std::vector<int> contrastNoise;
		std::map<int, std::vector<float> > shiftTransform;

	public:
		void SetDefaultAugProperty(DeformProperty &prop);

		// Pseudorandom Number
		int rand_Int(int min, int max);
		double rand_Double(double min, double max);
		float rand_Float(float min = 0.0f, float max = 1.0f);

		cv::Mat UpSamplingByMirroring(cv::Mat src, int width, int height);

		// Transformation - >> 보간법 - Label 영상 노이즈 발생하지 않게 고정
		cv::Mat ScaleTransform(cv::Mat src, float Bx = 1.0f, float By = 1.0f, float fx = 0.0, float fy = 0.0, int flags = cv::INTER_NEAREST);
		cv::Mat ShearTransform(cv::Mat src, float Bx = 1.0f, float By = 1.0f, int flags = cv::INTER_NEAREST);
		cv::Mat RotationTransform(cv::Mat src, int angle = 0, bool sheared_chk = false, double scale = 1.0, int flags = cv::INTER_NEAREST);
		cv::Mat ShiftTransform(cv::Mat src, float shift_rate_x, float shift_rate_y, bool bPosXDirection, bool bPosYDirection, cv::Rect ROI);

		// Deformation
		bool ElasticDeformation(cv::Mat src, int block_size = 0, int padding_width = 0, int padding_height = 0, float reposition_range = 1.0f);
		void FlipHorizontal(cv::Mat src);
		void FlipVertical(cv::Mat src);
		void BrightnessDeformation(cv::Mat src, int offset = 0);
		void ContrastDeformation(cv::Mat src, double alpha = 1.0);

		// Deformation Range Check
		std::vector<float> scaleRange(CRect FoV, CRect roi);
		std::vector<float> shearRange(CRect FoV, CRect roi);
		std::vector<int> rotationRange(CRect FoV, CRect roi);

		// Noise
		void SaltPepperNoise(cv::Mat src, double n = 0.0);
		void GaussianNoise(cv::Mat src, double average, double std, double alpha = 1.0, double beta = 0.0, double gamma = 0.0);
		void Smoothing(cv::Mat src, int smoothType = FILTER_GAUSSIAN, int filterSize_width = 3, int filterSize_height = 3);
		void Sharpening(cv::Mat src, int sharpeningType);
		void unsharpMaskFilter(cv::Mat src, int radius, int threshold, double amount);
		void BrightnessNoise(cv::Mat src, int brightnessdeformType);
		void ContrastNoise(cv::Mat src, int contrastdeformType);

		// Box Detection Label 처리 관련
		void ScaleBoxLabels(std::vector<cv::Rect> &coordis, std::vector<int> &nCls, int imgLen, int imgWid, float Bx, float By);
		void RotationBoxLabels(std::vector<cv::Rect> &coordis, std::vector<int> &cls, int imgLen, int imgWid, int angle, int flags = cv::INTER_NEAREST);
		void ShiftBoxLabels(std::vector<cv::Rect> &coordis, std::vector<int> &cls, int imgLen, int imgWid, float shift_rate_x, float shift_rate_y, bool bPosXDirection, bool bPosYDirection, cv::Rect ROI);
		void FlipBoxLabels(std::vector<cv::Rect> &coordis, int imgLen, int imgWid, bool bFlipHorizontal);

		void polynomialRegression(std::vector<double> InputPointX, std::vector<double> InputPointY, int maxDimension, cv::Mat buffer);
		float sigmoid(float x);

		CString ReplacePath(CString OldPath);
		CString ReplacePath(CString OldPath, std::vector<CString> TargetString, std::vector<CString> ReplaceString);

		// Sampling
		bool BackgroundRandomSampling(cv::Mat FoV, cv::Mat Label, int ROI_w, int ROI_h, int Good_count, CString folderPath, bool aug_chk = false, DeformProperty prop = DeformProperty());
		bool BackgroundRandomSampling(CString FoVPath, CString LabelPath, int ROI_w, int ROI_h, int Good_count, CString folderPath, bool channel_chk = false, bool aug_chk = false, DeformProperty prop = DeformProperty());
		bool BackgroundRandomSampling(CString FoVPath, cv::Mat Label, int ROI_w, int ROI_h, int Good_count, CString folderPath, bool channel_chk = false, bool aug_chk = false, DeformProperty prop = DeformProperty());

		bool BackgroundRandomSamplingwithDB(cv::Mat FoV, cv::Mat Label, CRect GOOD_rect, int ROI_w, int ROI_h, int Good_count, CString folderPath, bool aug_chk = false, DeformProperty prop = DeformProperty());
		bool BackgroundRandomSamplingwithDB(CString FoVPath, CString LabelPath, CRect GOOD_rect, int ROI_w, int ROI_h, int Good_count, CString folderPath, bool channel_chk = false, bool aug_chk = false, DeformProperty prop = DeformProperty());
		bool BackgroundRandomSamplingwithDB(CString FoVPath, cv::Mat Label, CRect GOOD_rect, int ROI_w, int ROI_h, int Good_count, CString folderPath, bool channel_chk = false, bool aug_chk = false, DeformProperty prop = DeformProperty());

		bool RandomSampling(cv::Mat Fov, cv::Mat Label, CRect NG_rect, int ROI_w, int ROI_h, int NG_count, CString folderPath, std::vector<cv::Point> &sample_Buffer, std::vector<int> &save_Buffer);
		bool RandomSampling(CString FoVPath, CString LabelPath, CRect NG_rect, int ROI_w, int ROI_h, int NG_count, CString folderPath, std::vector<cv::Point> &sample_Buffer, std::vector<int> &save_Buffer, bool channel_chk = false);
		bool RandomSampling(CString FoVPath, cv::Mat Label, CRect NG_rect, int ROI_w, int ROI_h, int NG_count, CString folderPath, std::vector<cv::Point> &sample_Buffer, std::vector<int> &save_Buffer, bool channel_chk = false);

		bool BgMixedRandomSampling(cv::Mat &ROIImg, cv::Mat FoV, CRect ROI_rect, int ROI_w, int ROI_h, int &zoom_rate, bool bOriScale, bool bg_mix, cv::Mat BgFoV); // CSML - BoxDetectionWithCadGerber
		bool BgMixedRandomSampling(cv::Mat &ROIImg, cv::Mat FoV, cv::Mat Label, CRect ROI_rect, int classNum, int ROI_w, int ROI_h, bool bFontResize, cv::Mat BgFoV, CRect BgRect); // CSML - OCR
		bool SingleRandomSampling(std::vector<cv::Mat> &_ROIImg, cv::Mat &ROILb, std::vector<cv::Mat> Fov, cv::Mat Label, CRect ROI_rect, int ROI_w, int ROI_h, bool bg_chk); // CSML - Segmentation
		bool SingleRandomSampling_B(std::vector<cv::Mat> &_ROIImg, cv::Mat &ROILb, std::vector<cv::Mat> Fov, cv::Mat Label, CRect ROI_rect, int ROI_w, int ROI_h, bool bg_chk, int &roix, int &roiy, int &roir, int &roib, bool bAug); // CSML - Segmentation

		// Sample Generation
		std::vector<std::vector<cv::Mat>> SingleSampleGenerator(cv::Mat FoV, cv::Mat Label, int ROI_w, int ROI_h, DeformProperty prop, cv::Point buffer, int count, bool &range_chk); // CSML
		bool SampleGenerator(cv::Mat FoV, cv::Mat Label, int ROI_w, int ROI_h, DeformProperty prop, std::vector<cv::Point> buffer, int count, CString folderPath, std::vector<int> save_Buffer);
		bool SampleGenerator(CString FoVPath, CString LabelPath, int ROI_w, int ROI_h, DeformProperty prop, std::vector<cv::Point> buffer, int count, CString folderPath, std::vector<int> save_Buffer, bool channel_chk = false);
		bool SampleGenerator(CString FoVPath, cv::Mat Label, int ROI_w, int ROI_h, DeformProperty prop, std::vector<cv::Point> buffer, int count, CString folderPath, std::vector<int> save_Buffer, bool channel_chk = false);
		bool SampleGenerator(DeformProperty prop, std::vector<CString> ImageList, int count, CString folderPath);
		bool SampleGenerator(DeformProperty prop, std::vector<cv::Mat> &ROIImg, cv::Mat &ROILb);  // CSML
		bool SampleGenerator(DeformProperty prop, std::vector<cv::Mat> &ROIImg, cv::Mat &ROILb, std::vector<cv::Rect> &boxParams, std::vector<cv::Rect> &goodParams, std::vector<int> &nClsBubble, std::vector<int> &nClsGood);  // CSML - Box Augmentation
		bool SampleGenerator(DeformProperty prop, std::vector<cv::Mat> &ROIImg, cv::Mat &ROILb, std::vector<cv::Rect> &boxParams, std::vector<int> &boxClass); // CSML - BoxDetectionWithCadGerber
		bool SampleGenerator_Coil(DeformProperty prop, std::vector<cv::Mat> &ROIImg, cv::Mat &ROILb);  // CSML - Coil Classification
		cv::Mat bukToMat(CString longPath);
		cv::Mat putToMat(CString longPath, int ch = 0);
		void putTobmp(CString longPath);
	};
}