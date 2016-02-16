#include "ofApp.h"

#include "ofxSquash.h"

//--------------------------------------------------------------
void ofApp::setup() {

	this->gui.init();

	//initialise receiver
	{
		auto port = 4444;
		string ipAddress = "127.0.0.1";

		this->receiver.init(port);

		this->gui.add(ofxCvGui::makePanel(this->previewColor, "Color"));
		this->gui.add(ofxCvGui::makePanel(this->previewDepth, "Depth"));
		this->gui.add(ofxCvGui::makePanel(this->previewInfrared, "Infrared"));
		this->gui.add(ofxCvGui::makePanel(this->previewBodyIndex, "BodyIndex"));
		this->gui.add(ofxCvGui::makePanel(this->previewColorCoordInDepthFrame, "ColorCoordInDepthView"));
	}

	ofSetFrameRate(60);
}

//--------------------------------------------------------------
void ofApp::update(){
	this->receiver.update();
	if (this->receiver.isFrameNew()) {
		auto & frame = this->receiver.getFrame();
		this->previewColor.loadData(frame.color);
		this->previewDepth.loadData(frame.depth);
		this->previewInfrared.loadData(frame.infrared);
		this->previewBodyIndex.loadData(frame.bodyIndex);
		this->previewColorCoordInDepthFrame.loadData(frame.colorCoordInDepthFrame);
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
