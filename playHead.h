#pragma once

#include "ofMain.h"
#include "tracker.h"

class playHead {
public:
	
	playHead ();
	
	int index;
	int length;
	int size;
	
	int r;
	int g;
	int b;
	
	vector<ofPoint> positions;
	
	float distanceTo(tracker p);
	void addPosition(ofPoint p);
	ofPoint getPosition();
	void draw(int debug);
};