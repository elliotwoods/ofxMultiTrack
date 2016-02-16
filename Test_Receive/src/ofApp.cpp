#include "ofApp.h"

#include "ofxSquash.h"

//--------------------------------------------------------------
void ofApp::setup() {

	this->gui.init();
	
	auto widgets = this->gui.addWidgets();
	widgets->addTitle("MultiTrack Receive");
	widgets->addFps();
	widgets->addLiveValueHistory("Incoming frame rate", [this]() {
		return (float) this->receiver.getFrameRate();
	});
	widgets->addLiveValueHistory("Dropped frames", [this]() {
		return (float) this->receiver.getReceiver().getDroppedFrames().size();
	});

	//initialise receiver
	{
		auto port = 4444;

		while (!this->receiver.init(port)) {
			port++;
		}

		this->clientIndex = port - 4444;
		widgets->addLiveValue<float>("Client index", [this]() {
			return this->clientIndex;
		});

		this->gui.add(ofxCvGui::Panels::make(this->color.texture, "Color"));
		this->gui.add(ofxCvGui::Panels::make(this->depth.texture, "Depth"));
		this->gui.add(ofxCvGui::Panels::make(this->infrared.texture, "Infrared"));
		this->gui.add(ofxCvGui::Panels::make(this->bodyIndex.texture, "BodyIndex"));
		this->gui.add(ofxCvGui::Panels::make(this->colorCoordInDepthFrame.texture, "ColorCoordInDepthView"));
	}


	//initialise texture sharing
	{
		auto suffix = ofToString(this->clientIndex);

		this->color.sender.init("color" + suffix);
		this->depth.sender.init("depth" + suffix);
		this->infrared.sender.init("infrared" + suffix);
		this->bodyIndex.sender.init("bodyIndex" + suffix);
		this->colorCoordInDepthFrame.sender.init("colorCoordInDepthFrame" + suffix);
	}


	ofSetFrameRate(60);
}

//--------------------------------------------------------------
void ofApp::update(){
	this->receiver.update();
	if (this->receiver.isFrameNew()) {
		auto & frame = this->receiver.getFrame();

		//load data in cast format
		{
			//YUY8 -> RGBA8
			this->color.texture.loadData((unsigned char *) frame.color.getData(), frame.color.getWidth() / 2, frame.color.getHeight(), GL_RGBA);
			this->color.send();

			//L16 -> RGBA8
			this->depth.texture.loadData((unsigned char *)frame.depth.getData(), frame.depth.getWidth() / 2, frame.depth.getHeight(), GL_RGBA);
			this->depth.send();

			//L16 -> RGBA8
			this->infrared.texture.loadData((unsigned char *)frame.infrared.getData(), frame.infrared.getWidth() / 2, frame.infrared.getHeight(), GL_RGBA);
			this->infrared.send();

			//L8 -> L8
			this->bodyIndex.texture.loadData(frame.bodyIndex);
			this->bodyIndex.send();

			//RG16 -> RGBA8
			this->colorCoordInDepthFrame.texture.loadData((unsigned char *)frame.colorCoordInDepthFrame.getData(), frame.colorCoordInDepthFrame.getWidth(), frame.colorCoordInDepthFrame.getHeight(), GL_RGBA);
			this->colorCoordInDepthFrame.send();
		}
	}
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
