#include "ofApp.h"

#include "ofxSquash.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(30);

	this->gui.init();
	this->bigGui = gui.getController().getRootGroup();
	this->guiController = & gui.getController();

	auto widgets = this->gui.addWidgets();
	widgets->addTitle("MultiTrack Publish");
	widgets->addFps();
	widgets->addEditableValue<float>("App framerate [Hz]", []() {
		return ofGetFrameRate();
	}, [](string result) {
		if (!result.empty()) {
			ofSetFrameRate(ofToFloat(result));
		}
	});
	widgets->addMemoryUsage();
	widgets->addLiveValueHistory("Kinect device framerate", [this]() {
		return this->publisher.getDeviceFrameRate();
	});
	widgets->addLiveValueHistory("Sending framerate", [this]() {
		return this->publisher.getPublisher().getSendFramerate();
	});
	widgets->addLiveValueHistory("Dropped frame", [this]() {
		return (float) this->droppedFrame;
	});
	widgets->addEditableValue<int>("Packet size", [this]() {
		return (int) this->publisher.getPublisher().getPacketSize();
	}, [this](string newSizeString) {
		if (!newSizeString.empty()) {
			this->publisher.getPublisher().setPacketSize(ofToInt(newSizeString));
		}
	});
	widgets->addLiveValue<size_t>("Current compressor queue size", [this]() {
		return this->publisher.getPublisher().getCurrentCompressorQueueSize();
	});
	widgets->addLiveValue<size_t>("Current socket buffer size", [this]() {
		return this->publisher.getPublisher().getCurrentSocketBufferSize();
	});
	widgets->addEditableValue<int>("Max socket buffer size", [this]() {
		return (int) this->publisher.getPublisher().getMaxSocketBufferSize();
	}, [this](string newSizeString) {
		if (!newSizeString.empty()) {
			this->publisher.getPublisher().setMaxSocketBufferSize(ofToInt(newSizeString));
		}
	});
	widgets->addButton("Save Depth To World Table", [this]() {
		string filename = "DepthToWorld.raw";
		{
			auto result = ofSystemTextBoxDialog("Enter filename [" + filename + "]");
			if (!result.empty()) {
				filename = result;
			}

			{
				ofFloatPixels depthToWorldTable;
				kinect->getDepthSource()->getDepthToWorldTable(depthToWorldTable);
				ofxSquashBuddies::Message message;
				message.setData(depthToWorldTable);
				const auto & saveString = message.getMessageString();
				ofBuffer saveBuffer(saveString);
				if (!ofBufferToFile(ofToDataPath(filename), saveBuffer)) {
					ofSystemAlertDialog("Error saving table to disk!");
				}
			}
		}
	});
	widgets->addToggle("Minimise GUI",
		[this]() {
		return guiMinimised;
	}, [this](bool minimise) {
		this->setGuiMinimised(minimise);
	});

	bool autoStart = true;

	//setup sender
	{
		auto port = 5000;

		//initialise the Kinect
		this->kinect = make_shared<ofxKinectForWindows2::Device>();
		this->kinect->open();
		this->kinect->initColorSource();
		this->kinect->initDepthSource();
		this->kinect->initInfraredSource();
		this->kinect->initBodyIndexSource();
		this->kinect->initBodySource();

		//initialise the ofxMultiTrack::Sender
		this->publisher.init(this->kinect, port);

		//build the gui
		ofSetWindowTitle("Publishing on : " + ofToString(port));
		auto sources = this->kinect->getSources();
		for (auto source : sources) {
			auto imageSource = dynamic_pointer_cast<ofBaseHasTexture>(source);
			if (imageSource) {
				auto panel = ofxCvGui::Panels::makeTexture(imageSource->getTexture(), source->getTypeName());

				if (dynamic_pointer_cast<ofxKinectForWindows2::Source::Depth>(source)) {
					panel->setInputRange(0, 8000.0f / (float)0xffff);
				}
				if (dynamic_pointer_cast<ofxKinectForWindows2::Source::Infrared>(source)) {
					panel->setInputRange(0, 1024.0f / (float)0xffff);
				}
				this->gui.add(panel);
			}
		}
	}

	//make a small gui for when we're 'minimised'
	this->smallGui = ofxCvGui::Panels::Groups::makeGrid();
	this->smallGui->add(widgets);
	this->setGuiMinimised(autoStart); //if autostart, then start minimised
}

//--------------------------------------------------------------
void ofApp::setGuiMinimised(bool minimised) {
	this->guiMinimised = minimised;
	if (minimised) {
		this->kinect->setUseTextures(false);
		this->guiController->setRootGroup(this->smallGui);
		ofSetWindowShape(300, 500);
	}
	else {
		this->kinect->setUseTextures(true);
		this->guiController->setRootGroup(this->bigGui);
		ofSetWindowShape(640 * 3, 480 * 2);
	}
}

//--------------------------------------------------------------
void ofApp::update(){
	this->kinect->update();
	this->droppedFrame = ! this->publisher.update();
}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	
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
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
