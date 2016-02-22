#include "NodeConnection.h"

using namespace ofxSquashBuddies;

namespace ofxMultiTrack {
	//---------
	bool NodeConnection::init(int port) {
		if (!this->receiver.init(port)) {
			return false;
		}

		this->frame.color.allocate(1280, 720, OF_PIXELS_YUY2);
		this->frame.depth.allocate(512, 424, OF_PIXELS_GRAY);
		this->frame.infrared.allocate(512, 424, OF_PIXELS_GRAY);
		this->frame.bodyIndex.allocate(512, 424, OF_PIXELS_GRAY);
		this->frame.colorCoordInDepthFrame.allocate(512, 424, OF_PIXELS_RG);

		return true;
	}

	//---------
	template<typename DataType>
	const DataType & readAndMove(uint8_t * & in) {
		auto & inTyped = ((DataType * &)in);
		return *inTyped++;
	}

	//---------
	template<typename DataType>
	void readAndMove(uint8_t * & in, DataType & out) {
		auto & inTyped = ((DataType * &)in);
		out = *inTyped++;
	}

	//---------
	void NodeConnection::update() {
		this->newFrame = false;

		this->receiver.update();

		//wait up to 1/20th of a sec for a new frame
		auto startTime = chrono::high_resolution_clock::now();
		auto timeout = chrono::milliseconds(1000) / 20;
		while (!this->receiver.isFrameNew()) {
			if (chrono::high_resolution_clock::now() - startTime > timeout) {
				break;
			}
			this->receiver.update();
		}

		if (this->receiver.isFrameNew()) {
			const auto & message = this->receiver.getMessage();

			if (message.hasHeader<Header::MultiTrack_2_2_Frame>()) {
				this->newFrame = true;

				auto headerSize = sizeof(Header::MultiTrack_2_2_Frame);
				auto bodySize = (size_t)Header::MultiTrack_2_2_Frame::Constants::TotalDataSize;

				//header
				auto & header = message.getHeader<Header::MultiTrack_2_2_Frame>();

				//body
				auto body = (uint8_t*)message.getBodyData();
				{
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

					{
						auto data = body;
						auto skeletonCount = readAndMove<uint8_t>(data);

						vector<ofxKinectForWindows2::Data::Body> bodies(skeletonCount);
						for (int i = 0; i < skeletonCount; i++) {
							auto & body = bodies[i];
							auto & joints = body.joints;

							readAndMove(data, (uint8_t&) body.tracked);
							readAndMove(data, (uint8_t&) body.bodyId);
							readAndMove(data, (uint64_t&) body.trackingId);
							readAndMove(data, (uint8_t&) body.leftHandState);
							readAndMove(data, (uint8_t&) body.rightHandState);
							auto jointsCount = readAndMove<uint8_t>(data);

							for (int i = 0; i < jointsCount; i++) {
								auto joint = readAndMove<_Joint>(data);
								auto jointOrientation = readAndMove<_JointOrientation>(data);
								joints.emplace(make_pair(joint.JointType, ofxKinectForWindows2::Data::Joint(joint, jointOrientation)));
							}

							bodies.push_back(body);
						}
						this->frame.bodies = bodies;
					}
					
				}
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