#include "AssisFunction.h"
using namespace std;
using namespace cv;

//镜面检测部分
int glassDetect(Mat& glass, int radiusThres, int contourAreaThres)
{
	//通过形态学闭操作提取镜面边缘，二值化后取反，与原图相加，使与边缘相连的缺陷独立出来
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Mat tt = glass.clone();	//预处理
	Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
	morphologyEx(tt, tt, MORPH_CLOSE, element);
	threshold(tt, tt, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	bitwise_not(tt, tt);
	element = getStructuringElement(MORPH_RECT, Size(9, 9));
	dilate(tt, tt, element);
	Mat addResult = tt + glass;
	//二值化后进行边缘检测并提取轮廓，通过轮廓的属性（最小外接圆的半径、轮廓面积）来判断缺陷
	GaussianBlur(addResult, addResult, Size(5, 5), 1);   //高斯滤波
	element = getStructuringElement(MORPH_RECT, Size(3, 3));
	erode(addResult, addResult, element);
	IplImage ipl = (IplImage)addResult;
	int th = Otsu(&ipl);
	threshold(addResult, addResult, th * 32 / 43, 255, CV_THRESH_BINARY);
	rectangle(addResult, Rect(Point(0, 0), Point(addResult.cols - 1, addResult.rows - 1)), Scalar(255), 1);
	element = getStructuringElement(MORPH_RECT, Size(3, 3));
	morphologyEx(addResult, addResult, MORPH_CLOSE, element);
	findContours(addResult, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	//遍历轮廓，判断缺陷
	int defect_flag = 0;	//缺陷标志位，0--无缺陷，1--有缺陷
	for (int index = 0; index < contours.size(); index++) {
		Point2f center;
		float radius;
		minEnclosingCircle(contours[index], center, radius);	//计算轮廓的最小外接圆
		int area = contourArea(contours[index]);	//计算轮廓面积
		int scale = 2;
		if (center.x > scale && center.x < glass.cols - scale && center.y > scale && center.y < glass.rows - scale && area < (glass.cols * glass.rows / 2) && radius < glass.cols * 2 / 5) {	//过滤掉太边缘的轮廓以及太大的轮廓
			if (radius >= radiusThres || area >= contourAreaThres) {	//若最小外接圆的半径大于"radiusThres",或轮廓面积大于"contourAreaThres"，则判断为缺陷
				circle(glass, center, radius, Scalar(255), 2, 8);
				drawContours(glass, contours, index, Scalar(255), 2, 8);
				defect_flag = 1;
			}
		}
	}

	return defect_flag;
}

//检测丝印的部分
int silkprintDetect(Mat silkprint, int radiusThres, int contourAreaThres, Mat &show_list)
{
	vector<int>areas; int area_temp;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Mat silkprint_temp = silkprint.clone();
	rectangle(silkprint_temp, Rect(Point(0, 0), Point(silkprint_temp.cols, silkprint_temp.rows)), Scalar(0), 3);
	Mat tt = Mat::zeros(silkprint_temp.rows, silkprint_temp.cols, silkprint_temp.type());
	findContours(silkprint_temp, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	int defect_flag = 0;	//缺陷标志位，0--无缺陷，1--有缺陷
	for (int index = 0; index < contours.size(); index++)
	{
		Point2f center;
		float radius;
		minEnclosingCircle(contours[index], center, radius);	//计算轮廓的最小外接圆
		int area = contourArea(contours[index]);	//计算轮廓面积
		int scale = 5;
		if (center.x > scale && center.x < show_list.cols - scale && center.y > scale && center.y < show_list.rows - scale && area < show_list.cols * show_list.rows / 2 && radius < show_list.cols * 2 / 5) {	//过滤掉太边缘的轮廓以及太大的轮廓
			if (radius >= radiusThres || area >= contourAreaThres) {	//若最小外接圆的半径大于"radiusThres",或轮廓面积大于"contourAreaThres"，则判断为缺陷
				circle(show_list, center, radius, Scalar(255), 2, 8);
				drawContours(show_list, contours, index, Scalar(255), 2, 8);
				defect_flag = 1;
			}
		}
	}

	return defect_flag;
}