#pragma once
#include "ofMain.h"

#include "pxcsession.h"

class PXCSenseManager;
class PXC3DScan;

class IntelFaceScanner: public ofThread {
public:
	IntelFaceScanner();
	~IntelFaceScanner();

	void				setup();
	void				update();
	void				draw(float x, float y);
	void				draw(float x, float y, float w, float h);
	void				setupScanner();
	void				startScan();
	void				stopScan();
	void				renderScan();
	
	ofEvent<void>		scanningStartedEvent;
	ofEvent<void>		scanningDoneEvent;

protected:
	void				open();
	void				close();
	void				threadedFunction();
	void				updateBitmap(PXCImage *image);
	bool 				checkDeviceConnection(bool state);
	pxcCHAR* 			getDeviceString();

	bool				bSenseManagerInited;
	bool				bDeviceConnected;
	bool				newFrame;
	int					frameCounter;
	int					curframeCounter;
	bool				bDoRender;
	bool				bDoScan;
	int					scanningFramesRemaining;

private:
	pxcCHAR* 			device_name;
	PXCSession*			session;
	PXCSenseManager*	senseManager;
	PXC3DScan*			scanner;

	unsigned char		*m_charBuffer;

	struct ScanningData {
        ScanningData() {
            //frame = NULL;
        }
        
        ScanningData(ofImage & pImage) {
            frame = pImage;
        }
        ofImage frame;

        string name;
    };
	ofThreadChannel<ScanningData> scanningData;
	ofTexture			texture;
	ofImage				frame;
	bool				bViewFrame;
	float				maxRangeValue;

};
