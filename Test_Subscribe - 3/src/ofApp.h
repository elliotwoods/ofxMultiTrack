#pragma once

#include "ofMain.h"

#include "ofxMultiTrack.h"
#include "ofxCvGui.h"
#include "ofxOsc.h"

#define RECEIVER_COUNT 3
class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		ofxCvGui::Builder gui;

		struct Receiver {
			ofxMultiTrack::Subscriber subscriber;

			ofImage color;
			ofShortImage depth;
			ofShortImage infrared;
			ofImage bodyIndex;
			ofShortImage colorCoordInDepthFrame;

			ofFloatPixels depthToWorldTable;

			int clientIndex = 0;
			bool newFrame = false;

			vector<ofxSquashBuddies::DroppedFrame> droppedFrames;
		};
		
		Receiver receivers[RECEIVER_COUNT];
};
