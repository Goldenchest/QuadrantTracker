#include "ColorViewer.h"
#include "HSVHistogram.h"
#include "ObjectTracker.h"
using namespace std;

cv::VideoCapture camera(1); // Set secondary webcam as default

ColorViewer ColorTracker; // Displays the color that's currently being tracked
HSVHistogram Hist; // Analyzes regions of interest and determines appropriate color to track
ObjectTracker TargetTracker; // Given an input color, tracks objects in image of that color

// Mat Images
cv::Mat targetImage; // Holds the current region of interest
cv::Mat image; // Holds each frame of the camera stream

// Boxes
cv::Rect boundingRect; // Visual representation of the current region of interest
cv::Point point1, point2; // The two points that define the region of interest

// Booleans
bool mouseButtonDown = false; // Checks to see if left mouse button has been pressed
bool targetSelected = false; // Checks to see if a target has been selected
bool HSVDefined = false; // Checks to see if a target color has been determined

string imageName = "Stream"; // The name of the video window

// Colors
cv::Scalar tealColor = cv::Scalar(255,255,0);
cv::Scalar redColor = cv::Scalar(0,0,255);
cv::Scalar blackColor = cv::Scalar(0,0,0);
cv::Scalar yellowColor = cv::Scalar(0,255,255);
cv::Scalar greenColor = cv::Scalar(0,255,0);

// Takes a rectangle and shrinks it to tighten its focus on the target's color
cv::Rect calibratedRect(const cv::Rect rect) {
	cv::Rect rect_;
	int x_padding = max(int(0.2*rect.width), 1);
	int y_padding = max(int(0.2*rect.height), 1);
	rect_.x = rect.x + x_padding;
	rect_.y = rect.y + y_padding;
	rect_.width = rect.width - 2*x_padding;
	rect_.height = rect.height - 2*y_padding;
	return rect_;
}

// Allows user to drag a rectangle around desired target in image
void CallBackFunc(int evnt, int x, int y, int flags, void* userdata) {
	if (evnt == cv::EVENT_LBUTTONDOWN) {
		mouseButtonDown = true;
		targetSelected = false;
		boundingRect = cv::Rect(0,0,0,0);
		point1 = cv::Point(x,y);
		cv::destroyWindow(ColorTracker.getColorSquareWindowName());
		targetImage.release();
	}
	if (evnt == cv::EVENT_MOUSEMOVE) {
		if (x < 0) x = 0;
		else if (x > image.cols) x = image.cols;
		if (y < 0) y = 0;
		else if (y > image.rows) y = image.rows;
		point2 = cv::Point(x,y);
		if (mouseButtonDown) {
			boundingRect = cv::Rect(point1,point2);
		}
		cv::imshow(imageName,image);
	}
	if (evnt == cv::EVENT_LBUTTONUP) {
		mouseButtonDown = false;
		if (boundingRect.area() != 0) {
			targetImage = image(calibratedRect(boundingRect));
		}
		else {
			boundingRect = cv::Rect(point1-cv::Point(5,5),point1+cv::Point(5,5));
			targetImage = image(calibratedRect(boundingRect));
		}
		targetSelected = true;
    }
}

// Wrapper function for CallBackFunc
void processMouseActions() {
	cv::setMouseCallback(imageName,CallBackFunc,NULL);
}

// Display the Hue, Saturation, and Value histograms of the tracked object
void showHistograms() {
	cv::imshow("Hue Histogram", Hist.getHueHistogramImage(targetImage));
	cv::imshow("Sat Histogram", Hist.getSatHistogramImage(targetImage));
	cv::imshow("Val Histogram", Hist.getValHistogramImage(targetImage));
}

// Quadrant drawing variables
int frameWidth = int(camera.get(CV_CAP_PROP_FRAME_WIDTH));
int frameHeight = int(camera.get(CV_CAP_PROP_FRAME_HEIGHT));
int centerRectSize = 50;
cv::Point centerPoint = cv::Point(frameWidth/2, frameHeight/2);
cv::Rect centerRectangle = cv::Rect(cv::Point(centerPoint.x-centerRectSize, centerPoint.y-centerRectSize),
	cv::Point(centerPoint.x+centerRectSize, centerPoint.y+centerRectSize));
bool targetInQ1 = false;
bool targetInQ2 = false;
bool targetInQ3 = false;
bool targetInQ4 = false;
bool targetCentered = false;

// Draw the red square in the center of the image
void drawCenterBox(int thickness=1) {
	cv::rectangle(image, centerRectangle, redColor, thickness);
}

// Draw the center dot
void drawCenterDot() {
	cv::circle(image, centerPoint, 3, redColor, -1);
}

// Highlight the desired quadrant
void highlightQuadrant(int quadrantNum) {
	if (quadrantNum == 1) {
		cv::rectangle(image, cv::Point(centerPoint.x,0), cv::Point(frameWidth, centerPoint.y), yellowColor, 2);
		drawCenterDot();
		drawCenterBox();
	}
	else if (quadrantNum == 2) {
		cv::rectangle(image, cv::Point(0,0), centerPoint, yellowColor, 2);
		drawCenterDot();
		drawCenterBox();
	}
	else if (quadrantNum == 3) {
		cv::rectangle(image, cv::Point(0, centerPoint.y), cv::Point(centerPoint.x, frameHeight), yellowColor, 2);
		drawCenterDot();
		drawCenterBox();
	}
	else if (quadrantNum == 4) {
		cv::rectangle(image, cv::Point(centerPoint.x,centerPoint.y), cv::Point(frameWidth, frameHeight), yellowColor, 2);
		drawCenterDot();
		drawCenterBox();
	}
}

// Highlight all quadrants
void highlightAllQuadrants() {
	cv::line(image, cv::Point(0, centerPoint.y), cv::Point(frameWidth, centerPoint.y), greenColor, 2);
	cv::line(image, cv::Point(centerPoint.x, 0), cv::Point(centerPoint.x, frameHeight), greenColor, 2);
	drawCenterDot();
	drawCenterBox(2);
}

// Draw the quadrants and highlight the appropriate quadrants
void drawQuadrants(cv::Point targetCoord) {
	cv::line(image, cv::Point(0, centerPoint.y), cv::Point(frameWidth, centerPoint.y), blackColor);
	cv::line(image, cv::Point(centerPoint.x, 0), cv::Point(centerPoint.x, frameHeight), blackColor);
	if (targetCoord != cv::Point(999,999)) {
		if (centerRectangle.contains(targetCoord)) {
			targetCentered = true;
			targetInQ1 = targetInQ2 = targetInQ3 = targetInQ4 = false;
			highlightAllQuadrants();
		}
		else if (targetCoord.x >= centerPoint.x) {
			if (targetCoord.y <= centerPoint.y) {
				targetInQ1 = true;
				targetInQ2 = targetInQ3 = targetInQ4 = targetCentered = false;
				highlightQuadrant(1);
			}
			else if (targetCoord.y > centerPoint.y) {
				targetInQ4 = true;
				targetInQ1 = targetInQ2 = targetInQ3 = targetCentered = false;
				highlightQuadrant(4);
			}
		}
		else if (targetCoord.x < centerPoint.x) {
			if (targetCoord.y <= centerPoint.y) {
				targetInQ2 = true;
				targetInQ1 = targetInQ3 = targetInQ4 = targetCentered = false;
				highlightQuadrant(2);
			}
			else if (targetCoord.y > centerPoint.y) {
				targetInQ3 = true;
				targetInQ1 = targetInQ2 = targetInQ4 = targetCentered = false;
				highlightQuadrant(3);
			}
		}
	}
}

// Display the current status of the tracked target
void displayTargetStatus() {
	double scale = 0.65;
	int thickness = 2;
	cv::Point textPos = cv::Point(10,30);
	if (targetInQ1)
		cv::putText(image, "UPPER RIGHT", textPos, cv::FONT_HERSHEY_SIMPLEX, scale, tealColor, thickness);
	else if (targetInQ2)
		cv::putText(image, "UPPER LEFT", textPos, cv::FONT_HERSHEY_SIMPLEX, scale, tealColor, thickness);
	else if (targetInQ3)
		cv::putText(image, "LOWER LEFT", textPos, cv::FONT_HERSHEY_SIMPLEX, scale, tealColor, thickness);
	else if (targetInQ4)
		cv::putText(image, "LOWER RIGHT", textPos, cv::FONT_HERSHEY_SIMPLEX, scale, tealColor, thickness);
	else if (targetCentered)
		cv::putText(image, "TARGET CENTERED", textPos, cv::FONT_HERSHEY_SIMPLEX, scale, tealColor, thickness);
	else
		cv::putText(image, "Click the object to track.", textPos, cv::FONT_HERSHEY_SIMPLEX, scale, tealColor, thickness);
}

int main() {
	// If secondary webcam is not detected, switch to primary webcam
	if (!camera.isOpened()) {
		camera = 0;
		if (!camera.isOpened())
			return 1;
	}
	ColorTracker.createColorSquare(); // Create color square to later display target color
	TargetTracker.setThresh(20, 50, 70); // Set HSV threshold
	TargetTracker.setMinContourArea(500); // Set threshold for minimum contour area
	while (cv::waitKey(20) != 13) { // While the user doesn't press Enter...
		if (!camera.read(image)) return 1; // If camera disconnects, exit
		cv::flip(image, image, 1); // Flip the image horizontally
		processMouseActions(); // Detect mouse actions (allows user to drag box around target)
		if (mouseButtonDown) { // If the left mouse button has been pressed...
			cv::rectangle(image, boundingRect, tealColor, 1); // Draw a rectangle from initial point to current mouse position
			targetSelected = false; // Target has not been selected
			HSVDefined = false; // Target color has not been defined
		}
		if (targetSelected && !HSVDefined) { // If a target selected but color not determined...
			targetImage = image(calibratedRect(boundingRect)); // Shrink region of interest to focus on center
			int hue = Hist.getMostAbundantHue(Hist.getHueHistogram(targetImage)); // Get hue [0,180]
			int sat = Hist.getMostAbundantSat(Hist.getSatHistogram(targetImage)); // Get saturation [0,255]
			int val = Hist.getMostAbundantVal(Hist.getValHistogram(targetImage)); // Get value [0,255]
			ColorTracker.setColorSquareHSV(hue, sat, val); // Set the color square to the appropriate color
			TargetTracker.setHSVToTrack(hue, sat, val); // Set the color to track
			HSVDefined = true; // A color has been determined for the target
		}
		if (targetSelected && HSVDefined) {
			TargetTracker.findTargetAndUpdateRectangle(image); // Search image for the largest appropriately colored target
			boundingRect = TargetTracker.getBoundingRect(); // Update the displayed rectangle to follow the target
			targetImage = image(calibratedRect(boundingRect)); // Create a new region of interest centered around the target
			showHistograms(); // Show the HSV Histograms of the newly updated region of interest
		}
		drawQuadrants(TargetTracker.getTargetCoordinates()); // Draw and highlight appropriate quadrants based on target's coordinates
		displayTargetStatus(); // Display the status of the target
		cv::imshow(imageName,image); // Display each frame of video
	}
}