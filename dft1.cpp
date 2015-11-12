#include "cxcore.h"
#include "cv.h"
#include "highgui.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream> //文件输入输出流

using namespace cv;
using namespace std;

#define Ideal 0
#define Gaussian 1
#define Butterworth 2

Mat CreatMatD(Size mn);
void GLPF(Mat &src, const double D0);
void ILPF(Mat &src, const double D0);
void BLPF(Mat &src, const double D0);
void LPF(int type, Mat &src,double D0);
void shift_image(Mat& src);


int main(void)
{	
	Mat im = imread(".\\image\\lena.bmp", CV_LOAD_IMAGE_GRAYSCALE);
	if (im.empty())
		return -1;

	Mat realInput(im.size(), CV_64FC1);
	Mat imaginaryInput(im.size(), CV_64FC1);
	Mat complexInput(im.size(), CV_64FC2,Scalar(0.0,0.0));

	convertScaleAbs(im, realInput, 1.0, 0.0);//实部矩阵
	imaginaryInput.setTo(0);//虚部矩阵
	Mat mer[] = { (Mat_<double>)realInput, (Mat_<double>)imaginaryInput };
	merge(mer,2,complexInput);//实部和虚部合成矩阵


	int M = cvGetOptimalDFTSize(im.rows - 1);//对于给定的矢量尺寸返回最优DFT尺寸
	int N = cvGetOptimalDFTSize(im.cols - 1);
	Mat dft_A(M, N, CV_64FC2);
	dft_A.setTo(0);
	complexInput.copyTo(dft_A);//补零后的矩阵

	double D0 = 50;
	dft(dft_A, dft_A);//傅里叶正变换

	Mat spl2[2];
	split(dft_A, spl2);
	Mat magI;
	magnitude(spl2[0], spl2[1], magI);
	magI += Scalar::all(1);                    // switch to logarithmic scale
	log(magI, magI);
	normalize(magI, magI, 0, 1, CV_MINMAX);	
	shift_image(magI);
	imshow("频谱图", magI);

	ILPF(dft_A, D0);
	idft(dft_A, dft_A);//傅里叶逆变换

	Mat spl[2];
	split(dft_A, spl);
	Mat image_Re(M, N, CV_64FC1);
	Mat image_Im(M, N, CV_64FC1);
	image_Re = spl[0];//实部为滤波后的图像
	image_Im = spl[1];
	normalize(image_Re, image_Re, 0, 1, CV_MINMAX);

	namedWindow("src");
	namedWindow("dst");
	
	imshow("src", im);
	imshow("dst", image_Re);

	waitKey(0);
	return 0;
}

Mat CreatMatD(Size mn)
{
	Mat D(mn, CV_64FC1);
	mn.width;
	int i, j;
	int state = -1;
	double tempD;
	int width, height;
	width = mn.width;
	height = mn.height;

	int x, y;
	x = width / 2;
	y = height / 2;

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			if (i > y && j > x)
			{
				state = 3;
			}
			else if (i > y)
			{
				state = 1;
			}
			else if (j > x)
			{
				state = 2;
			}
			else
			{
				state = 0;
			}

			switch (state)
			{
			case 0:
				tempD = (double)(i * i + j * j); D.at<double>(i,j) = sqrt(tempD); break;
			case 1:
				tempD = (double)((height - i) * (height - i) + j * j); D.at<double>(i, j) = sqrt(tempD); break;
			case 2:
				tempD = (double)(i * i + (width - j) * (width - j)); D.at<double>(i, j) = sqrt(tempD); break;
			case 3:
				tempD = (double)((height - i) * (height - i) + (width - j) * (width - j)); D.at<double>(i, j) = sqrt(tempD); break;
			default:
				break;
			}
		}
	}
	Mat showD;
	normalize(D, showD, 0, 1, CV_MINMAX);
	shift_image(showD);
	imshow("D", showD);
	return D;
}

void GLPF(Mat &src, const double D0)
{
	Mat D = CreatMatD(src.size());
	Mat H_mat(src.size(), CV_64FC2);
	int width = src.cols;
	int height = src.rows;
	double tempD;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			tempD = D.at<double>(i, j);
			//二维高斯低通滤波器传递函数
			tempD = exp(-0.5 * pow(tempD / D0, 2));
			H_mat.at<double>(i, j * 2) = tempD;
			H_mat.at<double>(i, j * 2 + 1) = 0.0;
		}
	}
	Mat spl[2];
	split(H_mat, spl);
	shift_image(spl[0]);
	imshow("H_mat", spl[0]);
	mulSpectrums(src, H_mat, src, CV_DXT_ROWS);
}

void ILPF(Mat &src, const double D0)
{
	Mat D = CreatMatD(src.size());
	Mat H_mat(src.size(), CV_64FC2);
	int width = src.cols;
	int height = src.rows;
	double tempD;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			tempD = D.at<double>(i, j);
			//二维理想低通滤波器传递函数
			if (tempD <= D0)
			{
				H_mat.at<double>(i, j * 2) = 1.0;
				H_mat.at<double>(i, j * 2 + 1) = 0.0;
			}
			else
			{
				H_mat.at<double>(i, j * 2) = 0.0;
				H_mat.at<double>(i, j * 2 + 1) = 0.0;
			}
		}
	}
	Mat spl[2];
	split(H_mat, spl);
	shift_image(spl[0]);
	imshow("H_mat", spl[0]);
	mulSpectrums(src, H_mat, src, CV_DXT_ROWS);
}

void BLPF(Mat &src, const double D0)
{
	Mat D = CreatMatD(src.size());
	Mat H_mat(src.size(), CV_64FC2);
	int width = src.cols;
	int height = src.rows;
	double tempD;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			tempD = D.at<double>(i, j);
			//2阶巴特沃思低通滤波器传递函数
			tempD = 1 / (1 + pow(tempD / D0, 2 * 2));
			H_mat.at<double>(i, j * 2) = tempD;
			H_mat.at<double>(i, j * 2 + 1) = 0.0;
		}
	}
	Mat spl[2];
	split(H_mat, spl);
	shift_image(spl[0]);
	imshow("H_mat", spl[0]);
	mulSpectrums(src, H_mat, src, CV_DXT_ROWS);			
}

void LPF(int type, Mat &src,double D0)
{
	switch (type)
	{
		case 0:	ILPF(src, D0); break;
		case 1:	GLPF(src, D0); break;
		case 2:	BLPF(src, D0); break;

		default:cout << "Error Type" << endl;
			break;
	}
}

void shift_image(Mat& src)
{
	int cx = src.cols / 2;
	int cy = src.rows / 2;

	Mat q0(src, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant 
	Mat q1(src, Rect(cx, 0, cx, cy));  // Top-Right
	Mat q2(src, Rect(0, cy, cx, cy));  // Bottom-Left
	Mat q3(src, Rect(cx, cy, cx, cy)); // Bottom-Right

	Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
	q2.copyTo(q1);
	tmp.copyTo(q2);
}