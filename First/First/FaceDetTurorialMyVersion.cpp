//Trasnparrency
void GCApplication::saveOutputImage(Mat*& output){

	if (image->empty()){
		cout << "GCApplication::image is empty. Output NOT saved\n";
		return;
	}

	//output = new Mat(image->size(), image->type());
	Scalar AScalar(char(-1), char(-1), char(-1));
	output = new Mat(image->size(), image->type(), AScalar);

	Mat binMask;

	if (!isInitialized)
		image->copyTo(*output);
	else
	{
		getBinMask(mask, binMask);
		image->copyTo(*output, binMask);
	}
}





//WORKING COPY: Successfull Mapping 2
#include "hsTriClasses.cpp"
#include "hsSkeleton.cpp"


//hs variables
String globalImageName;
Mat* PIShirtImage;
Mat* PIBodyImage;
Vector<size_t> PISelectedControlPoints;

//These are used by the skeletonSelector
Mat* skeletonOutput = NULL;
Mat* skeletonInputForTexture = NULL;


Mat contours_img;
vector<vector<cv::Point> > contours0;
vector<vector<cv::Point> > contours;
vector<Vec4i> hierarchy;

map<int, hsVertex> vertices;
map<int, hsTriangle> triangles;
map<int, hsVertex> deformedVertices;
map<int, hsTriangle> deformedTriangles;

GCApplication gcapp;
ArapDeform Glut_m_deform_obj;
hsSkeleton skeletonObj;


int GlutwinID;
int GlutWinWidth = 800;
int GlutWinHeight = 600;
bool allowFreeGlutToRun = true;

set<size_t> m_Selected;
size_t Glut_m_nSelected = std::numeric_limits<size_t>::max();

//hs methods
void loadGlutMesh();
void loadGlutDeformedMesh();
int createNodeFile(vector<vector<cv::Point> > contours);
Mat* k = NULL;
void hsDrawMesh(Mat*& output = k);
void hsLoadTrianglePoints(Mat* output);
void drawTriagnle(hsTriangle& triangle, bool flag = false, Mat*& output = skeletonOutput);
void hsColorTri3();
void hsColorTrianglesOnMesh(Mat* inputTextureFile, Mat* outputImage);
void GlutKeyboardCommon(unsigned char key, int x, int y);
void on_mouse(int event, int x, int y, int flags, void* param)
{
	gcapp.mouseClick(event, x, y, flags, param);
}
void skeleton_on_mouse(int event, int x, int y, int flags, void* param)
{
	skeletonObj.skeleton_mouseClick(event, x, y, flags, param);
}
void resetGlobalImage(){

	String defaultInput = "C:\\Users\\hooman\\Desktop\\dressup\\shirtOnWhite.png";
	String input;
	cout << "Enter full image name or d for default: ";
	cin >> input;
	globalImageName = (input != "d") ? input : defaultInput;
	cout << "GlobalImage Set to: " << globalImageName << "\n";
	return;
}
void saveExtractedImage(){
	Mat* temp;
	gcapp.saveOutputImage(temp);
	imwrite("extractedImage.png", *temp);
}
void PILoadContourPointsFronFileIntoDeformObj();
void PILoadVerticesAndTrianglesFromDeformObj(bool deformed = false);



void GlutKeyboardCommon(unsigned char key, int x, int y){

	bool flag = false;
	if (key == '\x1b')
	{
		glutDestroyWindow(GlutwinID);
		allowFreeGlutToRun = false;
		flag = true;
	}
	if ((key == 'A') || (key == 'a')) {
		Glut_m_deform_obj.setAddPoint();
		if (!Glut_m_deform_obj.isAddPoint())
		{
			Glut_m_deform_obj.buildMesh();
		}
	}

	if ((key == 'C') || (key == 'c'))
	{
		cout << "Clear every thing!" << endl;

		Glut_m_nSelected = std::numeric_limits<size_t>::max();
		m_Selected.clear();
		Glut_m_deform_obj.clearData();
	}

	if ((key == 'l'))
	{
		cout << "Loading the orig mesh\n";
		loadGlutMesh();
	}

	if ((key == 'L'))
	{
		cout << "Loading the deformed mesh\n";
		loadGlutDeformedMesh();
	}

	if ((key == 'Q') || (key == 'q'))  //HS MY stuff 
	{
		PILoadContourPointsFronFileIntoDeformObj();
	}

	if ((key == 'v') || (key == 'V'))
	{
		cout << "Clear constraints!" << endl;
		m_Selected.clear();
		Glut_m_nSelected = std::numeric_limits<size_t>::max();
	}

	if (!flag)
		glutPostRedisplay();
}
void GlutOnMouseMove(int x, int y)
{
	if (Glut_m_nSelected != std::numeric_limits<size_t>::max()) {
		GlutPoint newPos(x, y);
		Glut_m_deform_obj.setVertex(Glut_m_nSelected, newPos);
		Glut_m_deform_obj.updateMesh(false);
		glutPostRedisplay();
	}
}
void GlutMouse(int button, int state, int x, int y)
{
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			//glutIdleFunc(GlutSpinDisplay);
			cout << "Current mouse position: " << x << " " << y << endl;
			if (Glut_m_deform_obj.isAddPoint())
			{ //HS 
				Vertex point((float)x, (float)y);
				cout << "add point:" << x << " " << y << endl;
				point.vPosition << (float)x, (float)y;

				Glut_m_deform_obj.addPoint(point);

				m_Selected.clear();
				Glut_m_nSelected = std::numeric_limits<size_t>::max();
				Glut_m_deform_obj.updateConstraints(m_Selected);
			}
			else
			{
				Glut_m_nSelected = Glut_m_deform_obj.findHitVertex((float)x, (float)y);
				if (m_Selected.find(Glut_m_nSelected) == m_Selected.end())
					Glut_m_nSelected = std::numeric_limits<size_t>::max();
			}
		}
		else
		{
			Glut_m_nSelected = std::numeric_limits<size_t>::max();
		}

		break;
	case GLUT_MIDDLE_BUTTON:
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			cout << "Add control point at point: " << x << " " << y << endl;
			size_t nHit = Glut_m_deform_obj.findHitVertex((float)x, (float)y);
			if (nHit < std::numeric_limits<size_t>::max())
			{
				cout << "Find the vertex" << endl;
				if (m_Selected.find(nHit) == m_Selected.end())
					m_Selected.insert(nHit);
				else
				{
					m_Selected.erase(nHit);
				}
				// once modified the constraints, we should update them
				// i.e., precompute Gprime and B
				Glut_m_deform_obj.updateConstraints(m_Selected);
			}
			else
			{
				cout << "Doesn't find the vertex" << endl;
			}

		}

		break;
	default:
		break;
	}

	glutPostRedisplay();
}
void GlutSpinDisplay(void)
{
	glutPostRedisplay();
}
void GlutInit(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glShadeModel(GL_FLAT);
}
void GlutReshape(int w, int h)
{
	GlutWinWidth = w;
	GlutWinHeight = h;
}
void GlutDisplay(void){
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, (GLsizei)GlutWinWidth, (GLsizei)GlutWinHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, GlutWinWidth, GlutWinHeight, 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushMatrix();

	glPointSize(5.0f);

	//HS this is where we retreive the vertices****************
	const std::vector<Vertex>& verts = Glut_m_deform_obj.getDeformedVerts();
	const size_t np = verts.size();

	if (Glut_m_deform_obj.isAddPoint())
	{
		glColor3f(0.0, 1.0, 0.0);
		glBegin(GL_POINTS);

		for (int i = 0; i < np; i++)
		{
			//HS this is where we draw the vertices BEFORE THE MESH ********
			glVertex2f(verts[i].vPosition[0], verts[i].vPosition[1]);
		}
		glEnd();
	}
	else
	{
		glColor3f(0.0, 0.0, 1.0);
		glBegin(GL_POINTS);

		for (int i = 0; i < np; i++)
		{
			//HS this is where we draw the vertices IN THE MESH ********
			glVertex2f(verts[i].vPosition[0], verts[i].vPosition[1]);
		}
		glEnd();

		// this is drawing triangles edges ******************HS
		const std::vector<Triangle>& vTriangles = Glut_m_deform_obj.getTriangles();
		const size_t nt = vTriangles.size();

		glBegin(GL_LINES);
		for (int i = 0; i < nt; i++)
		{
			const size_t * tri = vTriangles[i].nVertices;

			// This is freeGlut drawing lines. Disable it no edges drawn just the vertices
			glVertex2f(verts[tri[0]].vPosition[0], verts[tri[0]].vPosition[1]);
			glVertex2f(verts[tri[1]].vPosition[0], verts[tri[1]].vPosition[1]);

			glVertex2f(verts[tri[1]].vPosition[0], verts[tri[1]].vPosition[1]);
			glVertex2f(verts[tri[2]].vPosition[0], verts[tri[2]].vPosition[1]);

			glVertex2f(verts[tri[2]].vPosition[0], verts[tri[2]].vPosition[1]);
			glVertex2f(verts[tri[0]].vPosition[0], verts[tri[0]].vPosition[1]);
		}
		glEnd();
	}

	// draw controllers
	glColor3f(1.0, 0.0, 0.0);
	std::set<size_t>::iterator cur(m_Selected.begin()), end(m_Selected.end());
	glBegin(GL_POINTS);

	while (cur != end) {
		size_t nSelected = *cur++;
		glVertex2f(verts[nSelected].vPosition[0], verts[nSelected].vPosition[1]);
	}
	glEnd();

	//glFlush();
	glPopMatrix();

	glutSwapBuffers();
}


int runExtractAlpha(){

	Mat image = imread(globalImageName, IMREAD_COLOR);
	contours_img = Mat::zeros(image.size(), CV_8UC1);

	if (image.empty())
	{
		cout << "\n Durn, couldn't read image filename " << endl;
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
			cout << "Exiting ..." << endl;
			destroyAllWindows();
			return 0;
		case 's':
			cout << "Enter s for shirt Image or b for body Image: ";
			char temp;
			cin >> temp;
			if (temp == 's'){
				gcapp.saveOutputImage(PIShirtImage);
				cout << "successfully saved shirt Image\n";
				break;
			}
			else if (temp == 'b') {
				gcapp.saveOutputImage(PIBodyImage);
				cout << "successfully saved body Image\n";
			}
			break;
		case 'r':
			cout << endl;
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
			cout << "<" << iterCount << "... ";
			int newIterCount = gcapp.nextIter();
			if (newIterCount > iterCount)
			{
				gcapp.showImage();
				cout << iterCount << ">" << endl;
			}
			else
				cout << "rect must be determined>" << endl;
			break;
		}
	}
}
void runExtractContours() {

	cout << "\Extracting Contours\n"
		"\tLevels can range from - 3 to 3\n"
		"\nHot keys: \n"
		"\tPress any Key to Exit\n" << endl;

	if (gcapp.extractedAlphaMap == NULL){
		cout << "alphaMap is NULL\n";
		return;
	}

	findContours(*gcapp.extractedAlphaMap, contours0, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

	contours.resize(contours0.size());
	for (size_t k = 0; k < contours0.size(); k++)
		approxPolyDP(Mat(contours0[k]), contours[k], 3, true);

	contours_img = Mat::zeros(gcapp.extractedAlphaMap->size(), CV_8UC1);


	//Levels <= 0 all look the same for the shirt. But >0 obviously are smaller stuff
	int levels = 0;
	//*********************** TO successfully create the mesh the level should be 0 so that we get only the outer region.
	// so that out contours vector has a size of 1 (we need to have only 1 contour which is the boundary of our image)
	cout << "contours size is: " << contours.size() << "\n";
	cout << "first contours size is: " << contours[0].size() << "\n";


	//The following 2 lines is the check in drawContour that throws an error if _levels doesnt pass the assert
	//	size_t ncontours = contours.size();
	//	CV_Assert(0 <= _levels && _levels < (int)ncontours);

	//in elow, the 3rd param levels used to be _levels <= 0 ? 3 : -1
	drawContours(contours_img, contours, levels, Scalar(128, 255, 255),
		3, 16, hierarchy, std::abs(levels));

	cv::imshow("contours", contours_img);

	if (createNodeFile(contours) == 0)
		cout << "Successfully created node file\n";
	else
		cout << "failed to create node file\n";
}
void runFreeGlut(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(GlutWinWidth, GlutWinHeight);
	glutInitWindowPosition(100, 100);
	GlutwinID = glutCreateWindow("Test OpenGL");
	GlutInit();
	glutDisplayFunc(GlutDisplay);
	glutReshapeFunc(GlutReshape);
	glutKeyboardFunc(GlutKeyboardCommon);
	glutMotionFunc(GlutOnMouseMove);
	glutMouseFunc(GlutMouse);

	//hs instead of glutMainLoop();
	allowFreeGlutToRun = true;
	while (allowFreeGlutToRun){
		glutMainLoopEvent();
	}
}
int runSkeletonSelection(){

	skeletonObj.help();

	Mat skeletonInputImage = imread(globalImageName, IMREAD_COLOR);
	if (skeletonInputImage.empty())
	{
		cout << "\n Durn, couldn't read image filename " << endl;
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
			cout << "Exiting returns 0..." << endl;
			destroyAllWindows();
			return 0;
			break;
		case 'l':
			skeletonObj.loadSkeleton();
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


void PILoadVerticesAndTrianglesFromDeformObj(bool deformed){

	if (deformed){

		//HS this is where we retreive the vertices****************
		const std::vector<Vertex>& verts = Glut_m_deform_obj.getDeformedVerts();
		const size_t npVertices = verts.size();

		for (int i = 0; i < npVertices; i++) {

			hsVertex temp(i, verts[i].vPosition[0], verts[i].vPosition[1]);
			deformedVertices[i] = temp;
		}

		// this is drawing triangles edges ******************HS
		const std::vector<Triangle>& vTriangles = Glut_m_deform_obj.getTriangles();
		const size_t nbTriangles = vTriangles.size();

		for (int i = 0; i < nbTriangles; i++) {
			hsTriangle temp(i, deformedVertices[vTriangles[i].nVertices[0]], deformedVertices[vTriangles[i].nVertices[1]], deformedVertices[vTriangles[i].nVertices[2]]);
			deformedTriangles[i] = temp;
		}
	}
	else{

		//HS this is where we retreive the vertices****************
		const std::vector<Vertex>& verts = Glut_m_deform_obj.getDeformedVerts();
		const size_t npVertices = verts.size();

		for (int i = 0; i < npVertices; i++) {

			hsVertex temp(i, verts[i].vPosition[0], verts[i].vPosition[1]);
			vertices[i] = temp;
		}

		// this is drawing triangles edges ******************HS
		const std::vector<Triangle>& vTriangles = Glut_m_deform_obj.getTriangles();
		const size_t nbTriangles = vTriangles.size();

		for (int i = 0; i < nbTriangles; i++) {
			hsTriangle temp(i, vertices[vTriangles[i].nVertices[0]], vertices[vTriangles[i].nVertices[1]], vertices[vTriangles[i].nVertices[2]]);
			triangles[i] = temp;
		}
	}

	cout << " Loaded " << deformedVertices.size() << " deformedVertices and " <<
		deformedTriangles.size() << "deformedTriangles ... Deformed: " << deformed << "\n";
}
void PILoadContourPointsFronFileIntoDeformObj() {

	//Reads the points from the boundary text file created by runExtractContours. And loads 
	//the points in the Glut_m_deform_obj

	//You can open sampleBoundary.node for a pre made file. Or createdBoundary.node to use the freshly calculated file
	ifstream GlutInfile("createdBoundary.node");
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
			Vertex point(x, y);
			point.vPosition << x, y;
			Glut_m_deform_obj.addPoint(point);
		}
	}
	else{
		cout << "Unable to open file\n";
	}

	GlutInfile.close();
	m_Selected.clear();
	Glut_m_nSelected = std::numeric_limits<size_t>::max();
	Glut_m_deform_obj.updateConstraints(m_Selected);

	cout << "Successfully loaded " << Glut_m_deform_obj.hsGetVertexSize() << " initial vertices\n";
}
void PIBuildMeshAndLoadTrianglePoints(){

	if (PIShirtImage == NULL){
		cout << "PIBuildMesh: PIShirtImage is NULL\n";
		return;
	}

	Glut_m_deform_obj.buildMesh();// press A insread
	PILoadVerticesAndTrianglesFromDeformObj();
	//shiftImage
	hsLoadTrianglePoints(PIShirtImage);

	//	hsDrawMesh(PIShirtImage);
	//	cv::imshow("shirt+mesh", *PIShirtImage);
}
void PISelectControlPoints(){

	hsSkeleton shirt;
	cout << "Loading shirt skeleton....";
	shirt.loadSkeleton();

	for (int i = 0; i < /*shirt.points.size()*/ 11; i++){

		size_t nHit = Glut_m_deform_obj.hsfindClosestVertex(shirt.points[i].x, shirt.points[i].y);

		if (nHit < std::numeric_limits<size_t>::max()) {
			cout << "Selected vertex: (" << Glut_m_deform_obj.hsGetVertex(nHit).vPosition.x() << ", "
				<< Glut_m_deform_obj.hsGetVertex(nHit).vPosition.y() << ") " << endl;

			PISelectedControlPoints.push_back(nHit);
		}
		else{
			cerr << "could not find the closest vertex to (" << shirt.points[i].x << ", " << shirt.points[i].y << ") " << "index: " << i << "\n";
		}
	}

	cout << "Selected: " << PISelectedControlPoints.size() << " control Points\n";
	if (PISelectedControlPoints.size() < 10){
		Glut_m_deform_obj.hsPrintInitialVertices();
	}


	//TODO remove this extra work later
	for (int i = 0; i < PISelectedControlPoints.size(); i++)
		m_Selected.insert(PISelectedControlPoints[i]);

	Glut_m_deform_obj.updateConstraints(m_Selected);
}
void PIDeformMesh(){

	hsSkeleton body;
	cout << "Loading body skeleton....";
	body.loadSkeleton();

	for (int i = 0; i < /*body.points.size()*/ PISelectedControlPoints.size(); i++){

		size_t selectedControlPoint = PISelectedControlPoints[i];

		GlutPoint newPos(body.points[i].x, body.points[i].y);
		Glut_m_deform_obj.setVertex(selectedControlPoint, newPos);
	}

	//It was originally false. Didnt make a difference for shirtImage.png
	Glut_m_deform_obj.updateMesh(true);
	cout << "Succsessfully deformed mesh\n";
}
void PIPlaceImage(){

	if (PIBodyImage == NULL || PIShirtImage == NULL){
		cout << "PIPlaceImage: PIShirtImage or PIBodyImage is NULL\n";
		return;
	}

	PILoadVerticesAndTrianglesFromDeformObj(true);

	//	hsDrawMesh(PIShirtImage);
	//	cv::imshow("shirt+DeformedMesh", *PIShirtImage);

	hsColorTrianglesOnMesh(PIShirtImage, PIBodyImage);
	cv::imshow("final", *PIBodyImage);
	waitKey();
}
void runPlaceImage() {

	PILoadContourPointsFronFileIntoDeformObj(); // press Q insread 
	PIBuildMeshAndLoadTrianglePoints();
	PISelectControlPoints();
	PIDeformMesh();
	PIPlaceImage();
}




int createNodeFile(vector<vector<cv::Point> > contours) {

	if (contours.size() == 0){
		cout << "No contours created\n";
		return -1;
	}

	ofstream myfile;
	myfile.open("createdBoundary.node");
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
void saveMeshFile(){

	ofstream myfile;
	myfile.open("hsDeformedNodes.node");
	myfile
		<< "# hsDeformedNodes.node\n"
		<< "#\n"
		<< vertices.size() << "	" << 2 << "	" << 0 << "	" << 0 << "\n"
		<< "# and here are the points:\n";

	for (int i = 0; i < vertices.size(); i++){
		myfile << vertices[i].vertexId << "	" << vertices[i].position.first << "	" << vertices[i].position.second << "	" << 0 << "\n";
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

	myfile1.close();
}


bool load = true;
void hsLoadTrianglePoints(Mat* output) {

	const Scalar S = Scalar(0, 255, 0);

	for (int j = 0; j < output->rows; j++){
		for (int i = 0; i < output->cols; i++) {

			for (int t = 0; t < triangles.size(); t++){

				hsVertexBary temp = hsTriangle::convToBary(i, j, triangles[t]);
				if (hsTriangle::isInTriangle(temp)) {
					//	circle(*skeletonOutput, Point(i, j), 1, S, -1);
					//	Vec3b color = skeletonInputForTexture->at<Vec3b>(cv::Point(i, j));
					//	skeletonOutput->at<Vec3b>(cv::Point(i, j)) = color;
					if (load){
						triangles[t].points.push_back(cv::Point(i, j));
					}
				}
			}
		}
	}
	load = false;
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
	hsColorTri3();
	cv::imshow("mesh", *skeletonOutput);
	cv::imshow("orig", *skeletonInputForTexture);
}
void loadGlutMesh() {

	Mat temp = imread(globalImageName, IMREAD_COLOR);
	skeletonInputForTexture = new Mat(temp);
	skeletonOutput = new Mat(temp.size(), temp.type());

	PILoadVerticesAndTrianglesFromDeformObj();

	//	hsColorTri3();

	hsLoadTrianglePoints(skeletonOutput);
	//	hsColorTrianglesOnMesh();
	hsDrawMesh();
	//	saveMeshFile();
	cv::imshow("mesh", *skeletonOutput);
	//	imshow("orig", *skeletonInputForTexture); 
}
void loadGlutDeformedMesh() {

	if (skeletonOutput != NULL /*&&skeletonInputForTexture != NULL*/){
		delete skeletonOutput;
		skeletonOutput = new Mat(PIBodyImage->size(), PIBodyImage->type());
		PIBodyImage->copyTo(*skeletonOutput);
	}

	skeletonInputForTexture = PIShirtImage;

	PILoadVerticesAndTrianglesFromDeformObj(true);

	hsColorTrianglesOnMesh(skeletonInputForTexture, skeletonOutput);

	//draw the mesh:

	for (int i = 0; i < deformedTriangles.size(); i++){
		drawTriagnle(deformedTriangles[i], false);
	}

	cv::imshow("mesh", *skeletonOutput);
}


void drawTriagnle(hsTriangle& triangle, bool flag, Mat*& output){

	const Scalar sRED = Scalar(0, 0, 255);
	const Scalar sGreen = Scalar(0, 255, 0);

	hspair a = triangle.a.position;
	hspair b = triangle.b.position;
	hspair c = triangle.c.position;



	if (flag){
		circle(*output, cv::Point(a.first, a.second), 1, sGreen, -1);
		circle(*output, cv::Point(b.first, b.second), 1, sGreen, -1);
		circle(*output, cv::Point(c.first, c.second), 1, sGreen, -1);

		line(*output, cv::Point(a.first, a.second), cv::Point(b.first, b.second), sGreen, 1);
		line(*output, cv::Point(a.first, a.second), cv::Point(c.first, c.second), sGreen, 1);
		line(*output, cv::Point(b.first, b.second), cv::Point(c.first, c.second), sGreen, 1);
	}
	else{
		unsigned char *input = (unsigned char*)(skeletonInputForTexture->data);
		int i, j;
		double blue, green, red;

		i = a.first;
		j = a.second;
		blue = (double)input[skeletonInputForTexture->step * j + i];
		green = (double)input[skeletonInputForTexture->step * j + i + 1];
		red = (double)input[skeletonInputForTexture->step * j + i + 2];
		Scalar sA(blue, green, red);

		i = b.first;
		j = b.second;
		blue = (double)input[skeletonInputForTexture->step * j + i];
		green = (double)input[skeletonInputForTexture->step * j + i + 1];
		red = (double)input[skeletonInputForTexture->step * j + i + 2];
		Scalar sB(blue, green, red);

		i = c.first;
		j = c.second;
		blue = (double)input[skeletonInputForTexture->step * j + i];
		green = (double)input[skeletonInputForTexture->step * j + i + 1];
		red = (double)input[skeletonInputForTexture->step * j + i + 2];
		Scalar sC(blue, green, red);
		circle(*output, cv::Point(a.first, a.second), 1, sA, -1);
		circle(*output, cv::Point(b.first, b.second), 1, sB, -1);
		circle(*output, cv::Point(c.first, c.second), 1, sC, -1);

		line(*output, cv::Point(a.first, a.second), cv::Point(b.first, b.second), sRED, 1);
		line(*output, cv::Point(a.first, a.second), cv::Point(c.first, c.second), sRED, 1);
		line(*output, cv::Point(b.first, b.second), cv::Point(c.first, c.second), sRED, 1);
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
			outputImage->at<Vec3b>(temp1) = color;
		}
	}
}


int main(int argc, char** argv) {

	//Just for convinece delete anyime 
	Mat temp = imread("bodyImage.png", IMREAD_COLOR);
	PIBodyImage = new Mat(temp);

	resetGlobalImage();

	while (1){

		namedWindow("NULL", WINDOW_AUTOSIZE);

		cout << "\nMain Menu\n"
			"\nHot keys: \n"
			"\tESC - Exit\n"
			"\ta - runExtractAlpha();\n"
			"\tc - runExtractContours();\n"
			"\tp - runPlaceImage();\n"
			"\ts - runSkeletonSelection();\n"
			"\tf - runFreeGlut(argc, argv);\n"
			"\tr - resetGlobalImage();\n" << endl;

		int c = waitKey(0);

		switch ((char)c) {
		case '\x1b':
			delete gcapp.extractedAlphaMap;
			destroyAllWindows();
			return 0;
			break;
		case 'a':
			destroyAllWindows();
			runExtractAlpha();
			destroyAllWindows();
			break;
		case 's':
			destroyAllWindows();
			runSkeletonSelection();
			destroyAllWindows();
			break;
		case 'f':
			destroyAllWindows();
			runFreeGlut(argc, argv);
			cout << "exited freegulut\n";
			break;
		case 'c':
			destroyAllWindows();
			runExtractContours();
			waitKey();
			break;
		case 'r':
			destroyAllWindows();
			resetGlobalImage();
			break;
		case 'p':
			destroyAllWindows();
			runPlaceImage();
			destroyAllWindows();
			break;
		}
	}
}



















//WORKING COPY: Successfull Mapping 1
#include "first.h"
#include "GCApplication.h"


//HS VARIABLES:
class hsVertex;
class hsTriangle;
class hsVertexBary;

GCApplication gcapp;
ArapDeform Glut_m_deform_obj;

Mat* textureP = NULL;
Mat* hsImage = NULL;
Mat contours_img;
vector<vector<cv::Point> > contours0;

set<size_t> m_Selected;
size_t Glut_m_nSelected = std::numeric_limits<size_t>::max();

map<int, hsVertex> vertices;
map<int, hsTriangle> triangles;
map<int, hsVertex> deformedVertices;
map<int, hsTriangle> deformedTriangles;

const int w = 500;
vector<vector<cv::Point> > contours;
vector<Vec4i> hierarchy;

bool isInTriangle(hsVertexBary& point);
int extractAlphaExit();
void loadGlutMesh();
void loadGlutDeformedMesh();
hsVertexBary convToBary(float x, float y, hsTriangle& tri);

int GlutwinID;
int GlutWinWidth = 800;
int GlutWinHeight = 600;



void GlutDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, (GLsizei)GlutWinWidth, (GLsizei)GlutWinHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, GlutWinWidth, GlutWinHeight, 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushMatrix();

	glPointSize(5.0f);

	//HS this is where we retreive the vertices****************
	const std::vector<Vertex>& verts = Glut_m_deform_obj.getDeformedVerts();
	const size_t np = verts.size();

	if (Glut_m_deform_obj.isAddPoint())
	{
		glColor3f(0.0, 1.0, 0.0);
		glBegin(GL_POINTS);

		for (int i = 0; i < np; i++)
		{
			//HS this is where we draw the vertices BEFORE THE MESH ********
			glVertex2f(verts[i].vPosition[0], verts[i].vPosition[1]);
		}
		glEnd();
	}
	else
	{
		glColor3f(0.0, 0.0, 1.0);
		glBegin(GL_POINTS);

		for (int i = 0; i < np; i++)
		{
			//HS this is where we draw the vertices IN THE MESH ********
			glVertex2f(verts[i].vPosition[0], verts[i].vPosition[1]);
		}
		glEnd();

		// this is drawing triangles edges ******************HS
		const std::vector<Triangle>& vTriangles = Glut_m_deform_obj.getTriangles();
		const size_t nt = vTriangles.size();

		glBegin(GL_LINES);
		for (int i = 0; i < nt; i++)
		{
			const size_t * tri = vTriangles[i].nVertices;

			// This is freeGlut drawing lines. Disable it no edges drawn just the vertices
			glVertex2f(verts[tri[0]].vPosition[0], verts[tri[0]].vPosition[1]);
			glVertex2f(verts[tri[1]].vPosition[0], verts[tri[1]].vPosition[1]);

			glVertex2f(verts[tri[1]].vPosition[0], verts[tri[1]].vPosition[1]);
			glVertex2f(verts[tri[2]].vPosition[0], verts[tri[2]].vPosition[1]);

			glVertex2f(verts[tri[2]].vPosition[0], verts[tri[2]].vPosition[1]);
			glVertex2f(verts[tri[0]].vPosition[0], verts[tri[0]].vPosition[1]);
		}
		glEnd();
	}

	// draw controllers
	glColor3f(1.0, 0.0, 0.0);
	std::set<size_t>::iterator cur(m_Selected.begin()), end(m_Selected.end());
	glBegin(GL_POINTS);

	while (cur != end) {
		size_t nSelected = *cur++;
		glVertex2f(verts[nSelected].vPosition[0], verts[nSelected].vPosition[1]);
	}
	glEnd();

	//glFlush();
	glPopMatrix();

	glutSwapBuffers();
}

void GlutKeyboardCommon(unsigned char key, int x, int y){

	if (key == '\x1b')
	{
		glutDestroyWindow(GlutwinID);
	}
	if ((key == 'A') || (key == 'a')) {
		Glut_m_deform_obj.setAddPoint();
		if (!Glut_m_deform_obj.isAddPoint())
		{
			Glut_m_deform_obj.buildMesh();
		}
	}

	if ((key == 'C') || (key == 'c'))
	{
		cout << "Clear every thing!" << endl;

		Glut_m_nSelected = std::numeric_limits<size_t>::max();
		m_Selected.clear();
		Glut_m_deform_obj.clearData();
	}

	if ((key == 'l'))
	{
		cout << "Loading the orig mesh\n";
		loadGlutMesh();
	}

	if ((key == 'L'))
	{
		cout << "Loading the deformed mesh\n";
		loadGlutDeformedMesh();
	}

	if ((key == 'Q') || (key == 'q'))  //HS MY stuff 
	{
		ifstream GlutInfile("sampleBoundary.node");
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
				Vertex point(x, y);
				point.vPosition << x, y;
				Glut_m_deform_obj.addPoint(point);
			}
		}
		else{
			cout << "Unable to open file\n";
		}

		GlutInfile.close();
		m_Selected.clear();
		Glut_m_nSelected = std::numeric_limits<size_t>::max();
		Glut_m_deform_obj.updateConstraints(m_Selected);
	}

	if ((key == 'v') || (key == 'V'))
	{
		cout << "Clear constraints!" << endl;
		m_Selected.clear();
		Glut_m_nSelected = std::numeric_limits<size_t>::max();
	}

	glutPostRedisplay();
}

void GlutSpinDisplay(void)
{
	glutPostRedisplay();
}

void GlutInit(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glShadeModel(GL_FLAT);
}

void GlutReshape(int w, int h)
{
	GlutWinWidth = w;
	GlutWinHeight = h;
}

void GlutMouse(int button, int state, int x, int y)
{
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			//glutIdleFunc(GlutSpinDisplay);
			cout << "Current mouse position: " << x << " " << y << endl;
			if (Glut_m_deform_obj.isAddPoint())
			{ //HS 
				Vertex point((float)x, (float)y);
				cout << "add point:" << x << " " << y << endl;
				point.vPosition << (float)x, (float)y;

				Glut_m_deform_obj.addPoint(point);

				m_Selected.clear();
				Glut_m_nSelected = std::numeric_limits<size_t>::max();
				Glut_m_deform_obj.updateConstraints(m_Selected);
			}
			else
			{
				Glut_m_nSelected = Glut_m_deform_obj.findHitVertex((float)x, (float)y);
				if (m_Selected.find(Glut_m_nSelected) == m_Selected.end())
					Glut_m_nSelected = std::numeric_limits<size_t>::max();
			}
		}
		else
		{
			Glut_m_nSelected = std::numeric_limits<size_t>::max();
		}

		break;
	case GLUT_MIDDLE_BUTTON:
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			cout << "Add control point at point: " << x << " " << y << endl;
			size_t nHit = Glut_m_deform_obj.findHitVertex((float)x, (float)y);
			if (nHit < std::numeric_limits<size_t>::max())
			{
				cout << "Find the vertex" << endl;
				if (m_Selected.find(nHit) == m_Selected.end())
					m_Selected.insert(nHit);
				else
				{
					m_Selected.erase(nHit);
				}
				// once modified the constraints, we should update them
				// i.e., precompute Gprime and B
				Glut_m_deform_obj.updateConstraints(m_Selected);
			}
			else
			{
				cout << "Doesn't find the vertex" << endl;
			}

		}

		break;
	default:
		break;
	}

	glutPostRedisplay();
}


void GlutOnMouseMove(int x, int y)
{
	if (Glut_m_nSelected != std::numeric_limits<size_t>::max()) {
		GlutPoint newPos(x, y);
		Glut_m_deform_obj.setVertex(Glut_m_nSelected, newPos);
		Glut_m_deform_obj.updateMesh(false);
		glutPostRedisplay();
	}
}


static void help()
{
	cout << "\nKosse nane raja -- select an object in a region\n"
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
	if (_image.empty() || _winName.empty())
		return;
	image = &_image;
	winName = &_winName;
	mask.create(image->size(), CV_8UC1);
	reset();
}

void GCApplication::showImage()
{

	if (image->empty() || winName->empty())
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


	vector<cv::Point>::const_iterator it;
	for (it = bgdPxls.begin(); it != bgdPxls.end(); ++it)
		circle(res, *it, radius, BLUE, thickness);
	for (it = fgdPxls.begin(); it != fgdPxls.end(); ++it)
		circle(res, *it, radius, RED, thickness);
	for (it = prBgdPxls.begin(); it != prBgdPxls.end(); ++it)
		circle(res, *it, radius, LIGHTBLUE, thickness);
	for (it = prFgdPxls.begin(); it != prFgdPxls.end(); ++it)
		circle(res, *it, radius, PINK, thickness);

	if (rectState == IN_PROCESS || rectState == SET)
		rectangle(res, cv::Point(rect.x, rect.y), cv::Point(rect.x + rect.width, rect.y + rect.height), GREEN, 2);

	if (!showAlphaMap && !showContours)
		cv::imshow(*winName, res);
	else if (showAlphaMap){
		cv::imshow(*winName, *extractedAlphaMap);
	}
	else if (showContours) {
		//FindDrawContours(extractedAlphaMap);
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

void GCApplication::setLblsInMask(int flags, cv::Point p, bool isPr)
{
	vector<cv::Point> *bpxls, *fpxls;
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
			rect = Rect(cv::Point(rect.x, rect.y), cv::Point(x, y));
			rectState = SET;
			setRectInMask();
			CV_Assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
			showImage();
		}
		if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, cv::Point(x, y), false);
			lblsState = SET;
			showImage();
		}
		break;
	case EVENT_RBUTTONUP:
		if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, cv::Point(x, y), true);
			prLblsState = SET;
			showImage();
		}
		break;
	case EVENT_MOUSEMOVE:
		if (rectState == IN_PROCESS)
		{
			rect = Rect(cv::Point(rect.x, rect.y), cv::Point(x, y));
			CV_Assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
			showImage();
		}
		else if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, cv::Point(x, y), false);
			showImage();
		}
		else if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, cv::Point(x, y), true);
			showImage();
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
	Vector<cv::Point> points;
	Vector3f acoefs;
	Vector3f bcoefs;

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

	int updateCoefs(hsTriangle& newPos){
		Matrix3f A;
		Vector3f b1;
		Vector3f b2;
		A << 1, this->a.position.second, this->a.position.first, 1, this->b.position.second, this->b.position.first,
			1, this->c.position.second, this->c.position.first;
		b1 << newPos.a.position.first, newPos.b.position.first, newPos.c.position.first;
		b2 << newPos.a.position.first, newPos.b.position.first, newPos.c.position.first;



		acoefs = A.colPivHouseholderQr().solve(b1);
		bcoefs = A.colPivHouseholderQr().solve(b2);

		//cout << "Here is the matrix A:\n" << A << endl;
		//cout << "Here is the vector b:\n" << b1 << endl;
		//cout << "The solution is1:\n" << acoefs << endl;
		//cout << "The solution is2:\n" << bcoefs << endl;

		return 0;
	}

	cv::Point getTransformed1(int indexOfPoint){
		float u = acoefs(0) + acoefs(1)*points[indexOfPoint].y + acoefs(2)*points[indexOfPoint].x;
		float v = bcoefs(0) + bcoefs(1)*points[indexOfPoint].y + bcoefs(2)*points[indexOfPoint].x;
		return cv::Point(u, v);
	}

	cv::Point getTransformed(hsTriangle& newTri, cv::Point p){
		hsVertexBary temp = convToBary(p.x, p.y, *this);
		float u = temp.alpha*newTri.a.position.first + temp.beta*newTri.b.position.first + temp.gamma*newTri.c.position.first;
		float v = temp.alpha*newTri.a.position.second + temp.beta*newTri.b.position.second + temp.gamma*newTri.c.position.second;
		return cv::Point(u, v);
	}

	cv::Point getTransformed(cv::Point p){
		float u = acoefs(0) + acoefs(1)*p.y + acoefs(2)*p.x;
		float v = bcoefs(0) + bcoefs(1)*p.y + bcoefs(2)*p.x;
		return cv::Point(u, v);
	}

};

static void on_mouse(int event, int x, int y, int flags, void* param)
{
	gcapp.mouseClick(event, x, y, flags, param);
}

int runExtractAlpha(){

	Mat image = imread("C:\\Users\\hooman\\Desktop\\dressup\\shirtOnWhite.png", IMREAD_COLOR);
	textureP = new Mat(image.size(), image.type());
	hsImage = new Mat(image);
	contours_img = Mat::zeros(image.size(), CV_8UC1);

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
	gcapp.showImage();

	for (;;)
	{
		int c = waitKey(0);
		switch ((char)c)
		{
		case '\x1b':
			cout << "Exiting ..." << endl;
			return extractAlphaExit();
		case 'e':
			return 0;
		case 'r':
			cout << endl;
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
				gcapp.showImage();
				cout << iterCount << ">" << endl;
			}
			else
				cout << "rect must be determined>" << endl;
			break;
		}
	}
}

int extractAlphaExit(){
	destroyAllWindows;
	return 0;
}

int createNodeFile(vector<vector<cv::Point> > contours, bool flag) {

	if (contours.size() != 1){
		cout << "contours size not 1\n";
		return -1;
	}

	ofstream myfile;
	myfile.open("createdBoundary.node");
	myfile
		<< "# createdBoundary.node\n"
		<< "#\n"
		<< contours[0].size() << "	" << 2 << "	" << 0 << "	" << 0 << "\n"
		<< "# and here are the points:\n";

	if (flag){
		for (int i = 0; i < contours[0].size(); i++){
			myfile << i << "	" << contours[0][i].x << "	" << contours[0][i].y << "	" << 0 << "	" << 0 << "\n";
		}
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
		circle(*textureP, cv::Point(a.first, a.second), 1, sGreen, -1);
		circle(*textureP, cv::Point(b.first, b.second), 1, sGreen, -1);
		circle(*textureP, cv::Point(c.first, c.second), 1, sGreen, -1);

		line(*textureP, cv::Point(a.first, a.second), cv::Point(b.first, b.second), sGreen, 1);
		line(*textureP, cv::Point(a.first, a.second), cv::Point(c.first, c.second), sGreen, 1);
		line(*textureP, cv::Point(b.first, b.second), cv::Point(c.first, c.second), sGreen, 1);
	}
	else{
		circle(*textureP, cv::Point(a.first, a.second), 1, sA, -1);
		circle(*textureP, cv::Point(b.first, b.second), 1, sB, -1);
		circle(*textureP, cv::Point(c.first, c.second), 1, sC, -1);

		line(*textureP, cv::Point(a.first, a.second), cv::Point(b.first, b.second), sRED, 1);
		line(*textureP, cv::Point(a.first, a.second), cv::Point(c.first, c.second), sRED, 1);
		line(*textureP, cv::Point(b.first, b.second), cv::Point(c.first, c.second), sRED, 1);
	}
}

void hsDrawMesh() {

	for (int i = 0; i < triangles.size(); i++){

		//	if (i == 3)
		drawTriagnle(triangles[i], false);
		//	else
		//	drawTriagnle(triangles[i]);
	}
}

hsVertexBary convToBary(float x, float y, hsTriangle& tri);

void hsColorTri3() {

	const Scalar S = Scalar(0, 255, 0);

	for (int j = 0; j < textureP->rows; j++){
		for (int i = 0; i < textureP->cols; i++) {

			hsVertexBary temp = convToBary(i, j, triangles[3]);
			if (isInTriangle(temp)) {
				//	circle(*textureP, Point(i, j), 1, S, -1);
				Vec3b color = hsImage->at<Vec3b>(cv::Point(i, j));
				textureP->at<Vec3b>(cv::Point(i, j)) = color;
			}
		}
	}
}

void transformImage(){

	int r = hsImage->rows;
	int c = hsImage->cols;
	Mat* trans = new Mat(r, c, CV_8UC3);

	hsTriangle tr1(0, hsVertex(0, hspair(10, 30)), hsVertex(1, hspair(10, 40)), hsVertex(2, hspair(20, 30)));
	hsTriangle tr2(1, hsVertex(0, hspair(20, 40)), hsVertex(1, hspair(20, 50)), hsVertex(2, hspair(30, 40)));

	tr1.updateCoefs(tr2);
	for (int j = 0; j < hsImage->rows; j++){
		for (int i = 0; i < hsImage->cols; i++) {

			Vec3b color = hsImage->at<Vec3b>(cv::Point(i, j));
			cv::Point temp = tr1.getTransformed(cv::Point(i, j));
			if (temp.x < trans->rows && temp.y < trans->cols)
				trans->at<Vec3b>(temp) = color;
		}
	}

	cv::imshow("orig", *hsImage);
	cv::imshow("trans", *trans);
}


bool load = true;
void hsLoadTrianglePoints() {

	const Scalar S = Scalar(0, 255, 0);

	for (int j = 0; j < textureP->rows; j++){
		for (int i = 0; i < textureP->cols; i++) {

			for (int t = 0; t < triangles.size(); t++){

				hsVertexBary temp = convToBary(i, j, triangles[t]);
				if (isInTriangle(temp)) {
					//	circle(*textureP, Point(i, j), 1, S, -1);
					//	Vec3b color = hsImage->at<Vec3b>(cv::Point(i, j));
					//	textureP->at<Vec3b>(cv::Point(i, j)) = color;
					if (load){
						triangles[t].points.push_back(cv::Point(i, j));
					}
				}
			}
		}
	}
	load = false;
}

void loadMesh() {

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
	hsColorTri3();
	cv::imshow("mesh", *textureP);
	cv::imshow("orig", *hsImage);
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

	alpha = ((y2 - y3)*(x - x3) + (x3 - x2)*(y - y3)) / ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
	beta = ((y3 - y1)*(x - x3) + (x1 - x3)*(y - y3)) / ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
	gama = float(1.0) - alpha - beta;

	if (!(alpha >= 0 && alpha <= 1 && beta >= 0 && beta <= 1 && gama >= 0 && gama <= 1)){
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

int findTriangle(int x, int y){

	for (int i = 0; i < triangles.size(); i++){
		hsVertexBary temp = convToBary(x, y, triangles[i]);
		if (isInTriangle(temp))
			return triangles[i].triangleId;
	}

	return -1;
}

void hsColorTrianglesOnMesh(){


	for (int tri = 0; tri < triangles.size(); tri++){

		for (int i = 0; i < triangles[tri].points.size(); i++){
			cv::Point temp = triangles[tri].points[i];
			Vec3b color = hsImage->at<Vec3b>(temp);
			cv::Point temp1 = triangles[tri].getTransformed(deformedTriangles[tri], temp);
			textureP->at<Vec3b>(temp1) = color;
		}
	}

	/*
	const Scalar G = Scalar(0, 255, 0);
	const Scalar B = Scalar(255, 0, 0);
	int tri = 3;
	triangles[tri].updateCoefs(deformedTriangles[tri]);

	cv::Point temp = cv::Point(triangles[tri].a.position.first, triangles[tri].a.position.second);
	cv::Point temp1 = triangles[tri].getTransformed(temp);
	circle(*textureP, temp, 5, B, -1);
	circle(*textureP, temp1, 5, G, -1);

	cv::Point btemp = cv::Point(triangles[tri].b.position.first, triangles[tri].b.position.second);
	cv::Point btemp1 = triangles[tri].getTransformed(temp);
	circle(*textureP, btemp, 5, B, -1);
	circle(*textureP, btemp1, 5, G, -1);

	cv::Point ctemp = cv::Point(triangles[tri].c.position.first, triangles[tri].c.position.second);
	cv::Point ctemp1 = triangles[tri].getTransformed(temp);
	circle(*textureP, ctemp, 5, B, -1);
	circle(*textureP, ctemp1, 5, G, -1); */
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
		myfile << vertices[i].vertexId << "	" << vertices[i].position.first << "	" << vertices[i].position.second << "	" << 0 << "\n";
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

	myfile1.close();
}

void loadGlutMesh() {

	if (textureP != NULL && hsImage != NULL){
		delete textureP;
		textureP = new Mat(hsImage->size(), hsImage->type());
	}

	//HS this is where we retreive the vertices****************
	const std::vector<Vertex>& verts = Glut_m_deform_obj.getDeformedVerts();
	const size_t npVertices = verts.size();

	for (int i = 0; i < npVertices; i++) {

		hsVertex temp(i, verts[i].vPosition[0], verts[i].vPosition[1]);
		vertices[i] = temp;
	}

	// this is drawing triangles edges ******************HS
	const std::vector<Triangle>& vTriangles = Glut_m_deform_obj.getTriangles();
	const size_t nbTriangles = vTriangles.size();

	for (int i = 0; i < nbTriangles; i++) {
		hsTriangle temp(i, vertices[vTriangles[i].nVertices[0]], vertices[vTriangles[i].nVertices[1]], vertices[vTriangles[i].nVertices[2]]);
		triangles[i] = temp;
	}

	//	hsColorTri3();

	hsLoadTrianglePoints();
	//	hsColorTrianglesOnMesh();
	hsDrawMesh();
	//	saveMeshFile();
	cv::imshow("mesh", *textureP);
	//	imshow("orig", *hsImage); 
}

void loadGlutDeformedMesh() {

	if (textureP != NULL && hsImage != NULL){
		delete textureP;
		textureP = new Mat(hsImage->size(), hsImage->type());
	}

	//HS this is where we retreive the vertices****************
	const std::vector<Vertex>& verts = Glut_m_deform_obj.getDeformedVerts();
	const size_t npVertices = verts.size();

	for (int i = 0; i < npVertices; i++) {

		hsVertex temp(i, verts[i].vPosition[0], verts[i].vPosition[1]);
		deformedVertices[i] = temp;
	}

	// this is drawing triangles edges ******************HS
	const std::vector<Triangle>& vTriangles = Glut_m_deform_obj.getTriangles();
	const size_t nbTriangles = vTriangles.size();

	for (int i = 0; i < nbTriangles; i++) {
		hsTriangle temp(i, deformedVertices[vTriangles[i].nVertices[0]], deformedVertices[vTriangles[i].nVertices[1]], deformedVertices[vTriangles[i].nVertices[2]]);
		deformedTriangles[i] = temp;
	}


	hsColorTrianglesOnMesh();

	//draw the mesh:
	for (int i = 0; i < deformedTriangles.size(); i++){
		drawTriagnle(deformedTriangles[i], false);
	}

	cv::imshow("mesh", *textureP);
}


void hsStartFreeGlut(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(GlutWinWidth, GlutWinHeight);
	glutInitWindowPosition(100, 100);
	GlutwinID = glutCreateWindow("Test OpenGL");
	GlutInit();
	glutDisplayFunc(GlutDisplay);
	glutReshapeFunc(GlutReshape);
	glutKeyboardFunc(GlutKeyboardCommon);
	glutMotionFunc(GlutOnMouseMove);
	glutMouseFunc(GlutMouse);
	glutMainLoop();
}


void runExtractContours() {

	findContours(*gcapp.extractedAlphaMap, contours0, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

	contours.resize(contours0.size());
	for (size_t k = 0; k < contours0.size(); k++)
		approxPolyDP(Mat(contours0[k]), contours[k], 3, true);

	contours_img = Mat::zeros(gcapp.extractedAlphaMap->size(), CV_8UC1);


	//Levels <= 0 all look the same for the shirt. But >0 obviously are smaller stuff
	int levels = 0;
	//*********************** TO successfully create the mesh the level should be 0 so that we get only the outer region.
	// so that out contours vector has a size of 1 (we need to have only 1 contour which is the boundary of our image)
	cout << "contours size is: " << contours.size() << "\n";
	cout << "first contours size is: " << contours[0].size() << "\n";


	//The following 2 lines is the check in drawContour that throws an error if _levels doesnt pass the assert
	//	size_t ncontours = contours.size();
	//	CV_Assert(0 <= _levels && _levels < (int)ncontours);

	//in elow, the 3rd param levels used to be _levels <= 0 ? 3 : -1
	drawContours(contours_img, contours, levels, Scalar(128, 255, 255),
		3, 16, hierarchy, std::abs(levels));

	cv::imshow("contours", contours_img);

	if (createNodeFile(contours, true) == 0)
		cout << "Successfully created node file\n";
	else
		cout << "failed to create node file\n";
}

int main(int argc, char** argv) {

	if (runExtractAlpha() != 0){
		return -1;
	}

	runExtractContours();
	waitKey();

	//Draw the mesh with image on it
	//	loadMesh();
	//	waitKey();


	hsStartFreeGlut(argc, argv);
	delete gcapp.extractedAlphaMap;
	extractAlphaExit();

	/*	Mat image = imread("C:\\Users\\hooman\\Desktop\\dressup\\shirtOnWhite.png", IMREAD_COLOR);
	hsImage = new Mat(image);

	transformImage();
	waitKey();*/
}
















//WORKING COPY: Merged FreeGlut and First
#include "first.h"
#include "GCApplication.h"


//HS VARIABLES:
class hsVertex;
class hsTriangle;
class hsVertexBary;

GCApplication gcapp;
ArapDeform Glut_m_deform_obj;

Mat* textureP = NULL;
Mat* hsImage = NULL;
Mat contours_img;
vector<vector<cv::Point> > contours0;

set<size_t> m_Selected;
size_t Glut_m_nSelected = std::numeric_limits<size_t>::max();

map<int, hsVertex> vertices;
map<int, hsTriangle> triangles;

const int w = 500;
vector<vector<cv::Point> > contours;
vector<Vec4i> hierarchy;

bool isInTriangle(hsVertexBary& point);
int extractAlphaExit();
void loadGlutMesh();

int GlutwinID;
int GlutWinWidth = 800;
int GlutWinHeight = 600;



void GlutDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, (GLsizei)GlutWinWidth, (GLsizei)GlutWinHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, GlutWinWidth, GlutWinHeight, 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushMatrix();

	glPointSize(5.0f);

	//HS this is where we retreive the vertices****************
	const std::vector<Vertex>& verts = Glut_m_deform_obj.getDeformedVerts();
	const size_t np = verts.size();

	if (Glut_m_deform_obj.isAddPoint())
	{
		glColor3f(0.0, 1.0, 0.0);
		glBegin(GL_POINTS);

		for (int i = 0; i < np; i++)
		{
			//HS this is where we draw the vertices BEFORE THE MESH ********
			glVertex2f(verts[i].vPosition[0], verts[i].vPosition[1]);
		}
		glEnd();
	}
	else
	{
		glColor3f(0.0, 0.0, 1.0);
		glBegin(GL_POINTS);

		for (int i = 0; i < np; i++)
		{
			//HS this is where we draw the vertices IN THE MESH ********
			glVertex2f(verts[i].vPosition[0], verts[i].vPosition[1]);
		}
		glEnd();

		// this is drawing triangles edges ******************HS
		const std::vector<Triangle>& vTriangles = Glut_m_deform_obj.getTriangles();
		const size_t nt = vTriangles.size();

		glBegin(GL_LINES);
		for (int i = 0; i < nt; i++)
		{
			const size_t * tri = vTriangles[i].nVertices;

			// This is freeGlut drawing lines. Disable it no edges drawn just the vertices
			glVertex2f(verts[tri[0]].vPosition[0], verts[tri[0]].vPosition[1]);
			glVertex2f(verts[tri[1]].vPosition[0], verts[tri[1]].vPosition[1]);

			glVertex2f(verts[tri[1]].vPosition[0], verts[tri[1]].vPosition[1]);
			glVertex2f(verts[tri[2]].vPosition[0], verts[tri[2]].vPosition[1]);

			glVertex2f(verts[tri[2]].vPosition[0], verts[tri[2]].vPosition[1]);
			glVertex2f(verts[tri[0]].vPosition[0], verts[tri[0]].vPosition[1]);
		}
		glEnd();
	}

	// draw controllers
	glColor3f(1.0, 0.0, 0.0);
	std::set<size_t>::iterator cur(m_Selected.begin()), end(m_Selected.end());
	glBegin(GL_POINTS);

	while (cur != end) {
		size_t nSelected = *cur++;
		glVertex2f(verts[nSelected].vPosition[0], verts[nSelected].vPosition[1]);
	}
	glEnd();

	//glFlush();
	glPopMatrix();

	glutSwapBuffers();
}

void GlutKeyboardCommon(unsigned char key, int x, int y){

	if (key == '\x1b')
	{
		glutDestroyWindow(GlutwinID);
	}
	if ((key == 'A') || (key == 'a')) {
		Glut_m_deform_obj.setAddPoint();
		if (!Glut_m_deform_obj.isAddPoint())
		{
			Glut_m_deform_obj.buildMesh();
		}
	}

	if ((key == 'C') || (key == 'c'))
	{
		cout << "Clear every thing!" << endl;

		Glut_m_nSelected = std::numeric_limits<size_t>::max();
		m_Selected.clear();
		Glut_m_deform_obj.clearData();
	}

	if ((key == 'L') || (key == 'l'))
	{
		cout << "Loading the mesh\n";
		loadGlutMesh();
		//waitKey();
	}

	if ((key == 'Q') || (key == 'q'))  //HS MY stuff 
	{
		ifstream GlutInfile("sampleBoundary.node");
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
				Vertex point(x, y);
				point.vPosition << x, y;
				Glut_m_deform_obj.addPoint(point);
			}
		}
		else{
			cout << "Unable to open file\n";
		}

		GlutInfile.close();
		m_Selected.clear();
		Glut_m_nSelected = std::numeric_limits<size_t>::max();
		Glut_m_deform_obj.updateConstraints(m_Selected);
	}

	if ((key == 'v') || (key == 'V'))
	{
		cout << "Clear constraints!" << endl;
		m_Selected.clear();
		Glut_m_nSelected = std::numeric_limits<size_t>::max();
	}

	glutPostRedisplay();
}

void GlutSpinDisplay(void)
{
	glutPostRedisplay();
}

void GlutInit(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glShadeModel(GL_FLAT);
}

void GlutReshape(int w, int h)
{
	GlutWinWidth = w;
	GlutWinHeight = h;
}

void GlutMouse(int button, int state, int x, int y)
{
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			//glutIdleFunc(GlutSpinDisplay);
			cout << "Current mouse position: " << x << " " << y << endl;
			if (Glut_m_deform_obj.isAddPoint())
			{ //HS 
				Vertex point((float)x, (float)y);
				cout << "add point:" << x << " " << y << endl;
				point.vPosition << (float)x, (float)y;

				Glut_m_deform_obj.addPoint(point);

				m_Selected.clear();
				Glut_m_nSelected = std::numeric_limits<size_t>::max();
				Glut_m_deform_obj.updateConstraints(m_Selected);
			}
			else
			{
				Glut_m_nSelected = Glut_m_deform_obj.findHitVertex((float)x, (float)y);
				if (m_Selected.find(Glut_m_nSelected) == m_Selected.end())
					Glut_m_nSelected = std::numeric_limits<size_t>::max();
			}
		}
		else
		{
			Glut_m_nSelected = std::numeric_limits<size_t>::max();
		}

		break;
	case GLUT_MIDDLE_BUTTON:
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			cout << "Add control point at point: " << x << " " << y << endl;
			size_t nHit = Glut_m_deform_obj.findHitVertex((float)x, (float)y);
			if (nHit < std::numeric_limits<size_t>::max())
			{
				cout << "Find the vertex" << endl;
				if (m_Selected.find(nHit) == m_Selected.end())
					m_Selected.insert(nHit);
				else
				{
					m_Selected.erase(nHit);
				}
				// once modified the constraints, we should update them
				// i.e., precompute Gprime and B
				Glut_m_deform_obj.updateConstraints(m_Selected);
			}
			else
			{
				cout << "Doesn't find the vertex" << endl;
			}

		}

		break;
	default:
		break;
	}

	glutPostRedisplay();
}


void GlutOnMouseMove(int x, int y)
{
	if (Glut_m_nSelected != std::numeric_limits<size_t>::max()) {
		GlutPoint newPos(x, y);
		Glut_m_deform_obj.setVertex(Glut_m_nSelected, newPos);
		Glut_m_deform_obj.updateMesh(false);
		glutPostRedisplay();
	}
}


static void help()
{
	cout << "\nKosse nane raja -- select an object in a region\n"
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
	if (_image.empty() || _winName.empty())
		return;
	image = &_image;
	winName = &_winName;
	mask.create(image->size(), CV_8UC1);
	reset();
}

void GCApplication::showImage()
{

	if (image->empty() || winName->empty())
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


	vector<cv::Point>::const_iterator it;
	for (it = bgdPxls.begin(); it != bgdPxls.end(); ++it)
		circle(res, *it, radius, BLUE, thickness);
	for (it = fgdPxls.begin(); it != fgdPxls.end(); ++it)
		circle(res, *it, radius, RED, thickness);
	for (it = prBgdPxls.begin(); it != prBgdPxls.end(); ++it)
		circle(res, *it, radius, LIGHTBLUE, thickness);
	for (it = prFgdPxls.begin(); it != prFgdPxls.end(); ++it)
		circle(res, *it, radius, PINK, thickness);

	if (rectState == IN_PROCESS || rectState == SET)
		rectangle(res, cv::Point(rect.x, rect.y), cv::Point(rect.x + rect.width, rect.y + rect.height), GREEN, 2);

	if (!showAlphaMap && !showContours)
		imshow(*winName, res);
	else if (showAlphaMap){
		imshow(*winName, *extractedAlphaMap);
	}
	else if (showContours) {
		//FindDrawContours(extractedAlphaMap);
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

void GCApplication::setLblsInMask(int flags, cv::Point p, bool isPr)
{
	vector<cv::Point> *bpxls, *fpxls;
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
			rect = Rect(cv::Point(rect.x, rect.y), cv::Point(x, y));
			rectState = SET;
			setRectInMask();
			CV_Assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
			showImage();
		}
		if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, cv::Point(x, y), false);
			lblsState = SET;
			showImage();
		}
		break;
	case EVENT_RBUTTONUP:
		if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, cv::Point(x, y), true);
			prLblsState = SET;
			showImage();
		}
		break;
	case EVENT_MOUSEMOVE:
		if (rectState == IN_PROCESS)
		{
			rect = Rect(cv::Point(rect.x, rect.y), cv::Point(x, y));
			CV_Assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
			showImage();
		}
		else if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, cv::Point(x, y), false);
			showImage();
		}
		else if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, cv::Point(x, y), true);
			showImage();
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

static void on_mouse(int event, int x, int y, int flags, void* param)
{
	gcapp.mouseClick(event, x, y, flags, param);
}

int runExtractAlpha(){

	Mat image = imread("C:\\Users\\hooman\\Desktop\\dressup\\shirtOnWhite.png", IMREAD_COLOR);
	textureP = new Mat(image.size(), image.type());
	hsImage = new Mat(image);
	contours_img = Mat::zeros(image.size(), CV_8UC1);

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
	gcapp.showImage();

	for (;;)
	{
		int c = waitKey(0);
		switch ((char)c)
		{
		case '\x1b':
			cout << "Exiting ..." << endl;
			return extractAlphaExit();
		case 'e':
			return 0;
		case 'r':
			cout << endl;
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
				gcapp.showImage();
				cout << iterCount << ">" << endl;
			}
			else
				cout << "rect must be determined>" << endl;
			break;
		}
	}
}

int extractAlphaExit(){
	destroyAllWindows;
	return 0;
}

int createNodeFile(vector<vector<cv::Point> > contours, bool flag) {

	if (contours.size() != 1){
		cout << "contours size not 1\n";
		return -1;
	}

	ofstream myfile;
	myfile.open("createdBoundary.node");
	myfile
		<< "# createdBoundary.node\n"
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
		circle(*textureP, cv::Point(a.first, a.second), 1, sGreen, -1);
		circle(*textureP, cv::Point(b.first, b.second), 1, sGreen, -1);
		circle(*textureP, cv::Point(c.first, c.second), 1, sGreen, -1);

		line(*textureP, cv::Point(a.first, a.second), cv::Point(b.first, b.second), sGreen, 1);
		line(*textureP, cv::Point(a.first, a.second), cv::Point(c.first, c.second), sGreen, 1);
		line(*textureP, cv::Point(b.first, b.second), cv::Point(c.first, c.second), sGreen, 1);
	}
	else{
		circle(*textureP, cv::Point(a.first, a.second), 1, sA, -1);
		circle(*textureP, cv::Point(b.first, b.second), 1, sB, -1);
		circle(*textureP, cv::Point(c.first, c.second), 1, sC, -1);

		line(*textureP, cv::Point(a.first, a.second), cv::Point(b.first, b.second), sRED, 1);
		line(*textureP, cv::Point(a.first, a.second), cv::Point(c.first, c.second), sRED, 1);
		line(*textureP, cv::Point(b.first, b.second), cv::Point(c.first, c.second), sRED, 1);
	}
}

void hsDrawMesh() {

	if (textureP != NULL && hsImage != NULL){
		delete textureP;
		textureP = new Mat(hsImage->size(), hsImage->type());
	}

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
				Vec3b color = hsImage->at<Vec3b>(cv::Point(i, j));
				textureP->at<Vec3b>(cv::Point(i, j)) = color;
			}
		}
	}
}

void loadMesh() {

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

	alpha = ((y2 - y3)*(x - x3) + (x3 - x2)*(y - y3)) / ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
	beta = ((y3 - y1)*(x - x3) + (x1 - x3)*(y - y3)) / ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
	gama = float(1.0) - alpha - beta;

	if (!(alpha >= 0 && alpha <= 1 && beta >= 0 && beta <= 1 && gama >= 0 && gama <= 1)){
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

void loadGlutMesh() {

	//HS this is where we retreive the vertices****************
	const std::vector<Vertex>& verts = Glut_m_deform_obj.getDeformedVerts();
	const size_t npVertices = verts.size();

	for (int i = 0; i < npVertices; i++) {

		hsVertex temp(i, verts[i].vPosition[0], verts[i].vPosition[1]);
		vertices[i] = temp;
	}

	// this is drawing triangles edges ******************HS
	const std::vector<Triangle>& vTriangles = Glut_m_deform_obj.getTriangles();
	const size_t nbTriangles = vTriangles.size();

	for (int i = 0; i < nbTriangles; i++) {
		hsTriangle temp(i, vertices[vTriangles[i].nVertices[0]], vertices[vTriangles[i].nVertices[1]], vertices[vTriangles[i].nVertices[2]]);
		triangles[i] = temp;
	}

	hsDrawMesh();
	hsColorTri3();
	imshow("mesh", *textureP);
	//	imshow("orig", *hsImage); 
}

void hsStartFreeGlut(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(GlutWinWidth, GlutWinHeight);
	glutInitWindowPosition(100, 100);
	GlutwinID = glutCreateWindow("Test OpenGL");
	GlutInit();
	glutDisplayFunc(GlutDisplay);
	glutReshapeFunc(GlutReshape);
	glutKeyboardFunc(GlutKeyboardCommon);
	glutMotionFunc(GlutOnMouseMove);
	glutMouseFunc(GlutMouse);
	glutMainLoop();
}


void runExtractContours() {

	findContours(*gcapp.extractedAlphaMap, contours0, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

	contours.resize(contours0.size());
	for (size_t k = 0; k < contours0.size(); k++)
		approxPolyDP(Mat(contours0[k]), contours[k], 3, true);

	contours_img = Mat::zeros(gcapp.extractedAlphaMap->size(), CV_8UC1);


	//Levels <= 0 all look the same for the shirt. But >0 obviously are smaller stuff
	int levels = 0;
	//*********************** TO successfully create the mesh the level should be 0 so that we get only the outer region.
	// so that out contours vector has a size of 1 (we need to have only 1 contour which is the boundary of our image)
	cout << "contours size is: " << contours.size() << "\n";
	cout << "first contours size is: " << contours[0].size() << "\n";


	//The following 2 lines is the check in drawContour that throws an error if _levels doesnt pass the assert
	//	size_t ncontours = contours.size();
	//	CV_Assert(0 <= _levels && _levels < (int)ncontours);

	//in elow, the 3rd param levels used to be _levels <= 0 ? 3 : -1
	drawContours(contours_img, contours, levels, Scalar(128, 255, 255),
		3, 16, hierarchy, std::abs(levels));

	imshow("contours", contours_img);

	if (createNodeFile(contours, true) == 0)
		cout << "Successfully created node file\n";
	else
		cout << "failed to create node file\n";
}

int main(int argc, char** argv) {


	if (runExtractAlpha() != 0){
		return -1;
	}

	runExtractContours();
	waitKey();

	//Draw the mesh with image on it
	//	loadMesh();
	//	waitKey();


	hsStartFreeGlut(argc, argv);
	delete gcapp.extractedAlphaMap;
	extractAlphaExit();
}

















//WORKING COPY: first merge of freeglut and first

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <freeglut.h>

#include <glut.h>
#include <set>

#include "ArapDeform.h"
#include "common.h"

#include <math.h>
#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include<vector>
#include<fstream>



using namespace cv;
using namespace std;

using namespace zzs;

using Eigen::MatrixXd;

int GlutwinID;
int GlutWinWidth = 800;
int GlutWinHeight = 600;

set<size_t> m_Selected;
size_t Glut_m_nSelected = std::numeric_limits<size_t>::max();

ArapDeform Glut_m_deform_obj;

void GlutDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, (GLsizei)GlutWinWidth, (GLsizei)GlutWinHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, GlutWinWidth, GlutWinHeight, 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushMatrix();

	glPointSize(5.0f);

	//HS this is where we retreive the vertices****************
	const std::vector<Vertex>& verts = Glut_m_deform_obj.getDeformedVerts();
	const size_t np = verts.size();

	if (Glut_m_deform_obj.isAddPoint())
	{
		glColor3f(0.0, 1.0, 0.0);
		glBegin(GL_POINTS);

		for (int i = 0; i < np; i++)
		{
			//HS this is where we draw the vertices BEFORE THE MESH ********
			glVertex2f(verts[i].vPosition[0], verts[i].vPosition[1]);
		}
		glEnd();
	}
	else
	{
		glColor3f(0.0, 0.0, 1.0);
		glBegin(GL_POINTS);

		for (int i = 0; i < np; i++)
		{
			//HS this is where we draw the vertices IN THE MESH ********
			glVertex2f(verts[i].vPosition[0], verts[i].vPosition[1]);
		}
		glEnd();

		// this is drawing triangles edges ******************HS
		const std::vector<Triangle>& vTriangles = Glut_m_deform_obj.getTriangles();
		const size_t nt = vTriangles.size();

		glBegin(GL_LINES);
		for (int i = 0; i < nt; i++)
		{
			const size_t * tri = vTriangles[i].nVertices;

			// This is freeGlut drawing lines. Disable it no edges drawn just the vertices
			glVertex2f(verts[tri[0]].vPosition[0], verts[tri[0]].vPosition[1]);
			glVertex2f(verts[tri[1]].vPosition[0], verts[tri[1]].vPosition[1]);

			glVertex2f(verts[tri[1]].vPosition[0], verts[tri[1]].vPosition[1]);
			glVertex2f(verts[tri[2]].vPosition[0], verts[tri[2]].vPosition[1]);

			glVertex2f(verts[tri[2]].vPosition[0], verts[tri[2]].vPosition[1]);
			glVertex2f(verts[tri[0]].vPosition[0], verts[tri[0]].vPosition[1]);
		}
		glEnd();
	}

	// draw controllers
	glColor3f(1.0, 0.0, 0.0);
	std::set<size_t>::iterator cur(m_Selected.begin()), end(m_Selected.end());
	glBegin(GL_POINTS);

	while (cur != end) {
		size_t nSelected = *cur++;
		glVertex2f(verts[nSelected].vPosition[0], verts[nSelected].vPosition[1]);
	}
	glEnd();

	//glFlush();
	glPopMatrix();

	glutSwapBuffers();
}

void GlutKeyboardCommon(unsigned char key, int x, int y){

	if (key == 27)
	{
		glutDestroyWindow(GlutwinID);
	}
	if ((key == 'A') || (key == 'a')) {
		Glut_m_deform_obj.setAddPoint();
		if (!Glut_m_deform_obj.isAddPoint())
		{
			Glut_m_deform_obj.buildMesh();
		}
	}

	if ((key == 'C') || (key == 'c'))
	{
		cout << "Clear every thing!" << endl;

		Glut_m_nSelected = std::numeric_limits<size_t>::max();
		m_Selected.clear();
		Glut_m_deform_obj.clearData();
	}

	if ((key == 'Q') || (key == 'q'))  //HS MY stuff 
	{
		ifstream GlutInfile("createdBoundary.node");
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
				Vertex point(x, y);
				point.vPosition << x, y;
				Glut_m_deform_obj.addPoint(point);
			}
		}
		else{
			cout << "Unable to open file\n";
		}

		GlutInfile.close();
		m_Selected.clear();
		Glut_m_nSelected = std::numeric_limits<size_t>::max();
		Glut_m_deform_obj.updateConstraints(m_Selected);
	}

	if ((key == 'v') || (key == 'V'))
	{
		cout << "Clear constraints!" << endl;
		m_Selected.clear();
		Glut_m_nSelected = std::numeric_limits<size_t>::max();
	}

	glutPostRedisplay();
}

void GlutSpinDisplay(void)
{
	glutPostRedisplay();
}

void GlutInit(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glShadeModel(GL_FLAT);
}

void GlutReshape(int w, int h)
{
	GlutWinWidth = w;
	GlutWinHeight = h;
}

void GlutMouse(int button, int state, int x, int y)
{
	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			//glutIdleFunc(GlutSpinDisplay);
			cout << "Current mouse position: " << x << " " << y << endl;
			if (Glut_m_deform_obj.isAddPoint())
			{ //HS 
				Vertex point((float)x, (float)y);
				cout << "add point:" << x << " " << y << endl;
				point.vPosition << (float)x, (float)y;

				Glut_m_deform_obj.addPoint(point);

				m_Selected.clear();
				Glut_m_nSelected = std::numeric_limits<size_t>::max();
				Glut_m_deform_obj.updateConstraints(m_Selected);
			}
			else
			{
				Glut_m_nSelected = Glut_m_deform_obj.findHitVertex((float)x, (float)y);
				if (m_Selected.find(Glut_m_nSelected) == m_Selected.end())
					Glut_m_nSelected = std::numeric_limits<size_t>::max();
			}
		}
		else
		{
			Glut_m_nSelected = std::numeric_limits<size_t>::max();
		}

		break;
	case GLUT_MIDDLE_BUTTON:
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			cout << "Add control point at point: " << x << " " << y << endl;
			size_t nHit = Glut_m_deform_obj.findHitVertex((float)x, (float)y);
			if (nHit < std::numeric_limits<size_t>::max())
			{
				cout << "Find the vertex" << endl;
				if (m_Selected.find(nHit) == m_Selected.end())
					m_Selected.insert(nHit);
				else
				{
					m_Selected.erase(nHit);
				}
				// once modified the constraints, we should update them
				// i.e., precompute Gprime and B
				Glut_m_deform_obj.updateConstraints(m_Selected);
			}
			else
			{
				cout << "Doesn't find the vertex" << endl;
			}

		}

		break;
	default:
		break;
	}

	glutPostRedisplay();
}


void GlutOnMouseMove(int x, int y)
{
	if (Glut_m_nSelected != std::numeric_limits<size_t>::max()) {
		GlutPoint newPos(x, y);
		Glut_m_deform_obj.setVertex(Glut_m_nSelected, newPos);
		Glut_m_deform_obj.updateMesh(false);
		glutPostRedisplay();
	}
}


static void help()
{
	cout << "\nKosse nane raja -- select an object in a region\n"
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
	void showImage();
	void mouseClick(int event, int x, int y, int showAlphaMaps, void* param);
	int nextIter();
	int getIterCount() const { return iterCount; }

	const Mat* hsGetOutPutImage() const { return image; }
private:
	void setRectInMask();
	void setLblsInMask(int showAlphaMaps, cv::Point p, bool isPr);

	const string* winName;

	Mat mask;
	Mat bgdModel, fgdModel;

	uchar rectState, lblsState, prLblsState;
	bool isInitialized;

	Rect rect;
	vector<cv::Point> fgdPxls, bgdPxls, prFgdPxls, prBgdPxls;
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
	if (_image.empty() || _winName.empty())
		return;
	image = &_image;
	winName = &_winName;
	mask.create(image->size(), CV_8UC1);
	reset();
}

void GCApplication::showImage()
{

	if (image->empty() || winName->empty())
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


	vector<cv::Point>::const_iterator it;
	for (it = bgdPxls.begin(); it != bgdPxls.end(); ++it)
		circle(res, *it, radius, BLUE, thickness);
	for (it = fgdPxls.begin(); it != fgdPxls.end(); ++it)
		circle(res, *it, radius, RED, thickness);
	for (it = prBgdPxls.begin(); it != prBgdPxls.end(); ++it)
		circle(res, *it, radius, LIGHTBLUE, thickness);
	for (it = prFgdPxls.begin(); it != prFgdPxls.end(); ++it)
		circle(res, *it, radius, PINK, thickness);

	if (rectState == IN_PROCESS || rectState == SET)
		rectangle(res, cv::Point(rect.x, rect.y), cv::Point(rect.x + rect.width, rect.y + rect.height), GREEN, 2);

	if (!showAlphaMap && !showContours)
		imshow(*winName, res);
	else if (showAlphaMap){
		imshow(*winName, *extractedAlphaMap);
	}
	else if (showContours) {
		//FindDrawContours(extractedAlphaMap);
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

void GCApplication::setLblsInMask(int flags, cv::Point p, bool isPr)
{
	vector<cv::Point> *bpxls, *fpxls;
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
			rect = Rect(cv::Point(rect.x, rect.y), cv::Point(x, y));
			rectState = SET;
			setRectInMask();
			CV_Assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
			showImage();
		}
		if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, cv::Point(x, y), false);
			lblsState = SET;
			showImage();
		}
		break;
	case EVENT_RBUTTONUP:
		if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, cv::Point(x, y), true);
			prLblsState = SET;
			showImage();
		}
		break;
	case EVENT_MOUSEMOVE:
		if (rectState == IN_PROCESS)
		{
			rect = Rect(cv::Point(rect.x, rect.y), cv::Point(x, y));
			CV_Assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
			showImage();
		}
		else if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, cv::Point(x, y), false);
			showImage();
		}
		else if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, cv::Point(x, y), true);
			showImage();
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
Mat contours_img;

map<int, hsVertex> vertices;
map<int, hsTriangle> triangles;

const int w = 500;
vector<vector<cv::Point> > contours;
vector<Vec4i> hierarchy;

bool isInTriangle(hsVertexBary& point);

vector<vector<cv::Point> > contours0;



static void on_mouse(int event, int x, int y, int flags, void* param)
{
	gcapp.mouseClick(event, x, y, flags, param);
}

int runExtractAlpha(){

	Mat image = imread("C:\\Users\\hooman\\Desktop\\dressup\\shirtOnWhite.png", IMREAD_COLOR);
	textureP = new Mat(image.size(), image.type());
	hsImage = new Mat(image);
	contours_img = Mat::zeros(image.size(), CV_8UC1);

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
	gcapp.showImage();

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
			gcapp.showImage();
			break;
		case 'm':
			gcapp.showAlphaMap = true;
			gcapp.showImage();
			break;
		case 'k':
			gcapp.showImage();
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
				gcapp.showImage();
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

int createNodeFile(vector<vector<cv::Point> > contours, bool flag) {

	if (contours.size() != 1){
		cout << "contours size not 1\n";
		return -1;
	}

	ofstream myfile;
	myfile.open("createdBoundary.node");
	myfile
		<< "# createdBoundary.node\n"
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
		circle(*textureP, cv::Point(a.first, a.second), 1, sGreen, -1);
		circle(*textureP, cv::Point(b.first, b.second), 1, sGreen, -1);
		circle(*textureP, cv::Point(c.first, c.second), 1, sGreen, -1);

		line(*textureP, cv::Point(a.first, a.second), cv::Point(b.first, b.second), sGreen, 1);
		line(*textureP, cv::Point(a.first, a.second), cv::Point(c.first, c.second), sGreen, 1);
		line(*textureP, cv::Point(b.first, b.second), cv::Point(c.first, c.second), sGreen, 1);
	}
	else{
		circle(*textureP, cv::Point(a.first, a.second), 1, sA, -1);
		circle(*textureP, cv::Point(b.first, b.second), 1, sB, -1);
		circle(*textureP, cv::Point(c.first, c.second), 1, sC, -1);

		line(*textureP, cv::Point(a.first, a.second), cv::Point(b.first, b.second), sRED, 1);
		line(*textureP, cv::Point(a.first, a.second), cv::Point(c.first, c.second), sRED, 1);
		line(*textureP, cv::Point(b.first, b.second), cv::Point(c.first, c.second), sRED, 1);
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
				Vec3b color = hsImage->at<Vec3b>(cv::Point(i, j));
				textureP->at<Vec3b>(cv::Point(i, j)) = color;
			}
		}
	}
}

void loadMesh() {

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

	alpha = ((y2 - y3)*(x - x3) + (x3 - x2)*(y - y3)) / ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
	beta = ((y3 - y1)*(x - x3) + (x1 - x3)*(y - y3)) / ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
	gama = float(1.0) - alpha - beta;

	if (!(alpha >= 0 && alpha <= 1 && beta >= 0 && beta <= 1 && gama >= 0 && gama <= 1)){
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

void hsStartFreeGlut(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(GlutWinWidth, GlutWinHeight);
	glutInitWindowPosition(100, 100);
	GlutwinID = glutCreateWindow("Test OpenGL");
	GlutInit();
	glutDisplayFunc(GlutDisplay);
	glutReshapeFunc(GlutReshape);
	glutKeyboardFunc(GlutKeyboardCommon);
	glutMotionFunc(GlutOnMouseMove);
	glutMouseFunc(GlutMouse);
	glutMainLoop();
}


void runExtractContours() {

	findContours(*gcapp.extractedAlphaMap, contours0, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

	contours.resize(contours0.size());
	for (size_t k = 0; k < contours0.size(); k++)
		approxPolyDP(Mat(contours0[k]), contours[k], 3, true);

	contours_img = Mat::zeros(gcapp.extractedAlphaMap->size(), CV_8UC1);


	//Levels <= 0 all look the same for the shirt. But >0 obviously are smaller stuff
	int levels = 0;
	//*********************** TO successfully create the mesh the level should be 0 so that we get only the outer region.
	// so that out contours vector has a size of 1 (we need to have only 1 contour which is the boundary of our image)
	cout << "contours size is: " << contours.size() << "\n";
	cout << "first contours size is: " << contours[0].size() << "\n";


	//The following 2 lines is the check in drawContour that throws an error if _levels doesnt pass the assert
	//	size_t ncontours = contours.size();
	//	CV_Assert(0 <= _levels && _levels < (int)ncontours);

	//in elow, the 3rd param levels used to be _levels <= 0 ? 3 : -1
	drawContours(contours_img, contours, levels, Scalar(128, 255, 255),
		3, 16, hierarchy, std::abs(levels));

	imshow("contours", contours_img);

	if (createNodeFile(contours, true) == 0)
		cout << "Successfully created node file\n";
	else
		cout << "failed to create node file\n";
}

int main(int argc, char** argv) {


	if (runExtractAlpha() != 0){
		return -1;
	}

	runExtractContours();
	waitKey();

	//Draw the mesh with image on it
	loadMesh();
	waitKey();

	delete gcapp.extractedAlphaMap;
	hsStartFreeGlut(argc, argv);
}













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
	cout << "\nKosse nane raja -- select an object in a region\n"
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
	if (_image.empty() || _winName.empty())
		return;
	image = &_image;
	winName = &_winName;
	mask.create(image->size(), CV_8UC1);
	reset();
}

void GCApplication::showImage(bool writeFile)
{

	if (image->empty() || winName->empty())
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
		cout << "wrote to file: " << line - 1 << "\n";
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
	textureP = new Mat(image.size(), image.type());
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
		circle(*textureP, Point(a.first, a.second), 5, sGreen, -1);
		circle(*textureP, Point(b.first, b.second), 5, sGreen, -1);
		circle(*textureP, Point(c.first, c.second), 5, sGreen, -1);

		line(*textureP, Point(a.first, a.second), Point(b.first, b.second), sGreen, 1);
		line(*textureP, Point(a.first, a.second), Point(c.first, c.second), sGreen, 1);
		line(*textureP, Point(b.first, b.second), Point(c.first, c.second), sGreen, 1);
	}
	else{
		circle(*textureP, Point(a.first, a.second), 5, sA, -1);
		circle(*textureP, Point(b.first, b.second), 5, sB, -1);
		circle(*textureP, Point(c.first, c.second), 5, sC, -1);

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


float Dot(vector<float>& a, vector<float>& b){
	return (a[0] * b[0] + a[1] * a[1]);
}
// Compute barycentric coordinates (u, v, w) for
// point p with respect to triangle (a, b, c)
void Barycentric(vector<float> p, vector<float> a, vector<float> b, vector<float> c, float& u, float& v, float& w)
{
	vector<float> v0(2), v1(2), v2(2);
	v0[0] = b[0] - a[0];
	v0[1] = b[1] - a[1];
	v1[0] = c[0] - a[0];
	v1[1] = c[1] - a[1];
	v2[0] = p[0] - a[0];
	v2[1] = p[1] - a[1];

	/*
	float d00 = Dot(v0, v0);
	float d01 = Dot(v0, v1);
	float d11 = Dot(v1, v1);
	float d20 = Dot(v2, v0);
	float d21 = Dot(v2, v1);
	float denom = d00 * d11 - d01 * d01;
	v = (d11 * d20 - d01 * d21) / denom;
	w = (d00 * d21 - d01 * d20) / denom;
	u = 1.0f - v - w;*/

	float d00 = Dot(v0, v0);
	float d01 = Dot(v0, v1);
	float d11 = Dot(v1, v1);
	float d20 = Dot(v2, v0);
	float d21 = Dot(v2, v1);
	float invDenom = 1.0 / (d00 * d11 - d01 * d01);
	v = (d11 * d20 - d01 * d21) * invDenom;
	w = (d00 * d21 - d01 * d20) * invDenom;
	u = 1.0f - v - w;
}



void hsColorTri3() {

	const Scalar S = Scalar(0, 255, 0);
	unsigned char* texturePData = (unsigned char*)(textureP->data);

	for (int j = 0; j < textureP->rows; j++){
		for (int i = 0; i < textureP->cols; i++) {

			hsVertexBary temp = convToBary(i, j, triangles[3]);
			if (isInTriangle(temp)) {
				/*	texturePData[textureP->step * j + i] = 0;
				texturePData[textureP->step * j + i + 1] = 255;
				texturePData[textureP->step * j + i + 2] = 0;*/
				circle(*textureP, Point(i, j), 1, S, -1);
			}
		}
	}

}

void loadMesh() {

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

	alpha = ((y2 - y3)*(x - x3) + (x3 - x2)*(y - y3)) / ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
	beta = ((y3 - y1)*(x - x3) + (x1 - x3)*(y - y3)) / ((y2 - y3)*(x1 - x3) + (x3 - x2)*(y1 - y3));
	gama = float(1.0) - alpha - beta;

	if (!(alpha >= 0 && alpha <= 1 && beta >= 0 && beta <= 1 && gama >= 0 && gama <= 1)){
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
		cout << "contours size is: " << contours.size() << "\n";
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




















//WORKING COPY: Full GrabCut + extractAlpha + Find and Draw Conttours + draw Triangulated mesh

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
	cout << "\nKosse nane raja -- select an object in a region\n"
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
	const Mat* image;
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
	if (_image.empty() || _winName.empty())
		return;
	image = &_image;
	winName = &_winName;
	mask.create(image->size(), CV_8UC1);
	reset();
}

void GCApplication::showImage(bool writeFile)
{

	if (image->empty() || winName->empty())
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
		cout << "wrote to file: " << line - 1 << "\n";
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


//VARIABLES:
GCApplication gcapp;

const int w = 500;
vector<vector<Point> > contours;
vector<Vec4i> hierarchy;


static void on_mouse(int event, int x, int y, int flags, void* param)
{
	gcapp.mouseClick(event, x, y, flags, param);
}

int runExtractAlpha(){

	Mat image = imread("C:\\Users\\hooman\\Desktop\\dressup\\shirtOnWhite.png", IMREAD_COLOR);
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

class hsVertex{
public:
	Point position;
	int vertexId;

	hsVertex() {
		vertexId = -1;
		Point p(-1, -1);
		position = p;
	}

	hsVertex(int vertextNb, Point position){
		this->vertexId = vertexId;
		this->position = position;
	}

	hsVertex(int vertexNb, int x, int y){
		this->vertexId = vertexNb;
		Point p(x, y);
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
class hsTriangle{
public:
	hsVertex a, b, c;
	int triangleId;

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
};
void drawTriagnle(Mat& img, Point a, Point b, Point c, Mat& texture){

	const Scalar sRED = Scalar(0, 0, 255);

	unsigned char *input = (unsigned char*)(texture.data);
	int i, j;
	double blue, green, red;

	i = a.x;
	j = a.y;
	blue = (double)input[texture.step * j + i];
	green = (double)input[texture.step * j + i + 1];
	red = (double)input[texture.step * j + i + 2];
	Scalar sA(blue, green, red);

	i = b.x;
	j = b.y;
	blue = (double)input[texture.step * j + i];
	green = (double)input[texture.step * j + i + 1];
	red = (double)input[texture.step * j + i + 2];
	Scalar sB(blue, green, red);

	i = c.x;
	j = c.y;
	blue = (double)input[texture.step * j + i];
	green = (double)input[texture.step * j + i + 1];
	red = (double)input[texture.step * j + i + 2];
	Scalar sC(blue, green, red);

	//	delete input;

	circle(texture, a, 5, sA, -1);
	circle(texture, b, 5, sB, -1);
	circle(texture, c, 5, sC, -1);

	line(texture, a, b, sRED, 1);
	line(texture, a, c, sRED, 1);
	line(texture, b, c, sRED, 1);
}

void drawMesh() {

	Mat image = imread("C:\\Users\\hooman\\Desktop\\dressup\\shirtOnWhite.png", IMREAD_COLOR);

	Mat out(image.size(), CV_8UC1);

	map<int, hsVertex> vertices;
	map<int, hsTriangle> triangles;

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
		drawTriagnle(out, vertices[a].position, vertices[b].position, vertices[c].position, image);
	}
	trianglesFile.close();

	imshow("mesh", out);
	imshow("orig", image);
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
		cout << "contours size is: " << contours.size() << "\n";
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
		drawMesh();
		waitKey();

	}

	delete gcapp.extractedAlphaMap;

	return 0;
}








//WORKIGING COPY backup
void drawTriagnle(Mat& img, pair<int, int> a, pair<int, int> b, pair<int, int> c, Mat& texture){

	const Scalar sRED = Scalar(0, 0, 255);

	Point A(a.first, a.second);
	Point B(b.first, b.second);
	Point C(c.first, c.second);

	unsigned char *input = (unsigned char*)(texture.data);
	int i, j;
	double blue, green, red;

	i = a.first;
	j = a.second;
	blue = (double)input[texture.step * j + i];
	green = (double)input[texture.step * j + i + 1];
	red = (double)input[texture.step * j + i + 2];
	Scalar sA(blue, green, red);

	i = b.first;
	j = b.second;
	blue = (double)input[texture.step * j + i];
	green = (double)input[texture.step * j + i + 1];
	red = (double)input[texture.step * j + i + 2];
	Scalar sB(blue, green, red);

	i = c.first;
	j = c.second;
	blue = (double)input[texture.step * j + i];
	green = (double)input[texture.step * j + i + 1];
	red = (double)input[texture.step * j + i + 2];
	Scalar sC(blue, green, red);

	//	delete input;

	circle(texture, A, 5, sA, -1);
	circle(texture, B, 5, sB, -1);
	circle(texture, C, 5, sC, -1);

	line(texture, A, B, sRED, 1);
	line(texture, A, C, sRED, 1);
	line(texture, B, C, sRED, 1);
}

class hsTriangle{
	Point a, b, c;
	int triangleId;

	hsTriangle(const hsTriangle& in){
		this->triangleId = in.triangleId;
		this->a = in.a;
		this->b = in.a;
		this->c = in.a;
	}
};

void drawMesh() {

	Mat image = imread("C:\\Users\\hooman\\Desktop\\dressup\\shirtOnWhite.png", IMREAD_COLOR);

	Mat out(image.size(), CV_8UC1);

	map<int, pair<int, int>> points;
	map<int, hsTriangle> triangles;

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

		drawTriagnle(out, points[a], points[b], points[c], image);
	}
	trianglesFile.close();

	imshow("mesh", out);
	imshow("orig", image);
}








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














//WORKING COPY: Full GrabCut + extractAlpha + Find and Draw Conttours

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <math.h>
#include <iostream>

using namespace cv;
using namespace std;

static void help()
{
	cout << "\nThis program demonstrates GrabCut segmentation -- select an object in a region\n"
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

	bool showAlphaMap = false;
	bool showContours = false;
	Mat* hsout;

	enum{ NOT_SET = 0, IN_PROCESS = 1, SET = 2 };
	static const int radius = 2;
	static const int thickness = -1;

	void reset();
	void setImageAndWinName(const Mat& _image, const string& _winName);
	void showImage();
	void mouseClick(int event, int x, int y, int showAlphaMaps, void* param);
	int nextIter();
	int getIterCount() const { return iterCount; }
private:
	void setRectInMask();
	void setLblsInMask(int showAlphaMaps, Point p, bool isPr);

	const string* winName;
	const Mat* image;
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
	if (_image.empty() || _winName.empty())
		return;
	image = &_image;
	winName = &_winName;
	mask.create(image->size(), CV_8UC1);
	reset();
}

void GCApplication::showImage()
{
	if (image->empty() || winName->empty())
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
	hsout = new Mat(image->size(), CV_8UC1, *n);
	Scalar* p = new Scalar(255);
	hsout->setTo(*p, binMask);


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
		imshow(*winName, *hsout);
	}
	else if (showContours) {
		//FindDrawContours(hsout);
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
			showImage();
		}
		if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), false);
			lblsState = SET;
			showImage();
		}
		break;
	case EVENT_RBUTTONUP:
		if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), true);
			prLblsState = SET;
			showImage();
		}
		break;
	case EVENT_MOUSEMOVE:
		if (rectState == IN_PROCESS)
		{
			rect = Rect(Point(rect.x, rect.y), Point(x, y));
			CV_Assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
			showImage();
		}
		else if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), false);
			showImage();
		}
		else if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), true);
			showImage();
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


//VARIABLES:
GCApplication gcapp;

const int w = 500;
vector<vector<Point> > contours;
vector<Vec4i> hierarchy;


static void on_mouse(int event, int x, int y, int flags, void* param)
{
	gcapp.mouseClick(event, x, y, flags, param);
}

int runExtractAlpha(){

	Mat image = imread("C:\\Users\\hooman\\Desktop\\dressup\\shirtOnWhite.png", IMREAD_COLOR);
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
	gcapp.showImage();

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
			gcapp.showImage();
			break;
		case 'm':
			gcapp.showAlphaMap = true;
			gcapp.showImage();
			break;
			//case 'c':
			//	destroyAllWindows;
			//	namedWindow("contours", 1);
			//	FindDrawContours(gcapp.hsout);
			//	break;
		case 'n':
			gcapp.showAlphaMap = false;
			gcapp.showContours = false;
			int iterCount = gcapp.getIterCount();
			cout << "<" << iterCount << "... ";
			int newIterCount = gcapp.nextIter();
			if (newIterCount > iterCount)
			{
				gcapp.showImage();
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

int main() {

	if (runExtractAlpha() == 0){

		vector<vector<Point> > contours0;
		findContours(*gcapp.hsout, contours0, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

		contours.resize(contours0.size());
		for (size_t k = 0; k < contours0.size(); k++)
			approxPolyDP(Mat(contours0[k]), contours[k], 3, true);

		Mat cnt_img = Mat::zeros(gcapp.hsout->size(), CV_8UC1);
		int levels = 0;

		//The following 2 lines is the check in drawContour that throws an error if _levels doesnt pass the assert
		//	size_t ncontours = contours.size();
		//	CV_Assert(0 <= _levels && _levels < (int)ncontours);

		drawContours(cnt_img, contours, levels/*_levels <= 0 ? 3 : -1*/, Scalar(128, 255, 255),
			3, 16, hierarchy, std::abs(levels));

		imshow("contours", cnt_img);

		waitKey();
	}

	delete gcapp.hsout;
	return 0;
}











//WORKING COPY Orignal contours from sample: Draws faces and extracts the contours. 

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <math.h>
#include <iostream>

using namespace cv;
using namespace std;

static void help1()
{
	cout
		<< "\nThis program illustrates the use of findContours and drawContours\n"
		<< "The original image is put up along with the image of drawn contours\n"
		<< "Usage:\n"
		<< "./contours2\n"
		<< "\nA trackbar is put up which controls the contour level from -3 to 3\n"
		<< endl;
}

const int w = 500;
int levels = 3;

vector<vector<Point> > contours;
vector<Vec4i> hierarchy;

static void on_trackbar(int, void*)
{
	Mat cnt_img = Mat::zeros(w, w, CV_8UC3);
	int _levels = levels - 3;
	drawContours(cnt_img, contours, _levels <= 0 ? 3 : -1, Scalar(128, 255, 255),
		3, 16, hierarchy, std::abs(_levels));

	imshow("contours", cnt_img);
}

int main(int argc, char**)
{
	Mat img = Mat::zeros(w, w, CV_8UC1);
	if (argc > 1)
	{
		help1();
		return -1;
	}
	//Draw 6 faces
	for (int i = 0; i < 6; i++)
	{
		int dx = (i % 2) * 250 - 30;
		int dy = (i / 2) * 150;
		const Scalar white = Scalar(255);
		const Scalar black = Scalar(0);

		if (i == 0)
		{
			for (int j = 0; j <= 10; j++)
			{
				double angle = (j + 5)*CV_PI / 21;
				line(img, Point(cvRound(dx + 100 + j * 10 - 80 * cos(angle)),
					cvRound(dy + 100 - 90 * sin(angle))),
					Point(cvRound(dx + 100 + j * 10 - 30 * cos(angle)),
					cvRound(dy + 100 - 30 * sin(angle))), white, 1, 8, 0);
			}
		}

		ellipse(img, Point(dx + 150, dy + 100), Size(100, 70), 0, 0, 360, white, -1, 8, 0);
		ellipse(img, Point(dx + 115, dy + 70), Size(30, 20), 0, 0, 360, black, -1, 8, 0);
		ellipse(img, Point(dx + 185, dy + 70), Size(30, 20), 0, 0, 360, black, -1, 8, 0);
		ellipse(img, Point(dx + 115, dy + 70), Size(15, 15), 0, 0, 360, white, -1, 8, 0);
		ellipse(img, Point(dx + 185, dy + 70), Size(15, 15), 0, 0, 360, white, -1, 8, 0);
		ellipse(img, Point(dx + 115, dy + 70), Size(5, 5), 0, 0, 360, black, -1, 8, 0);
		ellipse(img, Point(dx + 185, dy + 70), Size(5, 5), 0, 0, 360, black, -1, 8, 0);
		ellipse(img, Point(dx + 150, dy + 100), Size(10, 5), 0, 0, 360, black, -1, 8, 0);
		ellipse(img, Point(dx + 150, dy + 150), Size(40, 10), 0, 0, 360, black, -1, 8, 0);
		ellipse(img, Point(dx + 27, dy + 100), Size(20, 35), 0, 0, 360, white, -1, 8, 0);
		ellipse(img, Point(dx + 273, dy + 100), Size(20, 35), 0, 0, 360, white, -1, 8, 0);
	}
	//show the faces
	namedWindow("image", 1);
	imshow("image", img);
	//Extract the contours so that
	vector<vector<Point> > contours0;
	findContours(img, contours0, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

	contours.resize(contours0.size());
	for (size_t k = 0; k < contours0.size(); k++)
		approxPolyDP(Mat(contours0[k]), contours[k], 3, true);

	namedWindow("contours", 1);
	createTrackbar("levels+3", "contours", &levels, 7, on_trackbar);

	on_trackbar(0, 0);
	waitKey();

	return 0;
}









//WORKING COPY: press m to see the alpha map.
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

using namespace std;
using namespace cv;

static void help()
{
	cout << "\nThis program demonstrates GrabCut segmentation -- select an object in a region\n"
		"and then grabcut will attempt to segment it out.\n"
		"Call:\n"
		"./grabcut <image_name>\n"
		"\nSelect a rectangular area around the object you want to segment\n" <<
		"\nHot keys: \n"
		"\tESC - quit the program\n"
		"\tr - restore the original image\n"
		"\tn - next iteration\n"
		"\n"
		"\tleft mouse button - set rectangle\n"
		"\n"
		"\tCTRL+left mouse button - set GC_BGD pixels\n"
		"\tSHIFT+left mouse button - set GC_FGD pixels\n"
		"\n"
		"\tCTRL+right mouse button - set GC_PR_BGD pixels\n"
		"\tSHIFT+right mouse button - set GC_PR_FGD pixels\n" << endl;
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
	bool flag = false;
	enum{ NOT_SET = 0, IN_PROCESS = 1, SET = 2 };
	static const int radius = 2;
	static const int thickness = -1;

	void reset();
	void setImageAndWinName(const Mat& _image, const string& _winName);
	void showImage() const;
	void mouseClick(int event, int x, int y, int flags, void* param);
	int nextIter();
	int getIterCount() const { return iterCount; }
private:
	void setRectInMask();
	void setLblsInMask(int flags, Point p, bool isPr);

	const string* winName;
	const Mat* image;
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
	flag = false;
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
	if (_image.empty() || _winName.empty())
		return;
	image = &_image;
	winName = &_winName;
	mask.create(image->size(), CV_8UC1);
	reset();
}

void GCApplication::showImage() const
{
	if (image->empty() || winName->empty())
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
	Mat* hsout = new Mat(image->size(), CV_8UC1, *n);
	Scalar* p = new Scalar(255);
	hsout->setTo(*p, binMask);


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

	if (!flag)
		imshow(*winName, res);
	else{
		imshow(*winName, *hsout);
	}

	delete hsout;
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
			showImage();
		}
		if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), false);
			lblsState = SET;
			showImage();
		}
		break;
	case EVENT_RBUTTONUP:
		if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), true);
			prLblsState = SET;
			showImage();
		}
		break;
	case EVENT_MOUSEMOVE:
		if (rectState == IN_PROCESS)
		{
			rect = Rect(Point(rect.x, rect.y), Point(x, y));
			CV_Assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
			showImage();
		}
		else if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), false);
			showImage();
		}
		else if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), true);
			showImage();
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

GCApplication gcapp;

static void on_mouse(int event, int x, int y, int flags, void* param)
{
	gcapp.mouseClick(event, x, y, flags, param);
}

int main(void)
{


	Mat image = imread("C:\\Users\\hooman\\Desktop\\dressup\\shirtOnWhite.png", IMREAD_COLOR);
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
	gcapp.showImage();

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
			gcapp.showImage();
			break;
		case 'm':
			gcapp.flag = true;
			gcapp.showImage();
			break;
		case 'n':
			gcapp.flag = false;
			int iterCount = gcapp.getIterCount();
			cout << "<" << iterCount << "... ";
			int newIterCount = gcapp.nextIter();
			if (newIterCount > iterCount)
			{
				gcapp.showImage();
				cout << iterCount << ">" << endl;
			}
			else
				cout << "rect must be determined>" << endl;
			break;
		}
	}

exit_main:
	destroyWindow(winName);
	destroyWindow("kos");
	return 0;
}















//WORKING COPY. Extracting and manipulating the different channels of an image without opencv
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <iostream>
using namespace cv;
using namespace std;
int main(int argc, const char** argv){

	//"C:\\Users\\hooman\\Documents\\UBC\\Practice\\First\\x64\\Debug\\shirtOnWhite.png"
	//"C:\\Users\\hooman\\Desktop\\alphaImage.png"
	//"C:\\Users\\hooman\\Desktop\\rpga.png"       

	Mat src = imread("C:\\Users\\hooman\\Desktop\\dressup\\alphaImage.png", IMREAD_ANYCOLOR);
	cout << "we get here 0\n";
	cout << src.size() << "\n";
	Mat r(src.rows, src.cols, CV_8UC1);
	Mat g(src.rows, src.cols, CV_8UC1);
	Mat b(src.rows, src.cols, CV_8UC1);
	Mat a(src.rows, src.cols, CV_8UC1);

	for (int i = 0; i<src.rows; i++){
		//	cout << "kos\n";
		for (int j = 0; j< 130/*src.cols*/; j++){ //THe columns are the first entry when you print the size[cols, rows]. at 376 you get an error. for size [500,333].
			//Above line is where the error is thrown. Same with the split method. If you change the implementaion of split. Then it should work.
			Vec4b pixel = src.at<Vec4b>(i, j);
			//	cout << pixel.channels << "\n";
			//	cout << "i: " << i << "     j: " << j << "\n";
			b.at<uchar>(i, j) = pixel[0];
			g.at<uchar>(i, j) = pixel[1];
			r.at<uchar>(i, j) = pixel[2];
			a.at<uchar>(i, j) = pixel[3];

			//cout << int(pixel[3]) << "\n";
		}
		//	cout << "kos1\n";
	}

	cout << "we get here1\n";
	imshow("src", src);
	//	cout << "we get here2\n";
	//	imshow("r", r);
	//	imshow("g", g);
	//	imshow("b", b);
	imshow("a", a);

	src.release();
	r.release();
	g.release();
	b.release();
	a.release();

	waitKey(0);
}











//Working COPY: Splitting an image into its channels (colors) shorter version
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

int main()
{
	Mat src = imread("C:\\Users\\hooman\\Documents\\UBC\\Practice\\First\\x64\\Debug\\shirtOnWhite.png", 1);
	namedWindow("src", 1); imshow("src", src);

	// Split the image into different channels
	vector<Mat> rgbChannels(3);
	split(src, rgbChannels);

	// Show individual channels
	Mat g, fin_img;
	g = Mat::zeros(Size(src.cols, src.rows), CV_8UC1);

	// Showing Red Channel
	// G and B channels are kept as zero matrix for visual perception
	{
		vector<Mat> channels;
		channels.push_back(g);
		channels.push_back(g);
		channels.push_back(rgbChannels[2]);

		/// Merge the three channels
		merge(channels, fin_img);
		namedWindow("R", 1); imshow("R", fin_img);
	}

	// Showing Green Channel
	{
		vector<Mat> channels;
		channels.push_back(g);
		channels.push_back(rgbChannels[1]);
		channels.push_back(g);
		merge(channels, fin_img);
		namedWindow("G", 1); imshow("G", fin_img);
	}

	// Showing Blue Channel
	{
		vector<Mat> channels;
		channels.push_back(rgbChannels[0]);
		channels.push_back(g);
		channels.push_back(g);
		merge(channels, fin_img);
		namedWindow("B", 1); imshow("B", fin_img);
	}

	waitKey(0);
	return 0;
}











//HS WOKING COPY: Splitting an image into its channels (colors) and turning channel blue off.

#include <iostream>
#include "opencv2/opencv.hpp"
#include <stdio.h>    

using namespace cv;
using namespace std;

int main()
{

	Mat image, fin_img;
	image = imread("C:\\Users\\hooman\\Documents\\UBC\\Practice\\First\\x64\\Debug\\shirtOnWhite.png", CV_LOAD_IMAGE_COLOR);   // Read the file

	if (!image.data)                              // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	namedWindow("Display window", CV_WINDOW_AUTOSIZE);// Create a window for display.
	// Show our image inside it.

	// Create Windows
	namedWindow("Red", 1);
	namedWindow("Green", 1);
	namedWindow("Blue", 1);

	// Create Matrices (make sure there is an image in input!)

	Mat channel[3];
	imshow("Original Image", image);


	// The actual splitting.
	split(image, channel);


	channel[0] = Mat::zeros(image.rows, image.cols, CV_8UC1);//Set blue channel to 0

	//Merging red and green channels

	merge(channel, 3, image);
	imshow("R+G", image);
	imwrite("dest.jpg", image);

	waitKey(0);//Wait for a keystroke in the window
	return 0;
}






/*WORKING COPY: GrapCut demo working nicely. Original
*/

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>

using namespace std;
using namespace cv;

static void help()
{
	cout << "\nThis program demonstrates GrabCut segmentation -- select an object in a region\n"
		"and then grabcut will attempt to segment it out.\n"
		"Call:\n"
		"./grabcut <image_name>\n"
		"\nSelect a rectangular area around the object you want to segment\n" <<
		"\nHot keys: \n"
		"\tESC - quit the program\n"
		"\tr - restore the original image\n"
		"\tn - next iteration\n"
		"\n"
		"\tleft mouse button - set rectangle\n"
		"\n"
		"\tCTRL+left mouse button - set GC_BGD pixels\n"
		"\tSHIFT+left mouse button - set GC_FGD pixels\n"
		"\n"
		"\tCTRL+right mouse button - set GC_PR_BGD pixels\n"
		"\tSHIFT+right mouse button - set GC_PR_FGD pixels\n" << endl;
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
	enum{ NOT_SET = 0, IN_PROCESS = 1, SET = 2 };
	static const int radius = 2;
	static const int thickness = -1;

	void reset();
	void setImageAndWinName(const Mat& _image, const string& _winName);
	void showImage() const;
	void mouseClick(int event, int x, int y, int flags, void* param);
	int nextIter();
	int getIterCount() const { return iterCount; }
private:
	void setRectInMask();
	void setLblsInMask(int flags, Point p, bool isPr);

	const string* winName;
	const Mat* image;
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
	if (_image.empty() || _winName.empty())
		return;
	image = &_image;
	winName = &_winName;
	mask.create(image->size(), CV_8UC1);
	reset();
}

void GCApplication::showImage() const
{
	if (image->empty() || winName->empty())
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

	imshow(*winName, res);
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
			showImage();
		}
		if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), false);
			lblsState = SET;
			showImage();
		}
		break;
	case EVENT_RBUTTONUP:
		if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), true);
			prLblsState = SET;
			showImage();
		}
		break;
	case EVENT_MOUSEMOVE:
		if (rectState == IN_PROCESS)
		{
			rect = Rect(Point(rect.x, rect.y), Point(x, y));
			CV_Assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
			showImage();
		}
		else if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), false);
			showImage();
		}
		else if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), true);
			showImage();
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

GCApplication gcapp;

static void on_mouse(int event, int x, int y, int flags, void* param)
{
	gcapp.mouseClick(event, x, y, flags, param);
}

int main(void)
{


	Mat image = imread("C:\\Users\\hooman\\Documents\\UBC\\Practice\\First\\x64\\Debug\\shirtOnWhite.png", 1);
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
	gcapp.showImage();

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
			gcapp.showImage();
			break;
		case 'n':
			int iterCount = gcapp.getIterCount();
			cout << "<" << iterCount << "... ";
			int newIterCount = gcapp.nextIter();
			if (newIterCount > iterCount)
			{
				gcapp.showImage();
				cout << iterCount << ">" << endl;
			}
			else
				cout << "rect must be determined>" << endl;
			break;
		}
	}

exit_main:
	destroyWindow(winName);
	return 0;
}





/*WORKNG COPY: selecting a shape based on pixed intensity thresholding. Original*/
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
using namespace cv;
int threshold_value = 0;
int threshold_type = 3;;
int const max_value = 255;
int const max_type = 4;
int const max_BINARY_value = 255;

Mat src, src_gray, dst;
char* window_name = "Threshold Demo";

char* trackbar_type = "Type: \n 0: Binary \n 1: Binary Inverted \n 2: Truncate \n 3: To Zero \n 4: To Zero Inverted";
char* trackbar_value = "Value";

/// Function headers
void Threshold_Demo(int, void*);

/**
* @function main
*/
int main(int argc, char** argv)
{
	/// Load an image
	src = imread("C:\\Users\\hooman\\Documents\\UBC\\Practice\\First\\x64\\Debug\\shirtOnWhite.png", 1);

	/// Convert the image to Gray
	cvtColor(src, src_gray, CV_RGB2GRAY);

	/// Create a window to display results
	namedWindow(window_name, CV_WINDOW_AUTOSIZE);

	/// Create Trackbar to choose type of Threshold
	createTrackbar(trackbar_type,
		window_name, &threshold_type,
		max_type, Threshold_Demo);

	createTrackbar(trackbar_value,
		window_name, &threshold_value,
		max_value, Threshold_Demo);

	/// Call the function to initialize
	Threshold_Demo(0, 0);

	/// Wait until user finishes program
	while (true)
	{
		int c;
		c = waitKey(20);
		if ((char)c == 27)
		{
			break;
		}
	}

}

void Threshold_Demo(int, void*)
{
	/* 0: Binary
	1: Binary Inverted
	2: Threshold Truncated
	3: Threshold to Zero
	4: Threshold to Zero Inverted
	*/

	threshold(src_gray, dst, threshold_value, max_BINARY_value, threshold_type);

	imshow(window_name, dst);
}








/*WORKING COPY:   Detecting Eyes and faces from video. HS version
***********NOTE: Don't forget that to load the next frame you need to press a key on the keyboard. If you press c it will quit.
*/
#include "opencv2\objdetect\objdetect.hpp"
#include "opencv2\video\video.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

/** Function Headers */
void DetectAndDisplay(Mat& frame);

/** Global variables */
const String face_cascade_name = "C:\\Users\\hooman\\Documents\\UBC\\Practice\\First\\x64\\Debug\\haarcascade_frontalface_alt.xml";
const String eyes_cascade_name = "C:\\Users\\hooman\\Documents\\UBC\\Practice\\First\\x64\\Debug\\haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;

/**
* @function main
*/
int main(void)
{
	Mat image = imread("C:\\Users\\hooman\\Documents\\UBC\\Practice\\First\\x64\\Debug\\me.png", -1);

	if (!image.data) {
		cout << "error opening image\n";
		return -1;
	}

	face_cascade.load(face_cascade_name);
	bool temp = face_cascade.load(face_cascade_name);
	if (!temp) {
		printf("--(!)Error loading face cascade\n");
		return -1;
	}

	temp = eyes_cascade.load(eyes_cascade_name);
	if (!temp){
		printf("--(!)Error loading eyes cascade\n");
		return -1;
	}


	Mat frame;
	CvCapture* capture;
	capture = cvCaptureFromCAM(0);
	if (capture)
	{
		while (true)
		{
			frame = cvQueryFrame(capture);

			//-- 3. Apply the classifier to the frame
			if (!frame.empty())
			{
				namedWindow("My Window", WINDOW_AUTOSIZE);
				imshow("My Window", frame);
				waitKey(0);
				DetectAndDisplay(frame);
			}
			else
			{
				printf(" --(!) No captured frame -- Break!");
			}

			int c = waitKey(10);
			if ((char)c == 'c') { break; }
		}
	}


	//	DetectAndDisplay(image);



	return 0;
}

void DetectAndDisplay(Mat& image) {

	std::vector<Rect> faces;
	Mat frame_gray;
	cvtColor(image, frame_gray, COLOR_BGR2GRAY);
	//imshow(window_name, frame_gray);
	//waitKey(0);
	equalizeHist(frame_gray, frame_gray);
	//imshow(window_name, frame_gray);
	//waitKey(0);
	//-- Detect faces
	face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
	//imshow(window_name, image);
	//waitKey(0);

	int i = 0;
	Point center(faces[i].x + faces[i].width / 2, faces[i].y + faces[i].height / 2);
	ellipse(image, center, Size(faces[i].width / 2, faces[i].height / 2), 0, 0, 360, Scalar(255, 0, 255), 4, 8, 0);
	//imshow(window_name, image);
	//waitKey(0);
	Mat faceROI = frame_gray(faces[i]);
	std::vector<Rect> eyes;
	//-- In each face, detect eyes
	eyes_cascade.detectMultiScale(faceROI, eyes, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
	for (size_t j = 0; j < eyes.size(); j++)
	{
		Point eye_center(faces[i].x + eyes[j].x + eyes[j].width / 2, faces[i].y + eyes[j].y + eyes[j].height / 2);
		int radius = cvRound((eyes[j].width + eyes[j].height)*0.25);
		circle(image, eye_center, radius, Scalar(255, 0, 0), 4, 8, 0);
	}

	//-- Show what you got
	namedWindow("My Window", WINDOW_AUTOSIZE);
	imshow("My Window", image);
	waitKey(0);
}






/*WORKING COPY:   Detecting Eyes and faces from a signle image. My Version.
*/
#include "opencv2\objdetect\objdetect.hpp"
#include "opencv2\video\video.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

/** Function Headers */
void detectAndDisplay(Mat frame);

/** Global variables */
const String face_cascade_name = "C:\\Users\\hooman\\Documents\\UBC\\Practice\\First\\x64\\Debug\\haarcascade_frontalface_alt.xml";
const String eyes_cascade_name = "C:\\Users\\hooman\\Documents\\UBC\\Practice\\First\\x64\\Debug\\haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;

/**
* @function main
*/
int main(void)
{
	Mat image = imread("C:\\Users\\hooman\\Documents\\UBC\\Practice\\First\\x64\\Debug\\me.png", -1);

	if (!image.data) {
		cout << "error opening image\n";
		return -1;
	}

	face_cascade.load(face_cascade_name);
	bool temp = face_cascade.load(face_cascade_name);
	if (!temp) {
		printf("--(!)Error loading face cascade\n");
		return -1;
	}

	temp = eyes_cascade.load(eyes_cascade_name);
	if (!temp){
		printf("--(!)Error loading eyes cascade\n");
		return -1;
	}

	//namedWindow("My Window", WINDOW_AUTOSIZE);  you need this line before every call to imshow if you want to update the same window multiple times.
	//	imshow(window_name, image);
	//	waitKey(0);



	std::vector<Rect> faces;
	Mat frame_gray;
	cvtColor(image, frame_gray, COLOR_BGR2GRAY);
	//imshow(window_name, frame_gray);
	//waitKey(0);
	equalizeHist(frame_gray, frame_gray);
	//imshow(window_name, frame_gray);
	//waitKey(0);
	//-- Detect faces
	face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
	//imshow(window_name, image);
	//waitKey(0);

	int i = 0;
	Point center(faces[i].x + faces[i].width / 2, faces[i].y + faces[i].height / 2);
	ellipse(image, center, Size(faces[i].width / 2, faces[i].height / 2), 0, 0, 360, Scalar(255, 0, 255), 4, 8, 0);
	//imshow(window_name, image);
	//waitKey(0);
	Mat faceROI = frame_gray(faces[i]);
	std::vector<Rect> eyes;
	//-- In each face, detect eyes
	eyes_cascade.detectMultiScale(faceROI, eyes, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));
	for (size_t j = 0; j < eyes.size(); j++)
	{
		Point eye_center(faces[i].x + eyes[j].x + eyes[j].width / 2, faces[i].y + eyes[j].y + eyes[j].height / 2);
		int radius = cvRound((eyes[j].width + eyes[j].height)*0.25);
		circle(image, eye_center, radius, Scalar(255, 0, 0), 4, 8, 0);
	}

	//-- Show what you got
	imshow("My Window", image);
	waitKey(0);

	return 0;
}







/*Tutorial stock version for detecting eyes and faces from vide. Changed only the camera open flag 
***********NOTE: Don't forget that to load the next frame you need to press a key on the keyboard. If you press c it will quit.
*/
#include "opencv2\objdetect\objdetect.hpp"
#include "opencv2\video\video.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "opencv2\imgproc\imgproc.hpp"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

/** Function Headers */
void detectAndDisplay(Mat frame);

/** Global variables */
const String face_cascade_name = "C:\\Users\\hooman\\Documents\\UBC\\Practice\\First\\x64\\Debug\\haarcascade_frontalface_alt.xml";
const String eyes_cascade_name = "C:\\Users\\hooman\\Documents\\UBC\\Practice\\First\\x64\\Debug\\haarcascade_eye_tree_eyeglasses.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
string window_name = "Capture - Face detection";
RNG rng(12345);

/** @function main */
int main(int argc, const char** argv)
{
	CvCapture* capture;
	Mat frame;

	//-- 1. Load the cascades
	if (!face_cascade.load(face_cascade_name)){ printf("--(!)Error loading\n"); return -1; };
	if (!eyes_cascade.load(eyes_cascade_name)){ printf("--(!)Error loading\n"); return -1; };

	//-- 2. Read the video stream

	//HS need to call cvCaptureFromCAM() with 0 as opposed to -1
	capture = cvCaptureFromCAM(0);
	if (capture)
	{
		while (true)
		{
			frame = cvQueryFrame(capture);

			//-- 3. Apply the classifier to the frame
			if (!frame.empty())
			{
				detectAndDisplay(frame);
			}
			else
			{
				printf(" --(!) No captured frame -- Break!"); break;
			}

			int c = waitKey(10);
			if ((char)c == 'c') { break; }
		}
	}
	return 0;
}

/** @function detectAndDisplay */
void detectAndDisplay(Mat frame)
{
	std::vector<Rect> faces;
	Mat frame_gray;

	cvtColor(frame, frame_gray, CV_BGR2GRAY);
	equalizeHist(frame_gray, frame_gray);

	//-- Detect faces
	face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));

	for (size_t i = 0; i < faces.size(); i++)
	{
		Point center(faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5);
		ellipse(frame, center, Size(faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar(255, 0, 255), 4, 8, 0);

		Mat faceROI = frame_gray(faces[i]);
		std::vector<Rect> eyes;

		//-- In each face, detect eyes
		eyes_cascade.detectMultiScale(faceROI, eyes, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, Size(30, 30));

		for (size_t j = 0; j < eyes.size(); j++)
		{
			Point center(faces[i].x + eyes[j].x + eyes[j].width*0.5, faces[i].y + eyes[j].y + eyes[j].height*0.5);
			int radius = cvRound((eyes[j].width + eyes[j].height)*0.25);
			circle(frame, center, radius, Scalar(255, 0, 0), 4, 8, 0);
		}
	}
	//-- Show what you got
	imshow(window_name, frame);
}
