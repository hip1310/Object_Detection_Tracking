#include <jni.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/video/tracking.hpp>

#include "objects.hpp"

#include <android/log.h>

#include <iostream>
#include <string>
#include <vector>

using namespace std;
using namespace cv;

class Detector{

	public:

	static int frame_count, obj_count;
	static cv::Ptr<BackgroundSubtractorMOG2> pMOG2;
	static vector<Object> activeObjs;

	static void load(){
		int history = 100;
		float thresh = 30;
		//float thresh = 16;
		bool detectShadows = true;
		frame_count = obj_count = 0;
		pMOG2 = createBackgroundSubtractorMOG2(history, thresh, detectShadows);
	    pMOG2->setNMixtures(3);
	    pMOG2->setShadowValue(127);
	    pMOG2->setShadowThreshold(20);
	}

	void detect(Mat& img){
		    frame_count++;

		    Mat tempImg = img.clone();
			Mat background, foreground, mGray;

			cvtColor(img, mGray, CV_RGBA2GRAY);
			//GaussianBlur(mGray, mGray, cv::Size(21,21), 0, 0);

			//bgKNN->apply(mGray, foreground);
			pMOG2->apply(mGray, foreground);
			//pMOG2->getBackgroundImage(background);

			dilate(foreground, foreground, Mat(), Point(-1,-1), 5);
			erode(foreground, foreground, Mat(), Point(-1,-1), 5);

			/// Find contours
    		 std::vector<vector<Point> > contours;
			 findContours( foreground, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0,0));

			// drawContours(img,contours,-1,Scalar(255,0,0),2);
			 std::vector<Rect> boundRect;
			 std::vector<vector<Point> > contours1;

			 for(int i=0; i < contours.size(); i++){
					 double a = fabs(contourArea(contours[i]));

					 __android_log_print(ANDROID_LOG_DEBUG, "Motion_Detection", "Area : %f", a);
					 if(a > 22500 && a < 150000){
						 contours1.push_back(contours[i]);
					 }
			 }

			 //Get the moments
			 std::vector<Moments> mu(contours1.size());
			 for(int i=0; i < contours1.size(); i++){
				 mu[i] = moments(contours1[i], false);
			 }

			 //Get the mass centers
			 std::vector<Point2f> mc(contours1.size());
			 for( int i = 0; i < contours1.size(); i++ )
			      { mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); }

			 /// Draw contours
			 for( int i = 0; i< contours1.size(); i++ ){
					cv::Rect rect = boundingRect(contours1[i]);
					boundRect.push_back(rect);
			 }

			 if(frame_count > 300){
				 if(activeObjs.size() == 0){
					 for(int i=0; i < mc.size(); i++){
						 obj_count++;
						 Object obj(mc[i], boundRect[i], frame_count, obj_count);
						 activeObjs.push_back(obj);
						 circle( img, mc[i], 4, Scalar( 0, 0, 255), -1, 8, 0);
						 rectangle(img, boundRect[i], Scalar(255,0,0), 2);
						 char text[5];
						 sprintf(text, "%d", (int)obj_count);
						 cv::putText(img, text, Point(boundRect[i].x + 20, boundRect[i].y + 200), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,255,255), 4);
					 }
				 }
				 else{
					 vector<int> currObjMatch(mc.size());

					 for(int i=0; i < activeObjs.size(); i++){
						 bool remove = false;
						 remove = activeObjs[i].checkStatus(frame_count);

						 if(remove){
							 activeObjs.erase(activeObjs.begin() + i);
						 }
					 }

					 for(int i=0; i < mc.size(); i++){
						 bool match = false;
						 int matchIndex = -1;

						 for(int j=0; j < activeObjs.size(); j++){
							 match = activeObjs[j].sameDirectionMatch(mc[i], frame_count);

							 if(match){
								 matchIndex = j;
								 break;
							 }
						 }

						 if(!match){
							 for(int j=0; j < activeObjs.size(); j++){
								 match = activeObjs[j].aroundObjectMatch(mc[i]);

								 if(match){
									 matchIndex = j;
									 break;
								 }
							 }
						 }

						 if(!match){
							 for(int j=0; j < activeObjs.size(); j++){
								 match = activeObjs[j].oppDirectionMatch(mc[i], frame_count);

								 if(match){
									 matchIndex = j;
									 break;
								 }
							 }
						 }

						 currObjMatch[i] = matchIndex;
					 }

					 for(int i=0; i < currObjMatch.size(); i++){
						 if(currObjMatch[i] == -1){
							 if(mc[i].x > (640 - 150) || mc[i].x < 150){
								 obj_count++;
								 Object obj(mc[i], boundRect[i], frame_count, obj_count);
								 activeObjs.push_back(obj);
								 circle( img, mc[i], 4, Scalar( 0, 0, 255), -1, 8, 0);
								 rectangle(img, boundRect[i], Scalar(255,0,0), 2);
								 char text[5];
								 sprintf(text, "%d", (int)obj_count);
								 cv::putText(img, text, Point(boundRect[i].x + 20, boundRect[i].y + 20), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,255,255), 4);
							 }
							 else{
								 int vmin = 10, vmax = 256, smin = 30;
								 Mat hsv, hue, mask;

							       // Convert to HSV colorspace
							        cvtColor(tempImg, hsv, COLOR_BGR2HSV);

						            // Check for all the values in 'hsvimage' that are within the specified range
						            // and put the result in 'mask'
						            inRange(hsv, Scalar(0, smin, vmin), Scalar(180, 256, vmax), mask);

						            // Mix the specified channels
						            int channels[] = {0, 0};
						            hue.create(hsv.size(), hsv.depth());
						            mixChannels(&hsv, 1, &hue, 1, channels, 1);

						            bool match = false;
						            for(int j=0; j < activeObjs.size(); j++){
						            	match = activeObjs[i].findProjection(hue, mask, boundRect[j]);
							            if(match){
							            	currObjMatch[i] = j;
							            	break;
							            }
						            }
							 }
						 }
					 }

					 for(int i=0; i < currObjMatch.size(); i++){
						 if(currObjMatch[i] != -1){
							 activeObjs[currObjMatch[i]].update(mc[i], boundRect[i], frame_count);
							 circle( img, mc[i], 4, Scalar( 0, 0, 255), -1, 8, 0);
							 rectangle(img, boundRect[i], Scalar(255,0,0), 2);
							 char text[5];
							 sprintf(text, "%d", (int)activeObjs[currObjMatch[i]].label);
							 cv::putText(img, text, Point(boundRect[i].x + 20, boundRect[i].y + 20), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,255,255), 4);
						 }
					 }
				 }
			 }
			 char text[30];
			 sprintf(text, "Frame : %d", (int)frame_count);
			 cv::putText(img, text, Point(10,25), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,0,255), 2);
			 //cv::line( img, Point(320, 0), Point(320, 480), Scalar(0, 255, 0),  2, 8);
			 //sprintf(text, "In: %d Out: %d", (int)in, (int)out);
			 //cv::putText(img, text, Point(340, 25), FONT_HERSHEY_SIMPLEX, 1, Scalar(0,0,255), 2);
	}
};

int Detector::frame_count = 0;
int Detector::obj_count = 0;
//int Detector::in = 0;
//int Detector::out = 0;
vector<Object> Detector::activeObjs;
Ptr<BackgroundSubtractorMOG2> Detector::pMOG2;

extern "C"{
JNIEXPORT void JNICALL Java_com_example_trackmf_MainActivity_load(JNIEnv* jenv, jobject){
	Detector::load();
}

JNIEXPORT void JNICALL Java_com_example_trackmf_MainActivity_detectMotion(JNIEnv* jenv, jobject, jlong addrRgba){
	Mat& mRgba = *(Mat*)addrRgba;

	Detector *dObj = new Detector();
	dObj->detect(mRgba);

	delete(dObj);
}

}
