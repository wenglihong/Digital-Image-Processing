/*************************************************
@brief: 使用libjpeg.lib实现图像压缩写入到jpeg文件

@author：翁理洪

@date: 2015.11.08

@ref: http://stackoverflow.com/questions/5448957/compressing-iplimage-to-jpeg-using-libjpeg-in-opencv
**************************************************/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <jpeglib.h>
#include <iostream>
#include <fstream>

#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef long				LONG;


/**
@brief 将图像压缩为jpeg格式
@note
@param src 源图像
@param filename 写入文件的名称
@param quality 图像压缩比（0-100）
*/
bool jpegCompress(Mat src, string filename, int quality);


int main()
{
	Mat src = imread("lena.bmp");
	imshow("src", src);

	jpegCompress(src,"lena.jpg",50);

	waitKey(0);

	return 0;
}



bool jpegCompress(Mat src, string filename, int quality)
{
	if (src.channels() == 3)
		cvtColor(src, src, CV_BGR2RGB);

	//--------------------------------------------------------
	//打开要写入的文件
	FILE *f = fopen(filename.c_str(), "wb");
	BYTE *data = new BYTE[src.step*src.rows];
	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.step; j++)
		{
			data[i*src.step + j] = src.at<unsigned char>(i, j);
		}
	}

	if (f == NULL)
	{
		delete[] data;
		//delete [] pDataConv;
		return false;
	}
	//--------------------------------------------------------
	//创建压缩文件对象
	struct jpeg_compress_struct jcs = { 0 };
	struct jpeg_error_mgr jem;
	jcs.err = jpeg_std_error(&jem);
	jpeg_create_compress(&jcs);

	//--------------------------------------------------------
	//jpeg参数设置
	int nComponent = src.channels();			//图像通道数
	jpeg_stdio_dest(&jcs, f);					//目标文件
	jcs.image_height = src.rows;				//图像高度
	jcs.image_width = src.cols;					//图像宽度
	jcs.input_components = nComponent;			//图像通道数
	if (nComponent == 1)						//图像颜色空间
		jcs.in_color_space = JCS_GRAYSCALE;
	else
		jcs.in_color_space = JCS_RGB;
	jpeg_set_defaults(&jcs);
	jpeg_set_quality(&jcs, quality, true);		//图像压缩质量

	//--------------------------------------------------------
	//图像压缩
	jpeg_start_compress(&jcs, TRUE);

	JSAMPROW row_pointer[1];			// 一行位图
	int row_stride;						// 每一行的字节数 
	row_stride = jcs.image_width * nComponent;

	// 对每一行进行压缩
	while (jcs.next_scanline < jcs.image_height)
	{
		row_pointer[0] = &data[jcs.next_scanline * row_stride];
		jpeg_write_scanlines(&jcs, row_pointer, 1);
	}
	//---------------------------------------------------------
	//结束图像压缩，释放jpeg_compress_struct
	jpeg_finish_compress(&jcs);
	jpeg_destroy_compress(&jcs);
	fclose(f);
	delete[] data;

	return true;
}
