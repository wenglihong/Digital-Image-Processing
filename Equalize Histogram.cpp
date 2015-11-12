#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream> //�ļ����������

using namespace cv;
using namespace std;

MatND get_hist(Mat src);
void show_hist(MatND hist);

int main(void)
{
	string filename = ".\\SJTU.jpg";//�ļ�·��������
	Mat src = imread(filename);//��ȡԴͼ��
	if (src.empty())
		return -1;
	Mat dst;

	cvtColor( src, src, COLOR_BGR2GRAY );//ת��Ϊ�Ҷ�ͼ
	equalizeHist( src, dst );

	show_hist(get_hist(src));
	show_hist(get_hist(dst));

	imshow("ԭͼ��", src);//��ʾԴͼ
	imshow("ֱ��ͼ���⻯", dst);//��ʾֱ��ͼ���⻯���ͼ��

	imwrite(".\\SJTU_gray.bmp",src);
	imwrite(".\\SJTU_hist_equal.bmp",dst);

	waitKey();
	return 0;
}


MatND get_hist(Mat src)
{
	int bins = 180;
	int hist_size[] = { bins };
	float range[] = { 0, 180 };
	const float* ranges[] = { range };
	int channels[] = { 0 };
	cv::MatND hist;
	cv::calcHist(&src, 1, channels, Mat(), hist, 1, hist_size, ranges, true, false);
	return hist;
}

void show_hist(MatND hist,string hist_name)
{
	int bins = 180;
	int hist_height = 180;
	double max_val, max_val2;
	int scale = 2;
	cv::minMaxLoc(hist, 0, &max_val, 0, 0);

	Mat hist_img = Mat::zeros(hist_height, bins*scale, CV_8UC3);
	for (int i = 0; i < bins; i++)
	{
		float bin_val = hist.at<float>(i);
		int intensity = cvRound(bin_val*hist_height / max_val);
		cv::rectangle(hist_img, cv::Point(i*scale, hist_height - 1), cv::Point((i + 1)*scale - 1, hist_height - intensity), CV_RGB(255, 255, 255));
	}
	imshow(hist_name, hist_img);
}

