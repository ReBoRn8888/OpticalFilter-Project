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

//下面的函数获取到一个目标的三张图的分别的roi区域，存成3*vector的形式
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
		image1(Rect(newcenter.at(i).x - length, newcenter.at(i).y - length, length * 2, length * 2)).copyTo(Six_area1[i]);//第一张图ROI区域的输入
		image2(Rect(newcenter.at(i).x - length, newcenter.at(i).y - length, length * 2, length * 2)).copyTo(Six_area2[i]);//第二张图ROI区域的输入
		image4(Rect(newcenter.at(i).x - length, newcenter.at(i).y - length, length * 2, length * 2)).copyTo(Six_area3[i]);//第三张图ROI区域的输入
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
			if (ll[j] == ll[j + 1])//存在连续两个数相等，则众数+1  
				count++;
			else
				break;
		}
		if (MaxCount < count){
			MaxCount = count;//当前最大众数  
			index = j;//当前众数标记位置  
		}
		++j;
		k = j;//位置后移到下一个未出现的数字  
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
			threshold(roi3, thresroi3, 0, 255.0, CV_THRESH_BINARY | CV_THRESH_OTSU);    //二值化
			vector<vector<Point>> contours;
			vector<Vec4i> hierarchy;
			findContours(thresroi3, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

			//过滤掉小轮廓，取得中间最大的滤光片轮廓
			vector<vector<Point>>::iterator it_contour = contours.begin();
			while (it_contour != contours.end())
			{
				//                cout << contourArea(*it_contour) << endl;
				if (contourArea(*it_contour) < 100000 || contourArea(*it_contour) > 500000)//删去面积小于100000的轮廓
					it_contour = contours.erase(it_contour);
				else
					++it_contour;
			}

			//记录滤光片轮廓左上和右下点的坐标，用于精确定位整块滤光片
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
			Mat glass_mask(roi3.rows, roi3.cols, CV_8UC1, Scalar(0));   //滤光片mask
			drawContours(glass_mask, contours, -1, Scalar(255), CV_FILLED, 8);

			//精确定位整块滤光片
			Mat temp = roi2.clone();
			int width = tr.x - tl.x;	//图上内部小矩形的宽度
			int height = bl.y - tl.y;	//图上内部小矩形的高度
			double width_ratio = image_size[0] / image_size[2];		//大小矩形的宽度比
			double height_ratio = image_size[1] / image_size[3];	//大小矩形的高度比
			int newwidth = width * width_ratio;		//计算出图上外部大矩形的宽度，即可提取出完整滤光片
			int newheight = height * height_ratio;	//计算出图上外部大矩形的高度，即可提取出完整滤光片
			silkPrint_show_list.push_back(temp(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight)).clone());

			//在镜面四个角上画黑色矩形，避免因打光不足而导致误判（如扁平丝印4 5 6.bmp）
			int w = 60, scale = 15;
			rectangle(temp, Rect(Point(tl.x - scale, tl.y - scale), Point(tl.x + w, tl.y + w)), Scalar(0), CV_FILLED, 8);
			rectangle(temp, Rect(Point(bl.x - scale, bl.y - w), Point(bl.x + w, bl.y + scale)), Scalar(0), CV_FILLED, 8);
			rectangle(temp, Rect(Point(tr.x - w, tr.y - scale), Point(tr.x + scale, tr.y + w)), Scalar(0), CV_FILLED, 8);
			rectangle(temp, Rect(Point(br.x - w, br.y - w), Point(br.x + scale, br.y + scale)), Scalar(0), CV_FILLED, 8);

			Mat whole = temp(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight));	//提取出整块滤光片
			glass_mask = glass_mask(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight));	//调整glass_mask尺寸

			/*----------------------------------------------------------------*/
			/*-------------------------滤光片提取模块-------------------------*/
			/*----------------------------------------------------------------*/
			Mat glass;  //用于存储提取出的滤光片镜面部分
			roi3(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight)).copyTo(glass, glass_mask);  //提取出滤光片镜面
			glass_list.push_back(glass);

			/*----------------------------------------------------------------*/
			/*--------------------------丝印提取模块--------------------------*/
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
			Mat silkprint; //用于存储提取出的丝印部分

			//dilate(glass_thres, glass_thres, Mat(5, 5, CV_8U), Point(-1, -1), 2);
			silkprint = whole + glass_thres;//通过与二值化的镜面部分相加，来提取出丝印部分
			//imshow("temp_" + Int_to_String(i + 1), whole);
			//silkprint = silkprint - glass_thres;
			Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
			morphologyEx(silkprint, silkprint, MORPH_OPEN, element);
			//element = getStructuringElement(MORPH_RECT, Size(3, 3));
			//erode(silkprint, silkprint, Mat(5, 5, CV_8U), Point(-1, -1), 1);
			dilate(silkprint, silkprint, element, Point(-1, -1), 1);
			silkprint_list.push_back(silkprint);

			//Mat silkprint_mask;
			//bitwise_not(glass_mask, silkprint_mask);//将镜面mask按位取反，得到丝印mask
			//Mat element = getStructuringElement(MORPH_RECT, Size(20, 20));
			//erode(silkprint_mask, silkprint_mask, element);//腐蚀操作，消除丝印内部边缘的白边

			////计算众数，用于填充除丝印外的其他部分，便于OTSU的二值化操作
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
			//		if (ll[j] == ll[j + 1])//存在连续两个数相等，则众数+1  
			//			count++;
			//		else
			//			break;
			//	}
			//	if (MaxCount < count){
			//		MaxCount = count;//当前最大众数  
			//		index = j;//当前众数标记位置  
			//	}
			//	++j;
			//	k = j;//位置后移到下一个未出现的数字  
			//}
			//Mat gray_mask;
			//Mat gray(glass_mask.rows, glass_mask.cols, CV_8UC1, Scalar(ll[index]));
			//Mat tempmask;
			//bitwise_not(silkprint_mask, tempmask);
			//gray.copyTo(gray_mask, tempmask);

			//Mat silkprint;
			//whole.copyTo(silkprint, silkprint_mask);	//提取出丝印部分
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
			threshold(roi3, thresroi3, 0, 255.0, CV_THRESH_BINARY | CV_THRESH_OTSU);    //二值化
			vector<vector<Point>> contours;
			vector<Vec4i> hierarchy;
			findContours(thresroi3, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

			//过滤掉小轮廓，取得中间最大的滤光片轮廓
			vector<vector<Point>>::iterator it_contour = contours.begin();
			while (it_contour != contours.end())
			{
				//                cout << contourArea(*it_contour) << endl;
				if (contourArea(*it_contour) < 100000 || contourArea(*it_contour) > 500000)//删去面积小于100000的轮廓
					it_contour = contours.erase(it_contour);
				else
					++it_contour;
			}

			//记录滤光片轮廓左上和右下点的坐标，用于精确定位整块滤光片
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
			Mat inner_glass_mask(roi3.rows, roi3.cols, CV_8UC1, Scalar(0));   //滤光片mask
			drawContours(inner_glass_mask, contours, -1, Scalar(255), CV_FILLED, 8);

			//imshow("temp_" + Int_to_String(i + 1), thresroi3);
			//精确定位整块滤光片
			Mat temp = roi1.clone();
			int width = tr.x - tl.x;	//图上内部小矩形的宽度
			int height = bl.y - tl.y;	//图上内部小矩形的高度
			double width_ratio = image_size[0] / image_size[2];		//大小矩形的宽度比
			double height_ratio = image_size[1] / image_size[3];	//大小矩形的高度比
			int newwidth = width * width_ratio;		//计算出图上外部大矩形的宽度，即可提取出完整滤光片
			int newheight = height * height_ratio;	//计算出图上外部大矩形的高度，即可提取出完整滤光片
			silkPrint_show_list.push_back(temp(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight)).clone());

			//在镜面四个角上画黑色矩形，避免因打光不足而导致误判（如扁平丝印4 5 6.bmp）
			//int w = 60, scale = 15;
			//rectangle(temp, Rect(Point(tl.x - scale, tl.y - scale), Point(tl.x + w, tl.y + w)), Scalar(0), CV_FILLED, 8);
			//rectangle(temp, Rect(Point(bl.x - scale, bl.y - w), Point(bl.x + w, bl.y + scale)), Scalar(0), CV_FILLED, 8);
			//rectangle(temp, Rect(Point(tr.x - w, tr.y - scale), Point(tr.x + scale, tr.y + w)), Scalar(0), CV_FILLED, 8);
			//rectangle(temp, Rect(Point(br.x - w, br.y - w), Point(br.x + scale, br.y + scale)), Scalar(0), CV_FILLED, 8);


			Mat new_glass_outer = inner_glass_mask(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight));
			findContours(new_glass_outer, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			vector<Point> silkprint_inner_contour = contours[0];	//记录内部镜面轮廓（即丝印内部轮廓）

			Mat element = getStructuringElement(MORPH_RECT, Size(10, 10));
			dilate(new_glass_outer, new_glass_outer, element, Point(-1, -1), 1);
			Mat whole = temp(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight));	//提取出整块滤光片
			Mat thres;
			IplImage ipl = (IplImage)whole;
			int th = Otsu(&ipl);
			threshold(whole, thres, th * 17 / 19, 255.0, CV_THRESH_BINARY);    //二值化
			subtract(thres, new_glass_outer, thres);
			element = getStructuringElement(MORPH_RECT, Size(9, 9));
			morphologyEx(thres, thres, MORPH_OPEN, element);	//开操作消除较小明亮区域
			element = getStructuringElement(MORPH_RECT, Size(25, 25));
			morphologyEx(thres, thres, MORPH_CLOSE, element);	//闭操作消除低亮度的孤立点
			findContours(thres, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			vector<Point> silkprint_outer_contour = contours[1];	//记录丝印外部轮廓


			vector<vector<Point>> sk_contours;	//记录丝印内外轮廓
			sk_contours.push_back(silkprint_inner_contour);
			sk_contours.push_back(silkprint_outer_contour);
			//cout << th << endl;
			//threshold(whole, thres, th * 23 / 19, 255.0, CV_THRESH_BINARY);    //二值化                           
			//drawContours(thres, sk_contours, -1, Scalar(255), 3, 8);
			//rectangle(thres, Rect(Point(0, 0), Point(whole.cols, whole.rows)), Scalar(255), 3, 8);
			//line(thres, Point(0, 0), Point(thres.cols, 0), Scalar(255), 5, 8);
			//line(thres, Point(0, thres.rows), Point(thres.cols, thres.rows), Scalar(255), 15, 8);

			Mat silkprint_mask(thres.rows, thres.cols, CV_8UC1, Scalar(0));   //滤光片mask
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
			threshold(silkprint, silkprint, th * 19 / 29, 255.0, CV_THRESH_BINARY);    //二值化

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
			//threshold(thres, thres, 0, 255.0, CV_THRESH_BINARY | CV_THRESH_OTSU);    //二值化
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
			/*-------------------------滤光片提取模块-------------------------*/
			/*----------------------------------------------------------------*/
			//Mat glass;  //用于存储提取出的滤光片镜面部分
			//roi3(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight)).copyTo(glass, inner_glass_mask);  //提取出滤光片镜面
			//glass_list.push_back(glass);

			/*----------------------------------------------------------------*/
			/*--------------------------丝印提取模块--------------------------*/
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
			//Mat silkprint; //用于存储提取出的丝印部分

			////dilate(glass_thres, glass_thres, Mat(5, 5, CV_8U), Point(-1, -1), 2);
			//silkprint = whole + glass_thres;//通过与二值化的镜面部分相加，来提取出丝印部分
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

//镜面检测部分
void glassDetect(vector<Mat> glass_list, int radiusThres, int contourAreaThres,  vector<int>OutGlassed)
{
	for (int i = 0; i < glass_list.size(); i++){
		if (OutGlassed[i] == 1) {
			Mat result = glass_list[i].clone();	//显示检测结果
			Mat tt = glass_list[i].clone();	//预处理
			//imshow("src_" + to_string(i + 1), tt);
			//通过形态学闭操作提取镜面边缘，二值化后取反，与原图相加，使与边缘相连的缺陷独立出来
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

			//二值化后进行边缘检测并提取轮廓，通过轮廓的属性（最小外接圆的半径、轮廓面积）来判断缺陷
			GaussianBlur(addResult, addResult, Size(3, 3), 1);   //高斯滤波
			element = getStructuringElement(MORPH_RECT, Size(3, 3));
			erode(addResult, addResult, element);
			IplImage ipl = (IplImage)addResult;
			int th = Otsu(&ipl);
			threshold(addResult, addResult, th*17/19, 255, CV_THRESH_BINARY);
			rectangle(addResult, Rect(Point(0, 0), Point(addResult.cols - 1, addResult.rows - 1)), Scalar(255), 3);
			//imshow("sharp_" + to_string(i + 1), addResult);
			Canny(addResult, addResult, 50, 80);	//Canny操作的阈值需要调整
			findContours(addResult, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			//遍历轮廓，判断缺陷
			int defect_flag = 0;	//缺陷标志位，0--无缺陷，1--有缺陷
			for (int index = 0; index < contours.size(); index++) {
				Point2f center;
				float radius;
				minEnclosingCircle(contours[index], center, radius);	//计算轮廓的最小外接圆
				if (radius > radiusThres && radius < 100) {	//若最小外接圆的半径大于"radiusThres"，则判断为缺陷
					circle(result, center, radius, Scalar(255), 2, 8);
					defect_flag = 1;
				}
				int area = contourArea(contours[index]);	//计算轮廓面积
				if (area > contourAreaThres && area < 50000 && radius < 100) {	//或轮廓面积大于"contourAreaThres"，则判断为缺陷
					drawContours(result, contours, index, Scalar(255), 2, 8);
					defect_flag = 1;
				}
			}
			//putText(result, Int_to_String(i + 1), Point(tt.cols / 2, tt.rows / 2), FONT_HERSHEY_PLAIN, 5, Scalar(100), 5, 8);	//绘制出当前位置滤光片的编号
			imshow("glass_" + to_string(i + 1), result);
			if (defect_flag == 1) {
				cout << "第<" << to_string(i + 1) << ">位置镜面有缺陷" << endl;
				//imshow("glass_" + to_string(i + 1), result);
			}
		}
	}
}

//检测丝印的部分
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
				minEnclosingCircle(contours[index], center, radius);	//计算轮廓的最小外接圆
				int area = contourArea(contours[index]);	//计算轮廓面积
				int scale = 3;
				if (center.x > scale && center.x < show_temp.cols - scale && center.y > scale && center.y < show_temp.rows - scale && area < show_temp.cols * show_temp.rows / 2 && radius < min(show_temp.cols, show_temp.rows) / 2) {	//过滤掉太边缘的轮廓
					cout << "radius -- " << radius << "\t\tarea -- " << area << endl;
					if (radius >= radiusThres_silkprint || area >= contourAreaThres_silkprint) {	//若最小外接圆的半径大于"radiusThres",或轮廓面积大于"contourAreaThres"，则判断为缺陷
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
				cout << "第<" << to_string(i + 1) << ">位置丝印有缺陷" << endl;
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
			//Canny(silkprint, silkprint, 90, 90);	//Canny操作的阈值需要调整
			Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
			morphologyEx(silkprint, silkprint, MORPH_CLOSE, element);
			imshow("Threshold_" + to_string(i + 1), silkprint);
			element = getStructuringElement(MORPH_RECT, Size(2, 2));
			dilate(silkprint, silkprint, element);

			vector<vector<Point>> contours;
			vector<Vec4i> hierarchy;
			findContours(silkprint, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			//cout << "i = " << i + 1 << endl;
			//遍历轮廓，判断缺陷
			int defect_flag = 0;	//缺陷标志位，0--无缺陷，1--有缺陷
			for (int index = 0; index < contours.size(); index++) {
				Point2f center;
				float radius;
				minEnclosingCircle(contours[index], center, radius);	//计算轮廓的最小外接圆
				if (radius >= radiusThres_silkprint && radius < 100) {	//若最小外接圆的半径大于"radiusThres"，则判断为缺陷
																		//cout << "radius -- " << radius << endl;
					circle(show_temp, center, radius, Scalar(255), 2, 8);
					defect_flag = 1;
				}
				int area = contourArea(contours[index]);	//计算轮廓面积
				if (area >= contourAreaThres_silkprint && area < 50000 && radius < 100) {	//或轮廓面积大于"contourAreaThres"，则判断为缺陷
																							//cout << "area -- " << area << endl;
					drawContours(show_temp, contours, index, Scalar(255), 2, 8);
					defect_flag = 1;
				}
			}
			imshow("silkprint_" + to_string(i + 1), show_temp);
			if (defect_flag == 1) {
				cout << "第<" << to_string(i + 1) << ">位置丝印有缺陷" << endl;
				//imshow("silkprint_" + to_string(i + 1), show_temp);
			}
		}
	}

	return 0;
}




