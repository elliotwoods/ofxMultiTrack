#include "ofApp.h"

#include "ofxSquash.h"

//--------------------------------------------------------------
void ofApp::setup() {

	this->gui.init();
	
	auto widgets = this->gui.addWidgets();
	widgets->addTitle("MultiTrack Receive");
	widgets->addFps();
	widgets->addLiveValue<int>("Receiving on port", [this]() {
		return (float) this->nodeConnection.getReceiver().getPort();
	});
	widgets->addIndicator("Frame incoming", [this]() {
		return this->newFrame;
	});
	widgets->addLiveValueHistory("Receiver incoming frame rate", [this]() {
		return (float) this->nodeConnection.getReceiver().getIncomingFramerate();
	});
	widgets->addLiveValueHistory("Dropped frames", [this]() {
		return (float) this->nodeConnection.getReceiver().getDroppedFrames().size();
	});
	widgets->addLiveValue<string>("Sending Body OSC to ", [this]() {
		return this->oscHost + ":" + ofToString(this->oscPort);;
	});

	//initialise receiver
	{
		auto port = 4444;

		while (!this->nodeConnection.init(port)) {
			port++;
		}

		this->clientIndex = port - 4444;
		widgets->addLiveValue<float>("Client index", [this]() {
			return this->clientIndex;
		});

		this->oscPort = port + 1000;

		this->gui.add(ofxCvGui::Panels::make(this->color.texture, "Color"));
		this->gui.add(ofxCvGui::Panels::make(this->depth.texture, "Depth"));
		this->gui.add(ofxCvGui::Panels::make(this->infrared.texture, "Infrared"));
		this->gui.add(ofxCvGui::Panels::make(this->bodyIndex.texture, "BodyIndex"));
		this->gui.add(ofxCvGui::Panels::make(this->colorCoordInDepthFrame.texture, "ColorCoordInDepthView"));
		
		auto worldPanel = this->gui.addWorld();
		worldPanel->onDrawWorld += [this](ofCamera &) {
			auto & frame = this->nodeConnection.getFrame();
			const auto & bodies = frame.bodies;
			const auto & boneAtlas = ofxKinectForWindows2::Data::Body::getBonesAtlas();
			for (const auto & body : bodies) {
				if (body.tracked) {
					const auto & joints = body.joints;
					for (const auto & bone : boneAtlas) {
						auto findFirst = joints.find(bone.first);
						auto findSecond = joints.find(bone.second);
						if (findFirst != joints.end() && findSecond != joints.end()) {
							ofLine(findFirst->second.getPosition(), findSecond->second.getPosition());
						}
					}
				}
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
	this->nodeConnection.update();
	if (this->nodeConnection.isFrameNew()) {
		this->newFrame = true;
		const auto & frame = this->nodeConnection.getFrame();

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

		//send body via osc
		{
			const auto & bodies = frame.bodies;
			for (const auto & body : bodies) {
				ofxOscBundle bundle;
				const auto bodyAddress = "/bodies/" + ofToString(body.bodyId);
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
					const auto jointAddress = bodyAddress + "/joints/" + ofToString(joint.first);
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
