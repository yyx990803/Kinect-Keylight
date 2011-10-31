#include "particle.h"

particle::particle() {
	pos.set(0,0);
	vel.set(0,0);
	rotation = 0;
	radius = 0;
}

void particle::draw(int r, int g, int b) {
	
	pos.x += vel.x;
	pos.y += vel.y;
	vel.x *= 0.97;
	vel.y *= 0.97;
	rotation += vel.z;
	
	float x = pos.x + cos(rotation) * radius;
	float y = pos.y + sin(rotation) * radius;
	
	ofFill();
	ofSetColor(r, g, b, (int)(76.5+ofRandom(178.5)));
	ofCircle(x, y, 0.5 + ofRandom(1));
}

