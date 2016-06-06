#pragma once

#include "ofPixels.h"
#include "ofxKinectForWindows2.h"
#include "ofxMachineVision.h"
#include "ofxSquashBuddies/Message.h"

#define MULTITRACK_FRAME_COLOR_WIDTH 1920
#define MULTITRACK_FRAME_COLOR_HEIGHT 720

#define MULTITRACK_FRAME_EXT_COLOR_WIDTH 1280
#define MULTITRACK_FRAME_EXT_COLOR_HEIGHT 720

namespace ofxMultiTrack {
	class Frame {
	public:
		void serialize(ofxSquashBuddies::Message &) const;
		bool deserialize(const ofxSquashBuddies::Message &);

		const ofPixels & getColor() const;
		const ofShortPixels & getDepth() const;
		const ofShortPixels & getInfrared() const;
		const ofPixels & getBodyIndex() const;
		const ofShortPixels & getColorCoordInDepthFrame() const;
		const vector<ofxKinectForWindows2::Data::Body> getBodies() const;

		template<typename PixelType>
		static uint8_t * readPixelsFromData(uint8_t * data, ofPixels_<PixelType> & pixels) {
			auto frameSettings = readAndMove<Header::MultiTrack_2_3_Frame::FrameSettings>(data);
			pixels.allocate(frameSettings.width, frameSettings.height, Header::MultiTrack_2_3_Frame::toOf(frameSettings.pixelFormat));
			auto size = pixels.getTotalBytes();
			memcpy(pixels.getData(), data, size);
			data += size;
			return data;
		}
		template<typename PixelType>
		static uint8_t * writePixelsToData(uint8_t * data, const ofPixels_<PixelType> & pixels, const ofxSquashBuddies::Header::MultiTrack_2_3_Frame::FrameSettings & frameSettings) {
			*(ofxSquashBuddies::Header::MultiTrack_2_3_Frame::FrameSettings*) data = frameSettings;
			data += sizeof(frameSettings);

			memcpy(data, pixels.getData(), frameSettings.size());
			data += frameSettings.size();

			return data;
		}

		static uint8_t * readBodiesFromData(uint8_t * data, vector<ofxKinectForWindows2::Data::Body> &);
		static uint8_t * writeBodiesToData(uint8_t * data, const vector<ofxKinectForWindows2::Data::Body> &);
	protected:
		ofPixels color;
		ofShortPixels depth;
		ofShortPixels infrared;
		ofPixels bodyIndex;
		ofShortPixels colorCoordInDepthFrame;

		vector<ofxKinectForWindows2::Data::Body> bodies;
	};

	class DeviceFrame : public Frame {
	public:
		struct Stream {
			ofxSquashBuddies::Header::MultiTrack_2_3_Frame::DataType dataType;
			ofxSquashBuddies::Header::MultiTrack_2_3_Frame::FrameSettings frameSettings;
			shared_ptr<ofxKinectForWindows2::Source::Base> source;
			void * data;
			size_t size;
		};

		void init(shared_ptr<ofxKinectForWindows2::Device>);
		virtual void initColor(shared_ptr<ofxKinectForWindows2::Device>, int &);  // Color is special because it can be external
		virtual void initColorCoordInDepthView(shared_ptr<ofxKinectForWindows2::Device>, int &);

		virtual void copyFromKinect();

		const ofxSquashBuddies::Message & getMessage() const;
	protected:
		ofxSquashBuddies::Message message;
		vector<Stream> streams;
	private:
		ofFloatPixels colorCoordInDepthFrameFloat;
	};

	class ComboFrame : public DeviceFrame {
	public:
		void init(shared_ptr<ofxKinectForWindows2::Device>, shared_ptr<ofxMachineVision::Grabber::Simple>);
		virtual void initColor(shared_ptr<ofxKinectForWindows2::Device>, int &) override;
		virtual void initColorCoordInDepthView(shared_ptr<ofxKinectForWindows2::Device>, int &) override;
		
		void setCameraParams(const vector<float> & distortion, const ofMatrix4x4 & view, const ofMatrix4x4 & projection);

		virtual void copyFromKinect() override;
		void copyFromGrabber();
	protected:
		ofVec2f getUndistorted(const ofVec2f & point);

		shared_ptr<ofxMachineVision::Grabber::Simple> grabber;

		ofFloatPixels depthToWorldTable;
		vector<float> distortion;
		ofMatrix4x4 viewProjection;
	};
}
