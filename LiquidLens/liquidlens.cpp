//////////////////////////////////////////////////////////////////////////  
/// 
/// 
/// All rights reserved.  
///   
/// @file    liquidlens.cpp    
/// @brief   镜头对焦类
///  
/// 本文件为镜头对焦类的实现代码  
///  
/// @version 1.0     
/// @author  段仕鹏    
/// @date    2022/01/24  
///   
///  
///  修订说明：  
//////////////////////////////////////////////////////////////////////////  
#include <iostream>
#include "liquidlens.h"
#include "stdafx.h"  
#include "SerialPort.h"  
#include <ctime>

LiquidLens::LiquidLens() {};
LiquidLens::~LiquidLens() {};
bool LiquidLens::ChangeFocals()
{
	if (LLFocalPlace > 100.0 || LLFocalPlace < 0.0)
	{
		return false;
	}
	int temp = (int)(LLFocalPlace * 1023 / 100);

	int MSB = temp / 256;//MSB最高有效位
	int LSB = temp % 256;//LSB最低有效位
	data[0] = 0x53;//OP Code
	data[1] = 0X31;//Write CMD
	data[2] = 0x01;//FunctionCMD
	data[3] = 0x02;//发送Data数量
	data[4] = LSB;//LSB最低有效位
	data[5] = MSB;//MSB最高有效位
	//int CRC = ((int)data[0] + (int)data[1] + (int)data[2] + (int)data[3] + (int)data[4] + (int)data[5]) % 10;
	int CRC = (data[0] + data[1] + data[2] + data[3] + data[4] + data[5]) & 0xff;
	data[6] = CRC;//n Max = 8 不用管 //SUM (OP ~Data n)
	data[7] = 0x3E;//END Code

	/*
	BYTE data2[8];
	data2[0] = 0x53;
	data2[1] = 0X31;
	data2[2] = 0x01;
	data2[3] = 0x02;
	data2[4] = 0x00;
	data2[5] = 0x00;
	data2[6] = 0x87;
	data2[7] = 0x3E;
	*/

	//发送信号
	if (LLSerialPort.WriteData(data, 8))
	{
		return true;
	}
	else
	{
		return false;
	}

}

bool LiquidLens::IsChangeFocals()
{
	int i = 0;
	bool isOneSignal = false;

	//加一个定时器查看多久接收到6位数据
	time_t t_beginResponse = clock();
	while (LLSerialPort.GetBytesInCOM() < 6)
	{
		if (LLSerialPort.GetBytesInCOM() == 1)
		{
			time_t t_1Respons = clock();
			time1 = (double)(t_1Respons - t_beginResponse) / CLOCKS_PER_SEC;
		}
		else if (LLSerialPort.GetBytesInCOM() == 2)
		{
			time_t t_2Respons = clock();
			time2 = (double)(t_2Respons - t_beginResponse) / CLOCKS_PER_SEC;
		}
		else if (LLSerialPort.GetBytesInCOM() == 3)
		{
			time_t t_3Respons = clock();
			time3 = (double)(t_3Respons - t_beginResponse) / CLOCKS_PER_SEC;
		}
		else if (LLSerialPort.GetBytesInCOM() == 4)
		{
			time_t t_4Respons = clock();
			time4 = (double)(t_4Respons - t_beginResponse) / CLOCKS_PER_SEC;
		}
		else if (LLSerialPort.GetBytesInCOM() == 5)
		{
			time_t t_5Respons = clock();
			time5 = (double)(t_5Respons - t_beginResponse) / CLOCKS_PER_SEC;
		}
	}
	time_t t_6Respons = clock();
	time6 = (double)(t_6Respons - t_beginResponse) / CLOCKS_PER_SEC;

	//Sleep(50);//等待串口返回信号
	int numByte = LLSerialPort.GetBytesInCOM();

	while (LLSerialPort.GetBytesInCOM()) //加上起始位与终止位
	{
		char temp;
		LLSerialPort.ReadChar(temp);
		if (temp == 0x53 && numByte >= 6)	//开始位读取成功
		{
			dataBK[i] = temp; i++;
			while ((int)temp != 0x3E && i < 5)
			{
				LLSerialPort.ReadChar(temp);
				dataBK[i] = temp;
				i++;
			}
			LLSerialPort.ReadChar(temp);
			if (temp == 0x3E)
			{
				isOneSignal = true;
				dataBK[i] = temp;
				break;
			}
		}
		else //开始位读取失败
		{
			break;
		}

	}
	if (isOneSignal &&
		dataBK[0] == 0x53 &&
		dataBK[1] == 0x31 &&
		dataBK[2] == 0x01 &&
		dataBK[3] == 0x31 &&
		dataBK[4] == 0xB6 &&
		dataBK[5] == 0x3E)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool LiquidLens::GetFirmwareInfo()
{
	bool isGetFirmwareInfo = true;
	dataInfoSend[0] = 0x53;
	dataInfoSend[1] = 0x32;
	dataInfoSend[2] = 0x01;
	dataInfoSend[3] = 0x86;
	dataInfoSend[4] = 0x3E;
	if (!LLSerialPort.WriteData(dataInfoSend, 5))
	{
		isGetFirmwareInfo = false;
	}
	int i = 0;
	//int a=LLSerialPort.GetBytesInCOM();
	while (LLSerialPort.GetBytesInCOM())
	{
		if (i >= 14) { return false; };
		char temp;
		LLSerialPort.ReadChar(temp);
		dataInfoGet[i] = temp;
		i++;
	}
	return isGetFirmwareInfo;


}