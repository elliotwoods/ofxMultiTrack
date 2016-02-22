#include "ofApp.h"

#include "ofxSquash.h"

//--------------------------------------------------------------
void ofApp::setup() {

	this->gui.init();
	this->bigGui = gui.getController().getRootGroup();
	this->guiController = & gui.getController();

	auto widgets = this->gui.addWidgets();
	widgets->addTitle("MultiTrack Send");
	widgets->addFps();
	widgets->addLiveValueHistory("Kinect device framerate", [this]() {
		return this->node.getDeviceFrameRate();
	});
	widgets->addLiveValueHistory("Sending framerate", [this]() {
		return this->node.getSender().getSendFramerate();
	});
	widgets->addLiveValueHistory("Dropped frame", [this]() {
		return (float) this->droppedFrame;
	});
	widgets->addEditableValue<int>("Packet size", [this]() {
		return (int) this->node.getSender().getPacketSize();
	}, [this](string newSizeString) {
		if (!newSizeString.empty()) {
			this->node.getSender().setPacketSize(ofToInt(newSizeString));
		}
	});
	widgets->addLiveValue<size_t>("Current socket buffer size", [this]() {
		return this->node.getSender().getCurrentSocketBufferSize();
	});
	widgets->addEditableValue<int>("Max socket buffer size", [this]() {
		return (int) this->node.getSender().getMaxSocketBufferSize();
	}, [this](string newSizeString) {
		if (!newSizeString.empty()) {
			this->node.getSender().setMaxSocketBufferSize(ofToInt(newSizeString));
		}
	});
	widgets->addToggle("Minimise GUI",
		[this]() {
		return guiMinimised;
	}, [this](bool minimise) {
		this->setGuiMinimised(minimise);
	});

	//initialize sender
	{
		auto port = 4444;
		string ipAddress = "127.0.0.1";
		
		{
			auto result = ofSystemTextBoxDialog("Select remote IP (Default is " + ipAddress + ")");
			if (!result.empty()) {
				ipAddress = result;
			}
		}
		{
			auto result = ofSystemTextBoxDialog("Select remote port (Default is " + ofToString(port) + ")");
			if (!result.empty()) {
				port = ofToInt(result);
			}
		}

		this->node.init(ipAddress, port);
		ofSetWindowTitle("Sending to : " + ipAddress + ":" + ofToString(port));

		auto sources = this->node.getKinect().getSources();
		for (auto source : sources) {
			auto imageSource = dynamic_pointer_cast<ofBaseHasTexture>(source);
			if (imageSource) {
				auto panel = ofxCvGui::Panels::make(imageSource->getTexture(), source->getTypeName());
				this->gui.add(panel);
			}
			
		}
	}

	this->smallGui = ofxCvGui::Panels::Groups::makeGrid();
	this->smallGui->add(widgets);

	this->setGuiMinimised(false);

	ofSetFrameRate(60);
}

//--------------------------------------------------------------
void ofApp::setGuiMinimised(bool minimised) {
	this->guiMinimised = minimised;
	if (minimised) {
		this->node.setTexturesEnabled(false);
		this->guiController->setRootGroup(this->smallGui);
		ofSetWindowShape(300, 500);
	}
	else {
		this->node.setTexturesEnabled(true);
		this->guiController->setRootGroup(this->bigGui);
		ofSetWindowShape(640 * 3, 480 * 2);
	}
}

//--------------------------------------------------------------
void ofApp::update(){
	this->droppedFrame = ! this->node.update();
}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	auto shader = ofxAssets::AssetRegister().getShaderPointer("ofxCvGui::KinectForWindows2Depth");
	shader->reload();
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
