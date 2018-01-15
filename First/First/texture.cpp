#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <freeglut.h>

#include <math.h>
#include <map>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

class hsTrianVertex{
public:
	int vertexNb;
	Point position;

	hsTrianVertex() {
		vertexNb = -1;
		Point p(-1, -1);
		position = p;
	}

	hsTrianVertex(int vertextNb, Point position){
		this->vertexNb = vertexNb;
		this->position = position;
	}

	hsTrianVertex(int vertexNb, int x, int y){
		this->vertexNb = vertexNb;
		Point p(x, y);
		this->position = p;
	}

	hsTrianVertex(const hsTrianVertex& in){
		this->vertexNb = in.vertexNb;
		this->position = in.position;
	}

	hsTrianVertex& operator=(const hsTrianVertex& in) {
		if (this == &in)
			return *this;

		vertexNb = in.vertexNb;
		position = in.position;
		return *this;
	}
};

class hsTriangle{
public:
	int triangleNb;
	hsTrianVertex* vertices = new hsTrianVertex[3];

	hsTriangle() {
		triangleNb = -1;
		vertices = NULL;
	}

	hsTriangle(int triangleNb, hsTrianVertex a, hsTrianVertex b, hsTrianVertex c){
		this->triangleNb = triangleNb;
		this->vertices[0] = a;
		this->vertices[1] = a;
		this->vertices[2] = a;
	}

	hsTriangle(const hsTriangle& in) {
		this->triangleNb = in.triangleNb;
		this->vertices[0] = in.vertices[0];
		this->vertices[1] = in.vertices[1];
		this->vertices[2] = in.vertices[2];
	}
};
class hsTextureMapping{
public:
	map<int, hsTrianVertex>* trianVerticesP = new map<int, hsTrianVertex>();
	map<int, hsTriangle>* trianglesP = new map<int, hsTriangle>();
	Mat* inImage;
	Mat* outImage;

	void drawMesh() {

		//
		Mat texture = imread("C:\\Users\\hooman\\Desktop\\dressup\\shirtOnWhite.png", IMREAD_COLOR);

		Mat out(texture.size(), CV_8UC1);

		fstream pointsFile("C:\\Users\\hooman\\Desktop\\dressup\\coordinatesOfNodesForTriangulationOfShirtOnWhite.node", std::ios_base::in);
		int nbPoints;
		pointsFile >> nbPoints;

		for (int i = 0; i < nbPoints; i++) {
			int pointNb, xcord, ycord;

			pointsFile >> pointNb;
			pointsFile >> xcord;
			pointsFile >> ycord;

			hsTrianVertex temp(pointNb, xcord, ycord);
			(*trianVerticesP)[pointNb] = temp;
		}
		pointsFile.close();


		fstream trianglesFile("C:\\Users\\hooman\\Desktop\\dressup\\triangulatedMeshOfshirtOnWhite.ele", std::ios_base::in);
		int nbTriangles;
		trianglesFile >> nbTriangles;

		for (int i = 0; i < nbTriangles; i++) {
			int triNb, a, b, c;

			trianglesFile >> triNb;
			trianglesFile >> a;
			trianglesFile >> b;
			trianglesFile >> c;

			hsTriangle temp(triNb, trianVerticesP->at(a), trianVerticesP->at(b), trianVerticesP->at(c));
			(*trianglesP)[triNb] = temp;
			drawTriagnle(temp, out, texture);
		}

		trianglesFile.close();
		showImages(out, texture);
	}

	void drawTriagnle(hsTriangle& triangle, Mat& out, Mat& texture){

		const Scalar sRED = Scalar(0, 0, 255);
		Scalar scalars[3];

		Point A(triangle.vertices[0].position);
		Point B(triangle.vertices[1].position);
		Point C(triangle.vertices[2].position);

		unsigned char *input = (unsigned char*)(texture.data);
		int i, j;
		double blue, green, red;

		for (int k = 0; k < 3; k++) {

			i = triangle.vertices[k].position.x;
			j = i = triangle.vertices[k].position.y;
			blue = (double)input[texture.step * j + i];
			green = (double)input[texture.step * j + i + 1];
			red = (double)input[texture.step * j + i + 2];
			scalars[k] = Scalar(blue, green, red);
		}

		circle(out, A, 5, scalars[0], -1);
		circle(out, B, 5, scalars[1], -1);
		circle(out, C, 5, scalars[2], -1);

		line(out, A, B, sRED, 1);
		line(out, A, C, sRED, 1);
		line(out, B, C, sRED, 1);
	}

	void showImages(Mat& out, Mat& texture) {
		imshow("inImage", texture);
		imshow("outImage", out);
		waitKey();
	}
	~hsTextureMapping(){

		if (trianVerticesP != NULL && trianglesP != NULL)
		trianVerticesP->clear();
		trianglesP->clear();
	}
};


int main() {
	hsTextureMapping hs;
	hs.drawMesh();
	return 0;
}