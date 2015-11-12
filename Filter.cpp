/***********************************************
@breaf: 均值滤波器函数，包括算术均值滤波器,
		几何均值滤波器，谐波均值滤波器

@author：翁理洪

@date: 2015.10.25

@ref: 数字图像处理第二版（冈萨雷斯）P183-184
************************************************/


#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/core.hpp> 
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

/**
@brief 添加椒盐噪声
@note
@param image 要添加椒盐噪声的图像，3通道
@param n 椒盐噪点的数目
@param mn 卷积核的尺寸
*/
void salt(Mat& image, int n);
/** 
@brief 算术均值滤波器
@note 
@param src 源图像，3通道
@param dst 目标图像，3通道
@param anchor 卷积核的锚点
@param mn 卷积核的尺寸
*/
void ArithmeticMeanFilter(Mat src, Mat& dst, Point anchor, Size mn);
/**
@brief 几何均值滤波器
@note
@param src 源图像，3通道
@param dst 目标图像，3通道
@param anchor 卷积核的锚点
@param mn 卷积核的尺寸
*/
void GeometricMeanFilter(Mat src, Mat& dst, Point anchor, Size mn);
/**
@brief 谐波均值滤波器
@note
@param src 源图像，3通道
@param dst 目标图像，3通道
@param anchor 卷积核的锚点
@param mn 卷积核的尺寸
*/
void HarmonicMeanFilter(Mat src, Mat& dst, Point anchor, Size mn);
/**
@brief 逆谐波均值滤波器
@note
@param src 源图像，3通道
@param dst 目标图像，3通道
@param anchor 卷积核的锚点
@param mn 卷积核的尺寸
@param Q 逆谐波均值滤波器的阶数
*/
void ContraharmonicMeanFilter(Mat src, Mat& dst, Point anchor, Size mn, int Q);

int main(void)
{
	string filename = ".\\image\\pcb.tif";//文件路径及名称
	Mat src = imread(filename);//读取源图像
	if (src.empty())
		return -1;
	salt(src, src.total()*0.1);
	Mat dst(src.size(), src.type());

	ContraharmonicMeanFilter(src, dst, Point(1, 1), Size(3, 3),-2.5);
	imshow("src", src);
	imshow("dst", dst);

	waitKey();
	return 0;
}

void salt(Mat& image, int n)
{
	for (int k = 0; k<n; k++)
	{
		int i = rand() % image.cols;
		int j = rand() % image.rows;

		if (image.channels() == 1)
		{
			image.at<uchar>(j, i) = 255;
		}
		else
		{
			image.at<Vec3b>(j, i)[0] = 255;
			image.at<Vec3b>(j, i)[1] = 255;
			image.at<Vec3b>(j, i)[2] = 255;
		}
	}
}

void ArithmeticMeanFilter(Mat src, Mat& dst, Point anchor, Size mn)
{
	Mat chn[3];
	cv::split(src, chn);
	Mat b = chn[0];
	Mat g = chn[1];
	Mat r = chn[2];

	int w = mn.width;
	int h = mn.height;
	int endX = dst.cols - w;
	int endY = dst.rows - h;
	int pi = anchor.y;
	int pj = anchor.x;

	Mat roiB, roiG, roiR;
	Mat b2 = b.clone();
	Mat g2 = g.clone();
	Mat r2 = r.clone();
	for (int y = 0; y < endY; y++)
	{
		for (int x = 0; x < endX; x++)
		{
			roiB = b(Rect(x, y, w, h));
			roiG = g(Rect(x, y, w, h));
			roiR = r(Rect(x, y, w, h));

			double sum1 = 0.0;
			double sum2 = 0.0;
			double sum3 = 0.0;
			for (int i = 0; i < h; i++)
			{
				for (int j = 0; j < w; j++)
				{
					sum1 += roiB.at<unsigned char>(i, j);
					sum2 += roiG.at<unsigned char>(i, j);
					sum3 += roiR.at<unsigned char>(i, j);
				}
			}
			b2.at<unsigned char>(y + pi, x + pj) = (unsigned char)(sum1 / (w*h));
			g2.at<unsigned char>(y + pi, x + pj) = (unsigned char)(sum2 / (w*h));
			r2.at<unsigned char>(y + pi, x + pj) = (unsigned char)(sum3 / (w*h));
		}
	}
	Mat mer[3];
	mer[0] = b2;
	mer[1] = g2;
	mer[2] = r2;
	merge(mer, 3, dst);
}

void  GeometricMeanFilter(Mat src, Mat& dst,Point anchor,Size mn)
{
	Mat chn[3];
	cv::split(src, chn);
	Mat b = chn[0];
	Mat g = chn[1];
	Mat r = chn[2];

	int w = mn.width;
	int h = mn.height;
	int endX = dst.cols - w;
	int endY = dst.rows - h;
	int pi = anchor.y;
	int pj = anchor.x;

	Mat roiB, roiG, roiR;
	Mat b2 = b.clone();
	Mat g2 = g.clone();
	Mat r2 = r.clone();
	for (int y = 0; y < endY; y++)
	{
		for (int x = 0; x < endX; x++)
		{
			roiB = b(Rect(x, y, w, h));
			roiG = g(Rect(x, y, w, h));
			roiR = r(Rect(x, y, w, h));

			double product1 = 1.0;
			double product2 = 1.0;
			double product3 = 1.0;
			for (int i = 0; i < h; i++)
			{
				for (int j = 0; j < w; j++)
				{
					product1 *= roiB.at<unsigned char>(i, j);
					product2 *= roiG.at<unsigned char>(i, j);
					product3 *= roiR.at<unsigned char>(i, j);
				}
			}	
			b2.at<unsigned char>(y + pi, x + pj) = (unsigned char)pow(product1, 1.0 / (w*h));
			g2.at<unsigned char>(y + pi, x + pj) = (unsigned char)pow(product2, 1.0 / (w*h));
			r2.at<unsigned char>(y + pi, x + pj) = (unsigned char)pow(product3, 1.0 / (w*h));
		}
	}
	Mat mer[3];
	mer[0] = b2;
	mer[1] = g2;
	mer[2] = r2;
	merge(mer, 3, dst);

	//imshow("b", b);
	//imshow("g", g);
	//imshow("r", r);
	//imshow("b2", b2);
	//imshow("g2", g2);
	//imshow("r2", r2);
	//imshow("dst", dst);
}

void HarmonicMeanFilter(Mat src, Mat& dst, Point anchor, Size mn)
{
	Mat chn[3];
	cv::split(src, chn);
	Mat b = chn[0];
	Mat g = chn[1];
	Mat r = chn[2];

	int w = mn.width;
	int h = mn.height;
	int endX = dst.cols - w;
	int endY = dst.rows - h;
	int pi = anchor.y;
	int pj = anchor.x;

	Mat roiB, roiG, roiR;
	Mat b2 = b.clone();
	Mat g2 = g.clone();
	Mat r2 = r.clone();
	for (int y = 0; y < endY; y++)
	{
		for (int x = 0; x < endX; x++)
		{
			roiB = b(Rect(x, y, w, h));
			roiG = g(Rect(x, y, w, h));
			roiR = r(Rect(x, y, w, h));

			double sum1 = 0.0;
			double sum2 = 0.0;
			double sum3 = 0.0;
			for (int i = 0; i < h; i++)
			{
				for (int j = 0; j < w; j++)
				{
					roiB.at<unsigned char>(i, j) = roiB.at<unsigned char>(i, j) != 0 ? roiB.at<unsigned char>(i, j) : 1.0;
					roiG.at<unsigned char>(i, j) = roiG.at<unsigned char>(i, j) != 0 ? roiG.at<unsigned char>(i, j) : 1.0;
					roiR.at<unsigned char>(i, j) = roiR.at<unsigned char>(i, j) != 0 ? roiR.at<unsigned char>(i, j) : 1.0;
					sum1 += 1.0 / roiB.at<unsigned char>(i, j);
					sum2 += 1.0 / roiG.at<unsigned char>(i, j);
					sum3 += 1.0 / roiR.at<unsigned char>(i, j);
				}
			}
			b2.at<unsigned char>(y + pi, x + pj) = (unsigned char)(w*h / sum1);
			g2.at<unsigned char>(y + pi, x + pj) = (unsigned char)(w*h / sum2);
			r2.at<unsigned char>(y + pi, x + pj) = (unsigned char)(w*h / sum3);
		}
	}
	Mat mer[3];
	mer[0] = b2;
	mer[1] = g2;
	mer[2] = r2;
	merge(mer, 3, dst);
}

void ContraharmonicMeanFilter(Mat src, Mat& dst, Point anchor, Size mn, int Q)
{
	Mat chn[3];
	cv::split(src, chn);
	Mat b = chn[0];
	Mat g = chn[1];
	Mat r = chn[2];

	int w = mn.width;
	int h = mn.height;
	int endX = dst.cols - w;
	int endY = dst.rows - h;
	int pi = anchor.y;
	int pj = anchor.x;

	Mat roiB, roiG, roiR;
	Mat b2 = b.clone();
	Mat g2 = g.clone();
	Mat r2 = r.clone();
	for (int y = 0; y < endY; y++)
	{
		for (int x = 0; x < endX; x++)
		{
			roiB = b(Rect(x, y, w, h));
			roiG = g(Rect(x, y, w, h));
			roiR = r(Rect(x, y, w, h));

			double sum11 = 0.0;
			double sum12 = 0.0;
			double sum21 = 0.0;
			double sum22 = 0.0;
			double sum31 = 0.0;
			double sum32 = 0.0;
			for (int i = 0; i < h; i++)
			{
				for (int j = 0; j < w; j++)
				{
					sum11 += pow(roiB.at<unsigned char>(i, j), Q + 1);
					sum21 += pow(roiG.at<unsigned char>(i, j), Q + 1);
					sum32 += pow(roiR.at<unsigned char>(i, j), Q + 1);

					sum12 += pow(roiB.at<unsigned char>(i, j), Q);
					sum22 += pow(roiG.at<unsigned char>(i, j), Q);
					sum32 += pow(roiR.at<unsigned char>(i, j), Q);
				}
			}
			b2.at<unsigned char>(y + pi, x + pj) = (unsigned char)(sum11 / sum12);
			g2.at<unsigned char>(y + pi, x + pj) = (unsigned char)(sum21 / sum22);
			r2.at<unsigned char>(y + pi, x + pj) = (unsigned char)(sum31 / sum32);
		}
	}
	imshow("b",b2);
	imwrite("b.bmp", b2);
	Mat mer[3];
	mer[0] = b2;
	mer[1] = g2;
	mer[2] = r2;
	merge(mer, 3, dst);
}


