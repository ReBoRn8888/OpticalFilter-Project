#include "AssisFunction.h"
#include "Filter.h"
#include <thread>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

struct outResults
{
	vector<SinglefilterMessage> allFilter;
	vector<SinglefilterMessage> ngFileter;
};

SinglefilterMessage SingleResult[10];
templateGet FilterParameter;
outResults algorithmOutput;
void detectThread(Filter thisFilter, int radiusThres_silkprint, int contourAreaThres_silkprint, int radiusThres_glass, int contourAreaThres_glass, vector<Mat> silkPrint_show_list, 
				templateGet FilterParameter, int areaID, double xposParameter, double yposParameter){//滤光片检测线程
	int flag = 1;
	SingleResult[areaID].label = thisFilter.glassLabel[areaID];
	SingleResult[areaID].xpos = xposParameter;
	SingleResult[areaID].ypos = yposParameter;

	thisFilter.Area1[areaID].copyTo(SingleResult[areaID].src[0]);
	thisFilter.Area2[areaID].copyTo(SingleResult[areaID].src[1]);
	thisFilter.Area3[areaID].copyTo(SingleResult[areaID].src[2]);
	if (thisFilter.whetherGlassed[areaID] == -1){
		cout << "Area:" + Int_to_String(thisFilter.glassLabel[areaID]) + " have ng Filter" << endl;
		SingleResult[areaID].flag = -1;
	}
	else if (thisFilter.whetherGlassed[areaID] == 1){
		SingleResult[areaID].type = FilterParameter.filterType;
		int silkprint_flag = silkprintDetect(thisFilter.silkprint_list[areaID], radiusThres_silkprint, contourAreaThres_silkprint, silkPrint_show_list[areaID], FilterParameter);
		if (silkprint_flag != 0){
			cout << "Area:" + Int_to_String(thisFilter.glassLabel[areaID]) + " Silk defected" << endl;
			imwrite(Int_to_String(thisFilter.glassLabel[areaID]) + "silkprint.jpg", silkPrint_show_list[areaID]);
		}
		int glass_flag = glassDetect(thisFilter.glass_list[areaID], radiusThres_glass, contourAreaThres_glass, FilterParameter);
		if (glass_flag != 0){
			cout << "Area:" + Int_to_String(thisFilter.glassLabel[areaID]) + " Glass defected" << endl;
			imwrite(Int_to_String(thisFilter.glassLabel[areaID]) + "glass.jpg", thisFilter.glass_list[areaID]);
		}
		if (silkprint_flag || glass_flag)
			SingleResult[areaID].flag = 1;
	}
	else
		cout << "Area:" + Int_to_String(thisFilter.glassLabel[areaID]) + " don't have filter" << endl;
}

int main()
{
	int num = 0;		//当前第几组图片，编号从0开始
	vector<int> filterArea = { 210000, 320000, 215000, 225000, 270000, 145000, 250000, 360000 };//预置的滤光片面积，之后手动输入参数
	vector<int> filterHeight = { 0, 0, 0, 0, 590, 0, 719, 700 };//预置的滤光片高度，之后手动输入参数
	vector<int> filterWidth = { 0, 0, 0, 0, 730, 0, 645, 700 };//预置的滤光片宽度，之后手动输入参数
	vector<int> filterType = { 0, 0, 0, 1, 0, 0, 1, 2 };//预置的滤光片类型，之后手动输入参数
	int item_num = 6;	//一张图片中的滤光片数量
	string filepath = "E:\\研究生\\Work\\滤光片缺陷检测\\Image";	//输入图片的路径
	string postFix = "bmp";	//图片后缀名

	double xposParameter = 1.0;
	double yposParameter = 1.0;

	//下面是模板获取的参数的初始化
	FilterParameter.ratio = 1.3;					//滤光片长宽比（高度/宽度）
	FilterParameter.elementSize = 7;					//膨胀和腐蚀的尺寸
	FilterParameter.filterType = 0;		//滤光片种类
	FilterParameter.firstHeight = 770;					//粗定位的高度
	FilterParameter.firstWidth = 770;					//粗定位的宽度
	FilterParameter.filterHeight = 694;	               //滤光片的高度
	FilterParameter.filterWidth = 556;		           //滤光片的宽度
	FilterParameter.filterArea = 215000;		       //滤光片面积
	FilterParameter.filterLength = 2177;				//滤光片周长
	FilterParameter.areaUpoffset = 2000;					//面积差上限偏移量
	FilterParameter.areaDownoffset = 2000;				//面积差下限偏移量
	FilterParameter.lengthUpoffset = 500;				//周长差上限偏移量
	FilterParameter.lengthDownoffset = 500;				//周长差下限偏移量
	FilterParameter.glassThresOffset = 15;			//检测时透光区二值化阈值的偏移量
	FilterParameter.silkThresUpoffset = 20;				//检测时丝印二值化阈值的下限偏移量
	FilterParameter.silkThresDownoffset = 20;			//检测时丝印二值化阈值的上限偏移量

	Filter thisFilter(filepath, postFix, num, FilterParameter);

	if (thisFilter.whetherNull == false && thisFilter.tested_path.size() != 0){
		vector<Mat> silkPrint_show_list;
		if (FilterParameter.filterType == 0)//针对普通的图片
			thisFilter.imageMatting(silkPrint_show_list, FilterParameter);//分别提取出丝印和镜面部分 
		else if (FilterParameter.filterType == 1)//针对外部还有透光区的图片
			thisFilter.imageMatting2(silkPrint_show_list, FilterParameter);//分别提取出丝印和镜面部分
		else if (FilterParameter.filterType == 2)//针对透光区为圆形的图片
			thisFilter.imageMatting3(silkPrint_show_list, FilterParameter);//分别提取出丝印和镜面部分

		//到这里就把所有我们所需要提取的数据都拿出来了；接下来可以常使用多线程来处理。
		int radiusThres_glass = 5;		//检测到的最小外接圆半径的阈值，若最小外接圆半径大于该阈值，则是缺陷
		int contourAreaThres_glass = 7;	//检测到的轮廓的面积的阈值，若轮廓面积大于该阈值，则是缺陷
		int radiusThres_silkprint = 5;	//检测到的最小外接圆半径的阈值，若最小外接圆半径大于该阈值，则是缺陷
		int contourAreaThres_silkprint = 7;	//检测到的轮廓的面积的阈值，若轮廓面积大于该阈值，则是缺陷

		int areaID = 0;
		for (areaID = 0; areaID < thisFilter.whetherGlassed.size(); areaID++)	//线程并行执行
		{
			thread t(detectThread, thisFilter, radiusThres_silkprint, contourAreaThres_silkprint, radiusThres_glass, contourAreaThres_glass,
				silkPrint_show_list, FilterParameter, areaID, xposParameter, yposParameter);
			t.join();
		}

		//将检测结果整理到结构体
		for (int i = 0; i < thisFilter.glass_list.size(); i++){
			if (SingleResult[i].flag == 1){
				cout << "<" << SingleResult[i].label << ">号滤光片有缺陷" << endl;
				algorithmOutput.ngFileter.push_back(SingleResult[i]);
			}
			else if (SingleResult[i].flag == -1)
				cout << "<" << SingleResult[i].label << ">号位置滤光片有误" << endl;
			algorithmOutput.allFilter.push_back(SingleResult[i]);
		}
	}
	else if (thisFilter.tested_path.size() == 0)
		cout << "图片路径有误" << endl;
	else
		cout << "未检测到滤光片" << endl;

	waitKey();
	getchar();	//为了让命令行窗口在执行完后不马上关闭
}