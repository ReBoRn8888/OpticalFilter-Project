#include "Filter.h"
#include "AssisFunction.h"

Point tl, tr, bl, br;

Filter::Filter(string filepath, string postFix, int num, int pedestalArea)
{
	Mat image1, image2, image3, image4; int length; double ratio = 1.2;
	vector<string> tested_path;
	tested_path = pathGet(filepath, postFix);
	image1 = imread(tested_path[num * 3], 0);
	image2 = imread(tested_path[num * 3 + 1], 0);
	image3 = imread(tested_path[num * 3 + 2], 0);
	image4 = image3.clone();

	srcImg1 = image1.clone();
	srcImg2 = image2.clone();
	srcImg3 = image3.clone();
	vector<Point2f>mycenter; float myradius;

	

	vector<vector<Mat>> Six_area;
	


	vector<int> isGlass = GetArea(image3, 6, mycenter, myradius,whetherNull,pedestalArea);
	if (whetherNull == false){
		int item_num = isGlass.size();
		vector<int> outGlass(item_num);
		vector<Mat> Six_area1(item_num);
		vector<Mat> Six_area2(item_num);
		vector<Mat> Six_area3(item_num);

		vector<Point2f> newcenter = sortCenterpoint(mycenter, image1.rows / 2, isGlass, outGlass);

		for (int i = 0; i < item_num; i++)
		{
			length = myradius* ratio;
			image1(Rect(newcenter.at(i).x - length, newcenter.at(i).y - length, length * 2, length * 2)).copyTo(Six_area1[i]);//第一张图ROI区域的输入
			Area1.push_back(Six_area1[i]);
			image2(Rect(newcenter.at(i).x - length, newcenter.at(i).y - length, length * 2, length * 2)).copyTo(Six_area2[i]);//第二张图ROI区域的输入
			Area2.push_back(Six_area2[i]);
			image4(Rect(newcenter.at(i).x - length, newcenter.at(i).y - length, length * 2, length * 2)).copyTo(Six_area3[i]);//第三张图ROI区域的输入
			Area3.push_back(Six_area3[i]);
			putText(image3, to_string(i + 1), newcenter.at(i), FONT_HERSHEY_PLAIN, 5, Scalar(100), 5, 8);
		
			whetherGlassed.push_back(outGlass[i]);
		}
		imwrite("label.jpg", image3);
	}
}

void Filter::imageMatting(vector<Mat> &silkPrint_show_list, vector<double> image_size) 
{
	Mat roi1, roi2, roi3;
	for (int i = 0; i < Area1.size(); i++){
		//putText(src1, to_string(i), mycenter[i], FONT_HERSHEY_PLAIN, 5, Scalar(100), 5, 8);
		if (whetherGlassed[i] == 1) {
			roi1 = Area1[i].clone();
			roi2 = Area2[i].clone();
			roi3 = Area3[i].clone();

			float degree = RotatedDegree(roi3);
			Point2f center = Point2f(roi2.cols / 2, roi2.rows / 2);
			Mat rotateMat = getRotationMatrix2D(center, degree, 1);
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
			/*--------------------------滤光片提取模块-------------------------*/
			/*----------------------------------------------------------------*/
			Mat glass;  //用于存储提取出的滤光片镜面部分
			roi3(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight)).copyTo(glass, glass_mask);  //提取出滤光片镜面
			glass_list.push_back(glass);

			/*----------------------------------------------------------------*/
			/*---------------------------丝印提取模块--------------------------*/
			/*----------------------------------------------------------------*/
			findContours(glass_mask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			Mat glass_thres;
			threshold(glass, glass_thres, 0, 255.0, CV_THRESH_BINARY);
			IplImage ipl = (IplImage)whole;
			int th = Otsu(&ipl);
			//cout << th << endl;
			drawContours(whole, contours, -1, Scalar(0), 15, 8);
			threshold(whole, whole, th * 11 / 29, 255, CV_THRESH_BINARY);	//二值化，阈值的百分比需要调整
			Mat silkprint; //用于存储提取出的丝印部分
			silkprint = whole + glass_thres;//通过与二值化的镜面部分相加，来提取出丝印部分
			//imshow("silkprint", silkprint);
			Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
			morphologyEx(silkprint, silkprint, MORPH_CLOSE, element);
			dilate(silkprint, silkprint, element, Point(-1, -1), 1);
			silkprint_list.push_back(silkprint);
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

void Filter::imageMatting2(vector<Mat> &silkPrint_show_list, vector<double> image_size)
{
	Mat roi1, roi2, roi3;
	for (int i = 0; i < Area1.size(); i++) {
		//putText(src1, to_string(i), mycenter[i], FONT_HERSHEY_PLAIN, 5, Scalar(100), 5, 8);
		if (whetherGlassed[i] == 1) {
			roi1 = Area1[i].clone();
			roi2 = Area2[i].clone();
			roi3 = Area3[i].clone();

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

			//精确定位整块滤光片
			Mat temp = roi1.clone();
			int width = tr.x - tl.x;	//图上内部小矩形的宽度
			int height = bl.y - tl.y;	//图上内部小矩形的高度
			double width_ratio = image_size[0] / image_size[2];		//大小矩形的宽度比
			double height_ratio = image_size[1] / image_size[3];	//大小矩形的高度比
			int newwidth = width * width_ratio;		//计算出图上外部大矩形的宽度，即可提取出完整滤光片
			int newheight = height * height_ratio;	//计算出图上外部大矩形的高度，即可提取出完整滤光片
			Mat whole = temp(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight));	//提取出整块滤光片
			silkPrint_show_list.push_back(whole.clone());

			Mat new_glass_outer = inner_glass_mask(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight));
			findContours(new_glass_outer.clone(), contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			vector<Point> silkprint_inner_contour = contours[0];	//记录内部镜面轮廓（即丝印内部轮廓）

			Mat element = getStructuringElement(MORPH_RECT, Size(10, 10));
			dilate(new_glass_outer, new_glass_outer, element, Point(-1, -1), 1);
			Mat thres;
			IplImage ipl = (IplImage)whole;
			int th = Otsu(&ipl);
			threshold(whole, thres, th * 17 / 19, 255.0, CV_THRESH_BINARY);    //二值化，阈值的百分比需要调整
			thres = thres - new_glass_outer;
			element = getStructuringElement(MORPH_RECT, Size(9, 9));
			morphologyEx(thres, thres, MORPH_OPEN, element);	//开操作消除较小明亮区域
			element = getStructuringElement(MORPH_RECT, Size(25, 25));
			morphologyEx(thres, thres, MORPH_CLOSE, element);	//闭操作消除低亮度的孤立点
			//imshow("thres", thres);
			findContours(thres, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			vector<Point> silkprint_outer_contour = contours[1];	//记录丝印外部轮廓

			vector<vector<Point>> sk_contours;	//记录丝印内外轮廓
			sk_contours.push_back(silkprint_inner_contour);
			sk_contours.push_back(silkprint_outer_contour);
			Mat silkprint_mask(thres.rows, thres.cols, CV_8UC1, Scalar(0));   //滤光片mask
			drawContours(silkprint_mask, sk_contours, 1, Scalar(255), CV_FILLED, 8);
			drawContours(silkprint_mask, sk_contours, 0, Scalar(0), CV_FILLED, 8);
			drawContours(silkprint_mask, sk_contours, 1, Scalar(255), 2, 8);
			drawContours(silkprint_mask, sk_contours, 0, Scalar(0), 2, 8);
			Mat glass_mask;
			bitwise_not(silkprint_mask, glass_mask);

			/*----------------------------------------------------------------*/
			/*---------------------------丝印提取模块--------------------------*/
			/*----------------------------------------------------------------*/
			Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, 0, 3.3, 0, 0, -1, 0);
			filter2D(whole, whole, CV_8UC1, kernel);
			Mat silkprint = whole + glass_mask;
			ipl = (IplImage)silkprint;
			th = Otsu(&ipl);
			threshold(silkprint, silkprint, th * 102 / 107, 255.0, CV_THRESH_BINARY);    //二值化，阈值的百分比需要调整
			drawContours(silkprint, sk_contours, 1, Scalar(0), 3, 8);
			element = getStructuringElement(MORPH_RECT, Size(3, 3));
			morphologyEx(silkprint, silkprint, MORPH_OPEN, element);	//开操作去除较小的明亮区域
			//imshow("silkprint", silkprint);
			silkprint_list.push_back(silkprint);

			/*----------------------------------------------------------------*/
			/*--------------------------滤光片提取模块-------------------------*/
			/*----------------------------------------------------------------*/
			Mat inner_glass = roi3(Rect(temp.cols / 2 - newwidth / 2, temp.rows / 2 - newheight / 2, newwidth, newheight));
			//通过众数取得内部镜面的整体颜色
			vector<int> ll;
			for (int x = inner_glass.cols / 4; x < inner_glass.cols / 4 * 3; x++) {
				int value = inner_glass.at<uchar>(inner_glass.rows / 2, x);
				ll.push_back(value);
			}
			int modeNumber = getModeNumber(ll);	//该数值即为内部镜面单通道颜色数值
			Mat gray(thres.rows, thres.cols, CV_8UC1, Scalar(255 - modeNumber));	//设置外部镜面的颜色gray，使得原来的白色与gray相减后与内部镜面颜色相当
			drawContours(gray, sk_contours, 1, Scalar(0), CV_FILLED, 8);
			drawContours(silkprint_mask, sk_contours, 1, Scalar(255), CV_FILLED, 8);
			Mat outer_glass_mask;
			bitwise_not(silkprint_mask, outer_glass_mask);
			Mat glass;
			whole.copyTo(glass, outer_glass_mask);
			glass = glass + inner_glass - gray;	//将内部镜面与外部镜面相加，并减去gray，使得外部镜面与内部镜面颜色相当
			//imshow("show1_" + to_string(i + 1), glass);
			glass_list.push_back(glass);
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


Filter::~Filter(){}