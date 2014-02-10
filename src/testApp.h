#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxMidi.h"
#include "ofxLeapMotion.h"
#include "ofxSyphon.h"
#include "ofxOpenNI.h"
#include "ofxParticles.h"

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
        ofxHardwareDriver kinect;
        ofxIntSlider KinectAngleSlider;
        ofxOpenNIContext NIContext;
        ofxDepthGenerator NIdepthGen;
        ofxUserGenerator NIuserGen;
        bool _userDetected;
        void setUserDetected(bool userDetected);
    
        void updateKinectData();
        void setKinectAngle(int & angle);
        ofPoint Rhand;
        ofPoint Lhand;
    
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
