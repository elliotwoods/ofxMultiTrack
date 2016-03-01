#include "Node.h"

using namespace ofxSquashBuddies;

namespace ofxMultiTrack {
	//----------
	void Node::init(string ipAddress, int port) {
		this->sender.init(ipAddress, port);

		this->streams.color.enabled = true;
		this->streams.depth.enabled = true;
		this->streams.infrared.enabled = false;
		this->streams.bodyIndex.enabled = true;
		this->streams.colorCoordInDepthView.enabled = true;
		this->streams.bodies.enabled = true;

		this->setupKinect();

		//declare the streams
		{
			{
				auto & stream = this->streams.color;
				stream.frameSettings = {
					1920,
					720,
					Header::MultiTrack_2_3_Frame::PixelFormat::YUY2_8
				};
				stream.source = this->kinect.getColorSource();
			}
			{
				auto & stream = this->streams.depth;
				stream.frameSettings = {
					512,
					424,
					Header::MultiTrack_2_3_Frame::PixelFormat::L_16
				};
				stream.source = this->kinect.getDepthSource();
			}
			{
				auto & stream = this->streams.infrared;
				stream.frameSettings = {
					512,
					424,
					Header::MultiTrack_2_3_Frame::PixelFormat::L_16
				};
				stream.source = this->kinect.getInfraredSource();
			}
			{
				auto & stream = this->streams.bodyIndex;
				stream.frameSettings = {
					512,
					424,
					Header::MultiTrack_2_3_Frame::PixelFormat::L_8
				};
				stream.source = this->kinect.getBodyIndexSource();
			}
			{
				auto & stream = this->streams.colorCoordInDepthView;
				stream.frameSettings = {
					512,
					424,
					Header::MultiTrack_2_3_Frame::PixelFormat::RG_16
				};
				stream.source = this->kinect.getDepthSource();
			}
			{
				auto & stream = this->streams.bodies;
				stream.frameSettings = {
					512,
					424,
					Header::MultiTrack_2_3_Frame::PixelFormat::L_8
				};
				stream.source = this->kinect.getBodySource();
			}
		}

		this->setupMessage();
	}

	//---------
	template<typename DataType>
	void writeAndMove(uint8_t * & out, const DataType & in) {
		auto & outTyped = ((DataType * &)out);
		*outTyped++ = in;
	}

	//----------
	template<typename PixelType>
	void updateStream(ofPixels_<PixelType> & pixels, const Node::Stream & stream) {
		if (stream.enabled) {
			auto source = dynamic_pointer_cast<ofBaseHasPixels_<PixelType>>(stream.source);
			pixels = source->getPixels();
		}
	}

	//----------
	bool Node::update() {
		this->kinectFrameRateCounter.update(); 
		
		this->kinect.update();
		if (this->kinect.isFrameNew()) {
			this->kinectFrameRateCounter.addFrame();

			
			//color
			if (this->streams.color.enabled) {
				auto & inputPixels = this->kinect.getColorSource()->getYuvPixels();
				if (inputPixels.isAllocated()) {
					const auto inputWidth = inputPixels.getWidth();
					const auto inputHeight = inputPixels.getHeight();
					const auto outputWidth = this->streams.color.frameSettings.width;
					const auto outputHeight = this->streams.color.frameSettings.height;

					if (outputWidth == inputWidth && outputHeight == inputHeight) {
						//copy
						this->frame.color = inputPixels;
					}
					else {
						//resample
						if (inputWidth == 1920 && outputWidth == 1280 && inputHeight == 1080 && outputHeight == 720) {
							for (int j = 0; j < 720; j++) {
								auto input = inputPixels.getData() + 1920 * 2 * (j * 3 / 2);
								auto output = this->frame.color.getData() + 1280 * 2 * j;
								for (int i = 0; i < 1920; i += 2) {
									if (i % 6 < 4) {
										*output++ = *input++;
										*output++ = *input++;
										*output++ = *input++;
										*output++ = *input++;
									}
									else {
										*input++;
										*input++;
										*input++;
										*input++;
									}
								}
							}
						}
						else if (outputWidth == inputWidth) {
							for (int j = 0; j < outputHeight; j++) {
								auto outputRow = this->frame.color.getData() + outputWidth * 2 * j;
								auto inputRow = inputPixels.getData() + inputWidth * 2 * (j * inputHeight / outputHeight);
								memcpy(outputRow, inputRow, inputWidth * 2);
							}
						}
						else {
							ofLogError("ofxMultiTrack") << "Color resolution not supported for YUY2 resample";
						}
					}
				
				}
			}

			updateStream(this->frame.depth, this->streams.depth);
			updateStream(this->frame.infrared, this->streams.infrared);
			updateStream(this->frame.bodyIndex, this->streams.bodyIndex);

			//colorCoordInDepthView
			if (this->streams.colorCoordInDepthView.enabled && this->streams.color.enabled) {
				this->kinect.getDepthSource()->getColorInDepthFrameMapping(this->colorCoordInDepthFrameFloat);
				auto in = this->colorCoordInDepthFrameFloat.getData();
				auto out = this->frame.colorCoordInDepthFrame.getData();
				float widthScale = float(this->streams.color.frameSettings.width) / 1920.0f;
				float heightScale = float(this->streams.color.frameSettings.height) / 1080.0f;

				for (int i = 0; i < 512 * 424 * 2; i+=2) {
					*out++ = *in++ * widthScale;
					*out++ = *in++ * heightScale;
				}
			}

			//color preview
			{
				this->colorPreview.allocate(this->frame.color.getWidth(), this->frame.color.getHeight(), ofImageType::OF_IMAGE_COLOR);
				auto in = this->frame.color.getData();
				auto out = this->colorPreview.getPixels().getData();
				auto size = this->streams.color.frameSettings.width * this->streams.color.frameSettings.height;
				for (int i = 0; i < size; i+=2) {
					const float y1 = in[0];
					const float y2 = in[2];
					const float u = in[1];
					const float v = in[3];

					
					out[0] = ofClamp(y1 + 1.402 * (v - 128), 0, 255);
					out[1] = ofClamp(y1 - 0.344 * (u - 128) - 0.714 * (v - 128), 0, 255);
					out[2] = ofClamp(y1 + 1.772 * (u - 128), 0, 255);

					out[3] = ofClamp(y2 + 1.402 * (v - 128), 0, 255);
					out[4] = ofClamp(y2 - 0.344 * (u - 128) - 0.714 * (v - 128), 0, 255);
					out[5] = ofClamp(y2 + 1.772 * (u - 128), 0, 255);

					in += 4;
					out += 6;
				}
				this->colorPreview.update();
			}
			

			//skeletons
			{
				auto data = this->bodiesData;
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

	//----------
	void Node::setupKinect() {
		if (this->kinect.isOpen()) {
			this->kinect.close();
		}

		this->kinect.open();

		auto useTexture = true; /*HACK*/

		if (this->streams.color.enabled) {
			auto colorSource = this->kinect.initColorSource();
			colorSource->setYuvPixelsEnabled(true);
			colorSource->setUseTexture(useTexture);
		}
		

		if (this->streams.depth.enabled || this->streams.colorCoordInDepthView.enabled) {
			auto depthSource = this->kinect.initDepthSource();
			depthSource->setUseTexture(useTexture);
		}

		if (this->streams.infrared.enabled) {
			auto infraredSource = this->kinect.initInfraredSource();
			infraredSource->setUseTexture(useTexture);
			
		}

		if (this->streams.bodyIndex.enabled) {
			auto bodyIndexSource = this->kinect.initBodyIndexSource();
			bodyIndexSource->setUseTexture(useTexture);
		}

		if (this->streams.bodies.enabled) {
			this->kinect.initBodySource();
		}
	}

	//----------
	template<typename PixelType>
	void addStreamToMessage(ofPixels_<PixelType> & pixels, const Node::Stream & stream, uint8_t * & messageBodyPosition) {
		if (stream.enabled) {
			auto source = dynamic_pointer_cast<ofBaseDraws>(stream.source);

			auto & frameSettings = *(Header::MultiTrack_2_3_Frame::FrameSettings*) messageBodyPosition;
			frameSettings = stream.frameSettings;
			messageBodyPosition += sizeof(Header::MultiTrack_2_3_Frame::FrameSettings);

			pixels.setFromExternalPixels(
				(PixelType*) messageBodyPosition,
				frameSettings.width,
				frameSettings.height,
				Header::MultiTrack_2_3_Frame::toOf(frameSettings.pixelFormat));
			messageBodyPosition += pixels.getTotalBytes();
		}
	}
		

	//----------
	void Node::setupMessage() {
		//calculate the body size
		size_t bodySize = 0;
		{
			auto addImageStreamToBodySize = [&bodySize](const Stream & stream) {
				if (stream.enabled) {
					bodySize += sizeof(Header::MultiTrack_2_3_Frame::FrameSettings);

					const auto bitmapSize = stream.frameSettings.size();
					bodySize += bitmapSize;
				}
			};

			addImageStreamToBodySize(this->streams.color);
			addImageStreamToBodySize(this->streams.depth);
			addImageStreamToBodySize(this->streams.infrared);
			addImageStreamToBodySize(this->streams.bodyIndex);
			addImageStreamToBodySize(this->streams.colorCoordInDepthView);

			if (this->streams.bodies.enabled) {
				bodySize += Header::MultiTrack_2_3_Frame::Constants::SkeletonSize;
			}
		}

		//setup the message
		{
			auto headerSize = sizeof(Header::MultiTrack_2_3_Frame);
			this->message.resizeHeaderAndBody(headerSize + bodySize);

			//initialise the header
			auto & header = this->message.getHeader<Header::MultiTrack_2_3_Frame>(true);

			auto dataAvailable = 0;
			{
				if (this->streams.color.enabled) dataAvailable |= Header::MultiTrack_2_3_Frame::Color;
				if (this->streams.depth.enabled) dataAvailable |= Header::MultiTrack_2_3_Frame::Depth;
				if (this->streams.infrared.enabled) dataAvailable |= Header::MultiTrack_2_3_Frame::Infrared;
				if (this->streams.bodyIndex.enabled) dataAvailable |= Header::MultiTrack_2_3_Frame::BodyIndex;
				if (this->streams.colorCoordInDepthView.enabled) dataAvailable |= Header::MultiTrack_2_3_Frame::ColorCoordInDepthView;
				if (this->streams.bodies.enabled) dataAvailable |= Header::MultiTrack_2_3_Frame::Bodies;
			}
			header.dataAvailable = (Header::MultiTrack_2_3_Frame::DataAvailable) dataAvailable;
		}

		//wrap the message in pixel objects
		//this means that the memory is allocated in Message but can be accessed from each pixels
		{
			auto messageBodyPosition = (uint8_t*)this->message.getBodyData();
			
			addStreamToMessage(this->frame.color, this->streams.color, messageBodyPosition);
			addStreamToMessage(this->frame.depth, this->streams.depth, messageBodyPosition);
			addStreamToMessage(this->frame.infrared, this->streams.infrared, messageBodyPosition);
			addStreamToMessage(this->frame.bodyIndex, this->streams.bodyIndex, messageBodyPosition);
			addStreamToMessage(this->frame.colorCoordInDepthFrame, this->streams.colorCoordInDepthView, messageBodyPosition);

			this->bodiesData = messageBodyPosition;
		}
	}

}