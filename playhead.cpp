#include "playHead.h"

playHead::playHead() {
	index = 0;
	size = 2;
	length = 5;
	r = 255;
	g = 0;
	b = 0;
}

float playHead::distanceTo(tracker p) {
	ofPoint myp = getPosition();
	float dx = myp.x - p.posScreen.x;
	float dy = myp.y - p.posScreen.y;
	return sqrt(dx*dx + dy*dy);
}

void playHead::addPosition(ofPoint p) {
	while (positions.size() > length) {
		positions.erase(positions.begin(), positions.begin()+1);
	}
	positions.push_back(p);
}

ofPoint playHead::getPosition() {
	return positions[positions.size()-1];
}


void playHead::draw(int debug) {
	if (positions.size() > 1) {
		ofSetColor(r, g, b);
		ofSetLineWidth(3.5);
		for (int i = 0; i < positions.size()-1; i++) {
			
			float nx = positions[i+1].x;
			float ny = positions[i+1].y;
			
			if (debug == 0) {
				nx = 420 + ofMap(nx, 0, ofGetWidth(), 0, 400, true);
				ny = 320 + ofMap(ny, 0, ofGetHeight(), 0, 300, true);
				ofSetColor(0, 255, 0);
			}
			
			ofBeginShape();
			if (i==0) {
				if (debug == 0) {
					float ox = 420 + ofMap(positions[i].x, 0, ofGetWidth(), 0, 400, true);
					float oy = 320 + ofMap(positions[i].y, 0, ofGetHeight(), 0, 300, true);
					ofVertex(ox, oy);
				} else {
					ofVertex(positions[i].x, positions[i].y);
				}
			}
			ofVertex(nx, ny);
			ofEndShape();
		}
		ofSetLineWidth(1);
	}
}