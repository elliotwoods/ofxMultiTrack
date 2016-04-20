#pragma once

#include "Frame.h"

#include "ofxSquashBuddies/Publisher.h"
#include "ofxKinectForWindows2.h"

namespace ofxMultiTrack {
	class Publisher {
	public:
		void init(ofxKinectForWindows2::Device &, int port);
		bool update(); // return false if dropped a frame

		ofxKinectForWindows2::Device & getKinect();
		ofxSquashBuddies::Publisher & getPublisher();

		float getDeviceFrameRate() const;

	protected:
		ofxKinectForWindows2::Device * kinect = nullptr;

		ofxSquashBuddies::Publisher publisher;
		DeviceFrame deviceFrame;

		ofxSquashBuddies::Utils::FramerateCounter kinectFrameRateCounter;
	};
}