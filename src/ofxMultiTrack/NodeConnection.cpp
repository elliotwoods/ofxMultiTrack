#include "NodeConnection.h"

using namespace ofxSquashBuddies;

namespace ofxMultiTrack {
	//---------
	bool NodeConnection::init(int port) {
		if (!this->receiver.init(port)) {
			return false;
		}
		return true;
	}

	//---------
	void NodeConnection::update() {
		this->newFrame = false;

		//wait up to 1/20th of a sec for a new frame
		this->receiver.update(chrono::milliseconds(1000 / 20));

		if (this->receiver.isFrameNew()) {
			const auto & message = this->receiver.getMessage();
			if (this->frame.deserialize(message)) {
				this->newFrame = true;
			}
		}
	}

	//---------
	bool NodeConnection::isFrameNew() const {
		return this->newFrame;
	}

	//---------
	const Frame & NodeConnection::getFrame() const {
		return this->frame;
	}

	//---------
	ofxSquashBuddies::Receiver & NodeConnection::getReceiver() {
		return this->receiver;
	}

}