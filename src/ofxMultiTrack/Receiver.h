#pragma once

#include "Frame.h"
#include "ofxSquashBuddies/Receiver.h"

#include "ofxKinectForWindows2.h"

namespace ofxMultiTrack {
	class Receiver {
	public:
		bool init(int port);
		void update();

		bool isFrameNew() const;
		const Frame & getFrame() const;

		ofxSquashBuddies::Receiver & getReceiver();
	protected:
		Frame frame;

		ofxSquashBuddies::Receiver receiver;
		bool newFrame = false;
	};
}
