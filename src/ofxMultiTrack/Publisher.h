#pragma once

#include "Frame.h"

#include "ofxSquashBuddies/Publisher.h"
#include "ofxKinectForWindows2.h"

namespace ofxMultiTrack {
	class Publisher {
	public:
		void init(shared_ptr<ofxKinectForWindows2::Device>, int port);
		bool update(); // return false if dropped a frame

		shared_ptr<ofxKinectForWindows2::Device> getKinect();
		ofxSquashBuddies::Publisher & getPublisher();

		float getDeviceFrameRate() const;

	protected:
		shared_ptr<ofxKinectForWindows2::Device> kinect;

		ofxSquashBuddies::Publisher publisher;
		DeviceFrame deviceFrame;

		ofxSquashBuddies::Utils::FramerateCounter kinectFrameRateCounter;
	};
}