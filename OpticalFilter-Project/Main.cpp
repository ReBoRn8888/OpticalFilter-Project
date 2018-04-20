#include "AssisFunction.h"
#include "Filter.h"
#include <thread>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

SinglefilterMessage SingleResult[6];

void detectThread(Filter thisFilter, int radiusThres_silkprint, int contourAreaThres_silkprint, int radiusThres_glass, int contourAreaThres_glass, vector<Mat> silkPrint_show_list, int areaID)//滤光片检测线程
{
	int flag = 1;
	SingleResult[areaID].label = areaID;
	if (thisFilter.whetherGlassed[areaID] != 0){
		int silkprint_flag = silkprintDetect(thisFilter.silkprint_list[areaID], radiusThres_silkprint, contourAreaThres_silkprint, silkPrint_show_list[areaID]);
		if (silkprint_flag != 0){
			cout << "Area:" + Int_to_String(areaID) + " Silk defected" << endl;
			imwrite(Int_to_String(areaID) + "silkprint.jpg", silkPrint_show_list[areaID]);
		}
		int glass_flag = glassDetect(thisFilter.glass_list[areaID], radiusThres_glass, contourAreaThres_glass);
		if (glass_flag != 0){
			cout << "Area:" + Int_to_String(areaID) + " Glass defected" << endl;
			imwrite(Int_to_String(areaID) + "glass.jpg", thisFilter.glass_list[areaID]);
		}
		if (silkprint_flag || glass_flag)
			SingleResult[areaID].flag = 1;
	}
	else{
		cout << "Area:" + Int_to_String(areaID) + " don't have filter" << endl;
	}
}

int main()
{
	int num = 1;		//当前第几组图片，编号从0开始
	int typeIndex = 0;	//选择图片类型
	//vector<string> type = { "Flat_Sprinting", "Flat_NoSprinting", "DoubleCam", "Thread", "New" };	//输入图片的类型
	int item_num = 6;	//一张图片中的滤光片数量
	vector<string> filepath = { "E:\\研究生\\Work\\滤光片缺陷检测\\All",
								"D:\\Study\\Reborn\\Work\\Image\\Flat\\NoSprinting",
								"D:\\Study\\Reborn\\Work\\Image\\DoubleCam",
								"D:\\Study\\Reborn\\Work\\Image\\Thread",
								"D:\\Study\\Reborn\\Work\\Image\\Flat\\New"
								};	//输入图片的路径
	string postFix = "bmp";	//图片后缀名
	int pedestalArea = 500000;
	Filter thisFilter(filepath[typeIndex], postFix, num, pedestalArea);
	if (thisFilter.whetherNull == false){
		vector<double> image_sizes;
		double type;
		getConfigInfo(image_sizes, type);
		vector<Mat> silkPrint_show_list;
		if (type == 0)		//针对外部透光的图片
			thisFilter.imageMatting(silkPrint_show_list, image_sizes);//分别提取出丝印和镜面部分 
		else if (type == 1)
			thisFilter.imageMatting2(silkPrint_show_list, image_sizes);	//分别提取出丝印和镜面部分

		//到这里就把所有我们所需要提取的数据都拿出来了；接下来可以常使用多线程来处理。
		int radiusThres_glass = 10;		//检测到的最小外接圆半径的阈值，若最小外接圆半径大于该阈值，则是缺陷
		int contourAreaThres_glass = 30;	//检测到的轮廓的面积的阈值，若轮廓面积大于该阈值，则是缺陷
		int radiusThres_silkprint = 10;	//检测到的最小外接圆半径的阈值，若最小外接圆半径大于该阈值，则是缺陷
		int contourAreaThres_silkprint = 80;	//检测到的轮廓的面积的阈值，若轮廓面积大于该阈值，则是缺陷

		int areaID;
		for (areaID = 0; areaID < thisFilter.whetherGlassed.size() - 1; areaID++)	//线程并行执行
		{
			thread t(detectThread, thisFilter, radiusThres_silkprint, contourAreaThres_silkprint, radiusThres_glass, contourAreaThres_glass, silkPrint_show_list, areaID);
			t.join();
		}
		thread t(detectThread, thisFilter, radiusThres_silkprint, contourAreaThres_silkprint, radiusThres_glass, contourAreaThres_glass, silkPrint_show_list, areaID);
		t.join();//最后一个线程用join，表示只有当该线程结束之后才继续执行主进程

		//将检测结果输出文件
		ofstream out("result.txt", ios::trunc);
		if (out.is_open()){
			for (int i = 0; i < 6; i++){
				if (SingleResult[i].flag){
					out << "<" << SingleResult[i].label + 1 << ">号滤光片有缺陷" << endl;
					cout << "<" << SingleResult[i].label + 1 << ">号滤光片有缺陷" << endl;
				}
			}
			out.close();
		}
	}
	else{
		cout << "未检测到滤光片" << endl;
	}

	waitKey();
	//getchar();	//为了让命令行窗口在执行完后不马上关闭
}