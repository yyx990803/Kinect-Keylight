#include "tracker.h"

float tracker::distanceTo(tracker p) {
	float dx = posScreen.x - p.posScreen.x;
	float dy = posScreen.y - p.posScreen.y;
	return sqrt(dx*dx + dy*dy);
}