#include "ofApp.h"

ofApp::~ofApp()
{
	ofRemoveListener(scanner->scanningStartedEvent,this,&ofApp::onScanningStarted);
	ofRemoveListener(scanner->scanningDoneEvent,this,&ofApp::onScanningDone);
	delete scanner;
	scanner = NULL;
}
//--------------------------------------------------------------
void ofApp::setup(){	
	state = PREVIEW;

	scanner = new IntelFaceScanner();
	scanner->setup();

	ofAddListener(scanner->scanningStartedEvent,this,&ofApp::onScanningStarted);
	ofAddListener(scanner->scanningDoneEvent,this,&ofApp::onScanningDone);

	scanner->startScan();
}

//--------------------------------------------------------------
void ofApp::update(){
	if(state == RESET) {
		ofRemoveListener(scanner->scanningStartedEvent,this,&ofApp::onScanningStarted);
		ofRemoveListener(scanner->scanningDoneEvent,this,&ofApp::onScanningDone);
		delete scanner;
		scanner = NULL;
		setup();
		state = PREVIEW;
		//scanner->startScan();
	}

	//if(state == SCANNING) {
	//	curTime = ofGetElapsedTimeMillis();
	//	if((curTime - prevTime) > 5000) {
	//		scanner->stopScan();
	//		state = IDLE;
	//	}				
	//}

	if(scanner) {
		scanner->update();
	}

	if(state == PROCESS) 
	{
		loadPointCloud();
		state = RENDER;
	}
}

//--------------------------------------------------------------
void ofApp::draw(){

	if(state == RENDER) {
		ofBackground(0);
	
		cam.begin();
		ofTranslate(0,0,750);
		ofScale(1000, 1000, 1000);
		ofRotateX(-90);
		ofRotateZ(2);
		glEnable(GL_DEPTH_TEST);
	
		if( dispRaw ) {
			meshraw.drawVertices();
		} else {
			mesh.drawVertices();
		}
	
		cam.end();
	} else if(state == PROCESS) {		
		ofDrawBitmapString("LOADING POINTS...",ofGetWidth()/2,ofGetHeight()/2);
	}
	else if((state == PREVIEW) || (state == SCANNING)) {
		//scanner->draw(0,0);
		scanner->draw(0,0,ofGetWidth(),ofGetHeight());
	}

	ofDrawBitmapString(getStateAsString(),20,ofGetHeight()-80);
	ofDrawBitmapString(ofToString(ofGetFrameRate()),20,ofGetHeight()-40);
}

//--------------------------------------------------------------
void ofApp::onScanningStarted()
{
	cout << "scan started" << endl;
	//curTime = prevTime = ofGetElapsedTimeMillis();
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

//--------------------------------------------------------------
void ofApp::loadPointCloud()
{
	dispRaw = false;
	
	ofxPCL::PointCloud cloud(new ofxPCL::PointCloud::element_type);
	
	pcl::PolygonMesh polymesh;
	cout << "loading PLY file..." << endl;
	int result = pcl::io::load( ofToDataPath("Pierre3dscan.ply"),polymesh);
	cout << "File load result = " << result << endl;

	pcl::fromPCLPointCloud2(polymesh.cloud,*cloud);

	//cloud = ofxPCL::loadPointCloud<ofxPCL::PointCloud>(string("table_scene_lms400.pcd"));
	//cloud = ofxPCL::loadPointCloud<ofxPCL::PointCloud>(string("bun0.pcd"));
	
	//meshraw = ofxPCL::toOF(cloud);
	
	std::cerr << "PointCloud before filtering: " << cloud->width * cloud->height 
	<< " data points (" << pcl::getFieldsList (*cloud) << ")." << endl;
	
	//ofxPCL::downsample(cloud, ofVec3f(0.005f, 0.005f, 0.005f));
	
	std::cerr << "PointCloud after filtering: " << cloud->width * cloud->height 
	<< " data points (" << pcl::getFieldsList (*cloud) << ")." << endl;
	
	//ofxPCL::savePointCloud("table_scene_lms400_downsampled.pcd", cloud);

	mesh = ofxPCL::toOF(cloud);
	cloud.reset();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if(key == ' ') {
		if(state == RENDER) {
			state = RESET;
		}
		else if(state == PREVIEW) {
			//scanner->startScan();
		} 
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
	if(state == IDLE) return "Idle";
	if(state == PREVIEW) return "Preview";
	if(state == SETUP) return "Setup";
	if(state == SCANNING) return "Scanning";
	if(state == PROCESS) return "Process";
	if(state == RENDER) return "Render";
	return "NullState";
}
