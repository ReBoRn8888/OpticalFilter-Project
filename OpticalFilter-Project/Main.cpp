#include "AssisFunction.h"
#include "Filter.h"
#include <thread>
#include <windows.h>

using namespace std;
using namespace cv;

DWORD start, stop;

void silkprintThread(Filter thisFilter, int radiusThres_silkprint, int contourAreaThres_silkprint, vector<Mat> silkPrint_show_list,int areaID)//这个线程检测丝印
{
	if (thisFilter.whetherGlassed[areaID] != 0)
	{
		if (silkprintDetect(thisFilter.silkprint_list[areaID], radiusThres_silkprint, contourAreaThres_silkprint, silkPrint_show_list[areaID])!=0)
		{
			stop = GetTickCount();
			cout << "Area:" + Int_to_String(areaID) + " Silk defected\ttime: " << stop - start << " ms\n" << endl;
			imwrite(Int_to_String(areaID) + "silkprint.jpg", silkPrint_show_list[areaID]);
		}
	}
	else
	{
		cout << "Area:" + Int_to_String(areaID) + " don't have filter" << endl;
	}
}

void glassThread(Filter thisFilter, int radiusThres_glass, int contourAreaThres_glass, int areaID)//这个线程检测镜面
{
	if (thisFilter.whetherGlassed[areaID] != 0)
	{
		if (glassDetect(thisFilter.glass_list[areaID], radiusThres_glass, contourAreaThres_glass) != 0)
		{
			stop = GetTickCount();
			cout << "Area:" + Int_to_String(areaID) + " Glass defected\ttime: " << stop - start << " ms" << endl;
			imwrite(Int_to_String(areaID) + "glass.jpg", thisFilter.glass_list[areaID]);
		}
	}
}

int main()
{
	int num = 0;		//当前第几组图片，编号从0开始
	int typeIndex = 0;	//选择图片类型
	vector<string> type = { "Flat_Sprinting", "Flat_NoSprinting", "DoubleCam", "Thread", "New" };	//输入图片的类型
	int item_num = 6;	//一张图片中的滤光片数量
	vector<string> filepath = { "E:\\研究生\\Work\\图片\\扁平\\扁平丝印" ,
								"D:\\Study\\Reborn\\Work\\Image\\Flat\\NoSprinting",
								"D:\\Study\\Reborn\\Work\\Image\\DoubleCam",
								"D:\\Study\\Reborn\\Work\\Image\\Thread",
								"D:\\Study\\Reborn\\Work\\Image\\Flat\\New"
								};	//输入图片的路径
	string postFix = "bmp";	//图片后缀名
	vector<Mat> silkPrint_show_list;
	Filter thisFilter(filepath[typeIndex], postFix, num);
	//thisFilter.GetRoi(filepath[typeIndex], postFix, num);	//粗定位出每个滤光片



	vector<vector<double>> image_sizes = getImageSize(type[typeIndex]);	//获取滤光片大小矩形的长宽

	if (typeIndex == 0 && num == 3 || typeIndex == 0 && num == 6) {		//针对外部透光的图片
		thisFilter.imageMatting2(silkPrint_show_list, image_sizes[num]);//分别提取出丝印和镜面部分
	}
	else {
		thisFilter.imageMatting(silkPrint_show_list, image_sizes[num]);	//分别提取出丝印和镜面部分
	}

	//到这里就把所有我们所需要提取的数据都拿出来了；接下来可以常使用多线程来处理。

	int radiusThres_glass = 10;		//检测到的最小外接圆半径的阈值，若最小外接圆半径大于该阈值，则是缺陷
	int contourAreaThres_glass = 30;	//检测到的轮廓的面积的阈值，若轮廓面积大于该阈值，则是缺陷

	int radiusThres_silkprint = 10;	//检测到的最小外接圆半径的阈值，若最小外接圆半径大于该阈值，则是缺陷
	int contourAreaThres_silkprint = 80;	//检测到的轮廓的面积的阈值，若轮廓面积大于该阈值，则是缺陷

	start = GetTickCount();
	for (int areaID = 0; areaID < item_num; areaID++)	//线程并行执行
	{
		thread OuterSilk(silkprintThread, thisFilter, radiusThres_silkprint, contourAreaThres_silkprint, silkPrint_show_list, areaID);	//丝印检测线程
		thread InnerGlass(glassThread, thisFilter, radiusThres_glass, contourAreaThres_glass, areaID);       //镜面检测线程

		OuterSilk.detach(); 
		InnerGlass.detach(); 
	}
	waitKey();

	getchar();	//为了让命令行窗口在执行完后不马上关闭
}