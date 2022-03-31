#pragma once
//  Student Name: Alexander Len
//  Date: May 15, 2020

#include "ofMain.h"
#include  "ofxAssimpModelLoader.h"
#include "box.h"
#include "ray.h"
#include "Particle.h"
#include "ParticleEmitter.h"
#include "Octree.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void drawAxis(ofVec3f);
		void initLightingAndMaterials();
		void savePicture();
		void toggleWireframeMode();
		void togglePointsDisplay();
		void toggleSelectTerrain();
		void setCameraTarget();
		glm::vec3 getMousePointOnPlane();
		bool  doPointSelection();
		void drawBox(const Box &box);
		Box meshBounds(const ofMesh &);
		

		ofEasyCam cam;
		ofCamera onBoardCamera;
		ofCamera sideCamera;
		ofCamera frontCamera;
		ofCamera *theCam;
		ofxAssimpModelLoader mars, rover, lander;
		ofLight light;
		Box boundingBox;
		Box landerBounds;
	
		bool bAltKeyDown;
		bool bCtrlKeyDown;
		bool bWireframe;
		bool bDisplayPoints;
		bool bPointSelected;
		
		bool bLanderLoaded = false;
		bool bTerrainSelected;
		bool bLanderSelected = false;
	
		ofVec3f selectedPoint;
		ofVec3f intersectPoint;
		
		glm::vec3 mouseDownPos;

		void checkCollisions();
		void drawParticles();
		void drawLanding();
		float gravity;
		float damping;
		float radius;
		ofVec3f velocity;
		float lifespan;
		float rate;
		float restitution;

		ParticleEmitter emitter;
		ParticleEmitter landerEm;

		GravityForce grav;
		ImpulseForce impulse;
		ThrusterForce thrust;
		TurbulenceForce turbulence;

		Particle landerParticle;

		const float selectionRange = 4.0;

		int left;
		int right;
		int up;
		int down;
		int space;

		Octree oct;
		Ray sensor;
		float altitude;
		ofVec3f alt;
		int sensorDir;
		float randomX = rand() % 100 - 50; //random value from -50 to 49
		float randomZ = rand() % 100 - 50; //random value from -50 to 49
		Box landing = Box(Vector3(randomX, -10, randomZ), Vector3(randomX + 20, 10, randomZ + 20)); //Landing area is random from (-50, 0, -50) to (49, 0, 49)
		Box bounds = Box(Vector3(0, 0, 0), Vector3(0, 0, 0));
		bool inside = false;

		ofSoundPlayer fireSound;

		ofLight marsLight, landerLight, landingLight;
};
