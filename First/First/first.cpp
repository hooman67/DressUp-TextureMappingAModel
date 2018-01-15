//WORKING COPY: everything up to coloring triang 3

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <freeglut.h>

#include <math.h>
#include <map>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

static void help()
{
	cout << "\n -- select an object in a region\n"
		"and then grabcut will attempt to segment it out.\n"
		"Call:\n"
		"./grabcut <image_name>\n"
		"\nSelect a rectangular area around the object you want to segment\n" <<
		"\nHot keys: \n"
		"\tESC - Draw Contours\n"
		"\tm - Shows the Alpha Map\n"
		"\tr - restore the original image\n"
		"\tn - next iteration\n"
		"\n"
		"\tleft mouse button - set rectangle\n"
		"\n"
		"\tCTRL+left mouse button - set GC_BGD pixels\n"
		"\tSHIFT+left mouse button - set GC_FGD pixels\n"
		"\n"
		"\tCTRL+right mouse button - set GC_PR_BGD pixels\n"
		"\tSHIFT+right mouse button - set GC_PR_FGD pixels\n"
		"\n"
		"\tLevels can range from - 3 to 3\n" << endl;
}

const Scalar RED = Scalar(0, 0, 255);
const Scalar PINK = Scalar(230, 130, 255);
const Scalar BLUE = Scalar(255, 0, 0);
const Scalar LIGHTBLUE = Scalar(255, 255, 160);
const Scalar GREEN = Scalar(0, 255, 0);

const int BGD_KEY = EVENT_FLAG_CTRLKEY;
const int FGD_KEY = EVENT_FLAG_SHIFTKEY;

static void getBinMask(const Mat& comMask, Mat& binMask)
{
	if (comMask.empty() || comMask.type() != CV_8UC1)
		cerr << "comMask is empty or has incorrect type (not CV_8UC1)\n";
	if (binMask.empty() || binMask.rows != comMask.rows || binMask.cols != comMask.cols)
		binMask.create(comMask.size(), CV_8UC1);
	binMask = comMask & 1;
}

class GCApplication
{
public:

	const Mat* image;

	bool showAlphaMap = false;
	bool showContours = false;
	Mat* extractedAlphaMap;

	enum{ NOT_SET = 0, IN_PROCESS = 1, SET = 2 };
	static const int radius = 2;
	static const int thickness = -1;

	void reset();
	void setImageAndWinName(const Mat& _image, const string& _winName);
	void showImage(bool writeFile);
	void mouseClick(int event, int x, int y, int showAlphaMaps, void* param);
	int nextIter();
	int getIterCount() const { return iterCount; }

	const Mat* hsGetOutPutImage() const { return image; }
private:
	void setRectInMask();
	void setLblsInMask(int showAlphaMaps, Point p, bool isPr);

	const string* winName;
	
	Mat mask;
	Mat bgdModel, fgdModel;

	uchar rectState, lblsState, prLblsState;
	bool isInitialized;

	Rect rect;
	vector<Point> fgdPxls, bgdPxls, prFgdPxls, prBgdPxls;
	int iterCount;
};

void GCApplication::reset()
{
	showAlphaMap = false;
	if (!mask.empty())
		mask.setTo(Scalar::all(GC_BGD));
	bgdPxls.clear(); fgdPxls.clear();
	prBgdPxls.clear();  prFgdPxls.clear();

	isInitialized = false;
	rectState = NOT_SET;
	lblsState = NOT_SET;
	prLblsState = NOT_SET;
	iterCount = 0;
}

void GCApplication::setImageAndWinName(const Mat& _image, const string& _winName)
{
	if (_image.empty() || _winName.empty() )
		return;
	image = &_image;
	winName = &_winName;
	mask.create(image->size(), CV_8UC1);
	reset();
}

void GCApplication::showImage(bool writeFile)
{
	
	if (image->empty() || winName->empty() )
		return;

	Mat res;
	Mat binMask;
	if (!isInitialized)
		image->copyTo(res);
	else
	{
		getBinMask(mask, binMask);
		image->copyTo(res, binMask);
	}

	Scalar* n = new Scalar(0);
	extractedAlphaMap = new Mat(image->size(), CV_8UC1, *n);
	Scalar* p = new Scalar(255);
	extractedAlphaMap->setTo(*p, binMask);


	vector<Point>::const_iterator it;
	for (it = bgdPxls.begin(); it != bgdPxls.end(); ++it)
		circle(res, *it, radius, BLUE, thickness);
	for (it = fgdPxls.begin(); it != fgdPxls.end(); ++it)
		circle(res, *it, radius, RED, thickness);
	for (it = prBgdPxls.begin(); it != prBgdPxls.end(); ++it)
		circle(res, *it, radius, LIGHTBLUE, thickness);
	for (it = prFgdPxls.begin(); it != prFgdPxls.end(); ++it)
		circle(res, *it, radius, PINK, thickness);

	if (rectState == IN_PROCESS || rectState == SET)
		rectangle(res, Point(rect.x, rect.y), Point(rect.x + rect.width, rect.y + rect.height), GREEN, 2);

	if (!showAlphaMap && !showContours)
		imshow(*winName, res);
	else if (showAlphaMap){
		imshow(*winName, *extractedAlphaMap);
	}
	else if (showContours) {
		//FindDrawContours(extractedAlphaMap);
	}

	if (writeFile){
		ofstream myfile;
		myfile.open("boundary1.node");
		myfile
			<< "# boundary.node\n"
			<< "#\n"
			<< binMask.rows*binMask.cols << "	" << 2 << "	" << 0 << "	" << 0 << "\n"
			<< "# and here are the points:\n";

		bool *input = (bool*)(binMask.data);
		int line = 0;
		for (int j = 0; j < binMask.rows; j++){
			for (int i = 0; i < binMask.cols; i++){
				if (input[binMask.step * j + i]) {
					myfile << line++ << "	" << i << "	" << j << "	" << 0 << "	" << 0 << "\n";
				}
			}
		}
		cout << "wrote to file: " << line-1<< "\n";
	}
			
		
}

void GCApplication::setRectInMask()
{
	CV_Assert(!mask.empty());
	mask.setTo(GC_BGD);
	rect.x = max(0, rect.x);
	rect.y = max(0, rect.y);
	rect.width = min(rect.width, image->cols - rect.x);
	rect.height = min(rect.height, image->rows - rect.y);
	(mask(rect)).setTo(Scalar(GC_PR_FGD));
}

void GCApplication::setLblsInMask(int flags, Point p, bool isPr)
{
	vector<Point> *bpxls, *fpxls;
	uchar bvalue, fvalue;
	if (!isPr)
	{
		bpxls = &bgdPxls;
		fpxls = &fgdPxls;
		bvalue = GC_BGD;
		fvalue = GC_FGD;
	}
	else
	{
		bpxls = &prBgdPxls;
		fpxls = &prFgdPxls;
		bvalue = GC_PR_BGD;
		fvalue = GC_PR_FGD;
	}
	if (flags & BGD_KEY)
	{
		bpxls->push_back(p);
		circle(mask, p, radius, bvalue, thickness);
	}
	if (flags & FGD_KEY)
	{
		fpxls->push_back(p);
		circle(mask, p, radius, fvalue, thickness);
	}
}

void GCApplication::mouseClick(int event, int x, int y, int flags, void*)
{
	// TODO add bad args check
	switch (event)
	{
	case EVENT_LBUTTONDOWN: // set rect or GC_BGD(GC_FGD) labels
	{
		bool isb = (flags & BGD_KEY) != 0,
			isf = (flags & FGD_KEY) != 0;
		if (rectState == NOT_SET && !isb && !isf)
		{
			rectState = IN_PROCESS;
			rect = Rect(x, y, 1, 1);
		}
		if ((isb || isf) && rectState == SET)
			lblsState = IN_PROCESS;
	}
		break;
	case EVENT_RBUTTONDOWN: // set GC_PR_BGD(GC_PR_FGD) labels
	{
		bool isb = (flags & BGD_KEY) != 0,
			isf = (flags & FGD_KEY) != 0;
		if ((isb || isf) && rectState == SET)
			prLblsState = IN_PROCESS;
	}
		break;
	case EVENT_LBUTTONUP:
		if (rectState == IN_PROCESS)
		{
			rect = Rect(Point(rect.x, rect.y), Point(x, y));
			rectState = SET;
			setRectInMask();
			CV_Assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
			showImage(false);
		}
		if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), false);
			lblsState = SET;
			showImage(false);
		}
		break;
	case EVENT_RBUTTONUP:
		if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), true);
			prLblsState = SET;
			showImage(false);
		}
		break;
	case EVENT_MOUSEMOVE:
		if (rectState == IN_PROCESS)
		{
			rect = Rect(Point(rect.x, rect.y), Point(x, y));
			CV_Assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
			showImage(false);
		}
		else if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), false);
			showImage(false);
		}
		else if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), true);
			showImage(false);
		}
		break;
	}
}

int GCApplication::nextIter()
{
	if (isInitialized)
		grabCut(*image, mask, rect, bgdModel, fgdModel, 1);
	else
	{
		if (rectState != SET)
			return iterCount;

		if (lblsState == SET || prLblsState == SET)
			grabCut(*image, mask, rect, bgdModel, fgdModel, 1, GC_INIT_WITH_MASK);
		else
			grabCut(*image, mask, rect, bgdModel, fgdModel, 1, GC_INIT_WITH_RECT);

		isInitialized = true;
	}
	iterCount++;

	bgdPxls.clear(); fgdPxls.clear();
	prBgdPxls.clear(); prFgdPxls.clear();

	return iterCount;
}






typedef pair<float, float> hspair;
class hsVertex{
public:
	hspair position;
	int vertexId;

	hsVertex() {
		vertexId = -2;
		hspair p(-2.0f, -2.0f);
		position = p;
	}

	hsVertex(int vertextNb, hspair position){
		this->vertexId = vertexId;
		this->position = position;
	}

	hsVertex(int vertexNb, float x, float y){
		this->vertexId = vertexNb;
		hspair p(x, y);
		this->position = p;
	}

	hsVertex(const hsVertex& in){
		this->vertexId = in.vertexId;
		this->position = in.position;
	}

	hsVertex& operator=(const hsVertex& in) {
		if (this == &in)
			return *this;

		vertexId = in.vertexId;
		position = in.position;
		return *this;
	}
};
class hsVertexBary{
public:
	float alpha, beta, gamma;
	int vertexId;
	int triangleId;

	hsVertexBary() {
		vertexId = -1;
		triangleId = -1;
		alpha = -1.0f;
		beta = -1.0f;
		gamma = -1.0f;
	}

	hsVertexBary(int vertextNb, int triangleId, float alpha, float beta, float gamma){
		this->vertexId = vertexId;
		this->triangleId = triangleId;
		this->alpha = alpha;
		this->beta = beta;
		this->gamma = gamma;
	}


	hsVertexBary(const hsVertexBary& in){
		this->vertexId = in.vertexId;
		this->triangleId = in.triangleId;
		this->alpha = alpha;
		this->beta = beta;
		this->gamma = gamma;
	}

	hsVertexBary& operator=(hsVertexBary in) {

		cout << "HS WARNING = called on vertexBary" << "\n";
		if (this == &in)
			return *this;

		vertexId = in.vertexId;
		triangleId = in.vertexId;
		alpha = in.alpha;
		beta = in.beta;
		gamma = in.gamma;
		return *this;
	}
};
class hsTriangle{
public:
	hsVertex a, b, c;
	int triangleId;

	hsTriangle(){
		triangleId = -2;
	}

	hsTriangle(int triId, hsVertex a, hsVertex b, hsVertex c) {
		this->triangleId = triangleId;
		this->a = a;
		this->b = b;
		this->c = c;
	}
	hsTriangle(const hsTriangle& in){

		this->triangleId = in.triangleId;
		this->a = in.a;
		this->b = in.a;
		this->c = in.a;
	}
	hsTriangle& operator=(const hsTriangle& in) {
		if (this == &in)
			return *this;

		triangleId = in.triangleId;
		a = in.a;
		b = in.b;
		c = in.c;

		return *this;
	}
};


//VARIABLES:
GCApplication gcapp;
Mat* textureP;
Mat* hsImage;
map<int, hsVertex> vertices;
map<int, hsTriangle> triangles;

const int w = 500;
vector<vector<Point> > contours;
vector<Vec4i> hierarchy;

bool isInTriangle(hsVertexBary& point);


static void on_mouse(int event, int x, int y, int flags, void* param)
{
	gcapp.mouseClick(event, x, y, flags, param);
}

int runExtractAlpha(){

	Mat image = imread("C:\\Users\\hooman\\Desktop\\dressup\\shirtOnWhite.png", IMREAD_COLOR);
	textureP = new Mat(image.size(),image.type());
	hsImage = new Mat(image);

	if (image.empty())
	{
		cout << "\n Durn, couldn't read image filename " << endl;
		return 1;
	}

	help();

	const string winName = "image";
	namedWindow(winName, WINDOW_AUTOSIZE);
	setMouseCallback(winName, on_mouse, 0);

	gcapp.setImageAndWinName(image, winName);
	gcapp.showImage(false);

	for (;;)
	{
		int c = waitKey(0);
		switch ((char)c)
		{
		case '\x1b':
			cout << "Exiting ..." << endl;
			goto exit_main;
		case 'r':
			cout << endl;
			gcapp.reset();
			gcapp.showImage(false);
			break;
		case 'm':
			gcapp.showAlphaMap = true;
			gcapp.showImage(false);
			break;
		case 'k':
			gcapp.showImage(true);
			break;
		//case 'c':
		//	destroyAllWindows;
		//	namedWindow("contours", 1);
		//	FindDrawContours(gcapp.extractedAlphaMap);
		//	break;
		case 'n':
			gcapp.showAlphaMap = false;
			gcapp.showContours = false;
			int iterCount = gcapp.getIterCount();
			cout << "<" << iterCount << "... ";
			int newIterCount = gcapp.nextIter();
			if (newIterCount > iterCount)
			{
				gcapp.showImage(false);
				cout << iterCount << ">" << endl;
			}
			else
				cout << "rect must be determined>" << endl;
			break;
		}
	}

exit_main:
	destroyAllWindows;
	return 0;
}

int createNodeFile(vector<vector<Point> > contours, bool flag) {

	if (contours.size() != 1){
		cout << "contours size not 1\n";
		return -1;
	}

	ofstream myfile;
	myfile.open("boundary.node");
	myfile
		<< "# boundary.node\n"
		<< "#\n"
		<< contours[0].size() << "	" << 2 << "	" << 0 << "	" << 0 << "\n"
		<< "# and here are the points:\n";

	if (flag){
		for (int i = 0; i < contours[0].size(); i++){
			myfile << i << "	" << contours[0][i].x << "	" << contours[0][i].y << "	" << 0 << "	" << 0 << "\n";
		}
	}
	else{
	//	for ()
	}
	myfile.close();
	return 0;
}


void drawTriagnle(hsTriangle& triangle, bool flag = false){

	const Scalar sRED = Scalar(0, 0, 255);
	const Scalar sGreen = Scalar(0, 255, 0);

	hspair a = triangle.a.position;
	hspair b = triangle.b.position;
	hspair c = triangle.c.position;

	unsigned char *input = (unsigned char*)(hsImage->data);
	int i, j;
	double blue, green, red;

	i = a.first;
	j = a.second;
	blue = (double)input[hsImage->step * j + i];
	green = (double)input[hsImage->step * j + i + 1];
	red = (double)input[hsImage->step * j + i + 2];
	Scalar sA(blue, green, red);

	i = b.first;
	j = b.second;
	blue = (double)input[hsImage->step * j + i];
	green = (double)input[hsImage->step * j + i + 1];
	red = (double)input[hsImage->step * j + i + 2];
	Scalar sB(blue, green, red);

	i = c.first;
	j = c.second;
	blue = (double)input[hsImage->step * j + i];
	green = (double)input[hsImage->step * j + i + 1];
	red = (double)input[hsImage->step * j + i + 2];
	Scalar sC(blue, green, red);

	if (flag){
		circle(*textureP, Point(a.first,a.second), 1, sGreen, -1);
		circle(*textureP, Point(b.first, b.second), 1, sGreen, -1);
		circle(*textureP, Point(c.first, c.second), 1, sGreen, -1);

		line(*textureP, Point(a.first, a.second), Point(b.first, b.second), sGreen, 1);
		line(*textureP, Point(a.first, a.second), Point(c.first, c.second), sGreen, 1);
		line(*textureP, Point(b.first, b.second), Point(c.first, c.second), sGreen, 1);
	}
	else{
		circle(*textureP, Point(a.first, a.second), 1, sA, -1);
		circle(*textureP, Point(b.first, b.second), 1, sB, -1);
		circle(*textureP, Point(c.first, c.second), 1, sC, -1);

		line(*textureP, Point(a.first, a.second), Point(b.first, b.second), sRED, 1);
		line(*textureP, Point(a.first, a.second), Point(c.first, c.second), sRED, 1);
		line(*textureP, Point(b.first, b.second), Point(c.first, c.second), sRED, 1);
	}
} 

void hsDrawMesh() {
	for (int i = 0; i < triangles.size(); i++){

		if (i == 3)
			drawTriagnle(triangles[i], true);
		else
			drawTriagnle(triangles[i]);
	}
} 

hsVertexBary convToBary(float x, float y, hsTriangle& tri);

void hsColorTri3() {

	const Scalar S = Scalar(0, 255, 0);
	unsigned char* texturePData = (unsigned char*)(textureP->data);
	
	for (int j = 0; j < textureP->rows; j++){
		for (int i = 0; i < textureP->cols; i++) {

			hsVertexBary temp = convToBary(i, j, triangles[3]);
			if (isInTriangle(temp)) {
			//	circle(*textureP, Point(i, j), 1, S, -1);
				Vec3b color = hsImage->at<Vec3b>(Point(i, j));
				textureP->at<Vec3b>(Point(i, j)) = color;
			}
		}
	}
}

void loadMesh() {

	fstream pointsFile("C:\\Users\\hooman\\Desktop\\dressup\\coordinatesOfNodesForTriangulationOfShirtOnWhite.node", std::ios_base::in);
	int nbPoints;
	pointsFile >> nbPoints;

	for (int i = 0; i < nbPoints; i++) {
		int vertId,xcord, ycord;

		pointsFile >> vertId;
		pointsFile >> xcord;
		pointsFile >> ycord;
		hsVertex temp(vertId,xcord, ycord);
		vertices[vertId] = temp;
	}
	pointsFile.close();


	fstream trianglesFile("C:\\Users\\hooman\\Desktop\\dressup\\triangulatedMeshOfshirtOnWhite.ele", std::ios_base::in);
	int nbTriangles;
	trianglesFile >> nbTriangles;

	for (int i = 0; i < nbTriangles; i++) {
		int triId,a, b, c;

		trianglesFile >> triId;
		trianglesFile >> a;
		trianglesFile >> b;
		trianglesFile >> c;

		hsTriangle temp(triId, vertices[a], vertices[b], vertices[c]);
		triangles[triId] = temp;
	}
	trianglesFile.close();

	hsDrawMesh();
	hsColorTri3();
	imshow("mesh", *textureP);
	imshow("orig", *hsImage);
}

bool isInTriangle(hsVertexBary& point){
	return(point.alpha >= 0.0f && point.alpha <= 1.0f && point.beta >= 0.0f && point.beta <= 1.0f 
		&& point.gamma >= 0.0f && point.gamma <= 1.0f);
}

//TODO
/* it has to take hsTriangle as reference but havent done it yet
bool isInTriangle(float x, float y, hsTriangle tri){
	return isInTriangle(convToBary(x, y, tri));
}*/

hsVertexBary convToBary(float x, float y, hsTriangle& tri){
	float alpha, beta, gama;

	float x1 = tri.a.position.first;
	float y1 = tri.a.position.second;
	float x2 = tri.b.position.first;
	float y2 = tri.b.position.second;
	float x3 = tri.c.position.first;
	float y3 = tri.c.position.second;

	alpha = ( (y2-y3)*(x-x3) + (x3-x2)*(y-y3) ) / ( (y2-y3)*(x1-x3) + (x3-x2)*(y1-y3) );
	beta = ( (y3-y1)*(x-x3) + (x1-x3)*(y-y3) ) / ( (y2-y3)*(x1-x3) + (x3-x2)*(y1-y3) );
	gama = float(1.0) - alpha - beta;

	if ( !(alpha >= 0 && alpha <= 1 && beta >= 0 && beta <= 1 && gama >= 0 && gama <= 1) ){
		//cout << "Error in convToBary\n";
		return hsVertexBary(-1, -1, -1, -1, -1);
	}

	return hsVertexBary(-1, tri.triangleId, alpha, beta, gama);
}

void TestConversionToBary(){
	vector<float> a = { 0.0f, 0.0f };
	vector<float> b = { 0.0f, 6.0f };
	vector<float> c = { 6.0f, 0.0f };
	hsTriangle tri(0, hsVertex(0, hspair(0.0f, 0.0f)), hsVertex(0, hspair(0.0f, 6.0f)), hsVertex(0, hspair(6.0f, 0.0f)));

	for (int i = 1; i < 5; i++){
		hsVertexBary temp = convToBary(1, i, tri);
		cout << "hs: " << temp.alpha << "   " << temp.beta << "    " << temp.gamma << "\n";
		cout << "\n" << "isIn1: " << isInTriangle(temp);
		cout << "\n\n";
	}


	for (int i = 1; i < 4; i++){
		hsVertexBary temp = convToBary(2, i, tri);
		cout << "hs: " << temp.alpha << "   " << temp.beta << "    " << temp.gamma << "\n";
		cout << "\n" << "isIn1: " << isInTriangle(temp);
		cout << "\n\n";
	}

	for (int i = 1; i < 3; i++){
		hsVertexBary temp = convToBary(3, i, tri);
		cout << "hs: " << temp.alpha << "   " << temp.beta << "    " << temp.gamma << "\n";
		cout << "\n" << "isIn1: " << isInTriangle(temp);
		cout << "\n\n";
	}

	hsVertexBary temp = convToBary(4, 1, tri);
	cout << "hs: " << temp.alpha << "   " << temp.beta << "    " << temp.gamma << "\n";
	cout << "\n" << "isIn1: " << isInTriangle(temp);
	cout << "\n\n";
}

hsVertex convBack(hsVertexBary in, hsTriangle tri){
	float x = in.alpha*tri.a.position.first + in.beta*tri.b.position.first + in.gamma*tri.c.position.first;
	float y = in.alpha*tri.a.position.second + in.beta*tri.b.position.second + in.gamma*tri.c.position.second;
	return hsVertex(in.vertexId, x, y);
}

hsTriangle findTriangle(int x, int y){

	for (int i = 0; i < triangles.size(); i++){
		hsVertexBary temp = convToBary(x, y, triangles[i]);
		if (isInTriangle(temp))
			return triangles[i];
	}

	cout << "findTriangle erro\n";
	return hsTriangle();
}




int main() {

	if (runExtractAlpha() == 0){

		vector<vector<Point> > contours0;
		findContours(*gcapp.extractedAlphaMap, contours0, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

		contours.resize(contours0.size());
		for (size_t k = 0; k < contours0.size(); k++)
			approxPolyDP(Mat(contours0[k]), contours[k], 3, true);

		Mat cnt_img = Mat::zeros(gcapp.extractedAlphaMap->size(), CV_8UC1);


		//Levels <= 0 all look the same for the shirt. But >0 obviously are smaller stuff
		int levels = 0;
		//*********************** TO successfully create the mesh the level should be 0 so that we get only the outer region.
		// so that out contours vector has a size of 1 (we need to have only 1 contour which is the boundary of our image)
		cout <<  "contours size is: " << contours.size() << "\n";
		cout << "first contours size is: " << contours[0].size() << "\n";
		

		//The following 2 lines is the check in drawContour that throws an error if _levels doesnt pass the assert
	//	size_t ncontours = contours.size();
	//	CV_Assert(0 <= _levels && _levels < (int)ncontours);
	
		//in elow, the 3rd param levels used to be _levels <= 0 ? 3 : -1
		drawContours(cnt_img, contours, levels, Scalar(128, 255, 255),
			3, 16, hierarchy, std::abs(levels));

		imshow("contours", cnt_img);

		if (createNodeFile(contours, true) == 0)
			cout << "Successfully created node file\n";
		else
			cout << "failed to create node file\n";

		waitKey();

		//Draw the mesh with image on it
		loadMesh();
		waitKey();

	}

	delete gcapp.extractedAlphaMap; 
}


