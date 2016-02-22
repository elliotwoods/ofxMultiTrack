#include "Node.h"

using namespace ofxSquashBuddies;

namespace ofxMultiTrack {
	//----------
	void Node::init(string ipAddress, int port, bool useTexture) {
		//setup the sender
		{
			this->sender.init(ipAddress, port);
		}

		//setup the kinect
		{
			this->kinect.open();
			
			auto colorSource = this->kinect.initColorSource();
			colorSource->setYuvPixelsEnabled(true);
			colorSource->setUseTexture(useTexture);

			auto depthSource = this->kinect.initDepthSource();
			depthSource->setUseTexture(useTexture);

			auto bodyIndexSource = this->kinect.initBodyIndexSource();
			bodyIndexSource->setUseTexture(useTexture);

			auto infraredSource = this->kinect.initInfraredSource();
			infraredSource->setUseTexture(useTexture);

			this->kinect.initBodySource();
		}

		//setup the message
		{
			auto headerSize = sizeof(Header::MultiTrack_2_2_Frame);
			auto bodySize = (size_t)Header::MultiTrack_2_2_Frame::Constants::TotalDataSize;
			this->message.resizeHeaderAndBody(headerSize + bodySize);

			//initialise the header
			auto & header = this->message.getHeader<Header::MultiTrack_2_2_Frame>(true);
		}

		//wrap the message in pixel objects
		//this means that the memory is allocated in Message but can be accessed from each pixels
		{
			auto messageBodyPosition = (uint8_t*)this->message.getBodyData();
			this->frame.color.setFromExternalPixels(messageBodyPosition, 1280, 720, OF_PIXELS_YUY2);
			messageBodyPosition += Header::MultiTrack_2_2_Frame::Constants::ColorSize;

			this->frame.depth.setFromExternalPixels((uint16_t*)messageBodyPosition, 512, 424, OF_PIXELS_GRAY);
			messageBodyPosition += Header::MultiTrack_2_2_Frame::Constants::DepthSize;

			this->frame.infrared.setFromExternalPixels((uint16_t*)messageBodyPosition, 512, 424, OF_PIXELS_GRAY);
			messageBodyPosition += Header::MultiTrack_2_2_Frame::Constants::InfraredSize;

			this->frame.bodyIndex.setFromExternalPixels(messageBodyPosition, 512, 424, OF_PIXELS_GRAY);
			messageBodyPosition += Header::MultiTrack_2_2_Frame::Constants::BodyIndexSize;

			this->frame.colorCoordInDepthFrame.setFromExternalPixels((uint16_t*)messageBodyPosition, 512, 424, OF_PIXELS_RG);
			messageBodyPosition += Header::MultiTrack_2_2_Frame::Constants::ColorCoordInDepthViewSize;

			this->bodiesData = messageBodyPosition;
		}
	}

	//---------
	template<typename DataType>
	void writeAndMove(uint8_t * & out, const DataType & in) {
		auto & outTyped = ((DataType * &)out);
		*outTyped++ = in;
	}

	//----------
	bool Node::update() {
		this->kinectFrameRateCounter.update(); 
		
		this->kinect.update();
		if (this->kinect.isFrameNew()) {
			this->kinectFrameRateCounter.addFrame();
			//color pixels
			{
				auto & kinectColor = this->kinect.getColorSource()->getYuvPixels();
				if (kinectColor.isAllocated()) {
					for (int j = 0; j < 720; j++) {
						auto input = kinectColor.getData() + 1920 * 2 * (j * 3 / 2);
						auto output = this->frame.color.getData() + 1280 * 2 * j;
						for (int i = 0; i < 1280; i++) {
							*output++ = *input++;
							*output++ = *input++;

							*output++ = *input++;
							*output++ = *input++;

							input++;
							input++;
						}
					}
				}
			}
				
			this->frame.depth = this->kinect.getDepthSource()->getPixels();
			//this->frame.infrared = this->kinect.getInfraredSource()->getPixels();
			this->frame.bodyIndex = this->kinect.getBodyIndexSource()->getPixels();

			//get colorInDepth mapping
			{
				this->kinect.getDepthSource()->getColorInDepthFrameMapping(this->colorCoordInDepthFrameFloat, 1280, 720);
				auto coordinateMapper = this->kinect.getDepthSource()->getCoordinateMapper();
				auto in = this->colorCoordInDepthFrameFloat.getData();
				auto out = this->frame.colorCoordInDepthFrame.getData();
				for (int i = 0; i < 512 * 424 * 2; i++) {
					out[i] = in[i] * 32.0f;
				}
			}

			//skeletons
			{
				auto data = (uint8_t*) this->message.getBodyData();
				data += Header::MultiTrack_2_2_Frame::ColorSize
					+ Header::MultiTrack_2_2_Frame::DepthSize
					+ Header::MultiTrack_2_2_Frame::InfraredSize
					+ Header::MultiTrack_2_2_Frame::BodyIndexSize
					+ Header::MultiTrack_2_2_Frame::ColorCoordInDepthViewSize;

				auto bodies = this->kinect.getBodySource()->getBodies();
				
				writeAndMove(data, (uint8_t)bodies.size());
				for (const auto body : bodies) {
					const auto & joints = body.joints;
					writeAndMove(data, (uint8_t) body.tracked);
					writeAndMove(data, (uint8_t) body.bodyId);
					writeAndMove(data, (uint64_t) body.trackingId);
					writeAndMove(data, (uint8_t) body.leftHandState);
					writeAndMove(data, (uint8_t) body.rightHandState);
					writeAndMove(data, (uint8_t) joints.size());

					for (const auto & joint : joints) {
						writeAndMove(data, joint.second.getRawJoint());
						writeAndMove(data, joint.second.getRawJointOrientation());
					}
				}
			}

			if (!this->sender.send(this->message)) {
				return false;
			}
		}

		return true;
	}

	//----------
	ofxKinectForWindows2::Device & Node::getKinect() {
		return this->kinect;
	}

	//----------
	ofxSquashBuddies::Sender & Node::getSender() {
		return this->sender;
	}

	//----------
	void Node::setTexturesEnabled(bool texturesEnabled) {
		auto sources = this->kinect.getSources();
		for (auto source : sources) {
			auto textureSource = dynamic_pointer_cast<ofBaseHasTexture>(source);
			if (textureSource) {
				textureSource->setUseTexture(texturesEnabled);
			}
		}
	}

	//----------
	float Node::getDeviceFrameRate() const {
		return this->kinectFrameRateCounter.getFrameRate();
	}

}