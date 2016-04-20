#include "Subscriber.h"

using namespace ofxSquashBuddies;

namespace ofxMultiTrack {
	//---------
	bool Subscriber::init(string address, int port) {
		if (!this->subscriber.init(address, port)) {
			return false;
		}
		return true;
	}

	//---------
	void Subscriber::update() {
		this->newFrame = false;

		//wait up to 1/20th of a sec for a new frame
		this->subscriber.update(chrono::milliseconds(1000 / 20));

		if (this->subscriber.isFrameNew()) {
			const auto & message = this->subscriber.getMessage();
			if (this->frame.deserialize(message)) {
				this->newFrame = true;
			}
		}
	}

	//---------
	bool Subscriber::isFrameNew() const {
		return this->newFrame;
	}

	//---------
	const Frame & Subscriber::getFrame() const {
		return this->frame;
	}

	//---------
	ofxSquashBuddies::Subscriber & Subscriber::getSubscriber() {
		return this->subscriber;
	}
}