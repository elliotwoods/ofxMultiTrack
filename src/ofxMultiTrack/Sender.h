#pragma once

#include "Frame.h"

#include "ofxSquashBuddies/Sender.h"
#include "ofxKinectForWindows2.h"

namespace ofxMultiTrack {
	class Sender {
	public:
		void init(shared_ptr<ofxKinectForWindows2::Device>, string ipAddress, int port);
		virtual bool update(); // return false if dropped a frame

		shared_ptr<ofxKinectForWindows2::Device> getKinect();
		ofxSquashBuddies::Sender & getSender();

		float getDeviceFrameRate() const;

	protected:
		shared_ptr<ofxKinectForWindows2::Device> kinect;

		ofxSquashBuddies::Sender sender;
		DeviceFrame deviceFrame;

		ofxSquashBuddies::Utils::FramerateCounter kinectFrameRateCounter;
	};

	class SenderExtColor : public Sender {
	public:
		void init(shared_ptr<ofxKinectForWindows2::Device>, shared_ptr<ofxMachineVision::Grabber::Simple>, string ipAddress, int port);
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