#include "testApp.h"

bool notFountThisFrame(tracker t){
	return !t.bFoundThisFrame;
}

//--------------------------------------------------------------
void testApp::setup() {
	
	ofBackground(0, 0, 0);
	ofSetFrameRate(60);
    ofSetLogLevel(OF_LOG_VERBOSE);

	kinect.init();
	//kinect.init(true);  // shows infrared instead of RGB video image
	//kinect.init(false, false);  // disable infrared/rgb video iamge (faster fps)
	kinect.setVerbose(true);
	kinect.open();

	// start with the live kinect source
	kinectSource = &kinect;
	
	kinect.setCameraTiltAngle(0);

	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);

	nearThreshold = 190;
	farThreshold  = 172;
	idCount = 0;
	
	debug = true;
	showTracking = true;
	ease = 0.95;
	
	synth.loadSound("synth.wav");

}

//--------------------------------------------------------------
void testApp::update() {

	kinectSource->update();

	// there is a new frame and we are connected
	if(kinectSource->isFrameNew()) {

		// load grayscale depth image from the kinect source
		grayImage.setFromPixels(kinectSource->getDepthPixels(), kinect.width, kinect.height);
		
		// threshold with CV
		grayThreshNear = grayImage;
		grayThreshFar = grayImage;
		grayThreshNear.threshold(nearThreshold, true);
		grayThreshFar.threshold(farThreshold);
		cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);

		// update the cv images
		grayImage.flagImageChanged();
		
		// find contours which are between the size of 20*20 pixels and 1/5 the w*h pixels.
    	contourFinder.findContours(grayImage, 400, (kinect.width*kinect.height)/5, 10, false);
		
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
				
				for (int j = 0; j < trackerObjects.size(); j++){
					if (trackerObjects[j].bFoundThisFrame == false){
						
						float dx = trackerObjects[j].pos.x - contourFinder.blobs[i].centroid.x;
						float dy = trackerObjects[j].pos.y - contourFinder.blobs[i].centroid.y;
						float distance = sqrt(dx*dx + dy*dy);
						if (distance < minDistance){
							minDistance = distance;
							minIndex = j;
						}
					}
				}
				
				if (minIndex != -1 && minDistance < 100){		// 100 = just a guess.
					trackerObjects[minIndex].pos.x = contourFinder.blobs[i].centroid.x;
					trackerObjects[minIndex].pos.y = contourFinder.blobs[i].centroid.y;
					
					trackerObjects[minIndex].posSmooth.x = ease * trackerObjects[minIndex].posSmooth.x + 
					(1.0f-ease) * trackerObjects[minIndex].pos.x;
					trackerObjects[minIndex].posSmooth.y = ease * trackerObjects[minIndex].posSmooth.y + 
					(1.0f-ease) * trackerObjects[minIndex].pos.y;
					
					trackerObjects[minIndex].trail.push_back(trackerObjects[minIndex].posSmooth);
					
					trackerObjects[minIndex].whoThisFrame = i;
					trackerObjects[minIndex].bFoundThisFrame = true;
					trackerObjects[minIndex].nFramesActive ++;
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
					
					temp.trail.push_back(temp.pos);
					
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

	if(debug) {
		
		kinect.drawDepth(10, 10);
		kinect.draw(kinect.width+20, 10);
		grayImage.draw(10, kinect.height+20);
		contourFinder.draw(10, kinect.height+20);
		// draw instructions
		ofSetColor(255, 255, 255);
		stringstream reportStream;
		reportStream << "Near Treshold: " << nearThreshold << " (press: + -)" << endl
		<< "Far Threshold: " << farThreshold << " (press: < >)" << endl
		<< "Blobs found: " << contourFinder.nBlobs << endl
		<< "FPS: " << ofGetFrameRate() << endl
		<< "Connection: " << kinect.isConnected() << " (open: o, close: c)";
		ofDrawBitmapString(reportStream.str(),20,ofGetHeight()-65);
		
		for (int i = 0; i < trackerObjects.size(); i++){
			int id = trackerObjects[i].id;
			float ox = 10 + trackerObjects[i].posSmooth.x;
			float oy = 20 + kinect.height + trackerObjects[i].posSmooth.y;
			ofFill();
			ofSetColor(255, 0, 0);
			ofCircle(ox, oy, 2);
			ofDrawBitmapString("blob id:" + ofToString(id), ox+5, oy+5);
		}
		
    } else {
		
		for (int i = 0; i < trackerObjects.size(); i++){
			float ox = ofMap(trackerObjects[i].posSmooth.x, 0, kinect.width, 0, ofGetWidth(), true);
			float oy = ofMap(trackerObjects[i].posSmooth.y, 0, kinect.height, 0, ofGetHeight(), true);
			int id = trackerObjects[i].id;
			
			int frame = ofGetFrameNum();
			if (frame % 120 == 0) {
				float pct = ox / ofGetWidth();
				synth.play();
				synth.setSpeed( 0.1f + ((float)(ofGetHeight() - oy) / (float)ofGetHeight())*10);
				synth.setPan(pct);
			}
			
			if (showTracking) {
				ofFill();
				ofSetColor(255, 60, 60);
				ofCircle(ox, oy, 5);
				ofSetColor(255, 255, 255);
				ofDrawBitmapString("blob id:" + ofToString(id), ox+10, oy+5);
			}
		}
		
	}

}

//--------------------------------------------------------------
void testApp::exit() {
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
}

//--------------------------------------------------------------
void testApp::keyPressed (int key) {
	switch (key) {
		
		case 'd':
			debug = !debug;
			break;
			
		case 's':
			showTracking = !showTracking;
			break;
			
		case '>':
		case '.':
			farThreshold ++;
			if (farThreshold > 255) farThreshold = 255;
			break;

		case '<':
		case ',':
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

		case 'w':
			kinect.enableDepthNearValueWhite(!kinect.isDepthNearValueWhite());
			break;

		case 'o':
			kinect.open();
			break;

		case 'c':
			kinect.setCameraTiltAngle(0);		// zero the tilt
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