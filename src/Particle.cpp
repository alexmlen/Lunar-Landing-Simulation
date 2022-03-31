#include "Particle.h"


Particle::Particle() {
	velocity.set(0, 0, 0);
	acceleration.set(0, 0, 0);
	position.set(0, 0, 0);
	forces.set(0, 0, 0);
	lifespan = 5;
	birthtime = 0;
	radius = .001;
	damping = .9;
	mass = 1;
	color = ofColor::aquamarine;
}

//draws circle particle
void Particle::draw() {
	ofSetColor(color);
	ofDrawSphere(position, radius);
}

//integrator
void Particle::integrate() {
	float framerate = ofGetFrameRate();
	if (framerate < 1.0) return;
	float dt = 1.0 / framerate;
	position += (velocity * dt);
	ofVec3f accel = acceleration;
	accel += (forces * (1.0 / mass));
	velocity += accel * dt;
	velocity *= damping;
	forces.set(0, 0, 0);
}

//If a value gets too small just make it 0
float Particle::roundToZero(float value) {
	if (abs(value) < 0.012) { return 0; }
	else { return value; }
}

//  return age in seconds
float Particle::age() {
	return (ofGetElapsedTimeMillis() - birthtime)/1000.0;
}


