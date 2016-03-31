#pragma once

#include "ofxKinectForWindows2.h"
#include "ofxMultiTrack/Frame.h"

namespace ofxMultiTrack {
	namespace Serialisation {
		void operator<<(ofxMultiTrack::Frame &, ofxKinectForWindows2::Device &);
	}
}