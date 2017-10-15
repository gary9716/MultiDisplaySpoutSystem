/*

	Spout OpenFrameworks Receiver example

    Visual Studio using the Spout SDK

    Search for SPOUT for additions to a typical Openframeworks application

	Copyright (C) 2017 Lynn Jarvis.

	03.08.15 - Created project
	10.01.17 - Update for Spout 2.006

	=========================================================================
	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
	=========================================================================

	adapted by Kuan-Ting Chou on 2017-04-22 for NTMOFA art exhibition
*/
//Basically, this is a spout receiver and bezier warping app for panoramic projection(9 projectors). 
//This app has been proven that it can receive an opengl texture with size 15390 * 1200, retrieve part of it, warp an retrieved part and project via one of 9 projectors. 
//so it totally draw on 9 displays(projectors), each with an resolution of 1920 * 1200. Additionally, it run an Vive VR application. 
//All these compuation has been done in real time on a single computer.
//the computer's spec are: 
//CPU: i7 7700K, 
//GPU: GTX1060 Mini ITX OC, GTX1050Ti, ASUS HD7750

#include "ofApp.h"
#include "displayApp.h"

string modeName[] = {
	"Demo",
	"Calibration"
};

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(0, 0, 0);
	ofSetFrameRate(60);

	ofSetWindowTitle("Main App"); // Set the window title to show that it is a Spout Receiver
	bInitialized = false; // Spout receiver initialization
	areWindowsSetup = false;

	winWidth = ofGetWidth();
	winHeight = ofGetHeight();

	bezManager.setup(gridRes);
	bezWarps = &bezManager.bezierList;
	
	fbo.allocate(winWidth, winHeight);

	g_Width = winWidth;
	g_Height = winHeight;

	font.loadFont("arial.ttf", fontSize);
	
	// Record the area in which the cursor can move. 
	GetClipCursor(&rcOldClip);
	// Get the dimensions of the application's window. 
	GetWindowRect(ofGetWin32Window(), &rcClip);
	// Confine the cursor to the application's window. 
	if (ClipCursor(&rcClip) == 0) {
		cout << "temp confine cursor failed" << endl;
	}

	//setup display apps run on projectors
	setupRestOfWindows();

	//connect osc at last. make sure initialization has been done before receiving commands
	mainChannelReceiver.setup(mainAppOscPort);
	
	setAppMode(appMode);

	if (loadWarpSettings)
		bezManager.loadSettings();

	cout << "succeed allocating all apps" << endl;
	
	setupDone = true;

	ClipCursor(&rcOldClip); //release

} // end setup

void ofApp::setupRestOfWindows() {

	auto thisApp = (ofApp*)ofGetAppPtr();
	auto mainWindow = ofGetMainLoop()->getCurrentWindow();
	
	//allocate rest of windows dynamically
	for (int i = 0; i < numMonitorsToUse; i++) {
		float resX = monitorResolution[i].x;
		float resY = monitorResolution[i].y;
		int paramVal = subSectionIndices[i];
		int monitorIndex = monitorIndices[i];

		auto settings = createWinSetting(ofVec2f(0,0), resX, resY, monitorIndex, mainWindow);
		auto remainedWindow = ofCreateWindow(*settings);
		windows.push_back(remainedWindow);
		
		ofFbo* fboPtr = NULL;
		
		fboPtr = createFboAndPutIntoVector(resX, resY, defaultFormat, srcFbo); //create source frame buffer
		bezManager.addFbo(fboPtr, !warpInitPosRand, cornerOffsetX, cornerOffsetY);
		fboPtr = createFboAndPutIntoVector(resX, resY, defaultFormat, destFbo); //create destination frame buffer
		
		auto remainedApp = make_shared<displayApp>(thisApp, i, monitorIndex, fboPtr->getTextureReference());
		ofRunApp(remainedWindow, remainedApp);
	}

	areWindowsSetup = true;

}

void ofApp::onOSCMessageReceived(ofxOscMessage &msg) {
	string& addr = msg.getAddress();
	string& message = msg.getArgAsString(0);

	//cout << "ofApp osc addr:" << addr << ",msg:" << message << endl;
	if (addr == "winCtrl") {
		if (message == "show") {
			cout << "make all windows foreground" << endl;
			setAllWindowsForeground();
		}
		else if (message == "hide") {
			cout << "hide all windows" << endl;
			setAllWindowsBackground();
		}
	}
	else if (addr == "appCtrl") {
		if (message == "exit") {
			closeApp();
		}
		else if (message == "toggleDebug") {
			showDebugInfo = !showDebugInfo;
		}
	}
	else if (addr == "bezWarp") {
		if (message == "load") {
			bezManager.loadSettings();
		}
		else if (message == "save") {
			bezManager.saveSettings();
		}
		else if (message == "toggleCali") {
			bezManager.toggleGuideVisible();
		}
	}
	else if (addr == "cursorCtrl") {
		if (message == "show") {
			cout << "show cursor" << endl;
			ShowCursor(true);
		}
		else if (message == "hide") {
			cout << "hide cursor" << endl;
			ShowCursor(false);
		}
	}
	else if (addr == "appMode") {
		int toAppMode = stoi(message);
		setAppMode(toAppMode);
	}
	else if (addr == "console") {
		if (message == "focus") {
			showAndFocusOnConsole();
		}
	}
}

void ofApp::setAppMode(int toAppMode) {
	appMode = toAppMode;
	if (appMode == Demo) {
		bezManager.setGuideVisible(false);
		showDebugInfo = false;
		setAllWindowsForeground();
		showAndFocusOnThisAppWindow();
	}
	else if (appMode == Cali) {
		bezManager.setGuideVisible(true);
		setAllWindowsForeground();
		showAndFocusOnThisAppWindow();
		selectedAppIndex = 0;
		noWarping = false; //it's nonsense that do calibration without warping.
	}

}

//--------------------------------------------------------------
void ofApp::update() {
	while (mainChannelReceiver.hasWaitingMessages()) {		
		mainChannelReceiver.getNextMessage(&m);
		onOSCMessageReceived(m);
	}
}


void ofApp::extractPartialFromWholeTexture(int resX, int resY, int appIndex)
{
	/*
	for customization, one can extract arbitrary region based on appIndex
	for ex:
	if(appIndex == 0) {
		//extract region for application with index 0
	}
	else if(appIndex == 1) {
		//extract region for application with index 1
	}
	//... and so on
	*/
	
	//In my art exhibition, the width of overlapping area and the resolution of the display are fixed, so I use a formula to calculate the area that should be extracted.
	unsigned int startX = 0;
	//calculate the starting x coordinate in the wholeTexture. 
	//the rectangular area from (startX, 0) to (startX + resX, resY) will be drawn by calling wholeTex.drawSubsection.
	startX = (resX - overlapPixels) * subSectionIndices[appIndex]; 

	if(startX + resX < g_Width)
		wholeTex.drawSubsection(0, 0, resX, resY, startX, 0, resX, resY);
	else {
		int partLen = g_Width - startX;
		wholeTex.drawSubsection(0, 0, partLen, resY, startX, 0, partLen, resY);
		int restLen = resX - partLen;
		wholeTex.drawSubsection(partLen, 0, restLen, resY, 0, 0, restLen, resY);
	}
		
}


void ofApp::processTexWithAppIndex(int appIndex) {
	if (appIndex < 0 || appIndex >= srcFbo.size() || appIndex >= destFbo.size())
		return;

	int resX = monitorResolution[appIndex].x;
	int resY = monitorResolution[appIndex].y;

	if (noWarping) {
		//if no warping is specified, then directly draw subsection to destination frame buffer
		auto destFboPtr = destFbo[appIndex];

		ofPushStyle();
		destFboPtr->begin(); //begin drawing into destination frame buffer
		ofClear(0);

		if (wholeTex.isAllocated()) {
			extractPartialFromWholeTexture(resX, resY, appIndex);
		}

		destFboPtr->end(); //end drawing into destination frame buffer
		ofPopStyle();

	}
	else {
		//if warping is specified, then draw to source frame buffer first.
		//secondly, use source frame buffer as warping input and output to destination frame buffer
		auto srcFboPtr = srcFbo[appIndex];
		auto destFboPtr = destFbo[appIndex];
		
		//draw partial texture to src frame buffer and warp them later
		ofPushStyle();
		srcFboPtr->begin(); //begin drawing into source frame buffer
		ofClear(0);
		if (drawContent) {
			if (wholeTex.isAllocated()) {
				extractPartialFromWholeTexture(resX, resY, appIndex);
			}
		}
		else {
			//facilitate aligning overlapping area
			ofSetColor(255,0,0, 128); //half transparent red
			ofDrawRectangle(0, 0, overlapPixels, resY);

			ofSetColor(0, 255, 0, 128); //half transparent green
			ofDrawRectangle(resX - overlapPixels, 0, overlapPixels, resY);
		}

		srcFboPtr->end(); //end drawing into source frame buffer
		ofPopStyle();

		//warp content
		ofPushStyle();
		destFboPtr->begin(); //begin drawing into destination frame buffer
		ofClear(0);

		//use the drawn result in source frame buffer as warping input and draw the warping result into destination frame buffer 
		bezWarps->at(appIndex).draw(srcFboPtr->getTextureReference()); 
		
		destFboPtr->end(); //end drawing into destination frame buffer
		ofPopStyle();

	}
	
}

void ofApp::spoutTryToReceiveTex() {
	unsigned int width, height;
	// ====== SPOUT =====
	//
	// INITIALIZE A RECEIVER
	//
	// The receiver will attempt to connect to the name it is sent.
	// Alternatively set the optional bUseActive flag to attempt to connect to the active sender.
	// If the sender name is not initialized it will attempt to find the active sender
	// If the receiver does not find any senders the initialization will fail
	// and "CreateReceiver" can be called repeatedly until a sender is found.
	// "CreateReceiver" will update the passed name, and dimensions.

	if (!bInitialized) {
		
		// Create the receiver given SenderName
		if (spoutreceiver.CreateReceiver(SenderName, width, height, false)) {
			// Is the size of the detected sender different ?
			if (width != g_Width || height != g_Height) {
				// The sender dimensions have changed so update the global width and height
				g_Width = width;
				g_Height = height;
			}
			
			reallocateTexture(wholeTex);
			bInitialized = true;

			return; // return and wait for next frame

		} // receiver was initialized
		
	} // end initialization
	else {
		// The receiver has initialized so it's OK to draw
		// Save current global width and height - they will be changed
		// by ReceiveTexture if the sender changes dimensions
		width = g_Width;
		height = g_Height;

		auto associatedTexData = wholeTex.getTextureData();

		// Try to receive into the local the texture at the current size
		if (spoutreceiver.ReceiveTexture(SenderName, width, height,
			associatedTexData.textureID, associatedTexData.textureTarget)) {

			//	If the width and height are changed, the local texture has to be resized.
			if (width != g_Width || height != g_Height) {
				// Update the global width and height
				g_Width = width;
				g_Height = height;

				// Update the local texture to receive the new dimensions
				reallocateTexture(wholeTex);
				return; // do the rest of work in next frame
			}

		}
		else {
			// A texture read failure might happen if the sender
			// is closed. Release the receiver and start again.
			spoutreceiver.ReleaseReceiver();
			bInitialized = false;
			return;
		}
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofClear(0);

	spoutTryToReceiveTex(); //try to receive texture from spout sender

	if (setupDone) {
		
		if (appMode == Demo) {
			if (bInitialized)
				drawFromCenter("system works normally", 0, 0);
			else
				drawFromCenter("wait for sender", 0, 0);
		}
		else if(appMode == Cali){
			//served as an control panel for calibrating the projecting result.
			if (selectedAppIndex >= 0 && selectedAppIndex < destFbo.size()) {
				processTexWithAppIndex(selectedAppIndex);

				auto destFboPtr = destFbo[selectedAppIndex];
				destFboPtr->getTextureReference().draw(0, 0, winWidth, winHeight);

			}
		}
	}

	if (showDebugInfo) {
		sprintf(str, "FPS:%.0f", ofGetFrameRate());
		ofPushStyle();
		ofSetColor(0, 0, 255);
		font.drawString(str, 30, 60);
		ofPopStyle();
	}
	
}

void ofApp::gotMessage(ofMessage msg) {
	cout << "ofApp got msg:" << msg.message << endl;
}


void ofApp::passMouseEventToSelectedApp(float xRatio, float yRatio, int button, int eventType)
{
	if (appMode != Cali || selectedAppIndex < 0 || selectedAppIndex >= numMonitorsToUse)
		return;

	auto res = monitorResolution[selectedAppIndex];

	if (eventType == mPressed) {
		bezWarps->at(selectedAppIndex).mousePressed(xRatio * res.x, yRatio * res.y, button);
	}
	else if (eventType == mDragged) {
		bezWarps->at(selectedAppIndex).mouseDragged(xRatio * res.x, yRatio * res.y, button);
	}

}

//Input Event Section Start-----------------
void ofApp::mousePressed(int x, int y, int button) {
	passMouseEventToSelectedApp((float)x / winWidth, (float)y / winHeight, button, mPressed);
}

void ofApp::mouseDragged(int x, int y, int button) {
	passMouseEventToSelectedApp((float)x / winWidth, (float)y / winHeight, button, mDragged);
}

void ofApp::keyPressed(int key) {
	//cout << "key:" << key << " pressed in ofApp" << endl;
	
	if (appMode == Cali) {
		if (key == OF_KEY_F5) {
			bezManager.saveSettings();
		}
		else if (key == OF_KEY_F6) {
			bezManager.loadSettings();
		}
		else if (key == OF_KEY_ESC) {
			closeApp();
		}
		else if (key == OF_KEY_F1) {
			selectedAppIndex = selectedAppIndex - 1 < 0 ? numMonitorsToUse - 1 : selectedAppIndex - 1;
		}
		else if (key == OF_KEY_F2) {
			selectedAppIndex = selectedAppIndex + 1 >= numMonitorsToUse ? 0 : selectedAppIndex + 1;
		}
		else if (key == 122) { //Z
			drawContent = !drawContent;
			cout << "drawContent flag:" << drawContent << endl;
		}
		else if (selectedAppIndex >= 0 && selectedAppIndex < bezWarps->size()) {
			if (key == 100) { //D
				bezWarps->at(selectedAppIndex).defaults();
			}
			else {
				bezWarps->at(selectedAppIndex).keyPressed(key);
			}
		}
	}


}

//Input Event Section End-----------------

void ofApp::reallocateTexture(ofTexture& texture) {
	//check if the texture is allocated correctly, if not, allocate it
	if (!texture.isAllocated() || texture.getWidth() != this->g_Width || texture.getHeight() != this->g_Height) {
		int format = texture.isAllocated() ? texture.getTextureData().glInternalFormat : this->defaultFormat;
		texture.allocate(g_Width, g_Height, format);
		//cout << "allocated, w:" << g_Width << "," << g_Height << endl;
	}
}

ofFbo* ofApp::createFboAndPutIntoVector(int width, int height, int format, vector<ofFbo *>& container) {
	ofFbo* fboPtr = new ofFbo();
	
	if(!fboPtr->isAllocated())
		fboPtr->allocate(width, height, format);
	
	container.push_back(fboPtr);
	return fboPtr;
}

void ofApp::showAndFocusOnConsole() {
	HWND winID = GetConsoleWindow();
	
	ShowWindowAsync(winID, SW_SHOWNORMAL);
	SetForegroundWindow(winID);
	
	cout << "console:" << winID << " focused" << endl;
}

void ofApp::showAndFocusOnThisAppWindow()
{
	HWND winID = ofGetWin32Window();

	ShowWindowAsync(winID, SW_SHOWNORMAL);
	SetForegroundWindow(winID);

	cout << "focus on main app" << endl;
}

enum SpoutShareMode {
	Tex = 0,
	CPU = 1,
	Mem = 2
};

//using GUI app to adjust is better.(SpoutDXmode)
void ofApp::configSpout() {
	spoutreceiver.SetShareMode(Tex);
	spoutreceiver.SetDX9compatible(true);
}

void ofApp::closeApp() {
	std::exit(0);	
}

//dont call this directly. It's unsafe
//--------------------------------------------------------------
void ofApp::exit() {
	// ====== SPOUT =====
	if (bInitialized)
		spoutreceiver.ReleaseReceiver(); // Release the receiver

	closeApp();

}

void ofApp::setAllWindowsForeground() {
	if (!areWindowsSetup)
		return;
	for (const auto& win : windows) {
		HWND win32win = win->getWin32Window();
		ShowWindowAsync(win32win, SW_SHOWNOACTIVATE);
	}
}

void ofApp::setAllWindowsBackground() {
	if (!areWindowsSetup)
		return;
	for (const auto& win : windows) {
		HWND win32win = win->getWin32Window();
		ShowWindowAsync(win32win, SW_HIDE);
	}
}

void ofApp::drawFromCenter(const char* msg, float xOffset = 0, float yOffset = 0) {

	float msgH = font.stringHeight(msg);
	float msgW = font.stringWidth(msg);

	font.drawString(msg, (winWidth - msgW) / 2 + xOffset, (winHeight - msgH) / 2 + yOffset);

}

unique_ptr<ofGLFWWindowSettings> ofApp::createWinSetting(ofVec2f& pos, int width, int height, int monitorIndex = 0, shared_ptr<ofAppBaseWindow> sharedWin = nullptr) {
	unique_ptr<ofGLFWWindowSettings> settings(new ofGLFWWindowSettings());
	settings->windowMode = OF_GAME_MODE; //the mode which can run multiple application on multiple display. settings-> monitor would only be adopted in this mode.
	settings->monitor = monitorIndex; //the index of an monitor
	settings->width = width;
	settings->height = height;
	settings->setPosition(pos); //upper-left position
	settings->shareContextWith = sharedWin; //share opengl context with other windows application(example purpose: sharing an large texture among all window application)
	return settings;
}
