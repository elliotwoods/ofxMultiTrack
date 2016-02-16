#pragma once

#include "Frame.h"
#include "ofxSquashBuddies/Receiver.h"

namespace ofxMultiTrack {
	class NodeConnection {
	public:
		bool init(int port);
		void update();

		bool isFrameNew() const;
		const Frame & getFrame() const;
		double getFrameRate() const;

		ofxSquashBuddies::Receiver & getReceiver();
	protected:
		Frame frame;

		ofxSquashBuddies::Receiver receiver;
		bool newFrame = false;

		queue<chrono::high_resolution_clock::time_point> incomingFrameTimes;
		float frameRate = 0.0f;
	};
}