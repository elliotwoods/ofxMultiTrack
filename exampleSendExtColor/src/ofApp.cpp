#include "ofApp.h"

#include "ofxSquash.h"

//--------------------------------------------------------------
void ofApp::setup() {

	this->gui.init();
	this->bigGui = gui.getController().getRootGroup();
	this->guiController = & gui.getController();

	auto widgets = this->gui.addWidgets();
	widgets->addTitle("MultiTrack Send");
	widgets->addFps();
	widgets->addLiveValueHistory("Kinect device framerate", [this]() {
		return this->sender.getDeviceFrameRate();
	});
	widgets->addLiveValueHistory("Sending framerate", [this]() {
		return this->sender.getSender().getSendFramerate();
	});
	widgets->addLiveValueHistory("Dropped frame", [this]() {
		return (float) this->droppedFrame;
	});
	widgets->addEditableValue<int>("Packet size", [this]() {
		return (int) this->sender.getSender().getPacketSize();
	}, [this](string newSizeString) {
		if (!newSizeString.empty()) {
			this->sender.getSender().setPacketSize(ofToInt(newSizeString));
		}
	});
	widgets->addLiveValue<size_t>("Current socket buffer size", [this]() {
		return this->sender.getSender().getCurrentSocketBufferSize();
	});
	widgets->addEditableValue<int>("Max socket buffer size", [this]() {
		return (int) this->sender.getSender().getMaxSocketBufferSize();
	}, [this](string newSizeString) {
		if (!newSizeString.empty()) {
			this->sender.getSender().setMaxSocketBufferSize(ofToInt(newSizeString));
		}
	});
	widgets->addButton("Save Depth To World Table", [this]() {
		string filename = "DepthToWorld.raw";
		{
			auto result = ofSystemTextBoxDialog("Enter filename [" + filename + "]");
			if (!result.empty()) {
				filename = result;
			}

			ofFloatPixels depthToWorldTable;
			kinect->getDepthSource()->getDepthToWorldTable(depthToWorldTable);
			ofxSquashBuddies::Message message;
			message.setData(depthToWorldTable);
			const auto & saveString = message.getMessageString();
			ofBuffer saveBuffer(saveString);
			if (!ofBufferToFile(ofToDataPath(filename), saveBuffer)) {
				ofSystemAlertDialog("Error saving table to disk!");
			}
		}
	});
	widgets->addToggle("Minimise GUI",
		[this]() {
		return guiMinimised;
	}, [this](bool minimise) {
		this->setGuiMinimised(minimise);
	});

	bool autoStart = false;

	//get sender settings
	{
		string ipAddress = "127.0.0.1";
		auto port = 4444;

		//load settings file
		string settingsFileName = "settings.txt";
		{
			auto settingsFile = ifstream(ofToDataPath(settingsFileName).c_str(), ios::in);
			int lineNumber = 0;
			string line;
			while(getline(settingsFile, line)) {
				//check if it's not just white space
				if (line.find_first_not_of("\t\n\r ") != string::npos) {
					switch (lineNumber) {
					case 0:
						ipAddress = line;
						break;
					case 1:
						port = ofToInt(line);
						break;
					case 2:
						if (line.size() > 0) {
							autoStart = true;
						}
						break;
					default:
						break;
					}
				}
				lineNumber++;
			}
		}

		if (!autoStart) {
			bool userMadeChange = false;

			{
				auto result = ofSystemTextBoxDialog("Select remote IP (Default is " + ipAddress + ")");
				if (!result.empty()) {
					ipAddress = result;
					userMadeChange = true;
				}
			}
			{
				auto result = ofSystemTextBoxDialog("Select remote port (Default is " + ofToString(port) + ")");
				if (!result.empty()) {
					port = ofToInt(result);
					userMadeChange = true;
				}
			}

			if (userMadeChange) {
				auto settingsFile = ofstream(ofToDataPath(settingsFileName).c_str(), ios::out | ios::trunc);
				settingsFile << ipAddress << endl;
				settingsFile << port << endl;
				settingsFile.close();
			}
		}

		//setup grabber
		{
			auto device = make_shared<ofxMachineVision::Device::Webcam>();
			auto settings = make_shared<ofxMachineVision::Device::Webcam::InitialisationSettings>();
			settings->width = MULTITRACK_FRAME_EXT_COLOR_WIDTH;
			settings->height = MULTITRACK_FRAME_EXT_COLOR_HEIGHT;
			this->grabber = make_shared<ofxMachineVision::Grabber::Simple>();
			this->grabber->setDevice(device);
			this->grabber->open(settings);
			this->grabber->startCapture();
		}

		//setup Kinect
		{
			this->kinect = make_shared<ofxKinectForWindows2::Device>();
			this->kinect->open();
			this->kinect->initColorSource();
			this->kinect->initDepthSource();
			this->kinect->initInfraredSource();
			this->kinect->initBodyIndexSource();
			this->kinect->initBodySource();
		}

		//setup Sender
		{
			vector<float> distortion;
			distortion.push_back(0.0940593);
			distortion.push_back(-0.237928);
			distortion.push_back(0.00275671);
			distortion.push_back(-0.000131951);
			ofMatrix4x4 view = ofMatrix4x4(-0.999797, -0.0118014, -0.0163298, 0,
				0.0127842, -0.998028, -0.0614491, 0,
				-0.0155724, -0.0616454, 0.997977, 0,
				-0.0251298, 0.0330899, 0.119725, 1);
			ofMatrix4x4 projection = ofMatrix4x4(1.51642, 0, 0, 0,
				0, -2.69132, 0, 0,
				0.0231383, 0.0115212, 1.00669, 1,
				0, 0, -0.100334, 0);

			//initialise the ofxMultiTrack::Sender
			this->sender.init(this->kinect, this->grabber, ipAddress, port);
			this->sender.setCameraParams(distortion, view, projection);

			ofSetWindowTitle("Sending to : " + ipAddress + ":" + ofToString(port));
		}

		//build the gui
		{
			//grabber panel
			auto panel = ofxCvGui::Panels::makeTexture(this->grabber->getTexture(), this->grabber->getDeviceTypeName());
			this->gui.add(panel); 
			
			//Kinect panels
			auto sources = this->kinect->getSources();
			for (auto source : sources) {
				auto imageSource = dynamic_pointer_cast<ofBaseHasTexture>(source);
				if (imageSource) {
					auto panel = ofxCvGui::Panels::makeTexture(imageSource->getTexture(), source->getTypeName());
					//if (dynamic_pointer_cast<ofxKinectForWindows2::Source::Depth>(source)) {
					//	panel->setInputRange(0, 8000.0f / (float)0xffff);
					//}
					//if (dynamic_pointer_cast<ofxKinectForWindows2::Source::Infrared>(source)) {
					//	panel->setInputRange(0, 1024.0f / (float)0xffff);
					//}
					this->gui.add(panel);
				}
			}
		}
	}

	//make a small gui for when we're 'minimised'
	this->smallGui = ofxCvGui::Panels::Groups::makeGrid();
	this->smallGui->add(widgets);
	this->setGuiMinimised(autoStart); //if autostart, then start minimised

	ofSetFrameRate(60);
}

//--------------------------------------------------------------
void ofApp::setGuiMinimised(bool minimised) {
	this->guiMinimised = minimised;
	if (minimised) {
		this->kinect->setUseTextures(false);
		this->guiController->setRootGroup(this->smallGui);
		ofSetWindowShape(300, 500);
	}
	else {
		this->kinect->setUseTextures(true);
		this->guiController->setRootGroup(this->bigGui);
		ofSetWindowShape(640 * 3, 480 * 2);
	}
}

//--------------------------------------------------------------
void ofApp::update(){
	this->grabber->update();
	this->kinect->update();
	this->droppedFrame = ! this->sender.update();
}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
