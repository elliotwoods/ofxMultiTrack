#pragma once

#include "Frame.h"
#include "ofxSquashBuddies/Subscriber.h"

#include "ofxKinectForWindows2.h"

namespace ofxMultiTrack {
	class Subscriber {
	public:
		bool init(string address, int port);
		void update();

		bool isFrameNew() const;
		const Frame & getFrame() const;

		ofxSquashBuddies::Subscriber & getSubscriber();
	protected:
		Frame frame;

		ofxSquashBuddies::Subscriber subscriber;
		bool newFrame = false;
	};
}