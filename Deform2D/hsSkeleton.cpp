#include "GCApplication.h"

class hsSkeleton : public GCApplication {
public:

	static void help()
	{
		cout << "\nKosse nane raja -- Skeleton Setup Tool\n"
			"Call:\n"
			"\nHot keys: \n"
			"\tESC - Exit\n"
			"\tr - show skeleton image\n"
			"\ts - save Skeleton file\n"
			"\tl - load new skeleton file\n"
			"\tleft mouse button - select point/ Drag and Drop\n" << endl;
	}

	Vector<Point2f> points;
	const Mat* skeletonImage = NULL;
	const string* winName = NULL;
	int selectedPoint = std::numeric_limits<int>::max();

	void loadSkeleton(int select){

		String defaultFileName = "skeletonSample.node";
		String fileName;

		if (select == 1)
			fileName = "shirtSkeleton.node";
		else if (select == 2)
			fileName = "bodySkeleton.node";
		else {
			cout << "enter fileName (without the extension) or d for default: ";
			cin >> fileName;

			if (fileName == "d")
				fileName = defaultFileName;
			else
				fileName += ".node";
		}

		fstream pointsFile(fileName, std::ios_base::in);

		if (!pointsFile.is_open()){
			cerr << "loadSkeleton failed to open the file: "<< fileName<<"\n";
			return;
		}

		for (int i = 0; i < 17; i++) {
			float vertId, xcord, ycord;

			pointsFile >> vertId;
			pointsFile >> xcord;
			pointsFile >> ycord;
			points.push_back(Point2f(xcord, ycord));
		}
		pointsFile.close();
		cout << "Loaded skeleton: "<< fileName <<"\n";
	}

	void saveSkeleton(){

		if (points.empty()){
			cout << "There is not skeleton to save\n";
			return;
		}

		String fileName;
		cout << "Give a name (without the extension): ";
		cin >> fileName;
		fileName += ".node";

		ofstream myfile;
		myfile.open(fileName);

		for (int i = 0; i < 17; i++){
			myfile << i << "	" << points[i].x << "	" << points[i].y << "\n";
		}
		myfile.close();

		cout << "saved Skeleton: " << fileName << "\n";
	}

	int findHitPoint(float x, float y){

		float fx, fy;

		for (int i = 0; i < points.size(); i++)
		{
			fx = points[i].x;
			fy = points[i].y;
			double disSquare = (x - fx) * (x - fx) + (y - fy) * (y - fy);
			if (disSquare < 25)
			{
				return i;
			}
		}

		return std::numeric_limits<int>::max();
	}

	void setImageAndWinName(Mat& _image, const string& _winName)
	{
		if (_image.empty() || _winName.empty())
			return;
		skeletonImage = &_image;
		winName = &_winName;
	}

	void showSkeletonImage() {

		const Scalar Blue = Scalar(255, 0, 0);
		const Scalar Green = Scalar(0, 255, 0);

		if (skeletonImage->empty() || winName->empty())
			return;

		Mat out;
		skeletonImage->copyTo(out);
		
		if (!points.empty()){

			for (int p = 0; p < points.size(); p++) {
				circle(out, points[p], 4, Green, -1);
			}

			for (int p = 0; p < 7; p++){
				line(out, points[p], points[p + 1], Green, 2);
			}

			for (int p = 8; p < 13; p++){
				line(out, points[p], points[p + 1], Green, 2);
			}
			line(out, points[10], points[14], Green, 2);
			line(out, points[14], points[15], Green, 2);
			line(out, points[15], points[16], Green, 2);
		}

		cv::imshow(*winName, out);
	}

	void skeleton_mouseClick(int event, int x, int y, int flags, void*)
	{
		// TODO add bad args check
		switch (event) {
		case EVENT_LBUTTONDOWN:
			selectedPoint = findHitPoint((float)x, (float)y);
			break;
		case EVENT_LBUTTONUP:
			if (selectedPoint < points.size()){
				points[selectedPoint] = Point2f(float(x), float(y));

			    showSkeletonImage();
				
				selectedPoint = std::numeric_limits<size_t>::max();
			}
			break;
		case EVENT_MOUSEMOVE:
			if (selectedPoint < points.size()){
				points[selectedPoint] = Point2f(float(x), float(y));
				showSkeletonImage();
			}
			break;
		}
	}
};