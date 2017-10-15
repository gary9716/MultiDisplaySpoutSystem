# MultiDisplaySpoutSystem

It is a panoramic projection system implemented in OpenFrameworks and was used in my art exhibition in National Taiwan Museum of Fine Arts. It has been demonstrated that I can let the system spawn 9 windows applications on 9 displays and run a Vive VR application all in single computer in the same time and it's pretty smooth. ( I forgot the actual frame rate but I wouldn't feel dizzy during experiencing the VR app.) 
The overall system contains 2 OpenFrameworks classes and their roles are:
1. ofApp: 
* receive an large texture via [Spout](http://spout.zeal.co/) library (In my case, it received an texture with resolution of 15390* 1200 from Unity). 
* show calibration GUI in calibration mode.
* receive mouse and keyboard events.
* receive OSC message from other application like Unity so it can be controlled via application in other platform.
* manage the visibility of other windows applications which are the instances of displayApp.
* expose the texture processing API for displayApp.
2. displayApp:
* process the texture(extract part of the texture and warp the extracted content) via the API in ofApp.
* project it onto display.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine.

### Prerequisites

* Windows OS (Currently,it only support Windows OS and has worked well in 8.1 and 10) 
* OpenFrameworks SDK (I developed this with SDK version 0.9.8)
* Visual Studio 2015

### Setup

1. Follow the instructions in OpenFrameworks website: (it can be skipped if you have already setup OF SDK)
*  [Download SDK](http://openframeworks.cc/download/)
*  [Setup SDK with Visual Studio](http://openframeworks.cc/setup/vs/)

2. Download this repository and unzip it
3. Open the project generator in the OpenFrameworks SDK folder(the path would be "{path to the root of OF SDK}\projectGenerator-vs")
4. Set the Project path as where the unzipped repository folder is located in and click the "import" button.
5. If the project is successfully imported, it should look like this:
6. click the "Update" button and then click the "Open In IDE" button.
7. click Build > Build Solution and after the project is successfully built, the executable would be in the bin folder of project folder.

### Specify Parameters

There is a parameter file called "param.xml" under the path {project path}\bin\data.
The meaning of these parameters:
1. ShowDebugInfo: show debugging information like FPS.
2. MonitorIndices: the indices of monitors(or displays) you want to use. the information can be look up in console window.
3. SubSectionIndices: the indices used for calculating which part of texture should be extracted.
4. PauseAndLeave: pause for viewing the information in console window and terminate the application. mainly for adjusting parameters.(1 for using this function and 0 for not using this function)
5. AppMode: 0 for demonstration mode and 1 for calibration mode.
6. LoadMapping: load the bezier warping parameters saved in BezierWarpManager_settings.xml.(1 for loading and 0 for not loading)
7. DoWarp: 1 for applying bezier warping and 0 for simply displaying extracted region.
8. CornerX: the percentage of padding of X direction in calibration mode.
9. CornerY: the percentage of padding of Y direction in calibration mode.
10. OverlapPixels: the width of overlapping area between two consecutive displays.
11. SpoutSender: the name of the spout sender that sends the shared texture.
12. resolution: the resolution of certain monitor. 


## Specs of the computer in art exhibition:

* CPU: i7 7700K
* GPU: GTX 1060 Mini ITX OC 3G, GTX 1050Ti OC 4G, HD7750
* Motherboard: ROG STRIX Z270F
* RAM: DDR4 2400 8GB * 2

## Built With

* [Spout](http://spout.zeal.co/) - real-time video sharing system.
* [ofxBezierWarp](https://github.com/patrick88/ofxBezierWarp) - an addon that can do bezier warping and control the warping with mouse and keyboard.
* [ofxBezierWarpManager](https://github.com/TsubokuLab/ofxBezierWarpManager) - an addon that manage multiple bezier warps.

## Contributing

Please read [CONTRIBUTING.md](https://gist.github.com/PurpleBooth/b24679402957c63ec426) for details on our code of conduct, and the process for submitting pull requests to us.

## Authors

* Kuan-Ting Chou : the only developer of this system

## License

This project is licensed under the GNU Lesser General Public License - see the [LICENSE.txt](https://github.com/gary9716/MultiDisplaySpoutSystem/blob/master/LICENSE.txt) file for details

## Acknowledgments

* The great addons and library I have used in this project
* Inspired by my friend called Liu Ting Chun who is studying in Taipei National University of the Arts 
*  [README Template](https://gist.github.com/PurpleBooth/b24679402957c63ec426)