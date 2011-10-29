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


void playHead::draw() {
	ofSetColor(r, g, b);
	ofCircle(getPosition().x, getPosition().y, 2);
}