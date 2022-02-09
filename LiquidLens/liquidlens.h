#pragma once
#ifndef LIQUIDLENS_H_  
#define LIQUIDLENS_H_  
#endif

#include <iostream>
#include "stdafx.h"  
#include "SerialPort.h"  

class LiquidLens
{
public:
	LiquidLens();
	~LiquidLens();
	CSerialPort LLSerialPort;
	inline void SetFocalPlace(double focalplace) { LLFocalPlace = focalplace; };//…Ë÷√∂‘ΩπŒª÷√0-100
	bool ChangeFocals();
	bool IsChangeFocals();
	BYTE dataBK[6];
	bool GetFirmwareInfo();
	BYTE dataInfoGet[14];
	double time1, time2, time3, time4, time5, time6;
private:
	BYTE data[8];
	double LLFocalPlace = 1.0;

	BYTE dataInfoSend[5];

};
