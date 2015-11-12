#include<iostream>
#include<fstream>
#include<cstdio>
#include<string>
#include<vector>
#include<opencv2/core/core.hpp>
//#include<opencv2/contrib/contrib.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/objdetect/objdetect.hpp>
#include "HuffmanTree.h"
#include <conio.h>
#include <opencv2\imgcodecs\imgcodecs.hpp>


using namespace cv;
using namespace std;

const char* _windowname = "JPEG Compressed Image";
const char* _dctwindow = "DCT Image";
const char* _filename = ".//image//lena.bmp";

//Data for quantization matrix
int data[8][8] = {
	{ 16, 11, 10, 16, 24, 40, 51, 61 },
	{ 12, 12, 14, 19, 26, 58, 60, 55 },
	{ 14, 13, 16, 24, 40, 57, 69, 56 },
	{ 14, 17, 22, 29, 51, 87, 80, 62 },
	{ 18, 22, 37, 56, 68, 109, 103, 77 },
	{ 24, 35, 55, 64, 81, 104, 113, 92 },
	{ 49, 64, 78, 87, 103, 121, 120, 101 },
	{ 72, 92, 95, 98, 112, 100, 103, 99 }
};

unsigned char image[8][8] = {
	{ 52, 55, 61, 66, 70, 61, 64, 73 },
	{ 63, 59, 66, 90, 109, 85, 69, 72 },
	{ 62, 59, 68, 113, 144, 104, 66, 73 },
	{ 63, 58, 71, 122, 154, 106, 70, 69 },
	{ 67, 61, 68, 104, 126, 88, 68, 70 },
	{ 79, 65, 60, 70, 77, 68, 58, 75 },
	{ 85, 71, 64, 59, 55, 61, 65, 83 },
	{ 87, 79, 69, 68, 65, 76, 78, 94 }
};

void div(Mat src1, Mat src2, Mat dst)
{
	for (int x = 0; x < src1.cols; x++)
	{
		for (int y = 0; y < src1.rows; y++)
		{
			dst.at<float>(x, y) = src1.at<float>(x, y) / src2.at<float>(x, y);
		}
	}
}

int main()
{

	//int arrs[] = { 5, 7, 2, 13 }; 
	//vector<char> alphabet = { 'A', 'B', 'C', 'D' }; 
	//int len = sizeof(arrs) / sizeof(arrs[0]);
	//HuffmanTree hf = HuffmanTree(arrs, len); 
	//string testCode = "DBDBDABDCDADBDADBDADACDBDBD"; 
	//string encodeStr = hf.Encode(hf.HuffmanCoding(len), alphabet, testCode); 
	//printf("%s\n", encodeStr.c_str()); 
	//string decodeStr = hf.Decode(len, alphabet, encodeStr); 
	//printf("%s\n", decodeStr.c_str()); 
//	getch();
	
	//system("pause");

	Mat src = Mat(8, 8, CV_8UC1, &image);

	int x = 0;
	int y = 0;

	x += 300;
	y += 300;

	int height = src.size().height;
	int width = src.size().width;
	Mat A = src.clone();
	
	//Convert the 2D array data to image matrix
	Mat quant = Mat(8, 8, CV_32SC1, &data);
	quant.convertTo(quant, CV_32FC1);
	cout << quant;

	A.convertTo(A, CV_32FC1);
	cout << "src:" << endl;
	cout << A;
	cout << endl;
	
	Mat B(8, 8, CV_32FC1);
	Mat sub128(8, 8, CV_32FC1, Scalar::all(128));
	subtract(A, sub128, B);
	cout << "sub:" << endl;
	cout << B;
	cout << endl;
	
	dct(B, B);
	cout << "dct:" << endl;
	cout << B;
	cout << endl;

	Mat C(8, 8, CV_32FC1);
	divide(B, quant, C);
	cout << "T/Z:" << endl;
	cout << C;
	cout << endl;

	for (int x = 0; x < C.cols; x++)
	for (int y = 0; y < C.rows; y++)
		C.at<float>(x, y) = round(C.at<float>(x, y));
	cout << "C:" << endl;
	cout << C;
	cout << endl;

	string a;
	a.push_back('s');
	cout << a;

	system("pause");


	////Discrete Cosine Tranform
	//for (int i = 0; i < height; i += 8)
	//{
	//	for (int j = 0; j < width; j += 8)
	//	{
	//		Mat block = dctImage(Rect(j, i, 8, 8));

	//		for (int k = 0; k < 3; k++)
	//		{
	//			
	//		}

	//		//Quantization
	//		cvtColor(block, block, CV_BGR2GRAY);
	//		divide(block, quant, block);
	//		//cout << block;
	//		//cout << endl;
	//		multiply(block, quant, block);
	//		//cout << block;
	//		//cout << endl;
	//		merge(outplanes, 3, block);
	//	}
	//}

	//Inverse Discrete Cosine Transform
	/*for(int i=0; i < height; i+=8) {
	for(int j=0; j < width; j+=8) {
	Mat block = dctImage(Rect(j,i,8,8));
	vector<Mat> planes;
	split(block,planes);
	vector<Mat> outplanes(planes.size());
	for(int k=0; k < planes.size(); k++) {
	planes[k].convertTo(planes[k],CV_32FC1);
	idct(planes[k],outplanes[k]);
	outplanes[k].convertTo(outplanes[k],CV_8UC1);
	}
	merge(outplanes,block);
	}
	}*/

	//namedWindow(_dctwindow, CV_WINDOW_AUTOSIZE);
	//moveWindow(_dctwindow, x, y);
	//imshow(_dctwindow, dctImage);
	//cv::cvtColor(dctImage, dctImage, CV_BGR2RGB);
	//imwrite(".\\image\\dst.jpg", dctImage);
	//imencode();
	waitKey(0);

	destroyWindow(_windowname);
	destroyWindow(_dctwindow);
}