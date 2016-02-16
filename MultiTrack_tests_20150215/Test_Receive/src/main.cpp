#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main() {
	ofGLFWWindowSettings windowSettings;
	windowSettings.setGLVersion(3, 2);
	windowSettings.width = 640 * 2;
	windowSettings.height = 480;
	auto window = ofCreateWindow(windowSettings);

	ofRunApp(new ofApp());

}
