
//  Kevin M. Smith - CS 134 SJSU

#include "ParticleEmitter.h"

//ctor
ParticleEmitter::ParticleEmitter() {
	sys = new ParticleSystem();
	createdSys = true;
	init();
}
//ctor with particle system
ParticleEmitter::ParticleEmitter(ParticleSystem *s) {
	if (s == NULL)
	{
		cout << "fatal error: null particle system passed to ParticleEmitter()" << endl;
		ofExit();
	}
	sys = s;
	createdSys = false;
	init();
}

ParticleEmitter::~ParticleEmitter() {

	// deallocate particle system if emitter created one internally
	//
	if (createdSys) delete sys;
}

//all variable for ctor
void ParticleEmitter::init() {
	rate = 5;
	velocity = ofVec3f(0, 20, 0);
	lifespan = 3;
	mass = 1;
	randomLife = false;
	lifeMinMax = ofVec3f(2, 4);
	started = false;
	oneShot = false;
	fired = false;
	lastSpawned = 0;
	radius = 0.01;
	particleRadius = .001;
	visible = true;
	type = DirectionalEmitter;
	groupSize = 5;
	damping = .99;
	particleColor = ofColor::floralWhite;
	position = ofVec3f(0, 0, 0);
}

void ParticleEmitter::draw() {
	sys->draw();
}

//starts the emitter
void ParticleEmitter::start() {
	if (started) return;
	started = true;
	lastSpawned = ofGetElapsedTimeMillis();
}

//stop emitter
void ParticleEmitter::stop() {
	started = false;
	fired = false;
}

//updates emitter
void ParticleEmitter::update() {

	float time = ofGetElapsedTimeMillis();

	if (oneShot && started) {
		if (!fired) {

			// spawn a new particle(s)
			//
			for (int i = 0; i < groupSize; i++) {
				spawn(time);
			}

			lastSpawned = time;
		}
		fired = true;
		stop();
	}

	else if (((time - lastSpawned) > (1000.0 / rate)) && started) {

		// spawn a new particle(s)
		//
		for (int i= 0; i < groupSize; i++)
			spawn(time);
	
		lastSpawned = time;
	}

	sys->update();
}

//create 1 particle
void ParticleEmitter::spawn(float time) {

	Particle particle;

	switch (type) {
	case RadialEmitter:
	  {
		ofVec3f dir = ofVec3f(ofRandom(-1, 1), ofRandom(-1, 1), ofRandom(-1, 1));
		float speed = velocity.length();
		particle.velocity = dir.getNormalized() * speed;
		particle.position.set(position);
	  }
	break;
	case SphereEmitter:
		break;
	case DirectionalEmitter:
		particle.velocity = velocity;
		particle.position.set(position);
		break;
	case DiscEmitter:   // x-z plane
	  {  
		ofVec3f dir = ofVec3f(ofRandom(-1, 1), ofRandom(-.2, .2), ofRandom(-1, 1));
	//	dir.y = 0; 
		particle.position.set(position + (dir.normalized() * radius));
		particle.velocity = velocity;
	  }
	}

	// other particle attributes
	//
	if (randomLife) {
		particle.lifespan = ofRandom(lifeMinMax.x, lifeMinMax.y);
	}
	else particle.lifespan = lifespan;
	particle.birthtime = time;
	particle.radius = particleRadius;
	particle.mass = mass;
	particle.damping = damping;
	particle.color = particleColor;

	// add to system
	//
	sys->add(particle);
}
