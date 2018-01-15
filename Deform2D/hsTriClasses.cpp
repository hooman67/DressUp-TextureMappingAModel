#include "first.h"

class hsVertex{
public:
	cv::Point2f position;
	int vertexId;

	hsVertex() {
		vertexId = -2;
		cv::Point2f p(-2.0f, -2.0f);
		position = p;
	}

	hsVertex(int vertextNb, cv::Point2f position){
		this->vertexId = vertexId;
		this->position = position;
	}

	hsVertex(int vertexNb, float x, float y){
		this->vertexId = vertexNb;
		cv::Point2f p(x, y);
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
	Vector<cv::Point2f> points;
	Eigen::Vector3f acoefs;
	Eigen::Vector3f bcoefs;

	double rotCos = -10;
	double rotSin = -10;

	float trX, trY;

	hsTriangle(){
		triangleId = -2;
	}

	hsTriangle(int triId, hsVertex a, hsVertex b, hsVertex c) {
		this->triangleId = triangleId;
		this->a = a;
		this->b = b;
		this->c = c;
	}

	//One of these two does not work so I cannot pass a triagnle without reference.
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



	static hsVertexBary convToBary(float x, float y, hsTriangle& tri){
		float alpha, beta, gama;

		float x1 = tri.a.position.x;
		float y1 = tri.a.position.y;
		float x2 = tri.b.position.x;
		float y2 = tri.b.position.y;
		float x3 = tri.c.position.x;
		float y3 = tri.c.position.y;

		alpha = ((y2 - y3)*(x - x3) + (x3 - x2)*(y - y3)) / ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
		beta = ((y3 - y1)*(x - x3) + (x1 - x3)*(y - y3)) / ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
		gama = float(1.0) - alpha - beta;

		if (!(alpha >= 0 && alpha <= 1 && beta >= 0 && beta <= 1 && gama >= 0 && gama <= 1)){
			//cout << "Error in convToBary\n";
			return hsVertexBary(-1, -1, -1, -1, -1);
		}

		return hsVertexBary(-1, tri.triangleId, alpha, beta, gama);
	}

	int updateCoefs(hsTriangle& newPos){
		Eigen::Matrix3f A;
		Eigen::Vector3f b1;
		Eigen::Vector3f b2;
		A << 1, this->a.position.y, this->a.position.x, 1, this->b.position.y, this->b.position.x,
			1, this->c.position.y, this->c.position.x;
		b1 << newPos.a.position.x, newPos.b.position.x, newPos.c.position.x;
		b2 << newPos.a.position.x, newPos.b.position.x, newPos.c.position.x;



		acoefs = A.colPivHouseholderQr().solve(b1);
		bcoefs = A.colPivHouseholderQr().solve(b2);

		//cout << "Here is the matrix A:\n" << A << endl;
		//cout << "Here is the vector b:\n" << b1 << endl;
		//cout << "The solution is1:\n" << acoefs << endl;
		//cout << "The solution is2:\n" << bcoefs << endl;

		return 0;
	}

	cv::Point2f getTransformed1(int indexOfPoint){
		float u = acoefs(0) + acoefs(1)*points[indexOfPoint].y + acoefs(2)*points[indexOfPoint].x;
		float v = bcoefs(0) + bcoefs(1)*points[indexOfPoint].y + bcoefs(2)*points[indexOfPoint].x;
		return cv::Point2f(u, v);
	}

	//TODO
	/* it has to take hsTriangle as reference but havent done it yet
	bool isInTriangle(float x, float y, hsTriangle tri){
	return isInTriangle(convToBary(x, y, tri));
	}*/

	static bool isInTriangle(hsVertexBary& point){
		return(point.alpha >= 0.0f && point.alpha <= 1.0f && point.beta >= 0.0f && point.beta <= 1.0f
			&& point.gamma >= 0.0f && point.gamma <= 1.0f);
	}

	void TestConversionToBary(){
		vector<float> a = { 0.0f, 0.0f };
		vector<float> b = { 0.0f, 6.0f };
		vector<float> c = { 6.0f, 0.0f };
		hsTriangle tri(0, hsVertex(0, cv::Point2f(0.0f, 0.0f)), hsVertex(0, cv::Point2f(0.0f, 6.0f)), hsVertex(0, cv::Point2f(6.0f, 0.0f)));

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
		float x = in.alpha*tri.a.position.x + in.beta*tri.b.position.x + in.gamma*tri.c.position.x;
		float y = in.alpha*tri.a.position.y + in.beta*tri.b.position.y + in.gamma*tri.c.position.y;
		return hsVertex(in.vertexId, x, y);
	}





	cv::Point2f getTransformed(hsTriangle& newTri, cv::Point2f p){
		hsVertexBary temp = convToBary(p.x, p.y, *this);
		float u = temp.alpha*newTri.a.position.x + temp.beta*newTri.b.position.x + temp.gamma*newTri.c.position.x;
		float v = temp.alpha*newTri.a.position.y + temp.beta*newTri.b.position.y + temp.gamma*newTri.c.position.y;
		return cv::Point2f(u, v);
	}

	cv::Point2f getTransformed(cv::Point2f p){
		float u = acoefs(0) + acoefs(1)*p.y + acoefs(2)*p.x;
		float v = bcoefs(0) + bcoefs(1)*p.y + bcoefs(2)*p.x;
		return cv::Point2f(u, v);
	}

	cv::Point2f compute2DPolygonCentroid(const Point2f* vertices, int vertexCount)
	{
		Point centroid = { 0, 0 };
		double signedArea = 0.0;
		double x0 = 0.0; // Current vertex X
		double y0 = 0.0; // Current vertex Y
		double x1 = 0.0; // Next vertex X
		double y1 = 0.0; // Next vertex Y
		double a = 0.0;  // Partial signed area

		// For all vertices except last
		int i = 0;
		for (i = 0; i<vertexCount - 1; ++i)
		{
			x0 = vertices[i].x;
			y0 = vertices[i].y;
			x1 = vertices[i + 1].x;
			y1 = vertices[i + 1].y;
			a = x0*y1 - x1*y0;
			signedArea += a;
			centroid.x += (x0 + x1)*a;
			centroid.y += (y0 + y1)*a;
		}

		// Do last vertex separately to avoid performing an expensive
		// modulus operation in each iteration.
		x0 = vertices[i].x;
		y0 = vertices[i].y;
		x1 = vertices[0].x;
		y1 = vertices[0].y;
		a = x0*y1 - x1*y0;
		signedArea += a;
		centroid.x += (x0 + x1)*a;
		centroid.y += (y0 + y1)*a;

		signedArea *= 0.5;
		centroid.x /= (6.0*signedArea);
		centroid.y /= (6.0*signedArea);

		return centroid;
	}
	cv::Point2f get2DPolygonCentroid(){
		Point2f vec[] = { a.position, b.position, c.position };
		return compute2DPolygonCentroid(vec, 3);
	}

	cv::Point2f findRotation(cv::Point2f orig, cv::Point2f rotated){

		float x = orig.x;
		float y = orig.y;
		float u = rotated.x;
		float v = rotated.y;

		float cost, sint;

		if (x == 0.0){
			cost = 1.0;
			sint = 0.0;
			return cv::Point2f(-10,-10);
		}

		sint = ((y / x)*u - v) / (y*y / x + x);
		cost = (u - y*rotSin) / x;
		
		return cv::Point2f(cost, sint);
	}

	void setRotations(float cost, float sint){
		this->rotCos = cost;
		this->rotSin = sint;
	}

	cv::Point2f getRotation(hsTriangle& orig){
		cv::Point2f rot1, rot2, rot3;

		//To centre orig at (0,0):
		cv::Point2f origCenter = orig.get2DPolygonCentroid();

		//To centre this at (0,0)
		cv::Point2f thisCenter = get2DPolygonCentroid();


		rot1 = findRotation(orig.a.position - origCenter, a.position - thisCenter);
		rot2 = findRotation(orig.b.position - origCenter, b.position - thisCenter);
 		rot3 = findRotation(orig.c.position - origCenter, c.position - thisCenter);

		if (round(rot1.x) == round(rot2.x) && round(rot2.x) == round(rot3.x) && round(rot1.y) == round(rot2.y) && round(rot2.y) == round(rot3.y)){
			return rot1;
		}
		else
			return cv::Point2f(-10, -10);

	}

	cv::Point2f rotate(cv::Point2f in){
		double x = in.x*rotCos + in.y*rotSin;
		float y = -1 * in.x * rotSin + in.y*rotCos;
		return cv::Point2f(x, y);
	}

	bool rotate(){
		if (rotCos == -10 || rotSin == -10){
			cout << "set the rotations first (-10,-10) returned \n";
			return false;
		}

		cv::Point2f center = get2DPolygonCentroid();

		cv::Point2f ra = rotate(a.position - center);
		cv::Point2f rb = rotate(b.position - center);
		cv::Point2f rc = rotate(c.position - center);

		a.position = ra + center;
		b.position = rb + center;
		c.position = rc + center;

		return true;
	}

	static void rotationTest(Mat* Image){
		Point2f a(100, 100);
		Point2f b(100, 150);
		Point2f c(200, 150);
		Point2f shift(50, 50);

		hsTriangle tr(-1, hsVertex(0, a), hsVertex(1, b), hsVertex(2, c));
		hsTriangle trOrig(-1, hsVertex(0, a + shift), hsVertex(1, b + shift), hsVertex(2, c + shift));

		Scalar G(0, 255, 0);
		Scalar R(0, 0, 255);

		line(*Image, tr.a.position, tr.b.position, G, 2);
		line(*Image, tr.b.position, tr.c.position, G, 2);
		line(*Image, tr.a.position, tr.c.position, G, 2);


		tr.setRotations(0.5, 0.5);

		tr.rotate();


		line(*Image, tr.a.position, tr.b.position, R, 2);
		line(*Image, tr.b.position, tr.c.position, R, 2);
		line(*Image, tr.a.position, tr.c.position, R, 2);

		Point2f rotation = tr.getRotation(trOrig);

		cout << "cos= " << rotation.x << "  sin= " << rotation.y << "\n";
		imshow("in", *Image);
		waitKey();
	}

	void shiftForward(float x, float y){
		Point2f offset(x, y);
		a.position += offset;
		b.position += offset;
		c.position += offset;
	}
};