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

#pragma mark SenderExtColor
	//----------
	void SenderExtColor::init(shared_ptr<ofxKinectForWindows2::Device> kinect, shared_ptr<ofxMachineVision::Grabber::Simple> grabber, string ipAddress, int port) {
		this->kinect = kinect;
		this->grabber = grabber;

		this->sender.init(ipAddress, port);
		this->comboFrame.init(this->kinect, this->grabber);
	}

	//----------
	bool SenderExtColor::update() {
		if (!this->kinect) {
			ofLogWarning("ofxMultiTrack::SenderExtColor") << "Cannot update Sender until you initialise with a valid Kinect";
		}
		if (!this->grabber) {
			ofLogWarning("ofxMultiTrack::SenderExtColor") << "Cannot update Sender until you initialise with a valid Grabber";
		}

		auto isFrameNew = false;

		//update Kinect
		{
			this->kinectFrameRateCounter.update();

			if (this->kinect->isFrameNew()) {
				this->kinectFrameRateCounter.addFrame();

				this->comboFrame.copyFromKinect();
				isFrameNew = true;
			}
		}

		//update grabber, if required
		if (this->grabber)
		{
			this->grabberFrameRateCounter.update();

			if (this->grabber->isFrameNew()) {
				this->grabberFrameRateCounter.addFrame();

				this->comboFrame.copyFromGrabber();
				isFrameNew = true;
			}
		}

		//send frame if new
		if (isFrameNew) {
			auto sentSuccess = this->sender.send(this->comboFrame.getMessage());

			if (!sentSuccess) {
				return false;
			}
		}

		return true;
	}

	//----------
	void SenderExtColor::setCameraParams(const vector<float> & distortion, const ofMatrix4x4 & view, const ofMatrix4x4 & projection) {
		this->comboFrame.setCameraParams(distortion, view, projection);
	}

	//----------
	shared_ptr<ofxMachineVision::Grabber::Simple> SenderExtColor::getGrabber() {
		return this->grabber;
	}

	//----------
	float SenderExtColor::getGrabberFrameRate() const {
		return this->grabberFrameRateCounter.getFrameRate();
	}
}