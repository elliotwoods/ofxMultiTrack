#pragma once

#include "ofPixels.h" 

namespace ofxMultiTrack {
	struct Frame {
		ofPixels color;
		ofShortPixels depth;
		ofShortPixels infrared;
		ofPixels bodyIndex;
		ofShortPixels colorCoordInDepthFrame;
	};
}
