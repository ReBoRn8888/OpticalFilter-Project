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

using namespace cv;
using namespace std;

void ImageSharp(Mat &src, Mat &dst);//Sharp function

int Otsu(IplImage* src);

vector<string> pathGet(string filepath, string postFix);

vector<vector<double>> getImageSize(string type);

float RotatedDegree(Mat rotaImg);

string Int_to_String(int n);

int Get6th(vector<vector<Point>> contours, int& Filternum,int& lacks);//返回第六个数的大小，如果滤光片加底座的数量小于6，那么将最后的数量赋值给Fileternum
vector<int> GetArea(Mat, int, vector<Point2f>&, float&, bool&, int);
vector<Point2f>sortCenterpoint(vector<Point2f> centers, int middle, vector<int>isGlassed, vector<int>&OutGlassed);

int glassDetect(Mat& glass, int radiusThres, int contourAreaThres);
int silkprintDetect(Mat silkprint, int radiusThres, int contourAreaThres, Mat &show_list);

int getModeNumber(vector<int>);

vector<string> split(const string &, const string &);

void getConfigInfo(vector<double>&, double&);



#endif