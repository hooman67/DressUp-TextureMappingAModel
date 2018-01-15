#include "first.h"

class GCApplication {

	const Scalar RED = Scalar(0, 0, 255);
	const Scalar PINK = Scalar(230, 130, 255);
	const Scalar BLUE = Scalar(255, 0, 0);
	const Scalar LIGHTBLUE = Scalar(255, 255, 160);
	const Scalar GREEN = Scalar(0, 255, 0);

	const int BGD_KEY = EVENT_FLAG_CTRLKEY;
	const int FGD_KEY = EVENT_FLAG_SHIFTKEY;

	enum{ NOT_SET = 0, IN_PROCESS = 1, SET = 2 };
	static const int radius = 2;
	static const int thickness = -1;

	const string* winName;

	Mat mask;
	Mat bgdModel, fgdModel;

	uchar rectState, lblsState, prLblsState;
	bool isInitialized;

	Rect rect;
	vector<cv::Point> fgdPxls, bgdPxls, prFgdPxls, prBgdPxls;
	int iterCount;
	void setRectInMask();
	void setLblsInMask(int showAlphaMaps, cv::Point p, bool isPr);

public:

	static void help();
	const Mat* image;

	Mat* extractedAlphaMap;
	Mat binMask;
	bool showAlphaMap = false;
	bool showContours = false;



	void reset();
	void setImageAndWinName(const Mat& _image, const string& _winName);
	static void getBinMask(const Mat& comMask, Mat& binMask);
	void showImage();
	void mouseClick(int event, int x, int y, int showAlphaMaps, void* param);
	int nextIter();
	int getIterCount() const { return iterCount; }

	const Mat* hsGetOutPutImage() const { return image; }
	void saveOutputImage(Mat*& output);
};

