#include <iostream>
#include "stdafx.h"  
#include "SerialPort.h"  
#include "liquidlens.h"
#include <vector>
#include <fstream>
#include <ctime>
#include "MvCameraControl.h"
#include "MvCamera.h"
#include "hkcamera.h"
#include <windows.h>
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>  
#include "cvalgorithm.h"
#include <time.h>
#include <algorithm>

using namespace cv;
using namespace std;



int main()
{ 

	//double cameraExposureTime = 0.5;//相机曝光时间ms
	//double gain_db = 16.9807;//增益 0-16.9807


	double cameraExposureTime = 0.5;//相机曝光时间ms
	double gain_db = 16.9807;//增益 0-16.9807

	//平均1s1次 1h 3600
	int countMax = 200; //拍摄最大数量
	double temp = 45; //对焦位置
	bool isShowImg = 0; //是否显示图像
	bool isFusionImg = 0; //是否融合图像
	bool isSaveImg = 1; //过程图是否存储
	string filepath = "C:\\Users\\duanshipeng\\Pictures\\ImgFusionFilePath\\";
	int t_sleep = 100; //串口返回时间后等待时间
	bool isChangeSleep = 0; //等待时间是否改变  查看线性变化
	bool isChangeCount = 0; //对焦位置是否线性改变  查看线性变化
	bool isUseLL = 1; //是否使用液体镜头  一般0为相机稳定性测试
	double trueFocusPlace = 45.0;//47.0


	//打开镜头串口 COM4
	int com = 4;

	//创建镜头类
	LiquidLens LL;

	if (!LL.LLSerialPort.InitPort(com))
	{
		std::cout << "initPort fail !" << std::endl;
		return 0;
	}
	else
	{
		std::cout << "initPort success !" << std::endl;
	}
	//确认串口已经连接

	if (!LL.LLSerialPort.OpenListenThread())
	{
		std::cout << "OpenListenThread fail !" << std::endl;
		return 0;
	}
	else
	{
		std::cout << "OpenListenThread success !" << std::endl;
	}
	// 相机API 创建HKCamera类
	HKCamera HKC;
	// 图像处理算法API
	cvalgorithm CVA;



	LL.SetFocalPlace(100.0);
	int  count = 0;
	//100ms一小时36000,50ms五小时360000
	//1050ms一小时3428

	int sendTrueSignal = 0;//镜头发送信号判断
	int getTrueSignal = 0;//镜头返回信号判断

	vector<double> allFocalsPlace(countMax, 0);//对焦位置容器
	vector<bool> isOk(countMax, 0);;//镜头串口返回信号是否正确容器
	vector<double> valueFocal(countMax, 0);//对焦清晰度容器
	vector<double> t_response1(countMax, 0);//串口返回一bit数据时间容器
	vector<double> t_response2(countMax, 0);
	vector<double> t_response3(countMax, 0);
	vector<double> t_response4(countMax, 0);
	vector<double> t_response5(countMax, 0);
	vector<double> t_response6(countMax, 0);
	vector<double> t_send2get(countMax, 0);//镜头调节时间容器

	vector<double> t_allTakeImg(countMax, 0);//相机拍摄图片时间容器
	vector<double> t_allSaveImg(countMax, 0);//相机存储图片时间容器

	vector<double> t_allGetImg(countMax, 0);//相机获取图片时间容器
	vector<double> t_allImgProcessing(countMax, 0);//图片处理时间容器
	vector<double> t_onceTime(countMax, 0);//一次拍摄总共时间容器
	vector<double> t_sleepTime(countMax, 0);//一次拍摄总共时间容器

	if (HKC.OpenHKCamera() != MV_OK)
	{
		std::cout << "error:相机打开失败!" << endl;
		return 0;
	}
	HKC.ExposureTime = cameraExposureTime * 1000;
	HKC.Gain = gain_db;
	if (HKC.SetHKCamera() != MV_OK)
	{
		std::cout << "error:相机初始化失败!" << endl;
		return 0;
	}
	
	while (count < countMax)
	{
		//double temp = rand()%101;
		temp = (count % 2 == 0) ? trueFocusPlace : 0;//对焦位置
		if (isChangeCount)//线性对焦位置
		{
			temp = ((double)count ) * 100.0 / (double)countMax;
		}
		if (isChangeSleep && count % 2 == 0)
		{
			t_sleep++;
		}
		
		//temp += 1;
		//double temp = 41.0;
		allFocalsPlace[count] = temp;

		//时间：镜头变焦开始
		time_t t_SetFocalPlace = clock();

		if (isUseLL)
		{
			//设置镜头对焦位置 0-100 double
			LL.SetFocalPlace(temp);
			if (LL.ChangeFocals())
			{
				std::cout << "串口信号发送成功 !" << count << "/" << countMax << std::endl;
				sendTrueSignal++;
			}
			else
			{
				std::cout << "error：信号发送失败 !" << std::endl;
			}

			if (LL.IsChangeFocals())
			{
				getTrueSignal++;
				std::cout << "镜头串口返回成功！" << count << "/" << countMax << std::endl;
				isOk[count]=true;
			}
			else
			{
				std::cout << "error：镜头串口返回失败 !" << std::endl;
				isOk[count] = false;

			}
			
		}

		Sleep(t_sleep);


		t_sleepTime[count] = t_sleep;
		//时间：镜头变焦结束
		time_t t_GettFocalPlace = clock();
		t_send2get[count] = (double)(t_GettFocalPlace - t_SetFocalPlace) / CLOCKS_PER_SEC;//镜头变焦时间

		//每一个字节返回时间 存入数组中
		t_response1[count] = LL.time1;
		t_response2[count] = LL.time2;
		t_response3[count] = LL.time3;
		t_response4[count] = LL.time4;
		t_response5[count] = LL.time5;
		t_response6[count] = LL.time6;


		if (HKC.GetImg() != MV_OK)
		{
			std::cout << "error:相机拍摄失败！" << endl;
			break;
		}
		t_allTakeImg[count] = HKC.t_takeimg;
		t_allSaveImg[count] = HKC.t_transimg;

		//时间：拍摄图片结束
		time_t t_GetImg = clock();
		t_allGetImg[count] = (double)(t_GetImg - t_GettFocalPlace) / CLOCKS_PER_SEC;

		//图片buffer
		unsigned char* ImgBuf = HKC.GetImgBuf();

		//图片详细信息buffer
		//MV_FRAME_OUT_INFO_EX ImgInfo = HKC.getImageInfo();

		//图片转换成Mat格式方便opencv处理
		Mat imageSource = CVA.byte2mat(ImgBuf, 2448, 2048, 24).clone();

		//显示图片
		if (isShowImg)
		{
			namedWindow("img", CV_WINDOW_NORMAL);//CV_WINDOW_NORMAL就是0
			imshow("img", imageSource);
			waitKey();
		}
		if (isFusionImg)
		{
			CVA.images.push_back(imageSource);
		}
		if (isSaveImg)
		{
			string filepath_temp = filepath + "ImgNum" + to_string(10000 + count) + "sleeptime" + to_string(10000 + t_sleep);
			filepath_temp += ".jpg";
			imwrite(filepath_temp, imageSource);
		}


		//对焦清晰度评价
		valueFocal[count] = CVA.valueFocals(imageSource);

		//时间 图像处理结束
		time_t t_ImgProcessing = clock();
		t_allImgProcessing[count] = (double)(t_ImgProcessing - t_GetImg) / CLOCKS_PER_SEC;//图像处理时间
		t_onceTime[count] = (double)(t_ImgProcessing - t_SetFocalPlace) / CLOCKS_PER_SEC;//一次照片变焦+拍摄时间
		count++;

		//删除创建指针
		if (ImgBuf) { delete ImgBuf; };
	}

	if (HKC.CloseHKCamera() != MV_OK)
	{
		std::cout << "error:相机关闭失败！" << endl; 
		return 0;
	}

	if (isFusionImg)
	{
		//图像融合计算
		CVA.fusionImg();
		namedWindow("fusionImg", CV_WINDOW_NORMAL);//CV_WINDOW_NORMAL就是0
		imshow("fusionImg", CVA.Fusion);
		waitKey();
	}


	//写txt文件 统计出错结果 与时间
	ofstream OutFile("ImgAlgorithm.txt");

	OutFile << "对焦位置（0-100）" << "\t"<< "等待时间"<< "\t" << "串口返回" << "\t" << "图像处理返回" << "\t" << "与上一张清晰度对比" << "\t" << "对焦清晰度返回" << "\t" << "返回时间1" << "\t" << "返回时间2" << "\t" << "返回时间3" << "\t" << "返回时间4" << "\t" << "返回时间5" << "\t" << "返回时间6" << "\t" << "发送到返回时间" << "\t" << "拍摄时间" << "\t" << "存图时间" << "\t" << "总拍照获取图像时间" << "\t" << "图像处理时间" << "\t" << "一次完整对焦拍照时间" << endl;

	int isImgChange = 0;
	for (int i = 0; i < allFocalsPlace.size(); i++)
	{
		OutFile << allFocalsPlace[i] << "\t";
		OutFile << t_sleepTime[i] << "\t";
		OutFile << (isOk[i] ? 1 : 0) << "\t";
		if (i > 0)
		{
			//double a = abs(valueFocal[i] - valueFocal[i - 1]);
			//double b = 0.7 * (*max_element(valueFocal.begin(), valueFocal.end()) - *min_element(valueFocal.begin(), valueFocal.end()));
			if (abs(valueFocal[i] - valueFocal[i - 1]) > 0.8 * abs(valueFocal[1] - valueFocal[0]))
			{
				isImgChange++;
				OutFile << 1 << "\t";
			}
			else
			{
				OutFile << 0 << "\t";
			}
			OutFile << abs(valueFocal[i] - valueFocal[i - 1]) << "\t";
		}
		else
		{
			isImgChange++;
			OutFile << 1 << "\t";
			OutFile << (*max_element(valueFocal.begin(), valueFocal.end()) - *min_element(valueFocal.begin(), valueFocal.end())) << "\t";
		}
		OutFile << valueFocal[i] << "\t";

		OutFile << t_response1[i] << "\t" << t_response2[i] << "\t" << t_response3[i] << "\t" << t_response4[i] << "\t" << t_response5[i] << "\t" << t_response6[i] << "\t" << t_send2get[i] << "\t" << t_allTakeImg[i] << "\t" << t_allSaveImg[i] << "\t" << t_allGetImg[i] << "\t" << t_allImgProcessing[i] << "\t" << t_onceTime[i] << endl;


	}
	//OutFile << "串口返回成功\t" << getTrueSignal << endl;
	OutFile << "串口发送总数\t" << count << endl;
	OutFile << "串口发送成功\t" << sendTrueSignal << endl;
	OutFile << "串口返回成功\t" << getTrueSignal << endl;
	OutFile << "图像变焦成功数量\t" << isImgChange << endl;
	OutFile << "相机曝光时间\t" << cameraExposureTime << "ms" << endl;
	OutFile.close();

	std::cout << "串口成功数量\t" << getTrueSignal << endl;
	std::cout << "图像变焦成功数量\t" << isImgChange << endl;


	//读取模块Firmware 版本(未实现)
	//输入：无
	//输出：版本十六进制返回代码
	/*
		LL.GetFirmwareInfo();
		for(int i = 0; i < 14; i++)
		{
			std::cout << hex << (int)LL.dataInfoGet[i] << std::endl;
		}
	*/

	//读取镜头温度
	//输入：null
	//输出：镜头温度

	//控制相机SDK进行拍照存图



	

	return 0;
}