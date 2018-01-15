#include "meshwarp.h"
#include <iostream>
#include <opencv2\highgui\highgui.hpp>


using namespace cv;
using namespace std;


void hsMeshFileCreation(){

	
	//Writing a mesh file.
	FILE *file = fopen("C:\\Users\\hooman\\Documents\\UBC\\Practice\\2dMorphing-master\\x64\\Release\\m2.ele", "w");
	int m[] = { 3, 2 };
	std::fwrite(m, 2*sizeof(int), 2, file);

	//Didnet work
//	float kk[] = { 40.0, 80.0, 120.0, 40.0, 80.0, 120.0};
	float kk[] = { 40, 80, 120, 50, 80, 110 };

//	float kk[] = { 21, 2, 13, 54, 11, 2 };
//	fread(I->ch[0], sz[0] * sz[1], 2 * sizeof(float), fp);
	std::fwrite(kk, 6, 2 * sizeof(float), file); 


	/*
	//Test to see if you can correctly read a mesh file
	char *file = "C:\\Users\\hooman\\Documents\\UBC\\Practice\\2dMorphing-master\\x64\\Release\\m1.ele";
	FILE *fp;
	if ((fp = fopen(file, "r")) == NULL) {
	cout << "readImage: Can't open file " << file << endl;
	}
	int  sz[2];
	fread(sz, sizeof(int), 2, fp);
	cout << sz[0] << "   " << sz[1] << "\n";
	float* ch = (float *)malloc(2 * 10*11*sizeof(float));
	fread(ch, sz[0] * sz[1], 2 * sizeof(float), fp);
	cout << (int)ch[0] << "  " << (int)ch[1] << "\n";*/
	

	/*
	imageP M1 = readImage("C:\\Users\\hooman\\Documents\\UBC\\Practice\\2dMorphing-master\\x64\\Release\\m1.ele", MESH);
	float* k = (float*)M1->ch[0];
	for (int i = 0; i < M1->height*M1->width; i++)
		cout << k[i] << "\n"; */

	
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
 * main: 
 * 
 * Main function to collect input image, source, and target meshes. 
 * Pass them to meshWarp() and save result in output file. 
 */ 
void main() 
{ 
	
  Mat I1;
  imageP M1, M2;

  I1 = imread("C:\\Users\\hooman\\Documents\\UBC\\Practice\\2dMorphing-master\\x64\\Release\\shirtOnWhite.png", IMREAD_GRAYSCALE);
  Mat I2(I1.rows,I1.cols,CV_8UC1);
  M1 = readImage("C:\\Users\\hooman\\Documents\\UBC\\Practice\\2dMorphing-master\\x64\\Release\\m1.ele", MESH); // source mesh 
  M2 = readImage("C:\\Users\\hooman\\Documents\\UBC\\Practice\\2dMorphing-master\\x64\\Release\\m2.ele", MESH); // target mesh 

  meshWarp(I1, M1, M2, I2);

  imwrite("C:\\Users\\hooman\\Documents\\UBC\\Practice\\2dMorphing-master\\x64\\Release\\warpOutput.png", I2); 


} 
 
