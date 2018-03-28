#include "GetArea.h"


using namespace cv;
using namespace std;


int main()
{
	int num = 6;		//当前第几组图片，编号从0开始
	int typeIndex = 0;	//选择图片类型
	vector<string> type = { "Flat_Sprinting","Flat_NoSprinting", "DoubleCam", "Thread", "New" };	//输入图片的类型
	int item_num = 6;	//一张图片中的滤光片数量
	vector<string> filepath = { "D:\\Study\\Reborn\\Work\\Image\\Flat\\Sprinting" ,
								"D:\\Study\\Reborn\\Work\\Image\\Flat\\NoSprinting",
								"D:\\Study\\Reborn\\Work\\Image\\DoubleCam",
								"D:\\Study\\Reborn\\Work\\Image\\Thread",
								"D:\\Study\\Reborn\\Work\\Image\\Flat\\New"
								};	//输入图片的路径
	string postFix = "bmp";	//图片后缀名
	vector<vector<Mat>> Six_area(item_num);	//用于存放6个滤光片位置
	vector<Mat> silkPrint_show_list;
	vector<int> isGlassed;					//判断对应位置是否有滤光片
	vector<Mat> silkprint_list; vector<Mat> glass_list;		//存放提取出的丝印和镜面部分

	Six_area = GetRoi(filepath[typeIndex], postFix, num, isGlassed);	//粗定位出每个滤光片
	
	vector<vector<double>> image_sizes = getImageSize(type[typeIndex]);	//获取滤光片大小矩形的长宽

	if (typeIndex == 0 && num == 3 || typeIndex == 0 && num == 6) {
		imageMatting_outer(silkPrint_show_list, silkprint_list, glass_list, Six_area, image_sizes[num], isGlassed);	//分别提取出丝印和镜面部分(针对丝印外部还有镜面的情况)
	}
	else {
		imageMatting(silkPrint_show_list, silkprint_list, glass_list, Six_area, image_sizes[num], isGlassed);	//分别提取出丝印和镜面部分
	}

	//for (int i = 0; i < silkprint_list.size(); i++)
	//{
	//	//namedWindow("silkprint: " + Int_to_String(i + 1) + ".jpg", 0);
	//	//namedWindow("glass: " + Int_to_String(i + 1) + ".jpg", 0);
	//	imshow("silkprint: " + Int_to_String(i) + ".jpg", silkprint_list[i]);
	//	imshow("glass: " + Int_to_String(i) + ".jpg", glass_list[i]);
	//	//imshow(Int_to_String(i + 1) + ".jpg", Six_area[1][i]);
	//}

	int radiusThres_glass = 10;		//检测到的最小外接圆半径的阈值，若最小外接圆半径大于该阈值，则是缺陷
	int contourAreaThres_glass = 30;	//检测到的轮廓的面积的阈值，若轮廓面积大于该阈值，则是缺陷
	glassDetect(glass_list, radiusThres_glass, contourAreaThres_glass, isGlassed);	//滤光片镜面检测

	int radiusThres_silkprint = 2;	//检测到的最小外接圆半径的阈值，若最小外接圆半径大于该阈值，则是缺陷
	int contourAreaThres_silkprint = 15;	//检测到的轮廓的面积的阈值，若轮廓面积大于该阈值，则是缺陷
	//silkprintDetect(silkprint_list, radiusThres_silkprint, contourAreaThres_silkprint, silkPrint_show_list, isGlassed);	//滤光片丝印检测
	//silkprintDetect2(silkprint_list, radiusThres_silkprint, contourAreaThres_silkprint, silkPrint_show_list, isGlassed);	//滤光片丝印检测

	waitKey();
}