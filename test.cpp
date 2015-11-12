// TestLibjpeg.cpp : Defines the entry point for the console application.
//
#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<fstream>
#include<cstdio>
#include<string>
#include "memory.h"
#include "string.h"
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<jpeglib.h>
using namespace std;

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef long				LONG;
//typedef void *PVOID;
//typedef PVOID HANDLE;
//typedef void *LPVOID;

// 调色板项
typedef struct tagRGBQUAD {
	BYTE    rgbBlue;
	BYTE    rgbGreen;
	BYTE    rgbRed;
	BYTE    rgbReserved;
} RGBQUAD;
// bmp头信息
typedef struct tagBITMAPINFOHEADER{
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} BITMAPINFOHEADER;
// bmp信息
typedef struct tagBITMAPINFO {
	BITMAPINFOHEADER    bmiHeader;
	RGBQUAD             bmiColors[1];
} BITMAPINFO;

#include <pshpack2.h>
//bmp文件头
typedef struct tagBITMAPFILEHEADER {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER;

/***********************************************************************
压缩图像到jpeg格式
如果压缩前图像为灰度图或24位图，压缩后图像色彩模式不变
如果压缩前图像为256色索引位图，压缩后变为灰度图
************************************************************************/
void bmptojpg(const char *strSourceFileName, const char *strDestFileName)
{
	BITMAPFILEHEADER bfh;		// bmp文件头
	BITMAPINFOHEADER bih;		// bmp头信息
	RGBQUAD rq[256];			// 调色板
	int nAdjust;		// 用于字节对齐

	BYTE *data = NULL;//new BYTE[bih.biWidth*bih.biHeight];
	//BYTE *pDataConv = NULL;//new BYTE[bih.biWidth*bih.biHeight];
	int nComponent = 0;

	// 打开图像文件
	FILE *f = fopen(strSourceFileName, "rb");
	if (f == NULL)
	{
		printf("Open file error!\n");
		return;
	}
	// 读取文件头
	fread(&bfh, sizeof(bfh), 1, f);
	// 读取图像信息
	fread(&bih, sizeof(bih), 1, f);

	switch (bih.biBitCount)
	{
	case 8:
		if (bfh.bfOffBits - 1024<54)
		{
			fclose(f);
			return;
		}
		// 8位字节对齐
		nAdjust = bih.biWidth % 4;
		if (nAdjust) nAdjust = 4 - nAdjust;

		data = new BYTE[(bih.biWidth + nAdjust)*bih.biHeight];
		//pDataConv = new BYTE[bih.biWidth*bih.biHeight];

		// 定位调色板，并读取调色板
		fseek(f, bfh.bfOffBits - 1024, SEEK_SET);
		fread(rq, sizeof(RGBQUAD), 256, f);
		// 读取位图
		fread(data, (bih.biWidth + nAdjust)*bih.biHeight, 1, f);
		fclose(f);
		nComponent = 1;
		break;
	case 24:
	{
			   // 8位字节对齐
			   nAdjust = bih.biWidth * 3 % 4;
			   if (nAdjust) nAdjust = 4 - nAdjust;
			   data = new BYTE[(bih.biWidth * 3 + nAdjust)*bih.biHeight];
			   //pDataConv = new BYTE[bih.biWidth*bih.biHeight*3];
			   fseek(f, bfh.bfOffBits, SEEK_SET);
			   fread(data, (bih.biWidth * 3 + nAdjust)*bih.biHeight, 1, f);
			   fclose(f);
			   for (int j = 0; j<bih.biHeight; j++){
				   for (int i = 0; i<bih.biWidth; i++)
				   {
					   BYTE red = data[j*(bih.biWidth * 3 + nAdjust) + i * 3];
					   data[j*(bih.biWidth * 3 + nAdjust) + i * 3] = data[j*(bih.biWidth * 3 + nAdjust) + i * 3 + 2];
					   data[j*(bih.biWidth * 3 + nAdjust) + i * 3 + 2] = red;
				   }
			   }
			   nComponent = 3;
			   break;
	}
	default:
		fclose(f);
		return;
	}

	struct jpeg_compress_struct jcs;
	struct jpeg_error_mgr jem;
	jcs.err = jpeg_std_error(&jem);

	jpeg_create_compress(&jcs);

	f = fopen(strDestFileName, "wb");
	if (f == NULL)
	{
		delete[] data;
		//delete [] pDataConv;
		return;
	}
	jpeg_stdio_dest(&jcs, f);
	jcs.image_width = bih.biWidth; 			// 为图的宽和高，单位为像素 
	jcs.image_height = bih.biHeight;
	jcs.input_components = nComponent;			// 1,表示灰度图， 如果是彩色位图，则为3 
	if (nComponent == 1)
		jcs.in_color_space = JCS_GRAYSCALE; //JCS_GRAYSCALE表示灰度图，JCS_RGB表示彩色图像 
	else
		jcs.in_color_space = JCS_RGB;

	jpeg_set_defaults(&jcs);
	jpeg_set_quality(&jcs, 60, true);

	jpeg_start_compress(&jcs, TRUE);

	JSAMPROW row_pointer[1];			// 一行位图
	int row_stride;						// 每一行的字节数 

	row_stride = jcs.image_width*nComponent;		// 如果不是索引图,此处需要乘以3

	// 对每一行进行压缩
	while (jcs.next_scanline < jcs.image_height) {
		row_pointer[0] = &data[(jcs.image_height - jcs.next_scanline - 1) * (row_stride + nAdjust)];
		jpeg_write_scanlines(&jcs, row_pointer, 1);
	}

	jpeg_finish_compress(&jcs);

	jpeg_destroy_compress(&jcs);

	fclose(f);
	delete[] data;

}
/**************************************************************************
压缩图像到jpeg格式，如果压缩前的图像不是24位图，则强行转换为24位图后压缩
**************************************************************************/
void bmptojpg24(const char *strSourceFileName, const char *strDestFileName)
{
	BITMAPFILEHEADER bfh;		// bmp文件头
	BITMAPINFOHEADER bih;		// bmp头信息
	RGBQUAD rq[256];			// 调色板
	int i = 0, j = 0;
	int nAdjust; // 用于字节对齐
	int nAdjust24; // 用于字节对齐

	BYTE *data = NULL;//new BYTE[bih.biWidth*bih.biHeight];
	BYTE *pData24 = NULL;//new BYTE[bih.biWidth*bih.biHeight];
	int nComponent = 0;

	// 打开图像文件
	FILE *f = fopen(strSourceFileName, "rb");
	if (f == NULL)
	{
		printf("Open file error!\n");
		return;
	}
	// 读取文件头
	fread(&bfh, sizeof(bfh), 1, f);
	// 读取图像信息
	fread(&bih, sizeof(bih), 1, f);
	// 8位字节对齐
	nAdjust = bih.biWidth % 4;
	if (nAdjust) nAdjust = 4 - nAdjust;
	// 24位字节对齐
	nAdjust24 = bih.biWidth * 3 % 4;
	if (nAdjust24) nAdjust24 = 4 - nAdjust24;

	switch (bih.biBitCount)
	{
	case 8:
		if (bfh.bfOffBits - 1024<54)
		{
			fclose(f);
			return;
		}

		data = new BYTE[(bih.biWidth + nAdjust)*bih.biHeight];
		pData24 = new BYTE[(bih.biWidth * 3 + nAdjust24)*bih.biHeight];

		// 定位调色板，并读取调色板
		fseek(f, bfh.bfOffBits - 1024, SEEK_SET);
		fread(rq, sizeof(RGBQUAD), 256, f);
		// 读取位图
		fread(data, bih.biWidth*bih.biHeight, 1, f);
		fclose(f);
		nComponent = 3;
		for (j = 0; j<bih.biHeight; j++) {
			for (i = 0; i<bih.biWidth; i++)
			{
				pData24[j*(bih.biWidth * 3 + nAdjust24) + i * 3] = rq[data[j*(bih.biWidth + nAdjust) + i]].rgbRed;
				pData24[j*(bih.biWidth * 3 + nAdjust24) + i * 3 + 1] = rq[data[j*(bih.biWidth + nAdjust) + i]].rgbGreen;
				pData24[j*(bih.biWidth * 3 + nAdjust24) + i * 3 + 2] = rq[data[j*(bih.biWidth + nAdjust) + i]].rgbBlue;
			}
		}
		break;
	case 24:
	{
			   data = new BYTE[(bih.biWidth * 3 + nAdjust24)*bih.biHeight];
			   pData24 = new BYTE[(bih.biWidth * 3 + nAdjust24)*bih.biHeight];
			   fseek(f, bfh.bfOffBits, SEEK_SET);
			   fread(data, (bih.biWidth * 3 + nAdjust24)*bih.biHeight, 1, f);
			   fclose(f);
			   for (j = 0; j<bih.biHeight; j++){
				   for (i = 0; i<bih.biWidth; i++)
				   {
					   pData24[j*(bih.biWidth * 3 + nAdjust24) + i * 3] = data[j*(bih.biWidth * 3 + nAdjust24) + i * 3 + 2];
					   pData24[j*(bih.biWidth * 3 + nAdjust24) + i * 3 + 1] = data[j*(bih.biWidth * 3 + nAdjust24) + i * 3 + 1];
					   pData24[j*(bih.biWidth * 3 + nAdjust24) + i * 3 + 2] = data[j*(bih.biWidth * 3 + nAdjust24) + i * 3];
				   }
			   }
			   nComponent = 3;
			   break;
	}
	default:
		fclose(f);
		return;
	}

	// 以上图像读取完毕

	struct jpeg_compress_struct jcs;
	struct jpeg_error_mgr jem;
	jcs.err = jpeg_std_error(&jem);

	jpeg_create_compress(&jcs);

	f = fopen(strDestFileName, "wb");
	if (f == NULL)
	{
		delete[] data;
		//delete [] pDataConv;
		return;
	}
	jpeg_stdio_dest(&jcs, f);
	jcs.image_width = bih.biWidth; 			// 为图的宽和高，单位为像素 
	jcs.image_height = bih.biHeight;
	jcs.input_components = nComponent;			// 1,表示灰度图， 如果是彩色位图，则为3 
	if (nComponent == 1)
		jcs.in_color_space = JCS_GRAYSCALE; //JCS_GRAYSCALE表示灰度图，JCS_RGB表示彩色图像 
	else
		jcs.in_color_space = JCS_RGB;

	jpeg_set_defaults(&jcs);
	jpeg_set_quality(&jcs, 60, true);

	jpeg_start_compress(&jcs, TRUE);

	JSAMPROW row_pointer[1];			// 一行位图
	int row_stride;						// 每一行的字节数 

	row_stride = jcs.image_width*nComponent;		// 如果不是索引图,此处需要乘以3

	// 对每一行进行压缩
	while (jcs.next_scanline < jcs.image_height) {
		row_pointer[0] = &pData24[(jcs.image_height - jcs.next_scanline - 1) * (row_stride + nAdjust24)];
		jpeg_write_scanlines(&jcs, row_pointer, 1);
	}

	jpeg_finish_compress(&jcs);

	jpeg_destroy_compress(&jcs);

	fclose(f);
	delete[] data;
	delete[] pData24;

}

/***********************************************
*解压缩jpeg到bmp格式
*对于灰度图和24位图，图像解压后正常
*对于256色索引位图，图像解压后为灰度图
**************************************************/
void jpgtobmp(const char *strSourceFileName, const char *strDestFileName)
{
	BITMAPFILEHEADER bfh;		// bmp文件头
	BITMAPINFOHEADER bih;		// bmp头信息
	RGBQUAD rq[256];			// 调色板
	int nAdjust; // 用于字节对齐

	BYTE *data = NULL;//new BYTE[bih.biWidth*bih.biHeight];
	//BYTE *pDataConv = NULL;//new BYTE[bih.biWidth*bih.biHeight];
	int nComponent = 0;

	// 声明解压缩对象及错误信息管理器
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	FILE *f = fopen(strSourceFileName, "rb");
	if (f == NULL)
	{
		printf("Open file error!\n");
		return;
	}
	// 
	jpeg_stdio_src(&cinfo, f);
	jpeg_read_header(&cinfo, TRUE);
	nAdjust = cinfo.image_width*cinfo.num_components % 4;
	if (nAdjust) nAdjust = 4 - nAdjust;
	data = new BYTE[(cinfo.image_width*cinfo.num_components + nAdjust)*cinfo.image_height];

	jpeg_start_decompress(&cinfo);

	JSAMPROW row_pointer[1];
	while (cinfo.output_scanline < cinfo.output_height)
	{
		row_pointer[0] = &data[(cinfo.output_height - cinfo.output_scanline - 1)*(cinfo.image_width*cinfo.num_components + nAdjust)];
		jpeg_read_scanlines(&cinfo, row_pointer,
			1);
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	fclose(f);

	f = fopen(strDestFileName, "wb");
	if (f == NULL)
	{
		delete[] data;
		//delete [] pDataConv;
		return;
	}
	// 写文件头
	memset(&bfh, 0, sizeof(bfh));
	bfh.bfSize = sizeof(bfh)+sizeof(bih);
	bfh.bfOffBits = sizeof(bfh)+sizeof(bih);
	if (cinfo.num_components == 1)
	{
		bfh.bfOffBits += 1024;
		bfh.bfSize += 1024;
	}

	bfh.bfSize += (cinfo.image_width*cinfo.num_components + nAdjust)*cinfo.image_height;
	bfh.bfType = 0x4d42;
	fwrite(&bfh, sizeof(bfh), 1, f);
	// 写图像信息
	bih.biBitCount = cinfo.num_components * 8;
	bih.biSize = sizeof(bih);
	bih.biWidth = cinfo.image_width;
	bih.biHeight = cinfo.image_height;
	bih.biPlanes = 1;
	bih.biCompression = 0;
	bih.biSizeImage = (cinfo.image_width*cinfo.num_components + nAdjust)*cinfo.image_height;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;
	fwrite(&bih, sizeof(bih), 1, f);
	// 写调色板
	if (cinfo.num_components == 1)
	{
		for (int i = 0; i<256; i++)
		{
			rq[i].rgbBlue = i;
			rq[i].rgbGreen = i;
			rq[i].rgbRed = i;
			rq[i].rgbReserved = 0;
		}
		fwrite(rq, 1024, 1, f);
	}

	if (cinfo.num_components == 3)
	{
		// 调整rgb顺序
		for (int j = 0; j<bih.biHeight; j++)
		for (int i = 0; i<bih.biWidth; i++)
		{
			BYTE red = data[j*(cinfo.image_width*cinfo.num_components + nAdjust) + i * 3];
			data[j*(cinfo.image_width*cinfo.num_components + nAdjust) + i * 3] = data[j*(cinfo.image_width*cinfo.num_components + nAdjust) + i * 3 + 2];
			data[j*(cinfo.image_width*cinfo.num_components + nAdjust) + i * 3 + 2] = red;
		}
	}
	fwrite(data, (cinfo.image_width*cinfo.num_components + nAdjust)*cinfo.image_height, 1, f);
	fclose(f);
	delete[] data;

}
int main(int argc, char* argv[])
{
	switch (argc)
	{
	case 4:
		if (strcmp(argv[1], "j") == 0)
		{
			bmptojpg(argv[2], argv[3]);
			break;
		}
		else if (strcmp(argv[1], "j24") == 0)
		{
			bmptojpg24(argv[2], argv[3]);
			break;
		}
		else if (strcmp(argv[1], "b") == 0)
		{
			jpgtobmp(argv[2], argv[3]);
			break;
		}

	default:
		printf("本程序利用了libjpeg库，该libjpeg.lib在vs 2005下重新编译过，可以直接使用.\n");
		printf("转换bmp位图为jpg格式，或解压缩jpg格式图像为bmp格式\n");
		printf("TestLibjpeg.exe j|j24|b s_name d_name\n");
		printf("如：TestLibjpeg.exe j 05.bmp 05.jpg\n");
		printf("TestLibjpeg.exe j24 05.bmp 05.jpg\n");
		printf("TestLibjpeg.exe b 05.jpg 05.bmp\n");
		printf("\n格式如下：\n");
		printf("\tj:压缩图像到jpeg格式,如果压缩前图像为灰度图或24位图，压缩后图像色彩模式不变,如果压缩前图像为256色索引位图，压缩后变为灰度图;\n");
		printf("\tj24:压缩图像到jpeg格式，如果压缩前的图像不是24位图，则强行转换为24位图后压缩;\n");
		printf("\tb:解压缩jpeg到bmp格式,对于灰度图和24位图，图像解压后正常,对于256色索引位图，图像解压后为灰度图\n");
		break;
	}
	return 0;
}

