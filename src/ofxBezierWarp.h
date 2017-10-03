/*
 *  ofxBezierWarp.h
 *
 *  Created by Patrick Saint-Denis on 12-03-05.
 *	A Bezier Warp made of multiple homographies
 *  
 *  Forked by Teruaki Tsubokura on 13-08-28.
 *
 */

#pragma once

#include "ofMain.h"

class ofxBezierWarp{

public:
    int no;
    int layer;
    ofFbo *fbo;
	ofPoint corners[4];
	ofPoint anchors[8];
    ofPoint center;
	float cornerOffsetX = 0;
	float cornerOffsetY = 0;

    int mouseON, spritesON, anchorControl;
    int selectedSprite[4], selectedControlPoint[8], selectedCenter;
    bool showGrid;
	
	int edgeCornerIndices[4][2] = {
		{0,3},
		{0,1},
		{1,2},
		{2,3}
	};

	ofxBezierWarp() {
		no = 0;
		layer = 0;
		gridRes = 0;
		prev_gridRes = 0;
	}
	
    void setup(ofFbo* _fbo, bool defaultNoRand, float cornerOffset);
	void setup(ofFbo* _fbo, bool defaultNoRand, float cornerOffsetX, float cornerOffsetY);
	void update(); // if you need
	void draw();
	void draw(ofTexture& texture);
	void defaults();
	void setGridRes(int gridRes) {
		this->prev_gridRes = this->gridRes;
		this->gridRes = gridRes;
	}

	int getGridRes() {
		return gridRes;
	}

    void resetAnchors();
	void save();
	void load();
	void mousePressed(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void keyPressed(int clef);
    bool isSelected();
    
    void setCanvasSize(int _width, int _height);
    void setWarpResolution(int _res);
    void setGridVisible(bool _visible);
    bool bGradient;
	bool defaultNoRand;
private:
	int mousePosX, mousePosY, rad;
	float width, height;
	int gridRes;
	int prev_gridRes;
    void setup();
    void setup(int _width, int _height);
    void setup(int _width, int _height, int grid, int _layer);
    
	void sprites();
	float bezierPoint(float x0, float x1, float x2, float x3, float t);
    void drawGrid(float _stepX, float _stepY);
    
	std::vector<std::vector<ofPoint>> bezSurfPoints;
	void allocateBzPts();
	int allocatedGridRes;
};