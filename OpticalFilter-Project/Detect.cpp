#include "AssisFunction.h"
using namespace std;
using namespace cv;

//�����ⲿ��
int glassDetect(Mat& glass, int radiusThres, int contourAreaThres)
{
	//ͨ����̬ѧ�ղ�����ȡ�����Ե����ֵ����ȡ������ԭͼ��ӣ�ʹ���Ե������ȱ�ݶ�������
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Mat tt = glass.clone();	//Ԥ����
	Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
	morphologyEx(tt, tt, MORPH_CLOSE, element);
	threshold(tt, tt, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	bitwise_not(tt, tt);
	element = getStructuringElement(MORPH_RECT, Size(9, 9));
	dilate(tt, tt, element);
	Mat addResult = tt + glass;
	//��ֵ������б�Ե��Ⲣ��ȡ������ͨ�����������ԣ���С���Բ�İ뾶��������������ж�ȱ��
	GaussianBlur(addResult, addResult, Size(5, 5), 1);   //��˹�˲�
	element = getStructuringElement(MORPH_RECT, Size(3, 3));
	erode(addResult, addResult, element);
	IplImage ipl = (IplImage)addResult;
	int th = Otsu(&ipl);
	threshold(addResult, addResult, th * 32 / 43, 255, CV_THRESH_BINARY);
	rectangle(addResult, Rect(Point(0, 0), Point(addResult.cols - 1, addResult.rows - 1)), Scalar(255), 1);
	element = getStructuringElement(MORPH_RECT, Size(3, 3));
	morphologyEx(addResult, addResult, MORPH_CLOSE, element);
	findContours(addResult, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	//�����������ж�ȱ��
	int defect_flag = 0;	//ȱ�ݱ�־λ��0--��ȱ�ݣ�1--��ȱ��
	for (int index = 0; index < contours.size(); index++) {
		Point2f center;
		float radius;
		minEnclosingCircle(contours[index], center, radius);	//������������С���Բ
		int area = contourArea(contours[index]);	//�����������
		int scale = 2;
		if (center.x > scale && center.x < glass.cols - scale && center.y > scale && center.y < glass.rows - scale && area < (glass.cols * glass.rows / 2) && radius < glass.cols * 2 / 5) {	//���˵�̫��Ե�������Լ�̫�������
			if (radius >= radiusThres || area >= contourAreaThres) {	//����С���Բ�İ뾶����"radiusThres",�������������"contourAreaThres"�����ж�Ϊȱ��
				circle(glass, center, radius, Scalar(255), 2, 8);
				drawContours(glass, contours, index, Scalar(255), 2, 8);
				defect_flag = 1;
			}
		}
	}

	return defect_flag;
}

//���˿ӡ�Ĳ���
int silkprintDetect(Mat silkprint, int radiusThres, int contourAreaThres, Mat &show_list)
{
	vector<int>areas; int area_temp;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Mat silkprint_temp = silkprint.clone();
	rectangle(silkprint_temp, Rect(Point(0, 0), Point(silkprint_temp.cols, silkprint_temp.rows)), Scalar(0), 3);
	Mat tt = Mat::zeros(silkprint_temp.rows, silkprint_temp.cols, silkprint_temp.type());
	findContours(silkprint_temp, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
	int defect_flag = 0;	//ȱ�ݱ�־λ��0--��ȱ�ݣ�1--��ȱ��
	for (int index = 0; index < contours.size(); index++)
	{
		Point2f center;
		float radius;
		minEnclosingCircle(contours[index], center, radius);	//������������С���Բ
		int area = contourArea(contours[index]);	//�����������
		int scale = 5;
		if (center.x > scale && center.x < show_list.cols - scale && center.y > scale && center.y < show_list.rows - scale && area < show_list.cols * show_list.rows / 2 && radius < show_list.cols * 2 / 5) {	//���˵�̫��Ե�������Լ�̫�������
			if (radius >= radiusThres || area >= contourAreaThres) {	//����С���Բ�İ뾶����"radiusThres",�������������"contourAreaThres"�����ж�Ϊȱ��
				circle(show_list, center, radius, Scalar(255), 2, 8);
				drawContours(show_list, contours, index, Scalar(255), 2, 8);
				defect_flag = 1;
			}
		}
	}

	return defect_flag;
}