#include "ofApp.h"

#define SCREEN_EDGE (2160+540)

ofApp::~ofApp()
{
	if(scanner) {
		cleanupScanner();
	}
}
//--------------------------------------------------------------
void ofApp::setup(){	
	ofBackground(50,50,50);
	cam.enableOrtho();
	//cam.setOrientation(ofVec3f(0,-180,0));
	fbo.allocate(ofGetWidth(),ofGetHeight());

	scanner = NULL;
	logo.load("Intel-logo.png");
	logo.setAnchorPercent(0.5,0.5);
	spinner.load("loading.png");
	spinner.setAnchorPercent(0.5,0.5);
	font.load("DINNeuzeitGroteskStd-Light.otf",28);
	ofAddListener(meshLoader.meshLoadedEvent,this,&ofApp::onMeshLoaded);
	polymesh = NULL;
	bDebugInfo = false;
	state = SETUP;
}

//--------------------------------------------------------------
void ofApp::setupScanner(){
	scanner = new IntelFaceScanner();
	scanner->setup();

	ofAddListener(scanner->scanningStartedEvent,this,&ofApp::onScanningStarted);
	ofAddListener(scanner->scanningDoneEvent,this,&ofApp::onScanningDone);
}

//--------------------------------------------------------------
void ofApp::cleanupScanner(){
	ofRemoveListener(scanner->scanningStartedEvent,this,&ofApp::onScanningStarted);
	ofRemoveListener(scanner->scanningDoneEvent,this,&ofApp::onScanningDone);
	delete scanner;
	scanner = NULL;
}

//--------------------------------------------------------------
void ofApp::update(){
	for(int i=0;i < faces.size();i++)
	{
		faces[i].x += 1;
		if(faces[i].x > SCREEN_EDGE) 
		{
			faces.erase(faces.begin() + i);
		}
	}

	if(state == RESET) {
		cleanupScanner();
		setupScanner();
		if(polymesh) delete polymesh;
		state = PREVIEW;
	}
	else if(state == PROCESS) {
		polymesh = new pcl::PolygonMesh();
		meshLoader.loadFromDisk(*(polymesh),"3dscan.ply");
		state = LOADING;
	}
	else if(state == LOADED) 
	{
		loadPointCloud();
		state = RENDER;
	}
	else if(state == RENDER) {
		for (int i=0; i<mesh.getVertices().size(); i++) {
			

		}
	}

	if(scanner) {
		scanner->update();
	}

}

//--------------------------------------------------------------
void ofApp::draw()
{
	ofBackground(0);
	renderFaces();

	if(state == RENDER) 
	{
		//if(captureFace) 
		//{
		//	curFrame = ofGetFrameNum();
		//	if((curFrame - prevFrame) < 3)
		//	{
		//		fbo.begin();
		//			ofBackground(0);
		//			renderFace();
		//		fbo.end();
		//		
		//		//ofPixels p;
		//		//fbo.readToPixels(p);
		//		//ofSaveImage(p,"screeny" + ofToString(ofGetFrameNum()) + ".png");
		//	}
		//	else {
		//		captureFace = false;
		//	}
		//}
		//fbo.draw(0,0);
	}
	
	if(state == SETUP)
	{
		if(ofGetFrameNum() < 300) {
			logo.draw(ofGetWidth()/2.0f,ofGetHeight()/2.0f);
		}
		else
		{
			string s = "PRESS SPACEBAR TO BEGIN SCANNING";
			float w = font.stringWidth(s);
			font.drawString(s,ofGetWidth()/2.0f - w/2.0f,ofGetHeight()/2.0f);
		}
	} 
	else if((state == SCANNING) || (state == PROCESS) || (state == LOADING)) {		
		ofBackground(50,50,50);
		ofPushMatrix();
		ofTranslate(ofGetWidth()/2.0f,ofGetHeight()/2.0f);
		ofRotate(ofGetFrameNum() * 5.0f, 0, 0, 1);//rotate from centre
		spinner.draw(0,0);
		ofPopMatrix();
		//string s = "LOADING SCAN";
		//float w = font.stringWidth(s);
		//font.drawString(s,ofGetWidth()/2 - w/2.0f,ofGetHeight()/2);
	}
	else if((state == PREVIEW) || (state == SCANNING)) {
		scanner->draw(0,0,ofGetWidth(),ofGetHeight());
	}

	if(bDebugInfo) {
		ofDrawBitmapString(getStateAsString(),20,ofGetHeight()-80);
		ofDrawBitmapString("Num Faces = "+ofToString(faces.size()),20,ofGetHeight()-60);
		ofDrawBitmapString(ofToString(ofGetFrameRate()),20,ofGetHeight()-40);
	}

}

//--------------------------------------------------------------
void ofApp::onMeshLoaded() 
{
	if(state == LOADING) {
		state = LOADED;
	}
}

//--------------------------------------------------------------
void ofApp::onScanningStarted()
{
	cout << "scan started" << endl;
	state = SCANNING;
}

//--------------------------------------------------------------
void ofApp::onScanningDone()
{
	cout << "scan done" << endl;
	mesh.clear();
	meshraw.clear();
	state = PROCESS;
}

void ofApp::renderFaces()
{
	cam.begin();
	for(int i=0;i < faces.size();i++) {
		ofPushMatrix();
		ofTranslate(faces[i].x,faces[i].y,1100);
		ofScale(5000, 5000, 5000);
		ofRotateX(-90);
		glEnable(GL_DEPTH_TEST);
		faces[i].mesh.draw(OF_MESH_POINTS);
		ofPopMatrix();
	}
	cam.end();
}

void ofApp::renderFace()
{
	cam.begin();
	ofPushMatrix();
	ofTranslate(0,0,1100);
	ofScale(1000, 1000, 1000);
	ofRotateX(-90);
	glEnable(GL_DEPTH_TEST);
	
	if( dispRaw ) {
		meshraw.drawVertices();
	} else {
		mesh.drawVertices();
	}
	ofPopMatrix();
	cam.end();
}

//--------------------------------------------------------------
void ofApp::loadPointCloud()
{
	dispRaw = false;
	
	ofxPCL::PointCloud cloud(new ofxPCL::PointCloud::element_type);
	
	//pcl::PolygonMesh polymesh2;
	//cout << "loading PLY file..." << endl;
	//int result = pcl::io::load( ofToDataPath("Pierre3dscan.ply"),polymesh2);
	//cout << "File load result = " << result << endl;

	pcl::fromPCLPointCloud2(polymesh->cloud,*cloud);
	
	//meshraw = ofxPCL::toOF(cloud);
	
	std::cerr << "PointCloud before filtering: " << cloud->width * cloud->height 
	<< " data points (" << pcl::getFieldsList (*cloud) << ")." << endl;
	
	//ofxPCL::downsample(cloud, ofVec3f(0.005f, 0.005f, 0.005f));
	//ofxPCL::statisticalOutlierRemoval(cloud, 50, 1.0);

	std::cerr << "PointCloud after filtering: " << cloud->width * cloud->height 
	<< " data points (" << pcl::getFieldsList (*cloud) << ")." << endl;
	
	//ofxPCL::savePointCloud("table_scene_lms400_downsampled.pcd", cloud);
	Face newface;
	newface.x = 0;
	newface.y = 0;
	newface.mesh = ofxPCL::toOF(cloud);

	newface.mesh.getColors().resize(newface.mesh.getNumVertices());

	float min = 5.0;
	for(int i = 0; i < newface.mesh.getNumVertices();i++) 
	{
		float depth = newface.mesh.getVertex(i).y;
		/*cout << "depth = " << depth;*/
		if(depth < min) min = depth;
		depth = ofMap(depth,0.4,0.5,1,0);
		newface.mesh.getColors()[i].set(ofFloatColor(depth,depth,depth));
	}
	cout << "min =" << min << endl;
	//newface.mesh.setUsage( GL_DYNAMIC_DRAW );
	newface.mesh.setMode(OF_PRIMITIVE_POINTS);
	newface.mesh.enableColors();

	faces.push_back(newface);
	cloud.reset();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if(key == ' ') {
		if(state == RENDER) {
			state = RESET;
		}
		else if(state == SETUP) {
			setupScanner();
			state = PREVIEW;
		}
	}
	if(key == 'd') {
		bDebugInfo = !bDebugInfo;
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

string ofApp::getStateAsString() 
{
	if(state == IDLE) return "IDLE";
	if(state == PREVIEW) return "PREVIEW";
	if(state == SETUP) return "SETUP";
	if(state == SCANNING) return "SCANNING";
	if(state == PROCESS) return "PROCESS";
	if(state == LOADING) return "LOADING";
	if(state == LOADED) return "LOADED";
	if(state == RENDER) return "RENDER";
	return "NullState";
}
