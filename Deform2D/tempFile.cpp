#include "Deform2D.h"



//2nd deform methods
void MakeSquareMesh()
{
	m_mesh.Clear();

	const unsigned int nRowLen = 5;

	float fYStep = 2.0f / (float)(nRowLen - 1);
	float fXStep = 2.0f / (float)(nRowLen - 1);

	for (unsigned int yi = 0; yi < nRowLen; ++yi) {
		float fY = -1.0f + (float)yi * fYStep;
		for (unsigned int xi = 0; xi < nRowLen; ++xi) {
			float fX = -1.0f + (float)xi * fXStep;

			//Eigen::Vector3f vVert(fX,fY,0);
			//hsx
			rmsmesh::Point vVert;
			vVert.init(fX, fY, 0);
			m_mesh.AppendVertexData(vVert);
		}
	}

	for (unsigned int yi = 0; yi < nRowLen - 1; ++yi) {
		unsigned int nRow1 = yi * nRowLen;
		unsigned int nRow2 = (yi + 1) * nRowLen;

		for (unsigned int xi = 0; xi < nRowLen - 1; ++xi) {

			unsigned int nTri1[3] = { nRow1 + xi, nRow2 + xi + 1, nRow1 + xi + 1 };
			unsigned int nTri2[3] = { nRow1 + xi, nRow2 + xi, nRow2 + xi + 1 };

			m_mesh.AppendTriangleData(nTri1);
			m_mesh.AppendTriangleData(nTri2);
		}
	}

	InitializeDeformedMesh();
	hsDeformerShowImage();
}

/*
void UpdateScale()
{
glGetIntegerv(GL_VIEWPORT, m_nViewport);
float fViewCenterX = (float)m_nViewport[2] / 2;
float fViewCenterY = (float)m_nViewport[3] / 2;

m_mesh.GetBoundingBox(m_bounds);
m_vTranslate.X() = fViewCenterX - 0.5f * (m_bounds[0] + m_bounds[1]);
m_vTranslate.Y() = fViewCenterY - 0.5f * (m_bounds[2] + m_bounds[3]);

float fWidth = m_bounds[1] - m_bounds[0];
float fHeight = m_bounds[3] - m_bounds[2];
float fSizeObject = std::max(fWidth, fHeight);
float fSizeView = std::min(m_nViewport[2], m_nViewport[3]);

m_fScale = 0.5f * (fSizeView / fSizeObject);
}*/
Wml::Vector2f ViewToWorld(const Wml::Vector2f & vPoint)
{
	return (vPoint - m_vTranslate) / m_fScale;
}
Wml::Vector2f WorldToView(const Wml::Vector2f & vPoint)
{
	return (vPoint * m_fScale) + m_vTranslate;
}

void InitializeDeformedMesh()
{
	m_deformedMesh.Clear();

	unsigned int nVerts = m_mesh.GetNumVertices();
	for (unsigned int i = 0; i < nVerts; ++i) {
		Eigen::Vector3f vVertex;
		m_mesh.GetVertex(i, vVertex);
		rmsmesh::Point vVert;
		vVert.init(vVertex.x(), vVertex.y(), 0);
		m_deformedMesh.AppendVertexData(vVert);
	}

	unsigned int nTris = m_mesh.GetNumTriangles();
	for (unsigned int i = 0; i < nTris; ++i) {
		unsigned int nTriangle[3];
		m_mesh.GetTriangle(i, nTriangle);
		m_deformedMesh.AppendTriangleData(nTriangle);
	}

	m_deformer.InitializeFromMesh(&m_mesh);
	InvalidateConstraints();
}
void UpdateDeformedMesh()
{
	ValidateConstraints();
	m_deformer.UpdateDeformedMesh(&m_deformedMesh, true);
}
void InvalidateConstraints()
{
	m_bConstraintsValid = false;
}
void ValidateConstraints()
{
	if (m_bConstraintsValid)
		return;

	unsigned int nConstraints = m_vSelected.size();
	std::set<unsigned int>::iterator cur(m_vSelected.begin()), end(m_vSelected.end());
	while (cur != end) {
		unsigned int nVertex = *cur++;
		Eigen::Vector3f vVertex;
		m_deformedMesh.GetVertex(nVertex, vVertex);
		m_deformer.SetDeformedHandle(nVertex, Eigen::Vector2f(vVertex.x(), vVertex.y()));
	}

	m_deformer.ForceValidation();

	m_bConstraintsValid = true;
}
/*original deform2D
unsigned int FindHitVertex(float nX, float nY)
{
unsigned int nVerts = m_deformedMesh.GetNumVertices();
for (unsigned int i = 0; i < nVerts; ++i) {

Eigen::Vector3f vVertex;
m_deformedMesh.GetVertex(i, vVertex);
Wml::Vector2f vView = WorldToView(Wml::Vector2f(vVertex.x(), vVertex.y()));
float fX = vView.X();
float fY = vView.Y();

double fDist = sqrt(
(double)((nX - fX)*(nX - fX) + (nY - fY)*(nY - fY)));
if (fDist < 5)
return i;
}

return std::numeric_limits<unsigned int>::max();
}*/


void hsDeformerOnMouse(int event, int x, int y, int flags, void*){
	unsigned int nHit;
	// TODO add bad args check
	switch (event)	{
	case EVENT_LBUTTONDOWN:

		m_nSelected = m_deformer.FindHitVertex((float)x, (float)(y));
		break;
	case EVENT_RBUTTONDOWN: // set GC_PR_BGD(GC_PR_FGD) labels
		break;

	case EVENT_LBUTTONUP:
		m_nSelected = std::numeric_limits<unsigned int>::max();
		hsDeformerShowImage();

		break;
	case EVENT_RBUTTONUP:
		//	unsigned int nHit = FindHitVertex((float)x, (float)(m_nViewport[3] - 1 - y));
		nHit = m_deformer.FindHitVertex((float)x, (float)(y));
		if (nHit != std::numeric_limits<unsigned int>::max()) {

			if (m_vSelected.find(nHit) == m_vSelected.end())
				m_vSelected.insert(nHit);
			else {
				m_vSelected.erase(nHit);
				m_deformer.RemoveHandle(nHit);

				// restore position
				Eigen::Vector3f vVertex;
				m_mesh.GetVertex(nHit, vVertex);
				m_deformedMesh.SetVertex(nHit, vVertex);
			}

			InvalidateConstraints();
			hsDeformerShowImage();
		}
		break;
	case EVENT_MOUSEMOVE:
		if (m_nSelected != std::numeric_limits<unsigned int>::max()) {
			Eigen::Vector3f newPos(x, y, 0.0f);
			m_deformedMesh.SetVertex(m_nSelected, newPos);
			InvalidateConstraints();
			hsDeformerShowImage();
		}
		break;
	}
}
void hsDeformerShowImage(bool drawMesh){

	UpdateDeformedMesh();


	//	Mat temp = imread(globalImageName, IMREAD_COLOR);
	//	hsDeformerImage = new Mat(temp);
	hsDeformerImage = new Mat(600, 600, CV_8UC3);

	const Scalar RED = Scalar(0, 0, 255);
	const Scalar BLU = Scalar(255, 0, 0);

	//HS this is where we retreive the vertices****************
	const std::vector<rmsmesh::Vertex>& verts = m_deformer.getDeformedVerts();
	const unsigned int np = verts.size();

	for (int i = 0; i < np; i++)
	{
		//HS this is where we draw the vertices IN THE MESH ********
		circle(*hsDeformerImage, cv::Point(verts[i].vPosition[0], verts[i].vPosition[1]), 2, BLU, -1);
	}

	// this is drawing triangles edges ******************HS
	const std::vector<rmsmesh::Triangle>& vTriangles = m_deformer.getTriangles();
	const unsigned int nt = vTriangles.size();
	for (int i = 0; i < nt; i++)
	{
		const unsigned int * tri = vTriangles[i].nVerts;

		// This is freeGlut drawing lines. Disable it no edges drawn just the vertices
		cv::line(*hsDeformerImage, cv::Point(verts[tri[0]].vPosition[0], verts[tri[0]].vPosition[1]),
			cv::Point(verts[tri[1]].vPosition[0], verts[tri[1]].vPosition[1]), BLU, 1);

		cv::line(*hsDeformerImage, cv::Point(verts[tri[1]].vPosition[0], verts[tri[1]].vPosition[1]),
			cv::Point(verts[tri[2]].vPosition[0], verts[tri[2]].vPosition[1]), BLU, 1);

		cv::line(*hsDeformerImage, cv::Point(verts[tri[2]].vPosition[0], verts[tri[2]].vPosition[1]),
			cv::Point(verts[tri[0]].vPosition[0], verts[tri[0]].vPosition[1]), BLU, 1);
	}

	std::set<unsigned int>::iterator cur(m_vSelected.begin()), end(m_vSelected.end());
	while (cur != end) {
		unsigned int nSelected = *cur++;
		circle(*hsDeformerImage, cv::Point(verts[nSelected].vPosition[0], verts[nSelected].vPosition[1]), 2, RED, -1);
	}

	cv::imshow(hsDeformerWinName, *hsDeformerImage);
}



//My methods
void hsGluthelp(){
	std::cout << "\nHSGlut Menu\n"
		"\nHot keys: \n"
		"\th - print HELP\n"
		"\tESC - Exit\n"
		"\tq - Load vertices from boundary file\n"
		"\ta - Build Mesh\n"
		"\tu - fall to postRedisplay\n"
		"\tp - Select Control points based on shirt skeleton\n"
		"\tP - Move Control points based on body skeleton\n"
		"\tl - load mesh from Glut and Triangle points;\n"
		"\tL - show deformed/ placed image\n"
		"\tv - Clear selected control points\n"
		"\ts - save mesh file\n"
		"\tc - Clear every thing\n" << endl;
}
void saveMeshFile(){

	ofstream myfile;
	myfile.open("hsDeformedNodes.node");
	myfile
		<< "# hsDeformedNodes.node\n"
		<< "#\n"
		<< vertices.size() << "	" << 2 << "	" << 0 << "	" << 0 << "\n"
		<< "# and here are the points:\n";

	for (int i = 0; i < vertices.size(); i++){
		myfile << vertices[i].vertexId << "	" << vertices[i].position.x << "	" << vertices[i].position.y << "	" << 0 << "\n";
	}

	myfile.close();

	ofstream myfile1;
	myfile.open("hsDeformedTries.node");
	myfile
		<< "# hsDeformedTries.node\n"
		<< "#\n"
		<< vertices.size() << "	" << 3 << "	" << 0 << "	" << 0 << "\n"
		<< "# and here are the points:\n";

	for (int i = 0; i < triangles.size(); i++){
		myfile << triangles[i].triangleId << "	" << triangles[i].a.vertexId << "	" <<
			triangles[i].b.vertexId << "	" << triangles[i].c.vertexId << "   " << 0 << "\n";
	}

	std::cout << "saved mesh to hsDeformedNodes.node\n";
	myfile1.close();
}
void on_mouse(int event, int x, int y, int flags, void* param)
{
	gcapp.mouseClick(event, x, y, flags, param);
}
void skeleton_on_mouse(int event, int x, int y, int flags, void* param)
{
	skeletonObj.skeleton_mouseClick(event, x, y, flags, param);
}
void resetGlobalImage(){

	String defaultInput = "extractedImage.png";
	String input;
	std::cout << "Enter full image name or d for default: ";
	cin >> input;
	globalImageName = (input != "d") ? input : defaultInput;
	std::cout << "GlobalImage Set to: " << globalImageName << "\n";
	return;
}
void saveExtractedImage(){
	Mat* temp;
	//	gcapp.saveOutputImage(temp);
	//	imwrite("extractedImage.png", *temp);
	imwrite("alphaMap.png", gcapp.binMask);
	std::cout << "successfully saved extractedImage.png\n";
}
int createNodeFile(vector<vector<cv::Point> > contours) {

	if (contours.size() == 0){
		std::cout << "No contours created\n";
		return -1;
	}

	string fileName;
	std::cout << "Enter s for shirt contour or b for body contour: ";
	char temp;
	cin >> temp;
	if (temp == 's'){
		fileName = "shirtCreatedBoundary.node";
		std::cout << "shirt\n";
	}
	else if (temp == 'b') {
		fileName = "bodyCreatedBoundaryFull.node";
		std::cout << "body\n";
	}
	else if (temp == 'x') {
		std::cout << "exiting without saving the file\n";
		return -1;
	}

	ofstream myfile;
	myfile.open(fileName);
	myfile
		<< "# createdBoundary.node\n"
		<< "#\n"
		<< contours[0].size() << "	" << 2 << "	" << 0 << "	" << 0 << "\n"
		<< "# and here are the points:\n";

	for (int i = 0; i < contours[0].size(); i++){
		myfile << i << "	" << contours[0][i].x << "	" << contours[0][i].y << "	" << 0 << "	" << 0 << "\n";
	}
	myfile.close();
	return 0;
}

void hsColorTri3() {

	const Scalar S = Scalar(0, 255, 0);

	for (int j = 0; j < skeletonOutput->rows; j++){
		for (int i = 0; i < skeletonOutput->cols; i++) {

			hsVertexBary temp = hsTriangle::convToBary(i, j, triangles[3]);
			if (hsTriangle::isInTriangle(temp)) {
				//	circle(*skeletonOutput, Point(i, j), 1, S, -1);
				Vec3b color = skeletonInputForTexture->at<Vec3b>(cv::Point(i, j));
				skeletonOutput->at<Vec3b>(cv::Point(i, j)) = color;
			}
		}
	}
}
void hsColorTrianglesOnMesh(Mat* inputTextureFile, Mat* outputImage){

	for (int tri = 0; tri < triangles.size(); tri++){

		for (int i = 0; i < triangles[tri].points.size(); i++){
			cv::Point temp = triangles[tri].points[i];
			Vec3b color = inputTextureFile->at<Vec3b>(temp);
			cv::Point temp1 = triangles[tri].getTransformed(deformedTriangles[tri], temp);
			if (!(color[0] == char(0) && color[1] == char(0) && color[2] == char(0)))
				outputImage->at<Vec3b>(temp1) = color;
		}
	}
}
void hsColorTrianglesOnMeshBC(Mat* inputTextureFile, Mat* outputImage){

	std::cout << "coloring triangles BC\n";
	if (triangles.size() != deformedTriangles.size()){
		std::cout << "deformed triangles size != triangles size\n";
		return;
	}

	for (int tri = 0; tri < triangles.size(); tri++){

		bcbsi.bicubic_algo_singleSquare_onlyInsideTriangle(inputTextureFile, outputImage, triangles[tri], deformedTriangles[tri]);
	}
	std::cout << "FINISHED coloring triangles BC\n";
}

void drawTriagnle(hsTriangle& triangle, bool drawIngreen, Mat*& output){

	const Scalar sRED = Scalar(0, 0, 255);
	const Scalar sGreen = Scalar(0, 255, 0);
	const Scalar YELLO = Scalar(0, 255, 255);

	Point2f a = triangle.a.position;
	Point2f b = triangle.b.position;
	Point2f c = triangle.c.position;



	if (drawIngreen){
		circle(*output, cv::Point(a.x, a.y), 1, sGreen, -1);
		circle(*output, cv::Point(b.x, b.y), 1, sGreen, -1);
		circle(*output, cv::Point(c.x, c.y), 1, sGreen, -1);

		cv::line(*output, cv::Point(a.x, a.y), cv::Point(b.x, b.y), sGreen, 1);
		cv::line(*output, cv::Point(a.x, a.y), cv::Point(c.x, c.y), sGreen, 1);
		cv::line(*output, cv::Point(b.x, b.y), cv::Point(c.x, c.y), sGreen, 1);
	}
	else{

		cv::line(*output, cv::Point(a.x, a.y), cv::Point(b.x, b.y), sRED, 1);
		cv::line(*output, cv::Point(a.x, a.y), cv::Point(c.x, c.y), sRED, 1);
		cv::line(*output, cv::Point(b.x, b.y), cv::Point(c.x, c.y), sRED, 1);
	}
}
void hsDrawMesh(Mat*& output) {

	for (int i = 0; i < triangles.size(); i++){

		if (output != NULL)
			drawTriagnle(triangles[i], true, output);
		else
			drawTriagnle(triangles[i], true);

	}
}

void loadMesh() { //Load mesh from custom node nad .ele files

	fstream pointsFile("C:\\Users\\hooman\\Desktop\\dressup\\coordinatesOfNodesForTriangulationOfShirtOnWhite.node", std::ios_base::in);
	int nbPoints;
	pointsFile >> nbPoints;

	for (int i = 0; i < nbPoints; i++) {
		int vertId, xcord, ycord;

		pointsFile >> vertId;
		pointsFile >> xcord;
		pointsFile >> ycord;
		hsVertex temp(vertId, xcord, ycord);
		vertices[vertId] = temp;
	}
	pointsFile.close();


	fstream trianglesFile("C:\\Users\\hooman\\Desktop\\dressup\\triangulatedMeshOfshirtOnWhite.ele", std::ios_base::in);
	int nbTriangles;
	trianglesFile >> nbTriangles;

	for (int i = 0; i < nbTriangles; i++) {
		int triId, a, b, c;

		trianglesFile >> triId;
		trianglesFile >> a;
		trianglesFile >> b;
		trianglesFile >> c;

		hsTriangle temp(triId, vertices[a], vertices[b], vertices[c]);
		triangles[triId] = temp;
	}
	trianglesFile.close();

	hsDrawMesh();
	//hsColorTri3();
	cv::imshow("mesh", *skeletonOutput);
	cv::imshow("orig", *skeletonInputForTexture);
}
void loadGlutMesh() {

	Mat temp = imread(globalImageName, IMREAD_COLOR);
	skeletonInputForTexture = new Mat(temp);
	//	skeletonOutput = new Mat(temp.size(), temp.type());
	skeletonOutput = new Mat(temp);

	PILoadVerticesAndTrianglesFromDeformObj();


	hsLoadTrianglePoints(skeletonOutput);
	hsDrawMesh();
	std::cout << "successfully Loaded Triangle Points\n";
	cv::imshow("mesh", *skeletonOutput);
	//	waitKey();
}
void loadGlutDeformedMesh() {

	if (skeletonOutput != NULL /*&&skeletonInputForTexture != NULL*/){
		delete skeletonOutput;
		skeletonOutput = new Mat(PIBodyImage->size(), PIBodyImage->type());
		PIBodyImage->copyTo(*skeletonOutput);
	}

	skeletonInputForTexture = PIShirtImage;

	PILoadVerticesAndTrianglesFromDeformObj(true);



	hsColorTrianglesOnMeshBC(skeletonInputForTexture, skeletonOutput);
	//hsColorTrianglesOnMesh(skeletonInputForTexture, skeletonOutput);


	//draw the mesh:
	//for (int i = 0; i < deformedTriangles.size(); i++){
	//drawTriagnle(deformedTriangles[i], false);
	//}

	cv::imshow("mesh", *skeletonOutput);
}
void loadContours(){

	shirtContour.clear();
	bodyContour.clear();
	shirtContour.resize(1);
	bodyContour.resize(1);

	//ifstream GlutInfile("createdBoundary.node");
	ifstream GlutInfile("shirtCreatedBoundaryFull.node");
	if (GlutInfile.is_open()){
		string line;
		float word;
		vector< vector<string> >  tokens(2);

		//skipping the first 2 lines.
		getline(GlutInfile, line);
		getline(GlutInfile, line);
		getline(GlutInfile, line);
		istringstream iss(line);
		int size;
		iss >> size;

		//skipping another line
		getline(GlutInfile, line);

		for (int i = 0; i < size; i++)
		{
			getline(GlutInfile, line);

			istringstream iss(line);

			//skipping the first word of each line
			iss >> word;

			float x, y;
			iss >> x;
			iss >> y;
			Point2i p(x, y);
			shirtContour[0].push_back(p);
		}
	}
	else{
		std::cout << "Unable to open shirt Boundary file\n";
	}


	ifstream GlutInfile1("bodyCreatedBoundaryFull.node");
	if (GlutInfile1.is_open()){
		string line1;
		float word1;
		vector< vector<string> >  tokens1(2);

		//skipping the first 2 lines.
		getline(GlutInfile1, line1);
		getline(GlutInfile1, line1);
		getline(GlutInfile1, line1);
		istringstream iss1(line1);
		int size1;
		iss1 >> size1;

		//skipping another line
		getline(GlutInfile1, line1);

		for (int i = 0; i < size1; i++)
		{
			getline(GlutInfile1, line1);

			istringstream iss1(line1);

			//skipping the first word of each line
			iss1 >> word1;

			float x1, y1;
			iss1 >> x1;
			iss1 >> y1;
			Point2i p1(x1, y1);
			bodyContour[0].push_back(p1);
		}
	}
	else{
		std::cout << "Unable to open body Boundary file\n";
	}
}
void hsLoadTrianglePoints(Mat* output) {

	const Scalar S = Scalar(0, 255, 0);

	for (int j = 0; j < output->rows; j++){
		for (int i = 0; i < output->cols; i++) {

			for (int t = 0; t < triangles.size(); t++){

				//Her i are cols and j are rows. So the call to convToBary is correct.
				hsVertexBary temp = hsTriangle::convToBary(i, j, triangles[t]);
				if (hsTriangle::isInTriangle(temp)) {
					if (PIShirtImage == NULL)
						cerr << "hsLoadTrianglePoints failed PISHirt is NULL\n";
					Vec3b color = PIShirtImage->at<Vec3b>(cv::Point(i, j));
					if (!(color[0] == char(0) && color[1] == char(0) && color[2] == char(0)))
						triangles[t].points.push_back(cv::Point(i, j));
				}
			}
		}
	}
}


void sampleImage(){

	Mat img = imread("alphaMap.png");

	for (int i = 0; i < img.rows; i += 20){
		for (int j = 0; j < img.cols; j += 20){
			cv::Point temp(i, j);
			uchar color = img.at<uchar>(temp);
			if (color == uchar(1)){
				rmsmesh::Vertex point(temp.x, temp.y);
				m_deformer.hs_m_vInitialVertices.push_back(point);
			}
			std::cout << i << "  " << j << "\n";
		}
	}
	std::cout << "alphamap rows: " << img.rows << " cols: " << img.cols << "insertedVertices: " << m_deformer.hs_m_vInitialVertices.size() << "\n";
	m_deformer.buildDeformerMesh();
	m_deformer.buildTriangleMeshFromDeformer(&m_mesh);
	InitializeDeformedMesh();
}
void sampleContour(){

	//Load the boundary 
	for (int i = 0; i < contours0[0].size(); i++){
		rmsmesh::Vertex point(contours0[0][i].x, contours0[0][i].y);
		m_deformer.hs_m_vInitialVertices.push_back(point);
		int temp = m_deformer.hs_m_vInitialVertices.size() - 1;

		if (i < 184 || i > 211){
			if (i < 171 || i > 221)
				handsBoundaryMeshVertices.push_back(temp);
			else
				bodyBoundaryMeshVertices.push_back(temp);
		}
	}

	for (int i = 0; i < contours0[0].size(); i++){
		rmsmesh::Vertex point(contours0[0][i].x, contours0[0][i].y);
		m_deformer.hs_m_vInitialVertices.push_back(point);
		int temp = m_deformer.hs_m_vInitialVertices.size() - 1;


	}

	//Sample inside of the boundary
	for (int j = 0; j < gcapp.extractedAlphaMap->rows; j += 10) {
		for (int i = 0; i < gcapp.extractedAlphaMap->cols; i += 10)	{
			cv::Point2f temp(i, j);
			if (pointPolygonTest(contours0[0], temp, false) > 0) {
				rmsmesh::Vertex point(temp.x, temp.y);
				m_deformer.hs_m_vInitialVertices.push_back(point);
			}
		}
	}

	//	std::cout << "alphamap rows: " << img.rows << " cols: " << img.cols << "insertedVertices: " << m_deformer.hs_m_vInitialVertices.size() << "\n";
	m_deformer.buildDeformerMesh();
	m_deformer.buildTriangleMeshFromDeformer(&m_mesh);
	InitializeDeformedMesh();
}
void PILoadVerticesAndTrianglesFromDeformObj(bool deformed){

	deformedVertices.clear();
	deformedTriangles.clear();

	if (deformed){

		//HS this is where we retreive the vertices****************
		const std::vector<rmsmesh::Vertex>& verts = m_deformer.getDeformedVerts();
		const unsigned int npVertices = verts.size();

		for (int i = 0; i < npVertices; i++) {

			hsVertex temp(i, verts[i].vPosition[0], verts[i].vPosition[1]);
			deformedVertices[i] = temp;
		}

		// this is drawing triangles edges ******************HS
		const std::vector<rmsmesh::Triangle>& vTriangles = m_deformer.getTriangles();
		const unsigned int nbTriangles = vTriangles.size();

		for (int i = 0; i < nbTriangles; i++) {
			hsTriangle temp(i, deformedVertices[vTriangles[i].nVerts[0]], deformedVertices[vTriangles[i].nVerts[1]], deformedVertices[vTriangles[i].nVerts[2]]);
			deformedTriangles[i] = temp;
		}

		std::cout << " Loaded " << deformedVertices.size() << " deformedVertices and " <<
			deformedTriangles.size() << " deformedTriangles ...  \n";
	}
	else{

		//HS this is where we retreive the vertices****************
		const std::vector<rmsmesh::Vertex>& verts = m_deformer.getDeformedVerts();
		const unsigned int npVertices = verts.size();

		for (int i = 0; i < npVertices; i++) {

			hsVertex temp(i, verts[i].vPosition[0], verts[i].vPosition[1]);
			vertices[i] = temp;
		}

		// this is drawing triangles edges ******************HS
		const std::vector<rmsmesh::Triangle>& vTriangles = m_deformer.getTriangles();
		const unsigned int nbTriangles = vTriangles.size();

		for (int i = 0; i < nbTriangles; i++) {
			hsTriangle temp(i, vertices[vTriangles[i].nVerts[0]], vertices[vTriangles[i].nVerts[1]], vertices[vTriangles[i].nVerts[2]]);
			triangles[i] = temp;
		}
		std::cout << " Loaded " << vertices.size() << " vertices and " <<
			triangles.size() << " triangles ...  \n";
	}


}
void PILoadContourPointsFronFileIntoDeformObj() {

	//Reads the points from the boundary text file created by runExtractContours. And loads 
	//the points in the Glut_m_deform_obj

	//You can open sampleBoundary.node for a pre made file. Or createdBoundary.node to use the freshly calculated file
	ifstream GlutInfile("shirtCreatedBoundaryFull.node");
	if (GlutInfile.is_open()){
		string line;
		float word;
		vector< vector<string> >  tokens(2);

		//skipping the first 2 lines.
		getline(GlutInfile, line);
		getline(GlutInfile, line);
		getline(GlutInfile, line);
		istringstream iss(line);
		int size;
		iss >> size;

		//skipping another line
		getline(GlutInfile, line);

		for (int i = 0; i < size; i++)
		{
			getline(GlutInfile, line);

			istringstream iss(line);

			//skipping the first word of each line
			iss >> word;

			float x, y;
			iss >> x;
			iss >> y;

			rmsmesh::Vertex point(x, y);
			point.vPosition << x, y;
			m_deformer.hs_m_vInitialVertices.push_back(point);
		}
	}
	else{
		std::cout << "Unable to open file\n";
	}

	GlutInfile.close();

	std::cout << "Successfully loaded " << m_deformer.hs_m_vInitialVertices.size() << " initial vertices\n";



	m_deformer.buildDeformerMesh();
	m_deformer.buildTriangleMeshFromDeformer(&m_mesh);
	InitializeDeformedMesh();
}
void PISelectAllVerticesAsControlPoints(){

	for (unsigned int v = 0; v < 2; v++)
		m_vSelected.insert(v);

	InvalidateConstraints();
}
void PIShiftImage(){

	std::set<unsigned int>::iterator cur(m_vSelected.begin()), end(m_vSelected.end());
	unsigned int nSelected;

	while (cur != end)
	{
		nSelected = *cur++;
		float xpos = m_deformer.hsGetVertex(nSelected).vPosition.x();
		float ypos = m_deformer.hsGetVertex(nSelected).vPosition.y();
		Eigen::Vector3f newPos(xpos + 45, ypos + 55, 0.0f);
		m_deformedMesh.SetVertex(nSelected, newPos);
	}

	InvalidateConstraints();
}
void PISelectControlPoints(){

	hsSkeleton shirt;
	std::cout << "Loading shirt skeleton....";
	shirt.loadSkeleton(1);

	for (int i = 0; i < 8; i++){

		unsigned int nHit = m_deformer.hsfindClosestVertex(shirt.points[i].x, shirt.points[i].y);

		if (nHit < std::numeric_limits<unsigned int>::max()) {
			std::cout << "Selected vertex: (" << m_deformer.hsGetVertex(nHit).vPosition.x() << ", "
				<< m_deformer.hsGetVertex(nHit).vPosition.y() << ") " << endl;

			PISelectedControlPoints.push_back(pair<unsigned int, int>(nHit, i));
			m_vSelected.insert(nHit);
		}
		else{
			cerr << "could not find the closest vertex to (" << shirt.points[i].x << ", " << shirt.points[i].y << ") " << "index: " << i << "\n";
		}
	}

	for (int i = 9; i < 10; i++){

		unsigned int nHit = m_deformer.hsfindClosestVertex(shirt.points[i].x, shirt.points[i].y);

		if (nHit < std::numeric_limits<unsigned int>::max()) {
			std::cout << "Selected vertex: (" << m_deformer.hsGetVertex(nHit).vPosition.x() << ", "
				<< m_deformer.hsGetVertex(nHit).vPosition.y() << ") " << endl;

			PISelectedControlPoints.push_back(pair<unsigned int, int>(nHit, i));
			m_vSelected.insert(nHit);
		}
		else{
			cerr << "could not find the closest vertex to (" << shirt.points[i].x << ", " << shirt.points[i].y << ") " << "index: " << i << "\n";
		}
	}

	std::cout << "Selected: " << PISelectedControlPoints.size() << " control Points\n";

	InvalidateConstraints();
}
void PIMoveControlPoints(){

	hsSkeleton body;
	std::cout << "Loading body skeleton....";
	body.loadSkeleton(2);

	float offsetx = 0; //45;
	float offsety = 0;// 55;

	for (int i = 0; i < PISelectedControlPoints.size(); i++){

		unsigned int selectedControlPoint = PISelectedControlPoints[i].first;
		int destIndex = PISelectedControlPoints[i].second;

		Eigen::Vector3f newPos(body.points[destIndex].x + offsetx, body.points[destIndex].y + offsety, 0.0f);
		m_deformedMesh.SetVertex(selectedControlPoint, newPos);
	}

	InvalidateConstraints();
	std::cout << "returned from 2 press u to see the results\n";
}
void PIBuildMeshAndLoadTrianglePoints(){

	if (PIShirtImage == NULL){
		cout << "PIBuildMesh: PIShirtImage is NULL\n";
		return;
	}

	m_deformer.buildDeformerMesh();

	PILoadVerticesAndTrianglesFromDeformObj();
	//shiftImage
	hsLoadTrianglePoints(PIShirtImage);

	//	hsDrawMesh(PIShirtImage);
	//	cv::imshow("shirt+mesh", *PIShirtImage);
}
void PImakeSmaller(){

	hsSkeleton body;
	std::cout << "Loading body skeleton....";
	body.loadSkeleton(2);

	std::set<unsigned int>::iterator cur(m_vSelected.begin()), end(m_vSelected.end());
	unsigned int nSelected;

	nSelected = *cur;
	float xpos = m_deformer.hsGetVertex(nSelected).vPosition.x();
	float ypos = m_deformer.hsGetVertex(nSelected).vPosition.y();
	Eigen::Vector3f newPos(xpos + 10, ypos, 0.0f);
	m_deformedMesh.SetVertex(nSelected, newPos);

	InvalidateConstraints();
}

void findBoundaryVertices(){

	cout << "shirtContoursize in findBoundaryVertices: " << shirtContour[0].size() << "\n";
	for (int i = 0; i < shirtContour[0].size(); i++){

		//ignore loos boundary of shirt 
		if (i < 184 || i > 211){

			unsigned int nHit = m_deformer.hsfindClosestVertex(shirtContour[0][i].x, shirtContour[0][i].y);

			if (nHit < std::numeric_limits<unsigned int>::max()) {

				Eigen::Vector2f tmp = m_deformer.hsGetVertex(nHit).vPosition - Eigen::Vector2f(shirtContour[0][i].x, shirtContour[0][i].y);
				float dis = tmp.norm();
				//	if (dis < 0.000001){
				//			std::cout << "Selected vertex: (" << m_deformer.hsGetVertex(nHit).vPosition.x() << ", "
				//				<< m_deformer.hsGetVertex(nHit).vPosition.y() << ") " << endl;
				//			std::cout << dis << "\n";

				boundaryMeshVertices.push_back(nHit);
				//	}
			}
			else{
				cerr << "could not find the closest vertex to (" << shirtContour[0][i].x << ", " << shirtContour[0][i].y << ") " << "index: " << i << "\n";
				cerr << "*******returned********\n";
				break;
			}
		}
	}
}
void selectHandsBoundaryVertices(){

	if (handsBoundaryMeshVertices.size() == 0){
		std::cout << "handsBoundaryMeshVertices is empty did not select any control points\n";
		return;
	}

	for (int i = 0; i < handsBoundaryMeshVertices.size(); i++){
		m_vSelected.insert(handsBoundaryMeshVertices[i]);
	}

	InvalidateConstraints();
}
void selectBodyBoundaryVertices(){

	if (bodyBoundaryMeshVertices.size() == 0){
		std::cout << "bodyBoundaryMeshVertices is empty did not select any control points\n";
		return;
	}

	for (int i = 0; i < bodyBoundaryMeshVertices.size(); i++){
		m_vSelected.insert(bodyBoundaryMeshVertices[i]);
	}

	std::cout << "bodyBoundaryMeshVertices loop returned\n";
	InvalidateConstraints();

}
void selectMahdiSuggestedVertices(){

	if (mahdiMeshVertices.size() == 0){
		std::cout << "mahdiMeshVertices is empty did not select any control points\n";
		return;
	}

	for (int i = 0; i < mahdiMeshVertices.size(); i++){
		m_vSelected.insert(mahdiMeshVertices[i]);
	}

	InvalidateConstraints();
}
void findMahdiBoundaryVertices(){

	for (int i = 66; i < 307; i++){

		if (i == 66 || i == 133 || i == 139 || i == 169 || i == 186 || i == 211 || i == 237 || i == 248 || i == 222 || i == 306){

			unsigned int nHit = m_deformer.hsfindClosestVertex(shirtContour[0][i].x, shirtContour[0][i].y);

			if (nHit < std::numeric_limits<unsigned int>::max()) {

				Eigen::Vector2f tmp = m_deformer.hsGetVertex(nHit).vPosition - Eigen::Vector2f(shirtContour[0][i].x, shirtContour[0][i].y);
				float dis = tmp.norm();

				mahdiMeshVertices.push_back(nHit);
			}
			else{
				cerr << "could not find the closest vertex to (" << shirtContour[0][i].x << ", " << shirtContour[0][i].y << ") " << "index: " << i << "\n";
				cerr << "*******returned********\n";
				break;
			}
		}
	}

}
void selectAllBoundaryVertices(){

	if (boundaryMeshVertices.size() == 0){
		std::cout << "boundaryMeshVertices is empty did not select any control points\n";
		return;
	}

	for (int i = 0; i < boundaryMeshVertices.size(); i++){
		m_vSelected.insert(boundaryMeshVertices[i]);
	}

	InvalidateConstraints();
}

//TODO this should be replaced with a better algorithm.
//These two methods are what fucks up the results
//The paper asks the user to specifically select the body and hands boundaries and they feed just the specific part to their algo
//Instaed of just finding the closes point.
int hsfindClosestBodyBoundaryPoint(float x, float y, bool hands) {

	double minDistance = std::numeric_limits<double>::max();
	int closestVertex = -1;

	//TODO make the loops optimized later
	for (int i = 0; i < bodyContour[0].size(); i++)	{

		if (hands){
			if (i <= 235 || i >= 530){
				float fx = bodyContour[0][i].x;
				float fy = bodyContour[0][i].y;
				double disSquare = (x - fx) * (x - fx) + (y - fy) * (y - fy);

				if (disSquare < minDistance){
					minDistance = disSquare;
					closestVertex = i;
				}
			}
		}
		else{
			if (i > 235 && i < 530){
				float fx = bodyContour[0][i].x;
				float fy = bodyContour[0][i].y;
				double disSquare = (x - fx) * (x - fx) + (y - fy) * (y - fy);

				if (disSquare < minDistance){
					minDistance = disSquare;
					closestVertex = i;
				}
			}
		}
	}

	return (closestVertex == -1) ? std::numeric_limits<unsigned int>::max() : closestVertex;
}
int hsfindClosestBodyBoundaryPointAll(float x, float y, bool hands) {

	double minDistance = std::numeric_limits<double>::max();
	int closestVertex = -1;

	//TODO make the loops optimized later
	for (int i = 0; i < bodyContour[0].size(); i++)	{

		float fx = bodyContour[0][i].x;
		float fy = bodyContour[0][i].y;
		double disSquare = (x - fx) * (x - fx) + (y - fy) * (y - fy);

		if (disSquare < minDistance){
			minDistance = disSquare;
			closestVertex = i;
		}
	}

	return (closestVertex == -1) ? std::numeric_limits<unsigned int>::max() : closestVertex;
}
void moveSecondDeformControlPoints(int cont){

	std::set<unsigned int>::iterator cur(m_vSelected.begin()), end(m_vSelected.end());
	unsigned int nSelected;

	while (cur != end){

		nSelected = *cur++;
		float xpos = m_deformer.hsGetVertex(nSelected).vPosition.x();
		float ypos = m_deformer.hsGetVertex(nSelected).vPosition.y();
		int b;
		if (cont == 1)
			b = hsfindClosestBodyBoundaryPoint(xpos, ypos, true);
		else if (cont == 2)
			b = hsfindClosestBodyBoundaryPoint(xpos, ypos, false);
		else if (cont == 0)
			b = hsfindClosestBodyBoundaryPointAll(xpos, ypos, false);
		Eigen::Vector3f newPos(bodyContour[0][b].x, bodyContour[0][b].y, 0.0f);
		m_deformedMesh.SetVertex(nSelected, newPos);
	}

	InvalidateConstraints();
	std::cout << "moved control points for the second deformation\n";
}
void moveMahdiControlPoints(){

	std::set<unsigned int>::iterator cur(m_vSelected.begin()), end(m_vSelected.end());
	unsigned int nSelected;

	while (cur != end){
		nSelected = *cur++;
		if (nSelected == mahdiMeshVertices[66]){
			Eigen::Vector3f newPos(bodyContour[0][44].x, bodyContour[0][44].y, 0.0f);
			m_deformedMesh.SetVertex(nSelected, newPos);
		}
		else if (nSelected == mahdiMeshVertices[133]){
			Eigen::Vector3f newPos(bodyContour[0][125].x, bodyContour[0][125].y, 0.0f);
			m_deformedMesh.SetVertex(nSelected, newPos);
		}
		else if (nSelected == mahdiMeshVertices[139]){
			Eigen::Vector3f newPos(bodyContour[0][167].x, bodyContour[0][167].y, 0.0f);
			m_deformedMesh.SetVertex(nSelected, newPos);
		}
		else if (nSelected == mahdiMeshVertices[169]){
			Eigen::Vector3f newPos(bodyContour[0][232].x, bodyContour[0][232].y, 0.0f);
			m_deformedMesh.SetVertex(nSelected, newPos);
		}
		else if (nSelected == mahdiMeshVertices[186]){
			Eigen::Vector3f newPos(bodyContour[0][255].x, bodyContour[0][255].y, 0.0f);
			m_deformedMesh.SetVertex(nSelected, newPos);
		}
		else if (nSelected == mahdiMeshVertices[211]){
			Eigen::Vector3f newPos(bodyContour[0][495].x, bodyContour[0][495].y, 0.0f);
			m_deformedMesh.SetVertex(nSelected, newPos);
		}
		else if (nSelected == mahdiMeshVertices[222]){
			Eigen::Vector3f newPos(bodyContour[0][528].x, bodyContour[0][528].y, 0.0f);
			m_deformedMesh.SetVertex(nSelected, newPos);
		}
		else if (nSelected == mahdiMeshVertices[237]){
			Eigen::Vector3f newPos(bodyContour[0][569].x, bodyContour[0][569].y, 0.0f);
			m_deformedMesh.SetVertex(nSelected, newPos);
		}
		else if (nSelected == mahdiMeshVertices[248]){
			Eigen::Vector3f newPos(bodyContour[0][618].x, bodyContour[0][618].y, 0.0f);
			m_deformedMesh.SetVertex(nSelected, newPos);
		}
		else if (nSelected == mahdiMeshVertices[306]){
			Eigen::Vector3f newPos(bodyContour[0][700].x, bodyContour[0][700].y, 0.0f);
			m_deformedMesh.SetVertex(nSelected, newPos);
		}
	}

	InvalidateConstraints();
	std::cout << "moved control points for the second deformation\n";
}


/*
void performBothDeformations(){

loadContours();

for (int i = 0; i < shirtContour[0].size(); i++){

Point tem = shirtContour[0][i];
rmsmesh::Vertex point(tem.x, tem.y);
point.vPosition << tem.x, tem.y;
Glut_m_deform_obj.addPoint(point);
}

m_Selected.clear();
Glut_m_nSelected = std::numeric_limits<size_t>::max();
InvalidateConstraints();

std::cout << "Successfully loaded " << Glut_m_deform_obj.hsGetVertexSize() << " initial vertices\n";

Glut_m_deform_obj.buildMesh();



findBoundaryVertices();


loadGlutMesh();
PISelectControlPoints();
PIMoveControlPoints();
loadGlutDeformedMesh();
cv::waitKey();


PILoadVerticesAndTrianglesFromDeformObj(true);

//clear everything
Glut_m_nSelected = std::numeric_limits<unsigned int>::max();
m_Selected.clear();
Glut_m_deform_obj.clearData();
askForDestWhenSelectingControlPoints = false;;

//Load vertices again
for (int i = 0; i < deformedVertices.size(); i++){

float x = deformedVertices[i].position.x;
float y = deformedVertices[i].position.y;
rmsmesh::Vertex point(x, y);
point.vPosition << x, y;
Glut_m_deform_obj.addPoint(point);
}

m_Selected.clear();
Glut_m_nSelected = std::numeric_limits<unsigned int>::max();
Glut_m_deform_obj.updateConstraints(m_Selected);
Glut_m_deform_obj.buildMesh();


selectBoundaryVertices();
moveSecondDeformControlPoints();
loadGlutDeformedMesh();
cv::waitKey();
}*/
//TEST
/*

Mat kosIm = Mat::zeros(500, 500, CV_8UC3);

const Scalar sRED = Scalar(0, 0, 255);
const Scalar sYEL = Scalar(0, 255, 255);
const Scalar sGreen = Scalar(0, 255, 0);


for (int i = 0; i < bodyContour[0].size(); i++){

//	cout << "body i: " << i << " (" << bodyContour[0][i].x << ", " << bodyContour[0][i].y << ")" << "\n";
circle(kosIm, bodyContour[0][i], 3, sGreen, -1);
}
for (int i = 0; i < bodyContour[0].size() - 1; i++){

line(kosIm, bodyContour[0][i], bodyContour[0][i + 1], sGreen, 1);
}
line(kosIm, bodyContour[0][bodyContour[0].size() - 1], bodyContour[0][0], sGreen, 1);

const std::vector<Vertex>& verts = Glut_m_deform_obj.getDeformedVerts();

int cer = 0;
for (int i = 0; i < handsBoundaryMeshVertices.size(); i++){

int b = hsfindClosestBodyBoundaryPoint(verts[handsBoundaryMeshVertices[i]].vPosition[0], verts[handsBoundaryMeshVertices[i]].vPosition[1]);
circle(kosIm, bodyContour[0][b], 2, sRED, -1);
cer++;
imshow("kost", kosIm);
waitKey();
}

cout << "boundary vertice size: " << handsBoundaryMeshVertices.size() << " cer: " << cer << "\n";
imshow("kost", kosIm);
waitKey();
*/


int runExtractAlpha(){

	Mat image = imread(globalImageName, IMREAD_COLOR);
	contours_img = Mat::zeros(image.size(), CV_8UC1);

	if (image.empty())
	{
		std::cout << "\n Durn, couldn't read image filename " << endl;
		return 1;
	}

	GCApplication::help();

	const string winName = "image";
	namedWindow(winName, WINDOW_AUTOSIZE);
	setMouseCallback(winName, on_mouse, 0);

	gcapp.setImageAndWinName(image, winName);
	gcapp.showImage();

	for (;;)
	{
		int c = waitKey(0);
		switch ((char)c)
		{
		case '\x1b':
			std::cout << "Exiting ..." << endl;
			destroyAllWindows();
			return 0;
		case 's':
			std::cout << "Enter s for shirt Image or b for body Image: ";
			char temp;
			cin >> temp;
			if (temp == 's'){
				gcapp.saveOutputImage(PIShirtImage);
				std::cout << "successfully saved shirt Image\n";
				break;
			}
			else if (temp == 'b') {
				gcapp.saveOutputImage(PIBodyImage);
				std::cout << "successfully saved body Image\n";
			}
			break;
		case 'r':
			std::cout << endl;
			gcapp.reset();
			gcapp.showImage();
			break;
		case 'm':
			gcapp.showAlphaMap = true;
			gcapp.showImage();
			break;
		case 'k':
			gcapp.showImage();
			break;
		case 'i':
			saveExtractedImage();
			break;
		case 'n':
			gcapp.showAlphaMap = false;
			gcapp.showContours = false;
			int iterCount = gcapp.getIterCount();
			std::cout << "<" << iterCount << "... ";
			int newIterCount = gcapp.nextIter();
			if (newIterCount > iterCount)
			{
				gcapp.showImage();
				std::cout << iterCount << ">" << endl;
			}
			else
				std::cout << "rect must be determined>" << endl;
			break;
		}
	}
}
void runHsDeformeMesh() {

	hsGluthelp();

	setMouseCallback(hsDeformerWinName, hsDeformerOnMouse, 0);
	hsDeformerImage = new Mat(600, 600, CV_8UC3);
	const Scalar BLU = Scalar(255, 0, 0);
	circle(*hsDeformerImage, cv::Point(10, 10), 2, BLU, -1);
	namedWindow(hsDeformerWinName, WINDOW_AUTOSIZE);
	cv::imshow(hsDeformerWinName, *hsDeformerImage);


	allowFreeGlutToRun = true;
	askForDestWhenSelectingControlPoints = false;

	while (allowFreeGlutToRun){
		bool flag = false;

		int cOrigTemp = waitKey(0);
		switch ((char)cOrigTemp) {
		case '\x1b':
			destroyAllWindows();
			allowFreeGlutToRun = false;
			flag = true;
			break;
		case 'q':
			//PILoadContourPointsFronFileIntoDeformObj();
			//sampleImage();
			sampleContour();
			break;

		case 'l':
			std::cout << "Loading the orig mesh\n";
			loadGlutMesh();
			break;
		case 'L':
			std::cout << "Loading the deformed mesh\n";
			loadGlutDeformedMesh();
			break;
		case 'b':
			PISelectAllVerticesAsControlPoints();
			break;

		case 'B':
			std::cout << "Running shift\n";
			PIShiftImage();
			break;
		case 'p':
			std::cout << "Running PI1\n";
			PISelectControlPoints();
			//	askForDestWhenSelectingControlPoints = true;
			break;
		case 'P':
			std::cout << "Running PI2\n";
			PIMoveControlPoints();
			//dont run postRedisplay
			//	flag = true;
			break;
		case '1':
			loadContours();
			selectBodyBoundaryVertices();
			/*	loadContours();
			cout << "Mahdi: Loaded contours\n";
			findMahdiBoundaryVertices();
			cout << "Mahdi: found vertices\n";
			selectMahdiSuggestedVertices();
			cout << "Mahdi: selected control points\n";*/
			std::cout << "bodyBoundaryMeshVertices returned\n";
			break;
		case '2':
			//loadContours();
			//moveSecondDeformControlPoints(2);
			moveMahdiControlPoints();
			break;
		case '3':
			selectHandsBoundaryVertices();
			break;
		case '4':
			loadContours();
			moveSecondDeformControlPoints(1);
			break;
		case '5':
			loadContours();
			findBoundaryVertices();
		case '6':
			selectAllBoundaryVertices();
			break;
		case '7':
			loadContours();
			moveSecondDeformControlPoints(0);
			break;
		case 's':
			//	PILoadVerticesAndTrianglesFromDeformObj(true);
			m_vSelected.clear();
			m_deformer.copyDefToInVert();
			m_deformer.clearAll();
			//	saveMeshFile();
			break;
		case 'z':
			std::cout << "Running smaller\n";
			PImakeSmaller();
			break;
		case 'u':
			//make sure we fall to postRedisplay()
			//	flag = false;
			break;
		}
		if (!flag)
			hsDeformerShowImage();
	}
}
void runExtractContours() {

	std::cout << "\Extracting Contours\n"
		"\tLevels can range from - 3 to 3\n"
		"\nHot keys: \n"
		"\tPress any Key to Exit\n" << endl;

	if (gcapp.extractedAlphaMap == NULL){
		std::cout << "alphaMap is NULL\n";
		return;
	}

	findContours(*gcapp.extractedAlphaMap, contours0, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

	//To save full boundary
	//	createNodeFile(contours0);

	//hs this part is fitting a curve to the bounday points
	contours.resize(contours0.size());
	//hs Approximates a polygonal curve(s) with the specified precision. approxPolyDP()
	for (unsigned int k = 0; k < contours0.size(); k++)
		approxPolyDP(Mat(contours0[k]), contours[k], 3, true);

	contours_img = Mat::zeros(gcapp.extractedAlphaMap->size(), CV_8UC1);


	//Levels <= 0 all look the same for the shirt. But >0 obviously are smaller stuff
	int levels = 0;
	//*********************** TO successfully create the mesh the level should be 0 so that we get only the outer region.
	// so that out contours vector has a size of 1 (we need to have only 1 contour which is the boundary of our image)
	std::cout << "contours size is: " << contours.size() << "\n";
	std::cout << "first contours size is: " << contours[0].size() << "\n";


	//The following 2 lines is the check in drawContour that throws an error if _levels doesnt pass the assert
	//	unsigned int ncontours = contours.size();
	//	CV_Assert(0 <= _levels && _levels < (int)ncontours);

	//in elow, the 3rd param levels used to be _levels <= 0 ? 3 : -1
	cv::drawContours(contours_img, contours, levels, Scalar(128, 255, 255),
		3, 16, hierarchy, std::abs(levels));

	cv::imshow("contours", contours_img);
	cv::waitKey();

	std::cout << "***HS countours0[0].size: " << contours0[0].size() << "\n";

	//	if (createNodeFile(contours0) == 0)
	//		std::cout << "Successfully created Full node file\n";
	//	else
	//		std::cout << "failed to create node file\n";


	//	if (createNodeFile(contours) == 0)
	//		cout << "Successfully created node file\n";
	//	else
	//	cout << "failed to create node file\n";
}
int runSkeletonSelection(){

	skeletonObj.help();

	Mat skeletonInputImage = imread(globalImageName, IMREAD_COLOR);
	if (skeletonInputImage.empty())
	{
		std::cout << "\n Durn, couldn't read image filename " << endl;
		return -1;
	}

	const string winName = "image";
	namedWindow(winName, WINDOW_AUTOSIZE);
	setMouseCallback(winName, skeleton_on_mouse, 0);

	skeletonObj.setImageAndWinName(skeletonInputImage, winName);
	skeletonObj.showSkeletonImage();

	for (;;)
	{
		int c = waitKey(0);
		switch ((char)c)
		{
		case '\x1b':
			std::cout << "Exiting returns 0..." << endl;
			destroyAllWindows();
			return 0;
			break;
		case 'l':
			//call with 1 for shirt and 2 for body
			skeletonObj.loadSkeleton(1);
			break;
		case 's':
			skeletonObj.saveSkeleton();
			break;
		case 'r':
			skeletonObj.showSkeletonImage();
			break;
		}
	}

	return -1;
}
void runBcBsI(){
	int desiredSizeX = 500;
	int desiredSizeY = 500;


	//Point2i tl(200, 150);  
	//Point2i br(250, 200);
	Point2i a(100, 100);
	Point2i b(150, 150);
	Point2i c(100, 150);

	Point2i a1(100, 0);
	Point2i b1(200, 100);
	Point2i c1(100, 100);

	hsTriangle inTri(-1, hsVertex(0, a.x, a.y), hsVertex(1, b.x, b.y), hsVertex(2, c.x, c.y));
	hsTriangle outTri(-1, hsVertex(0, a1.x, a1.y), hsVertex(1, b1.x, b1.y), hsVertex(2, c1.x, c1.y));


	Mat* scaledImage = new Mat(desiredSizeY, desiredSizeX, CV_8UC3);
	bcbsi.bicubic_algo_singleSquare_onlyInsideTriangle(PIShirtImage, scaledImage, inTri, outTri);

	line(*PIShirtImage, a1, b1, Scalar(0, 0, 255), 1);
	line(*PIShirtImage, a1, c1, Scalar(0, 0, 255), 1);
	line(*PIShirtImage, b1, c1, Scalar(0, 0, 255), 1);

	line(*PIShirtImage, a, b, Scalar(0, 255, 0), 1);
	line(*PIShirtImage, a, c, Scalar(0, 255, 0), 1);
	line(*PIShirtImage, b, c, Scalar(0, 255, 0), 1);

	cv::imshow("input", *PIShirtImage);


	cv::waitKey();

	namedWindow("OUTPUT Image");
	cv::imshow("OUTPUT Image", *scaledImage);
	cv::waitKey(-1);
	cv::waitKey();
}



Mat shirtContIm = Mat::zeros(500, 500, CV_8UC3);
Mat bodyContIm = Mat::zeros(500, 500, CV_8UC3);
void drawLoadedContours(){

	if (shirtContour.size() != 0 && shirtContour[0].size() != 0){

		int levels = 0;

		std::cout << "shirt contours size is: " << shirtContour.size() << "\n";
		std::cout << "first shirt contours size is: " << shirtContour[0].size() << "\n";

		cv::drawContours(shirtContIm, shirtContour, levels, Scalar(128, 255, 255),
			3, 16);

		//	cv::imshow("shirtContour", shirtContIm);
		//	waitKey();
	}
	else
		std::cout << "shirtContour is empty\n";


	if (bodyContour.size() != 0 && bodyContour[0].size() != 0){

		int levels = 0;

		std::cout << "body contours size is: " << bodyContour.size() << "\n";
		std::cout << "body first contours size is: " << bodyContour[0].size() << "\n";

		cv::drawContours(bodyContIm, bodyContour, levels, Scalar(128, 255, 255),
			3, 16);

		//		cv::imshow("bodyContour", bodyContIm);
		//		waitKey();
	}
	else
		std::cout << "bodyContour is empty\n";
}
//backu
/*
void hsDrawContourPoints(){

Mat shirtContIm = Mat::zeros(500, 500, CV_8UC3);
Mat bodyContIm = Mat::zeros(500, 500, CV_8UC3);
const Scalar sRED = Scalar(0, 0, 255);
const Scalar sYEL = Scalar(0, 255, 255);
const Scalar sGreen = Scalar(0, 255, 0);

if (shirtContour.size() != 0 && shirtContour[0].size() != 0){

cout << "i: " << 0 << " (" << shirtContour[0][0].x << ", " << shirtContour[0][0].y << ")" << "\n";
circle(shirtContIm, shirtContour[0][0], 3, sGreen, -1);

cout << "i: " << 1 << " (" << shirtContour[0][1].x << ", " << shirtContour[0][1].y << ")" << "\n";
circle(shirtContIm, shirtContour[0][1], 3, sYEL, -1);

for (int i = 2; i < shirtContour[0].size(); i++){

cout << "i: " << i << " (" << shirtContour[0][i].x << ", " << shirtContour[0][i].y << ")"<< "\n";
circle(shirtContIm, shirtContour[0][i], 3, sRED, -1);

}
imshow("shirtContour", shirtContIm);
waitKey();

}
else
cout << "shirtContour is empty\n";



if (bodyContour.size() != 0 && bodyContour[0].size() != 0){

cout << "i: " << 0 << " (" << bodyContour[0][0].x << ", " << bodyContour[0][0].y << ")" << "\n";
circle(bodyContIm, bodyContour[0][0], 1, sGreen, -1);

cout << "i: " << 1 << " (" << bodyContour[0][1].x << ", " << bodyContour[0][1].y << ")" << "\n";
circle(bodyContIm, bodyContour[0][1], 1, sYEL, -1);

for (int i = 2; i < bodyContour[0].size(); i++){

cout << "i: " << i << " (" << bodyContour[0][i].x << ", " << bodyContour[0][i].y << ")" << "\n";
circle(bodyContIm, bodyContour[0][i], 3, sRED, -1);

imshow("bodyContour", bodyContIm);
waitKey();
}

}
else
cout << "bodyContour is empty\n";
}
*/
void hsDrawContourPoints(){

	Mat shirtContIm1 = Mat::zeros(500, 500, CV_8UC3);
	Mat bodyContIm1 = Mat::zeros(500, 500, CV_8UC3);

	const Scalar sRED = Scalar(0, 0, 255);
	const Scalar sYEL = Scalar(0, 255, 255);
	const Scalar sGreen = Scalar(0, 255, 0);

	Point offset((142 - 97), (92 - 37));

	std::cout << "shirt i: " << 0 << " (" << shirtContour[0][0].x << ", " << shirtContour[0][0].y << ")" << "\n";
	circle(shirtContIm, shirtContour[0][0], 3, sYEL, -1);
	circle(bodyContIm1, shirtContour[0][0] + offset, 1, sYEL, -1);

	std::cout << "body i: " << 0 << " (" << bodyContour[0][0].x << ", " << bodyContour[0][0].y << ")" << "\n";
	circle(bodyContIm, bodyContour[0][0], 3, sYEL, -1);
	circle(bodyContIm1, bodyContour[0][0], 1, sYEL, -1);

	for (int i = 1; i < shirtContour[0].size(); i++){

		std::cout << "body i: " << i << " (" << bodyContour[0][i].x << ", " << bodyContour[0][i].y << ")" << "\n";
		circle(bodyContIm, bodyContour[0][i], 3, sGreen, -1);
		circle(bodyContIm1, bodyContour[0][i], 1, sGreen, -1);

		std::cout << "shirt i: " << i << " (" << shirtContour[0][i].x << ", " << shirtContour[0][i].y << ")" << "\n";
		circle(shirtContIm, shirtContour[0][i], 3, sRED, -1);
		circle(bodyContIm1, shirtContour[0][i] + offset, 1, sRED, -1);

		cv::imshow("bodyContour1", bodyContIm1);
		cv::imshow("shirtContour", shirtContIm);
		cv::imshow("bodyContour", bodyContIm);
		cv::waitKey();

	}

}
void hsContourPlotter(){

	Mat shirtContIm1 = Mat::zeros(500, 500, CV_8UC3);

	const Scalar sRED = Scalar(0, 0, 255);
	const Scalar sYEL = Scalar(0, 255, 255);
	const Scalar sGreen = Scalar(0, 255, 0);


	for (int i = 0; i < shirtContour[0].size(); i++){

		if (i == 66 || i == 133 || i == 139 || i == 169 || i == 186 || i == 211 || i == 237 || i == 248 || i == 222 || i == 306){
			std::cout << "body i: " << i << " (" << shirtContour[0][i].x << ", " << shirtContour[0][i].y << ")" << "\n";
			circle(bodyContIm, shirtContour[0][i], 3, sGreen, -1);
		}
		else{
			std::cout << "body i: " << i << " (" << shirtContour[0][i].x << ", " << shirtContour[0][i].y << ")" << "\n";
			circle(bodyContIm, shirtContour[0][i], 3, sRED, -1);
		}

		cv::imshow("bodyContour", bodyContIm);
		cv::waitKey();

	}

}
void hsd(){

	Mat shirtContIm1 = Mat::zeros(500, 500, CV_8UC3);
	Mat bodyContIm1 = Mat::zeros(500, 500, CV_8UC3);

	const Scalar sRED = Scalar(0, 0, 255);
	const Scalar sYEL = Scalar(0, 255, 255);
	const Scalar sGreen = Scalar(0, 255, 0);

	Point offset((142 - 97), (92 - 37));

	//	cout << "shirt i: " << 0 << " (" << shirtContour[0][0].x << ", " << shirtContour[0][0].y << ")" << "\n";
	//	circle(shirtContIm, shirtContour[0][0], 1, sYEL, -1);

	std::cout << "body i: " << 0 << " (" << bodyContour[0][0].x << ", " << bodyContour[0][0].y << ")" << "\n";
	circle(bodyContIm, bodyContour[0][0], 1, sYEL, -1);

	for (int i = 1; i < bodyContour[0].size(); i++){

		std::cout << "body i: " << i << " (" << bodyContour[0][i].x << ", " << bodyContour[0][i].y << ")" << "\n";
		circle(bodyContIm, bodyContour[0][i], 1, sGreen, -1);

		//		cout << "shirt i: " << i << " (" << shirtContour[0][i].x << ", " << shirtContour[0][i].y << ")" << "\n";
		//		circle(shirtContIm, shirtContour[0][i], 1, sRED, -1);
	}


	for (int i = 0; i < bodyContour[0].size() - 1; i++){

		line(bodyContIm, bodyContour[0][i], bodyContour[0][i + 1], sGreen, 1);
		//		line(shirtContIm, shirtContour[0][i], shirtContour[0][i+1], sRED,1);
	}
	line(bodyContIm, bodyContour[0][bodyContour[0].size() - 1], bodyContour[0][0], sGreen, 1);
	//	line(shirtContIm, shirtContour[0][24], shirtContour[0][0], sRED, 1);

	//	imshow("bodyContour1", bodyContIm1);
	//	imshow("shirtContour", shirtContIm);
	cv::imshow("bodyContour", bodyContIm);
	cv::waitKey();

}



int main(int argc, char ** argv){

	std::cout << "***** Try: AmCfQlpPLs12\n";
	Mat temp1 = imread("bodyImage.png", IMREAD_COLOR);
	PIBodyImage = new Mat(temp1);

	Mat temp2 = imread("extractedImage.png");
	PIShirtImage = new Mat(temp2);

	globalImageName = "extractedImage.png";

	//resetGlobalImage();

	while (1){

		namedWindow("NULL", WINDOW_AUTOSIZE);

		std::cout << "\nMain Menu\n"
			"\nHot keys: \n"
			"\tESC - Exit\n"
			"\ta - runExtractAlpha();\n"
			"\tc - runExtractContours();\n"
			"\tp - runPlaceImage();\n"
			"\ts - runSkeletonSelection();\n"
			"\tf - runFreeGlut(argc, argv);\n"
			"\tb - resize image\n"
			"\tr - resetGlobalImage();\n" << endl;

		int c = waitKey(0);

		switch ((char)c) {
		case '\x1b':
			delete gcapp.extractedAlphaMap;
			destroyAllWindows();
			return 0;
			break;
		case 't':
			destroyAllWindows();
			//			performBothDeformations();
			destroyAllWindows();
			break;
		case 'a':
			destroyAllWindows();
			runExtractAlpha();
			destroyAllWindows();
			break;
		case 'b':
			destroyAllWindows();
			runBcBsI();
			destroyAllWindows();
			break;
		case 's':
			destroyAllWindows();
			runSkeletonSelection();
			destroyAllWindows();
			break;
		case 'f':
			destroyAllWindows();
			runHsDeformeMesh();
			std::cout << "exited freegulut\n";
			break;
		case 'c':
			destroyAllWindows();
			runExtractContours();
			cv::waitKey();
			break;
		case 'r':
			destroyAllWindows();
			resetGlobalImage();
			break;
		case 'p':
			destroyAllWindows();

			//			PILoadContourPointsFronFileIntoDeformObj();
			//			Glut_m_deform_obj.buildMesh();
			//			loadGlutMesh();
			cv::waitKey();
			//			PISelectControlPoints();
			//			PIMoveControlPoints();
			//			loadGlutDeformedMesh();
			//			waitKey();
			destroyAllWindows();

			break;
		}
	}
	//runBcBsI();
	//	loadContours();
	//	drawLoadedContours();
	//	hsContourPlotter();
	//	hsd();


}
