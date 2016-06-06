#pragma once

#include "Frame.h"

#include "ofxSquashBuddies/Publisher.h"
#include "ofxKinectForWindows2.h"
#include "ofxMachineVision.h"

namespace ofxMultiTrack {
	class Publisher {
	public:
		void init(shared_ptr<ofxKinectForWindows2::Device>, int port);
		virtual bool update(); // return false if dropped a frame

		shared_ptr<ofxKinectForWindows2::Device> getKinect();
		ofxSquashBuddies::Publisher & getPublisher();

		float getDeviceFrameRate() const;

	protected:
		shared_ptr<ofxKinectForWindows2::Device> kinect;

		ofxSquashBuddies::Publisher publisher;
		DeviceFrame deviceFrame;

		ofxSquashBuddies::Utils::FramerateCounter kinectFrameRateCounter;
	};

	class PublisherExtColor : public Publisher {
	public:
		void init(shared_ptr<ofxKinectForWindows2::Device>, shared_ptr<ofxMachineVision::Grabber::Simple>, int port);
		virtual bool update() override;

		void setCameraParams(const vector<float> & distortion, const ofMatrix4x4 & view, const ofMatrix4x4 & projection);

		shared_ptr<ofxMachineVision::Grabber::Simple> getGrabber();

		float getGrabberFrameRate() const;

	protected:
		shared_ptr<ofxMachineVision::Grabber::Simple> grabber;

		ComboFrame comboFrame;

		ofxSquashBuddies::Utils::FramerateCounter grabberFrameRateCounter;
	};
}