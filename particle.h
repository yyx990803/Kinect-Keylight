#pragma once

#include "ofMain.h"

class particle {
public:
	
	particle ();
	
	ofPoint pos;
	ofPoint vel;
	float rotation;
	float radius;
	
	void draw(int r, int g, int b);
};