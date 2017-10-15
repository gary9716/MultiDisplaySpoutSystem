#include "ofMain.h"
#include "ofApp.h"
#include "ofxXmlSettings.h"

#include <iostream>
#include <string>
#include <algorithm>

/*
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

//--------------------------------------------------------------
// to change options for console window (Visual Studio)
//
// Properties > Linker > System > Subsystem
//    for console : Windows (/SUBSYSTEM:CONSOLE)
//
//    for Window : Windows (/SUBSYSTEM:WINDOWS)
//
// Click APPLY and OK. Then make changes to Main as below
//--------------------------------------------------------------


int debugMsgSize = 40; //debug message's font size

int outputMonitorInfo(); //output monitor's or display's information for configuring parameter file
void trim(string &str);
void PauseAndThenLeave(); //suspend application to view console output
ofGLFWWindowSettings* createWinSettingForMainOfApp(int width, int height, shared_ptr<ofAppBaseWindow> sharedWin);
void createSampleParamFile(const string& fileName); //create parameter template
void parseParamsXml(const string& path,
	vector<int>& monitorIndices,
	vector<int>& subSectionIndices,
	vector<ofVec2f>& monitorResolution,
	bool& pauseAndLeave,
	int& appMode,
	bool& loadWarpSetting,
	bool& doWarp,
	float& cornerX,
	float& cornerY,
	unsigned int& overlapPixels,
	bool& isDebugging,
	string& spoutSenderName);
shared_ptr<ofAppBaseWindow> createWindowInPrimaryMonitor();

// for displaying console ( debugging use )
//========================================================================
int main(int argc,      // Number of strings in array argv  
	char *argv[],   // Array of command-line argument strings  
	char *envp[])  // Array of environment variable strings   
{ // Properties > Linker > System > Subsystem, set the field to "Windows (/SUBSYSTEM:CONSOLE)"

// for hiding console
//========================================================================
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) { // Properties > Linker > System > Subsystem, set the field to "Windows (/SUBSYSTEM:WINDOWS)"

	//init and declare some local vars
	string line;
	vector<int> monitorIndices;
	vector<int> subSectionIndices;
	vector<ofVec2f> monitorResolution;
	bool pauseAndLeave = false; //pause for viewing monitor indices and size. terminiate the application after press any key.
	bool isDebugging = true; //show debug info
	int numMonIndices = 0; //number of monitors for displaying content
	int appMode = Demo;
	bool loadWarpSetting = false; //load bezier warping parameters saved before
	bool doWarp = true; //apply bezier warping effect or just fill up display without any warping 
	float cornerX = 0, cornerY = 0; //the padding proportion in X and Y direction.
	unsigned int overlapPixels = 0;
	string spoutSenderName = "";

	//createSampleParamFile("sampleParams.xml");
	//exit(0);

	parseParamsXml("params.xml",
		monitorIndices,
		subSectionIndices,
		monitorResolution,
		pauseAndLeave,
		appMode,
		loadWarpSetting,
		doWarp,
		cornerX,
		cornerY,
		overlapPixels,
		isDebugging,
		spoutSenderName);

	int numAvailableMonitors = outputMonitorInfo();
	if (numAvailableMonitors == 0)
		return -1;
	
	if (appMode >= NumAppMode) {
		return -1;
	}

	if (pauseAndLeave)
		PauseAndThenLeave();

	auto mainWindow = createWindowInPrimaryMonitor();

	auto mainApp = make_shared<ofApp>(appMode,
		doWarp,
		loadWarpSetting,
		isDebugging,
		debugMsgSize,
		overlapPixels,
		cornerX,
		cornerY,
		monitorResolution,
		subSectionIndices,
		monitorIndices,
		spoutSenderName);

	ofRunApp(mainWindow, mainApp);
	ofRunMainLoop();

}

shared_ptr<ofAppBaseWindow> createWindowInPrimaryMonitor() {

	ofVec2f winSize(800, 600);
	auto settings = createWinSettingForMainOfApp(winSize.x, winSize.y, nullptr);
	
	//create first window and rest of windows would share the same opengl context with the first one
	return ofCreateWindow(*settings);
	
}

void createSampleParamFile(const string& path) {
	ofxXmlSettings xmlSettings;
	xmlSettings.setValue("ShowDebugInfo", 1); //1 for showing debug information, 0 for not showing it
	xmlSettings.setValue("MonitorIndices", "0,1");// list of monitor indices. please refer to the information output from the function "outputMonitorInfo"
	xmlSettings.setValue("SubSectionIndices", "0,1");// list of subsection indices. the index is used in calculate the subsection displayed on corresponding display 
	xmlSettings.setValue("PauseAndLeave", 0);
	xmlSettings.setValue("AppMode", Demo);
	xmlSettings.setValue("LoadMapping", 0);
	xmlSettings.setValue("DoWarp", 1);
	xmlSettings.setValue("CornerX", 0.04);
	xmlSettings.setValue("CornerY", 0.02);
	xmlSettings.setValue("OverlapPixels", 210);
	xmlSettings.setValue("SpoutSender", "UnitySender1");
	for (int i = 0; i < 2; i++) {
		int tagNum = xmlSettings.addTag("resolution");
		xmlSettings.setValue("resolution:X", 1920, tagNum);
		xmlSettings.setValue("resolution:Y", 1200, tagNum);
	}

	xmlSettings.saveFile(path);
}

void parseParamsXml(const string& path,
	vector<int>& monitorIndices,
	vector<int>& subSectionIndices,
	vector<ofVec2f>& monitorResolution,
	bool& pauseAndLeave,
	int& appMode,
	bool& loadWarpSetting,
	bool& doWarp,
	float& cornerX,
	float& cornerY,
	unsigned int& overlapPixels,
	bool& isDebugging,
	string& spoutSenderName) {

	ofxXmlSettings xmlSettings;
	if (xmlSettings.loadFile(path)) {
		monitorIndices.clear();
		auto params = ofSplitString(
			xmlSettings.getValue("MonitorIndices", ""),
			",", true, true);
		for (string param : params) {
			monitorIndices.push_back(stoi(param));
		}
		
		subSectionIndices.clear();
		params = ofSplitString(
			xmlSettings.getValue("SubSectionIndices", ""),
			",", true, true);
		for (string param : params) {
			subSectionIndices.push_back(stoi(param));
		}
		
		pauseAndLeave = xmlSettings.getValue("PauseAndLeave", 0) == 1;
		appMode = xmlSettings.getValue("AppMode", Demo);
		loadWarpSetting = xmlSettings.getValue("LoadMapping", 0) == 1;
		doWarp = xmlSettings.getValue("DoWarp", 0) == 1;
		
		cornerX = xmlSettings.getValue("CornerX", 0.05);
		cornerY = xmlSettings.getValue("CornerY", 0.05);

		overlapPixels = xmlSettings.getValue("OverlapPixels", 0);
		isDebugging = xmlSettings.getValue("ShowDebugInfo", 0) == 1;
		spoutSenderName = xmlSettings.getValue("SpoutSender", "");
		
		monitorResolution.clear();
		int numTagsToParsed = monitorIndices.size();
		for (int i = 0; i < numTagsToParsed; i++) {
			ofVec2f mSize(
				xmlSettings.getValue("resolution:X", 1920, i),
				xmlSettings.getValue("resolution:Y", 1200, i)
				);
			cout << "m_i:" << monitorIndices[i] << ",w:" << mSize.x << ",h:" << mSize.y << endl;
			monitorResolution.push_back(mSize);
		}

	}
	else {
		cout << "cannot load the params xml file" << endl;
		cin.get();
		exit(0);
	}

}


void trim(string &str) {
	std::remove(str.begin(), str.end(), ' ');
	std::remove(str.begin(), str.end(), '\n');
	std::remove(str.begin(), str.end(), '	');

	return;
}

void PauseAndThenLeave() {
	system("pause");
	exit(0);
}

ofGLFWWindowSettings* createWinSettingForMainOfApp(int width, int height, shared_ptr<ofAppBaseWindow> sharedWin) {
	ofGLFWWindowSettings* settings = new ofGLFWWindowSettings();
	settings->windowMode = OF_WINDOW; //in this mode, app would only run on primary display
	settings->resizable = false;
	settings->width = width;
	settings->height = height;
	settings->setPosition(ofVec2f(100, 100)); //set upper-left position
	settings->shareContextWith = sharedWin; //share opengl context
	return settings;
}

typedef struct monitorInfo {
	ofPoint phySize;
	ofPoint pos;
	GLFWmonitor *monitor;
	int monIndex; //index used to specify in window setting
} MonInfo;

//sort monitor info by its virtual monitor position in windows system 
bool cmp_by_posX(MonInfo a, MonInfo b)
{
	return a.pos.x < b.pos.x;
}

int outputMonitorInfo() {
	ofAppGLFWWindow ofGLFWWin;
	if (glfwInit() != GL_TRUE) {
		cout << "fail to init glfw" << endl;
		exit(255);
	}

	int count;
	const auto monitors = glfwGetMonitors(&count);
	vector<MonInfo> monInfos;
	//sort by it's x component of virtual desktop.
	for (int i = 0; i < count; i++) {
		auto monitor = monitors[i];
		int w, h, x, y;
		MonInfo monInfo;
		glfwGetMonitorPhysicalSize(monitor, &w, &h);
		monInfo.phySize.x = w;
		monInfo.phySize.y = h;

		glfwGetMonitorPos(monitor, &x, &y);
		monInfo.pos.x = x;
		monInfo.pos.y = y;

		monInfo.monitor = monitor;
		monInfo.monIndex = i;
		monInfos.push_back(monInfo);
	}

	sort(monInfos.begin(), monInfos.end(), cmp_by_posX);

	for (int i = 0; i < count; i++) {
		MonInfo& monInfo = monInfos[i];
		ofLogNotice() << "monitor index:" << monInfo.monIndex << ", physical size: " << monInfo.phySize.x << "x" << monInfo.phySize.y << "mm at " << monInfo.pos.x << ", " << monInfo.pos.y;
	}

	cout << "num available monitors:" << count << endl;

	return count;
}
