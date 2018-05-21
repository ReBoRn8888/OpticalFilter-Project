#ifndef _SECOND_LAYER_H_
#define _SECOND_LAYER_H_
#include <algorithm>
#include <math.h>
#include <io.h>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <ctime>

struct templateGet
{
	double filterType;
	double ratio;
	int elementSize;
	int firstHeight;
	int firstWidth;
	int filterHeight;
	int filterWidth;
	int filterArea;
	int filterLength;
	double silkThresUpoffset;
	double silkThresDownoffset;
	double glassThresOffset;
	double areaUpoffset;
	double areaDownoffset;
	double lengthUpoffset;
	double lengthDownoffset;
};

using namespace cv;
using namespace std;

void ImageSharp(Mat &src, Mat &dst);//Sharp function

int Otsu(IplImage* src);

vector<string> pathGet(string filepath, string postFix);

vector<vector<double>> getImageSize(string type);

float RotatedDegree(Mat rotaImg);

string Int_to_String(int n);

int Get6th(vector<vector<Point>> contours,int,double);//返回第六个数的大小，如果滤光片加底座的数量小于6，那么将最后的数量赋值给Fileternum
vector<int> GetArea(Mat, int, vector<Point2f>&, bool&, int,double);
vector<Point2f>sortCenterpoint(vector<Point2f> centers, int middle, vector<int>isGlassed, vector<int>&OutGlassed);

int glassDetect(Mat&, int, int, templateGet);
int silkprintDetect(Mat, int, int, Mat &, templateGet);

int getAveragePix(Mat, int);
int getModePix(Mat, int);

Mat localyuzhi(Mat src, int blocksize, int yuzhi);

#endif