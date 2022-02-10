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

void readImages(vector<Mat>& images)//�����ÿɽ�ʡ50%���ڴ�����
{
	int numImages = 2;
	static const char* filenames[] =
	{
	   "C:\\Users\\duanshipeng\\Pictures\\lenaright.png",
	   "C:\\Users\\duanshipeng\\Pictures\\lenaleft.png",
	};

	for (int i = 0; i < numImages; i++)
	{
		Mat im = imread(filenames[i]);
		images.push_back(im);
	}

}


int main()
{ 
	//����ģ��ѵ����
	//Mat lena; 
	//string filename_lena = "C:\\Users\\duanshipeng\\Pictures\\lenaGood.png";
	//lena = imread(filename_lena, IMREAD_UNCHANGED);
	//Mat lena_left= lena; Mat lena_right= lena;
	////Rect region(0, 0, 203, 406);
	//Rect region(203, 0, 203, 406);
	//GaussianBlur(lena_right(region), lena_right(region), Size(0, 0), 4);
	//imshow("lena", lena_right);
	//waitKey();
	//imwrite("C:\\Users\\duanshipeng\\Pictures\\lenaleft.png", lena_right);

	//Rect region2(203, 0, 203, 406);
	//GaussianBlur(lena_left(region2), lena_left(region2), Size(0, 0), 4);
	//imshow("lena", lena_left);
	//waitKey();

	int cameraExposureTime = 500;//����ع�ʱ��ms
	//ƽ��1s1�� 1h 3600
	int countMax = 10;//�����������
	double temp = 45;//�Խ�λ��
	bool isShowImg = 0;

	//������ͷ��
	LiquidLens LL;
	//�򿪾�ͷ���� COM4
	int com = 4;

	if (!LL.LLSerialPort.InitPort(com))
	{
		std::cout << "initPort fail !" << std::endl;
		return 0;
	}
	else
	{
		std::cout << "initPort success !" << std::endl;
	}
	//ȷ�ϴ����Ѿ�����

	if (!LL.LLSerialPort.OpenListenThread())
	{
		std::cout << "OpenListenThread fail !" << std::endl;
		return 0;
	}
	else
	{
		std::cout << "OpenListenThread success !" << std::endl;
	}
	// ���API ����HKCamera��
	HKCamera HKC;
	// ͼ�����㷨API
	cvalgorithm CVA;



	LL.SetFocalPlace(100.0);
	int  count = 0;
	//100msһСʱ36000,50ms��Сʱ360000
	//1050msһСʱ3428

	int sendTrueSignal = 0;//��ͷ�����ź��ж�
	int getTrueSignal = 0;//��ͷ�����ź��ж�

	vector<double> allFocalsPlace(countMax, 0);//�Խ�λ������
	vector<bool> isOk;//��ͷ���ڷ����ź��Ƿ���ȷ����
	vector<double> valueFocal(countMax, 0);//�Խ�����������
	vector<double> t_response1(countMax, 0);//���ڷ���һbit����ʱ������
	vector<double> t_response2(countMax, 0);
	vector<double> t_response3(countMax, 0);
	vector<double> t_response4(countMax, 0);
	vector<double> t_response5(countMax, 0);
	vector<double> t_response6(countMax, 0);
	vector<double> t_send2get(countMax, 0);//��ͷ����ʱ������

	vector<double> t_allTakeImg(countMax, 0);//�������ͼƬʱ������
	vector<double> t_allSaveImg(countMax, 0);//����洢ͼƬʱ������

	vector<double> t_allGetImg(countMax, 0);//�����ȡͼƬʱ������
	vector<double> t_allImgProcessing(countMax, 0);//ͼƬ����ʱ������
	vector<double> t_onceTime(countMax, 0);//һ�������ܹ�ʱ������

	if (HKC.OpenHKCamera() != MV_OK)
	{
		std::cout << "error:�����ʧ��!" << endl;
		return 0;
	}
	HKC.ExposureTime = cameraExposureTime * 1000;
	if (HKC.SetHKCamera() != MV_OK)
	{
		std::cout << "error:�����ʼ��ʧ��!" << endl;
		return 0;
	}
	
	while (count < countMax)
	{
		//double temp = rand()%101;
		//double temp = (count % 2 == 0) ? 100 : 0;//�Խ�λ��
		temp += 1;
		//double temp = 41.0;
		allFocalsPlace[count] = temp;

		//ʱ�䣺��ͷ�佹��ʼ
		time_t t_SetFocalPlace = clock();

		//���þ�ͷ�Խ�λ�� 0-100 double
		LL.SetFocalPlace(temp);

		if (LL.ChangeFocals())
		{
			std::cout << "�����źŷ��ͳɹ� !" << count << "/" << countMax << std::endl;
			sendTrueSignal++;
		}
		else
		{
			std::cout << "error���źŷ���ʧ�� !" << std::endl;
		}

		if (LL.IsChangeFocals())
		{
			getTrueSignal++;
			std::cout << "��ͷ���ڷ��سɹ���" << count << "/" << countMax << std::endl;
			isOk.push_back(true);
		}
		else
		{
			std::cout << "error����ͷ���ڷ���ʧ�� !" << std::endl;
			isOk.push_back(false);

		}
		//ʱ�䣺��ͷ�佹����
		time_t t_GettFocalPlace = clock();
		t_send2get[count] = (double)(t_GettFocalPlace - t_SetFocalPlace) / CLOCKS_PER_SEC;//��ͷ�佹ʱ��

		//ÿһ���ֽڷ���ʱ�� ����������
		t_response1[count] = LL.time1;
		t_response2[count] = LL.time2;
		t_response3[count] = LL.time3;
		t_response4[count] = LL.time4;
		t_response5[count] = LL.time5;
		t_response6[count] = LL.time6;


		if (HKC.GetImg() != MV_OK)
		{
			std::cout << "error:�������ʧ�ܣ�" << endl;
			break;
		}
		t_allTakeImg[count] = HKC.t_takeimg;
		t_allSaveImg[count] = HKC.t_transimg;

		//ʱ�䣺����ͼƬ����
		time_t t_GetImg = clock();
		t_allGetImg[count] = (double)(t_GetImg - t_GettFocalPlace) / CLOCKS_PER_SEC;

		//ͼƬbuffer
		unsigned char* ImgBuf = HKC.GetImgBuf();

		//ͼƬ��ϸ��Ϣbuffer
		//MV_FRAME_OUT_INFO_EX ImgInfo = HKC.getImageInfo();

		//ͼƬת����Mat��ʽ����opencv����
		Mat imageSource = CVA.byte2mat(ImgBuf, 2448, 2048, 24).clone();

		//��ʾͼƬ
		if (isShowImg)
		{
			namedWindow("img", CV_WINDOW_NORMAL);//CV_WINDOW_NORMAL����0
			imshow("img", imageSource);
			waitKey();
		}
		CVA.images.push_back(imageSource);

		//�Խ�����������
		valueFocal[count] = CVA.valueFocals(imageSource);

		//ʱ�� ͼ�������
		time_t t_ImgProcessing = clock();
		t_allImgProcessing[count] = (double)(t_ImgProcessing - t_GetImg) / CLOCKS_PER_SEC;//ͼ����ʱ��
		t_onceTime[count] = (double)(t_ImgProcessing - t_SetFocalPlace) / CLOCKS_PER_SEC;//һ����Ƭ�佹+����ʱ��
		count++;

		//ɾ������ָ��
		if (ImgBuf) { delete ImgBuf; };
	}

	if (HKC.CloseHKCamera() != MV_OK)
	{
		std::cout << "error:����ر�ʧ�ܣ�" << endl; 
		return 0;
	}

	//ͼ���ںϼ���
	CVA.fusionImg();
	namedWindow("fusionImg", CV_WINDOW_NORMAL);//CV_WINDOW_NORMAL����0
	imshow("fusionImg", CVA.Fusion);
	waitKey();

	//дtxt�ļ� ͳ�Ƴ����� ��ʱ��
	ofstream OutFile("ImgAlgorithm.txt");

	OutFile << "�Խ�λ�ã�0-100��" << "\t" << "���ڷ���" << "\t" << "ͼ������" << "\t" << "����һ�������ȶԱ�" << "\t" << "�Խ������ȷ���" << "\t" << "����ʱ��1" << "\t" << "����ʱ��2" << "\t" << "����ʱ��3" << "\t" << "����ʱ��4" << "\t" << "����ʱ��5" << "\t" << "����ʱ��6" << "\t" << "���͵�����ʱ��" << "\t" << "����ʱ��" << "\t" << "��ͼʱ��" << "\t" << "�����ջ�ȡͼ��ʱ��" << "\t" << "ͼ����ʱ��" << "\t" << "һ�������Խ�����ʱ��" << endl;

	int isImgChange = 0;
	for (int i = 0; i < allFocalsPlace.size(); i++)
	{
		OutFile << allFocalsPlace[i] << "\t";
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
	//OutFile << "���ڷ��سɹ�\t" << getTrueSignal << endl;
	OutFile << "���ڷ�������\t" << count << endl;
	OutFile << "���ڷ��ͳɹ�\t" << sendTrueSignal << endl;
	OutFile << "���ڷ��سɹ�\t" << getTrueSignal << endl;
	OutFile << "ͼ��佹�ɹ�����\t" << isImgChange << endl;
	OutFile << "����ع�ʱ��\t" << cameraExposureTime << "ms" << endl;
	OutFile.close();

	std::cout << "���ڳɹ�����\t" << getTrueSignal << endl;
	std::cout << "ͼ��佹�ɹ�����\t" << isImgChange << endl;


	//��ȡģ��Firmware �汾(δʵ��)
	//���룺��
	//������汾ʮ�����Ʒ��ش���
	/*
		LL.GetFirmwareInfo();
		for(int i = 0; i < 14; i++)
		{
			std::cout << hex << (int)LL.dataInfoGet[i] << std::endl;
		}
	*/

	//��ȡ��ͷ�¶�
	//���룺null
	//�������ͷ�¶�

	//�������SDK�������մ�ͼ



	

	return 0;
}