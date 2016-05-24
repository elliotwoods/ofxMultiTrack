#include "Publisher.h"

using namespace ofxSquashBuddies;

namespace ofxMultiTrack {
	//----------
	void Publisher::init(shared_ptr<ofxKinectForWindows2::Device> kinect, int port) {
		this->kinect = kinect;
		this->grabber.reset();

		this->publisher.init(port);
		this->deviceFrame.init(this->kinect);
	}

	//----------
	void Publisher::init(shared_ptr<ofxKinectForWindows2::Device> kinect, shared_ptr<ofxMachineVision::Grabber::Simple> grabber, int port) {
		this->kinect = kinect;
		this->grabber = grabber;

		this->publisher.init(port);
		this->deviceFrame.init(this->kinect, this->grabber);
	}

	//----------
	bool Publisher::update() {
		if (!this->kinect) {
			ofLogWarning("ofxMultiTrack::Publisher") << "Cannot update Publisher until you initialise with a valid Kinect";
		}

		auto isFrameNew = false;

		//update Kinect
		{
			this->kinectFrameRateCounter.update();

			if (this->kinect->isFrameNew()) {
				this->kinectFrameRateCounter.addFrame();

				this->deviceFrame.copyFromKinect();
				isFrameNew = true;
			}
		}

		//update grabber, if required
		if (this->grabber)
		{
			this->grabberFrameRateCounter.update();

			if (this->grabber->isFrameNew()) {
				this->grabberFrameRateCounter.addFrame();

				this->deviceFrame.copyFromGrabber();
				isFrameNew = true;
			}
		}
		
		//send frame if new
		if (isFrameNew) {
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
	shared_ptr<ofxMachineVision::Grabber::Simple> Publisher::getGrabber() {
		return this->grabber;
	}

	//----------
	ofxSquashBuddies::Publisher & Publisher::getPublisher() {
		return this->publisher;
	}

	//----------
	float Publisher::getDeviceFrameRate() const {
		return this->kinectFrameRateCounter.getFrameRate();
	}

	//----------
	float Publisher::getGrabberFrameRate() const {
		return this->grabberFrameRateCounter.getFrameRate();
	}
}