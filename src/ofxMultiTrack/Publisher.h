#pragma once

#include "Frame.h"

#include "ofxSquashBuddies/Publisher.h"
#include "ofxKinectForWindows2.h"
#include "ofxMachineVision.h"

namespace ofxMultiTrack {
	class Publisher {
	public:
		void init(shared_ptr<ofxKinectForWindows2::Device>, int port);
		void init(shared_ptr<ofxKinectForWindows2::Device>, shared_ptr<ofxMachineVision::Grabber::Simple>, int port);
		bool update(); // return false if dropped a frame

		shared_ptr<ofxKinectForWindows2::Device> getKinect();
		shared_ptr<ofxMachineVision::Grabber::Simple> getGrabber();
		ofxSquashBuddies::Publisher & getPublisher();

		float getDeviceFrameRate() const;
		float getGrabberFrameRate() const;

	protected:
		shared_ptr<ofxKinectForWindows2::Device> kinect;
		shared_ptr<ofxMachineVision::Grabber::Simple> grabber;

		ofxSquashBuddies::Publisher publisher;
		DeviceFrame deviceFrame;

		ofxSquashBuddies::Utils::FramerateCounter kinectFrameRateCounter;
		ofxSquashBuddies::Utils::FramerateCounter grabberFrameRateCounter;
	};
}