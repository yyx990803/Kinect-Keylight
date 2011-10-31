#pragma once

#include "ofMain.h"
#include "particle.h"

class tracker {
public:
	
	tracker ();
	
	ofPoint		pos;
	ofPoint		posSmooth;
	ofPoint		posScreen;
	int			nFramesActive;
	bool		bFoundThisFrame;
	int			whoThisFrame;
	int			id;
	
	float size;
	float sizeTarget;
	
	int r;
	int g;
	int b;
	
	vector < particle > particles;
	
	float distanceTo(float x, float y);
	void update(float x, float y, int index, int kinectWidth=640, int kinectHeight=480);
	void draw(int debug=0, int kinectWidth=640, int kinectHeight=480);
	void drawParticles();
	void emit(ofPoint direction);
};