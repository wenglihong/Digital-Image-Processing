#include "cv.h"
#include "math.h"
#include "cxcore.h"
#include "highgui.h"

#include <IOSTREAM> // write for test you can neglect it
using namespace std;

#define GLPF 0 // gaussian lowpass filter
#define ILPF 1 // ideal lowpass filter
#define BLPF 2 // butterworth lowpass filter

////////////////////////compute the meshgrid arrays needed for LPF//////////////////////////////////////////////////
// CDM compute meshgrid frequency matrices (ok!)
// see Gonzalez Digital image processing using matlab page93 function dftuv
void CDM(int M, int N, CvMat *mat)
{
	int width = mat->rows;
	int height = mat->cols;

	if (M != width && N != height)
	{
		cout << "ERROR! THE SIZE DOES NOT MATCH WITH MAT" << endl;
		return;
	}

	if (cvGetElemType(mat) < CV_32F)
	{
		cout << "ERROR! THE TYPE DOES NOT MATCH WITH MAT" << endl;
		return;
	}

	CvMat *U, *V;
	U = cvCreateMat(M, N, CV_32FC1);
	V = cvCreateMat(M, N, CV_32FC1);

	for (int u = 0; u < M; ++u)
	for (int v = 0; v < N; ++v)
	{
		float tm1, tm2;
		tm1 = (float)((u > cvRound(M / 2)) ? u - M : u);
		tm2 = (float)((v > cvRound(N / 2)) ? v - N : v);

		*((float *)CV_MAT_ELEM_PTR(*U, u, v)) = tm1;

		*((float *)CV_MAT_ELEM_PTR(*V, u, v)) = tm2;
	}

	for (int u = 0; u < M; ++u)
	for (int v = 0; v < N; ++v)
	{
		float t1, t2;
		t1 = CV_MAT_ELEM(*U, float, u, v);
		t2 = CV_MAT_ELEM(*V, float, u, v);
		*((float *)CV_MAT_ELEM_PTR(*mat, u, v)) = sqrt(t1*t1 + t2*t2);

	}

}
////////////////////////////create low pass filter//////////////////////////////////////////////
// for ideal and gaussian low pass filter
void lpfilter(int type, CvMat *matD, CvMat *matH, float D0)
{
	if (D0 < 0)
	{
		cout << "ERROR! D0 MUST BE POSITIVE" << endl;
		return;
	}

	int w = matD->rows;
	int h = matD->cols;

	switch (type)
	{


	case ILPF:
	{
				 for (int u = 0; u < w; ++u)
				 for (int v = 0; v < h; ++v)
				 {
					 float Elem_D;
					 Elem_D = CV_MAT_ELEM(*matD, float, u, v);
					 if (Elem_D <= D0)
						 *((float *)CV_MAT_ELEM_PTR(*matH, u, v)) = 1.0;
					 else
						 *((float *)CV_MAT_ELEM_PTR(*matH, u, v)) = 0.0;

				 }

	}
		break;
	case GLPF:
	{
				 for (int u = 0; u < w; ++u)
				 for (int v = 0; v < h; ++v)
				 {
					 float Elem_D, h;
					 Elem_D = CV_MAT_ELEM(*matD, float, u, v);

					 h = exp(-(Elem_D*Elem_D) / (2 * D0*D0));

					 *((float *)CV_MAT_ELEM_PTR(*matH, u, v)) = h;


				 }
	}
		break;
	default:
		cout << "NO TYPE FINDED OR ERROR TYPE!" << endl;
		break;
	}

}
// for butterworth low pass filter 
void lpfilter(int type, CvMat *matD, CvMat *matH, float D0, int n)
{
	if (D0 < 0)
	{
		cout << "ERROR! D0 MUST BE POSITIVE" << endl;
		return;
	}

	int w = matD->rows;
	int h = matD->cols;

	switch (type)
	{
	case BLPF:
	{
				 for (int u = 0; u < w; ++u)
				 for (int v = 0; v < h; ++v)
				 {
					 float Elem_D, t;
					 Elem_D = CV_MAT_ELEM(*matD, float, u, v) / D0;
					 t = pow(Elem_D, 2 * n);
					 *((float *)CV_MAT_ELEM_PTR(*matH, u, v)) = 1 / (1 + t);
				 }
	}
		break;
	default:
		cout << "ERROR TYPE!" << endl;
		break;
	}
}
//////////////////////////fftshift////////////////////////////////////////////////
// Rearrange the quadrants of Fourier image so that the origin is at
// the image center
// src & dst arrays of equal size & type
// code comes from http://www.opencv.org.cn/
void cvShiftDFT(CvArr * src_arr, CvArr * dst_arr)
{
	CvMat * tmp;
	CvMat q1stub, q2stub;
	CvMat q3stub, q4stub;
	CvMat d1stub, d2stub;
	CvMat d3stub, d4stub;
	CvMat * q1, *q2, *q3, *q4;
	CvMat * d1, *d2, *d3, *d4;

	CvSize size = cvGetSize(src_arr);
	CvSize dst_size = cvGetSize(dst_arr);
	int cx, cy;

	if (dst_size.width != size.width ||
		dst_size.height != size.height){
		cvError(CV_StsUnmatchedSizes, "cvShiftDFT", "Source and Destination arrays must have equal sizes", __FILE__, __LINE__);
	}

	if (src_arr == dst_arr){
		tmp = cvCreateMat(size.height / 2, size.width / 2, cvGetElemType(src_arr));
	}

	cx = size.width / 2;
	cy = size.height / 2; // image center

	q1 = cvGetSubRect(src_arr, &q1stub, cvRect(0, 0, cx, cy));
	q2 = cvGetSubRect(src_arr, &q2stub, cvRect(cx, 0, cx, cy));
	q3 = cvGetSubRect(src_arr, &q3stub, cvRect(cx, cy, cx, cy));
	q4 = cvGetSubRect(src_arr, &q4stub, cvRect(0, cy, cx, cy));
	d1 = cvGetSubRect(dst_arr, &d1stub, cvRect(0, 0, cx, cy));
	d2 = cvGetSubRect(dst_arr, &d2stub, cvRect(cx, 0, cx, cy));
	d3 = cvGetSubRect(dst_arr, &d3stub, cvRect(cx, cy, cx, cy));
	d4 = cvGetSubRect(dst_arr, &d4stub, cvRect(0, cy, cx, cy));

	if (src_arr != dst_arr){
		if (!CV_ARE_TYPES_EQ(q1, d1)){
			cvError(CV_StsUnmatchedFormats, "cvShiftDFT", "Source and Destination arrays must have the same format", __FILE__, __LINE__);
		}
		cvCopy(q3, d1, 0);
		cvCopy(q4, d2, 0);
		cvCopy(q1, d3, 0);
		cvCopy(q2, d4, 0);
	}
	else{
		cvCopy(q3, tmp, 0);
		cvCopy(q1, q3, 0);
		cvCopy(tmp, q1, 0);
		cvCopy(q4, tmp, 0);
		cvCopy(q2, q4, 0);
		cvCopy(tmp, q2, 0);
	}
}
////////////////////////perform fourier transform//////////////////////////////////////////////////
//fft2
// code comes from http://www.opencv.org.cn/
void fft2(IplImage *src, CvMat *dst)
{
	IplImage * realInput;
	IplImage * imaginaryInput;
	IplImage * complexInput;
	int dft_M, dft_N;
	CvMat* dft_A, tmp;
	IplImage * image_Re;
	IplImage * image_Im;

	realInput = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	imaginaryInput = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	complexInput = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 2);

	cvScale(src, realInput, 1.0, 0.0);
	cvZero(imaginaryInput);
	cvMerge(realInput, imaginaryInput, NULL, NULL, complexInput);

	dft_M = cvGetOptimalDFTSize(src->height - 1);
	dft_N = cvGetOptimalDFTSize(src->width - 1);

	dft_A = cvCreateMat(dft_M, dft_N, CV_32FC2);
	image_Re = cvCreateImage(cvSize(dft_N, dft_M), IPL_DEPTH_32F, 1);
	image_Im = cvCreateImage(cvSize(dft_N, dft_M), IPL_DEPTH_32F, 1);

	// copy A to dft_A and pad dft_A with zeros
	cvGetSubRect(dft_A, &tmp, cvRect(0, 0, src->width, src->height));
	cvCopy(complexInput, &tmp, NULL);
	if (dft_A->cols > src->width)
	{
		cvGetSubRect(dft_A, &tmp, cvRect(src->width, 0, dft_A->cols - src->width, src->height));
		cvZero(&tmp);
	}

	// no need to pad bottom part of dft_A with zeros because of
	// use nonzero_rows parameter in cvDFT() call below

	cvDFT(dft_A, dft_A, CV_DXT_FORWARD, complexInput->height);

	cvCopy(dft_A, dst);

	cvReleaseImage(&realInput);
	cvReleaseImage(&imaginaryInput);
	cvReleaseImage(&complexInput);
	cvReleaseImage(&image_Re);
	cvReleaseImage(&image_Im);

}

int main()
{
	IplImage *src = cvLoadImage("lena.bmp", CV_LOAD_IMAGE_GRAYSCALE);

	if (!src)
	{
		cout << "NO IMAGE FIND!" << endl;
		return -1;
	}

	int M = src->height;
	int N = src->width;

	CvMat *matD; // create mat for meshgrid frequency matrices
	matD = cvCreateMat(M, N, CV_32FC1);

	CDM(M, N, matD);

	CvMat *matH;
	matH = cvCreateMat(M, N, CV_32FC1); // mat for lowpass filter

	float D0 = 10.0;
	lpfilter(GLPF, matD, matH, D0);

	IplImage *srcshift; // shift center
	srcshift = cvCloneImage(src);

	CvMat *Fourier;
	Fourier = cvCreateMat(M, N, CV_32FC2);



	cvShiftDFT(srcshift, srcshift);
	fft2(srcshift, Fourier);


	IplImage *image_re, *image_im;
	image_re = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);
	image_im = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 1);

	cvSplit(Fourier, image_re, image_im, 0, 0);

	cvMul(image_re, matH, image_re);// filter
	cvMul(image_im, matH, image_im);

	IplImage *dst;
	dst = cvCreateImage(cvGetSize(src), IPL_DEPTH_32F, 2);

	cvMerge(image_re, image_im, 0, 0, dst);
	cvDFT(dst, dst, CV_DXT_INV_SCALE);// fourier inverse transform

	cvZero(image_re);
	cvZero(image_im);

	cvSplit(dst, image_re, image_im, 0, 0);
	cvShiftDFT(image_re, image_re);

	double max, min; // normalize
	cvMinMaxLoc(image_re, &min, &max, NULL, NULL);

	double scale, shift;
	scale = 1.0 / (max - min);
	shift = -min*scale;
	cvCvtScale(image_re, image_re, scale, shift);

	cvNamedWindow("LPF");
	cvShowImage("LPF", image_re);

	cvWaitKey(0);

	cvReleaseImage(&image_re);
	cvReleaseImage(&image_im);
	cvReleaseImage(&srcshift);
	cvReleaseImage(&dst);
	cvReleaseImage(&src);
	cvDestroyWindow("LPF");
	
	return 0;
}
