#include "ofApp.h"

#include "ofxSquash.h"

//--------------------------------------------------------------
void ofApp::setup() {

	this->gui.init();

	//initialise sender
	{
		auto port = 4444;
		string ipAddress = "127.0.0.1";
		
		auto result = ofSystemTextBoxDialog("Select remote IP (Default is " + ipAddress + ")");
		if (!result.empty()) {
			ipAddress = result;
		}

		this->sender.init(ipAddress, port);
		ofSetWindowTitle("Sending to : " + ipAddress + ":" + ofToString(port));

		auto sources = this->sender.getKinect().getSources();
		for (auto source : sources) {
			auto imageSource = dynamic_pointer_cast<ofBaseHasTexture>(source);
			if (imageSource) {
				auto panel = ofxCvGui::makePanel(imageSource->getTexture(), source->getTypeName());
				this->gui.add(panel);

// 				if (dynamic_pointer_cast<ofxKinectForWindows2::Source::Depth>(source)) {
// 					//if it's the depth source
// 					auto style = make_shared<ofxCvGui::Panels::Texture::Style>();
// 					style->shader = ofxAssets::AssetRegister().getShaderPointer("ofxCvGui::KinectForWindows2Depth");
// 					panel->setStyle(style);
// 				}
			}
			
		}
	}

	ofSetFrameRate(60);
}

//--------------------------------------------------------------
void ofApp::update(){
	this->sender.update();
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
