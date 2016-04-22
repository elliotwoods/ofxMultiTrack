#pragma once

#include "ofMain.h"

#include "ofxMultiTrack.h"
#include "ofxCvGui.h"
#include "ofxSpout.h"
#include "ofxOsc.h"

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

		ofxMultiTrack::Subscriber subscriber;

		struct Texture {
			ofTexture texture;
			ofxSpout::Sender sender;

			void send() {
				this->sender.send(this->texture);
			}
		};
		Texture color;
		Texture depth;
		Texture infrared;
		Texture bodyIndex;
		Texture colorCoordInDepthFrame;

		ofFloatPixels depthToWorldTable;

		int clientIndex = 0;
		bool newFrame = false;
		
		ofxOscSender oscSender;
		int oscPort;
		string oscHost = "127.0.0.1";
};
