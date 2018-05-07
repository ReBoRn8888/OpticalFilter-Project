#include "AssisFunction.h"
using namespace std;
using namespace cv;

//�����ⲿ��
int glassDetect(Mat& glass, int radiusThres, int contourAreaThres, templateGet FilterParameter)
{
	int defect_flag = 0;	//ȱ�ݱ�־λ��0--��ȱ�ݣ�1--��ȱ��
	//��ȡ������������ͨ��������ܳ��ж�ȱ�����
	Mat tt;
	//imwrite("test.jpg", glass);
	threshold(glass, tt, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	//imwrite("test2.jpg", tt);
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(tt, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);
	//ɾȥС������ֻ������������
	vector<vector<Point>>::iterator it_contour = contours.begin();
	while (it_contour != contours.end()){
		if (contourArea(*it_contour) < FilterParameter.filterArea * 0.8)
			it_contour = contours.erase(it_contour);
		else
			++it_contour;
	}
	int a = contourArea(contours[0]);
	int l = contours[0].size();
	if (a < FilterParameter.filterArea - FilterParameter.areaDownoffset || a > FilterParameter.filterArea + FilterParameter.areaUpoffset || 
		l < FilterParameter.filterLength - FilterParameter.lengthDownoffset || l > FilterParameter.filterLength + FilterParameter.lengthUpoffset)
		defect_flag = 1;
	//�������ؾ�ֵ+offset���� �� ���ؾ�ֵ-offset���£���ͼ����ж�ֵ��
	Mat thresDown, thresUp, thres;
	threshold(glass, thresDown, getAveragePix(glass, 0) - FilterParameter.glassThresDownoffset, 255, CV_THRESH_BINARY_INV);
	threshold(glass, thresUp, getAveragePix(glass, 0) + FilterParameter.glassThresUpoffset, 255, CV_THRESH_BINARY);
	thres = thresDown + thresUp;
	//imwrite("thres1.jpg", thresDown);
	//imwrite("thres2.jpg", thresUp);
	findContours(thres, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	//ɾȥ�����˿ӡ������
	it_contour = contours.begin();
	while (it_contour != contours.end()){
		if (contourArea(*it_contour) > FilterParameter.filterArea * 0.8)
			it_contour = contours.erase(it_contour);
		else
			++it_contour;
	}
	//�����������ж�ȱ��
	for (int index = 0; index < contours.size(); index++) {
		Point2f center;
		float radius;
		minEnclosingCircle(contours[index], center, radius);	//������������С���Բ
		int area = contourArea(contours[index]);	//�����������
		int scale = 3;
		int todo_flag = 0;
		if (center.x > scale && center.x < glass.cols - scale && center.y > scale && center.y < glass.rows - scale && area < (glass.cols * glass.rows / 2) && radius < glass.cols * 2 / 5)
			todo_flag = 1;	//���˵�̫��Ե�������Լ�̫�������
		if (todo_flag) {
			if (radius >= radiusThres || area >= contourAreaThres) {	//����С���Բ�İ뾶����"radiusThres",�������������"contourAreaThres"�����ж�Ϊȱ��
				drawContours(glass, contours, index, Scalar(255), 2, 8);
				defect_flag = 1;
			}
		}
	}

	return defect_flag;
}

//���˿ӡ�Ĳ���
int silkprintDetect(Mat silkprint, int radiusThres, int contourAreaThres, Mat &show_list, templateGet FilterParameter)
{
	int defect_flag = 0;	//ȱ�ݱ�־λ��0--��ȱ�ݣ�1--��ȱ��
	//��ȡ��˿ӡ����������
	Mat tt;
	threshold(silkprint, tt, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(tt, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);
	//ɾȥС������ֻ������������
	vector<vector<Point>>::iterator it_contour = contours.begin();
	it_contour = contours.begin();
	while (it_contour != contours.end()){
		if (contourArea(*it_contour) < FilterParameter.filterArea * 0.8)
			it_contour = contours.erase(it_contour);
		else
			++it_contour;
	}
	//�������ؾ�ֵ+offset���� �� ���ؾ�ֵ-offset���£���ͼ����ж�ֵ��
	Mat thresDown, thresUp, thres;
	threshold(silkprint, thresDown, getAveragePix(silkprint, 0) - FilterParameter.silkThresDownoffset, 255, CV_THRESH_BINARY_INV);
	threshold(silkprint, thresUp, getAveragePix(silkprint, 0) + FilterParameter.silkThresUpoffset, 255, CV_THRESH_BINARY);
	thres = thresDown + thresUp;
	drawContours(thres, contours, -1, Scalar(0), 3);
	findContours(thres, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	//ɾȥ�����˿ӡ������
	it_contour = contours.begin();
	while (it_contour != contours.end()){
		if (contourArea(*it_contour) > FilterParameter.filterArea * 0.8)
			it_contour = contours.erase(it_contour);
		else
			++it_contour;
	}
	for (int index = 0; index < contours.size(); index++)
	{
		Point2f center;
		float radius;
		minEnclosingCircle(contours[index], center, radius);	//������������С���Բ
		int area = contourArea(contours[index]);	//�����������
		int scale = 5;
		int todo_flag = 0;
		if (center.x > scale && center.x < show_list.cols - scale && center.y > scale && center.y < show_list.rows - scale && area < show_list.cols * show_list.rows / 2 && radius < show_list.cols * 2 / 5)
			todo_flag = 1;	//���˵�̫��Ե�������Լ�̫�������
		if (todo_flag) {
			if (radius >= radiusThres || area >= contourAreaThres) {	//����С���Բ�İ뾶����"radiusThres",�������������"contourAreaThres"�����ж�Ϊȱ��
				drawContours(show_list, contours, index, Scalar(255), 2, 8);
				defect_flag = 1;
			}
		}
	}

	return defect_flag;
}