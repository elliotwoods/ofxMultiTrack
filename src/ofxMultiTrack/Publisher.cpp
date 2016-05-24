#include "Publisher.h"

using namespace ofxSquashBuddies;

namespace ofxMultiTrack {
	//----------
	void Publisher::init(shared_ptr<ofxKinectForWindows2::Device> kinect, int port) {
		this->kinect = kinect;

		this->publisher.init(port);
		this->deviceFrame.init(this->kinect);
	}

	//----------
	bool Publisher::update() {
		if (!this->kinect) {
			ofLogWarning("ofxMultiTrack::Sender") << "Cannot update Sender until you initialise with a valid Kinect";
		}

		this->kinectFrameRateCounter.update(); 
		
		if (this->kinect->isFrameNew()) {
			this->kinectFrameRateCounter.addFrame();

			this->deviceFrame.copyFromKinect();

			auto sentSuccess = this->publisher.send(this->deviceFrame.getMessage());

			if (!sentSuccess) {
				return false;
			}
		}

		return true;	
	}

	//----------
	shared_ptr<ofxKinectForWindows2::Device> Publisher::getKinect() {
		return this->kinect;
	}

	//----------
	ofxSquashBuddies::Publisher & Publisher::getPublisher() {
		return this->publisher;
	}

	//----------
	float Publisher::getDeviceFrameRate() const {
		return this->kinectFrameRateCounter.getFrameRate();
	}
}