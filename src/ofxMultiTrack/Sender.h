#pragma once

#include "Frame.h"

#include "ofxSquashBuddies/Sender.h"
#include "ofxKinectForWindows2.h"

namespace ofxMultiTrack {
	class Sender {
	public:
		void init(shared_ptr<ofxKinectForWindows2::Device>, string ipAddress, int port);
		bool update(); // return false if dropped a frame

		shared_ptr<ofxKinectForWindows2::Device> getKinect();
		ofxSquashBuddies::Sender & getSender();

		float getDeviceFrameRate() const;

	protected:
		shared_ptr<ofxKinectForWindows2::Device> kinect;

		ofxSquashBuddies::Sender sender;
		DeviceFrame deviceFrame;

		ofxSquashBuddies::Utils::FramerateCounter kinectFrameRateCounter;
	};
}