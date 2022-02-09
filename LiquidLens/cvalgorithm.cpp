#pragma once
#ifndef CVALGORITHM_H_  
#define CVALGORITHM_H_  
#endif

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include "cvalgorithm.h"

using namespace std;
using namespace cv;

cvalgorithm::cvalgorithm() {};
cvalgorithm::~cvalgorithm() {};

double cvalgorithm::valueFocals(Mat img)
{
	//	����ͼ��Խ�
	Mat imageGrey;
	cvtColor(img, imageGrey, CV_RGB2GRAY);
	Mat imageSobel;
	Laplacian(imageGrey, imageSobel, CV_16U, 1, 1);

	//ͼ���ƽ���Ҷ�
	return mean(imageSobel)[0];
}

Mat cvalgorithm::byte2mat(unsigned char* pimg, int w, int h, int bits) {
	int size[] = { h,w };
	//int size[] = { h,w };
	cv::Mat mat;

	if (bits == 8) {
		mat = cv::Mat(2, size, CV_8UC1, pimg, 0);
	}
	else if (bits == 16)
	{
		mat = cv::Mat(w, size, CV_16UC1, pimg, 0);
	}
	else if (bits == 24) {
		mat = cv::Mat(2, size, CV_8UC3, pimg, 0);
	}

	return mat;
}
