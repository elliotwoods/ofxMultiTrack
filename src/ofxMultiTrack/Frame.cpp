#include "Frame.h"

using namespace ofxKinectForWindows2;
using namespace ofxSquashBuddies;

namespace ofxMultiTrack {
#pragma mark Frame
	//----------
	void Frame::serialize(ofxSquashBuddies::Message &) const {

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

	//----------
	bool Frame::deserialize(const ofxSquashBuddies::Message & message) {
		if (message.hasHeader<Header::MultiTrack_2_3_Frame>()) {
			auto & header = message.getHeader<Header::MultiTrack_2_3_Frame>();

			auto body = (uint8_t*)message.getBodyData();
			{
				if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::Color) {
					readPixels(body, this->color);
				}

				if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::Depth) {
					readPixels(body, this->depth);
				}

				if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::Infrared) {
					readPixels(body, this->infrared);
				}

				if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::BodyIndex) {
					readPixels(body, this->bodyIndex);
				}

				if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::ColorCoordInDepthView) {
					readPixels(body, this->colorCoordInDepthFrame);
				}

				if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::Bodies) {
					this->bodies = readBodies(body);
				}
			}

			return true;
		} else {
			return false;
		}
	}

	//----------
	const ofPixels & Frame::getColor() const {
		return this->color;
	}

	//----------
	const ofShortPixels & Frame::getDepth() const {
		return this->depth;
	}

	//----------
	const ofShortPixels & Frame::getInfrared() const {
		return this->infrared;
	}

	//----------
	const ofPixels & Frame::getBodyIndex() const {
		return this->bodyIndex;
	}

	//----------
	const ofShortPixels & Frame::getColorCoordInDepthFrame() const {
		return this->colorCoordInDepthFrame;
	}

	//----------
	const vector<ofxKinectForWindows2::Data::Body> Frame::getBodies() const {
		return this->bodies;
	}

#pragma mark DeviceFrame
	//----------
	template<typename PixelType>
	void entanglePixelsWithMessage(ofPixels_<PixelType> & pixels, const DeviceFrame::Stream & stream, uint8_t * & messageBodyPosition) {
		auto source = dynamic_pointer_cast<ofBaseDraws>(stream.source);

		auto & frameSettings = *(Header::MultiTrack_2_3_Frame::FrameSettings*) messageBodyPosition;
		frameSettings = stream.frameSettings;
		messageBodyPosition += sizeof(Header::MultiTrack_2_3_Frame::FrameSettings);

		pixels.setFromExternalPixels(
			(PixelType*)messageBodyPosition,
			frameSettings.width,
			frameSettings.height,
			Header::MultiTrack_2_3_Frame::toOf(frameSettings.pixelFormat));
		messageBodyPosition += pixels.getTotalBytes();
	}

	//----------
	void DeviceFrame::init(ofxKinectForWindows2::Device & device) {
		//--
		// Declare the streams
		//--
		//
		this->streams.clear();
		int dataAvailable = 0;
		{
			//color
			{
				auto source = device.getColorSource();
				if (source) {
					auto dataType = Header::MultiTrack_2_3_Frame::Color;
					dataAvailable |= dataType;

					Header::MultiTrack_2_3_Frame::FrameSettings frameSettings = {
						MULTITRACK_FRAME_COLOR_WIDTH,
						MULTITRACK_FRAME_COLOR_HEIGHT,
						Header::MultiTrack_2_3_Frame::PixelFormat::YUY2_8
					};
					Stream stream = {
						dataType,
						frameSettings,
						source,
						0,
						frameSettings.size()
					};
					this->streams.push_back(stream);
				}
			}

			//depth
			{
				auto source = device.getDepthSource();
				if (source) {
					auto dataType = Header::MultiTrack_2_3_Frame::Depth;
					dataAvailable |= dataType;

					Header::MultiTrack_2_3_Frame::FrameSettings frameSettings = {
						512,
						424,
						Header::MultiTrack_2_3_Frame::PixelFormat::L_16,
					};
					Stream stream = {
						dataType,
						frameSettings,
						source,
						0,
						frameSettings.size()
					};
					this->streams.push_back(stream);
				}
			}

			//infrared
			{
				auto source = device.getInfraredSource();
				if (source) {
					auto dataType = Header::MultiTrack_2_3_Frame::Infrared;
					dataAvailable |= dataType;

					Header::MultiTrack_2_3_Frame::FrameSettings frameSettings = {
						512,
						424,
						Header::MultiTrack_2_3_Frame::PixelFormat::L_16
					};
					Stream stream = {
						dataType,
						frameSettings,
						source,
						0,
						frameSettings.size()
					};
					this->streams.push_back(stream);
				}
			}

			//body index
			{
				auto source = device.getBodyIndexSource();
				if (source) {
					auto dataType = Header::MultiTrack_2_3_Frame::BodyIndex;
					dataAvailable |= dataType;

					Header::MultiTrack_2_3_Frame::FrameSettings frameSettings = {
						512,
						424,
						Header::MultiTrack_2_3_Frame::PixelFormat::L_8
					};
					Stream stream = {
						dataType,
						frameSettings,
						source,
						0,
						frameSettings.size()
					};
					this->streams.push_back(stream);
				}
			}

			//color coord in depth view
			{
				auto depthSource = device.getDepthSource();
				auto colorSource = device.getColorSource();
				if (depthSource && colorSource) {
					auto dataType = Header::MultiTrack_2_3_Frame::ColorCoordInDepthView;
					dataAvailable |= dataType;

					Header::MultiTrack_2_3_Frame::FrameSettings frameSettings = {
						512,
						424,
						Header::MultiTrack_2_3_Frame::PixelFormat::RG_16
					};
					Stream stream = {
						dataType,
						frameSettings,
						depthSource,
						0,
						frameSettings.size()
					};
					this->streams.push_back(stream);
				}
			}

			//bodies
			{
				auto source = device.getBodySource();
				if (source) {
					auto dataType = Header::MultiTrack_2_3_Frame::Bodies;
					dataAvailable |= dataType;

					Header::MultiTrack_2_3_Frame::FrameSettings frameSettings = {
						0,
						0,
						Header::MultiTrack_2_3_Frame::PixelFormat::Unknown
					};
					Stream stream = {
						dataType,
						frameSettings,
						source,
						0,
						Header::MultiTrack_2_3_Frame::Constants::SkeletonSize
					};
					this->streams.push_back(stream);
				}
			}
		}
		//
		//--


		//calculate the body size
		size_t bodySize = 0;
		{
			for (const auto & stream : this->streams) {
				if (stream.dataType != Header::MultiTrack_2_3_Frame::DataType::Bodies) {
					//all image sources also need a header for the image as well as the data
					bodySize += sizeof(Header::MultiTrack_2_3_Frame::FrameSettings);
				}
				bodySize += stream.size;
			}
		}

		//setup the message
		{
			auto headerSize = sizeof(Header::MultiTrack_2_3_Frame);
			this->message.resizeHeaderAndBody(headerSize + bodySize);

			//initialise the header
			auto & header = this->message.getHeader<Header::MultiTrack_2_3_Frame>(true);
			header.dataAvailable = (Header::MultiTrack_2_3_Frame::DataAvailable) dataAvailable;
		}

		//wrap the message in pixel objects
		//this means that the memory is allocated in Message but can be accessed from each pixels
		{
			auto messageBodyPosition = (uint8_t*)this->message.getBodyData();
			for (auto & stream : this->streams) {
				stream.data = messageBodyPosition;

				switch (stream.dataType) {
				case Header::MultiTrack_2_3_Frame::Color:
					entanglePixelsWithMessage(this->color, stream, messageBodyPosition);
					break;
				case Header::MultiTrack_2_3_Frame::Depth:
					entanglePixelsWithMessage(this->depth, stream, messageBodyPosition);
					break;
				case Header::MultiTrack_2_3_Frame::Infrared:
					entanglePixelsWithMessage(this->infrared, stream, messageBodyPosition);
					break;
				case Header::MultiTrack_2_3_Frame::BodyIndex:
					entanglePixelsWithMessage(this->bodyIndex, stream, messageBodyPosition);
					break;
				case Header::MultiTrack_2_3_Frame::ColorCoordInDepthView:
					entanglePixelsWithMessage(this->colorCoordInDepthFrame, stream, messageBodyPosition);
					break;
				case Header::MultiTrack_2_3_Frame::Bodies:
					break;
				}
			}

		}
	}

	//----------
	template<typename PixelType>
	void copyImageStream(ofPixels_<PixelType> & pixels, const DeviceFrame::Stream & stream) {
		auto source = dynamic_pointer_cast<ofBaseHasPixels_<PixelType>>(stream.source);
		if (source) {
			pixels = source->getPixels();
		}
	}

	//---------
	template<typename DataType>
	void writeAndMove(uint8_t * & out, const DataType & in) {
		auto & outTyped = ((DataType * &)out);
		*outTyped++ = in;
	}

	//----------
	void DeviceFrame::copyFromKinect() {
		//for each active stream, copy from the device source to the local pixels object
		for (auto & stream : this->streams) {
			switch (stream.dataType) {
			case Header::MultiTrack_2_3_Frame::Color:
			{
				auto source = dynamic_pointer_cast<Source::Color>(stream.source);
				if (source) {
					const auto & inputPixels = source->getYuvPixels();
					if (inputPixels.isAllocated()) {
						const auto inputWidth = inputPixels.getWidth();
						const auto inputHeight = inputPixels.getHeight();
						const auto outputWidth = stream.frameSettings.width;
						const auto outputHeight = stream.frameSettings.height;

						if (outputWidth == inputWidth && outputHeight == inputHeight) {
							//copy if dimensions are the same
							copyImageStream(this->color, stream);
						}
						else {
							//resample if dimensions are different
							if (outputWidth == inputWidth) {
								//resample rows only
								for (int j = 0; j < outputHeight; j++) {
									auto outputRow = this->color.getData() + outputWidth * 2 * j;
									auto inputRow = inputPixels.getData() + inputWidth * 2 * (j * inputHeight / outputHeight);
									memcpy(outputRow, inputRow, inputWidth * 2);
								}
							}
							else {
								//because YUV is quite special, we don't want to resample in any other case
								ofLogError("ofxMultiTrack") << "Color resolution not supported for YUY2 resample";
							}
						}
					}
				}

				break;
			}

			case Header::MultiTrack_2_3_Frame::Depth:
			{
				copyImageStream(this->depth, stream);
				break;
			}
			case Header::MultiTrack_2_3_Frame::Infrared:
			{
				copyImageStream(this->infrared, stream);
				break;
			}
			case Header::MultiTrack_2_3_Frame::BodyIndex:
			{
				copyImageStream(this->bodyIndex, stream);
				break;
			}
			case Header::MultiTrack_2_3_Frame::ColorCoordInDepthView:
			{
				auto depthSource = dynamic_pointer_cast<Source::Depth>(stream.source);
				if (depthSource) {
					depthSource->getColorInDepthFrameMapping(this->colorCoordInDepthFrameFloat);
					auto in = this->colorCoordInDepthFrameFloat.getData();
					auto out = this->colorCoordInDepthFrame.getData();
					float widthScale = float(MULTITRACK_FRAME_COLOR_WIDTH) / 1920.0f;
					float heightScale = float(MULTITRACK_FRAME_COLOR_HEIGHT) / 1080.0f;

					for (int i = 0; i < 512 * 424; i++) {
						*out++ = (uint16_t) (*in++ * widthScale);
						*out++ = (uint16_t) (*in++ * heightScale);
					}
				}
				break;
			}
			case Header::MultiTrack_2_3_Frame::Bodies:
			{
				auto source = dynamic_pointer_cast<Source::Body>(stream.source);
				if (source) {
					auto data = (uint8_t*)stream.data;
					auto bodies = source->getBodies();

					writeAndMove(data, (uint8_t)bodies.size());
					for (const auto body : bodies) {
						const auto & joints = body.joints;
						writeAndMove(data, (uint8_t)body.tracked);
						writeAndMove(data, (uint8_t)body.bodyId);
						writeAndMove(data, (uint64_t)body.trackingId);
						writeAndMove(data, (uint8_t)body.leftHandState);
						writeAndMove(data, (uint8_t)body.rightHandState);
						writeAndMove(data, (uint8_t)joints.size());

						for (const auto & joint : joints) {
							writeAndMove(data, joint.second.getRawJoint());
							writeAndMove(data, joint.second.getRawJointOrientation());
						}
					}
				}
				break;
			}
			default:
				break;
			}
		}
	}


	//----------
	const ofxSquashBuddies::Message & DeviceFrame::getMessage() const {
		return this->message;
	}
}
