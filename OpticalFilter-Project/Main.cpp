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
	int num = 0;		//��ǰ�ڼ���ͼƬ����Ŵ�0��ʼ
	vector<int> filterArea = { 210000, 320000, 215000, 225000, 270000, 145000, 250000, 360000 };//Ԥ�õ��˹�Ƭ�����֮���ֶ��������
	vector<int> filterHeight = { 0, 0, 0, 0, 590, 0, 719, 700 };//Ԥ�õ��˹�Ƭ�߶ȣ�֮���ֶ��������
	vector<int> filterWidth = { 0, 0, 0, 0, 730, 0, 645, 700 };//Ԥ�õ��˹�Ƭ��ȣ�֮���ֶ��������
	vector<int> filterType = { 0, 0, 0, 1, 0, 0, 1, 2 };//Ԥ�õ��˹�Ƭ���ͣ�֮���ֶ��������
	int item_num = 6;	//һ��ͼƬ�е��˹�Ƭ����
	string filepath = "E:\\�о���\\Work\\�˹�Ƭȱ�ݼ��\\Image";	//����ͼƬ��·��
	string postFix = "bmp";	//ͼƬ��׺��

	double xposParameter = 1.0;
	double yposParameter = 1.0;

	//������ģ���ȡ�Ĳ����ĳ�ʼ��
	FilterParameter.ratio = 1.3;					//�˹�Ƭ����ȣ��߶�/��ȣ�
	FilterParameter.elementSize = 7;					//���ͺ͸�ʴ�ĳߴ�
	FilterParameter.filterType = 0;		//�˹�Ƭ����
	FilterParameter.firstHeight = 770;					//�ֶ�λ�ĸ߶�
	FilterParameter.firstWidth = 770;					//�ֶ�λ�Ŀ��
	FilterParameter.filterHeight = 694;	               //�˹�Ƭ�ĸ߶�
	FilterParameter.filterWidth = 556;		           //�˹�Ƭ�Ŀ��
	FilterParameter.filterArea = 215000;		       //�˹�Ƭ���
	FilterParameter.filterLength = 2177;				//�˹�Ƭ�ܳ�
	FilterParameter.areaUpoffset = 2000;					//���������ƫ����
	FilterParameter.areaDownoffset = 2000;				//���������ƫ����
	FilterParameter.lengthUpoffset = 500;				//�ܳ�������ƫ����
	FilterParameter.lengthDownoffset = 500;				//�ܳ�������ƫ����
	FilterParameter.glassThresOffset = 15;			//���ʱ͸������ֵ����ֵ��ƫ����
	FilterParameter.silkThresUpoffset = 20;				//���ʱ˿ӡ��ֵ����ֵ������ƫ����
	FilterParameter.silkThresDownoffset = 20;			//���ʱ˿ӡ��ֵ����ֵ������ƫ����

	Filter thisFilter(filepath, postFix, num, FilterParameter);

	if (thisFilter.whetherNull == false && thisFilter.tested_path.size() != 0){
		vector<Mat> silkPrint_show_list;
		if (FilterParameter.filterType == 0)//�����ͨ��ͼƬ
			thisFilter.imageMatting(silkPrint_show_list, FilterParameter);//�ֱ���ȡ��˿ӡ�;��沿�� 
		else if (FilterParameter.filterType == 1)//����ⲿ����͸������ͼƬ
			thisFilter.imageMatting2(silkPrint_show_list, FilterParameter);//�ֱ���ȡ��˿ӡ�;��沿��
		else if (FilterParameter.filterType == 2)//���͸����ΪԲ�ε�ͼƬ
			thisFilter.imageMatting3(silkPrint_show_list, FilterParameter);//�ֱ���ȡ��˿ӡ�;��沿��

		//������Ͱ�������������Ҫ��ȡ�����ݶ��ó����ˣ����������Գ�ʹ�ö��߳�������
		int radiusThres_glass = 5;		//��⵽����С���Բ�뾶����ֵ������С���Բ�뾶���ڸ���ֵ������ȱ��
		int contourAreaThres_glass = 7;	//��⵽���������������ֵ��������������ڸ���ֵ������ȱ��
		int radiusThres_silkprint = 5;	//��⵽����С���Բ�뾶����ֵ������С���Բ�뾶���ڸ���ֵ������ȱ��
		int contourAreaThres_silkprint = 7;	//��⵽���������������ֵ��������������ڸ���ֵ������ȱ��

		int areaID = 0;
		for (areaID = 0; areaID < thisFilter.whetherGlassed.size(); areaID++)	//�̲߳���ִ��
		{
			thread t(detectThread, thisFilter, radiusThres_silkprint, contourAreaThres_silkprint, radiusThres_glass, contourAreaThres_glass,
				silkPrint_show_list, FilterParameter, areaID, xposParameter, yposParameter);
			t.join();
		}

		//������������ṹ��
		for (int i = 0; i < thisFilter.glass_list.size(); i++){
			if (SingleResult[i].flag == 1){
				cout << "<" << SingleResult[i].label << ">���˹�Ƭ��ȱ��" << endl;
				algorithmOutput.ngFileter.push_back(SingleResult[i]);
			}
			else if (SingleResult[i].flag == -1)
				cout << "<" << SingleResult[i].label << ">��λ���˹�Ƭ����" << endl;
			algorithmOutput.allFilter.push_back(SingleResult[i]);
		}
	}
	else if (thisFilter.tested_path.size() == 0)
		cout << "ͼƬ·������" << endl;
	else
		cout << "δ��⵽�˹�Ƭ" << endl;

	waitKey();
	getchar();	//Ϊ���������д�����ִ��������Ϲر�
}