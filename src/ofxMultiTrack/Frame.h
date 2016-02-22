#pragma once

#include "ofPixels.h" 
#include "ofxKinectForWindows2/Data/Body.h"

namespace ofxMultiTrack {
	struct Frame {
		ofPixels color;
		ofShortPixels depth;
		ofShortPixels infrared;
		ofPixels bodyIndex;
		ofShortPixels colorCoordInDepthFrame;

		vector<ofxKinectForWindows2::Data::Body> bodies;
	};
}
