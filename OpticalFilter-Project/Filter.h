
#include "AssisFunction.h"

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


	Filter(string filepath, string postFix, int num);
	

	void imageMatting(vector<Mat> &, vector<double>);

	void imageMatting2(vector<Mat> &, vector<double>);



	~Filter();




};




