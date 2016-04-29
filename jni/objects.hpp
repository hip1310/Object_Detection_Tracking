#include <iostream>
#include <string>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>

using namespace std;
using namespace cv;

class Object{
	public:
	   struct features{
	        Point2f center;
	        Rect rect;
	        int frame;
	        int direction;
	        double velocity;
	    };

     vector<features> objFeatures;
     int label;

     Object(Point2f c, Rect r, int frm, int l);
     void update(Point2f c, Rect r, int frm);
     double match(Point2f c, Rect r, int frm);
     bool sameDirectionMatch(Point2f c, int frm);
     bool oppDirectionMatch(Point2f c, int frm);
     bool aroundObjectMatch(Point2f c);
     bool checkStatus(int frm);
     bool findProjection(Mat &hue, Mat &mask, Rect currRect);
};
