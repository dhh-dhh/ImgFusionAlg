#pragma once
#ifndef HKCAMERA_H_  
#define HKCAMERA_H_  
#endif

#include <iostream>
#include "stdafx.h"  
#include "MvCameraControl.h"
#include "MvCamera.h"
#include "resource.h"		// Ö÷·ûºÅ
//#include "afxwin.h"
#include <Windows.h>

class HKCamera : public CMvCamera
{
public:
	HKCamera();	// Standard constructor
	~HKCamera();
	int GetImgCallBack();
	void(__stdcall* ImageCallBackEx)(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser) {};
	unsigned char* GetImgBuf() { return m_ImageBuf; };
	MV_FRAME_OUT_INFO_EX getImageInfo() { return m_stImageInfo; };

	int OpenHKCamera();
	int CloseHKCamera();
	int GetImg();
	int SetHKCamera();
	int ExposureTime = 500000;
	double Gain = 16.0;
	double t_takeimg;
	double t_transimg;
private:

	int nDataSize = 15040512;
	MV_FRAME_OUT_INFO_EX m_stImageInfo;
	unsigned char* m_ImageBuf;
	MV_FRAME_OUT stImageInfo = { 0 };

	int nRet = MV_OK;
	void* handle = NULL;
	MV_CC_DEVICE_INFO_LIST stDeviceList;

public:
	//void* m_hDevHandle;
};
