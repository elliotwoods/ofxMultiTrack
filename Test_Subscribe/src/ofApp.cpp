#include "ofApp.h"

#include "ofxSquash.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(60);

	this->gui.init();
	
	auto widgets = this->gui.addWidgets();
	widgets->addTitle("MultiTrack Subscribe");
	widgets->addFps();
	widgets->addMemoryUsage();
	widgets->addIndicatorBool("Frame incoming", [this]() {
		return this->newFrame;
	});
	widgets->addLiveValueHistory("Receiver incoming frame rate", [this]() {
		return (float) this->subscriber.getSubscriber().getIncomingFramerate();
	});
	widgets->addLiveValueHistory("Dropped frames", [this]() {
		return (float) this->subscriber.getSubscriber().getDroppedFrames().size();
	});
	widgets->addIndicatorBool("---Dropped packets", [this]() {
		bool droppedPacket = false;
		for (const auto & droppedFrame : this->droppedFrames) {
			if (droppedFrame.reason == ofxSquashBuddies::DroppedFrame::Reason::DroppedPackets) {
				return true;
			}
		}
		return false;
	});
	widgets->addIndicatorBool("---Skipped frame", [this]() {
		bool droppedPacket = false;
		for (const auto & droppedFrame : this->droppedFrames) {
			if (droppedFrame.reason == ofxSquashBuddies::DroppedFrame::Reason::SkippedFrame) {
				return true;
			}
		}
		return false;
	});
	widgets->addButton("Load Depth To World Table", [this]() {
		ofBuffer loadBuffer;
		{
			auto result = ofSystemLoadDialog("Select file", false, ofToDataPath(""));
			if (result.bSuccess) {
				auto filePath = result.filePath;
				loadBuffer = ofBufferFromFile(filePath);
			}
		}
		
		if (loadBuffer.size()) {
			ofxSquashBuddies::Message message;
			message.pushData(loadBuffer.getData(), loadBuffer.size());
			message.getData(this->depthToWorldTable);
		}
	});

	//initialise receiver
	{
		auto port = 5000;
		string address = "localhost";

		{
			auto result = ofSystemTextBoxDialog("Select remote address [" + address + "]");
			if (!result.empty()) {
				address = result;
			}
		}
		{
			auto result = ofSystemTextBoxDialog("Select remote port [" + ofToString(port) + "]");
			if (!result.empty()) {
				port = ofToInt(result);
			}
		}

		this->subscriber.init(address, port);

		ofSetWindowTitle("Subscribing on : " + address + ":" + ofToString(port));

		{
			auto colorPanel = ofxCvGui::Panels::makeTexture(this->color.getTexture(), "Color");
			auto depthPanel = ofxCvGui::Panels::makeTexture(this->depth.getTexture(), "Depth");
			auto infraredPanel = ofxCvGui::Panels::makeTexture(this->infrared.getTexture(), "Infrared");
			auto bodyIndexPanel = ofxCvGui::Panels::makeTexture(this->bodyIndex.getTexture(), "BodyIndex");
			auto colorCoordInDepthFramePanel = ofxCvGui::Panels::makeTexture(this->colorCoordInDepthFrame.getTexture(), "ColorCoordInDepthView");

			infraredPanel->setInputRange(0, 0x0FFF);
			depthPanel->setInputRange(0, 8000.0f / float(0xffff));
			
			bodyIndexPanel->setInputRange(0, 12.0f / float(0xff));
			colorCoordInDepthFramePanel->setInputRange(0, 2000.0f / float(0xffff));

			this->gui.add(colorPanel);
			this->gui.add(depthPanel);
			this->gui.add(infraredPanel);
			this->gui.add(bodyIndexPanel);
			this->gui.add(colorCoordInDepthFramePanel);
		}

		auto worldPanel = this->gui.addWorld();
		worldPanel->onDrawWorld += [this](ofCamera &) {
			auto & frame = this->subscriber.getFrame();
			const auto & bodies = frame.getBodies();
			const auto & boneAtlas = ofxKinectForWindows2::Data::Body::getBonesAtlas();
			for (const auto & body : bodies) {
				body.drawWorld();
			}

			if (this->depthToWorldTable.isAllocated()) {
				// TODO: This can be optimized
				ofMesh mesh;
				auto depth = this->subscriber.getFrame().getDepth().getData();
				auto lut = depthToWorldTable.getData();
				for (int i = 0; i < depthToWorldTable.size(); ++i) {
					ofVec3f v = ofVec3f(lut[i * 2 + 0], lut[i * 2 + 1], 1.0) * depth[i] * 0.001f;
					mesh.addVertex(v);
				}
				mesh.draw(OF_MESH_POINTS);
			}
		};
	}
}

//--------------------------------------------------------------
void ofApp::update(){
	this->subscriber.update();
	if (this->subscriber.isFrameNew()) {
		this->newFrame = true;
		const auto & frame = this->subscriber.getFrame();

		//load data in cast format
		{
			this->color = frame.getColor();
			this->depth = frame.getDepth();
			this->infrared = frame.getInfrared();
			this->bodyIndex = frame.getBodyIndex();
			this->colorCoordInDepthFrame = frame.getColorCoordInDepthFrame();
		}
	} else
	{
		this->newFrame = false;
	}
	this->droppedFrames = this->subscriber.getSubscriber().getDroppedFrames();
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
