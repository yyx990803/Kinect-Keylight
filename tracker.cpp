#include "tracker.h"

tracker::tracker() {
	size = 0;
	sizeTarget = 12;
	r = 255;
	g = 0;
	b = 0;
}

float tracker::distanceTo(float x, float y) {
	float dx = pos.x - x;
	float dy = pos.y - y;
	return sqrt(dx*dx + dy*dy);
}

void tracker::update(float x, float y, int index, int kinectWidth, int kinectHeight) {
	pos.x = x;
	pos.y = y;
	float ease = 0.95;
	posSmooth.x = ease * posSmooth.x + (1.0f-ease) * x;
	posSmooth.y = ease * posSmooth.y + (1.0f-ease) * y;
	
	posScreen.x = ofMap(posSmooth.x, 0, kinectWidth, 0, ofGetWidth(), true);
	posScreen.y = ofMap(posSmooth.y, 0, kinectHeight, 0, ofGetHeight(), true);
	
	whoThisFrame = index;
	bFoundThisFrame = true;
	nFramesActive ++;
}

void tracker::draw(int debug, int kinectWidth, int kinectHeight) {
	if (debug == 0) {
		float ox = 10 + ofMap(posSmooth.x, 0, kinectWidth, 0, 400);
		float oy = 320 + ofMap(posSmooth.y, 0, kinectHeight, 0, 300);
		ofFill();
		ofSetColor(255, 0, 0);
		ofCircle(ox, oy, 2);
		ofCircle(ox + 410, oy, 5);
		ofDrawBitmapString("blob id:" + ofToString(id), ox+5, oy+5);
	} else {
		float ox = posScreen.x;
		float oy = posScreen.y;
		float hw = ofGetWidth()/2;
		r = 63 + (int)((1-min(ox/hw, 1.0f))*189);
		g = 63 + (int)(abs((ox>hw ? ox-(hw*2) : ox) / hw) * 189);
		b = 63 + (int)(max(( (ox - hw) / hw), 0.0f) * 189);
		ofFill();
		ofSetColor(r, g, b);
		ofCircle(ox, oy, size);
		ofNoFill();
		ofCircle(ox, oy, size);
		size += (sizeTarget - size)*0.2;
		if (debug == 1) {
			ofSetColor(255, 255, 255);
			ofDrawBitmapString("blob id:" + ofToString(id), ox+10, oy+5);
		}
		if (debug == 2)
			drawParticles();
	}
}

void tracker::drawParticles() {
	if (particles.size() > 0) {
		for (int i = 0; i < particles.size(); i++) {
			if (ofRandom(1.0)>0.4) {
				particles[i].draw(r, g, b);
			}
		}
		
		if (ofRandom(1.0) > 0.8) {
			particles.erase(particles.begin(), particles.begin()+1);
		}
		
		while (particles.size() > 30) {
			particles.erase(particles.begin(), particles.begin()+1);
		}
	}
}

void tracker::emit(ofPoint direction) {
	size = 8;
	int q = 20 + (int)ofRandom(20);
	for (int i = 0; i < q; i++) {
		particle p;
		p.pos.set(posScreen.x, posScreen.y);
		float dx = direction.x - posScreen.x;
		float dy = direction.y - posScreen.y;
		p.pos.x += dx * (0.6*((float)i/(float)q));
		p.pos.y += dy * (0.6*((float)i/(float)q));
		float rr = ((dx+dy)/500.0f)*((float)i/(float)q);
		p.pos.x += -rr + ofRandom(rr+rr);
		p.pos.y += -rr + ofRandom(rr+rr);
		
		p.vel.x = dx/(100.0f + ofRandom(500.0f));
		p.vel.y = dy/(100.0f + ofRandom(500.0f));
		p.vel.z = -0.1 + ofRandom(0.2);
		
		p.radius = ofRandom(20);
		
		particles.push_back(p);
	}
}