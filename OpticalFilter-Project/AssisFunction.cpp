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
	stream << n;  //n为int类型
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
		u += i * histogram[i];  //整幅图像的平均灰度
	}

	int threshold;
	float maxVariance = 0;
	float w0 = 0, avgValue = 0;
	for (int i = 0; i < 256; i++)
	{
		w0 += histogram[i];  //假设当前灰度i为阈值, 0~i 灰度像素所占整幅图像的比例即前景比例
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
	intptr_t lfDir; //long lfDir报错
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
	//下面就把大轮廓提取出来了，还有最大轮廓的那个中心点
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

	//取点进行几何运算计算角度

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
	float k = (Line1_a.y - Line1_b.y) / (Line1_a.x - Line1_b.x); //逆时针转
	float ks = atan(k) * 180 / 3.14;
	circle(tt, Line1_b, 5, Scalar(120), 2, 8);
	circle(tt, Line1_a, 5, Scalar(255), 2, 8);
	//imwrite("ss.jpg", tt);
	//imwrite("contours.jpg", Maps);
	//waitKey();
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

vector<int>GetArea(Mat img, int item_num, vector<Point2f>&mycenter,bool&whetherNull, int filterArea)
{
	Mat edge;
	threshold(img, edge, 120, 255, 0);

	Mat dstImg = Mat::zeros(edge.rows, edge.cols, edge.type());
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;



	findContours(edge, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);//这东西好像会直接搞坏掉原图
	Mat drawing;
	dstImg.copyTo(drawing);

	for (int i = 0; i < contours.size(); i++){
		Scalar color = Scalar(255, 255, 255);
		if (contours[i].size()>200)
			drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
	}

	int index = 0; int comTemp = 10000000000; int Areacount = 0; int filterAndnull = 0; 
	int the_num = Get6th(contours, filterAndnull, filterArea);//这个the_num 可以作为以后判断的标志位，如果没有滤光片返回-1，有则返回滤光片数量


	vector <vector<Point>> contours_poly(filterAndnull);
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
		bool flagFilter = xx<filterArea*1.05 && xx>filterArea*0.95;
		if (flagFilter)
		{
			approxPolyDP(Mat(contours[index]), contours_poly[Areacount], 10, true);//逼近曲线，应该要调整
			minEnclosingCircle(contours_poly[Areacount], center[Areacount], radius[Areacount]);
			isGlass[Areacount] = 1;
			flagCount++;
			Areacount++;
		}
	}
		mycenter = center;

		return isGlass;
	}


int Get6th(vector<vector<Point>> contours, int& Filternum, int FilterArea)//注意第二个参数不能乱取，一定要少于总轮廓的数量，>0
{
	int length = contours.size(); vector<int> flag; int frontFlag = 1;
	vector<int> idx; int FilterCount = 0; int count=0;
	int front=1;
	int next = 1;

	double g_dConLength;
	for (int i = 0; i < length; i++)
	{
		g_dConLength = contourArea(contours[i]);
		idx.push_back(g_dConLength);
	}
	sort(idx.begin(), idx.end());

	for (int index = 1; index <=length; index++)
	{
		bool flagFilter = idx.at(length - index)<FilterArea*1.05 && idx.at(length - index)>FilterArea*0.95;
		if (flagFilter)
		{
			FilterCount++;
		}
		if (idx.at(length - index) < FilterArea*0.8)
		{
			break;
		}
		count++;
	}
	Filternum = FilterCount;
	if (FilterCount == 0)
	{
		return -1;
	}
	return idx.at(length - FilterCount);
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
			if (list[j] == list[j + 1])//存在连续两个数相等，则众数+1  
				count++;
			else
				break;
		}
		if (MaxCount < count) {
			MaxCount = count;//当前最大众数  
			index = j;//当前众数标记位置  
		}
		++j;
		k = j;//位置后移到下一个未出现的数字  
	}
	return list[index];
}