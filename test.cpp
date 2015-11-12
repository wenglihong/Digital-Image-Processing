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

// ��ɫ����
typedef struct tagRGBQUAD {
	BYTE    rgbBlue;
	BYTE    rgbGreen;
	BYTE    rgbRed;
	BYTE    rgbReserved;
} RGBQUAD;
// bmpͷ��Ϣ
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
// bmp��Ϣ
typedef struct tagBITMAPINFO {
	BITMAPINFOHEADER    bmiHeader;
	RGBQUAD             bmiColors[1];
} BITMAPINFO;

#include <pshpack2.h>
//bmp�ļ�ͷ
typedef struct tagBITMAPFILEHEADER {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} BITMAPFILEHEADER;

/***********************************************************************
ѹ��ͼ��jpeg��ʽ
���ѹ��ǰͼ��Ϊ�Ҷ�ͼ��24λͼ��ѹ����ͼ��ɫ��ģʽ����
���ѹ��ǰͼ��Ϊ256ɫ����λͼ��ѹ�����Ϊ�Ҷ�ͼ
************************************************************************/
void bmptojpg(const char *strSourceFileName, const char *strDestFileName)
{
	BITMAPFILEHEADER bfh;		// bmp�ļ�ͷ
	BITMAPINFOHEADER bih;		// bmpͷ��Ϣ
	RGBQUAD rq[256];			// ��ɫ��
	int nAdjust;		// �����ֽڶ���

	BYTE *data = NULL;//new BYTE[bih.biWidth*bih.biHeight];
	//BYTE *pDataConv = NULL;//new BYTE[bih.biWidth*bih.biHeight];
	int nComponent = 0;

	// ��ͼ���ļ�
	FILE *f = fopen(strSourceFileName, "rb");
	if (f == NULL)
	{
		printf("Open file error!\n");
		return;
	}
	// ��ȡ�ļ�ͷ
	fread(&bfh, sizeof(bfh), 1, f);
	// ��ȡͼ����Ϣ
	fread(&bih, sizeof(bih), 1, f);

	switch (bih.biBitCount)
	{
	case 8:
		if (bfh.bfOffBits - 1024<54)
		{
			fclose(f);
			return;
		}
		// 8λ�ֽڶ���
		nAdjust = bih.biWidth % 4;
		if (nAdjust) nAdjust = 4 - nAdjust;

		data = new BYTE[(bih.biWidth + nAdjust)*bih.biHeight];
		//pDataConv = new BYTE[bih.biWidth*bih.biHeight];

		// ��λ��ɫ�壬����ȡ��ɫ��
		fseek(f, bfh.bfOffBits - 1024, SEEK_SET);
		fread(rq, sizeof(RGBQUAD), 256, f);
		// ��ȡλͼ
		fread(data, (bih.biWidth + nAdjust)*bih.biHeight, 1, f);
		fclose(f);
		nComponent = 1;
		break;
	case 24:
	{
			   // 8λ�ֽڶ���
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
	jcs.image_width = bih.biWidth; 			// Ϊͼ�Ŀ�͸ߣ���λΪ���� 
	jcs.image_height = bih.biHeight;
	jcs.input_components = nComponent;			// 1,��ʾ�Ҷ�ͼ�� ����ǲ�ɫλͼ����Ϊ3 
	if (nComponent == 1)
		jcs.in_color_space = JCS_GRAYSCALE; //JCS_GRAYSCALE��ʾ�Ҷ�ͼ��JCS_RGB��ʾ��ɫͼ�� 
	else
		jcs.in_color_space = JCS_RGB;

	jpeg_set_defaults(&jcs);
	jpeg_set_quality(&jcs, 60, true);

	jpeg_start_compress(&jcs, TRUE);

	JSAMPROW row_pointer[1];			// һ��λͼ
	int row_stride;						// ÿһ�е��ֽ��� 

	row_stride = jcs.image_width*nComponent;		// �����������ͼ,�˴���Ҫ����3

	// ��ÿһ�н���ѹ��
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
ѹ��ͼ��jpeg��ʽ�����ѹ��ǰ��ͼ����24λͼ����ǿ��ת��Ϊ24λͼ��ѹ��
**************************************************************************/
void bmptojpg24(const char *strSourceFileName, const char *strDestFileName)
{
	BITMAPFILEHEADER bfh;		// bmp�ļ�ͷ
	BITMAPINFOHEADER bih;		// bmpͷ��Ϣ
	RGBQUAD rq[256];			// ��ɫ��
	int i = 0, j = 0;
	int nAdjust; // �����ֽڶ���
	int nAdjust24; // �����ֽڶ���

	BYTE *data = NULL;//new BYTE[bih.biWidth*bih.biHeight];
	BYTE *pData24 = NULL;//new BYTE[bih.biWidth*bih.biHeight];
	int nComponent = 0;

	// ��ͼ���ļ�
	FILE *f = fopen(strSourceFileName, "rb");
	if (f == NULL)
	{
		printf("Open file error!\n");
		return;
	}
	// ��ȡ�ļ�ͷ
	fread(&bfh, sizeof(bfh), 1, f);
	// ��ȡͼ����Ϣ
	fread(&bih, sizeof(bih), 1, f);
	// 8λ�ֽڶ���
	nAdjust = bih.biWidth % 4;
	if (nAdjust) nAdjust = 4 - nAdjust;
	// 24λ�ֽڶ���
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

		// ��λ��ɫ�壬����ȡ��ɫ��
		fseek(f, bfh.bfOffBits - 1024, SEEK_SET);
		fread(rq, sizeof(RGBQUAD), 256, f);
		// ��ȡλͼ
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

	// ����ͼ���ȡ���

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
	jcs.image_width = bih.biWidth; 			// Ϊͼ�Ŀ�͸ߣ���λΪ���� 
	jcs.image_height = bih.biHeight;
	jcs.input_components = nComponent;			// 1,��ʾ�Ҷ�ͼ�� ����ǲ�ɫλͼ����Ϊ3 
	if (nComponent == 1)
		jcs.in_color_space = JCS_GRAYSCALE; //JCS_GRAYSCALE��ʾ�Ҷ�ͼ��JCS_RGB��ʾ��ɫͼ�� 
	else
		jcs.in_color_space = JCS_RGB;

	jpeg_set_defaults(&jcs);
	jpeg_set_quality(&jcs, 60, true);

	jpeg_start_compress(&jcs, TRUE);

	JSAMPROW row_pointer[1];			// һ��λͼ
	int row_stride;						// ÿһ�е��ֽ��� 

	row_stride = jcs.image_width*nComponent;		// �����������ͼ,�˴���Ҫ����3

	// ��ÿһ�н���ѹ��
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
*��ѹ��jpeg��bmp��ʽ
*���ڻҶ�ͼ��24λͼ��ͼ���ѹ������
*����256ɫ����λͼ��ͼ���ѹ��Ϊ�Ҷ�ͼ
**************************************************/
void jpgtobmp(const char *strSourceFileName, const char *strDestFileName)
{
	BITMAPFILEHEADER bfh;		// bmp�ļ�ͷ
	BITMAPINFOHEADER bih;		// bmpͷ��Ϣ
	RGBQUAD rq[256];			// ��ɫ��
	int nAdjust; // �����ֽڶ���

	BYTE *data = NULL;//new BYTE[bih.biWidth*bih.biHeight];
	//BYTE *pDataConv = NULL;//new BYTE[bih.biWidth*bih.biHeight];
	int nComponent = 0;

	// ������ѹ�����󼰴�����Ϣ������
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
	// д�ļ�ͷ
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
	// дͼ����Ϣ
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
	// д��ɫ��
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
		// ����rgb˳��
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
		printf("������������libjpeg�⣬��libjpeg.lib��vs 2005�����±����������ֱ��ʹ��.\n");
		printf("ת��bmpλͼΪjpg��ʽ�����ѹ��jpg��ʽͼ��Ϊbmp��ʽ\n");
		printf("TestLibjpeg.exe j|j24|b s_name d_name\n");
		printf("�磺TestLibjpeg.exe j 05.bmp 05.jpg\n");
		printf("TestLibjpeg.exe j24 05.bmp 05.jpg\n");
		printf("TestLibjpeg.exe b 05.jpg 05.bmp\n");
		printf("\n��ʽ���£�\n");
		printf("\tj:ѹ��ͼ��jpeg��ʽ,���ѹ��ǰͼ��Ϊ�Ҷ�ͼ��24λͼ��ѹ����ͼ��ɫ��ģʽ����,���ѹ��ǰͼ��Ϊ256ɫ����λͼ��ѹ�����Ϊ�Ҷ�ͼ;\n");
		printf("\tj24:ѹ��ͼ��jpeg��ʽ�����ѹ��ǰ��ͼ����24λͼ����ǿ��ת��Ϊ24λͼ��ѹ��;\n");
		printf("\tb:��ѹ��jpeg��bmp��ʽ,���ڻҶ�ͼ��24λͼ��ͼ���ѹ������,����256ɫ����λͼ��ͼ���ѹ��Ϊ�Ҷ�ͼ\n");
		break;
	}
	return 0;
}

