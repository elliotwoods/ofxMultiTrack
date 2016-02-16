#include "NodeConnection.h"

using namespace ofxSquashBuddies;

namespace ofxMultiTrack {
	//---------
	void NodeConnection::init(int port) {
		this->receiver.init(port);

		this->frame.color.allocate(1280, 720, OF_PIXELS_YUY2);
		this->frame.depth.allocate(512, 424, OF_PIXELS_GRAY);
		this->frame.infrared.allocate(512, 424, OF_PIXELS_GRAY);
		this->frame.bodyIndex.allocate(512, 424, OF_PIXELS_GRAY);
		this->frame.colorCoordInDepthFrame.allocate(512, 424, OF_PIXELS_RG);
	}

	//---------
	void NodeConnection::update() {
		this->newFrame = false;

		this->receiver.update();
		if (this->receiver.isFrameNew()) {
			const auto & message = this->receiver.getMessage();
			this->incomingFrameTimes.push(chrono::high_resolution_clock::now());

			if (message.hasHeader<Header::MultiTrack_2_2_Frame>()) {
				this->newFrame = true;

				auto headerSize = sizeof(Header::MultiTrack_2_2_Frame);
				auto bodySize = (size_t)Header::MultiTrack_2_2_Frame::Constants::TotalDataSize;

				//header
				{
					auto & header = message.getHeader<Header::MultiTrack_2_2_Frame>();
				}

				//body
				{
					auto body = (uint8_t*)message.getBodyData();

					auto & color = this->frame.color;
					memcpy(color.getData(), body, Header::MultiTrack_2_2_Frame::Constants::ColorSize);
					body += Header::MultiTrack_2_2_Frame::Constants::ColorSize;

					auto & depth = this->frame.depth;
					memcpy(depth.getData(), body, Header::MultiTrack_2_2_Frame::Constants::DepthSize);
					body += Header::MultiTrack_2_2_Frame::Constants::DepthSize;

					auto & infrared = this->frame.infrared;
					memcpy(infrared.getData(), body, Header::MultiTrack_2_2_Frame::Constants::InfraredSize);
					body += Header::MultiTrack_2_2_Frame::Constants::InfraredSize;

					auto & bodyIndex = this->frame.bodyIndex;
					memcpy(bodyIndex.getData(), body, Header::MultiTrack_2_2_Frame::Constants::BodyIndexSize);
					body += Header::MultiTrack_2_2_Frame::Constants::BodyIndexSize;

					auto & colorCoordInDepthFrame = this->frame.colorCoordInDepthFrame;
					memcpy(colorCoordInDepthFrame.getData(), body, Header::MultiTrack_2_2_Frame::Constants::ColorCoordInDepthViewSize);
					body += Header::MultiTrack_2_2_Frame::Constants::ColorCoordInDepthViewSize;
				}
			}
		}

		//calculate fps
		if (!incomingFrameTimes.empty()) {
			auto now = chrono::high_resolution_clock::now();
			auto old = incomingFrameTimes.front();
			auto size = incomingFrameTimes.size();
			while (incomingFrameTimes.size() > 30) {
				incomingFrameTimes.pop();
			}
			this->frameRate = (double) (size - 1) * 1e6 / (double)chrono::duration_cast<chrono::microseconds>(now - old).count();
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
	double NodeConnection::getFrameRate() const {
		return this->frameRate;
	}

	//---------
	ofxSquashBuddies::Receiver & NodeConnection::getReceiver() {
		return this->receiver;
	}

}