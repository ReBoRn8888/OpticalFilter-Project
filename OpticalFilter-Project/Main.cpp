#include "AssisFunction.h"
#include "Filter.h"
#include <thread>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

SinglefilterMessage SingleResult[6];

void detectThread(Filter thisFilter, int radiusThres_silkprint, int contourAreaThres_silkprint, int radiusThres_glass, int contourAreaThres_glass, vector<Mat> silkPrint_show_list, int areaID)//�˹�Ƭ����߳�
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
	int num = 1;		//��ǰ�ڼ���ͼƬ����Ŵ�0��ʼ
	int typeIndex = 0;	//ѡ��ͼƬ����
	//vector<string> type = { "Flat_Sprinting", "Flat_NoSprinting", "DoubleCam", "Thread", "New" };	//����ͼƬ������
	int item_num = 6;	//һ��ͼƬ�е��˹�Ƭ����
	vector<string> filepath = { "E:\\�о���\\Work\\�˹�Ƭȱ�ݼ��\\All",
								"D:\\Study\\Reborn\\Work\\Image\\Flat\\NoSprinting",
								"D:\\Study\\Reborn\\Work\\Image\\DoubleCam",
								"D:\\Study\\Reborn\\Work\\Image\\Thread",
								"D:\\Study\\Reborn\\Work\\Image\\Flat\\New"
								};	//����ͼƬ��·��
	string postFix = "bmp";	//ͼƬ��׺��
	int pedestalArea = 500000;
	Filter thisFilter(filepath[typeIndex], postFix, num, pedestalArea);
	if (thisFilter.whetherNull == false){
		vector<double> image_sizes;
		double type;
		getConfigInfo(image_sizes, type);
		vector<Mat> silkPrint_show_list;
		if (type == 0)		//����ⲿ͸���ͼƬ
			thisFilter.imageMatting(silkPrint_show_list, image_sizes);//�ֱ���ȡ��˿ӡ�;��沿�� 
		else if (type == 1)
			thisFilter.imageMatting2(silkPrint_show_list, image_sizes);	//�ֱ���ȡ��˿ӡ�;��沿��

		//������Ͱ�������������Ҫ��ȡ�����ݶ��ó����ˣ����������Գ�ʹ�ö��߳�������
		int radiusThres_glass = 10;		//��⵽����С���Բ�뾶����ֵ������С���Բ�뾶���ڸ���ֵ������ȱ��
		int contourAreaThres_glass = 30;	//��⵽���������������ֵ��������������ڸ���ֵ������ȱ��
		int radiusThres_silkprint = 10;	//��⵽����С���Բ�뾶����ֵ������С���Բ�뾶���ڸ���ֵ������ȱ��
		int contourAreaThres_silkprint = 80;	//��⵽���������������ֵ��������������ڸ���ֵ������ȱ��

		int areaID;
		for (areaID = 0; areaID < thisFilter.whetherGlassed.size() - 1; areaID++)	//�̲߳���ִ��
		{
			thread t(detectThread, thisFilter, radiusThres_silkprint, contourAreaThres_silkprint, radiusThres_glass, contourAreaThres_glass, silkPrint_show_list, areaID);
			t.join();
		}
		thread t(detectThread, thisFilter, radiusThres_silkprint, contourAreaThres_silkprint, radiusThres_glass, contourAreaThres_glass, silkPrint_show_list, areaID);
		t.join();//���һ���߳���join����ʾֻ�е����߳̽���֮��ż���ִ��������

		//�����������ļ�
		ofstream out("result.txt", ios::trunc);
		if (out.is_open()){
			for (int i = 0; i < 6; i++){
				if (SingleResult[i].flag){
					out << "<" << SingleResult[i].label + 1 << ">���˹�Ƭ��ȱ��" << endl;
					cout << "<" << SingleResult[i].label + 1 << ">���˹�Ƭ��ȱ��" << endl;
				}
			}
			out.close();
		}
	}
	else{
		cout << "δ��⵽�˹�Ƭ" << endl;
	}

	waitKey();
	//getchar();	//Ϊ���������д�����ִ��������Ϲر�
}