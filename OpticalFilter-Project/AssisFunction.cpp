#include "AssisFunction.h"

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

vector<string> pathGet(string filepath, string postFix)
{
	_finddata_t fileDir;
	intptr_t lfDir; //long lfDir����
	vector<string> testpath;
	char filename[128];
	postFix = "\\*." + postFix;
	strcpy_s(filename, filepath.c_str());
	strcat_s(filename, postFix.c_str());
	if ((lfDir = _findfirst(filename, &fileDir)) == -1l);
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
	threshold(rotaImg, glass_thres, 0, 255.0, CV_THRESH_BINARY | CV_THRESH_OTSU);

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

	return ks;
}

vector<Point2f>sortCenterpoint(vector<Point2f> centers, int middle, vector<int>isGlassed, vector<int>&OutGlassed)
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

	for (int i = 0; i < vectB.size(); i++)
	{
		new_center.push_back(Point2f(vectB[i].first, vectB[i].second));
		NewisGlassed.push_back(vectB[i].third);
	}

	for (int i = 0; i < vectA.size(); i++)
	{
		new_center.push_back(Point2f(vectA[i].first, vectA[i].second));
		NewisGlassed.push_back(vectA[i].third);
	}

	OutGlassed = NewisGlassed;
	return new_center;
}

vector<int>GetArea(Mat img, int item_num, vector<Point2f>&mycenter,bool&whetherNull, int filterArea,double ratio)
{
	Mat edge;
	threshold(img, edge, 120, 255, 0);

	Mat dstImg = Mat::zeros(edge.rows, edge.cols, edge.type());
	imwrite("ss.jpg",img);
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;



	findContours(edge, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);//�ⶫ�������ֱ�Ӹ㻵��ԭͼ
	Mat drawing;
	dstImg.copyTo(drawing);

	for (int i = 0; i < contours.size(); i++){
		Scalar color = Scalar(255, 255, 255);
		if (contours[i].size()>200)
			drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
	}

	int index = 0; int comTemp = 10000000000; int Areacount = 0;
	int filterAndnull = Get6th(contours, filterArea, ratio);


	vector<vector<Point>> contours_poly(filterAndnull);
	vector<Point2f>  center(filterAndnull);
	vector<float> radius(filterAndnull);
	vector<int> isGlass(filterAndnull);
	
	if (filterAndnull == 0)
	{
		whetherNull = 1;
		return isGlass;
	}
	else
	{
		whetherNull = 0;
	}
	int flagCount=0;
	for (; index >= 0; index = hierarchy[index][0])
	{
		Scalar color(255);
		int xx = contourArea(contours[index]);
		bool flagFilter = xx<filterArea*1.1 && xx>filterArea*0.9;

		RotatedRect rect = minAreaRect(contours[index]);
		double ratios1 = rect.size.height / rect.size.width;
		double ratios2 = rect.size.width / rect.size.height;
		bool ratio_flag1 = ratios1<ratio*1.1 && ratios1>ratio*0.9;
		bool ratio_flag2 = ratios2<ratio*1.1 && ratios2>ratio*0.9;
		bool ratio_flag = ratio_flag1 || ratio_flag2;

		Mat boundRect;
		if (flagFilter && ratio_flag)
		{
			approxPolyDP(Mat(contours[index]), contours_poly[Areacount], 5, true);//�ƽ����ߣ�Ӧ��Ҫ����
			minEnclosingCircle(contours_poly[Areacount], center[Areacount], radius[Areacount]);
			isGlass[Areacount] = 1;
			flagCount++;
			Areacount++;
		}
	}
	mycenter = center;

	return isGlass;
}


int Get6th(vector<vector<Point>> contours, int FilterArea,double ratio)//ע��ڶ�������������ȡ��һ��Ҫ������������������>0
{
	int length = contours.size(); vector<int> flag;
	vector<int> idx; int FilterCount = 0;
	//Mat boundRect; //�����Σ��ж�ratio
	//int front=1;
	//int next = 1;

	double g_dConLength;
	for (int i = 0; i < length; i++)
	{
		g_dConLength = contourArea(contours[i]);
		idx.push_back(g_dConLength);
	}
	//sort(idx.begin(), idx.end());
	for (int index = 0; index < length; index++)
	{
		RotatedRect rect = minAreaRect(contours[index]);
		double ratios1 = rect.size.height / rect.size.width;
		double ratios2 = rect.size.width / rect.size.height;
		bool ratio_flag1 = ratios1<ratio*1.1 && ratios1>ratio*0.9;
		bool ratio_flag2 = ratios2<ratio*1.1 && ratios2>ratio*0.9;
		bool ratio_flag = ratio_flag1 || ratio_flag2;
		bool flagFilter = idx[index]<FilterArea*1.1 && idx[index]>FilterArea*0.9;
		if (flagFilter && ratio_flag)
		{
			FilterCount++;
		}
	}

	return FilterCount;
}

int getAveragePix(Mat input, int ignore){
	int w = input.cols;
	int h = input.rows;
	int sum = 0, cnt = 0;
	for (size_t x = 0; x < w; x++){
		for (size_t y = 0; y < h; y++){
			int value = int(input.at<uchar>(y, x));
			if (value != ignore){
				sum += value;
				cnt++;
			}
		}
	}
	int result = sum / cnt;
	return result;
}

int getModePix(Mat input, int ignore){
	int w = input.cols;
	int h = input.rows;
	vector<int> list;
	int sum = 0, cnt = 0;
	for (size_t x = 0; x < w; x++){
		for (size_t y = 0; y < h; y++){
			int value = int(input.at<uchar>(y, x));
			if (value != ignore)
				list.push_back(value);
		}
	}
	sort(list.begin(), list.end());
	int k = 0;
	int MaxCount = 1;
	int index = 0;
	while (k <= list.size() - 1) {
		int count = 1;
		int j;
		for (j = k; j < list.size() - 1; j++) {
			if (list[j] == list[j + 1])//����������������ȣ�������+1  
				count++;
			else
				break;
		}
		if (MaxCount < count) {
			MaxCount = count;//��ǰ�������  
			index = j;//��ǰ�������λ��  
		}
		++j;
		k = j;//λ�ú��Ƶ���һ��δ���ֵ�����  
	}
	return list[index];
}

Mat localyuzhi(Mat src, int blocksize, int yuzhi)
{
	Mat dst = Mat::zeros(src.rows, src.cols, src.type());
	double  num = (blocksize * 2 + 1)*(blocksize * 2 + 1), all = 0, n = 0;
	vector<double>avg(src.rows*src.cols);
	for (int j = blocksize; j < src.rows - blocksize; j++)
	{
		for (int i = blocksize; i < src.cols - blocksize; i++)
		{

			for (int pj = j - blocksize; pj <= j + blocksize; pj++)
			{
				for (int pi = i - blocksize; pi <= i + blocksize; pi++)
				{
					all = (all + src.at<uchar>(pj, pi));
				}
			}
			avg[n] = all / num;
			all = 0;
			if (abs(src.at<uchar>(j, i) - avg[n])>yuzhi)
			{
				dst.at<uchar>(j, i) = 255;
			}
			else
			{
				dst.at<uchar>(j, i) = 0;
			}
			n++;
		}
	}
	return dst;

}