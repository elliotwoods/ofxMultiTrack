#include "Frame.h"

using namespace ofxKinectForWindows2;
using namespace ofxSquashBuddies;

namespace ofxMultiTrack {
#pragma mark Frame
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
	template<typename DataType>
	void writeAndMove(uint8_t * & out, const DataType & in) {
		auto & outTyped = ((DataType * &)out);
		*outTyped++ = in;
	}

	//----------
	void Frame::serialize(ofxSquashBuddies::Message & message) const {
		//--
		// Gather the streams
		//--
		//
		map<Header::MultiTrack_2_3_Frame::DataType, Header::MultiTrack_2_3_Frame::FrameSettings> streams;
		{
			if (this->color.isAllocated()) {
				Header::MultiTrack_2_3_Frame::FrameSettings frameSettings = {
					this->color.getWidth(),
					this->color.getHeight(),
					Header::MultiTrack_2_3_Frame::PixelFormat::YUY2_8
				};
				streams[Header::MultiTrack_2_3_Frame::Color] = frameSettings;
			}
			if (this->depth.isAllocated()) {
				Header::MultiTrack_2_3_Frame::FrameSettings frameSettings = {
					this->depth.getWidth(),
					this->depth.getHeight(),
					Header::MultiTrack_2_3_Frame::PixelFormat::L_16
				};
				streams[Header::MultiTrack_2_3_Frame::Depth] = frameSettings;
			}
			if (this->infrared.isAllocated()) {
				Header::MultiTrack_2_3_Frame::FrameSettings frameSettings = {
					this->infrared.getWidth(),
					this->infrared.getHeight(),
					Header::MultiTrack_2_3_Frame::PixelFormat::L_16
				};
				streams[Header::MultiTrack_2_3_Frame::Infrared] = frameSettings;
			}
			if (this->bodyIndex.isAllocated()) {
				Header::MultiTrack_2_3_Frame::FrameSettings frameSettings = {
					this->bodyIndex.getWidth(),
					this->bodyIndex.getHeight(),
					Header::MultiTrack_2_3_Frame::PixelFormat::L_8
				};
				streams[Header::MultiTrack_2_3_Frame::BodyIndex] = frameSettings;
			}
			if (this->colorCoordInDepthFrame.isAllocated()) {
				Header::MultiTrack_2_3_Frame::FrameSettings frameSettings = {
					this->colorCoordInDepthFrame.getWidth(),
					this->colorCoordInDepthFrame.getHeight(),
					Header::MultiTrack_2_3_Frame::PixelFormat::RG_16
				};
				streams[Header::MultiTrack_2_3_Frame::ColorCoordInDepthView] = frameSettings;
			}
			if (!this->bodies.empty()) {
					Header::MultiTrack_2_3_Frame::FrameSettings frameSettings = {
					0,
					0,
					Header::MultiTrack_2_3_Frame::PixelFormat::Unknown
				};
				streams[Header::MultiTrack_2_3_Frame::Bodies] = frameSettings;
			}
		}
		//
		//--



		//--
		// Setup the message
		//--
		//
		size_t bodySize = 0;
		int dataAvailable;
		for (const auto & stream : streams) {
			dataAvailable |= stream.first;

			if (stream.first == Header::MultiTrack_2_3_Frame::Bodies) {
				bodySize += Header::MultiTrack_2_3_Frame::Constants::SkeletonSize;
			}
			else {
				bodySize += sizeof(Header::MultiTrack_2_3_Frame::FrameSettings);
				bodySize += stream.second.size();
			}
		}

		{
			auto headerSize = sizeof(Header::MultiTrack_2_3_Frame);
			message.resizeHeaderAndBody(headerSize + bodySize);

			//initialise the header
			auto & header = message.getHeader<Header::MultiTrack_2_3_Frame>(true);
			header.dataAvailable = (Header::MultiTrack_2_3_Frame::DataAvailable) dataAvailable;
		}
		//
		//--



		//--
		// Write the message
		//--
		//
		auto bodyDataPosition = (uint8_t *) message.getBodyData();
		for (const auto & stream : streams) {

			//write the image
			if (stream.first != Header::MultiTrack_2_3_Frame::Bodies) {
				//write the pixels
				switch (stream.first) {
				case Header::MultiTrack_2_3_Frame::Color:
					Frame::writePixelsToData(bodyDataPosition, this->color, stream.second);
					break;
				case Header::MultiTrack_2_3_Frame::Depth:
					Frame::writePixelsToData(bodyDataPosition, this->depth, stream.second);
					break;
				case Header::MultiTrack_2_3_Frame::Infrared:
					Frame::writePixelsToData(bodyDataPosition, this->infrared, stream.second);
					break;
				case Header::MultiTrack_2_3_Frame::BodyIndex:
					Frame::writePixelsToData(bodyDataPosition, this->bodyIndex, stream.second);
					break;
				case Header::MultiTrack_2_3_Frame::ColorCoordInDepthView:
					Frame::writePixelsToData(bodyDataPosition, this->colorCoordInDepthFrame, stream.second);
					break;
				}
			}
			//write the bodies
			else {
				Frame::writeBodiesToData(bodyDataPosition, this->bodies);
			}
		}
		//
		//--
	}

	//----------
	bool Frame::deserialize(const ofxSquashBuddies::Message & message) {
		if (message.hasHeader<Header::MultiTrack_2_3_Frame>()) {
			auto & header = message.getHeader<Header::MultiTrack_2_3_Frame>();

			auto body = (uint8_t*)message.getBodyData();
			{
				if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::Color) {
					body = readPixelsFromData(body, this->color);
				}

				if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::Depth) {
					body = readPixelsFromData(body, this->depth);
				}

				if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::Infrared) {
					body = readPixelsFromData(body, this->infrared);
				}

				if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::BodyIndex) {
					body = readPixelsFromData(body, this->bodyIndex);
				}

				if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::ColorCoordInDepthView) {
					body = readPixelsFromData(body, this->colorCoordInDepthFrame);
				}

				if (header.dataAvailable & Header::MultiTrack_2_3_Frame::DataAvailable::Bodies) {
					readBodiesFromData(body, this->bodies);
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

	//---------
	uint8_t * Frame::readBodiesFromData(uint8_t * data, vector<ofxKinectForWindows2::Data::Body> & bodies) {
		auto skeletonCount = readAndMove<uint8_t>(data);

		bodies = vector<ofxKinectForWindows2::Data::Body>(skeletonCount);
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
				auto jointInDepthSpace = readAndMove<ofVec2f>(data);
				joints.emplace(make_pair(joint.JointType, ofxKinectForWindows2::Data::Joint(joint, jointOrientation, jointInDepthSpace)));
			}

			bodies.push_back(body);
		}

		return data;
	}

	//---------
	uint8_t * Frame::writeBodiesToData(uint8_t * data, const vector<ofxKinectForWindows2::Data::Body> & bodies) {
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
				writeAndMove(data, (_Joint) joint.second.getRawJoint());
				writeAndMove(data, (_JointOrientation) joint.second.getRawJointOrientation());
				writeAndMove(data, (ofVec2f) joint.second.getPositionInDepthMap());
			}
		}

		return data;
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
					source->setYuvPixelsEnabled(true);
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
					data = Frame::writeBodiesToData(data, bodies);
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
