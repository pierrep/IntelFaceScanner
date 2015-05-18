#pragma once

#include "ofMain.h"
#include "IntelFaceScanner.h"
#include "ofxPCL.h"

class ofApp : public ofBaseApp{

	public:

		enum State {IDLE,SETUP,RESET,PREVIEW,SCANNING,PROCESS,RENDER};

		~ofApp();
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		string getStateAsString();
		void loadPointCloud();
		
		void onScanningStarted();
		void onScanningDone();

		IntelFaceScanner*	scanner;

		ofEasyCam cam;
		ofVboMesh mesh, meshraw;
		bool dispRaw;

		State state;
		//float curTime;
		//float prevTime;
		
};
