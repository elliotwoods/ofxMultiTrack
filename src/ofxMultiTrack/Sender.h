#pragma once

#include "Frame.h"

#include "ofxSquashBuddies/Sender.h"
#include "ofxKinectForWindows2.h"

namespace ofxMultiTrack {
	class Sender {
	public:
		void init(ofxKinectForWindows2::Device &, string ipAddress, int port);
		bool update(); // return false if dropped a frame

		ofxKinectForWindows2::Device & getKinect();
		ofxSquashBuddies::Sender & getSender();

		float getDeviceFrameRate() const;

	protected:
		ofxKinectForWindows2::Device * kinect = nullptr;

		ofxSquashBuddies::Sender sender;
		DeviceFrame deviceFrame;

		ofxSquashBuddies::Utils::FramerateCounter kinectFrameRateCounter;
	};
}