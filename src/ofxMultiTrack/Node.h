#pragma once

#include "Frame.h"

#include "ofxSquashBuddies/Sender.h"
#include "ofxKinectForWindows2.h"

namespace ofxMultiTrack {
	class Node {
	public:
		struct Stream {
			ofParameter<bool> enabled = ofParameter<bool>("Enabled", false);
			ofxSquashBuddies::Header::MultiTrack_2_3_Frame::FrameSettings frameSettings;
			shared_ptr<ofxKinectForWindows2::Source::Base> source;
		};
		void init(string ipAddress, int port);
		bool update(); // return false if dropped a frame

		ofxKinectForWindows2::Device & getKinect();
		ofxSquashBuddies::Sender & getSender();

		void setTexturesEnabled(bool);

		float getDeviceFrameRate() const;
		ofImage colorPreview;

		struct {
			Stream color;
			Stream depth;
			Stream infrared;
			Stream bodyIndex;
			Stream colorCoordInDepthView;
			Stream bodies;
		} streams;
	protected:
		void setupKinect();
		void setupMessage();
		ofxKinectForWindows2::Device kinect;

		ofxSquashBuddies::Sender sender;
		ofxSquashBuddies::Message message;

		Frame frame;
		ofFloatPixels colorCoordInDepthFrameFloat;
		uint8_t * bodiesData;

		ofTexture yuyPreview;
		ofxSquashBuddies::Utils::FramerateCounter kinectFrameRateCounter;
	};
}