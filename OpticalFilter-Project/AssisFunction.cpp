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


vector<vector<double>> getImageSize(string type) {
	/*********************************************************************************************************/
	/*	num				--		image_size[0]	,	image_size[1]	,	image_size[2]	,	image_size[3]
	/*											（扁平丝印）
	/*	0(1 2 3)		--		728,563,576,435
	/*	1(4 5 6)		--		660,810,576,720
	/*	2(7 8 9)		--		576,714,456,594
	/*	3(10 11 12)		--		600,732,456,606
	/*	4(13 14 15)		--		762,636,642,492
	/*	5(16 17 18)		--		642,576,648,348
	/*	6(19 20 21)		--		666,744,498,630
	/*
	/*											（扁平非丝印）
	/*	0(1 2 3)		--		628,727,528,660
	/*	1(4 5 6)		--		648,720,534,654
	/*	2(7 8 9)		--		738,594,582,486
	/*	3(10 11 12)		--		720,594,600,468
	/*	4(13 14 15)		--		720,588,672,522
	/*	5(16 17 18)		--		720,606,660,528
	/*	6(19 20 21)		--		725,588,672,528
	/*
	/*	image_size[0]--外部大矩形实际测量的宽度，需要人工给出该参数的值
	/*	image_size[1]--外部大矩形实际测量的高度，需要人工给出该参数的值
	/*	image_size[2]--内部小矩形实际测量的宽度，需要人工给出该参数的值
	/*	image_size[3]--内部小矩形实际测量的高度，需要人工给出该参数的值
	/*********************************************************************************************************/
	vector<vector<double>> image_sizes;
	if (type == "Flat_Sprinting") {
		image_sizes.push_back({ 24.77, 19.62, 20.25, 15.35 });
		image_sizes.push_back({ 21.89, 27.54, 20.14, 25.71 });
		image_sizes.push_back({ 18.68, 23.71, 16.01, 21.04 });
		image_sizes.push_back({ 20.69, 25.50, 16.24, 21.24 });
		image_sizes.push_back({ 25.99, 20.71, 22.88, 17.51 });
		image_sizes.push_back({ 20.92, 19.06, 16.54, 12.80 });
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

vector<int>GetArea(Mat img, int item_num, vector<Point2f>&mycenter, float&myradius)
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

	findContours(edge, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);//这东西好像会直接搞坏掉原图
	int index = 0; int comTemp = 10000000000; int Areacount = 0;
	int the_num = Get6th(contours, item_num);
	for (; index >= 0; index = hierarchy[index][0])
	{
		Scalar color(255);
		int xx = arcLength(contours[index], true);
		if (arcLength(contours[index], true) >= the_num)
		{

			approxPolyDP(Mat(contours[index]), contours_poly[Areacount], 3, true);//逼近曲线，应该要调整
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

int Get6th(vector<vector<Point>> contours, int Area_num)//注意第二个参数不能乱取，一定要少于总轮廓的数量，>0
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

int getModeNumber(vector<int> ll) {
	sort(ll.begin(), ll.end());
	int k = 0;
	int MaxCount = 1;
	int index = 0;
	while (k <= ll.size() - 1) {
		int count = 1;
		int j;
		for (j = k; j < ll.size() - 1; j++) {
			if (ll[j] == ll[j + 1])//存在连续两个数相等，则众数+1  
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
	return ll[index];
}