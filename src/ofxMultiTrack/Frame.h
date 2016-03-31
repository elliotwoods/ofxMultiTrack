#pragma once

#include "ofPixels.h" 
#include "ofxKinectForWindows2.h"
#include "ofxSquashBuddies/Message.h"

#define MULTITRACK_FRAME_COLOR_WIDTH 1920
#define MULTITRACK_FRAME_COLOR_HEIGHT 720

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

		void init(ofxKinectForWindows2::Device &);
		void copyFromKinect();

		const ofxSquashBuddies::Message & getMessage() const;
	protected:
		ofxSquashBuddies::Message message;
		vector<Stream> streams;
	private:
		ofFloatPixels colorCoordInDepthFrameFloat;
	};
}
