#include "displayApp.h"

//--------------------------------------------------------------
void displayApp::setup() {

	ofBackground(0, 0, 0);
	sprintf(str, "Monitor %d app", monitorIndex);
	ofSetWindowTitle(str);
	winWidth = ofGetWidth();
	winHeight = ofGetHeight();
	
	font.loadFont("arial.ttf", mainAppPtr->fontSize);

	//sprintf(str, "testMsg from display app %d", appIndex);
	//mainAppPtr->gotMessage(ofMessage(str));
	
} // end setup


//--------------------------------------------------------------
void displayApp::draw() {
	mainAppPtr->processTexWithAppIndex(appIndex);

	ofClear(0);
	if (finalTex->isAllocated())
		finalTex->draw(0, 0, winWidth, winHeight);

}

void displayApp::gotMessage(ofMessage msg) {
	string& content = msg.message;
}

//--------------------------------------------------------------
void displayApp::update() {

}

void displayApp::drawFromCenter(const char* msg, float xOffset = 0, float yOffset = 0) {

	float msgH = font.stringHeight(msg);
	float msgW = font.stringWidth(msg);

	font.drawString(msg, (winWidth - msgW) / 2 + xOffset, (winHeight - msgH) / 2 + yOffset);

}

//--------------------------------------------------------------
void displayApp::exit() {

}
