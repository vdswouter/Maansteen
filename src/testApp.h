#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxMidi.h"
#include "ofxLeapMotion.h"
#include "ofxSyphon.h"
#include "ofxParticles.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"


class testApp : public ofBaseApp{

	public:
    // ===== OPEN FRAMEWORKS ====
		void setup();
		void update();
		void draw();
        void keyPressed(int key);
        void keyReleased(int key);
        void gotMessage(ofMessage msg);
    
        bool showData = false;
    bool log = false;
    
    // ===== LEAP =====
        ofxLeapMotion leap;
        vector<ofPoint> fingersYpos;
        vector<ofxLeapMotionSimpleHand> simplehands;
        void GetLeapData();
        int prevLX = 0;
        int prevLY = 0;
        int prevRX = 0;
        int prevRY = 0;
    
    
    // ===== KINECT =====
    ofxKinect kinect;
    ofxCvGrayscaleImage grayImage;
    ofxCvGrayscaleImage grayBg;
    ofxCvGrayscaleImage grayDiff;
    ofxCvContourFinder contourFinder;
    
    ofxPanel kinectDataPanel;
    ofxIntSlider KinectAngleSlider;
    ofxIntSlider videoThresholdSlider;
    ofxVec2Slider contourAreaSlider;
    ofxVec2Slider cameraViewSlider;
    ofxButton btnSetBackground;
    bool _userDetected;
    bool showKinectControls = false;
    void setUserDetected(bool userDetected);
    bool backgroundSet = false;
    int videoTreshold = 80;
    
    void updateKinectData();
    void setBackgroundImg();
    void setKinectAngle(int & angle);
    void setVideoThreshold(int & threshold);
    
    
    
    // ===== ARDUINO =====
        ofArduino arduino;
        bool arduinoIsSetup;
    
        void setupArduino(const int & version);
        void digitalPinChanged(const int & pinNum);
        void analogPinChanged(const int & pinNum);
        void updateArduino();
        void setPad(int padNum, bool val);
        void setTreshold(int & treshold);

        const int waitingForUserPin = 4;
        const int calibratingPin = 2;
        const int letsDancePin = 3;
    
        const int knockSensorPin = 1;
        int _treshold = 5;
        ofxIntSlider treshold;
    
    // ===== MIDI =====
        ofxMidiOut midi;
    
        const int KinectMidiChannel = 1;
        const int ArduinoMidiChannel = 1;
        const int LeapMidiChannel = 1;
        const int VideoMidiChannel = 15;
        vector<bool> playingPads;
    
        void sendMidiSignals();
    
    // ===== PARTICLES =====
        void setupParticles();
        void updateParticles();
        ofVec3f centerPoint;
        ofVec3f mousePos;
        ofVec3f mouseVel;
        ofVec3f prevMousePos;
        ofVec3f prevMouseVel;
    
        ofxParticleSystem particleSystem;
        ofxParticleEmitter leftEmitter, rightEmitter;
        float rotAcc, gravAcc, fieldMult, drag;
        ofFloatPixels vectorField;
    
        ofTexture tex;
        ofEasyCam cam;
    
    // ===== SYPHON =====
        ofxSyphonServer syphonOutput;
};
