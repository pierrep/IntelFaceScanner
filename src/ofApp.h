#pragma once

#include "ofMain.h"
#include "IntelFaceScanner.h"
#include "ofxPCL.h"
#include "ofxParticle3D.h"
#include "MeshLoader.h"

class ofApp : public ofBaseApp{

	public:

		enum State {IDLE,SETUP,RESET,PREVIEW,SCANNING,PROCESS,LOADING,LOADED,RENDER};

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

		void setupScanner();
		void cleanupScanner();
		string getStateAsString();
		void loadPointCloud();
		void renderFace();
		void renderFaces();
		void onScanningStarted();
		void onScanningDone();
		void onMeshLoaded();

		IntelFaceScanner*	scanner;

		ofEasyCam cam;
		ofVboMesh mesh, meshraw;
		bool dispRaw;

		vector<ofxParticle3D> particles;

		State state;

		/* UI */
		ofImage	logo;
		ofImage	spinner;
		ofTrueTypeFont font;
		bool	bDebugInfo;

		MeshLoader meshLoader;
		pcl::PolygonMesh* polymesh;

		ofFbo fbo;


		float curTime;
		float prevTime;
		float curFrame;
		float prevFrame;

		struct Face {
				int x;
				int y;
				ofMesh mesh;
		};

		vector<Face> faces;
		bool bDrawWireFrame;
};
