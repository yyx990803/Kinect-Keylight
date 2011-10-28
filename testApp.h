#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"

typedef struct {
	
	ofPoint		pos;
	int			nFramesActive;
	bool		bFoundThisFrame;
	int			whoThisFrame;
	int			id;
	
	ofPoint		posSmooth;
	vector <ofPoint> trail;
	
} tracker;

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
		bool debug;
		bool showTracking;
	
		vector < tracker > trackerObjects;
		int idCount;
		float ease;
	
		ofSoundPlayer  synth;
};
