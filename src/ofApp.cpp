//  Student Name: Alexander Len
//  Date: May 15, 2020


#include "ofApp.h"
#include "Util.h"
#include <glm/gtx/intersect.hpp>

// setup scene, lighting, models
//
void ofApp::setup(){
	fireSound.load("thrust.wav");
	altitude = -1;
	velocity = ofVec3f(0, -5, 0);
	lifespan = 1.0;
	rate = 5.0;
	damping = 0.99;
	gravity = 1;
	radius = 0.5;
	restitution = 0.4;

	sensorDir = -1;
	
	emitter.setRate(rate);
	emitter.setOneShot(false);

	emitter.start();
	emitter.setLifespan(lifespan);
	emitter.setVelocity(velocity);

	ParticleSystem *sys = emitter.sys;
	grav.set(ofVec3f(0, -1, 0));
	sys->addForce(&grav);
	sys->addForce(new TurbulenceForce(ofVec3f(-3, -1, -1), ofVec3f(3, 1, 1)));

	landerEm.setRate(rate);
	landerEm.setOneShot(true);

	landerEm.setLifespan(-1);
	landerEm.setVelocity(ofVec3f(0, 5, 0));

	ParticleSystem *landerSys = landerEm.sys;
	landerSys->addForce(&grav);
	turbulence.set(ofVec3f(-3, -1, -1), ofVec3f(3, 1, 1));
	landerSys->addForce(&turbulence);
	thrust.set(ofVec3f(0, 0, 0));
	landerSys->addForce(&thrust);

	landerEm.start();

	bWireframe = false;
	bDisplayPoints = false;
	bAltKeyDown = false;
	bCtrlKeyDown = false;
	bTerrainSelected = true;
	cam.setDistance(10);
	cam.setNearClip(.1);
	cam.setFov(65.5);  
	ofSetVerticalSync(true);
	cam.disableMouseInput();
	ofEnableSmoothing();
	ofEnableDepthTest();

	onBoardCamera.setPosition(0, 100, 0);
	onBoardCamera.lookAt(glm::vec3(0, 0, 0));
	onBoardCamera.setNearClip(.1);
	onBoardCamera.setFov(65.5);

	sideCamera.setPosition(0, 0, 0);
	sideCamera.lookAt(glm::vec3(0, 0, 0));
	sideCamera.setNearClip(.1);
	sideCamera.setFov(65.5);

	frontCamera.setPosition(100, 50, 100);
	frontCamera.lookAt(glm::vec3(0, 0, 0));
	frontCamera.setNearClip(.1);
	frontCamera.setFov(65.5);   // approx equivalent to 28mm in 35mm format


	theCam = &cam;
	initLightingAndMaterials();

	//models
	mars.loadModel("moon-houdini.obj");
	mars.setScaleNormalization(false);
	mars.setRotation(2, 180, 0, 0, 1);

	boundingBox = meshBounds(mars.getMesh(0));

	if (lander.loadModel("lander.obj")) {
		lander.setScaleNormalization(false);
		lander.setPosition(0, 0, 0);
		bLanderLoaded = true;
		lander.setRotation(2, 180, 0, 0, 1);
	}
	else cout << "Error: Can't load model:" << " lander.obj" << endl;
	ofMesh mesh = mars.getMesh(0);
	oct.create(mesh, 5);
	sensor = Ray(Vector3(0, 0, 0), Vector3(0, -1, 0));
	alt = ofVec3f(0, std::numeric_limits<float>::infinity(), 0);

	emitter.setPosition(ofVec3f(0, 10, 0));
	fireSound.setLoop(true);
	fireSound.setVolume(0.2);

	ofEnableLighting();

	//Lighting for the center area
	marsLight.setup();
	marsLight.enable();
	marsLight.setPointLight();
	marsLight.setScale(.05);
	marsLight.setAttenuation(.2, .001, .001);
	marsLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	marsLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	marsLight.setSpecularColor(ofFloatColor(1, 1, 1));
	marsLight.setPosition(0, 2, 0);

	//Lighting for the light on the lander
	landerLight.setup();
	landerLight.enable();
	landerLight.setPointLight();
	landerLight.setScale(.02);
	landerLight.setAttenuation(.2, .001, .001);
	landerLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	landerLight.setDiffuseColor(ofFloatColor(0.2, 0.2, 0.2));
	landerLight.setSpecularColor(ofFloatColor(0.2, 0.2, 0.2));
	landerLight.setPosition(0, 0, 0);
	
	//Light for the landing area
	landingLight.setup();
	landingLight.enable();
	landingLight.setPointLight();
	landingLight.setScale(.05);
	landingLight.setAttenuation(.2, .001, .001);
	landingLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
	landingLight.setDiffuseColor(ofFloatColor(1, 1, 1));
	landingLight.setSpecularColor(ofFloatColor(1, 1, 1));
	landingLight.setPosition(randomX + 10, 10, randomZ + 10);
	
}

//Checks if the lander hits something, in this case the ground
void ofApp::checkCollisions() {

	// for each particle, determine if we hit the groud plane.
	//
	for (auto i = 0; i < emitter.sys->particles.size(); i++) {

		ofVec3f vel = emitter.sys->particles[i].velocity; // velocity of particle
		if (vel.y >= 0) break;                             // ascending;

		ofVec3f pos = emitter.sys->particles[i].position;
		if (altitude <= 0) {
			ofVec3f norm = ofVec3f(-vel.x, -vel.y, -vel.z).getNormalized();
			ofVec3f f = (restitution + 1.0)*((-vel.dot(norm))*norm);
			emitter.sys->particles[i].forces += ofGetFrameRate() * f;
		}
	}

	ofVec3f vel = landerEm.sys->particles[0].velocity; // velocity of particle
	if (vel.y >= 0) return;                             // ascending;
	ofVec3f pos = landerEm.sys->particles[0].position;

	if (altitude <= 0 || altitude == std::numeric_limits<float>::infinity()) {

		ofVec3f norm = ofVec3f(0, 1, 0);
		ofVec3f f = (restitution + 1.0)*((-vel.dot(norm))*norm);
		landerEm.sys->particles[0].forces += ofGetFrameRate() * f;
		turbulence.toggle(true);
		if (altitude != std::numeric_limits<float>::infinity()) {
			landerEm.sys->particles[0].position.y = alt.y;
			altitude = 0;
		}
	}
	else {
		turbulence.toggle(false);
	}
}

//Updates scene
void ofApp::update() {
	onBoardCamera.setPosition(lander.getPosition().x, lander.getPosition().y + 10, lander.getPosition().z);
	int x = right - left;
	int y = space;
	int z = down - up;
	int scale = 5;
	int yScale = 10;
	if (y == 0 && altitude == 0.0) {
		x = 0;
		z = 0;
	}
	thrust.set(ofVec3f(x*scale, y*yScale, z*scale));
	landerEm.update();
	ofVec3f lPos = landerEm.sys->particles[0].position;
	lander.setPosition(lPos.x, lPos.y, lPos.z);
	if (altitude == std::numeric_limits<float>::infinity()) {
		sensorDir = -sensorDir;
	}
	sensor = Ray(Vector3(lPos.x, lPos.y, lPos.z), Vector3(0, sensorDir, 0));
	alt = oct.intersect(sensor, oct.root);
	if (alt.y != std::numeric_limits<float>::infinity()) {
		if (sensorDir == 1) {
			landerEm.sys->particles[0].position.y = alt.y;
			altitude = 0;
		}
		else {
			altitude = lPos.y - alt.y;
		}
	}
	else {
		altitude = alt.y;
	}
	checkCollisions();
	ofVec3f lastPos = landerEm.sys->particles[0].position;
	sideCamera.setPosition(lastPos.x + 1, lastPos.y + 1, lastPos.z + 1);
	sideCamera.lookAt(glm::vec3(lastPos.x + 10, lastPos.y - 1, lastPos.z + 10));
	frontCamera.lookAt(glm::vec3(lastPos.x, lastPos.y, lastPos.z));
	cam.lookAt(glm::vec3(lastPos.x, lastPos.y, lastPos.z));

	grav.set(ofVec3f(0, -gravity, 0));
	emitter.setPosition(landerEm.sys->particles[0].position);
	if ((abs(x) + y + abs(z)) != 0) {
		if (!fireSound.isPlaying()) {
			fireSound.play();
		}
		emitter.start();
	}
	else {
		if (fireSound.isPlaying()) {
			fireSound.stop();
		}
		emitter.stop();
	}
	emitter.setParticleRadius(radius);
	emitter.update();

	//Sets position of the light for the landing area
	landerLight.setPosition(lander.getPosition());
}

//--------------------------------------------------------------
void ofApp::drawParticles() {
	ofSetBackgroundColor(ofColor::black);
	ofPushMatrix();
	ofRotateDeg(90, 0, 0, 1);
	ofSetLineWidth(1);
	ofSetColor(ofColor::dimGrey);
	ofPopMatrix();


	ofFill();
	ofPushMatrix();
	ofRotateXDeg(90);
	ofTranslate(0, 0, .1);
	ofSetColor(ofColor::darkGreen);
	ofPopMatrix();

	emitter.draw();
}

//Sets up the landing area
void ofApp::drawLanding() {
	inside = false;
	ofNoFill();
	ofVec3f pos = landerEm.sys->particles[0].position;
	Vector3 pos2 = Vector3(pos.x, pos.y, pos.z);
	Vector3 *points = new Vector3[4];
	if (points == nullptr)
	{
		ofSetColor(0, 0, 0, 100);
	}
	else
	{
		points[0] = pos2;
		points[1] = bounds.min();
		points[2] = bounds.max();
		if (landing.inside(points, 3)) {
			ofSetColor(0, 255, 0, 100);
			inside = true;
		}
		else {
			ofSetColor(255, 0, 0, 100);
		}
	}
	//Actually draws the box for the landing area
	//Was used for testing to see exact area where the lander should go
	//now indicated by light
	//
	//drawBox(landing);
}

void ofApp::draw(){

	ofBackground(ofColor::black);

	theCam->begin();
	ofPushMatrix();
	drawLanding();
	//Sets the color of the light at the landing area
	if (!inside) { //Not in the landing area
		landingLight.setDiffuseColor(ofFloatColor(0.6, 0.1, 0.1));
		landingLight.setSpecularColor(ofFloatColor(0.6, 0.1, 0.1));
	}
	else if (inside && altitude != 0.0) { //In landing area but not landed
		landingLight.setDiffuseColor(ofFloatColor(0.6, 0.6, 0.1));
		landingLight.setSpecularColor(ofFloatColor(0.6, 0.6, 0.1));
	}
	else { //In landing area and landed
		landingLight.setDiffuseColor(ofFloatColor(0.1, 0.6, 0.1));
		landingLight.setSpecularColor(ofFloatColor(0.1, 0.6, 0.1));
	}
	drawParticles();
	if (bWireframe) {                    // wireframe mode  (include axis)
		ofDisableLighting();
		ofSetColor(ofColor::slateGray);
		mars.drawWireframe();
		if (bLanderLoaded) {
			lander.drawWireframe();
			if (!bTerrainSelected) drawAxis(lander.getPosition());
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}
	else {
		ofEnableLighting();              // shaded mode
		mars.drawFaces();

		if (bLanderLoaded) {
			lander.drawFaces();
			if (!bTerrainSelected) drawAxis(lander.getPosition());

			ofVec3f min = lander.getSceneMin() + lander.getPosition();
			ofVec3f max = lander.getSceneMax() + lander.getPosition();

			bounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
		if (bTerrainSelected) drawAxis(ofVec3f(0, 0, 0));
	}


	if (bDisplayPoints) { 
		glPointSize(3);
		ofSetColor(ofColor::green);
		mars.drawVertices();
	}

	// highlight selected point (draw sphere around selected point)
	//
	if (bPointSelected) {
		ofSetColor(ofColor::blue);
		ofDrawSphere(selectedPoint, .1);
	}
	ofNoFill();
	ofSetColor(ofColor::white);
	theCam->end();

	// draws UI
	//
	string strControls;
	strControls += "Red = Not in landing zone, Yellow = In landing zone, Green = Successfully landed in landing zone";
	ofSetColor(ofColor::white);
	ofDrawBitmapString(strControls, 10, 10);

	string str;
	str += "Altitude: " + std::to_string(altitude);
	ofSetColor(ofColor::white);
	ofDrawBitmapString(str, 10, 30);

	bool landed = (inside && altitude == 0.0);
	if (landed) {
		string strLand;
		strLand += "Successfully Landed";
		ofSetColor(ofColor::white);
		ofDrawBitmapString(strLand, 10, 50);
	}
}

// 

// Draw an XYZ axis in RGB at world (0,0,0) for reference.
//
void ofApp::drawAxis(ofVec3f location) {

	ofPushMatrix();
	ofTranslate(location);

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(1, 0, 0));
	

	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 1, 0));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(ofPoint(0, 0, 0), ofPoint(0, 0, 1));

	ofPopMatrix();
}


void ofApp::keyPressed(int key) {

	switch (key) {
	case 'C':
	case 'c':
		if (cam.getMouseInputEnabled()) cam.disableMouseInput();
		else cam.enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'H':
	case 'h':
		break;
	case 'r':
		cam.reset();
		break;
	case 's':
		savePicture();
		break;
	case 't':
		setCameraTarget();
		break;
	case 'u':
		break;
	case 'v':
		togglePointsDisplay();
		break;
	case 'V':
		break;
	case 'w':
		toggleWireframeMode();
		break;
	case OF_KEY_ALT:
		cam.enableMouseInput();
		bAltKeyDown = true;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = true;
		break;
	case OF_KEY_SHIFT:
		break;
	case OF_KEY_DEL:
		break;
	case OF_KEY_F1:
		theCam = &cam;
		break;
	case OF_KEY_F2:
		theCam = &onBoardCamera;
		break;
	case OF_KEY_F3:
		theCam = &sideCamera;
		break;
	case OF_KEY_F4:
		theCam = &frontCamera;
		break;
	case ' ':
		space = 1;
		//emitter.start();
		break;
	case OF_KEY_LEFT:
		left = 1;
		break;
	case OF_KEY_RIGHT:
		right = 1;
		break;
	case OF_KEY_UP:
		up = 1;
		break;
	case OF_KEY_DOWN:
		down = 1;
		break;
	default:
		break;
	}
}

void ofApp::toggleWireframeMode() {
	bWireframe = !bWireframe;
}

void ofApp::toggleSelectTerrain() {
	bTerrainSelected = !bTerrainSelected;
}

void ofApp::togglePointsDisplay() {
	bDisplayPoints = !bDisplayPoints;
}

void ofApp::keyReleased(int key) {

	switch (key) {
	
	case OF_KEY_ALT:
		cam.disableMouseInput();
		bAltKeyDown = false;
		break;
	case OF_KEY_CONTROL:
		bCtrlKeyDown = false;
		break;
	case OF_KEY_SHIFT:
		break;
	case ' ':
		space = 0;
		break;
	case OF_KEY_LEFT:
		left = 0;
		break;
	case OF_KEY_RIGHT:
		right = 0;
		break;
	case OF_KEY_UP:
		up = 0;
		break;
	case OF_KEY_DOWN:
		down = 0;
		break;
	default:
		break;
	}
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

	

}


//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

	// if moving camera, don't allow mouse interaction
	//
	if (cam.getMouseInputEnabled()) return;

	// if lander is loaded, test for selection
	//
	if (bLanderLoaded) {
		glm::vec3 origin = theCam->getPosition();
		glm::vec3 mouseWorld = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);

		bool hit = landerBounds.intersect(Ray(Vector3(origin.x, origin.y, origin.z), Vector3(mouseDir.x, mouseDir.y, mouseDir.z)), 0, 10000);
		if (hit) {
			bLanderSelected = true;
			cout << "lander hit" << endl;
		}
	}

}


//draw a box from a "Box" class  
//
void ofApp::drawBox(const Box &box) {
	Vector3 min = box.parameters[0];
	Vector3 max = box.parameters[1];
	Vector3 size = max - min;
	Vector3 center = size / 2 + min;
	ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
	float w = size.x();
	float h = size.y();
	float d = size.z();
	ofNoFill();
	ofDrawBox(p, w, h, d);
}

// return a Mesh Bounding Box for the entire Mesh
//
Box ofApp::meshBounds(const ofMesh & mesh) {
	int n = mesh.getNumVertices();
	ofVec3f v = mesh.getVertex(0);
	ofVec3f max = v;
	ofVec3f min = v;
	for (int i = 1; i < n; i++) {
		ofVec3f v = mesh.getVertex(i);

		if (v.x > max.x) max.x = v.x;
		else if (v.x < min.x) min.x = v.x;

		if (v.y > max.y) max.y = v.y;
		else if (v.y < min.y) min.y = v.y;

		if (v.z > max.z) max.z = v.z;
		else if (v.z < min.z) min.z = v.z;
	}
	return Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
}



//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {


}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

bool ofApp::doPointSelection() {

	ofMesh mesh = mars.getMesh(0);
	int n = mesh.getNumVertices();
	float nearestDistance = 0;
	int nearestIndex = 0;

	bPointSelected = false;

	ofVec2f mouse(mouseX, mouseY);
	vector<ofVec3f> selection;

	for (int i = 0; i < n; i++) {
		ofVec3f vert = mesh.getVertex(i);
		ofVec3f posScreen = cam.worldToScreen(vert);
		float distance = posScreen.distance(mouse);
		if (distance < selectionRange) {
			selection.push_back(vert);
			bPointSelected = true;
		}
	}

	if (bPointSelected) {
		float distance = 0;
		for (auto i = 0; i < selection.size(); i++) {
			ofVec3f point =  cam.worldToCamera(selection[i]);

			float curDist = point.length(); 

			if (i == 0 || curDist < distance) {
				distance = curDist;
				selectedPoint = selection[i];
			}
		}
	}
	return bPointSelected;
}

void ofApp::setCameraTarget() {

}


//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}



//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

	static float ambient[] =
	{ .5f, .5f, .5, 1.0f };
	static float diffuse[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float position[] =
	{5.0, 5.0, 5.0, 0.0 };

	static float lmodel_ambient[] =
	{ 1.0f, 1.0f, 1.0f, 1.0f };

	static float lmodel_twoside[] =
	{ GL_TRUE };


	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position);


	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
	glShadeModel(GL_SMOOTH);
} 

void ofApp::savePicture() {
	ofImage picture;
	picture.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	picture.save("screenshot.png");
	cout << "picture saved" << endl;
}

//--------------------------------------------------------------
//
// support drag-and-drop of model (.obj) file loading.  when
// model is dropped in viewport, place origin under cursor
//
void ofApp::dragEvent(ofDragInfo dragInfo) {
	if (false) {
	//if (rover.loadModel(dragInfo.files[0])) {
		//bRoverLoaded = true;
		rover.setScaleNormalization(false);
	//	rover.setScale(.5, .5, .5);
		rover.setPosition(0, 0, 0);
		rover.setRotation(1, 180, 1, 0, 0);

		// We want to drag and drop a 3D object in space so that the model appears 
		// under the mouse pointer where you drop it !
		//
		// Our strategy: intersect a plane parallel to the camera plane where the mouse drops the model
		// once we find the point of intersection, we can position the rover/lander
		// at that location.
		//

		// Setup our rays
		//
		glm::vec3 origin = theCam->getPosition();
		glm::vec3 camAxis = theCam->getZAxis();
		glm::vec3 mouseWorld = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
		glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
		float distance;

		bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);
		if (hit) {
			// find the point of intersection on the plane using the distance 
			// We use the parameteric line or vector representation of a line to compute
			//
			// p' = p + s * dir;
			//
			glm::vec3 intersectPoint = origin + distance * mouseDir;

			// Now position the rover's origin at that intersection point
			//
		    glm::vec3 min = rover.getSceneMin();
			glm::vec3 max = rover.getSceneMax();
			float offset = (max.y - min.y) / 2.0;
			rover.setPosition(intersectPoint.x, intersectPoint.y - offset, intersectPoint.z);

			// set up bounding box for rover while we are at it
			//
			//roverBounds = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));
		}
	}
	

}


//  intersect the mouse ray with the plane normal to the camera 
//  return intersection point.   (package code above into function)
//
glm::vec3 ofApp::getMousePointOnPlane() {
	// Setup our rays
	//
	glm::vec3 origin = theCam->getPosition();
	glm::vec3 camAxis = theCam->getZAxis();
	glm::vec3 mouseWorld = theCam->screenToWorld(glm::vec3(mouseX, mouseY, 0));
	glm::vec3 mouseDir = glm::normalize(mouseWorld - origin);
	float distance;

	bool hit = glm::intersectRayPlane(origin, mouseDir, glm::vec3(0, 0, 0), camAxis, distance);

	if (hit) {
		// find the point of intersection on the plane using the distance 
		// We use the parameteric line or vector representation of a line to compute
		//
		// p' = p + s * dir;
		//
		glm::vec3 intersectPoint = origin + distance * mouseDir;
		
		return intersectPoint;
	}
	else return glm::vec3(0, 0, 0);
}
