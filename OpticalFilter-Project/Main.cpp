#include "AssisFunction.h"
#include "Filter.h"
#include <thread>
#include <windows.h>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

DWORD start, stop;
int silkprint_flag[6] = { 0, 0, 0, 0, 0, 0 };
int glass_flag[6] = { 0, 0, 0, 0, 0, 0 };

void silkprintThread(Filter thisFilter, int radiusThres_silkprint, int contourAreaThres_silkprint, vector<Mat> silkPrint_show_list,int areaID)//����̼߳��˿ӡ
{
	if (thisFilter.whetherGlassed[areaID] != 0)
	{
		int flag = silkprintDetect(thisFilter.silkprint_list[areaID], radiusThres_silkprint, contourAreaThres_silkprint, silkPrint_show_list[areaID]);
		silkprint_flag[areaID] = flag;
		if (flag != 0)
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

void glassThread(Filter thisFilter, int radiusThres_glass, int contourAreaThres_glass, int areaID)//����̼߳�⾵��
{
	if (thisFilter.whetherGlassed[areaID] != 0)
	{
		int flag = glassDetect(thisFilter.glass_list[areaID], radiusThres_glass, contourAreaThres_glass);
		glass_flag[areaID] = flag;
		if (flag != 0)
		{
			stop = GetTickCount();
			cout << "Area:" + Int_to_String(areaID) + " Glass defected\ttime: " << stop - start << " ms" << endl;
			imwrite(Int_to_String(areaID) + "glass.jpg", thisFilter.glass_list[areaID]);
		}
	}
}

int main()
{
	int num = 6;		//��ǰ�ڼ���ͼƬ����Ŵ�0��ʼ
	int typeIndex = 0;	//ѡ��ͼƬ����
	vector<string> type = { "Flat_Sprinting", "Flat_NoSprinting", "DoubleCam", "Thread", "New" };	//����ͼƬ������
	int item_num = 6;	//һ��ͼƬ�е��˹�Ƭ����
	vector<string> filepath = { "E:\\�о���\\Work\\ͼƬ\\��ƽ\\��ƽ˿ӡ" ,
								"D:\\Study\\Reborn\\Work\\Image\\Flat\\NoSprinting",
								"D:\\Study\\Reborn\\Work\\Image\\DoubleCam",
								"D:\\Study\\Reborn\\Work\\Image\\Thread",
								"D:\\Study\\Reborn\\Work\\Image\\Flat\\New"
								};	//����ͼƬ��·��
	string postFix = "bmp";	//ͼƬ��׺��
	vector<Mat> silkPrint_show_list;
	Filter thisFilter(filepath[typeIndex], postFix, num);
	//thisFilter.GetRoi(filepath[typeIndex], postFix, num);	//�ֶ�λ��ÿ���˹�Ƭ


	vector<vector<double>> image_sizes = getImageSize(type[typeIndex]);	//��ȡ�˹�Ƭ��С���εĳ���

	if (typeIndex == 0 && num == 3 || typeIndex == 0 && num == 6)		//����ⲿ͸���ͼƬ
		thisFilter.imageMatting2(silkPrint_show_list, image_sizes[num]);//�ֱ���ȡ��˿ӡ�;��沿��
	else
		thisFilter.imageMatting(silkPrint_show_list, image_sizes[num]);	//�ֱ���ȡ��˿ӡ�;��沿��

	//������Ͱ�������������Ҫ��ȡ�����ݶ��ó����ˣ����������Գ�ʹ�ö��߳�������

	int radiusThres_glass = 10;		//��⵽����С���Բ�뾶����ֵ������С���Բ�뾶���ڸ���ֵ������ȱ��
	int contourAreaThres_glass = 30;	//��⵽���������������ֵ��������������ڸ���ֵ������ȱ��

	int radiusThres_silkprint = 10;	//��⵽����С���Բ�뾶����ֵ������С���Բ�뾶���ڸ���ֵ������ȱ��
	int contourAreaThres_silkprint = 80;	//��⵽���������������ֵ��������������ڸ���ֵ������ȱ��

	start = GetTickCount();
	int areaID;
	for (areaID = 0; areaID < item_num - 1; areaID++)	//�̲߳���ִ��
	{
		thread OuterSilk(silkprintThread, thisFilter, radiusThres_silkprint, contourAreaThres_silkprint, silkPrint_show_list, areaID);	//˿ӡ����߳�
		thread InnerGlass(glassThread, thisFilter, radiusThres_glass, contourAreaThres_glass, areaID);       //�������߳�
		OuterSilk.detach(); 
		InnerGlass.detach(); 
	}
	thread OuterSilk(silkprintThread, thisFilter, radiusThres_silkprint, contourAreaThres_silkprint, silkPrint_show_list, areaID);	//˿ӡ����߳�
	thread InnerGlass(glassThread, thisFilter, radiusThres_glass, contourAreaThres_glass, areaID);       //�������߳�
	OuterSilk.join();	//���һ���߳���join����ʾֻ�е����߳̽���֮��ż���ִ��������
	InnerGlass.join();	//���һ���߳���join����ʾֻ�е����߳̽���֮��ż���ִ��������

	//�����������ļ�
	ofstream out("result.txt", ios::trunc);
	if (out.is_open())
	{
		for (int i = 0; i < sizeof(silkprint_flag) / sizeof(silkprint_flag[0]); i++){
			if (silkprint_flag[i] == 1 || glass_flag[i] == 1)
				out << "<" << i + 1 << ">���˹�Ƭ��ȱ��" << endl;
		}
		out.close();
	}
	waitKey();
	//getchar();	//Ϊ���������д�����ִ��������Ϲر�
}