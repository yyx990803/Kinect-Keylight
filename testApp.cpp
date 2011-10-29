#include "testApp.h"

bool notFountThisFrame(tracker t){
	return !t.bFoundThisFrame;
}

//--------------------------------------------------------------
void testApp::setup() {
	
	ofBackground(0, 0, 0);
	ofSetFrameRate(40);
    ofSetLogLevel(OF_LOG_VERBOSE);

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

	nearThreshold = 190;
	farThreshold  = 172;
	idCount = 0;
	
	debug = true;
	showTracking = true;
	ease = 0.95;
	playSpeed = 2;
	
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
					
					trackerObjects[minIndex].posScreen.x = ofMap(trackerObjects[minIndex].posSmooth.x, 0, width, 0, ofGetWidth(), true);
					trackerObjects[minIndex].posScreen.y = ofMap(trackerObjects[minIndex].posSmooth.y, 0, height, 0, ofGetHeight(), true);
					
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
		
		kinect.drawDepth(ofGetWidth()-1230, ofGetHeight()-310, 400, 300);
		kinect.draw(ofGetWidth()-820, ofGetHeight()-310, 400, 300);
		grayImage.draw(ofGetWidth()-410, ofGetHeight()-310, 400, 300);
		contourFinder.draw(ofGetWidth()-410, ofGetHeight()-310, 400, 300);
		// draw instructions
		ofSetColor(255, 255, 255);
		stringstream reportStream;
		reportStream << "Near Treshold: " << nearThreshold << " (press: + -)" << endl
		<< "Far Threshold: " << farThreshold << " (press: < >)" << endl
		<< "Play Speed: " << playSpeed << " (press: [ ])" << endl
		<< "Blobs found: " << contourFinder.nBlobs << endl
		<< "Toggle Debug: d / Toggle Tracking: t" << endl
		<< "FPS: " << ofGetFrameRate() << endl
		<< "Connection: " << kinect.isConnected() << " (open: o, close: c)";
		ofDrawBitmapString(reportStream.str(),20,ofGetHeight()-85);
		
		for (int i = 0; i < trackerObjects.size(); i++){
			int id = trackerObjects[i].id;
			float ox = ofGetWidth()-410 + ofMap(trackerObjects[i].posSmooth.x, 0, width, 0, 400);
			float oy = ofGetHeight() - 310 + ofMap(trackerObjects[i].posSmooth.y, 0, height, 0, 300);
			ofFill();
			ofSetColor(255, 0, 0);
			ofCircle(ox, oy, 2);
			ofDrawBitmapString("blob id:" + ofToString(id), ox+5, oy+5);
		}
		
    }
	
	if (showTracking) {
		for (int i = 0; i < trackerObjects.size(); i++){
			
			float ox = trackerObjects[i].posScreen.x;
			float oy = trackerObjects[i].posScreen.y;
			int id = trackerObjects[i].id;
			
			ofFill();
			float hw = ofGetWidth()/2;
			int r = 63 + (int)((1-min(ox/hw, 1.0f))*189);
			int g = 63 + (int)(abs((ox>hw ? ox-(hw*2) : ox) / hw) * 189);
			int b = 63 + (int)(max(( (ox - hw) / hw), 0.0f) * 189);
			ofSetColor(r, g, b);
			ofCircle(ox, oy, 5);
			ofSetColor(255, 255, 255);
			ofDrawBitmapString("blob id:" + ofToString(id), ox+10, oy+5);
		}
	}
	
	if (trackerObjects.size() > 1) {
		
		if (playhead.index > trackerObjects.size()-1) {
			playhead.index = 0;
		}
		
		tracker target = trackerObjects[playhead.index];
		ofPoint newPos;
		
		float currentX = playhead.getPosition().x;
		float currentY = playhead.getPosition().y;
		
		float targetX = target.posScreen.x;
		float targetY = target.posScreen.y;
		float newX = currentX + (targetX - currentX) * (float)playSpeed / 10.0f;
		float newY = currentY + (targetY - currentY) * (float)playSpeed / 10.0f;
		
		newPos.set(newX, newY);
		playhead.addPosition(newPos);
		
		if (playhead.distanceTo(target) < 5) {
			
			playhead.index ++;
			if (playhead.index > trackerObjects.size()-1) {
				playhead.index = 0;
			}
			
			//TODO Inherit color from target
			//TODO Target emit particles
			
			int soundIndex = ofClamp((int)(targetY/ofGetHeight()*30), 0, 29);
			float pct = targetX / ofGetWidth();
			sounds[soundIndex].play();
			sounds[soundIndex].setPan(pct);
			
		}
		
		playhead.draw();
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
			
		case 't':
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
			
		case ']':
		case '}':
			playSpeed ++;
			break;
			
		case '[':
		case '{':
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