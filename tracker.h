#pragma once

#include "ofMain.h"
#include "particle.h"

class tracker {
public:
	
	tracker ();
	
	ofPoint		pos;
	int			nFramesActive;
	bool		bFoundThisFrame;
	int			whoThisFrame;
	int			id;
	
	float size;
	float sizeTarget;
	
	int r;
	int g;
	int b;
	
	ofPoint		posSmooth;
	ofPoint		posScreen;
	vector < particle > particles;
	
	float distanceTo(float x, float y);
	void update(float x, float y, int index, int ow=640, int oh=480);
	void draw(int debug=0, float x=10, float y=310, int ow=640, int oh=480, int width=400, int height=300);
	void drawParticles();
	void emit(ofPoint direction);
};