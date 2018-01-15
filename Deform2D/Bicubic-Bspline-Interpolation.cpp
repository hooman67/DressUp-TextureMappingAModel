#include "hsTriClasses.cpp"
#define MAX_IMAGE_DIM 2048

class BcBsI{

	/*backuup
	void bicubic_algo(Mat* origImg, int newWidth, int newHeight, Mat* scaledImage)
	{
		Size inputSize = origImg->size();
		//Final image to be returned 
		uchar * inputData = origImg->data;                 //converting original image to usigned char format         
		uchar * scaledData = scaledImage->data;
		int i, j, m, n;         // variables for loops 
		int x, y;               // relative position in old image   
		float dx, dy;           // delta_x and delta_y   
		float tx, ty;           // scaling ratio (old/new)   
		float Bmdx;            // Bspline m-dx 
		float Bndy;            // Bspline dy-n   


		tx = inputSize.width / (float)newWidth;
		ty = inputSize.height / (float)newHeight;

		for (i = 0; i<scaledImage->rows; i++)
		{
			for (j = 0; j<scaledImage->cols; j++)
			{
				x = (int)(tx*j); //j in the orig image. = width = cols
				y = (int)(ty*i); // i in the orig image = height = rows

				dx = tx*j - x; //derivs of x and y
				dy = ty*i - y;

				// these two loops calculate the summation of F(i+m,j+n)*R(m-dx)*R(n-dy) where m and n are varying from -1 to 2. R is the Bspline function      
				for (m = -1; m <= 2; m++)
				{
					Bmdx = BSpline(m - dx);
					for (n = -1; n <= 2; n++)
					{
						Bndy = BSpline(dy - n);
						scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 0] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 0)] * Bmdx * Bndy;
						scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 1] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 1)] * Bmdx * Bndy;
						scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 2] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 2)] * Bmdx * Bndy;
					}
				}
			}

		}
	}*/

//everything works
	/*
	void bicubic_algo_singleSquare_onlyInsideTriangle(Mat* origImg, Mat* outImage, hsTriangle& inTri, hsTriangle& outTri)
	{

		hsTriangle tempTri(outTri.triangleId, outTri.a, outTri.b, outTri.c);
		Mat* scaledImage = new Mat(outImage->rows, outImage->cols, CV_8UC3);

		float in_tlX = MIN(MIN(inTri.a.position.x, inTri.b.position.x), inTri.c.position.x);
		float in_tlY = MIN(MIN(inTri.a.position.y, inTri.b.position.y), inTri.c.position.y);
		float in_brX = MAX(MAX(inTri.a.position.x, inTri.b.position.x), inTri.c.position.x);
		float in_brY = MAX(MAX(inTri.a.position.y, inTri.b.position.y), inTri.c.position.y);

		float out_tlX = MIN(MIN(tempTri.a.position.x, tempTri.b.position.x), tempTri.c.position.x);
		float out_tlY = MIN(MIN(tempTri.a.position.y, tempTri.b.position.y), tempTri.c.position.y);
		float out_brX = MAX(MAX(tempTri.a.position.x, tempTri.b.position.x), tempTri.c.position.x);
		float out_brY = MAX(MAX(tempTri.a.position.y, tempTri.b.position.y), tempTri.c.position.y);

		in_tlX = floor(in_tlX);
		in_tlY = floor(in_tlY);
		in_brX = ceil(in_brX);
		in_brY = ceil(in_brY);

		out_tlX = floor(out_tlX);
		out_tlY = floor(out_tlY);
		out_brX = ceil(out_brX);
		out_brY = ceil(out_brY);

		float out_widthOffset = out_tlX;
		float out_heightOffset = out_tlY;

		//shift the square
		out_tlX = 0;
		out_tlY = 0;
		out_brX -= out_widthOffset;
		out_brY -= out_heightOffset;

		//shift the triangle
		tempTri.shiftForward(out_widthOffset*-1, out_heightOffset*-1);

		float inputSquareWidth = in_brX - in_tlX;
		float inputSquareHeight = in_brY - in_tlY;

		float outputSquareWidth = out_brX - out_tlX;
		float outputSquareHeight = out_brY - out_tlY;


		//orig woks if square is at 0,0
		//	float widthOffset = in_tlX;  //cols = j 
		//	float heightOffset = in_tlY; //rows = i

		float in_widthOffset = in_tlX;  //cols = j 
		float in_heightOffset = in_tlY; //rows = i


		//This didnt work
		//	float widthOffset = in_tlX - out_tlX;  //cols = j 
		//	float heightOffset = in_tlY - out_tlY; //rows = i

		Point2f rotationOffset = tempTri.getRotation(inTri);

		uchar * inputData = origImg->data;                 //converting original image to usigned char format         
		uchar * scaledData = scaledImage->data;
		int i, j, m, n;         // variables for loops 
		int x, y;               // relative position in old image   
		float dx, dy;           // delta_x and delta_y   
		float tx, ty;           // scaling ratio (old/new)   
		float Bmdx;            // Bspline m-dx 
		float Bndy;            // Bspline dy-n   


		tx = inputSquareWidth / outputSquareWidth;
		ty = inputSquareHeight / outputSquareHeight;

		for (i = 0; i< MIN(scaledImage->rows, outputSquareHeight); i++)
		{
			for (j = 0; j< MIN(scaledImage->cols, outputSquareWidth); j++) {

				hsVertexBary temp = hsTriangle::convToBary(j, i, tempTri);
				if (hsTriangle::isInTriangle(temp)){
					x = (int)(tx*j); //j in the orig image. = width = cols
					y = (int)(ty*i); // i in the orig image = height = rows

					dx = tx*j - x; //derivs of x and y
					dy = ty*i - y;

					//	x = x*rotationOffset.x + widthOffset;
					//	y = y*rotationOffset.y + heightOffset;
					x += in_widthOffset;
					y += in_heightOffset;

					// these two loops calculate the summation of F(i+m,j+n)*R(m-dx)*R(n-dy) where m and n are varying from -1 to 2. R is the Bspline function      
					for (m = -1; m <= 2; m++)
					{
						Bmdx = BSpline(m - dx);
						for (n = -1; n <= 2; n++)
						{
							Bndy = BSpline(dy - n);
							scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 0] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 0)] * Bmdx * Bndy;
							scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 1] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 1)] * Bmdx * Bndy;
							scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 2] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 2)] * Bmdx * Bndy;
						}
					}
				}
			}
		}

		for (i = 0; i < MIN(scaledImage->rows, outputSquareHeight); i++)
		{
			for (j = 0; j < MIN(scaledImage->cols, outputSquareWidth); j++) {
				hsVertexBary temp = hsTriangle::convToBary(j, i, tempTri);
				if (hsTriangle::isInTriangle(temp)){

					Point2i p(j, i);
					cv::Point2i tp = tempTri.getTransformed(outTri, p);

					Vec3b color = scaledImage->at<Vec3b>(p);
					outImage->at<Vec3b>(tp) = color;
				}
			}
		}
	}*/



    bool IsDebugMode = false;

	float BSpline(float x)
	{
		float f = x;
		if (f < 0.0)
			f = -f;

		if (f >= 0.0 && f <= 1.0)
			return (2.0 / 3.0) + (0.5) * (f* f * f) - (f*f);
		else if (f > 1.0 && f <= 2.0)
			return 1.0 / 6.0 * pow((2.0 - f), 3.0);
		return 1.0;
	}

	IplImage * createImage(int w, int h)
	{
		IplImage * img = NULL;
		img = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);
		return img;
	}

	int pos(int num)
	{
		return num > 0 ? num : 0;
	}



public:
	
	/*
	void bicubic_algo(Mat* origImg, int newWidth, int newHeight, Mat* scaledImage)
	{
		Size inputSize = origImg->size();
      //Final image to be returned 
		uchar * inputData = origImg->data;                 //converting original image to usigned char format         
		uchar * scaledData = scaledImage->data;
		int i, j, m, n;         // variables for loops 
		int x, y;               // relative position in old image   
		float dx, dy;           // delta_x and delta_y   
		float tx, ty;           // scaling ratio (old/new)   
		float Bmdx;            // Bspline m-dx 
		float Bndy;            // Bspline dy-n   


		tx = float(origImg->cols) / float(newWidth);
		ty = float(origImg->rows) / float(newHeight);

		for (i = 0; i<scaledImage->rows; i++)
		{ 
			for (j = 0; j<scaledImage->cols; j++)
			{
				x = (int)(tx*j); //j in the orig image. = width = cols
				y = (int)(ty*i); // i in the orig image = height = rows

				dx = tx*j - x; //derivs of x and y
				dy = ty*i - y;

				// these two loops calculate the summation of F(i+m,j+n)*R(m-dx)*R(n-dy) where m and n are varying from -1 to 2. R is the Bspline function      
				for (m = -1; m <= 2; m++)
				{
					Bmdx = BSpline(m - dx);
					for (n = -1; n <= 2; n++)
					{
						Bndy = BSpline(dy - n);
						scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 0] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 0)] * Bmdx * Bndy;
						scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 1] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 1)] * Bmdx * Bndy;
						scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 2] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 2)] * Bmdx * Bndy;
					}
				}
			}
				
		}
	} */



	void bicubic_algo_singleSquare(Mat* origImg, int newWidth, int newHeight, Point2i tl, Point2i br, Mat* scaledImage)
	{
		float inputSquareWidth = br.x - tl.x;
		float inputSquareHeight = br.y - tl.y;

		float widthOffset = tl.x;  //cols = j 
		float heightOffset = tl.y; //rows = i

		uchar * inputData = origImg->data;                 //converting original image to usigned char format         
		uchar * scaledData = scaledImage->data;
		int i, j, m, n;         // variables for loops 
		int x, y;               // relative position in old image   
		float dx, dy;           // delta_x and delta_y   
		float tx, ty;           // scaling ratio (old/new)   
		float Bmdx;            // Bspline m-dx 
		float Bndy;            // Bspline dy-n   


		tx = float(inputSquareWidth) / float(newWidth);
		ty = float(inputSquareHeight) / float(newHeight);

		for (i = 0; i<scaledImage->rows; i++)
		{
			for (j = 0; j<scaledImage->cols; j++)
			{
				x = (int)(tx*j);// +widthOffset; //j in the orig image. = width = cols
				y = (int)(ty*i);// +heightOffset; // i in the orig image = height = rows

				dx = tx*j - x; //derivs of x and y
				dy = ty*i - y;

				x += widthOffset;
				y += heightOffset;

				// these two loops calculate the summation of F(i+m,j+n)*R(m-dx)*R(n-dy) where m and n are varying from -1 to 2. R is the Bspline function      
				for (m = -1; m <= 2; m++)
				{
					Bmdx = BSpline(m - dx);
					for (n = -1; n <= 2; n++)
					{
						Bndy = BSpline(dy - n);
						scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 0] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 0)] * Bmdx * Bndy;
						scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 1] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 1)] * Bmdx * Bndy;
						scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 2] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 2)] * Bmdx * Bndy;
					}
				}
			}
		}
	}
	
	void bicubic_algo_singleSquare_onlyInsideTriangle(Mat* origImg, int newWidth, int newHeight, Point2i tl, Point2i br, Mat* scaledImage, hsTriangle& inTri, hsTriangle& outTri)
	{
		float inputSquareWidth = br.x - tl.x;
		float inputSquareHeight = br.y - tl.y;

		float widthOffset = tl.x;  //cols = j 
		float heightOffset = tl.y; //rows = i

		uchar * inputData = origImg->data;                 //converting original image to usigned char format         
		uchar * scaledData = scaledImage->data;
		int i, j, m, n;         // variables for loops 
		int x, y;               // relative position in old image   
		float dx, dy;           // delta_x and delta_y   
		float tx, ty;           // scaling ratio (old/new)   
		float Bmdx;            // Bspline m-dx 
		float Bndy;            // Bspline dy-n   


		tx = float(inputSquareWidth) / float(newWidth);
		ty = float(inputSquareHeight) / float(newHeight);

		for (i = 0; i<scaledImage->rows; i++)
		{
			for (j = 0; j<scaledImage->cols; j++) {
					
				hsVertexBary temp = hsTriangle::convToBary(j, i, outTri);
				if (hsTriangle::isInTriangle(temp)){
					x = (int)(tx*j); //j in the orig image. = width = cols
					y = (int)(ty*i); // i in the orig image = height = rows

		//			hsVertexBary temp1 = hsTriangle::convToBary(y + heightOffset, x + widthOffset, inTri);
		//			if (hsTriangle::isInTriangle(temp1)){

						dx = tx*j - x; //derivs of x and y
						dy = ty*i - y;

						x += widthOffset;
						y += heightOffset;

						// these two loops calculate the summation of F(i+m,j+n)*R(m-dx)*R(n-dy) where m and n are varying from -1 to 2. R is the Bspline function      
						for (m = -1; m <= 2; m++)
						{
							Bmdx = BSpline(m - dx);
							for (n = -1; n <= 2; n++)
							{
								Bndy = BSpline(dy - n);
								scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 0] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 0)] * Bmdx * Bndy;
								scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 1] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 1)] * Bmdx * Bndy;
								scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 2] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 2)] * Bmdx * Bndy;
							}
						}
				//	}
				}
			}
		}
	}
	
	void bicubic_algo_singleSquare_onlyInsideTriangle(Mat* origImg, Mat* outImage, hsTriangle& inTri, hsTriangle& outTri)
	{
	
		hsTriangle tempTri(outTri.triangleId, outTri.a, outTri.b, outTri.c);
		Scalar AScalar(char(0), char(0), char(0));
		Mat* scaledImage = new Mat(outImage->rows,outImage->cols,CV_8UC3, AScalar);

		float in_tlX = MIN(MIN(inTri.a.position.x, inTri.b.position.x), inTri.c.position.x);
		float in_tlY = MIN(MIN(inTri.a.position.y, inTri.b.position.y), inTri.c.position.y);
		float in_brX = MAX(MAX(inTri.a.position.x, inTri.b.position.x), inTri.c.position.x);
		float in_brY = MAX(MAX(inTri.a.position.y, inTri.b.position.y), inTri.c.position.y);

		float out_tlX = MIN(MIN(tempTri.a.position.x, tempTri.b.position.x), tempTri.c.position.x);
		float out_tlY = MIN(MIN(tempTri.a.position.y, tempTri.b.position.y), tempTri.c.position.y);
		float out_brX = MAX(MAX(tempTri.a.position.x, tempTri.b.position.x), tempTri.c.position.x);
		float out_brY = MAX(MAX(tempTri.a.position.y, tempTri.b.position.y), tempTri.c.position.y);

		in_tlX = floor(in_tlX);
		in_tlY = floor(in_tlY);
		in_brX = ceil(in_brX);
		in_brY = ceil(in_brY);

		out_tlX = floor(out_tlX);
		out_tlY = floor(out_tlY);
		out_brX = ceil(out_brX);
		out_brY = ceil(out_brY);

		float out_widthOffset = out_tlX;
		float out_heightOffset = out_tlY;

		//shift the square
		out_tlX = 0;
		out_tlY = 0;
		out_brX -= out_widthOffset;
		out_brY -= out_heightOffset;

		//shift the triangle
		tempTri.shiftForward(out_widthOffset*-1, out_heightOffset*-1);
		
		float inputSquareWidth = in_brX - in_tlX;
		float inputSquareHeight = in_brY - in_tlY;

		float outputSquareWidth = out_brX - out_tlX;
		float outputSquareHeight = out_brY - out_tlY;


		//orig woks if square is at 0,0
	//	float widthOffset = in_tlX;  //cols = j 
	//	float heightOffset = in_tlY; //rows = i

			float in_widthOffset = in_tlX;  //cols = j 
			float in_heightOffset = in_tlY; //rows = i


		//This didnt work
	//	float widthOffset = in_tlX - out_tlX;  //cols = j 
	//	float heightOffset = in_tlY - out_tlY; //rows = i

			Point2f rotationOffset = tempTri.getRotation(inTri);

		uchar * inputData = origImg->data;                 //converting original image to usigned char format         
		uchar * scaledData = scaledImage->data;
		int i, j, m, n;         // variables for loops 
		int x, y;               // relative position in old image   
		float dx, dy;           // delta_x and delta_y   
		float tx, ty;           // scaling ratio (old/new)   
		float Bmdx;            // Bspline m-dx 
		float Bndy;            // Bspline dy-n   


		tx = inputSquareWidth / outputSquareWidth;
		ty = inputSquareHeight / outputSquareHeight;

		for (i = 0; i< MIN(scaledImage->rows, outputSquareHeight); i++)
		{
			for (j = 0; j< MIN(scaledImage->cols, outputSquareWidth); j++) {

				hsVertexBary temp = hsTriangle::convToBary(j, i, tempTri);
				if (hsTriangle::isInTriangle(temp)){
					x = (int)(tx*j); //j in the orig image. = width = cols
					y = (int)(ty*i); // i in the orig image = height = rows

					dx = tx*j - x; //derivs of x and y
					dy = ty*i - y;

				//	x = x*rotationOffset.x + widthOffset;
				//	y = y*rotationOffset.y + heightOffset;
					x += in_widthOffset;
					y += in_heightOffset;

					// these two loops calculate the summation of F(i+m,j+n)*R(m-dx)*R(n-dy) where m and n are varying from -1 to 2. R is the Bspline function      
					for (m = -1; m <= 2; m++)
					{
						Bmdx = BSpline(m - dx);
						for (n = -1; n <= 2; n++)
						{
							Bndy = BSpline(dy - n);
							scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 0] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 0)] * Bmdx * Bndy;
							scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 1] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 1)] * Bmdx * Bndy;
							scaledData[i*scaledImage->step[0] + j*scaledImage->step[1] + 2] += inputData[pos((y + n)*origImg->step[0] + (x + m)*origImg->step[1] + 2)] * Bmdx * Bndy;
						}
					}
				}
			}
		}

		for (i = 0; i < MIN(scaledImage->rows, outputSquareHeight); i++)
		{
			for (j = 0; j < MIN(scaledImage->cols, outputSquareWidth); j++) {
				hsVertexBary temp = hsTriangle::convToBary(j, i, tempTri);
				if (hsTriangle::isInTriangle(temp)){

					Point2i p(j, i);
					cv::Point2i tp = tempTri.getTransformed(outTri, p);

					Vec3b color = scaledImage->at<Vec3b>(p);
					if (!(color[0] == char(0) && color[1] == char(0) && color[2] == char(0)))
						outImage->at<Vec3b>(tp) = color;
				}
			}
		}
	}
};

