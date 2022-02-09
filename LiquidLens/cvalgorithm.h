#pragma once
#ifndef CVALGORITHM_H_  
#define CVALGORITHM_H_  
#endif

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
using namespace std;
using namespace cv;

class cvalgorithm
{
public:
	cvalgorithm();
	~cvalgorithm();

	Mat byte2mat(unsigned char* pimg, int w, int h, int bits);
	double valueFocals(Mat img);

private:




};
