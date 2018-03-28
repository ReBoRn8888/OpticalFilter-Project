#include "GetArea.h"
using namespace std;
using namespace cv;

Point tl, tr, bl, br;

class d3sort {
public:
	d3sort(int a, int b, int c) :first(a), second(b), third(c) {}
	int first;
	int second;
	int third;
	bool operator < (const d3sort &m)const {
		return first < m.first;
	}
};

string Int_to_String(int n)

{
	ostringstream stream;
	stream << n;  //nΪint����
	return stream.str();
}

void ImageSharp(Mat &src, Mat &dst)
{
	int nAmount = 200;
	double sigma = 3;
	int threshold = 1;
	float amount = nAmount / 100.0f;

	Mat imgBlurred;
	GaussianBlur(src, imgBlurred, Size(), sigma, sigma);

	Mat lowContrastMask = abs(src - imgBlurred)<threshold;
	dst = src*(1 + amount) + imgBlurred*(-amount);
	src.copyTo(dst, lowContrastMask);
}

int Get6th(vector<vector<Point>> contours, int Area_num)//ע��ڶ�������������ȡ��һ��Ҫ������������������>0
{
	int length = contours.size();
	vector<int> idx;
	double g_dConLength;
	for (int i = 0; i < length; i++)
	{
		g_dConLength = arcLength(contours[i], true);
		idx.push_back(g_dConLength);
	}
	sort(idx.begin(), idx.end());

	return idx.at(length - Area_num);

}

int Otsu(IplImage* src)
{
	int height = src->height;
	int width = src->width;

	//histogram
	float histogram[256] = { 0 };
	for (int i = 0; i < height; i++)
	{
		unsigned char* p = (unsigned char*)src->imageData + src->widthStep * i;
		for (int j = 0; j < width; j++)
		{
			histogram[*p++]++;
		}
	}

	//normalize histogram & average pixel value
	int size = height * width;
	float u = 0;
	for (int i = 0; i < 256; i++)
	{
		histogram[i] = histogram[i] / size;
		u += i * histogram[i];  //����ͼ���ƽ���Ҷ�
	}

	int threshold;
	float maxVariance = 0;
	float w0 = 0, avgValue = 0;
	for (int i = 0; i < 256; i++)
	{
		w0 += histogram[i];  //���赱ǰ�Ҷ�iΪ��ֵ, 0~i �Ҷ�������ռ����ͼ��ı�����ǰ������
		avgValue += i * histogram[i]; //avgValue/w0 = u0

		float t = avgValue / w0 - u;  //t=u0-u
		float variance = t * t * w0 / (1 - w0);
		if (variance > maxVariance)
		{
			maxVariance = variance;
			threshold = i;
		}
	}

	return threshold;
}

vector<Point2f> sortCenterpoint(vector<Point2f> centers, int middle, vector<int>isGlassed, vector<int>&OutGlassed)
{
	vector<Point2f> new_center;
	vector< d3sort > vectA;
	vector< d3sort > vectB;
	vector<int>NewisGlassed;

	for (int i = 0; i < centers.size(); i++)
	{
		d3sort my(centers[i].x, centers[i].y, isGlassed[i]);

		if (centers[i].y>middle)
		{
			vectA.push_back(my);
		}
		else
		{
			vectB.push_back(my);
		}

	}

	sort(vectA.begin(), vectA.end());
	sort(vectB.begin(), vectB.end());

	for (int i = 0; i < vectA.size(); i++)
	{
		new_center.push_back(Point2f(vectA[i].first, vectA[i].second));
		NewisGlassed.push_back(vectA[i].third);
	}

	for (int i = 0; i < vectB.size(); i++)
	{
		new_center.push_back(Point2f(vectB[i].first, vectB[i].second));
		NewisGlassed.push_back(vectB[i].third);
	}
	OutGlassed = NewisGlassed;
	return new_center;
}

vector<int> GetArea(Mat img, int item_num, vector<Point2f>&mycenter, float&myradius)
{
	Mat edge;
	threshold(img, edge, 120, 255, 0);

	Mat dstImg = Mat::zeros(edge.rows, edge.cols, edge.type());
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	vector <vector<Point>> contours_poly(item_num);
	vector<Point2f>  center(item_num);
	vector<float> radius(item_num);
	vector<int> isGlass(item_num);

	findContours(edge, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);//�ⶫ�������ֱ�Ӹ㻵��ԭͼ
	int index = 0; int comTemp = 10000000000; int Areacount = 0;
	int the_num = Get6th(contours, item_num);
	for (; index >= 0; index = hierarchy[index][0])
	{
		Scalar color(255);
		int xx = arcLength(contours[index], true);
		if (arcLength(contours[index], true) >= the_num)
		{

			approxPolyDP(Mat(contours[index]), contours_poly[Areacount], 3, true);//�ƽ����ߣ�Ӧ��Ҫ����
			minEnclosingCircle(contours_poly[Areacount], center[Areacount], radius[Areacount]);

			if (comTemp > radius[Areacount])
				comTemp = radius[Areacount];
			//areaCount++;
			if (arcLength(contours[index], true) < the_num*1.2)
				isGlass[Areacount] = 1;
			else
				isGlass[Areacount] = 0;
			Areacount++;
			if (Areacount == item_num)
				break;
		}

	}
	myradius = comTemp;
	mycenter = center;

	return isGlass;
}

//����ĺ�����ȡ��һ��Ŀ�������ͼ�ķֱ��roi���򣬴��3*vector����ʽ
vector<vector<Mat>> GetRoi(string filepath, string postFix, int num, vector<int>&OutGlassed)
{
	Mat image1, image2, image3, image4; int length; double ratio = 1.2;
	int item_num = 6; vector<string> tested_path;
	tested_path = pathGet(filepath, postFix);
	image1 = imread(tested_path[num * 3], 0);
	image2 = imread(tested_path[num * 3 + 1], 0);
	image3 = imread(tested_path[num * 3 + 2], 0);
	image4 = imread(tested_path[num * 3 + 2], 0);
	vector<Point2f>mycenter(item_num); float myradius;

	//imwrite("BG2.bmp", image2);
	vector<int> outGlass(item_num);

	vector<vector<Mat>> Six_area;
	vector<Mat> Six_area1(item_num);
	vector<Mat> Six_area2(item_num);
	vector<Mat> Six_area3(item_num);

	vector<int> isGlass = GetArea(image3, item_num, mycenter, myradius);

	vector<Point2f> newcenter = sortCenterpoint(mycenter, image1.rows / 2, isGlass, outGlass);

	for (int i = 0; i < item_num; i++)
	{
		length = myradius* ratio;
		image1(Rect(newcenter.at(i).x - length, newcenter.at(i).y - length, length * 2, length * 2)).copyTo(Six_area1[i]);//��һ��ͼROI���������
		image2(Rect(newcenter.at(i).x - length, newcenter.at(i).y - length, length * 2, length * 2)).copyTo(Six_area2[i]);//�ڶ���ͼROI���������
		image4(Rect(newcenter.at(i).x - length, newcenter.at(i).y - length, length * 2, length * 2)).copyTo(Six_area3[i]);//������ͼROI���������
		putText(image3, to_string(i + 1), newcenter.at(i), FONT_HERSHEY_PLAIN, 5, Scalar(100), 5, 8);
		//imwrite("tempROI.jpg", Six_area[i]);
	}
	namedWindow("src", 0);
	imshow("src", image3);
	Six_area.push_back(Six_area1);
	Six_area.push_back(Six_area2);
	Six_area.push_back(Six_area3);
	OutGlassed = outGlass;
	return Six_area;
}

int getModeNumber(vector<int> ll) {
	sort(ll.begin(), ll.end());
	int k = 0;
	int MaxCount = 1;
	int index = 0;
	while (k <= ll.size() - 1){
		int count = 1;
		int j;
		for (j = k; j < ll.size() - 1; j++){
			if (ll[j] == ll[j + 1])//����������������ȣ�������+1  
				count++;
			else
				break;
		}
		if (MaxCount < count){
			MaxCount = count;//��ǰ�������  
			index = j;//��ǰ�������λ��  
		}
		++j;
		k = j;//λ�ú��Ƶ���һ��δ���ֵ�����  
	}
	return ll[index];
}

void imageMatting(vector<Mat> &silkPrint_show_list, vector<Mat> &silkprint_list, vector<Mat> &glass_list, vector<vector<Mat>> Six_area, vector<double> image_size, vector<int>OutGlassed) {
	Mat roi1, roi2, roi3;
	for (int i = 0; i < Six_area.at(0).size(); i++){
		//putText(src1, to_string(i), mycenter[i], FONT_HERSHEY_PLAIN, 5, Scalar(100), 5, 8);
		if (OutGlassed[i] == 1) {
			roi1 = Six_area.at(0)[i];
			roi2 = Six_area.at(1)[i];
			roi3 = Six_area.at(2)[i];

			float degree = RotatedDegree(roi3);
			Point2f center = Point2f(roi2.cols / 2, roi2.rows / 2);
			Mat rotateMat = getRotationMatrix2D(center, degree, 1);
			warpAffine(roi1, roi1, rotateMat, roi1.size());
			warpAffine(roi2, roi2, rotateMat, roi2.size());
			warpAffine(roi3, roi3, rotateMat, roi3.size());

			Mat thresroi3;
			threshold(roi3, thresroi3, 0, 255.0, CV_THRESH_BINARY | CV_THRESH_OTSU);    //��ֵ��
			vector<vector<Point>> contours;
			vector<Vec4i> hierarchy;
			findContours(thresroi3, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

			//���˵�С������ȡ���м������˹�Ƭ����
			vector<vector<Point>>::iterator it_contour = contours.begin();
			while (it_contour != contours.end())
			{
				//                cout << contourArea(*it_contour) << endl;
				if (contourArea(*it_contour) < 100000 || contourArea(*it_contour) > 500000)//ɾȥ���С��100000������
					it_contour = contours.erase(it_contour);
				else
					++it_contour;
			}

			//��¼�˹�Ƭ�������Ϻ����µ�����꣬���ھ�ȷ��λ�����˹�Ƭ
			tl = Point(99999, 99999), tr = Point(-1, 99999), bl = Point(99999, -1), br = Point(-1, -1);
			for (int pp = 0; pp < contours[0].size(); pp++) {
				if (contours[0][pp].x < roi3.cols / 3 && contours[0][pp].y < roi3.rows / 3) {
					if (tl.x > contours[0][pp].x)
						tl.x = contours[0][pp].x;
					if (tl.y > contours[0][pp].y)
						tl.y = contours[0][pp].y;
				}
				else if (contours[0][pp].x < roi3.cols / 3 && contours[0][pp].y > roi3.rows / 3 * 2) {
					if (bl.x > contours[0][pp].x)
						bl.x = contours[0][pp].x;
					if (bl.y < contours[0][pp].y)
						bl.y = contours[0][pp].y;
				}
				else if (contours[0][pp].x > roi3.cols / 3 * 2 && contours[0][pp].y < roi3.rows / 3) {
					if (tr.x < contours[0][pp].x)
						tr.x = contours[0][pp].x;
					if (tr.y > contours[0][pp].y)
						tr.y = contours[0][pp].y;
				}
				else if (contours[0][pp].x > roi3.cols / 3 * 2 && contours[0][pp].y > roi3.rows / 3 * 2) {
					if (br.x < contours[0][pp].x)
						br.x = contours[0][pp].x;
					if (br.y < contours[0][pp].y)
						br.y = contours[0][pp].y;
				}
			}
			Mat glass_mask(roi3.rows, roi3.cols, CV_8UC1, Scalar(0));   //�˹�Ƭmask
			drawContours(glass_mask, contours, -1, Scalar(255), CV_FILLED, 8);

			//��ȷ��λ�����˹�Ƭ
			Mat temp = roi2.clone();
			int width = tr.x - tl.x;	//ͼ���ڲ�С���εĿ��
			int height = bl.y - tl.y;	//ͼ���ڲ�С���εĸ߶�
			double width_ratio = image_size[0] / image_size[2];		//��С���εĿ�ȱ�
			double height_ratio = image_size[1] / image_size[3];	//��С���εĸ߶ȱ�
			int newwidth = width * width_ratio;		//�����ͼ���ⲿ����εĿ�ȣ�������ȡ�������˹�Ƭ
			int newheight = height * height_ratio;	//�����ͼ���ⲿ����εĸ߶ȣ�������ȡ�������˹�Ƭ
			silkPrint_show_list.push_back(temp(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight)).clone());

			//�ھ����ĸ����ϻ���ɫ���Σ��������ⲻ����������У����ƽ˿ӡ4 5 6.bmp��
			int w = 60, scale = 15;
			rectangle(temp, Rect(Point(tl.x - scale, tl.y - scale), Point(tl.x + w, tl.y + w)), Scalar(0), CV_FILLED, 8);
			rectangle(temp, Rect(Point(bl.x - scale, bl.y - w), Point(bl.x + w, bl.y + scale)), Scalar(0), CV_FILLED, 8);
			rectangle(temp, Rect(Point(tr.x - w, tr.y - scale), Point(tr.x + scale, tr.y + w)), Scalar(0), CV_FILLED, 8);
			rectangle(temp, Rect(Point(br.x - w, br.y - w), Point(br.x + scale, br.y + scale)), Scalar(0), CV_FILLED, 8);

			Mat whole = temp(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight));	//��ȡ�������˹�Ƭ
			glass_mask = glass_mask(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight));	//����glass_mask�ߴ�

			/*----------------------------------------------------------------*/
			/*-------------------------�˹�Ƭ��ȡģ��-------------------------*/
			/*----------------------------------------------------------------*/
			Mat glass;  //���ڴ洢��ȡ�����˹�Ƭ���沿��
			roi3(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight)).copyTo(glass, glass_mask);  //��ȡ���˹�Ƭ����
			glass_list.push_back(glass);

			/*----------------------------------------------------------------*/
			/*--------------------------˿ӡ��ȡģ��--------------------------*/
			/*----------------------------------------------------------------*/

			findContours(glass_mask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			Mat glass_thres;
			threshold(glass, glass_thres, 0, 255.0, CV_THRESH_BINARY);

			IplImage ipl = (IplImage)whole;
			int th = Otsu(&ipl);
			cout << th << endl;
			drawContours(whole, contours, -1, Scalar(0), 15, 8);
			threshold(whole, whole, th*10/29, 255, CV_THRESH_BINARY);
			//Canny(whole, whole, 100, 100);

			//threshold(whole, whole, 50, 255.0, CV_THRESH_BINARY);
			Mat silkprint; //���ڴ洢��ȡ����˿ӡ����

			//dilate(glass_thres, glass_thres, Mat(5, 5, CV_8U), Point(-1, -1), 2);
			silkprint = whole + glass_thres;//ͨ�����ֵ���ľ��沿����ӣ�����ȡ��˿ӡ����
			//imshow("temp_" + Int_to_String(i + 1), whole);
			//silkprint = silkprint - glass_thres;
			Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
			morphologyEx(silkprint, silkprint, MORPH_OPEN, element);
			//element = getStructuringElement(MORPH_RECT, Size(3, 3));
			//erode(silkprint, silkprint, Mat(5, 5, CV_8U), Point(-1, -1), 1);
			dilate(silkprint, silkprint, element, Point(-1, -1), 1);
			silkprint_list.push_back(silkprint);

			//Mat silkprint_mask;
			//bitwise_not(glass_mask, silkprint_mask);//������mask��λȡ�����õ�˿ӡmask
			//Mat element = getStructuringElement(MORPH_RECT, Size(20, 20));
			//erode(silkprint_mask, silkprint_mask, element);//��ʴ����������˿ӡ�ڲ���Ե�İױ�

			////������������������˿ӡ����������֣�����OTSU�Ķ�ֵ������
			//vector<int> ll;
			//for (int x = whole.cols/4; x < whole.cols/4*3; x++) {
			//	int value = whole.at<uchar>(50, x);
			//	ll.push_back(value);
			//}
			//sort(ll.begin(), ll.end());
			//int k = 0;
			//int MaxCount = 1;
			//int index = 0;
			//while (k <= ll.size() - 1){
			//	int count = 1;
			//	int j;
			//	for (j = k; j < ll.size() - 1; j++){
			//		if (ll[j] == ll[j + 1])//����������������ȣ�������+1  
			//			count++;
			//		else
			//			break;
			//	}
			//	if (MaxCount < count){
			//		MaxCount = count;//��ǰ�������  
			//		index = j;//��ǰ�������λ��  
			//	}
			//	++j;
			//	k = j;//λ�ú��Ƶ���һ��δ���ֵ�����  
			//}
			//Mat gray_mask;
			//Mat gray(glass_mask.rows, glass_mask.cols, CV_8UC1, Scalar(ll[index]));
			//Mat tempmask;
			//bitwise_not(silkprint_mask, tempmask);
			//gray.copyTo(gray_mask, tempmask);

			//Mat silkprint;
			//whole.copyTo(silkprint, silkprint_mask);	//��ȡ��˿ӡ����
			//add(silkprint, gray_mask, silkprint);

			//rectangle(silkprint, Rect(Point(1, 1), Point(silkprint.cols - 1, silkprint.rows - 1)), Scalar(ll[index]), 7, 8);
			//GaussianBlur(silkprint, silkprint, Size(5, 5), 0);
			////imshow("before_" + Int_to_String(i + 1), silkprint);
			////drawContours(silkprint, contours, 0, Scalar(ll[index]), 20, 8);
			////imshow("temp_" + Int_to_String(i + 1), silkprint);
			//silkprint_list.push_back(silkprint);
		}
		else
		{
			Mat dsc = Mat::zeros(1, 1, CV_16U);
			silkprint_list.push_back(dsc);
			glass_list.push_back(dsc);
			silkPrint_show_list.push_back(dsc);
		}
	}
}

void imageMatting_outer(vector<Mat> &silkPrint_show_list, vector<Mat> &silkprint_list, vector<Mat> &glass_list, vector<vector<Mat>> Six_area, vector<double> image_size, vector<int>OutGlassed) {
	Mat roi1, roi2, roi3;
	for (int i = 0; i < Six_area.at(0).size(); i++) {
		//putText(src1, to_string(i), mycenter[i], FONT_HERSHEY_PLAIN, 5, Scalar(100), 5, 8);
		if (OutGlassed[i] == 1) {
			roi1 = Six_area.at(0)[i];
			roi2 = Six_area.at(1)[i];
			roi3 = Six_area.at(2)[i];

			float degree = RotatedDegree(roi3);
			Point2f center = Point2f(roi2.cols / 2, roi2.rows / 2);
			Mat rotateMat = getRotationMatrix2D(center, degree, 1);
			warpAffine(roi1, roi1, rotateMat, roi1.size());
			warpAffine(roi2, roi2, rotateMat, roi2.size());
			warpAffine(roi3, roi3, rotateMat, roi3.size());

			Mat thresroi3;
			threshold(roi3, thresroi3, 0, 255.0, CV_THRESH_BINARY | CV_THRESH_OTSU);    //��ֵ��
			vector<vector<Point>> contours;
			vector<Vec4i> hierarchy;
			findContours(thresroi3, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

			//���˵�С������ȡ���м������˹�Ƭ����
			vector<vector<Point>>::iterator it_contour = contours.begin();
			while (it_contour != contours.end())
			{
				//                cout << contourArea(*it_contour) << endl;
				if (contourArea(*it_contour) < 100000 || contourArea(*it_contour) > 500000)//ɾȥ���С��100000������
					it_contour = contours.erase(it_contour);
				else
					++it_contour;
			}

			//��¼�˹�Ƭ�������Ϻ����µ�����꣬���ھ�ȷ��λ�����˹�Ƭ
			tl = Point(99999, 99999), tr = Point(-1, 99999), bl = Point(99999, -1), br = Point(-1, -1);
			for (int pp = 0; pp < contours[0].size(); pp++) {
				if (contours[0][pp].x < roi3.cols / 3 && contours[0][pp].y < roi3.rows / 3) {
					if (tl.x > contours[0][pp].x)
						tl.x = contours[0][pp].x;
					if (tl.y > contours[0][pp].y)
						tl.y = contours[0][pp].y;
				}
				else if (contours[0][pp].x < roi3.cols / 3 && contours[0][pp].y > roi3.rows / 3 * 2) {
					if (bl.x > contours[0][pp].x)
						bl.x = contours[0][pp].x;
					if (bl.y < contours[0][pp].y)
						bl.y = contours[0][pp].y;
				}
				else if (contours[0][pp].x > roi3.cols / 3 * 2 && contours[0][pp].y < roi3.rows / 3) {
					if (tr.x < contours[0][pp].x)
						tr.x = contours[0][pp].x;
					if (tr.y > contours[0][pp].y)
						tr.y = contours[0][pp].y;
				}
				else if (contours[0][pp].x > roi3.cols / 3 * 2 && contours[0][pp].y > roi3.rows / 3 * 2) {
					if (br.x < contours[0][pp].x)
						br.x = contours[0][pp].x;
					if (br.y < contours[0][pp].y)
						br.y = contours[0][pp].y;
				}
			}
			Mat inner_glass_mask(roi3.rows, roi3.cols, CV_8UC1, Scalar(0));   //�˹�Ƭmask
			drawContours(inner_glass_mask, contours, -1, Scalar(255), CV_FILLED, 8);

			//imshow("temp_" + Int_to_String(i + 1), thresroi3);
			//��ȷ��λ�����˹�Ƭ
			Mat temp = roi1.clone();
			int width = tr.x - tl.x;	//ͼ���ڲ�С���εĿ��
			int height = bl.y - tl.y;	//ͼ���ڲ�С���εĸ߶�
			double width_ratio = image_size[0] / image_size[2];		//��С���εĿ�ȱ�
			double height_ratio = image_size[1] / image_size[3];	//��С���εĸ߶ȱ�
			int newwidth = width * width_ratio;		//�����ͼ���ⲿ����εĿ�ȣ�������ȡ�������˹�Ƭ
			int newheight = height * height_ratio;	//�����ͼ���ⲿ����εĸ߶ȣ�������ȡ�������˹�Ƭ
			silkPrint_show_list.push_back(temp(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight)).clone());

			//�ھ����ĸ����ϻ���ɫ���Σ��������ⲻ����������У����ƽ˿ӡ4 5 6.bmp��
			//int w = 60, scale = 15;
			//rectangle(temp, Rect(Point(tl.x - scale, tl.y - scale), Point(tl.x + w, tl.y + w)), Scalar(0), CV_FILLED, 8);
			//rectangle(temp, Rect(Point(bl.x - scale, bl.y - w), Point(bl.x + w, bl.y + scale)), Scalar(0), CV_FILLED, 8);
			//rectangle(temp, Rect(Point(tr.x - w, tr.y - scale), Point(tr.x + scale, tr.y + w)), Scalar(0), CV_FILLED, 8);
			//rectangle(temp, Rect(Point(br.x - w, br.y - w), Point(br.x + scale, br.y + scale)), Scalar(0), CV_FILLED, 8);


			Mat new_glass_outer = inner_glass_mask(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight));
			findContours(new_glass_outer, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			vector<Point> silkprint_inner_contour = contours[0];	//��¼�ڲ�������������˿ӡ�ڲ�������

			Mat element = getStructuringElement(MORPH_RECT, Size(10, 10));
			dilate(new_glass_outer, new_glass_outer, element, Point(-1, -1), 1);
			Mat whole = temp(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight));	//��ȡ�������˹�Ƭ
			Mat thres;
			IplImage ipl = (IplImage)whole;
			int th = Otsu(&ipl);
			threshold(whole, thres, th * 17 / 19, 255.0, CV_THRESH_BINARY);    //��ֵ��
			subtract(thres, new_glass_outer, thres);
			element = getStructuringElement(MORPH_RECT, Size(9, 9));
			morphologyEx(thres, thres, MORPH_OPEN, element);	//������������С��������
			element = getStructuringElement(MORPH_RECT, Size(25, 25));
			morphologyEx(thres, thres, MORPH_CLOSE, element);	//�ղ������������ȵĹ�����
			findContours(thres, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			vector<Point> silkprint_outer_contour = contours[1];	//��¼˿ӡ�ⲿ����


			vector<vector<Point>> sk_contours;	//��¼˿ӡ��������
			sk_contours.push_back(silkprint_inner_contour);
			sk_contours.push_back(silkprint_outer_contour);
			//cout << th << endl;
			//threshold(whole, thres, th * 23 / 19, 255.0, CV_THRESH_BINARY);    //��ֵ��                           
			//drawContours(thres, sk_contours, -1, Scalar(255), 3, 8);
			//rectangle(thres, Rect(Point(0, 0), Point(whole.cols, whole.rows)), Scalar(255), 3, 8);
			//line(thres, Point(0, 0), Point(thres.cols, 0), Scalar(255), 5, 8);
			//line(thres, Point(0, thres.rows), Point(thres.cols, thres.rows), Scalar(255), 15, 8);

			Mat silkprint_mask(thres.rows, thres.cols, CV_8UC1, Scalar(0));   //�˹�Ƭmask
			drawContours(silkprint_mask, sk_contours, 1, Scalar(255), CV_FILLED, 8);
			drawContours(silkprint_mask, sk_contours, 0, Scalar(0), CV_FILLED, 8);
			drawContours(silkprint_mask, sk_contours, 1, Scalar(255), 2, 8);
			drawContours(silkprint_mask, sk_contours, 0, Scalar(0), 2, 8);
			Mat glass_mask;
			bitwise_not(silkprint_mask, glass_mask);
			//Mat glass;
			////erode(glass_mask, glass_mask, Mat(5, 5, CV_8U), Point(-1, -1), 2);
			//whole.copyTo(glass, glass_mask);

			Mat silkprint = whole + glass_mask;
			element = getStructuringElement(MORPH_RECT, Size(5, 5));
			morphologyEx(silkprint, silkprint, MORPH_CLOSE, element);
			ipl = (IplImage)silkprint;
			th = Otsu(&ipl);
			threshold(silkprint, silkprint, th * 19 / 29, 255.0, CV_THRESH_BINARY);    //��ֵ��

			silkprint_list.push_back(silkprint);


			drawContours(silkprint_mask, sk_contours, 1, Scalar(255), CV_FILLED, 8);
			Mat outer_glass_mask;
			bitwise_not(silkprint_mask, outer_glass_mask);
			Mat glass;
			whole.copyTo(glass, outer_glass_mask);
			//imshow("show1_" + Int_to_String(i + 1), roi3(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight)));
			glass = glass + roi3(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight));
			//imshow("show2_" + Int_to_String(i + 1), glass);
			glass_list.push_back(glass);
			//subtract(whole, thres, thres);
			//threshold(thres, thres, 0, 255.0, CV_THRESH_BINARY | CV_THRESH_OTSU);    //��ֵ��
			//Canny(thres, thres, 50, 80);
			//findContours(thres, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			//drawContours(whole, contours, -1, Scalar(0), 3, 8);

			//vector<int> ll;
			//for (int x = whole.cols/4; x < whole.cols/4*3; x++) {
			//	int value = whole.at<uchar>(40, x);
			//	ll.push_back(value);
			//}
			//int modeNumber = getModeNumber(ll);

			/*----------------------------------------------------------------*/
			/*-------------------------�˹�Ƭ��ȡģ��-------------------------*/
			/*----------------------------------------------------------------*/
			//Mat glass;  //���ڴ洢��ȡ�����˹�Ƭ���沿��
			//roi3(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight)).copyTo(glass, inner_glass_mask);  //��ȡ���˹�Ƭ����
			//glass_list.push_back(glass);

			/*----------------------------------------------------------------*/
			/*--------------------------˿ӡ��ȡģ��--------------------------*/
			/*----------------------------------------------------------------*/

			//findContours(inner_glass_mask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			//Mat glass_thres;
			//threshold(glass, glass_thres, 0, 255.0, CV_THRESH_BINARY);

			//ipl = (IplImage)whole;
			//th = Otsu(&ipl);
			//cout << th << endl;
			//drawContours(whole, contours, -1, Scalar(0), 15, 8);
			//threshold(whole, whole, th * 10 / 29, 255, CV_THRESH_BINARY);
			////Canny(whole, whole, 100, 100);

			////threshold(whole, whole, 50, 255.0, CV_THRESH_BINARY);
			//Mat silkprint; //���ڴ洢��ȡ����˿ӡ����

			////dilate(glass_thres, glass_thres, Mat(5, 5, CV_8U), Point(-1, -1), 2);
			//silkprint = whole + glass_thres;//ͨ�����ֵ���ľ��沿����ӣ�����ȡ��˿ӡ����
			////imshow("temp_" + Int_to_String(i + 1), whole);
			////silkprint = silkprint - glass_thres;
			//element = getStructuringElement(MORPH_RECT, Size(3, 3));
			//morphologyEx(silkprint, silkprint, MORPH_OPEN, element);
			////element = getStructuringElement(MORPH_RECT, Size(3, 3));
			////erode(silkprint, silkprint, Mat(5, 5, CV_8U), Point(-1, -1), 1);
			//dilate(silkprint, silkprint, element, Point(-1, -1), 1);
			//silkprint_list.push_back(silkprint);
		}
		else
		{
			Mat dsc = Mat::zeros(1, 1, CV_16U);
			silkprint_list.push_back(dsc);
			glass_list.push_back(dsc);
			silkPrint_show_list.push_back(dsc);
		}
	}
}


vector<string> pathGet(string filepath, string postFix)
{
	_finddata_t fileDir;
	intptr_t lfDir; //long lfDir����
	vector<string> testpath;
	char filename[128];
	postFix = "\\*." + postFix;
	strcpy_s(filename, filepath.c_str());
	strcat_s(filename, postFix.c_str());
	if ((lfDir = _findfirst(filename, &fileDir)) == -1l)
		printf("No file is found\n");
	else {
		do {
			char name[256];
			strcpy_s(name, filepath.c_str());
			strcat_s(name, "\\");
			strcat_s(name, fileDir.name);
			testpath.push_back(name);
		} while (_findnext(lfDir, &fileDir) == 0);
	}
	_findclose(lfDir);
	return testpath;
}

float RotatedDegree(Mat rotaImg)
{
	Mat glass_thres;
	threshold(rotaImg, glass_thres, 25, 255.0, CV_THRESH_BINARY);

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Point2f center; float radius;
	Mat ts = Mat::zeros(glass_thres.rows, glass_thres.cols, glass_thres.type());
	Mat tt = Mat::zeros(glass_thres.rows, glass_thres.cols, glass_thres.type());
	//imwrite("my_show.jpg", glass_thres);

	findContours(glass_thres, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	int max_value = 0; vector<Point> max_contour;
	//����ͰѴ�������ȡ�����ˣ���������������Ǹ����ĵ�
	for (int index = 0; index < contours.size(); index++)
	{
		if (contours[index].size()>max_value)
		{
			max_value = contours[index].size();
			max_contour = contours[index];
		}

		drawContours(tt, contours, index, Scalar(100), 1, 8);
	}

	minEnclosingCircle(max_contour, center, radius);
	//circle(tt, center, radius, Scalar(255), 2, 8);

	//ȡ����м����������Ƕ�

	Point2f Line1_a; Point2f Line1_b;

	line(ts, Point(center.x - 120, 0), Point(center.x - 120, radius), Scalar(150), 2, CV_AA);
	line(ts, Point(center.x + 120, 0), Point(center.x + 120, radius), Scalar(150), 2, CV_AA);


	Mat Maps;
	threshold(tt + ts, Maps, 200, 255.0, CV_THRESH_BINARY);

	vector<vector<Point>> MapContours;
	vector<Vec4i> MapHierarchy;
	vector<Point2f> Calc_points;
	findContours(Maps, MapContours, MapHierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	for (int ins = 0; ins < MapContours.size(); ins++)
	{
		minEnclosingCircle(MapContours[ins], center, radius);
		Calc_points.push_back(center);
	}
	Line1_a = Calc_points.at(0);
	Line1_b = Calc_points.at(1);
	float k = (Line1_a.y - Line1_b.y) / (Line1_a.x - Line1_b.x); //��ʱ��ת
	float ks = atan(k) * 180 / 3.14;
	circle(tt, Line1_b, 5, Scalar(120), 2, 8);
	circle(tt, Line1_a, 5, Scalar(255), 2, 8);
	//imwrite("ss.jpg", tt);
	//imwrite("contours.jpg", Maps);
	//waitKey();
	return ks;
}

vector<vector<double>> getImageSize(string type) {
	/*********************************************************************************************************/
	/*	num				--		image_size[0]	,	image_size[1]	,	image_size[2]	,	image_size[3]
	/*											����ƽ˿ӡ��
	/*	0(1 2 3)		--		728,563,576,435
	/*	1(4 5 6)		--		660,810,576,720
	/*	2(7 8 9)		--		576,714,456,594
	/*	3(10 11 12)		--		600,732,456,606
	/*	4(13 14 15)		--		762,636,642,492
	/*	5(16 17 18)		--		642,576,648,348
	/*	6(19 20 21)		--		666,744,498,630
	/*
	/*											����ƽ��˿ӡ��
	/*	0(1 2 3)		--		628,727,528,660
	/*	1(4 5 6)		--		648,720,534,654
	/*	2(7 8 9)		--		738,594,582,486
	/*	3(10 11 12)		--		720,594,600,468
	/*	4(13 14 15)		--		720,588,672,522
	/*	5(16 17 18)		--		720,606,660,528
	/*	6(19 20 21)		--		725,588,672,528
	/*
	/*	image_size[0]--�ⲿ�����ʵ�ʲ����Ŀ�ȣ���Ҫ�˹������ò�����ֵ
	/*	image_size[1]--�ⲿ�����ʵ�ʲ����ĸ߶ȣ���Ҫ�˹������ò�����ֵ
	/*	image_size[2]--�ڲ�С����ʵ�ʲ����Ŀ�ȣ���Ҫ�˹������ò�����ֵ
	/*	image_size[3]--�ڲ�С����ʵ�ʲ����ĸ߶ȣ���Ҫ�˹������ò�����ֵ
	/*********************************************************************************************************/
	vector<vector<double>> image_sizes;
	if (type == "Flat_Sprinting") {
		image_sizes.push_back({ 24.77, 19.62, 20.25, 15.35 });
		image_sizes.push_back({ 21.89, 27.54, 20.14, 25.71 });
		image_sizes.push_back({ 18.68, 23.71, 16.01, 21.04 });
		image_sizes.push_back({ 20.69, 25.50, 16.24, 21.24 });
		image_sizes.push_back({ 25.99, 20.71, 22.88, 17.51 });
		image_sizes.push_back({ 21.12, 19.18, 16.54, 12.80 });
		image_sizes.push_back({ 22.68, 25.37, 17.08, 22.46 });
	}
	else if (type == "Flat_NoSprinting") {
		image_sizes.push_back({ 628,727,528,660 });
		image_sizes.push_back({ 648,720,534,654 });
		image_sizes.push_back({ 25.14, 20.53, 23.44, 18.79 });
		image_sizes.push_back({ 25.14, 20.48, 23.38, 18.76 });
		image_sizes.push_back({ 25.27, 20.34, 23.41, 18.86 });
		image_sizes.push_back({ 720,606,660,528 });
		image_sizes.push_back({ 725,588,672,528 });
	}
	else if (type == "Thread") {
		image_sizes.push_back({ 0, 0, 0, 0 });
		image_sizes.push_back({ 0, 0, 0, 0 });
		image_sizes.push_back({ 15.02, 11.61, 13.41, 10.58 });
		image_sizes.push_back({ 0, 0, 0, 0 });
	}
	else if (type == "New") {
		image_sizes.push_back({ 750,618,636,498 });
		image_sizes.push_back({ 762,594,642,480 });
		image_sizes.push_back({ 768,612,648,480 });
		image_sizes.push_back({ 768,624,642,480 });
		image_sizes.push_back({ 756,600,648,486 });
		image_sizes.push_back({ 762,618,648,480 });
		image_sizes.push_back({ 762,612,636,486 });
		image_sizes.push_back({ 756,618,648,486 });
		image_sizes.push_back({ 750,612,648,486 });
		image_sizes.push_back({ 774,606,654,486 });
		image_sizes.push_back({ 558,714,498,624 });
		image_sizes.push_back({ 558,702,486,624 });
		image_sizes.push_back({ 564,672,504,630 });
		image_sizes.push_back({ 570,684,498,624 });
		image_sizes.push_back({ 558,690,492,624 });
		image_sizes.push_back({ 756,582,576,408 });
		image_sizes.push_back({ 564,672,486,618 });
		image_sizes.push_back({ 768,588,558,402 });
		image_sizes.push_back({ 570,684,486,618 });
		image_sizes.push_back({ 552,684,492,624 });
		image_sizes.push_back({ 552,708,492,630 });
		image_sizes.push_back({ 558,702,486,630 });
		image_sizes.push_back({ 564,684,504,630 });
		image_sizes.push_back({ 516,528,498,624 });
		image_sizes.push_back({ 750,600,570,432 });
		image_sizes.push_back({ 606,732,534,654 });
		image_sizes.push_back({ 606,720,552,654 });
		image_sizes.push_back({ 588,714,546,660 });
		image_sizes.push_back({ 606,714,534,684 });
		image_sizes.push_back({ 744,600,672,522 });
		image_sizes.push_back({ 732,594,672,534 });
		image_sizes.push_back({ 720,606,672,516 });
		image_sizes.push_back({ 726,582,678,534 });
		image_sizes.push_back({ 726,576,678,528 });
		image_sizes.push_back({ 732,606,654,540 });
		image_sizes.push_back({ 732,588,678,528 });
		image_sizes.push_back({ 726,588,666,534 });
	}

	return image_sizes;
}

//�����ⲿ��
void glassDetect(vector<Mat> glass_list, int radiusThres, int contourAreaThres,  vector<int>OutGlassed)
{
	for (int i = 0; i < glass_list.size(); i++){
		if (OutGlassed[i] == 1) {
			Mat result = glass_list[i].clone();	//��ʾ�����
			Mat tt = glass_list[i].clone();	//Ԥ����
			//imshow("src_" + to_string(i + 1), tt);
			//ͨ����̬ѧ�ղ�����ȡ�����Ե����ֵ����ȡ������ԭͼ��ӣ�ʹ���Ե������ȱ�ݶ�������
			vector<vector<Point>> contours;
			vector<Vec4i> hierarchy;
			Mat element = getStructuringElement(MORPH_RECT, Size(30, 30));
			morphologyEx(tt, tt, MORPH_CLOSE, element);
			threshold(tt, tt, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
			bitwise_not(tt, tt);
			element = getStructuringElement(MORPH_RECT, Size(9, 9));
			dilate(tt, tt, element);
			imshow("result_" + to_string(i + 1), tt);
			Mat addResult;
			add(tt, glass_list[i].clone(), addResult);

			//��ֵ������б�Ե��Ⲣ��ȡ������ͨ�����������ԣ���С���Բ�İ뾶��������������ж�ȱ��
			GaussianBlur(addResult, addResult, Size(3, 3), 1);   //��˹�˲�
			element = getStructuringElement(MORPH_RECT, Size(3, 3));
			erode(addResult, addResult, element);
			IplImage ipl = (IplImage)addResult;
			int th = Otsu(&ipl);
			threshold(addResult, addResult, th*17/19, 255, CV_THRESH_BINARY);
			rectangle(addResult, Rect(Point(0, 0), Point(addResult.cols - 1, addResult.rows - 1)), Scalar(255), 3);
			//imshow("sharp_" + to_string(i + 1), addResult);
			Canny(addResult, addResult, 50, 80);	//Canny��������ֵ��Ҫ����
			findContours(addResult, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			//�����������ж�ȱ��
			int defect_flag = 0;	//ȱ�ݱ�־λ��0--��ȱ�ݣ�1--��ȱ��
			for (int index = 0; index < contours.size(); index++) {
				Point2f center;
				float radius;
				minEnclosingCircle(contours[index], center, radius);	//������������С���Բ
				if (radius > radiusThres && radius < 100) {	//����С���Բ�İ뾶����"radiusThres"�����ж�Ϊȱ��
					circle(result, center, radius, Scalar(255), 2, 8);
					defect_flag = 1;
				}
				int area = contourArea(contours[index]);	//�����������
				if (area > contourAreaThres && area < 50000 && radius < 100) {	//�������������"contourAreaThres"�����ж�Ϊȱ��
					drawContours(result, contours, index, Scalar(255), 2, 8);
					defect_flag = 1;
				}
			}
			//putText(result, Int_to_String(i + 1), Point(tt.cols / 2, tt.rows / 2), FONT_HERSHEY_PLAIN, 5, Scalar(100), 5, 8);	//���Ƴ���ǰλ���˹�Ƭ�ı��
			imshow("glass_" + to_string(i + 1), result);
			if (defect_flag == 1) {
				cout << "��<" << to_string(i + 1) << ">λ�þ�����ȱ��" << endl;
				//imshow("glass_" + to_string(i + 1), result);
			}
		}
	}
}

//���˿ӡ�Ĳ���
int silkprintDetect(vector<Mat> silkprint_list, int radiusThres_silkprint, int contourAreaThres_silkprint, vector<Mat> &show_list, vector<int>OutGlassed)
{
	Mat silkprint_temp;
	int defect_flag = 0;
	vector<int>areas; int area_temp;
	for (int i = 0; i < silkprint_list.size(); i++)
	{
		if (OutGlassed[i] != 0)
		{
			cout << "i = " << i + 1 << endl;
			//imwrite("A" + Int_to_String(i + 1) + ".jpg", silkprint_list[i]);
			vector<vector<Point>> contours; Point2f Tem_center; float Tem_radius;
			vector<Vec4i> hierarchy;
			Mat silkprint_temp = silkprint_list[i].clone();
			Mat show_temp = show_list[i].clone();
			rectangle(silkprint_temp, Rect(Point(0, 0), Point(silkprint_temp.cols - 1, silkprint_temp.rows - 1)), Scalar(0), 3);
			//imshow("src_" + to_string(i + 1), silkprint_temp);
			//imwrite("show.jpg", show_list[i]);
			Mat tt = Mat::zeros(silkprint_temp.rows, silkprint_temp.cols, silkprint_temp.type());
			//Canny(silkprint_temp, silkprint_temp, 10, 10);
			findContours(silkprint_temp, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			imshow("temp_" + Int_to_String(i + 1), silkprint_temp);
			for (int index = 0; index < contours.size(); index++)
			{
				Point2f center;
				float radius;
				minEnclosingCircle(contours[index], center, radius);	//������������С���Բ
				int area = contourArea(contours[index]);	//�����������
				int scale = 3;
				if (center.x > scale && center.x < show_temp.cols - scale && center.y > scale && center.y < show_temp.rows - scale && area < show_temp.cols * show_temp.rows / 2 && radius < min(show_temp.cols, show_temp.rows) / 2) {	//���˵�̫��Ե������
					cout << "radius -- " << radius << "\t\tarea -- " << area << endl;
					if (radius >= radiusThres_silkprint || area >= contourAreaThres_silkprint) {	//����С���Բ�İ뾶����"radiusThres",�������������"contourAreaThres"�����ж�Ϊȱ��
						//cout << "radius -- " << radius << endl;
						circle(show_temp, center, radius, Scalar(255), 2, 8);
						drawContours(show_temp, contours, index, Scalar(255), 2, 8);
						defect_flag = 1;
					}
				}
			}
			imshow("B" + Int_to_String(i + 1) + ".jpg", show_temp);
			if (defect_flag)
			{
				//imshow("B" + Int_to_String(i + 1) + ".jpg", show_temp);
				cout << "��<" << to_string(i + 1) << ">λ��˿ӡ��ȱ��" << endl;
			}
			defect_flag = 0;
		}
	}

	return 0;
}

int silkprintDetect2(vector<Mat> silkprint_list, int radiusThres_silkprint, int contourAreaThres_silkprint, vector<Mat> &show_list, vector<int>OutGlassed)
{
	for (int i = 0; i < silkprint_list.size(); i++) {
		if (OutGlassed[i] == 1) {
			Mat show_temp = show_list[i].clone();
			//imshow("src_" + to_string(i + 1), silkprint_list[i]);
			Mat silkprint = silkprint_list[i].clone();
			//imshow("src_" + to_string(i + 1), silkprint);
			IplImage ipl = (IplImage)silkprint;
			int th = Otsu(&ipl);
			threshold(silkprint, silkprint, th*55/50, 255, CV_THRESH_BINARY);
			//Canny(silkprint, silkprint, 90, 90);	//Canny��������ֵ��Ҫ����
			Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
			morphologyEx(silkprint, silkprint, MORPH_CLOSE, element);
			imshow("Threshold_" + to_string(i + 1), silkprint);
			element = getStructuringElement(MORPH_RECT, Size(2, 2));
			dilate(silkprint, silkprint, element);

			vector<vector<Point>> contours;
			vector<Vec4i> hierarchy;
			findContours(silkprint, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			//cout << "i = " << i + 1 << endl;
			//�����������ж�ȱ��
			int defect_flag = 0;	//ȱ�ݱ�־λ��0--��ȱ�ݣ�1--��ȱ��
			for (int index = 0; index < contours.size(); index++) {
				Point2f center;
				float radius;
				minEnclosingCircle(contours[index], center, radius);	//������������С���Բ
				if (radius >= radiusThres_silkprint && radius < 100) {	//����С���Բ�İ뾶����"radiusThres"�����ж�Ϊȱ��
																		//cout << "radius -- " << radius << endl;
					circle(show_temp, center, radius, Scalar(255), 2, 8);
					defect_flag = 1;
				}
				int area = contourArea(contours[index]);	//�����������
				if (area >= contourAreaThres_silkprint && area < 50000 && radius < 100) {	//�������������"contourAreaThres"�����ж�Ϊȱ��
																							//cout << "area -- " << area << endl;
					drawContours(show_temp, contours, index, Scalar(255), 2, 8);
					defect_flag = 1;
				}
			}
			imshow("silkprint_" + to_string(i + 1), show_temp);
			if (defect_flag == 1) {
				cout << "��<" << to_string(i + 1) << ">λ��˿ӡ��ȱ��" << endl;
				//imshow("silkprint_" + to_string(i + 1), show_temp);
			}
		}
	}

	return 0;
}




