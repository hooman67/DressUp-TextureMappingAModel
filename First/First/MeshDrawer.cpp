// HS WORKING COPY: Draw a triangulated mesh by reading the .node and .ele files

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <map>
#include <iostream>
#include <fstream>


using namespace cv;
using namespace std;

void drawTriagnle(Mat& img, pair<int, int> a, pair<int, int> b, pair<int, int> c){
	const Scalar RED = Scalar(0, 0, 255);

	Point A(a.first, a.second);
	Point B(b.first, b.second);
	Point C(c.first, c.second);

	circle(img, A, 2, RED, 1);
	circle(img, B, 2, RED, 1);
	circle(img, C, 2, RED, 1);
	line(img, A, B, RED, 1);
	line(img, A, C, RED, 1);
	line(img, B, C, RED, 1);

}

void drawMesh(string fileName = "") {

	Mat out(500, 500, CV_8UC1);
	map<int, pair<int, int>> points;

	fstream pointsFile("C:\\Users\\hooman\\Desktop\\dressup\\coordinatesOfNodesForTriangulationOfShirtOnWhite.node", std::ios_base::in);
	int nbPoints;
	pointsFile >> nbPoints;

	for (int i = 0; i < nbPoints; i++) {
		int xcord, ycord;

		//skipping the line #
		pointsFile >> xcord;

		pointsFile >> xcord;
		pointsFile >> ycord;
		pair<int, int> temp(xcord, ycord);
		points[i] = temp;
	}
	pointsFile.close();


	fstream trianglesFile("C:\\Users\\hooman\\Desktop\\dressup\\triangulatedMeshOfshirtOnWhite.ele", std::ios_base::in);
	int nbTriangles;
	trianglesFile >> nbTriangles;

	for (int i = 0; i < nbTriangles; i++) {
		int a, b, c;

		//skipping the triangle #
		trianglesFile >> a;

		trianglesFile >> a;
		trianglesFile >> b;
		trianglesFile >> c;

		drawTriagnle(out, points[a], points[b], points[c]);
	}
	trianglesFile.close();

	imshow("mesh", out);
}

int main() {

	drawMesh();
	waitKey();
	return 0;
}

