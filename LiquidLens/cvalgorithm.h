#pragma once
#ifndef CVALGORITHM_H_  
#define CVALGORITHM_H_  
#endif

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <vector>
#include <string>
using namespace std;
using namespace cv;

class cvalgorithm
{
public:
	cvalgorithm();
	~cvalgorithm();

	Mat byte2mat(unsigned char* pimg, int w, int h, int bits);
	double valueFocals(Mat img);

	vector<Mat> images;
	void readImages();
	int numImages;
	vector<string> filenames = 
	{
		   "C:\\Users\\duanshipeng\\Pictures\\lenaright.png",
		   "C:\\Users\\duanshipeng\\Pictures\\lenaleft.png"
	};

	Mat Fusion;
	void fusionImg();
private:




};
