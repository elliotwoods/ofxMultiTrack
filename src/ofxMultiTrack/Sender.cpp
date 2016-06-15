#include "Sender.h"

using namespace ofxSquashBuddies;

namespace ofxMultiTrack {
	//----------
	void Sender::init(shared_ptr<ofxKinectForWindows2::Device> kinect, string ipAddress, int port) {
		this->kinect = kinect;

		this->sender.init(ipAddress, port);
		this->deviceFrame.init(this->kinect);
	}

	//----------
	bool Sender::update() {
		if (!this->kinect) {
			ofLogWarning("ofxMultiTrack::Sender") << "Cannot update Sender until you initialise with a valid Kinect";
		}

		this->kinectFrameRateCounter.update(); 
		
		if (this->kinect->isFrameNew()) {
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
	shared_ptr<ofxKinectForWindows2::Device> Sender::getKinect() {
		return this->kinect;
	}

	//----------
	ofxSquashBuddies::Sender & Sender::getSender() {
		return this->sender;
	}

	//----------
	float Sender::getDeviceFrameRate() const {
		return this->kinectFrameRateCounter.getFrameRate();
	}
}