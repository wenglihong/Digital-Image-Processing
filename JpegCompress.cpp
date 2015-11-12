/*************************************************
@brief: ʹ��libjpeg.libʵ��ͼ��ѹ��д�뵽jpeg�ļ�

@author�������

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
@brief ��ͼ��ѹ��Ϊjpeg��ʽ
@note
@param src Դͼ��
@param filename д���ļ�������
@param quality ͼ��ѹ���ȣ�0-100��
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
	//��Ҫд����ļ�
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
	//����ѹ���ļ�����
	struct jpeg_compress_struct jcs = { 0 };
	struct jpeg_error_mgr jem;
	jcs.err = jpeg_std_error(&jem);
	jpeg_create_compress(&jcs);

	//--------------------------------------------------------
	//jpeg��������
	int nComponent = src.channels();			//ͼ��ͨ����
	jpeg_stdio_dest(&jcs, f);					//Ŀ���ļ�
	jcs.image_height = src.rows;				//ͼ��߶�
	jcs.image_width = src.cols;					//ͼ����
	jcs.input_components = nComponent;			//ͼ��ͨ����
	if (nComponent == 1)						//ͼ����ɫ�ռ�
		jcs.in_color_space = JCS_GRAYSCALE;
	else
		jcs.in_color_space = JCS_RGB;
	jpeg_set_defaults(&jcs);
	jpeg_set_quality(&jcs, quality, true);		//ͼ��ѹ������

	//--------------------------------------------------------
	//ͼ��ѹ��
	jpeg_start_compress(&jcs, TRUE);

	JSAMPROW row_pointer[1];			// һ��λͼ
	int row_stride;						// ÿһ�е��ֽ��� 
	row_stride = jcs.image_width * nComponent;

	// ��ÿһ�н���ѹ��
	while (jcs.next_scanline < jcs.image_height)
	{
		row_pointer[0] = &data[jcs.next_scanline * row_stride];
		jpeg_write_scanlines(&jcs, row_pointer, 1);
	}
	//---------------------------------------------------------
	//����ͼ��ѹ�����ͷ�jpeg_compress_struct
	jpeg_finish_compress(&jcs);
	jpeg_destroy_compress(&jcs);
	fclose(f);
	delete[] data;

	return true;
}
