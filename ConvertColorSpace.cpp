#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream> //�ļ����������

using namespace cv;
using namespace std;

int main(void)
{
	string filename = ".\\selfie.jpg";//�ļ�·��������
	Mat src = imread(filename);//��ȡԴͼ��
	if (src.empty())
		return -1;
	Mat img_gray,img_binary;

	imshow("viewer", src);//��ʾԴͼ
	
	cvtColor(src, img_gray, CV_RGB2GRAY);//ת��Ϊ�Ҷ�ͼ
	threshold(img_gray, img_binary, 60, 255, THRESH_BINARY);//ת��Ϊ��ֵͼ��

	//�����ͼ���ļ�
	imwrite(".\\selfie.bmp", src);
	imwrite(".\\selfie_gray.bmp", img_gray);
	imwrite(".\\selfie_binary.bmp", img_binary);

	waitKey();
	return 0;
}