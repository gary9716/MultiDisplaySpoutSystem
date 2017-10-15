/*

	Spout OpenFrameworks Receiver example

	Copyright (C) 2015-2017 Lynn Jarvis.

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

	adapted by Kuan-Ting Chou on 2017-04-22 for NTMOFA art exhibition
*/
#pragma once

#include <stdlib.h>     /* exit, EXIT_FAILURE */
#include "ofMain.h"
#include "SpoutSDK/Spout.h" // Spout SDK
#include "ofxOsc.h"
#include "ofxBezierWarpManager.h"

enum AppMode {
	Demo = 0, //simply display
	Cali = 1, //Calibration
	NumAppMode
};

enum WinCtrlAct {
	floating = 0, //just show the window but it wont steal the focus
	select = 1, //select the certain app and output its content to main app
	hide = 2, //make it seems like minimized

};

enum InputEventType {
	mPressed = 0,
	mDragged = 1
};

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		void exit();
		void closeApp();
		void mousePressed(int x, int y, int button);
		void mouseDragged(int x, int y, int button);
		void keyPressed(int key);
		void gotMessage(ofMessage msg);
		void processTexWithAppIndex(int appIndex);

		bool bInitialized;		     // Initialization result
		bool setupDone = false;
		ofxBezierWarpManager bezManager;
		vector<ofxBezierWarp>* bezWarps = nullptr;
		ofTexture wholeTex; //the single texture shared among all OF application
		
		ofFbo fbo;

		char SenderName[256] = {0};	     // Sender name used by a receiver
		int g_Width, g_Height;       // Used for checking sender size change
		
		int appMode = Demo;
		int numMonitorsToUse = 1;
		bool showDebugInfo = false;
		bool showMonitorIndex = false;
		bool loadWarpSettings = false;
		int selectedAppIndex = -1; //the content of selected app will be outputed to the screen where main app locate

		const int defaultFormat = GL_RGB;
		unsigned int overlapPixels = 210;
		const int mainAppOscPort = 10000; //the port for receiving osc message from Unity
		int fontSize = 32;
		int gridRes = 20;
		bool drawContent = true;
		bool drawGridInCali = false;
		float cornerOffsetX = 0.05f;
		float cornerOffsetY = 0.05f;

		ofApp(int appMode,
			bool toWarp,
			bool loadWarpSettings, 
			bool showDebugInfo, 
			int fontSize, 
			unsigned int overlapPixels,
			float cornerX,
			float cornerY,
			vector<ofVec2f>& monitorResolution, 
			vector<int>& subSectionIndices,
			vector<int>& monitorIndices,
			string& spoutSenderName) {

			this->loadWarpSettings = loadWarpSettings;
			this->showDebugInfo = showDebugInfo;
			this->fontSize = fontSize;
			this->monitorResolution = monitorResolution;
			this->numMonitorsToUse = monitorIndices.size();
			this->subSectionIndices = subSectionIndices;
			this->monitorIndices = monitorIndices;
			this->appMode = appMode;
			this->noWarping = !toWarp;
			this->cornerOffsetX = cornerX;
			this->cornerOffsetY = cornerY;
			this->overlapPixels = overlapPixels;
			
			if(spoutSenderName.size() != 0)
				strncpy(SenderName, spoutSenderName.c_str(), spoutSenderName.size());
			else
				SenderName[0] = 0;
		}

	private:
		void setAllWindowsForeground();
		void setAllWindowsBackground();
		void configSpout();
		void spoutTryToReceiveTex();
		void onOSCMessageReceived(ofxOscMessage &msg);
		void reallocateTexture(ofTexture& tex);
		void drawFromCenter(const char* msg, float xOffset, float yOffset);
		unique_ptr<ofGLFWWindowSettings> createWinSetting(ofVec2f& pos, int width, int height, int monitorIndex, shared_ptr<ofAppBaseWindow> sharedWin);
		void setupRestOfWindows();
		void setAppMode(int appMode);
		ofFbo* createFboAndPutIntoVector(int width, int height, int format, vector<ofFbo *>& container);
		void showAndFocusOnConsole();
		void showAndFocusOnThisAppWindow();
		void passMouseEventToSelectedApp(float xRatio, float yRatio, int button, int eventType);
		void extractPartialFromWholeTexture(int resX, int resY, int appIndex);
		SpoutReceiver spoutreceiver; // A Spout receiver object
		ofxOscReceiver mainChannelReceiver;
		ofxOscMessage m;
		vector<shared_ptr<ofAppBaseWindow>> windows;
		unsigned int winWidth = 0;
		unsigned int winHeight = 0;
		ofTrueTypeFont font;
		char str[256];
		

		vector<ofVec2f> monitorResolution;
		vector<int> subSectionIndices;
		vector<int> monitorIndices;
		vector<ofFbo *> srcFbo; //list
		vector<ofFbo *> destFbo; //list of frame buffer that store the final result
		std::ostringstream oss;

		bool areWindowsSetup = false;

		bool warpInitPosRand = false;
		bool noWarping = false; //no bezier warping and masking
		HWINEVENTHOOK g_hook;
		RECT rcClip;
		RECT rcOldClip;
};
