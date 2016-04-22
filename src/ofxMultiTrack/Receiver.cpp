#ifdef OFXMULTITRACK_UDP
#include "Receiver.h"

using namespace ofxSquashBuddies;

namespace ofxMultiTrack {
	//---------
	bool Receiver::init(int port) {
		if (!this->receiver.init(port)) {
			return false;
		}
		return true;
	}

	//---------
	void Receiver::update() {
		this->newFrame = false;

		//wait up to 1/20th of a sec for a new frame
		this->receiver.update();

		if (this->receiver.isFrameNew()) {
			const auto & message = this->receiver.getMessage();
			if (this->frame.deserialize(message)) {
				this->newFrame = true;
			}
		}
	}

	//---------
	bool Receiver::isFrameNew() const {
		return this->newFrame;
	}

	//---------
	const Frame & Receiver::getFrame() const {
		return this->frame;
	}

	//---------
	ofxSquashBuddies::Receiver & Receiver::getReceiver() {
		return this->receiver;
	}

}
#endif // OFXMULTITRACK_UDP