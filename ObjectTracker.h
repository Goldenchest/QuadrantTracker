#ifndef OBJECT_TRACKER_H
#define OBJECT_TRACKER_H
#include "OpenCVIncludes.h"

class ObjectTracker {
private:
	cv::Rect boundingRect;
	int Ht, St, Vt;
	int minContourArea;
	std::vector<std::vector<cv::Point> > contours;
	std::vector<std::vector<cv::Point> >::iterator itc;
	cv::Scalar lowerHSV, upperHSV;
	cv::Mat result;
	cv::Scalar tealColor;
	bool bContourDetected;
	cv::Point contourCoordinates;
	
public:
	ObjectTracker() {
		setThresh(50,50,50);
		setMinContourArea(100);
		tealColor = cv::Scalar(255,255,0);
		contourCoordinates = cv::Point(NULL, NULL);
	}
	void setMinContourArea(int area) {
		minContourArea = area;
	}
	cv::Rect getBoundingRect() {
		return boundingRect;
	}
	void setThresh(int H_thresh, int S_thresh, int V_thresh) {
		Ht = H_thresh;
		St = S_thresh;
		Vt = V_thresh;
	}
	void setHSVToTrack(int H, int S, int V) {
		lowerHSV = cv::Scalar(H-Ht, S-St, V-Vt);
		upperHSV = cv::Scalar(H+Ht, S+St, V+Vt);
	}
	bool contourDetected() {
		return bContourDetected;
	}
	void findTargetAndUpdateRectangle(cv::Mat& image) {
		cv::cvtColor(image, image, CV_BGR2HSV);
		cv::inRange(image, lowerHSV, upperHSV, result);
		cv::imshow("Thresholded", result);
		cv::findContours(result, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
		cv::cvtColor(image, image, CV_HSV2BGR);
		bContourDetected = false;
		std::vector<cv::Point> biggestContour;
		int minContourArea_ = minContourArea;
		for (itc = contours.begin(); itc != contours.end(); ++itc) {
			int contourArea = int(cv::contourArea(*itc));
			if (contourArea > minContourArea_) {
				bContourDetected = true;
				boundingRect = cv::boundingRect(cv::Mat(*itc));
				biggestContour = *itc;
				minContourArea_ = contourArea;
			}
		}
		if (bContourDetected) {
			cv::rectangle(image, boundingRect, tealColor, 1);
			cv::Moments mom = cv::moments(cv::Mat(biggestContour));
			contourCoordinates = cv::Point(int(mom.m10/mom.m00),int(mom.m01/mom.m00));
			cv::circle(image,contourCoordinates,2,tealColor,2);
			printContourCoordinates(image,contourCoordinates);
		}
	}
	cv::Point getTargetCoordinates() {
		return contourCoordinates;
	}
	std::string convertInt(int num) {
		std::stringstream ss;
		ss << num;
		return ss.str();
	}
	void printContourCoordinates(cv::Mat image, cv::Point coordinates) {
		if (contourDetected()) {
			cv::Point textOrg(contourCoordinates.x-110,contourCoordinates.y-70);
			std::string coordinate = "Coordinates: (" + convertInt(coordinates.x) + "," + convertInt(coordinates.y) + ")";
			int fontface = cv::FONT_HERSHEY_SIMPLEX;
			double scale = 0.5;
			int thickness = 1;
			cv::putText(image,coordinate,textOrg,fontface,scale,tealColor,thickness);
		}
	}
};

#endif OBJECT_TRACKER_H
