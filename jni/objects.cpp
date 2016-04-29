#include "objects.hpp"

using namespace std;
using namespace cv;

Object::Object(Point2f c, Rect r, int frm, int l){
	 features f;
	 f.center = c;
	 f.rect = r;
	 f.frame = frm;
	 objFeatures.push_back(f);

	 label = l;
}

void Object::update(Point2f c, Rect r, int frm){
	 features f;
	 f.center = c;
	 f.rect = r;
	 f.frame = frm;

	 if(c.x - ((objFeatures[objFeatures.size() - 1].center).x) > 0){
		 f.direction = 0;
	 }
	 else{
		 f.direction = 1;
	 }

	    double totalDistance = 0;
	    int totalFrames = 0;

		for(int i=0; i<objFeatures.size(); i++){
			double distance;
			int tempfrm;
			if(i == objFeatures.size()-1){
				distance = norm(objFeatures[i].center - c);
				tempfrm = abs(objFeatures[i].frame - frm);
				totalDistance = totalDistance + distance;
				totalFrames = totalFrames + tempfrm;
			}
			else{
				distance = norm(objFeatures[i].center - objFeatures[i+1].center);
				tempfrm = abs(objFeatures[i].frame - objFeatures[i+1].frame);
				totalDistance = totalDistance + distance;
				totalFrames = totalFrames + tempfrm;
			}

		}

		double vel = totalDistance/totalFrames;
		f.velocity = vel;
	    objFeatures.push_back(f);
}

double Object::match(Point2f c, Rect r, int frm){
	int lastFeature = objFeatures.size() - 1;
	double projection = objFeatures[lastFeature].velocity * (abs(frm - objFeatures[lastFeature].frame));
	double matchV = abs(projection - (norm(objFeatures[lastFeature].center - c)));

	double matchA = abs(r.area() - objFeatures[lastFeature].rect.area());
	return ((0.7 * matchV) + (0.3 * matchA));
}

bool Object::sameDirectionMatch(Point2f c, int frm){
	int lastFeature = objFeatures.size() - 1;
	int dir = objFeatures[lastFeature].direction;
	double vel = objFeatures[lastFeature].velocity;
	Point2f last = objFeatures[lastFeature].center;
	double projection = vel * (abs(frm - objFeatures[lastFeature].frame));

	if(dir == 0){
		if((c.x - last.x) < 0){
			return false;
		}

		double dist = norm(c - last);
		int match = abs(dist - projection);

		if(match < 100){
			return true;
		}
		else{
			return false;
		}
	}
	else{
		if((c.x - last.x) > 0){
			return false;
		}

		double dist = norm(c - last);
		int match = abs(dist - projection);

		if(match < 100){
			return true;
		}
		else{
			return false;
		}
	}

	return false;
}

bool Object::oppDirectionMatch(Point2f c, int frm){
	int lastFeature = objFeatures.size() - 1;
	int dir = objFeatures[lastFeature].direction;
	double vel = objFeatures[lastFeature].velocity;
	Point2f last = objFeatures[lastFeature].center;
	double projection = vel * (abs(frm - objFeatures[lastFeature].frame));

	if(dir == 0){
		if((c.x - last.x) > 0){
			return false;
		}

		double dist = norm(c - last);
		int match = abs(dist - projection);

		if(match < 100){
			return true;
		}
		else{
			return false;
		}
	}
	else{
		if((c.x - last.x) < 0){
			return false;
		}

		double dist = norm(c - last);
		int match = abs(dist - projection);

		if(match < 100){
			return true;
		}
		else{
			return false;
		}
	}

	return false;
}

bool Object::aroundObjectMatch(Point2f c){
	int lastFeature = objFeatures.size() - 1;
	Point2f last = objFeatures[lastFeature].center;

	double dist = norm(c - last);
	if(dist < 100){
		return true;
	}
	else{
		return false;
	}

	return false;
}

bool Object::checkStatus(int frm){
	int lastFeature = objFeatures.size() - 1;
	int lastFrame = objFeatures[lastFeature].frame;

	int notUpdated = abs(lastFrame - frm);

	if(notUpdated >= 5){
		int dir = objFeatures[lastFeature].direction;
		Point2f lastOccurence = objFeatures[lastFeature].center;
		if(dir == 0){
			if((lastOccurence.x < 640) && (lastOccurence.x > (640 - 200))){
				return true;
			}
			else{
				return false;
			}
		}
		else{
			if((lastOccurence.x > 0) && (lastOccurence.x < 200)){
				return true;
			}
			else{
				return false;
			}
		}
	}

	return false;
}

bool Object::findProjection(Mat &hue, Mat &mask, Rect currRect){
	int lastFeature = objFeatures.size() - 1;
	Rect prevRect = objFeatures[lastFeature].rect;

	Mat hist, backproj;
	int hsize = 16;
	float hranges[] = {0,180};
	const float* phranges = hranges;

    // Create images based on selected regions of interest
    Mat roi(hue, prevRect), maskroi(mask, prevRect);

	// Compute the histogram and normalize it
	calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
	normalize(hist, hist, 0, 255, CV_MINMAX);

    // Compute the histogram backprojection
    calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
    backproj &= mask;
    RotatedRect rotatedRect = CamShift(backproj, prevRect, TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1));
    Rect trackRect = rotatedRect.boundingRect();

    Rect projection = currRect & trackRect;

    if(projection.area() > 0){
    	return true;
    }

    return false;

}
