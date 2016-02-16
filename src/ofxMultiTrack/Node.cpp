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

	//----------
	bool Node::update() {
		this->kinect.update();
		if (this->kinect.isFrameNew()) {
			if (this->sender.getCurrentSocketBufferSize() < 100) {
				this->kinect.getColorSource()->getYuvPixels().resizeTo(this->frame.color, ofInterpolationMethod::OF_INTERPOLATE_NEAREST_NEIGHBOR);
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

				// TODO Skeletons

				if (!this->sender.send(this->message)) {
					return false;
				}
			}
		}

		return true;
	}

	//----------
	ofxKinectForWindows2::Device & Node::getKinect() {
		return this->kinect;
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
}