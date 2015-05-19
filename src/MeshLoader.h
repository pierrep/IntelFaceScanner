#pragma once

#include "ofThread.h"
#include "ofTypes.h" 
#include "ofThreadChannel.h"
#include "ofxPCL.h"

using namespace std;

class MeshLoader : public ofThread {
public:
    MeshLoader();
    ~MeshLoader();

	void loadFromDisk(pcl::PolygonMesh& mesh, string file);

	ofEvent<void>		meshLoadedEvent;

private:
	void update(ofEventArgs & a);
    virtual void threadedFunction();

    
    // Entry to load.
    struct ofMeshLoaderEntry {
        ofMeshLoaderEntry() {
            mesh = NULL;
        }
        
        ofMeshLoaderEntry(pcl::PolygonMesh & pMesh) {
            mesh = &pMesh;
        }
        pcl::PolygonMesh* mesh;
        string filename;
        string name;
    };


    typedef map<string, ofMeshLoaderEntry>::iterator entry_iterator;

	int                 nextID;
    int                 lastUpdate;

	ofThreadChannel<ofMeshLoaderEntry> meshes_to_load_from_disk;
	ofThreadChannel<ofMeshLoaderEntry> meshes_to_update;
};


