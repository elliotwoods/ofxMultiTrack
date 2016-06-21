#pragma once

#include "ofMain.h"

#include "ofxKinectForWindows2.h"
#include "ofxMultiTrack.h"
#include "ofxCvGui.h"

#include <json/json.h>

class ofApp : public ofBaseApp{

	public:
		void setup();
		
		void setGuiMinimised(bool);

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

		bool guiMinimised = false;
		ofxCvGui::Controller * guiController;
		ofxCvGui::PanelGroupPtr bigGui;
		ofxCvGui::PanelGroupPtr smallGui;

		shared_ptr<ofxKinectForWindows2::Device> kinect;
		shared_ptr<ofxMachineVision::Grabber::Simple> grabber;
		ofxMultiTrack::SenderExtColor sender;

		bool droppedFrame = false;
};