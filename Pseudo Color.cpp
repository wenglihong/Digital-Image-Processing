/***********************************************
@breaf: 灰度图转伪彩色

@author：翁理洪

@date: 2015.11.05

@ref:http://www.cnblogs.com/xianglan/archive/2010/12/29/1920727.html
	 http://m.ithao123.cn/content-6236596.html
************************************************/

#include <opencv2/opencv.hpp>
#include <stdio.h>
using namespace cv;
using namespace std;

/**
@brief 将灰度图转换成伪彩色，方法1
@note  映射规则参考 http://www.cnblogs.com/xianglan/archive/2010/12/29/1920727.html
@param src 源图像
@param dst 目标图像
*/
void grayToPseudocilor1(Mat src, Mat &dst);

/**
@brief 将灰度图转换成伪彩色，方法2
@note  映射规则参考 http://m.ithao123.cn/content-6236596.html
@param src 源图像
@param dst 目标图像
@param colorbar 颜色条作为映射来源
*/
void grayToPseudocilor2(Mat src, Mat &dst, Mat colorbar);

/**
@brief 将灰度图转换成伪彩色，方法3
@note  映射规则参考 http://m.ithao123.cn/content-6236596.html
@param src 源图像
@param dst 目标图像
*/
void grayToPseudocilor3(Mat src, Mat &dst);

int main()
{
	Mat img = imread(".\\image\\pcb.tif", CV_LOAD_IMAGE_GRAYSCALE);//采用灰度格式读取图片
	Mat colorbar = imread(".\\image\\colorbar.png");


	Mat img_pseudocolor1(img.rows, img.cols, CV_8UC3);
	Mat img_pseudocolor2(img.rows, img.cols, CV_8UC3);
	Mat img_pseudocolor3(img.rows, img.cols, CV_8UC3);

	grayToPseudocilor1(img, img_pseudocolor1);
	grayToPseudocilor2(img, img_pseudocolor2,colorbar);
	grayToPseudocilor3(img, img_pseudocolor3);

	imshow("bmp_img", img);
	imshow("img_pseudocolor1", img_pseudocolor1);
	imshow("img_pseudocolor2", img_pseudocolor2);
	imshow("img_pseudocolor3", img_pseudocolor3);
	imwrite("img_pseudocolor1.bmp", img_pseudocolor1);
	imwrite("img_pseudocolor2.bmp", img_pseudocolor2);
	imwrite("img_pseudocolor3.bmp", img_pseudocolor3);

	//IMWRITE_PNG_COMPRESSION
	JpegDecoder;
	waitKey(0);
	return 0;
}

void grayToPseudocilor1(Mat src, Mat &dst)
{
	int tmp = 0;
	for (int y = 0; y<src.rows; y++)//转为伪彩色图像的具体算法
	{
		for (int x = 0; x<src.cols; x++)
		{
			tmp = src.at<unsigned char>(y, x);
			int r, g, b;
			r = tmp; g = tmp; b = tmp;
			if (r < 127)
				r = 0;
			else if (r>191)
				r = 255;
			else
				r = (r - 127) * 4 - 1;

			if (g < 64)
				g = 4 * g;
			else if (g>191)
				g = 256 - (g - 191) * 4;
			else
				g = 255;

			if (b < 64)
				b = 255;
			else if (g>127)
				b = 0;
			else
				b = 256 - (b - 63) * 4;

			dst.at<Vec3b>(y, x)[0] = b; //blue
			dst.at<Vec3b>(y, x)[1] = g; //green
			dst.at<Vec3b>(y, x)[2] = r; //red
		}
	}
}

void grayToPseudocilor2(Mat src, Mat &dst, Mat colorbar)
{
	int tmp = 0;
	resize(colorbar, colorbar,Size(256,1));
	Mat chan[3];
	split(colorbar, chan);

	for (int y = 0; y<src.rows; y++)//转为伪彩色图像的具体算法
	{
		for (int x = 0; x<src.cols; x++)
		{
			tmp = src.at<unsigned char>(y, x);
			int b = chan[0].at<unsigned char>(0, tmp);
			int g = chan[1].at<unsigned char>(0, tmp);
			int r = chan[2].at<unsigned char>(0, tmp);
			
			dst.at<Vec3b>(y, x)[0] = b; //blue
			dst.at<Vec3b>(y, x)[1] = g; //green
			dst.at<Vec3b>(y, x)[2] = r; //red
		}
	}
}

void grayToPseudocilor3(Mat src, Mat &dst)
{
	for (int y = 0; y < src.rows; y++)//转为彩虹图的具体算法，主要思路是把灰度图对应的0～255的数值分别转换成彩虹色：红、橙、黄、绿、青、蓝。
	{
		for (int x = 0; x < src.cols; x++)
		{
			int tmp = src.at<uchar>(y, x);
			int r, g, b;
			if (tmp <= 51)
			{
				b = 255;
				g = tmp * 5;
				r = 0;
			}
			else if (tmp <= 102)
			{
				tmp -= 51;
				b = 255 - tmp * 5;
				g = 255;
				r = 0;
			}
			else if (tmp <= 153)
			{
				tmp -= 102;
				b = 0;
				g = 255;
				r = tmp * 5;
			}
			else if (tmp <= 204)
			{
				tmp -= 153;
				b = 0;
				g = 255 - uchar(128.0*tmp / 51.0 + 0.5);
				r = 255;
			}
			else
			{
				tmp -= 204;
				b = 0;
				g = 127 - uchar(127.0*tmp / 51.0 + 0.5);
				r = 255;
			}

			dst.at<Vec3b>(y, x)[0] = b; //blue
			dst.at<Vec3b>(y, x)[1] = g; //green
			dst.at<Vec3b>(y, x)[2] = r; //red
		}
	}
}