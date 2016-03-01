#include "NodeConnection.h"

using namespace ofxSquashBuddies;

namespace ofxMultiTrack {
	//---------
	bool NodeConnection::init(int port) {
		if (!this->receiver.init(port)) {
			return false;
		}

		//default allocations (these actually can change as frames arrive)
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
	vector<ofxKinectForWindows2::Data::Body> readBodies(uint8_t * data) {
		auto skeletonCount = readAndMove<uint8_t>(data);

		vector<ofxKinectForWindows2::Data::Body> bodies(skeletonCount);
		for (int i = 0; i < skeletonCount; i++) {
			auto & body = bodies[i];
			auto & joints = body.joints;

			readAndMove(data, (uint8_t&)body.tracked);
			readAndMove(data, (uint8_t&)body.bodyId);
			readAndMove(data, (uint64_t&)body.trackingId);
			readAndMove(data, (uint8_t&)body.leftHandState);
			readAndMove(data, (uint8_t&)body.rightHandState);
			auto jointsCount = readAndMove<uint8_t>(data);

			for (int i = 0; i < jointsCount; i++) {
				auto joint = readAndMove<_Joint>(data);
				auto jointOrientation = readAndMove<_JointOrientation>(data);
				joints.emplace(make_pair(joint.JointType, ofxKinectForWindows2::Data::Joint(joint, jointOrientation)));
			}

			bodies.push_back(body);
		}

		return bodies;
	}

	//---------
	template<typename PixelType>
	void readPixels(uint8_t *& data, ofPixels_<PixelType> & pixels) {
		auto frameSettings = readAndMove<Header::MultiTrack_2_3_Frame::FrameSettings>(data);
		pixels.allocate(frameSettings.width, frameSettings.height, Header::MultiTrack_2_3_Frame::toOf(frameSettings.pixelFormat));
		auto size = pixels.getTotalBytes();
		memcpy(pixels.getData(), data, size);
		data += size;
	}

	//---------
	void NodeConnection::update() {
		this->newFrame = false;

		//wait up to 1/20th of a sec for a new frame
		this->receiver.update(chrono::milliseconds(1000 / 20));

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

					this->frame.bodies = readBodies(body);
				}
			}

			if (message.hasHeader<Header::MultiTrack_2_3_Frame>()) {
				this->newFrame = true;

				auto & header = message.getHeader<Header::MultiTrack_2_3_Frame>();
				
				auto body = (uint8_t*) message.getBodyData();
				{
					if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::Color) {
						readPixels(body, this->frame.color);
					}

					if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::Depth) {
						readPixels(body, this->frame.depth);
					}

					if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::Infrared) {
						readPixels(body, this->frame.infrared);
					}

					if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::BodyIndex) {
						readPixels(body, this->frame.bodyIndex);
					}

					if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::ColorCoordInDepthView) {
						readPixels(body, this->frame.colorCoordInDepthFrame);
					}

					if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::Bodies) {
						this->frame.bodies = readBodies(body);
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