#include "ofApp.h"

#include "ofxSquash.h"

//--------------------------------------------------------------
void ofApp::setup() {
	this->gui.init();
	
	auto widgets = this->gui.addWidgets();
	widgets->addTitle("MultiTrack Subscribe");
	widgets->addFps();
	widgets->addMemoryUsage();
	widgets->addIndicator("Frame incoming", [this]() {
		return this->newFrame;
	});
	widgets->addLiveValueHistory("Receiver incoming frame rate", [this]() {
		return (float) this->subscriber.getSubscriber().getIncomingFramerate();
	});
	widgets->addLiveValueHistory("Dropped frames", [this]() {
		return (float) this->subscriber.getSubscriber().getDroppedFrames().size();
	});
	widgets->addLiveValue<string>("Sending Body OSC to ", [this]() {
		return this->oscHost + ":" + ofToString(this->oscPort);;
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

		this->oscPort = port + 1000;

		ofSetWindowTitle("Subscribing on : " + address + ":" + ofToString(port));

		this->gui.add(ofxCvGui::Panels::makeTexture(this->color.texture, "Color"));
		this->gui.add(ofxCvGui::Panels::makeTexture(this->depth.texture, "Depth"));
		this->gui.add(ofxCvGui::Panels::makeTexture(this->infrared.texture, "Infrared"));
		this->gui.add(ofxCvGui::Panels::makeTexture(this->bodyIndex.texture, "BodyIndex"));
		this->gui.add(ofxCvGui::Panels::makeTexture(this->colorCoordInDepthFrame.texture, "ColorCoordInDepthView"));
		
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


	//initialise texture sharing
	{
		try {
			auto suffix = ofToString(this->clientIndex);

			this->color.sender.init("color" + suffix);
			this->depth.sender.init("depth" + suffix);
			this->infrared.sender.init("infrared" + suffix);
			this->bodyIndex.sender.init("bodyIndex" + suffix);
			this->colorCoordInDepthFrame.sender.init("colorCoordInDepthFrame" + suffix);
		}
		catch (std::exception e) {
			ofSystemAlertDialog(string("Couldn't initialise Spout texture sharing : ") + e.what());
		}
		catch (...) {
			ofSystemAlertDialog("Couldn't initialise Spout texture sharing");
		}
	}


	//initialise osc (not a standard format)
	{
		this->oscSender.setup(this->oscHost, this->oscPort);
	}

	ofSetFrameRate(60);
}

//--------------------------------------------------------------
void ofApp::update(){
	this->subscriber.update();
	if (this->subscriber.isFrameNew()) {
		this->newFrame = true;
		const auto & frame = this->subscriber.getFrame();

		//load data in cast format
		{
			//YUY8 -> RGBA8
			this->color.texture.loadData((unsigned char *) frame.getColor().getData(), frame.getColor().getWidth() / 2, frame.getColor().getHeight(), GL_RGBA);
			this->color.send();

			//L16 -> RGBA8
			this->depth.texture.loadData((unsigned char *)frame.getDepth().getData(), frame.getDepth().getWidth() / 2, frame.getDepth().getHeight(), GL_RGBA);
			this->depth.send();

			//L16 -> RGBA8
			this->infrared.texture.loadData((unsigned char *)frame.getInfrared().getData(), frame.getInfrared().getWidth() / 2, frame.getInfrared().getHeight(), GL_RGBA);
			this->infrared.send();

			//L8 -> L8
			this->bodyIndex.texture.loadData(frame.getBodyIndex());
			this->bodyIndex.send();

			//RG16 -> RGBA8
			this->colorCoordInDepthFrame.texture.loadData((unsigned char *)frame.getColorCoordInDepthFrame().getData(), frame.getColorCoordInDepthFrame().getWidth(), frame.getColorCoordInDepthFrame().getHeight(), GL_RGBA);
			this->colorCoordInDepthFrame.send();
		}

		//send body via osc
		{
			const auto & bodies = frame.getBodies();
			for (const auto & body : bodies) {
				ofxOscBundle bundle;
				const auto bodyAddress = "/bodies/" + ofToString((int) body.bodyId);
				{
					ofxOscMessage message;
					message.addBoolArg(body.tracked);
					message.addInt32Arg(body.trackingId);
					message.addInt32Arg(body.leftHandState);
					message.addInt32Arg(body.rightHandState);
					message.setAddress(bodyAddress);
					bundle.addMessage(message);
				}

				for (const auto & joint : body.joints) {
					const auto jointAddress = bodyAddress + "/joints/" + ofToString((int) joint.first);
					{
						ofxOscMessage message;
						const auto & position = joint.second.getPosition();
						message.addFloatArg(position.x);
						message.addFloatArg(position.y);
						message.addFloatArg(position.z);
						message.setAddress(jointAddress + "/position");
						bundle.addMessage(message);
					}
					{
						ofxOscMessage message;
						const auto & orientation = joint.second.getOrientation();
						message.addFloatArg(orientation.x());
						message.addFloatArg(orientation.y());
						message.addFloatArg(orientation.z());
						message.addFloatArg(orientation.w());
						message.setAddress(jointAddress + "/orientation");
						bundle.addMessage(message);
					}
				}
				this->oscSender.sendBundle(bundle);
			}
		}
	} else
	{
		this->newFrame = false;
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
