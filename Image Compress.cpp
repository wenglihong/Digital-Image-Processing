// OpenCV version 2 C++ API functions 
// 
// Split color source image into its color planes. 
// Perform Discrete Cosine Transform on each plane and merge. 
// 
// Then split the merged image into its color planes. 
// Perform inverse DCT on each plane and merge. 
// 
// Reconstituted image should look a lot like the original. 
// (That's kind of the point of this little exercise.) 
// 
// 
// davekw7x 
// 

#include <iostream> 
#include <opencv2/highgui/highgui.hpp> 
using namespace std;
using namespace cv;

#define SHOW_DCT_PLANES 0
int main(int argc, char ** argv)
{
	

	Mat originalImage = imread(".\\image\\lena.bmp",1);

	
	// Split the image into its three planes vector<Mat> planes; 
	Mat planes[3];
	split(originalImage, planes);
	// Convert each plane to a type suitable for cv.dct(), 
	// and do the transform on each one. 
	Mat outplanes[3];

	for (size_t i = 0; i < 3; i++)
	{
		planes[i].convertTo(planes[i], CV_32FC1);
		dct(planes[i], outplanes[i]);
	}
	Mat IDCT;
	idct(outplanes[0], IDCT);
	normalize(IDCT,IDCT,200,0);
	imshow("idct", IDCT);


	// Now put the planes together into a single image 
	Mat merged;
	merge(outplanes,3, merged);
	cout << "Merged DCT image has type = " << merged.type() << endl;
	// 
	// Show what we have so far 
	// namedWindow("Original", CV_WINDOW_AUTOSIZE); 
	int x = 0;
	int y = 0;
	moveWindow("Original", x, y);
	imshow("Original", originalImage);
	// To display the DCT of the individual planes, uncomment the 
	// following #define directive. 
	// #define SHOW_DCT_PLANES 1 
#ifdef SHOW_DCT_PLANES 
	x += 100;
	y += 100;
	namedWindow("DCT Image[0]", CV_WINDOW_AUTOSIZE);
	moveWindow("DCT Image[0]", x, y);
	imshow("DCT Image[0]", outplanes[0]);
	x += 100;
	y += 100;
	namedWindow("DCT Image[1]", CV_WINDOW_AUTOSIZE);
	moveWindow("DCT Image[1]", x, y); imshow("DCT Image[1]", outplanes[1]);
	x += 100; y += 100; 
	namedWindow("DCT Image[2]", CV_WINDOW_AUTOSIZE); 
	moveWindow("DCT Image[2]", x, y); 
	imshow("DCT Image[2]", outplanes[2]);
#endif 
	x += 100; 
	y += 100;
	namedWindow("Merged DCT", CV_WINDOW_AUTOSIZE);
	moveWindow("Merged DCT", x, y);
	imshow("Merged DCT", merged);
	// Start with the merged image and go the other way: 
	// Split into planes and do inverse DCT on each. split(merged, planes);
	for (size_t i = 0; i < 3; i++)
	{
		idct(planes[i], outplanes[i]);
		outplanes[i].convertTo(outplanes[i], CV_8UC1);
	}
	Mat remerged; 
	merge(outplanes, 3,remerged);
	cout << "Reconstituted image type = " << remerged.type() << endl; 
	x += 100; y += 100;
	namedWindow("Reconstituted Image", CV_WINDOW_AUTOSIZE);
	moveWindow("Reconstituted Image", x, y);
	imshow("Reconstituted Image", remerged);
	waitKey(0);
	destroyAllWindows();
}