#include "IntelFaceScanner.h"


#include "pxcsensemanager.h"
#include "pxc3dscan.h"

IntelFaceScanner::IntelFaceScanner() {

	session = 0;
	senseManager = 0;
	bSenseManagerInited = false;
	frameCounter = 0;
	
	m_charBuffer = NULL;
	
	m_charBuffer = new unsigned char[640*480*4];

	newFrame = false;
	bViewFrame = true;

	bDoRender = false;
	bDoScan = false;
}

//--------------------------------------------------------------
IntelFaceScanner::~IntelFaceScanner()
{
	scanningData.close();
	
	waitForThread(true);

	close();
}

//--------------------------------------------------------------
void IntelFaceScanner::close()
{
	if(scanner) {
		//scanner->Release();
		//scanner = NULL;
	}

	if(bSenseManagerInited) {
		senseManager->Close();
		senseManager->Release();
		senseManager = NULL;
	}

	if(session) {
		session->Release();
		session = NULL;
	}


}

//--------------------------------------------------------------
void IntelFaceScanner::open() 
{

	session=PXCSession::CreateInstance();
	if (!session) {
       ofLogError("IntelFaceScanner") << "Failed to create an SDK session, aborting.";
       std::exit(1);
    } else {
		ofLogNotice("IntelFaceScanner") << "Created an Intel Realsense SDK session";
	}

	senseManager = PXCSenseManager::CreateInstance();

	if(!senseManager)
	{
		ofLogError("IntelFaceScanner") << "Failed to create SenseManager, exiting.";
		return;
	} else {
		ofLogNotice("IntelFaceScanner") << "Created a SenseManager";
	}

	// Enable the 3D Capture video module
    pxcStatus result = senseManager->Enable3DScan();
    if (result < PXC_STATUS_NO_ERROR)
    {
        ofLogError("IntelFaceScanner") << "Error: Enable3DScan failed (" << result << ")";
		return;
    }

	// Initialize the pipeline
	pxcStatus status = senseManager->Init();
	if (status < PXC_STATUS_NO_ERROR) {
		ofLogError("IntelFaceScanner") << "Sense Manager init failed....exiting";
		return;
	} else {
		ofLogNotice("IntelFaceScanner") << "Pipeline initialised.";
		bSenseManagerInited = true;
	}

	PXCCapture::DeviceInfo device_info;
	//PXCCapture::Device *device = senseManager->QueryCaptureManager()->QueryDevice();   
 //   senseManager->QueryCaptureManager()->QueryDevice()->QueryDeviceInfo(&device_info);
	//if(device_info.model == PXCCapture::DEVICE_MODEL_IVCAM)
	//{
	//	device->SetDepthConfidenceThreshold(1);
	//	device->SetMirrorMode(PXCCapture::Device::MIRROR_MODE_DISABLED);
	//	device->SetIVCAMFilterOption(6);
	//}

	// Get camera info
	maxRangeValue  = senseManager->QueryCaptureManager()->QueryDevice()->QueryDepthSensorRange().max;
    senseManager->QueryCaptureManager()->QueryCapture()->QueryDeviceInfo(0, &device_info);
    ofLogNotice("IntelFaceScanner") << "Camera: "<< device_info.name << " Firmware: "<<  device_info.firmware[0] <<"."<<  device_info.firmware[1] << "." << device_info.firmware[2] << "." <<  device_info.firmware[3];

	// Get Mesh Capture module instance (do not release, resources handled by SenseManager)
	scanner = senseManager->Query3DScan();
    if (!scanner) {
		ofLogError("IntelFaceScanner") << "3DScan module unavailable";
		return;
	}

	// Report the resulting profile
  //  {
  //      PXCVideoModule::DataDesc VideoProfile;
  //      result = scanner->QueryInstance<PXCVideoModule>()->QueryCaptureProfile(PXCBase::WORKING_PROFILE, &VideoProfile);
  //      if (result < PXC_STATUS_NO_ERROR) {
		//	ofLogError("IntelFaceScanner") << "Failed to capture profile, exiting";
		//	return;
		//}
  //      else
  //      {
		//	ofLogNotice("IntelFaceScanner") << "Colour width: " << VideoProfile.streams.color.sizeMax.width;
		//	ofLogNotice("IntelFaceScanner") << "Colour height: " << VideoProfile.streams.color.sizeMax.height;
		//	ofLogNotice("IntelFaceScanner") << "Colour framerate: " << VideoProfile.streams.color.frameRate.max;
		//	ofLogNotice("IntelFaceScanner") << "Depth width: " << VideoProfile.streams.depth.sizeMax.width;
		//	ofLogNotice("IntelFaceScanner") << "Depth height: " << VideoProfile.streams.depth.sizeMax.height;
		//	ofLogNotice("IntelFaceScanner") << "Depth framerate: " << VideoProfile.streams.depth.frameRate.max;
  //      }
  //  }

	// Configure the system
    PXC3DScan::Configuration config = scanner->QueryConfiguration();
	config.mode = PXC3DScan::FACE;
	//config.options = config.options | PXC3DScan::SOLIDIFICATION;
	config.minFramesBeforeScanStart = 50;

    result = scanner->SetConfiguration(config);
    if (result != PXC_STATUS_NO_ERROR)
    {
        ofLogError("IntelFaceScanner") << "Error: scanner->SetConfiguration() failed. Error = " << result;
		return;
    }

	scanningFramesRemaining = 50;

}

//--------------------------------------------------------------
void IntelFaceScanner::setup()
{

	open();

	bDeviceConnected = true;	
	ofLogNotice("IntelFaceScanner") << "Device connected, starting thread.";
	startThread();
}

//--------------------------------------------------------------
void IntelFaceScanner::startScan() {
	//lock();
	//	bDoScan = true;
	//unlock();

}

//--------------------------------------------------------------
void IntelFaceScanner::stopScan() {
	//lock();
	//	bDoRender = true;
	//unlock();

}

void IntelFaceScanner::renderScan()
{
  // Setup the output path to a writable location.
    size_t unused;
    WCHAR* pUserProfilePath;
    _wdupenv_s(&pUserProfilePath, &unused, L"USERPROFILE");
    const PXC3DScan::FileFormat format = PXC3DScan::PLY; // OBJ, PLY or STL
    const pxcCHAR* ext = PXC3DScan::FileFormatToString(format);
    const size_t FSIZE = 4096;
    WCHAR filename[FSIZE];
    //swprintf_s(filename, FSIZE, L"%s\\Documents\\Pierre3dscan.%s", pUserProfilePath, ext);
    swprintf_s(filename, FSIZE, L".\\data\\Pierre3dscan.%s", ext);

    // If applicable, reconstruct the 3D Mesh to the specific file/format
	pxcStatus result = PXC_STATUS_NO_ERROR;
    bool bMeshSaved = false;
    if (scanner->IsScanning())
    {
        wprintf_s(L"Generating %s...", filename);
        result = scanner->Reconstruct(format, filename);
        if (result >= PXC_STATUS_NO_ERROR)
        {
            bMeshSaved = true;
            wprintf_s(L"done.\n");
        }
		else if (result == PXC_STATUS_FILE_WRITE_FAILED)
        {
            wprintf_s(L"the file could not be created using the provided path. Aborting.\n");
        }
        else if (result == PXC_STATUS_ITEM_UNAVAILABLE || result == PXC_STATUS_DATA_UNAVAILABLE)
        {
            wprintf_s(L"no scan data found. Aborting.\n");
        }
        else if (result < PXC_STATUS_NO_ERROR)
        {
            wprintf_s(L"\nError: Reconstruct returned %d\n", result);
        }
    }
    else wprintf_s(L"Empty scanning volume.\n");
}

//--------------------------------------------------------------
void IntelFaceScanner::threadedFunction(){
	pxcStatus result;

	while ((scanningFramesRemaining) && (isThreadRunning()))
	{

		pxcStatus status = senseManager->AcquireFrame(true);
		if (checkDeviceConnection(senseManager->IsConnected())) 
		{
			if (status < PXC_STATUS_NO_ERROR)  {				
				ofLogError() << "Error with Sensemanager...! status=" << status;
				return;
			}
			frameCounter++;

		}

		if (scanner->IsScanning())
        {
			if(!bDoScan) {
				bDoScan = true;
				ofNotifyEvent(scanningStartedEvent,this);
				ofLogNotice("IntelFaceScanner") << "Scanning started... frame:" << frameCounter;
			}
            scanningFramesRemaining--;
		}
		// Get the preview image for this frame
        PXCImage* preview_image = scanner->AcquirePreviewImage();

		senseManager->ReleaseFrame();

		if(preview_image) {
			updateBitmap(preview_image);
			preview_image->Release();
		}

	}


	ofLogNotice("IntelFaceScanner") << "Rendering scan... frame:" << frameCounter;
	renderScan();
	ofLogNotice("IntelFaceScanner") << "Scanning stopped... frame:" << frameCounter;
	curframeCounter = frameCounter;
	ofNotifyEvent(scanningDoneEvent,this);

}

//--------------------------------------------------------------
void IntelFaceScanner::update()
{
	// check if there's a new analyzed frame and upload
	// it to the texture. we use a while loop to drop any
	// extra frame in case the main thread is slower than
	// the analysis
	// tryReceive doesn't reallocate or make any copies
	newFrame = false;
	ScanningData data;
	while(scanningData.tryReceive(data)) {
		newFrame = true;
	}

	if(newFrame) {
		if( (!texture.isAllocated()) || (texture.getWidth() != data.frame.getWidth()) ) {
			texture.allocate(data.frame.getWidth(),data.frame.getHeight(),GL_LUMINANCE);
		}
		texture.loadData(data.frame.getPixels());
	}

}

//--------------------------------------------------------------
void IntelFaceScanner::draw(float x, float y) 
{
	texture.draw(x,y,640,480);
}

//--------------------------------------------------------------
void IntelFaceScanner::draw(float x, float y, float w, float h) 
{
	if(texture.isAllocated()) {
		texture.draw(x,y,w,h);
	}
}

//--------------------------------------------------------------
bool IntelFaceScanner::checkDeviceConnection(bool state) {
    if (state) {
        if (!bDeviceConnected) {
			ofLogNotice() << "Device Reconnected";
		}
		bDeviceConnected = true;
    } else {
        if (bDeviceConnected) {
			ofLogNotice() << "Device Disconnected";
		}
		bDeviceConnected = false;
    }
    return bDeviceConnected;
}

//--------------------------------------------------------------
void IntelFaceScanner::updateBitmap(PXCImage *newimage) 
{
	if(!newimage) return;

	PXCImage* image = newimage;
	PXCImage::ImageInfo info=image->QueryInfo();
    PXCImage::ImageData data;

	if(info.format == PXCImage::PIXEL_FORMAT_RGB32)
	{
		if (image->AcquireAccess(PXCImage::ACCESS_READ,PXCImage::PIXEL_FORMAT_RGB32, &data)>=PXC_STATUS_NO_ERROR) 
		{
			for(int i=0; i < info.height;i++) {
				memcpy_s((char*)(m_charBuffer+i*data.pitches[0]),info.width*4,data.planes[0]+i*data.pitches[0],info.width*4);
			}
			frame.setFromPixels(m_charBuffer, info.width, info.height, OF_IMAGE_COLOR_ALPHA, true);
			
			ScanningData fdata(frame);
			scanningData.send(fdata);

			image->ReleaseAccess(&data);
		}
	}
}