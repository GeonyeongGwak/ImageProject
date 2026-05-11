#pragma once

#include "mat_type.h"
#include "ANN_type.h"



namespace csml
{
	class AFX_EXT_CLASS factory
	{
	public:
		static layer::ptr input(CString sInsSel, CString sOutName, CString sWeight, CString sBias, int neurons, eInitializer selInit = eInitializer::InitHe);
		static layer::ptr input(std::vector<CString> sInsSel, CString sOutName, CString sWeight, CString sBias, int neurons, eInitializer selInit = eInitializer::InitHe);

		static layer::ptr gan_input(std::vector<CString> sInsSel, CString sOutName);
		static layer::ptr gan_input(CString sInsSel, CString sOutName);

		static layer::ptr affine(std::vector<CString> sInsSel, CString sOutName, CString sWeight, CString sBias, int neurons, eInitializer selInit = eInitializer::InitHe);
		static layer::ptr affine(CString sInsSel, CString sOutName, CString sWeight, CString sBias, int neurons, eInitializer selInit = eInitializer::InitHe);
		
		static layer::ptr relu(std::vector<CString> sInsSel, CString sOutName, float alpha = 1.0f);
		static layer::ptr relu(CString sInsSel, CString sOutName, float alpha = 1.0f);
		
		static layer::ptr lrelu(std::vector<CString> sInsSel, CString sOutName, float alpha = 1.0f);
		static layer::ptr lrelu(CString sInsSel, CString sOutName, float alpha = 1.0f);

		static layer::ptr lrelu_t(std::vector<CString> sInsSel, CString sOutName, CString sOrgName, float alpha = 1.0f);
		static layer::ptr lrelu_t(CString sInsSel, CString sOutName, CString sOrgName, float alpha = 1.0f);

		static layer::ptr upsample(std::vector<CString> sInsSel, CString sOutName, UINT iScale);
		static layer::ptr upsample(CString sInsSel, CString sOutName, UINT iScale);

		static layer::ptr tanh(CString sInsSel, CString sOutName);
		static layer::ptr tanh(std::vector<CString> sInsSel, CString sOutName);

		static layer::ptr sigmoid(CString sInsSel, CString sOutName);
		static layer::ptr sigmoid(std::vector<CString> sInsSel, CString sOutName);

		static layer::ptr sigmoidOut(CString sInsSel, CString sOutName, loss_func::ptr lossF);
		static layer::ptr sigmoidOut(std::vector<CString> sInsSel, CString sOutName, loss_func::ptr lossF);
		
		static layer::ptr softmax(CString sInsSel, CString sOutName, loss_func::ptr lossF, int nOutCh);
		static layer::ptr softmax(std::vector<CString> sInsSel, CString sOutName, loss_func::ptr lossF, int nOutCh);
		
		static layer::ptr mse(CString sInsSel, CString sOutName, loss_func::ptr loss);
		static layer::ptr mse(std::vector<CString> sInsSel, CString sOutName, loss_func::ptr loss);

		static layer::ptr mse_yolo(CString sInsSel, CString sOutName, loss_func::ptr loss, int GL, int GW, float LamdaC, float LamdaWH);
		static layer::ptr mse_yolo(std::vector<CString> sInsSel, CString sOutName, loss_func::ptr loss, int GL, int GW, float LamdaC, float LamdaWH);

		static layer::ptr batchNorm(CString sInsSel, CString sOutName, CString sMean, CString sVar, CString sGamma, CString sBeta);
		static layer::ptr batchNorm(std::vector<CString> sInsSel, CString sOutName, CString sMean, CString sVar, CString sGamma, CString sBeta);

		static layer::ptr mse_yolov2(CString sInsSel, CString sOutName, loss_func::ptr loss, int GL, int GW, float LamdaC, float LamdaWH, int nBoxes, int nClasses);
		static layer::ptr mse_yolov2(std::vector<CString> sInsSel, CString sOutName, loss_func::ptr loss, int GL, int GW, float LamdaC, float LamdaWH, int nBoxes, int nClasses);

		static layer::ptr lrn(CString sInsSel, CString sOutName, unsigned int n=5.0, double alpha=1e-4, double beta=0.75, double k=2.0);
		static layer::ptr lrn(std::vector<CString> sInsSel, CString sOutName, unsigned int n=5.0, double alpha=1e-4, double beta=0.75, double k=2.0);

		static layer::ptr lrn_manual(CString sInsSel, CString sOutName, unsigned int n = 5.0, double alpha = 1e-4, double beta = 0.75, double k = 2.0);
		static layer::ptr lrn_manual(std::vector<CString> sInsSel, CString sOutName, unsigned int n = 5.0, double alpha = 1e-4, double beta = 0.75, double k = 2.0);

		static layer::ptr dropout(CString sInsSel, CString sOutName, float ratio);
		static layer::ptr dropout(std::vector<CString> sInsSel, CString sOutName, float ratio);

		static layer::ptr conv(CString sInsSel, CString sOutName, CString sFilter, CString sBias, 
								int fx, int fy, int strideX, int strideY, int dilateX, int dilateY, int nOutCh, int nGroup = 1,
								eInitializer selInit = eInitializer::InitHe, ePad bZeroPadding= ePad::Same, eFusedActiMode actFuncMode = eFusedActiMode::NONE);
		static layer::ptr conv(std::vector<CString> sInsSel, CString sOutName, CString sFilter, CString sBias, 
								int fx, int fy, int strideX, int strideY, int dilateX, int dilateY, int nOutCh, int nGroup = 1,
								eInitializer selInit = eInitializer::InitHe, ePad bZeroPadding= ePad::Same, eFusedActiMode actFuncMode = eFusedActiMode::NONE);

		static layer::ptr conv_transpose(CString sInsSel, CString sOutName, CString sFilter, CString sBias, 
										int fx, int fy, int strideX, int strideY, int nInCh, int nGroup = 1, bool bUpdateParam = true, bool bUseBias = false, 
										eInitializer selInit = eInitializer::InitHe, ePad bZeroPadding = ePad::Same, eOdd bOdd_h = eOdd::Even, eOdd bOdd_w = eOdd::Even);
		static layer::ptr conv_transpose(std::vector<CString> sInsSel, CString sOutName, CString sFilter, CString sBias, 
										int fx, int fy, int strideX, int strideY, int nInCh, int nGroup = 1, bool bUpdateParam = true, bool bUseBias = false, 
										eInitializer selInit = eInitializer::InitHe, ePad bZeroPadding = ePad::Same, eOdd bOdd_h = eOdd::Even, eOdd bOdd_w = eOdd::Even);

		static layer::ptr pool(CString sInsSel, CString sOutName, 
								int fx, int fy, int strideX, int strideY, 
								ePad bZeroPadding = ePad::NoPad, ePool nMode = ePool::MaxPool);
		static layer::ptr pool(std::vector<CString> sInsSel, CString sOutName, 
								int fx, int fy, int strideX, int strideY, 
								ePad bZeroPadding = ePad::NoPad, ePool nMode = ePool::MaxPool);

		static layer::ptr gap(CString sInsSel, CString sOutName);
		static layer::ptr gap(std::vector<CString> sInsSel, CString sOutName);

		static layer::ptr unpool(CString sInsSel, CString sOutName, CString sRefSel, 
								int fx, int fy, int strideX, int strideY, 
								ePad bZeroPadding = ePad::NoPad, eOdd bOdd_h = eOdd::Even, eOdd bOdd_w = eOdd::Even, ePool nMode = ePool::MaxPool);
		static layer::ptr unpool(std::vector<CString> sInsSel, CString sOutName, CString sRefSel, 
								int fx, int fy, int strideX, int strideY, 
								ePad bZeroPadding = ePad::NoPad, eOdd bOdd_h = eOdd::Even, eOdd bOdd_w = eOdd::Even, ePool nMode = ePool::MaxPool);
		
		static layer::ptr roipool(CString sInsSel, CString sOutName,
			int szx, int szy);
		static layer::ptr roipool(std::vector<CString> sInsSel, CString sOutName,
			int szx, int szy);

		//layer for multiscale gradient gan 200618 kyh
		static layer::ptr combine(std::vector<CString> sInsSel, CString sInsSel2, CString grad2Name, CString sOutName);
		static layer::ptr combine(CString sInsSel, CString sInsSel2, CString grad2Name, CString sOutName);

		static layer::ptr combine_t(std::vector<CString> sInsSel, CString sInsSel2, CString sOutName);
		static layer::ptr combine_t(CString sInsSel, CString sInsSel2, CString sOutName);

		static layer::ptr rpn(std::vector<CString> sInsSel, CString sName, int nAnchors);
		static layer::ptr move_gpu(CString sInsSel, CString sName, int prev_gpuid, int next_gpuid);
		static layer::ptr move_gpu(std::vector<CString> sInsSel, CString sName, int prev_gpuid, int next_gpuid);
		static layer::ptr RpnConv(CString sInsSel, CString sName, int nAnchors);
		static layer::ptr RpnConv(std::vector<CString> sInsSel, CString sName, int nAnchors);
		static layer::ptr RCNNLabelCreate(CString sInsSel, CString sName, CString sFeature, int nAnchors);
		static layer::ptr RCNNLabelCreate(std::vector<CString> sInsSel, CString sName, CString sFeature, int nAnchors);
		static layer::ptr FRCNN(CString sInsSel, CString sName, int nAnchors);
		static layer::ptr FRCNN(std::vector<CString> sInsSel, CString sName, int nAnchors);
		static layer::ptr softmax_Focal(CString sInsSel, CString sName, loss_func::ptr lossF, int nOutCh);
		static layer::ptr softmax_Focal(std::vector<CString> sInsSel, CString sName, loss_func::ptr lossF, int nOutCh);
		
		static layer::ptr channelScale(CString sInsSel, CString sScaleSel, CString sOutName);
		static layer::ptr channelScale(std::vector<CString> sInsSel, std::vector<CString> sScaleSel, CString sOutName);
		
		static layer::ptr linear(CString sInsSel, CString sResultSel, CString sOutName, loss_func::ptr lossF);
		static layer::ptr linear(std::vector<CString> sInsSel, CString sResultSel, CString sOutName, loss_func::ptr lossF);
		static layer::ptr gan_operator(CString sInsSel, CString sOutName, loss_func::ptr lossF);
		static layer::ptr gan_operator(std::vector<CString> sInsSel, CString sOutName, loss_func::ptr lossF);

		static layer::ptr alloc(CString sLayerName, archive & ar, loss_func::ptr loss = loss_func::ptr());
		static layer::ptr input(archive & ar);
		static layer::ptr gan_input(archive & ar);
		static layer::ptr affine(archive & ar);
		static layer::ptr relu(archive & ar);
		static layer::ptr lrelu(archive & ar);
		static layer::ptr lrelu_t(archive & ar);
		static layer::ptr upsample(archive & ar);
		static layer::ptr tanh(archive & ar);
		static layer::ptr sigmoid(archive & ar);
		static layer::ptr sigmoidOut(archive & ar, loss_func::ptr loss);
		static layer::ptr softmax(archive & ar, loss_func::ptr loss);
		static layer::ptr mse(archive & ar, loss_func::ptr loss);
		static layer::ptr mse_yolo(archive & ar, loss_func::ptr loss);
		static layer::ptr mse_yolov2(archive & ar, loss_func::ptr loss);
		static layer::ptr batchNorm(archive & ar);
		static layer::ptr lrn(archive & ar);
		static layer::ptr lrn_manual(archive & ar);
		static layer::ptr dropout(archive & ar);
		static layer::ptr conv(archive & ar);
		static layer::ptr conv_transpose(archive & ar);
		static layer::ptr pool(archive & ar);
		static layer::ptr gap(archive & ar);
		static layer::ptr unpool(archive & ar);
		static layer::ptr move_gpu(archive & ar);
		static layer::ptr softmax_Focal(archive & ar, loss_func::ptr loss);
		static layer::ptr RpnConv(archive & ar, loss_func::ptr loss);
		static layer::ptr RCNNLabelCreate(archive & ar, loss_func::ptr loss);
		static layer::ptr channelScale(archive & ar);
		static layer::ptr combine(archive & ar);
		static layer::ptr combine_t(archive & ar);
		static layer::ptr linear(archive & ar, loss_func::ptr loss);
	};
}


