#include "GetArea.h"


using namespace cv;
using namespace std;


int main()
{
	int num = 6;		//��ǰ�ڼ���ͼƬ����Ŵ�0��ʼ
	int typeIndex = 0;	//ѡ��ͼƬ����
	vector<string> type = { "Flat_Sprinting","Flat_NoSprinting", "DoubleCam", "Thread", "New" };	//����ͼƬ������
	int item_num = 6;	//һ��ͼƬ�е��˹�Ƭ����
	vector<string> filepath = { "D:\\Study\\Reborn\\Work\\Image\\Flat\\Sprinting" ,
								"D:\\Study\\Reborn\\Work\\Image\\Flat\\NoSprinting",
								"D:\\Study\\Reborn\\Work\\Image\\DoubleCam",
								"D:\\Study\\Reborn\\Work\\Image\\Thread",
								"D:\\Study\\Reborn\\Work\\Image\\Flat\\New"
								};	//����ͼƬ��·��
	string postFix = "bmp";	//ͼƬ��׺��
	vector<vector<Mat>> Six_area(item_num);	//���ڴ��6���˹�Ƭλ��
	vector<Mat> silkPrint_show_list;
	vector<int> isGlassed;					//�ж϶�Ӧλ���Ƿ����˹�Ƭ
	vector<Mat> silkprint_list; vector<Mat> glass_list;		//�����ȡ����˿ӡ�;��沿��

	Six_area = GetRoi(filepath[typeIndex], postFix, num, isGlassed);	//�ֶ�λ��ÿ���˹�Ƭ
	
	vector<vector<double>> image_sizes = getImageSize(type[typeIndex]);	//��ȡ�˹�Ƭ��С���εĳ���

	if (typeIndex == 0 && num == 3 || typeIndex == 0 && num == 6) {
		imageMatting_outer(silkPrint_show_list, silkprint_list, glass_list, Six_area, image_sizes[num], isGlassed);	//�ֱ���ȡ��˿ӡ�;��沿��(���˿ӡ�ⲿ���о�������)
	}
	else {
		imageMatting(silkPrint_show_list, silkprint_list, glass_list, Six_area, image_sizes[num], isGlassed);	//�ֱ���ȡ��˿ӡ�;��沿��
	}

	//for (int i = 0; i < silkprint_list.size(); i++)
	//{
	//	//namedWindow("silkprint: " + Int_to_String(i + 1) + ".jpg", 0);
	//	//namedWindow("glass: " + Int_to_String(i + 1) + ".jpg", 0);
	//	imshow("silkprint: " + Int_to_String(i) + ".jpg", silkprint_list[i]);
	//	imshow("glass: " + Int_to_String(i) + ".jpg", glass_list[i]);
	//	//imshow(Int_to_String(i + 1) + ".jpg", Six_area[1][i]);
	//}

	int radiusThres_glass = 10;		//��⵽����С���Բ�뾶����ֵ������С���Բ�뾶���ڸ���ֵ������ȱ��
	int contourAreaThres_glass = 30;	//��⵽���������������ֵ��������������ڸ���ֵ������ȱ��
	glassDetect(glass_list, radiusThres_glass, contourAreaThres_glass, isGlassed);	//�˹�Ƭ������

	int radiusThres_silkprint = 2;	//��⵽����С���Բ�뾶����ֵ������С���Բ�뾶���ڸ���ֵ������ȱ��
	int contourAreaThres_silkprint = 15;	//��⵽���������������ֵ��������������ڸ���ֵ������ȱ��
	//silkprintDetect(silkprint_list, radiusThres_silkprint, contourAreaThres_silkprint, silkPrint_show_list, isGlassed);	//�˹�Ƭ˿ӡ���
	//silkprintDetect2(silkprint_list, radiusThres_silkprint, contourAreaThres_silkprint, silkPrint_show_list, isGlassed);	//�˹�Ƭ˿ӡ���

	waitKey();
}