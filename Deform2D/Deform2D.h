#include <algorithm>
#include <unordered_map>
#include "RigidMeshDeformer2D.h"
#include "LinearAlgebra.h"
#include "hsSkeleton.cpp"
#include "Bicubic-Bspline-Interpolation.cpp"

//hs variables
String globalImageName;
Mat* PIShirtImage;
Mat* PIBodyImage;
Mat* hsDeformerImage;
Vector<pair<unsigned int, int>> PISelectedControlPoints;

int GlutwinID;

bool allowFreeGlutToRun = true;
bool askForDestWhenSelectingControlPoints = false;

//These are used by the skeletonSelector
Mat* skeletonOutput = NULL;
Mat* skeletonInputForTexture = NULL;


Mat contours_img;
vector<vector<cv::Point> > contours0;
vector<vector<cv::Point> > contours;
vector<vector<cv::Point> > shirtContour;
vector<vector<cv::Point> > bodyContour;
vector<Vec4i> hierarchy;

map<int, hsVertex> vertices;
map<int, hsTriangle> triangles;
map<int, hsVertex> deformedVertices;
map<int, hsTriangle> deformedTriangles;
//KEY == arrayIndex = index of the bounday point in shirtContours[0]
//VALUE == index of the vertice in m_vInitialVertices (glut)
Vector<int> handsBoundaryMeshVertices;
Vector<int> bodyBoundaryMeshVertices;
Vector<int> mahdiMeshVertices;
Vector<int> boundaryMeshVertices;
unordered_map<int,int> mahdiMap;

GCApplication gcapp;
hsSkeleton skeletonObj;
BcBsI bcbsi;

//hs predecls
void loadGlutMesh();
void loadGlutDeformedMesh();
int createNodeFile(vector<vector<cv::Point> > contours);
Mat* k = NULL;
void hsDrawMesh(Mat*& output = k);
void hsLoadTrianglePoints(Mat* output);
void drawTriagnle(hsTriangle& triangle, bool drawIngreen = false, Mat*& output = skeletonOutput);
void hsGluthelp();
void PILoadContourPointsFronFileIntoDeformObj();
void saveMeshFile();
void PILoadVerticesAndTrianglesFromDeformObj(bool deformed = false);
void PISelectControlPoints();
void PIMoveControlPoints();
void PIShiftImage();
void PImakeSmaller();
void PISelectAllVerticesAsControlPoints();
void performBothDeformations();
void selectHandsBoundaryVertices();
void selectBodyBoundaryVertices();
void selectAllBoundaryVertices();
void moveSecondDeformControlPoints(int cont);
int hsfindClosestBodyBoundaryPointAll(float x, float y, bool hands);
void findBoundaryVertices();
void loadContours();
void sampleImage();
void sampleContour();


void hsColorTri3();
void hsColorTrianglesOnMesh(Mat* inputTextureFile, Mat* outputImage);
void hsColorTrianglesOnMeshBC(Mat* inputTextureFile, Mat* outputImage);

void hsDeformerOnMouse(int event, int x, int y, int flags, void*);
void hsDeformerShowImage(bool drawMesh = true);
const string hsDeformerWinName = "hsDeformer";




//Deform stuff
rmsmesh::TriangleMesh m_mesh;
rmsmesh::TriangleMesh m_deformedMesh;

float m_bounds[6];
Wml::Vector2f m_vTranslate;
float m_fScale;

rmsmesh::RigidMeshDeformer2D m_deformer;
bool m_bConstraintsValid;

std::set<unsigned int> m_vSelected;
unsigned int m_nSelected;

int m_nViewport[4];

// predecls
void InitializeDeformedMesh();
void UpdateDeformedMesh();
void InvalidateConstraints();
void ValidateConstraints();
//unsigned int FindHitVertex(float nX, float nY);