#include "stdafx.h"
#include "hkcamera.h"
#include "MvCamera.h"
#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include "MvCameraControl.h"
#include <ctime>

//#include <windows.h>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

HKCamera::HKCamera()
{

	return;
}

HKCamera::~HKCamera()
{

	return;
}

int HKCamera::SetHKCamera()
{
	//设置曝光时间
	nRet = MV_CC_SetFloatValue(handle, "ExposureTime", ExposureTime);
	//设置增益
	nRet = MV_CC_SetFloatValue(handle, "Gain", Gain);
	return nRet;
}

int HKCamera::OpenHKCamera()
{
	do
	{

		//nRet = MV_CC_SetEnumValue(handle, "PixelFormat", 0x02180015);//设置图像格式
		if (MV_OK != nRet)
		{
			//printf("Enum Devices fail! nRet [0x%x]\n", nRet);
			break;
		}
		// Enum device

		memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
		nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
		if (MV_OK != nRet)
		{
			//printf("Enum Devices fail! nRet [0x%x]\n", nRet);
			break;
		}

		if (stDeviceList.nDeviceNum > 0)
		{
			for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
			{
				//printf("[device %d]:\n", i);
				MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
				if (NULL == pDeviceInfo)
				{
					break;
				}
			}
		}
		else
		{
			//printf("Find No Devices!\n");
			break;
		}

		//printf("Please Input camera index:");
		unsigned int nIndex = 0;
		//scanf_s("%d", &nIndex);

		if (nIndex >= stDeviceList.nDeviceNum)
		{
			//printf("Input error!\n");
			break;
		}

		// Select device and create handle
		nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
		if (MV_OK != nRet)
		{
			//printf("Create Handle fail! nRet [0x%x]\n", nRet);
			break;
		}

		// Open device
		nRet = MV_CC_OpenDevice(handle);
		if (MV_OK != nRet)
		{
			//printf("Open Device fail! nRet [0x%x]\n", nRet);
			break;
		}

		// Detection network optimal package size(It only works for the GigE camera)
		if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
		{
			int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
			if (nPacketSize > 0)
			{
				nRet = MV_CC_SetIntValue(handle, "GevSCPSPacketSize", nPacketSize);
				if (nRet != MV_OK)
				{
					//printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet);
				}
			}
			else
			{
				//printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
			}
		}

		// Set trigger mode as off //off是内触发 实时视频流和单帧采集 //on是外触发 包含软触发与硬触发
		nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF);

		//***************************************************** 软触发设置开始 ******************************************************************//

		nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_ON);

		if (MV_OK != nRet)
		{
			//printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
			break;
		}

		nRet = MV_CC_SetEnumValue(handle, "TriggerSource", MV_TRIGGER_SOURCE_SOFTWARE);
		if (MV_OK != nRet)
		{
			//printf("Set TriggerSource fail! nRet [0x%x]\n", nRet);
			break;
			//return nRet;
		}
		/*
		// execute command
		nRet = MV_CC_SetCommandValue(handle, "TriggerSoftware");
		if (MV_OK != nRet)
		{
			//printf("Execute TriggerSoftware fail! nRet [0x%x]\n", nRet);
			break;
			//return nRet;
		}
		*/
		//***************************************************** 软触发设置结束 ******************************************************************//


		// Register image callback //可以注释掉
		//nRet = MV_CC_RegisterImageCallBackForBGR(handle, ImageCallBackEx, handle);
		//if (MV_OK != nRet)
		//{
		//	//printf("Register Image CallBack fail! nRet [0x%x]\n", nRet);
		//	//return nRet;
		//}

		// Start grab image
		nRet = MV_CC_StartGrabbing(handle);
		if (MV_OK != nRet)
		{
			//printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
			break;
		}

	} while (0);

	return nRet;

}

int HKCamera::GetImg()
{
	time_t t_begintakeimg = clock();//开始拍摄

	//软件触发 开始拍摄一张图
	nRet = MV_CC_SetCommandValue(handle, "TriggerSoftware");
	if (MV_OK != nRet)
	{
		//printf("Execute TriggerSoftware fail! nRet [0x%x]\n", nRet);
		//break;
		return nRet;
	}

	time_t t_endtakeimg = clock();//结束拍摄
	t_takeimg = (double)(t_endtakeimg - t_begintakeimg) / CLOCKS_PER_SEC;//拍摄用时


	//IN OUT MV_FRAME_OUT_INFO_EX* m_stImageInfo;
	memset(&m_stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));//开辟内存空间
	m_ImageBuf = (unsigned char*)malloc(sizeof(unsigned char) * nDataSize);//开辟内存空间

	//time_t t_1 = clock();//结束拍摄
	//double t_openspace = (double)(t_1 - t_endtakeimg) / CLOCKS_PER_SEC;//拍摄用时

	nRet = MV_CC_GetImageForBGR(handle, m_ImageBuf, nDataSize, &m_stImageInfo, 2000); //因为图像转换成BGR24格式有耗时
	time_t t_endsaveimg = clock();//结束存图
	t_transimg = (double)(t_endsaveimg - t_endtakeimg) / CLOCKS_PER_SEC;//存图用时


	return nRet;
}

int HKCamera::CloseHKCamera()
{
	do {
		// Stop grab image
		nRet = MV_CC_StopGrabbing(handle);
		if (MV_OK != nRet)
		{
			//printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
			break;
		}

		// Unregister image callback
		nRet = MV_CC_RegisterImageCallBackEx(handle, NULL, NULL);
		if (MV_OK != nRet)
		{
			//printf("Unregister Image CallBack fail! nRet [0x%x]\n", nRet);
			break;
		}

		// Close device
		nRet = MV_CC_CloseDevice(handle);
		if (MV_OK != nRet)
		{
			//printf("Close Device fail! nRet [0x%x]\n", nRet);
			break;
		}

		// Destroy handle
		nRet = MV_CC_DestroyHandle(handle);
		if (MV_OK != nRet)
		{
			//printf("Destroy Handle fail! nRet [0x%x]\n", nRet);
			break;
		}
	} while (0);

	if (nRet != MV_OK)
	{
		if (handle != NULL)
		{
			MV_CC_DestroyHandle(handle);
			handle = NULL;
		}
	}
	return 0;
}

int HKCamera::GetImgCallBack()
{
	//	int nRet = MV_OK;
	//	void* handle = NULL;
	do
	{
		// Enum device
		MV_CC_DEVICE_INFO_LIST stDeviceList;
		memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
		nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
		if (MV_OK != nRet)
		{
			//printf("Enum Devices fail! nRet [0x%x]\n", nRet);
			break;
		}

		if (stDeviceList.nDeviceNum > 0)
		{
			for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
			{
				//printf("[device %d]:\n", i);
				MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
				if (NULL == pDeviceInfo)
				{
					break;
				}
			}
		}
		else
		{
			//printf("Find No Devices!\n");
			break;
		}

		//printf("Please Input camera index:");
		unsigned int nIndex = 0;
		//scanf_s("%d", &nIndex);

		if (nIndex >= stDeviceList.nDeviceNum)
		{
			//printf("Input error!\n");
			break;
		}

		// Select device and create handle
		nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
		if (MV_OK != nRet)
		{
			//printf("Create Handle fail! nRet [0x%x]\n", nRet);
			break;
		}

		// Open device
		nRet = MV_CC_OpenDevice(handle);
		if (MV_OK != nRet)
		{
			//printf("Open Device fail! nRet [0x%x]\n", nRet);
			break;
		}

		// Detection network optimal package size(It only works for the GigE camera)
		if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
		{
			int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
			if (nPacketSize > 0)
			{
				nRet = MV_CC_SetIntValue(handle, "GevSCPSPacketSize", nPacketSize);
				if (nRet != MV_OK)
				{
					//printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet);
				}
			}
			else
			{
				//printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
			}
		}

		// Set trigger mode as off
		nRet = MV_CC_SetEnumValue(handle, "TriggerMode", MV_TRIGGER_MODE_OFF);
		if (MV_OK != nRet)
		{
			//printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
			break;
		}

		// Register image callback
		nRet = MV_CC_RegisterImageCallBackForBGR(handle, ImageCallBackEx, handle);
		if (MV_OK != nRet)
		{
			//printf("Register Image CallBack fail! nRet [0x%x]\n", nRet);
			break;
		}

		// Start grab image
		nRet = MV_CC_StartGrabbing(handle);
		if (MV_OK != nRet)
		{
			//printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
			break;
		}

		//nRet=MV_CC_GetImageBuffer(handle, &stImageInfo, 1000);

		//IN OUT MV_FRAME_OUT_INFO_EX* m_stImageInfo;
		memset(&m_stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
		m_ImageBuf = (unsigned char*)malloc(sizeof(unsigned char) * nDataSize);

		nRet = MV_CC_GetImageForBGR(handle, m_ImageBuf, nDataSize, &m_stImageInfo, 1000);

		//printf("Press a key to stop grabbing.\n");

		// Stop grab image
		nRet = MV_CC_StopGrabbing(handle);
		if (MV_OK != nRet)
		{
			//printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
			break;
		}

		// Unregister image callback
		nRet = MV_CC_RegisterImageCallBackEx(handle, NULL, NULL);
		if (MV_OK != nRet)
		{
			//printf("Unregister Image CallBack fail! nRet [0x%x]\n", nRet);
			break;
		}

		// Close device
		nRet = MV_CC_CloseDevice(handle);
		if (MV_OK != nRet)
		{
			//printf("Close Device fail! nRet [0x%x]\n", nRet);
			break;
		}

		// Destroy handle
		nRet = MV_CC_DestroyHandle(handle);
		if (MV_OK != nRet)
		{
			//printf("Destroy Handle fail! nRet [0x%x]\n", nRet);
			break;
		}
	} while (0);

	if (nRet != MV_OK)
	{
		if (handle != NULL)
		{
			MV_CC_DestroyHandle(handle);
			handle = NULL;
		}
	}

	return 0;

}

