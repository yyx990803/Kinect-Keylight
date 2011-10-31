#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "tracker.h"
#include "playHead.h"

class testApp : public ofBaseApp {
public:

	void setup();
	void update();
	void draw();
	void exit();

	void drawPointCloud();

	void keyPressed  (int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);

	ofxKinect 			kinect;
	
	/// used to switch between the live kinect and the recording player
	ofxBase3DVideo* 	kinectSource;

	ofxCvColorImage		colorImg;

	ofxCvGrayscaleImage grayImage;			// grayscale depth image
	ofxCvGrayscaleImage grayThreshNear;		// the near thresholded image
	ofxCvGrayscaleImage grayThreshFar;		// the far thresholded image

	ofxCvContourFinder 	contourFinder;
	int 				nearThreshold;
	int					farThreshold;

	vector < tracker > trackerObjects;
	int idCount;

	vector < ofSoundPlayer > sounds;
	
	playHead playhead;
	
	int width;
	int height;
	int cols;
	int rows;
	float playSpeed;
	
	int debug;
};
