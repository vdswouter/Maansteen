#pragma once

#include "ofMain.h"
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
    
    
    // ===== KINECT =====
        ofxOpenNIContext NIContext;
        ofxDepthGenerator NIdepthGen;
        ofxUserGenerator NIuserGen;
        bool _userDetected;
        void setUserDetected(bool userDetected);
    
        void updateKinectData();
        ofPoint Rhand;
        ofPoint Lhand;
    
    // ===== ARDUINO =====
        ofArduino arduino;
        bool arduinoIsSetup;
    
        void setupArduino(const int & version);
        void digitalPinChanged(const int & pinNum);
        void analogPinChanged(const int & pinNum);
        void updateArduino();
        void KnockHandler(int pinNum);

        const int waitingForUserPin = 3;
        const int calibratingPin = 4;
        const int letsDancePin = 5;
    
        const int knockSensorPin = 1;
        const int knockTreshold = 200;
    
    // ===== MIDI =====
        ofxMidiOut midi;
    
        const int KinectMidiChannel = 1;
        const int ArduinoMidiChannel = 2;
        const int LeapMidiChannel = 3;
        const int VideoMidiChannel = 15;
        vector<int> chordNotes;
        vector<bool> playingChordNotes;
    
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
