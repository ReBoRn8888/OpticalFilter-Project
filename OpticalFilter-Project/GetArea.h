#include <algorithm>
#include <math.h>
#include <opencv/cv.h>
#include <io.h>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <ctime>
using namespace std;
using namespace cv;

void ImageSharp(Mat &src, Mat &dst);
int Get6th(vector<vector<Point>> contours, int Area_num);
vector<int> GetArea(Mat, int, vector<Point2f>&, float&);
vector<vector<Mat>> GetRoi(string filepath, string postFix, int num, vector<int>&OutGlassed);
int Otsu(IplImage* src);
vector<string> pathGet(string filepath, string postFix);
vector<vector<double>> getImageSize(string type);

void imageMatting(vector<Mat> &, vector<Mat> &silkprint_list, vector<Mat> &glass_list, vector<vector<Mat>> Six_area, vector<double> image_size, vector<int>);
void imageMatting_outer(vector<Mat> &, vector<Mat> &silkprint_list, vector<Mat> &glass_list, vector<vector<Mat>> Six_area, vector<double> image_size, vector<int>);
void glassDetect(vector<Mat> , int , int, vector<int>);
int silkprintDetect(vector<Mat> silkprint_list, int threshold1, int, vector<Mat> &show_list, vector<int>OutGlassed);
int silkprintDetect2(vector<Mat> silkprint_list, int threshold1, int, vector<Mat> &show_list, vector<int>OutGlassed);
string Int_to_String(int n);
vector<Point2f> sortCenterpoint(vector<Point2f> centers, int middle, vector<int>isGlassed, vector<int>&OutGlassed);
float RotatedDegree(Mat rotaImg);
int getModeNumber(vector<int> list);