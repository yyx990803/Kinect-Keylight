#include "testApp.h"

bool notFountThisFrame(tracker t){
	return !t.bFoundThisFrame;
}

//--------------------------------------------------------------
void testApp::setup() {
	
	ofBackground(0, 0, 0);
	ofSetFrameRate(31);
    ofSetLogLevel(OF_LOG_VERBOSE);
	ofEnableAlphaBlending();
	ofEnableSmoothing();

	kinect.init();
	//kinect.init(true);  // shows infrared instead of RGB video image
	//kinect.init(false, false);  // disable infrared/rgb video iamge (faster fps)
	kinect.setVerbose(true);
	kinect.open();

	// start with the live kinect source
	kinectSource = &kinect;
	
	width = kinect.width;
	height = kinect.height;
	
	colorImg.allocate(width, height);
	grayImage.allocate(width, height);
	grayThreshNear.allocate(width, height);
	grayThreshFar.allocate(width, height);

	nearThreshold = 255;
	farThreshold  = 225; //roof 140 / hand 225
	idCount = 0;
	
	debug = 0; // 0: debug, 1: overlay, 2: projection
	playSpeed = 2;
	cols = 3;
	rows = 10;
	
	ofPoint p;
	p.set(width/2, height/2);
	playhead.addPosition(p);
	
	for (int i = 1; i <= 30; i++) {
		ofSoundPlayer s;
		s.loadSound(ofToString(i) + ".mp3");
		sounds.push_back(s);
	}

}

//--------------------------------------------------------------
void testApp::update() {

	kinectSource->update();

	// there is a new frame and we are connected
	if(kinectSource->isFrameNew()) {

		// load grayscale depth image from the kinect source
		grayImage.setFromPixels(kinectSource->getDepthPixels(), width, height);
		
		// threshold with CV
		grayThreshNear = grayImage;
		grayThreshFar = grayImage;
		grayThreshNear.threshold(nearThreshold, true);
		grayThreshFar.threshold(farThreshold);
		cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);

		// update the cv images
		grayImage.flagImageChanged();
		
		// find contours which are between the size of 20*20 pixels and 1/5 the w*h pixels.
    	contourFinder.findContours(grayImage, 400, (width*height)/5, 10, false);
		
		// Tracking
		
		// a) assume all trackers are *not* found this frame. 
		for (int i = 0; i < trackerObjects.size(); i++){
			trackerObjects[i].bFoundThisFrame = false;
		}
		
		// b) for all blobs this frame, let's see if we can match them to the trackers.  
		int nBlobsFoundThisFrame = contourFinder.nBlobs;
		
		// assume that none have been found:
		bool bAmIFoundThisFrame[nBlobsFoundThisFrame];
		
		if (nBlobsFoundThisFrame > 0){
			
			for (int i = 0; i < nBlobsFoundThisFrame; i++){
				bAmIFoundThisFrame[i] = false;
			}
			
			// now, look through every tracker, and see how far away they are from this blob.
			// find the minimum distance, and see if this is reasonable. 
			for (int i = 0; i < nBlobsFoundThisFrame; i++){
				
				float	minDistance = 100000;
				int		minIndex	= -1;
				
				float bx = contourFinder.blobs[i].centroid.x;
				float by = contourFinder.blobs[i].centroid.y;
				
				for (int j = 0; j < trackerObjects.size(); j++){
					if (trackerObjects[j].bFoundThisFrame == false){
						float distance = trackerObjects[j].distanceTo(bx, by);
						if (distance < minDistance){
							minDistance = distance;
							minIndex = j;
						}
					}
				}
				
				if (minIndex != -1 && minDistance < 100){		// 100 = just a guess.
					trackerObjects[minIndex].update(bx, by, i);
					bAmIFoundThisFrame[i] = true;	// we got one !
				}
			}
		}
		
		// c) for all non found blobs, add a tracker. 
		if (nBlobsFoundThisFrame > 0){
			for (int i = 0; i < nBlobsFoundThisFrame; i++){
				if (bAmIFoundThisFrame[i] == false){
					
					tracker temp;
					temp.pos = contourFinder.blobs[i].centroid;
					temp.posSmooth = temp.pos;
					
					temp.nFramesActive = 0;
					temp.whoThisFrame = i;
					temp.bFoundThisFrame = true;
					temp.id = idCount;
					trackerObjects.push_back(temp);
					idCount ++;
				}
			}
		}
		
		// d) kill all trackers that haven't been found
		trackerObjects.erase( remove_if(trackerObjects.begin(), trackerObjects.end(), notFountThisFrame), trackerObjects.end() );
		
	}
    
}

//--------------------------------------------------------------
void testApp::draw() {

	ofSetColor(255, 255, 255);
	
	if (debug == 0 || debug == 1) {
		// draw instructions
		ofSetColor(255, 255, 255);
		stringstream reportStream;
		reportStream << "Near Treshold: " << nearThreshold << " (- +)" << endl
		<< "Far Threshold: " << farThreshold << " ([ ])" << endl
		<< "Play Speed: " << playSpeed << " (< >)" << endl
		<< "Toggle Debug Mode: d" << endl
		<< "Blobs found: " << contourFinder.nBlobs << endl
		<< "FPS: " << ofGetFrameRate() << endl
		<< "Connection: " << kinect.isConnected() << " (open: o, close: c)";
		ofDrawBitmapString(reportStream.str(),20,ofGetHeight()-95);
	}

	if(debug == 0) {
		//debug mode with all information
		kinect.drawDepth(10, 10, 400, 300);
		kinect.draw(420, 10, 400, 300);
		grayImage.draw(10, 320, 400, 300);
		contourFinder.draw(10, 320, 400, 300);
		ofNoFill();
		ofSetColor(200, 200, 200);
		ofRect(10, 10, 400, 300);
		ofRect(420, 10, 400, 300);
		ofRect(10, 320, 400, 300);
		ofRect(420, 320, 400, 300);
    } else if (debug == 1) {
		//overlay mode for calibration
		kinect.draw(0, 0, ofGetWidth(), ofGetHeight());
	}
	
	if (trackerObjects.size() > 1) {
		
		if (playhead.index > trackerObjects.size()-1) {
			playhead.index = 0;
		}
		
		int oi = playhead.index;
		ofPoint newPos;
		
		float currentX = playhead.getPosition().x;
		float currentY = playhead.getPosition().y;
		
		float targetX = trackerObjects[oi].posScreen.x;
		float targetY = trackerObjects[oi].posScreen.y;
		float newX = currentX + (targetX - currentX) * (float)playSpeed / 10.0f;
		float newY = currentY + (targetY - currentY) * (float)playSpeed / 10.0f;
		
		newPos.set(newX, newY);
		playhead.addPosition(newPos);
		
		if (playhead.distanceTo(trackerObjects[oi]) < 10.0f) {
			
			playhead.index ++;
			if (playhead.index > trackerObjects.size()-1) {
				playhead.index = 0;
			}
			
			playhead.r = trackerObjects[oi].r;
			playhead.g = trackerObjects[oi].g;
			playhead.b = trackerObjects[oi].b;
			
			trackerObjects[oi].emit(trackerObjects[playhead.index].posScreen);
			
			//play corresponding sound
			int sx = ofClamp((int)(targetX/ofGetWidth()*cols), 0, cols-1);
			int sy = ofClamp((int)(targetY/ofGetHeight()*rows), 0, rows-1);
			int soundIndex = sy * cols + sx;
			float pct = targetX / ofGetWidth();
			sounds[soundIndex].play();
			sounds[soundIndex].setPan(pct);
			
		}
		
		playhead.draw(debug);
	}
	
	for (int i = 0; i < trackerObjects.size(); i++){
		//draw all tracked objects
		trackerObjects[i].draw(debug);
	}
	
}

//--------------------------------------------------------------
void testApp::exit() {
	kinect.close();
}

//--------------------------------------------------------------
void testApp::keyPressed (int key) {
	switch (key) {
		
		case 'd':
			debug ++;
			if (debug > 2) debug = 0;
			break;
			
		case ']':
		case '}':
			farThreshold ++;
			if (farThreshold > 255) farThreshold = 255;
			break;

		case '[':
		case '{':
			farThreshold --;
			if (farThreshold < 0) farThreshold = 0;
			break;

		case '+':
		case '=':
			nearThreshold ++;
			if (nearThreshold > 255) nearThreshold = 255;
			break;

		case '-':
			nearThreshold --;
			if (nearThreshold < 0) nearThreshold = 0;
			break;
			
		case '.':
		case '>':
			playSpeed ++;
			break;
			
		case ',':
		case '<':
			playSpeed --;
			break;

		case 'w':
			kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
			break;

		case 'o':
			kinect.open();
			break;

		case 'c':
			kinect.close();
			break;
	}
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button)
{}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h)
{}