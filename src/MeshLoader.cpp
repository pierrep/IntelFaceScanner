#include "MeshLoader.h"
#include <sstream>


MeshLoader::MeshLoader(){
	nextID = 0;
    ofAddListener(ofEvents().update, this, &MeshLoader::update);
    
    startThread();
    lastUpdate = 0;
}

MeshLoader::~MeshLoader(){
	meshes_to_load_from_disk.close();
	meshes_to_update.close();
	waitForThread(true);
    ofRemoveListener(ofEvents().update, this, &MeshLoader::update);
}

// Load a mesh from disk.
//--------------------------------------------------------------
void MeshLoader::loadFromDisk(pcl::PolygonMesh& mesh, string filename) {
	nextID++;
	ofMeshLoaderEntry entry(mesh);
	entry.filename = filename;
	entry.name = filename;
    
	meshes_to_load_from_disk.send(entry);
}


// Reads from the queue and loads new meshes.
//--------------------------------------------------------------
void MeshLoader::threadedFunction() {
	thread.setName("MeshLoader " + thread.name());
	ofMeshLoaderEntry entry;
	while( meshes_to_load_from_disk.receive(entry) ) {
		if(pcl::io::load( ofToDataPath(entry.filename),*(entry.mesh)) == 0 )  
		{
			ofLogNotice("MeshLoader") << "Mesh loaded correctly.";
			meshes_to_update.send(entry);
		}else
		{
			ofLogError("MeshLoader") << "couldn't load file: \"" << entry.filename << "\"";
		}
	}
	ofLogVerbose("MeshLoader") << "finishing thread on closed queue";
}


// Check the update queue
//--------------------------------------------------------------
void MeshLoader::update(ofEventArgs & a){
    // Load 1 mesh per update
	ofMeshLoaderEntry entry;
	if (meshes_to_update.tryReceive(entry)) {
		ofNotifyEvent(meshLoadedEvent,this);
	}
}

