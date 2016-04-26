#include "ofApp.h"

#include "ofxSquash.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(60);

	this->gui.init();
	
	{
		auto widgets = this->gui.addWidgets();
		widgets->addTitle("Subscriber x 3");
		widgets->addFps();
	}

	for (int i = 0; i < RECEIVER_COUNT; i++) {
		auto widgets = this->gui.addWidgets();
		widgets->addTitle("MultiTrack Subscribe");
		widgets->addFps();
		widgets->addMemoryUsage();
		widgets->addIndicatorBool("Frame incoming", [this, i]() {
			return this->receivers[i].newFrame;
		});
		widgets->addLiveValueHistory("Receiver incoming frame rate", [this, i]() {
			return (float) this->receivers[i].subscriber.getSubscriber().getIncomingFramerate();
		});
		widgets->addLiveValueHistory("Dropped frames", [this, i]() {
			return (float) this->receivers[i].subscriber.getSubscriber().getDroppedFrames().size();
		});
		widgets->addIndicatorBool("---Dropped packets", [this, i]() {
			bool droppedPacket = false;
			for (const auto & droppedFrame : this->receivers[i].droppedFrames) {
				if (droppedFrame.reason == ofxSquashBuddies::DroppedFrame::Reason::DroppedPackets) {
					return true;
				}
			}
			return false;
		});
		widgets->addIndicatorBool("---Skipped frame", [this, i]() {
			bool droppedPacket = false;
			for (const auto & droppedFrame : this->receivers[i].droppedFrames) {
				if (droppedFrame.reason == ofxSquashBuddies::DroppedFrame::Reason::SkippedFrame) {
					return true;
				}
			}
			return false;
		});
		widgets->addButton("Load Depth To World Table", [this, i]() {
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
				message.getData(this->receivers[i].depthToWorldTable);
			}
		});

		//initialise receiver
		{
			auto port = 5000;
			string address = "10.1.100." + ofToString(212 + i);

			this->receivers[i].subscriber.init(address, port);

			ofSetWindowTitle("Subscribing on : " + address + ":" + ofToString(port));

			{
				auto colorPanel = ofxCvGui::Panels::makeTexture(this->receivers[i].color.getTexture(), "Color");
				auto depthPanel = ofxCvGui::Panels::makeTexture(this->receivers[i].depth.getTexture(), "Depth");
				auto infraredPanel = ofxCvGui::Panels::makeTexture(this->receivers[i].infrared.getTexture(), "Infrared");
				auto bodyIndexPanel = ofxCvGui::Panels::makeTexture(this->receivers[i].bodyIndex.getTexture(), "BodyIndex");
				auto colorCoordInDepthFramePanel = ofxCvGui::Panels::makeTexture(this->receivers[i].colorCoordInDepthFrame.getTexture(), "ColorCoordInDepthView");

				depthPanel->setInputRange(0, 8000.0f / float(0xffff));
				infraredPanel->setInputRange(0, 0x0FFF);

				bodyIndexPanel->setInputRange(0, 12.0f / float(0xff));
				colorCoordInDepthFramePanel->setInputRange(0, 2000.0f / float(0xffff));

				this->gui.add(colorPanel);
				this->gui.add(depthPanel);
				this->gui.add(infraredPanel);
				this->gui.add(bodyIndexPanel);
				this->gui.add(colorCoordInDepthFramePanel);
			}

			auto worldPanel = this->gui.addWorld();
			worldPanel->onDrawWorld += [this, i](ofCamera &) {
				auto & frame = this->receivers[i].subscriber.getFrame();
				const auto & bodies = frame.getBodies();
				const auto & boneAtlas = ofxKinectForWindows2::Data::Body::getBonesAtlas();
				for (const auto & body : bodies) {
					body.drawWorld();
				}

				if (this->receivers[i].depthToWorldTable.isAllocated()) {
					// TODO: This can be optimized
					ofMesh mesh;
					auto depth = this->receivers[i].subscriber.getFrame().getDepth().getData();
					auto lut = this->receivers[i].depthToWorldTable.getData();
					for (int i = 0; i < this->receivers[i].depthToWorldTable.size(); ++i) {
						ofVec3f v = ofVec3f(lut[i * 2 + 0], lut[i * 2 + 1], 1.0) * depth[i] * 0.001f;
						mesh.addVertex(v);
					}
					mesh.draw(OF_MESH_POINTS);
				}
			};
		}

	}
}

//--------------------------------------------------------------
void ofApp::update(){
	for (int i = 0; i < RECEIVER_COUNT; i++) {
		this->receivers[i].subscriber.update();
		if (this->receivers[i].subscriber.isFrameNew()) {
			this->receivers[i].newFrame = true;
			const auto & frame = this->receivers[i].subscriber.getFrame();

			//load data in cast format
			{
				this->receivers[i].color = frame.getColor();
				this->receivers[i].depth = frame.getDepth();
				this->receivers[i].infrared = frame.getInfrared();
				this->receivers[i].bodyIndex = frame.getBodyIndex();
				this->receivers[i].colorCoordInDepthFrame = frame.getColorCoordInDepthFrame();
			}
		}
		else
		{
			this->receivers[i].newFrame = false;
		}
		this->receivers[i].droppedFrames = this->receivers[i].subscriber.getSubscriber().getDroppedFrames();
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
