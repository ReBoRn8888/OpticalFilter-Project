#include "Filter.h"
#include "AssisFunction.h"

Point tl, tr, bl, br;

Filter::Filter(string filepath, string postFix, int num, templateGet FilterParameter)
{
	Mat image1, image2, image3, image4;
	tested_path = pathGet(filepath, postFix);
	if (tested_path.size() == 0)
		return;
	image1 = imread(tested_path[num * 3], 0);
	image2 = imread(tested_path[num * 3 + 1], 0);
	image3 = imread(tested_path[num * 3 + 2], 0);
	image4 = image3.clone();

	srcImg1 = image1.clone();
	srcImg2 = image2.clone();
	srcImg3 = image3.clone();

	vector<Point2f>mycenter;
	vector<vector<Mat>> Six_area;

	vector<int> isGlass = GetArea(image3, 6, mycenter, whetherNull, FilterParameter.filterArea, FilterParameter.ratio);

	if (whetherNull == false) {
		int item_num = isGlass.size();
		vector<int> outGlass(item_num);
		vector<Mat> Six_area1(item_num);
		vector<Mat> Six_area2(item_num);
		vector<Mat> Six_area3(item_num);

		vector<Point2f> newcenter = sortCenterpoint(mycenter, image1.rows / 2, isGlass, outGlass);
		int X1, Y1, X2, Y2;
		for (int i = 0; i < item_num; i++){
			X1 = newcenter.at(i).x - FilterParameter.firstWidth / 2;
			X2 = newcenter.at(i).x + FilterParameter.firstWidth / 2;
			Y1 = newcenter.at(i).y - FilterParameter.firstHeight / 2;
			Y2 = newcenter.at(i).y + FilterParameter.firstHeight / 2;
			if (X1 >= 0 && X2 <= srcImg1.cols && Y1 >= 0 && Y2 <= srcImg1.rows)
			{
				image1(Rect(X1, Y1, FilterParameter.firstWidth, FilterParameter.firstHeight)).copyTo(Six_area1[i]);//��һ��ͼROI���������
				Area1.push_back(Six_area1[i]);
				image2(Rect(X1, Y1, FilterParameter.firstWidth, FilterParameter.firstHeight)).copyTo(Six_area2[i]);//�ڶ���ͼROI���������
				Area2.push_back(Six_area2[i]);
				image4(Rect(X1, Y1, FilterParameter.firstWidth, FilterParameter.firstHeight)).copyTo(Six_area3[i]);//������ͼROI���������
				Area3.push_back(Six_area3[i]);
			}
			else
			{
				image1.copyTo(Six_area1[i]);//��һ��ͼROI���������
				Area1.push_back(Six_area1[i]);
				image2.copyTo(Six_area2[i]);//�ڶ���ͼROI���������
				Area2.push_back(Six_area2[i]);
				image4.copyTo(Six_area3[i]);//������ͼROI���������
				Area3.push_back(Six_area3[i]);
				outGlass[i] = 0;
			}
			int A = srcImg1.cols / 3;
			int B = srcImg1.rows / 2;
			int localFlag = 10 * int(newcenter.at(i).x / A) + newcenter.at(i).y / B;
			switch (localFlag){
				case 0:
					glassLabel.push_back(1); break;
				case 1:
					glassLabel.push_back(4); break;
				case 10:
					glassLabel.push_back(2); break;
				case 11:
					glassLabel.push_back(5); break;
				case 20:
					glassLabel.push_back(3); break;
				case 21:
					glassLabel.push_back(6); break;
			};
			putText(image3, to_string(glassLabel[i]), newcenter.at(i), FONT_HERSHEY_PLAIN, 5, Scalar(100), 5, 8);

			whetherGlassed.push_back(outGlass[i]);
		}
		imwrite("label.jpg", image3);
	}
}

void Filter::imageMatting(vector<Mat> &silkPrint_show_list, templateGet FilterParameter) {
	Mat roi1, roi2, roi3;
	for (int i = 0; i < Area1.size(); i++){
		if (whetherGlassed[i] == 1) {
			roi1 = Area1[i].clone();
			roi2 = Area2[i].clone();
			roi3 = Area3[i].clone();

			Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
			morphologyEx(roi3, roi3, MORPH_OPEN, element);	//������������С��������

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

			//���˵�С������ȡ���м�͸����������
			vector<vector<Point>>::iterator it_contour = contours.begin();
			while (it_contour != contours.end()){
				if (contourArea(*it_contour) < 100000 || contourArea(*it_contour) > FilterParameter.filterArea * 1.2)//ɾȥ�����С�͹��������
					it_contour = contours.erase(it_contour);
				else
					++it_contour;
			}

			//��¼�˹�Ƭ�������Ϻ����µ������
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

			//�����˹�Ƭ������ж��Ƿ�Ϊ�����˹�Ƭ
			int width = tr.x - tl.x;	//ͼ���ڲ�С���εĿ��
			int height = bl.y - tl.y;	//ͼ���ڲ�С���εĸ߶�
			double thisRatio = (double)height / (double)width;
			bool typeFlag1 = thisRatio < FilterParameter.ratio*1.1 && thisRatio >FilterParameter.ratio*0.9;
			if (typeFlag1)
				whetherGlassed[i] = 1;
			else
				whetherGlassed[i] = -1;

			//��ȷ��λ�����˹�Ƭ
			Mat temp = roi2.clone();
			silkPrint_show_list.push_back(temp(Rect(temp.cols / 2 - FilterParameter.filterWidth / 2, temp.rows / 2 - FilterParameter.filterHeight / 2, FilterParameter.filterWidth, FilterParameter.filterHeight)).clone());
			Mat whole_temp = temp(Rect(temp.cols / 2 - FilterParameter.filterWidth / 2, temp.rows / 2 - FilterParameter.filterHeight / 2, FilterParameter.filterWidth, FilterParameter.filterHeight)).clone();	//��ȡ�������˹�Ƭ
			//�ھ����ĸ����ϻ���ɫ���Σ��������ⲻ����������У����ƽ˿ӡ4 5 6.bmp��
			int w = 60, scale = 15;
			rectangle(temp, Rect(Point(tl.x - scale, tl.y - scale), Point(tl.x + w, tl.y + w)), Scalar(0), CV_FILLED, 8);
			rectangle(temp, Rect(Point(bl.x - scale, bl.y - w), Point(bl.x + w, bl.y + scale)), Scalar(0), CV_FILLED, 8);
			rectangle(temp, Rect(Point(tr.x - w, tr.y - scale), Point(tr.x + scale, tr.y + w)), Scalar(0), CV_FILLED, 8);
			rectangle(temp, Rect(Point(br.x - w, br.y - w), Point(br.x + scale, br.y + scale)), Scalar(0), CV_FILLED, 8);

			Mat whole = temp(Rect(temp.cols / 2 - FilterParameter.filterWidth / 2, temp.rows / 2 - FilterParameter.filterHeight / 2, FilterParameter.filterWidth, FilterParameter.filterHeight));	//��ȡ�������˹�Ƭ
			Area1[i] = Area1[i](Rect(temp.cols / 2 - FilterParameter.filterWidth / 2, temp.rows / 2 - FilterParameter.filterHeight / 2, FilterParameter.filterWidth, FilterParameter.filterHeight));	//�����ߴ�
			Area2[i] = Area2[i](Rect(temp.cols / 2 - FilterParameter.filterWidth / 2, temp.rows / 2 - FilterParameter.filterHeight / 2, FilterParameter.filterWidth, FilterParameter.filterHeight));	//�����ߴ�
			Area3[i] = Area3[i](Rect(temp.cols / 2 - FilterParameter.filterWidth / 2, temp.rows / 2 - FilterParameter.filterHeight / 2, FilterParameter.filterWidth, FilterParameter.filterHeight));	//�����ߴ�
			glass_mask = glass_mask(Rect(temp.cols / 2 - FilterParameter.filterWidth / 2, temp.rows / 2 - FilterParameter.filterHeight / 2, FilterParameter.filterWidth, FilterParameter.filterHeight));	//����glass_mask�ߴ�

			/*----------------------------------------------------------------*/
			/*--------------------------͸������ȡģ��-------------------------*/
			/*----------------------------------------------------------------*/
			//Mat tt;
			//threshold(whole_temp, tt, 250, 255, CV_THRESH_BINARY);
			element = getStructuringElement(MORPH_RECT, Size(FilterParameter.elementSize, FilterParameter.elementSize));
			Mat forGlassContour;
			erode(glass_mask, forGlassContour, element, Point(-1, -1), 1);

			//imwrite("cs.jpg", tt);
			findContours(forGlassContour, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			it_contour = contours.begin();
			while (it_contour != contours.end()){
				if (contourArea(*it_contour) < 100000 || contourArea(*it_contour) > FilterParameter.filterArea * 2)//ɾȥ�����С�͹��������
					it_contour = contours.erase(it_contour);
				else
					++it_contour;
			}
			Mat glass;  //���ڴ洢��ȡ�����˹�Ƭ���沿��
			roi2(Rect(temp.cols / 2 - FilterParameter.filterWidth / 2, temp.rows / 2 - FilterParameter.filterHeight / 2, FilterParameter.filterWidth, FilterParameter.filterHeight)).copyTo(glass, forGlassContour);  //��ȡ���˹�Ƭ����
			drawContours(glass, contours, -1, Scalar(getAveragePix(glass, 0)), 3);
			glass_list.push_back(glass);

			/*----------------------------------------------------------------*/
			/*---------------------------˿ӡ��ȡģ��--------------------------*/
			/*----------------------------------------------------------------*/
			Mat forSilkContour;
			dilate(glass_mask, forSilkContour, element, Point(-1, -1), 1);
			Mat silkprint_mask;
			bitwise_not(forSilkContour, silkprint_mask);
			findContours(forSilkContour, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			it_contour = contours.begin();
			while (it_contour != contours.end()){
				if (contourArea(*it_contour) < 100000 || contourArea(*it_contour) > FilterParameter.filterArea * 2)//ɾȥ�����С�͹��������
					it_contour = contours.erase(it_contour);
				else
					++it_contour;
			}
			drawContours(glass_mask, contours, -1, Scalar(255), CV_FILLED);
			Mat silkprint; //���ڴ洢��ȡ����˿ӡ����
			roi1(Rect(temp.cols / 2 - FilterParameter.filterWidth / 2, temp.rows / 2 - FilterParameter.filterHeight / 2, FilterParameter.filterWidth, FilterParameter.filterHeight)).copyTo(silkprint, silkprint_mask);  //��ȡ���˹�Ƭ����
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

void Filter::imageMatting2(vector<Mat> &silkPrint_show_list, templateGet FilterParameter){
	Mat roi1, roi2, roi3;
	for (int i = 0; i < Area1.size(); i++) {
		if (whetherGlassed[i] == 1) {
			roi1 = Area1[i].clone();
			roi2 = Area2[i].clone();
			roi3 = Area3[i].clone();

			Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
			morphologyEx(roi3, roi3, MORPH_OPEN, element);	//������������С��������

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

			//���˵�С������ȡ���м�͸����������
			vector<vector<Point>>::iterator it_contour = contours.begin();
			while (it_contour != contours.end()){
				if (contourArea(*it_contour) < 100000 || contourArea(*it_contour) > FilterParameter.filterArea * 1.2)//ɾȥ�����С�͹��������
					it_contour = contours.erase(it_contour);
				else
					++it_contour;
			}
			//��¼�˹�Ƭ�������Ϻ����µ������
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

			//�����˹�Ƭ������ж��Ƿ�Ϊ�����˹�Ƭ
			int width = tr.x - tl.x;	//ͼ���ڲ�С���εĿ��
			int height = bl.y - tl.y;	//ͼ���ڲ�С���εĸ߶�
			double thisRatio = (double)height / (double)width;
			bool typeFlag1 = thisRatio < FilterParameter.ratio*1.1 && thisRatio >FilterParameter.ratio*0.9;
			if (typeFlag1)
				whetherGlassed[i] = 1;
			else{
				whetherGlassed[i] = -1;
				return;
			}

			//��ȷ��λ�����˹�Ƭ
			Mat temp = roi1.clone();
			Mat whole = temp(Rect(temp.cols / 2 - FilterParameter.filterWidth / 2, temp.rows / 2 - FilterParameter.filterHeight / 2, FilterParameter.filterWidth, FilterParameter.filterHeight));	//��ȡ�������˹�Ƭ
			Area1[i] = Area1[i](Rect(temp.cols / 2 - FilterParameter.filterWidth / 2, temp.rows / 2 - FilterParameter.filterHeight / 2, FilterParameter.filterWidth, FilterParameter.filterHeight));	//�����ߴ�
			Area2[i] = Area2[i](Rect(temp.cols / 2 - FilterParameter.filterWidth / 2, temp.rows / 2 - FilterParameter.filterHeight / 2, FilterParameter.filterWidth, FilterParameter.filterHeight));	//�����ߴ�
			Area3[i] = Area3[i](Rect(temp.cols / 2 - FilterParameter.filterWidth / 2, temp.rows / 2 - FilterParameter.filterHeight / 2, FilterParameter.filterWidth, FilterParameter.filterHeight));	//�����ߴ�
			inner_glass_mask = inner_glass_mask(Rect(temp.cols / 2 - FilterParameter.filterWidth / 2, temp.rows / 2 - FilterParameter.filterHeight / 2, FilterParameter.filterWidth, FilterParameter.filterHeight));	//�����ߴ�
			silkPrint_show_list.push_back(whole.clone());


			/*----------------------------------------------------------------*/
			/*--------------------------͸������ȡģ��-------------------------*/
			/*----------------------------------------------------------------*/
			element = getStructuringElement(MORPH_RECT, Size(FilterParameter.elementSize, FilterParameter.elementSize));
			dilate(inner_glass_mask, inner_glass_mask, element, Point(-1, -1), 1);
			Mat outer_glass_mask;
			IplImage ipl = (IplImage)whole;
			int th = Otsu(&ipl);
			threshold(whole, outer_glass_mask, th * 17 / 19, 255.0, CV_THRESH_BINARY);    //��ֵ������ֵ�İٷֱ���Ҫ����
			outer_glass_mask = outer_glass_mask - inner_glass_mask;
			element = getStructuringElement(MORPH_RECT, Size(9, 9));
			morphologyEx(outer_glass_mask, outer_glass_mask, MORPH_OPEN, element);	//������������С��������
			element = getStructuringElement(MORPH_RECT, Size(25, 25));
			morphologyEx(outer_glass_mask, outer_glass_mask, MORPH_CLOSE, element);	//�ղ������������ȵĹ�����
			element = getStructuringElement(MORPH_RECT, Size(FilterParameter.elementSize, FilterParameter.elementSize));
			erode(inner_glass_mask, inner_glass_mask, element, Point(-1, -1), 1);
			erode(inner_glass_mask, inner_glass_mask, element, Point(-1, -1), 1);
			erode(outer_glass_mask, outer_glass_mask, element, Point(-1, -1), 1);
			Mat glass_mask = inner_glass_mask + outer_glass_mask;
			findContours(glass_mask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			it_contour = contours.begin();
			while (it_contour != contours.end()){
				if (contourArea(*it_contour) < 100000 || contourArea(*it_contour) > FilterParameter.filterArea * 2)//ɾȥ�����С�͹��������
					it_contour = contours.erase(it_contour);
				else
					++it_contour;
			}
			Mat inner_glass, outer_glass;
			Area2[i].copyTo(inner_glass, inner_glass_mask);
			Mat gray(inner_glass.rows, inner_glass.cols, CV_8UC1, Scalar(255 - getAveragePix(inner_glass, 0)));	//�����ⲿ�������ɫ��ֵgray��ʹ��ԭ���İ�ɫ��gray��������ڲ�������ɫ�൱
			Area1[i].copyTo(outer_glass, outer_glass_mask);
			outer_glass = outer_glass - gray;
			Mat glass = inner_glass + outer_glass;
			drawContours(glass, contours, -1, Scalar(getAveragePix(glass, 0), 5));
			glass_list.push_back(glass);

			/*----------------------------------------------------------------*/
			/*---------------------------˿ӡ��ȡģ��--------------------------*/
			/*----------------------------------------------------------------*/
			Mat silkprint_mask;
			bitwise_not(inner_glass_mask + outer_glass_mask, silkprint_mask);
			element = getStructuringElement(MORPH_RECT, Size(FilterParameter.elementSize, FilterParameter.elementSize));
			erode(silkprint_mask, silkprint_mask, element, Point(-1, -1), 1);
			erode(silkprint_mask, silkprint_mask, element, Point(-1, -1), 1);
			findContours(silkprint_mask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			it_contour = contours.begin();
			while (it_contour != contours.end()){
				if (contourArea(*it_contour) < 100000 || contourArea(*it_contour) > FilterParameter.filterArea * 2)//ɾȥ�����С�͹��������
					it_contour = contours.erase(it_contour);
				else
					++it_contour;
			}
			Mat silkprint;
			Area1[i].copyTo(silkprint, silkprint_mask);
			drawContours(silkprint, contours, -1, Scalar(getAveragePix(silkprint, 0), 5));
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

void Filter::imageMatting3(vector<Mat> &silkPrint_show_list, templateGet FilterParameter) {
	Mat roi1, roi2, roi3;
	for (int i = 0; i < Area1.size(); i++) {
		if (whetherGlassed[i] == 1) {
			roi1 = Area1[i].clone();
			roi2 = Area2[i].clone();
			roi3 = Area3[i].clone();

			/*----------------------------------------------------------------*/
			/*--------------------------͸������ȡģ��-------------------------*/
			/*----------------------------------------------------------------*/
			Mat forGlassContour;
			threshold(roi2, forGlassContour, 250, 255, CV_THRESH_BINARY);
			Mat element = getStructuringElement(MORPH_RECT, Size(FilterParameter.elementSize, FilterParameter.elementSize));
			erode(forGlassContour, forGlassContour, element, Point(-1, -1), 1);
			vector<vector<Point>> contours;
			vector<Vec4i> hierarchy;
			findContours(forGlassContour, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			vector<vector<Point>>::iterator it_contour = contours.begin();
			while (it_contour != contours.end()){
				if (contourArea(*it_contour) < 100000 || contourArea(*it_contour) > FilterParameter.filterArea * 2)//ɾȥ�����С�͹��������
					it_contour = contours.erase(it_contour);
				else
					++it_contour;
			}
			Mat glass_mask(roi3.rows, roi3.cols, CV_8UC1, Scalar(0));   //�˹�Ƭmask
			drawContours(glass_mask, contours, -1, Scalar(255), CV_FILLED, 8);

			Mat glass;
			roi2.copyTo(glass, glass_mask);
			drawContours(glass, contours, -1, Scalar(getModePix(glass, 0)), 3);
			glass_list.push_back(glass);

			/*----------------------------------------------------------------*/
			/*---------------------------˿ӡ��ȡģ��--------------------------*/
			/*----------------------------------------------------------------*/
			Mat dsc(roi3.rows, roi3.cols, CV_8UC1, Scalar(100));
			silkprint_list.push_back(dsc);
			silkPrint_show_list.push_back(dsc);
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