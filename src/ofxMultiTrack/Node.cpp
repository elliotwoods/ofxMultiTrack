#include "Node.h"

using namespace ofxSquashBuddies;

namespace ofxMultiTrack {
	//----------
	void Node::init(string ipAddress, int port) {
		this->sender.init(ipAddress, port);

		this->kinect.open();
		this->kinect.initColorSource();
		this->kinect.initDepthSource();
		this->kinect.initInfraredSource();
		this->kinect.initBodyIndexSource();
		this->kinect.initBodySource();

		this->deviceFrame.init(this->kinect);
		
	}

	//----------
	bool Node::update() {
		this->kinectFrameRateCounter.update(); 
		
		this->kinect.update();
		if (this->kinect.isFrameNew()) {
			this->kinectFrameRateCounter.addFrame();

			this->deviceFrame.copyFromKinect();

			auto sentSuccess = this->sender.send(this->deviceFrame.getMessage());

			if (!sentSuccess) {
				return false;
			}
		}

		return true;
	}

	//----------
	ofxKinectForWindows2::Device & Node::getKinect() {
		return this->kinect;
	}

	//----------
	ofxSquashBuddies::Sender & Node::getSender() {
		return this->sender;
	}

	//----------
	void Node::setTexturesEnabled(bool texturesEnabled) {
		auto sources = this->kinect.getSources();
		for (auto source : sources) {
			auto textureSource = dynamic_pointer_cast<ofBaseHasTexture>(source);
			if (textureSource) {
				textureSource->setUseTexture(texturesEnabled);
			}
		}
	}

	//----------
	float Node::getDeviceFrameRate() const {
		return this->kinectFrameRateCounter.getFrameRate();
	}
}