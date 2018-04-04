#include "AssisFunction.h"
#include "Filter.h"
#include <thread>
using namespace std;
using namespace cv;
 
void t1(Filter thisFilter, int radiusThres_silkprint, int contourAreaThres_silkprint, vector<Mat> silkPrint_show_list,int areaID)//����̼߳��˿ӡ
{
	if (thisFilter.whetherGlassed[areaID] != 0)
	{
		if (silkprintDetect(thisFilter.silkprint_list[areaID], radiusThres_silkprint, contourAreaThres_silkprint, silkPrint_show_list[areaID])!=0)
		{
			cout << "Area:" + Int_to_String(areaID) + " Silk defected" << endl;
			imwrite(Int_to_String(areaID) + "silkprint.jpg", silkPrint_show_list[areaID]);
			
		}
		//�˹�Ƭ˿ӡ���

		//cout << "Area:" + Int_to_String(areaID) + " Glass Detected" << endl;
	}
	else
	{
		cout << "Area:" + Int_to_String(areaID) + " don't have filter" << endl;
	}
}

void t2(Filter thisFilter, int radiusThres_glass, int contourAreaThres_glass, int areaID)//����̼߳�⾵��
{
	if (thisFilter.whetherGlassed[areaID] != 0)
	{
		if (glassDetect(thisFilter.glass_list[areaID], radiusThres_glass, contourAreaThres_glass) != 0)
		{
			cout << "Area:" + Int_to_String(areaID) + " Glass defected" << endl;
			imwrite(Int_to_String(areaID) + "glass.jpg", thisFilter.glass_list[areaID]);
		}
		//cout << "Area:" + Int_to_String(areaID) + " Silk Detected" << endl;
	}
	else
	{
		//cout << "Area:" + Int_to_String(areaID) + " don't have filter" << endl;
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
	vector<vector<Mat>> Six_area(item_num);	//���ڴ��6���˹�Ƭλ��
	vector<Mat> silkPrint_show_list;
	Filter thisFilter(filepath[typeIndex], postFix, num);
	//thisFilter.GetRoi(filepath[typeIndex], postFix, num);	//�ֶ�λ��ÿ���˹�Ƭ



	vector<vector<double>> image_sizes = getImageSize(type[typeIndex]);	//��ȡ�˹�Ƭ��С���εĳ���

	if (typeIndex == 0 && num == 3 || typeIndex == 0 && num == 6) {		//����ⲿ͸���ͼƬ
		thisFilter.imageMatting2(silkPrint_show_list, image_sizes[num]);//�ֱ���ȡ��˿ӡ�;��沿��
	}
	else {
		thisFilter.imageMatting(silkPrint_show_list, image_sizes[num]);	//�ֱ���ȡ��˿ӡ�;��沿��
	}
	//for (int i = 0; i < thisFilter.silkprint_list.size(); i++){
	//	imshow("silkprint_" + to_string(i), thisFilter.silkprint_list[i]);
	//}
	/*int i = 0;
	imshow("silkprint_" + to_string(i), thisFilter.silkprint_list[i]);*/

	//������Ͱ�������������Ҫ��ȡ�����ݶ��ó����ˣ����������Գ�ʹ�ö��߳�������

	int radiusThres_glass = 10;		//��⵽����С���Բ�뾶����ֵ������С���Բ�뾶���ڸ���ֵ������ȱ��
	int contourAreaThres_glass = 30;	//��⵽���������������ֵ��������������ڸ���ֵ������ȱ��

	int radiusThres_silkprint = 10;	//��⵽����С���Բ�뾶����ֵ������С���Բ�뾶���ڸ���ֵ������ȱ��
	int contourAreaThres_silkprint = 80;	//��⵽���������������ֵ��������������ڸ���ֵ������ȱ��

	for (int areaID = 0; areaID < item_num; areaID++)
	{
		thread OuterSilk(t1, thisFilter, radiusThres_silkprint, contourAreaThres_silkprint, silkPrint_show_list, areaID);	//˿ӡ����߳�
		thread InnerGlass(t2, thisFilter, radiusThres_glass, contourAreaThres_glass, areaID);       //�������߳�

		OuterSilk.join(); //�ȴ�th1ִ����
		InnerGlass.join(); //�ȴ�th2ִ����
	}

	waitKey();
}