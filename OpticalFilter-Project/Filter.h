
#include "AssisFunction.h"

struct SinglefilterMessage
{
	Mat src[3];//获取的最终检测用的精确定位的图像
	double type;//检测类型;扁平、螺纹、双摄等具体的可以细分，使用12345这种数字既可
	int flag = 0;//OK、NG结果，0为ok，1为NG
	int label;//图像中的位置（1-6）
	double xpos;//plc提供的坐标
	double ypos;//plc提供的坐标
};

class Filter
{
public:

	vector<Mat> glass_list;
	vector<Mat> silkprint_list;//In public for convience
	Mat srcImg1, srcImg2, srcImg3;//three each group
	//vector<string> SingleType;//For example:"Empty","Printing"&etc.
	vector<int> whetherGlassed;//whether coverd glass
	vector<Mat> Area1;
	vector<Mat> Area2;
	vector<Mat> Area3;//Six area each map

	Filter(string filepath, string postFix, int num, int pedestalArea);
	bool whetherNull;
	void imageMatting(vector<Mat> &, vector<double>);
	void imageMatting2(vector<Mat> &, vector<double>);

	~Filter();
};




