#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream> //文件输入输出流

using namespace cv;
using namespace std;

int main(void)
{
	string filename = ".\\selfie.jpg";//文件路径及名称
	Mat src = imread(filename);//读取源图像
	if (src.empty())
		return -1;
	Mat img_gray,img_binary;

	imshow("viewer", src);//显示源图
	
	cvtColor(src, img_gray, CV_RGB2GRAY);//转换为灰度图
	threshold(img_gray, img_binary, 60, 255, THRESH_BINARY);//转换为二值图像

	//保存各图像到文件
	imwrite(".\\selfie.bmp", src);
	imwrite(".\\selfie_gray.bmp", img_gray);
	imwrite(".\\selfie_binary.bmp", img_binary);

	waitKey();
	return 0;
}