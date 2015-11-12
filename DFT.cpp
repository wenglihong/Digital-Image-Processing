#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

Mat CDM(int M, int N);
Mat ILPF(Mat matD, float D0);
void RearRange(Mat &src);

int main(int argc, char ** argv)
{
	Mat I = imread("lena.bmp", CV_LOAD_IMAGE_GRAYSCALE);
	if (I.empty())
		return -1;
	//resize(I, I, Size(32,32));

	Mat padded;                            //expand input image to optimal size
	int m = getOptimalDFTSize(I.rows);
	int n = getOptimalDFTSize(I.cols); // on the border add zero values
	copyMakeBorder(I, padded, 0, m - I.rows, 0, n - I.cols, BORDER_CONSTANT, Scalar::all(0));

	ofstream out_inf;
	out_inf.open("I.xls");
	out_inf << I;
	out_inf.close();

	Mat planes[] = { Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F) };
	Mat complexI;
	merge(planes, 2, complexI);         // Add to the expanded another plane with zeros

	dft(complexI, complexI);            // this way the result may fit in the source matrix

	// compute the magnitude and switch to logarithmic scale
	// => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
	split(complexI, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
	Mat Re = planes[0];
	Mat Im = planes[1];
	//magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude  
	//Mat magI = planes[0];

	//magI += Scalar::all(1);                    // switch to logarithmic scale
	//log(magI, magI);

	//// crop the spectrum, if it has an odd number of rows or columns
	//magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));

	//// rearrange the quadrants of Fourier image  so that the origin is at the image center        
	//int cx = magI.cols / 2;
	//int cy = magI.rows / 2;

	//Mat q0(magI, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant 
	//Mat q1(magI, Rect(cx, 0, cx, cy));  // Top-Right
	//Mat q2(magI, Rect(0, cy, cx, cy));  // Bottom-Left
	//Mat q3(magI, Rect(cx, cy, cx, cy)); // Bottom-Right

	//Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
	//q0.copyTo(tmp);
	//q3.copyTo(q0);
	//tmp.copyTo(q3);

	//q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
	//q2.copyTo(q1);
	//tmp.copyTo(q2);

	//normalize(magI, magI, 0, 1, CV_MINMAX); // Transform the matrix with float values into a 
	// viewable image form (float between values 0 and 1).

	imshow("Input Image", I);    // Show the result
	//imshow("spectrum magnitude", magI);
	waitKey();

	Mat D = CDM(Re.rows, Re.cols);
	Mat H = ILPF(D,1000);
	Mat G, G_Re, G_Im;

	mulSpectrums(Re, H, G_Re, CV_DXT_ROWS);
	mulSpectrums(Im, H, G_Im, CV_DXT_ROWS);
	Mat temp[] = { G_Re, G_Im };
	merge(temp, 2, G);
	Mat dst;
	idft(G, dst);

	split(dst, temp);
	//G_Re.setTo(0);
	//G_Im.setTo(0);
	G_Re = temp[0];
	G_Im = temp[1];
	//magnitude(temp[0], temp[1], temp[0]);
	//Mat S = temp[0];


	//Mat p0(G_Re, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant 
	//Mat p1(G_Re, Rect(cx, 0, cx, cy));  // Top-Right
	//Mat p2(G_Re, Rect(0, cy, cx, cy));  // Bottom-Left
	//Mat p3(G_Re, Rect(cx, cy, cx, cy)); // Bottom-Right

	//Mat t;                           // swap quadrants (Top-Left with Bottom-Right)
	//p0.copyTo(t);
	//p3.copyTo(p0);
	//t.copyTo(p3);

	//p1.copyTo(t);                    // swap quadrant (Top-Right with Bottom-Left)
	//p2.copyTo(p1);
	//t.copyTo(p2);

	normalize(G_Re, G_Re, 0, 1, CV_MINMAX); // Transform the matrix with float values into a 
	imshow("S", G_Re);
	//imshow("magI", magI);
	//imshow("F", F);
	//cout << magI;
	//cout << endl;
	//cout << F;

	/*cout << D;
	cout << endl;
	cout << H;*/
	waitKey();
	return 0;
}


////////////////////////compute the meshgrid arrays needed for LPF//////////////////////////////////////////////////
// CDM compute meshgrid frequency matrices (ok!)
// see Gonzalez Digital image processing using matlab page93 function dftuv
Mat CDM(int M, int N)
{
	/*int width = matD.rows;
	int height = matD.cols;*/
	//if (M != width && N != height)
	//{
	//	cout << "ERROR! THE SIZE DOES NOT MATCH WITH MAT" << endl;
	//	return;
	//}

	//if (cvGetElemType(matD) < CV_32F)
	//{
	//	cout << "ERROR! THE TYPE DOES NOT MATCH WITH MAT" << endl;
	//	return;
	//}

	Mat matD(M, N, CV_32FC1);
	Mat U(M, N, CV_32FC1);
	Mat V(M, N, CV_32FC1);

	/*CvMat *U, *V;
	U = cvCreateMat(M, N, CV_32FC1);
	V = cvCreateMat(M, N, CV_32FC1);*/

	for (int u = 0; u < M; ++u)
	for (int v = 0; v < N; ++v)
	{
		float tm1, tm2;
		tm1 = (float)((u > cvRound(M / 2)) ? u - M : u);
		tm2 = (float)((v > cvRound(N / 2)) ? v - N : v);

		U.at<float>(u, v) = tm1;
		V.at<float>(u, v) = tm2;

	/*	*((float *)CV_MAT_ELEM_PTR(*U, u, v)) = tm1;

		*((float *)CV_MAT_ELEM_PTR(*V, u, v)) = tm2;*/
	}

	for (int u = 0; u < M; ++u)
	for (int v = 0; v < N; ++v)
	{
		float t1, t2;
		t1 = U.at<float>(u, v);
		t2 = V.at<float>(u, v);
		matD.at<float>(u, v) = sqrt(t1*t1 + t2*t2);
		/*t1 = CV_MAT_ELEM(*U, float, u, v);
		t2 = CV_MAT_ELEM(*V, float, u, v);
		*((float *)CV_MAT_ELEM_PTR(*matD, u, v)) = sqrt(t1*t1 + t2*t2);*/
	}
	return matD;
}

////////////////////////////create low pass filter//////////////////////////////////////////////
// for ideal and gaussian low pass filter
Mat ILPF( Mat matD, float D0)
{
	/*if (D0 < 0)
	{
		cout << "ERROR! D0 MUST BE POSITIVE" << endl;
		return ;
	}*/

	int w = matD.rows;
	int h = matD.cols;
	Mat matH(w, h, CV_32FC1);

	for (int u = 0; u < w; ++u)
	for (int v = 0; v < h; ++v)
	{
		float Elem_D;
		Elem_D = matD.at<float>(u, v);
		//Elem_D = CV_MAT_ELEM(*matD, float, u, v);
		if (Elem_D <= D0)
			matH.at<float>(u, v) = 1.0;
		/*	*((float *)CV_MAT_ELEM_PTR(*matH, u, v)) = 1.0;*/
		else
			matH.at<float>(u, v) = 0.0;
		/*	*((float *)CV_MAT_ELEM_PTR(*matH, u, v)) = 0.0;*/

	}	
	return matH;

}


void RearRange(Mat &src)
{
	// rearrange the quadrants of Fourier image  so that the origin is at the image center        
	int cx = src.cols / 2;
	int cy = src.rows / 2;

	Mat q0(src, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant 
	Mat q1(src, Rect(cx, 0, cx, cy));  // Top-Right
	Mat q2(src, Rect(0, cy, cx, cy));  // Bottom-Left
	Mat q3(src, Rect(cx, cy, cx, cy)); // Bottom-Right

	Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
	q2.copyTo(q1);
	tmp.copyTo(q2);
}
