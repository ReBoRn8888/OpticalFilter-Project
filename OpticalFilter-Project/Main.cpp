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
				templateGet FilterParameter, int areaID, double xposParameter, double yposParameter){//�˹�Ƭ����߳�
	int flag = 1;
	SingleResult[areaID].label = areaID;
	SingleResult[areaID].xpos = xposParameter;
	SingleResult[areaID].ypos = yposParameter;

	thisFilter.Area1[areaID].copyTo(SingleResult[areaID].src[0]);
	thisFilter.Area2[areaID].copyTo(SingleResult[areaID].src[1]);
	thisFilter.Area3[areaID].copyTo(SingleResult[areaID].src[2]);
	if (thisFilter.whetherGlassed[areaID] == -1){
		cout << "Area:" + Int_to_String(areaID) + " have ng Filter" << endl;
		SingleResult[areaID].flag = 1;
	}
	if (thisFilter.whetherGlassed[areaID] == 1){
		SingleResult[areaID].type = FilterParameter.filterType;
		int silkprint_flag = silkprintDetect(thisFilter.silkprint_list[areaID], radiusThres_silkprint, contourAreaThres_silkprint, silkPrint_show_list[areaID], FilterParameter);
		if (silkprint_flag != 0){
			cout << "Area:" + Int_to_String(areaID) + " Silk defected" << endl;
			imwrite(Int_to_String(areaID) + "silkprint.jpg", silkPrint_show_list[areaID]);
		}
		int glass_flag = glassDetect(thisFilter.glass_list[areaID], radiusThres_glass, contourAreaThres_glass, FilterParameter);
		if (glass_flag != 0){
			cout << "Area:" + Int_to_String(areaID) + " Glass defected" << endl;
			imwrite(Int_to_String(areaID) + "glass.jpg", thisFilter.glass_list[areaID]);
		}
		if (silkprint_flag || glass_flag)
			SingleResult[areaID].flag = 1;
	}
	else
		cout << "Area:" + Int_to_String(areaID) + " don't have filter" << endl;
}

int main()
{
	int num = 6;		//��ǰ�ڼ���ͼƬ����Ŵ�0��ʼ
	vector<int> filterArea = { 320000, 320000, 215000, 225000, 270000, 145000, 250000 };//Ԥ�õ��˹�Ƭ�����֮���ֶ��������
	vector<int> filterHeight = { 0, 0, 0, 0, 0, 590, 719 };//Ԥ�õ��˹�Ƭ�߶ȣ�֮���ֶ��������
	vector<int> filterWidth = { 0, 0, 0, 0, 0, 730, 645 };//Ԥ�õ��˹�Ƭ��ȣ�֮���ֶ��������
	vector<int> filterType = { 0, 0, 0, 1, 0, 0, 1 };//Ԥ�õ��˹�Ƭ���ͣ�֮���ֶ��������
	int item_num = 6;	//һ��ͼƬ�е��˹�Ƭ����
	string filepath = "E:\\�о���\\Work\\�˹�Ƭȱ�ݼ��\\ͼƬ\\��ƽ\\��ƽ˿ӡ";	//����ͼƬ��·��
	string postFix = "bmp";	//ͼƬ��׺��

	double xposParameter=1.0;
	double yposParameter=1.0;

	//������ģ���ȡ�Ĳ����ĳ�ʼ��
	FilterParameter.ratio = 0.75306;					//�˹�Ƭ�����
	FilterParameter.elementSize = 9;					//���ͺ͸�ʴ�ĳߴ�
	FilterParameter.filterType = filterType[num];		//�˹�Ƭ����
	FilterParameter.firstHeight = 800;					//�ֶ�λ�ĸ߶�
	FilterParameter.firstWidth = 800;					//�ֶ�λ�Ŀ��
	FilterParameter.filterHeight = filterHeight[num];	//�˹�Ƭ�ĸ߶�
	FilterParameter.filterWidth = filterWidth[num];		//�˹�Ƭ�Ŀ��
	FilterParameter.filterArea = filterArea[num];		//�˹�Ƭ���
	FilterParameter.filterLength = 2400;				//�˹�Ƭ�ܳ�
	FilterParameter.areaUpoffset = 800;					//���������ƫ����
	FilterParameter.areaDownoffset = 700;				//���������ƫ����
	FilterParameter.lengthUpoffset = 50;				//�ܳ�������ƫ����
	FilterParameter.lengthDownoffset = 50;				//�ܳ�������ƫ����
	FilterParameter.glassThresUpoffset = 10;				//���ʱ͸������ֵ����ֵ������ƫ����
	FilterParameter.glassThresDownoffset = 10;			//���ʱ͸������ֵ����ֵ������ƫ����
	FilterParameter.silkThresUpoffset = 10;				//���ʱ˿ӡ��ֵ����ֵ������ƫ����
	FilterParameter.silkThresDownoffset = 10;			//���ʱ˿ӡ��ֵ����ֵ������ƫ����

	Filter thisFilter(filepath, postFix, num, FilterParameter);

	if (thisFilter.whetherNull == false){
		vector<Mat> silkPrint_show_list;
		if (FilterParameter.filterType == 0)//�����ͨ�ı�ƽͼƬ
			thisFilter.imageMatting(silkPrint_show_list, FilterParameter);//�ֱ���ȡ��˿ӡ�;��沿�� 
		else if (FilterParameter.filterType == 1)//����ⲿ͸��ı�ƽͼƬ
			thisFilter.imageMatting2(silkPrint_show_list, FilterParameter);//�ֱ���ȡ��˿ӡ�;��沿��

		//������Ͱ�������������Ҫ��ȡ�����ݶ��ó����ˣ����������Գ�ʹ�ö��߳�������
		int radiusThres_glass = 10;		//��⵽����С���Բ�뾶����ֵ������С���Բ�뾶���ڸ���ֵ������ȱ��
		int contourAreaThres_glass = 30;	//��⵽���������������ֵ��������������ڸ���ֵ������ȱ��
		int radiusThres_silkprint = 10;	//��⵽����С���Բ�뾶����ֵ������С���Բ�뾶���ڸ���ֵ������ȱ��
		int contourAreaThres_silkprint = 40;	//��⵽���������������ֵ��������������ڸ���ֵ������ȱ��

		int areaID = 0;
		for (areaID = 0; areaID < thisFilter.whetherGlassed.size() - 1; areaID++)	//�̲߳���ִ��
		{
			thread t(detectThread, thisFilter, radiusThres_silkprint, contourAreaThres_silkprint, radiusThres_glass, contourAreaThres_glass,
				silkPrint_show_list, FilterParameter, areaID, xposParameter, yposParameter);
			t.join();
		}
		thread t(detectThread, thisFilter, radiusThres_silkprint, contourAreaThres_silkprint, radiusThres_glass, contourAreaThres_glass,
			silkPrint_show_list, FilterParameter, areaID, xposParameter, yposParameter);
		t.join();//���һ���߳���join����ʾֻ�е����߳̽���֮��ż���ִ��������

		//������������ṹ��
		for (int i = 0; i < thisFilter.glass_list.size(); i++){
			if (SingleResult[i].flag){
				cout << "<" << SingleResult[i].label + 1 << ">���˹�Ƭ��ȱ��" << endl;
				algorithmOutput.ngFileter.push_back(SingleResult[i]);
			}
			algorithmOutput.allFilter.push_back(SingleResult[i]);
		}
	}
	else{
		cout << "δ��⵽�˹�Ƭ" << endl;
	}

	waitKey();
	getchar();	//Ϊ���������д�����ִ��������Ϲر�
}