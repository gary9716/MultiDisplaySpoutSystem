//Created by Kuan-Ting Chou on 2017-04-22

#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofApp.h"

//we only display the final texture in this app. the final texture is processed(retrieve subSection and warp it) in main OF app(ofApp).  
class displayApp : public ofBaseApp
{
public:
	void setup();
	void update();
	void draw();
	void exit();
	void gotMessage(ofMessage msg);
	
	ofApp* mainAppPtr; //main OF app
	ofTexture* finalTex; //the texture for restoring final result
	ofTrueTypeFont font;

	int appIndex = -1;
	int monitorIndex = -1;
	
	displayApp(ofApp* mainAppPtr, 
		int appIndex, 
		int monitorIndex, 
		ofTexture& finalTex) {

		this->mainAppPtr = mainAppPtr;
		this->appIndex = appIndex;
		this->monitorIndex = monitorIndex;
		this->finalTex = &finalTex;
	}

	void drawFromCenter(const char* msg, float xOffset, float yOffset);
	
	unsigned int winWidth = 0;
	unsigned int winHeight = 0;
	char str[256];
	
};

