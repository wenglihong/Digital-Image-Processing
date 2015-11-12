#include "cxcore.h"
#include "cv.h"
#include "highgui.h"
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;
double D0 = 30;
void ILPF(CvMat* src, const double D0)
{
	int i, j;
	int state = -1;
	double tempD;
	long width, height;
	width = src->width;
	height = src->height;

	long x, y;
	x = width / 2;
	y = height / 2;

	CvMat* H_mat;
	H_mat = cvCreateMat(src->height, src->width, CV_64FC2);
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
				tempD = (double)(i * i + j * j); tempD = sqrt(tempD); break;
			case 1:
				tempD = (double)((height - i) * (height - i) + j * j); tempD = sqrt(tempD); break;
			case 2:
				tempD = (double)(i * i + (width - j) * (width - j)); tempD = sqrt(tempD); break;
			case 3:
				tempD = (double)((height - i) * (height - i) + (width - j) * (width - j)); tempD = sqrt(tempD); break;
			default:
				break;
			}

			//��ά��˹��ͨ�˲������ݺ���
			/*tempD = exp(-0.5 * pow(tempD / D0, 2));
			((double*)(H_mat->data.ptr + H_mat->step * i))[j * 2] = tempD;
			((double*)(H_mat->data.ptr + H_mat->step * i))[j * 2 + 1] = 0.0;*/

			//˥��ϵ��Ϊ2�Ķ�άָ����ͨ�˲������ݺ���
			/*	tempD = exp(-pow(tempD / D0, 2));
			((double*)(H_mat->data.ptr + H_mat->step * i))[j * 2] = tempD;
			((double*)(H_mat->data.ptr + H_mat->step * i))[j * 2 + 1] = 0.0;*/

			//2�װ�����˼��ͨ�˲������ݺ���
			tempD = 1 / (1 + pow(tempD / D0, 2 * 2));
			((double*)(H_mat->data.ptr + H_mat->step * i))[j * 2] = tempD;
			((double*)(H_mat->data.ptr + H_mat->step * i))[j * 2 + 1] = 0.0;


			//��ά�����ͨ�˲������ݺ���
			//	if(tempD <= D0)
			//	{
			//		((double*)(H_mat->data.ptr + H_mat->step * i))[j *2] = 1.0;
			//    	//((double*)(H_mat->data.ptr + H_mat->step * i))[j * 2 + 1] = 0.0;
			//	}
			//	else
			//	{
			//		((double*)(H_mat->data.ptr + H_mat->step * i))[j*2 ] = 0.0;
			//    	//((double*)(H_mat->data.ptr + H_mat->step * i))[j * 2 + 1] = 0.0;
			//	}
		}
	}
	cvMulSpectrums(src, H_mat, src, CV_DXT_ROWS);
	cvReleaseMat(&H_mat);
}



int main(int argc, char ** argv)
{
	const char* filename = argc >= 2 ? argv[1] : "lena.png";
	IplImage * im;

	IplImage * realInput;
	IplImage * imaginaryInput;
	IplImage * complexInput;
	int dft_M, dft_N;
	CvMat* dft_A, tmp, *dft_B;
	IplImage * image_Re;
	IplImage * image_Im;
	double m, M;

	im = cvLoadImage(filename, CV_LOAD_IMAGE_GRAYSCALE);
	if (!im)
		return -1;

	realInput = cvCreateImage(cvGetSize(im), IPL_DEPTH_64F, 1);
	imaginaryInput = cvCreateImage(cvGetSize(im), IPL_DEPTH_64F, 1);
	complexInput = cvCreateImage(cvGetSize(im), IPL_DEPTH_64F, 2);

	cvScale(im, realInput, 1.0, 0.0);//�����Ա任��[0,1]
	cvZero(imaginaryInput);//ͼ������
	cvMerge(realInput, imaginaryInput, NULL, NULL, complexInput);//����ͨ��dst0��dst1��dst2,dst3��Ϊdst��

	dft_M = cvGetOptimalDFTSize(im->height - 1);//���ڸ�����ʸ���ߴ緵������DFT�ߴ�
	dft_N = cvGetOptimalDFTSize(im->width - 1);
	dft_B = cvCreateMat(dft_M, dft_N, CV_64FC2);//��������
	dft_A = cvCreateMat(dft_M, dft_N, CV_64FC2);
	cvZero(dft_A);//ԭ�������У�û����仰��
	cvZero(dft_B);

	image_Re = cvCreateImage(cvSize(dft_N, dft_M), IPL_DEPTH_64F, 1);
	image_Im = cvCreateImage(cvSize(dft_N, dft_M), IPL_DEPTH_64F, 1);

	cvGetSubRect(dft_A, &tmp, cvRect(0, 0, im->width, im->height));//���������ͼ������ľ��������Ӽ��ľ���ͷ 
	cvCopy(complexInput, &tmp, NULL);


	cvDFT(dft_A, dft_A, CV_DXT_FORWARD, complexInput->height);

	ILPF(dft_A, D0);
	cvDFT(dft_A, dft_A, CV_DXT_INVERSE, complexInput->height);///��任 ��ά����Ҷ�任

	cvNamedWindow("win");
	cvNamedWindow("magnitude");
	cvShowImage("win", im);

	cvSplit(dft_A, image_Re, image_Im, 0, 0);

	cvMinMaxLoc(image_Re, &m, &M, NULL, NULL, NULL);//����������������ȫ����Сֵ�����ֵ 
	cvScale(image_Re, image_Re, 1.0 / (M - m), 1.0*(-m) / (M - m));

	cvShowImage("magnitude", image_Re);

	cvNamedWindow("image_im");
	cvShowImage("image_im", image_Im);

	cvWaitKey(-1);
	return 0;
}