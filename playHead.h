#pragma once

#include "ofMain.h"
#include "tracker.h"

class playHead {
public:
	
	playHead ();
	
	int index;
	int length;
	int size;
	vector<ofPoint> positions;
	
	int r;
	int g;
	int b;
	
	float distanceTo(tracker p);
	void addPosition(ofPoint p);
	ofPoint getPosition();
	void draw();
};