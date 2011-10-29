#pragma once

#include "ofMain.h"

class tracker {
public:
	ofPoint		pos;
	int			nFramesActive;
	bool		bFoundThisFrame;
	int			whoThisFrame;
	int			id;
	
	ofPoint		posSmooth;
	ofPoint		posScreen;
	
	float distanceTo(tracker p);
};