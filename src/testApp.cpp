#include "testApp.h"

// ===== OPEN FRAMEWORKS =====
void testApp::setup(){
    ofBackground(0);
    ofSetFrameRate(60);
    ofSetCircleResolution(180);
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    
    //Midisetup: poorten oplijsten + virtuele poort aanmaken + de constante waarden aanmaken
    midi.listPorts();
    midi.openVirtualPort("MaanSteen");
    chordNotes.push_back(60);
    chordNotes.push_back(67);
    chordNotes.push_back(76);
    chordNotes.push_back(52);
    chordNotes.push_back(84);
    chordNotes.push_back(43);
    chordNotes.push_back(91);
    chordNotes.push_back(36);
    chordNotes.push_back(96);
    chordNotes.push_back(28);
    for (int i = 0; i < 10; i++) {
        playingChordNotes.push_back(false);
    }
    
    
    //start de leapmotion
    leap.open();
    
    //setup the arduino over firmata
    arduino.connect("/dev/tty.usbmodem1451",57600);
    ofAddListener(arduino.EInitialized, this, &testApp::setupArduino);
    arduinoIsSetup = false;
    
    //kinect oproepen via openNI
//    NIContext.setup();
//    NIdepthGen.setup(&NIContext);
//    NIuserGen.setup(&NIContext);
//    _userDetected = false;
//    Rhand.x = 0;
//    Rhand.y = 0;
//    Lhand.x = 0;
//    Lhand.y = 0;
    
    
    //syphon server opstarten voor het doorsturen van de beelden naar het syphon framework
    syphonOutput.setName("Maansteen");
    
    //particles setup
    setupParticles();
    
}

void testApp::update(){
    GetLeapData();
    updateArduino();
//    updateKinectData();
    
    sendMidiSignals();
}

void testApp::draw(){
    cam.begin();
    // Draw system.
    ofSetLineWidth(2.0);
    particleSystem.draw(tex);
    cam.end();


    // hiervoor alle drawing die naar Syphon wordt gestuurd.
    syphonOutput.publishScreen();
    
    ofSetColor(255);
    if (showData) {
        // leapdata
        ofLine(0, 730, 1024, 730);
        ofDrawBitmapString("LeapMotion", 5, 720);
        for (int i = 0; i < fingersYpos.size(); i++) {
            int newY = 730 - (fingersYpos[i].y * 0.8);
            ofEllipse((i * 80)+15, newY, 20, 20);
            ofDrawBitmapString("hand: "+ofToString(fingersYpos[i].x), (i*80)+5, newY - 17);
            ofDrawBitmapString("y: "+ofToString(round(fingersYpos[i].y)), (i*80)+5, newY-35);
        }
        
        // kinectdata
        ofDrawBitmapString("Kinect DepthImage + Users", 25, 14);
        if (_userDetected) {
            ofSetColor(0, 200, 200);
            ofEllipse(Rhand, 20, 20);
            ofEllipse(Lhand, 20, 20);
        }
        NIdepthGen.draw(5,20,400,300);
        NIuserGen.draw();
        ofLine(0, 330, 1024, 330);
        
        //padsdata
        ofLine(450, 0, 450, 330);
        ofDrawBitmapString("Drumpads", 460, 14);

    }

    ofDrawBitmapString("druk S voor de datavisualisaties.", 20, 748);
}

// ===== LEAP =====

void testApp::GetLeapData(){
    fingersYpos.clear();
    simplehands.clear();
    
    
    //haal handen op van de leap
    simplehands = leap.getSimpleHands();
    
    if (leap.isFrameNew() && simplehands.size()) {
        for (int i = 0; i < simplehands.size(); i++) {
            //loop door handen
            for (int j = 0; j < simplehands[i].fingers.size(); j++) {
                //loop door vingers + xpos van de finger ophalen
                ofPoint vinger;
                vinger.x = i;
                vinger.y = simplehands[i].fingers[j].pos.y;
                fingersYpos.push_back( vinger );
            }
        }
    }
    
    updateParticles();
    
}

// ===== PARTICLES =====

void testApp::setupParticles(){
    centerPoint.set(0, 0, 0);

    leftEmitter.setPosition(ofVec3f(100, 100));
    leftEmitter.setVelocity(ofVec3f(150.0, 150.0, -150.0));
    leftEmitter.posSpread = ofVec3f(10, 10, 10);
    leftEmitter.velSpread = ofVec3f(10.0,10);
    leftEmitter.life = 20;
    leftEmitter.lifeSpread = 5.0;
    leftEmitter.numPars = 1;
    leftEmitter.color = ofColor(200, 100, 100);
    leftEmitter.colorSpread = ofColor(50, 50, 50);
    leftEmitter.size = 22;
    
    rightEmitter.setPosition(ofVec3f(100, 100));
    rightEmitter.setVelocity(ofVec3f(-150.0, 150.0, 150.0));
    rightEmitter.posSpread = ofVec3f(10, 10, 10);
    rightEmitter.life = 20;
    rightEmitter.lifeSpread = 5.0;
    rightEmitter.numPars = 1;
    rightEmitter.size = 22;
    rightEmitter.color = ofColor(100, 100, 200);
    rightEmitter.colorSpread = ofColor(50, 50, 50);
    
    vectorField.allocate(128, 128, 3);
    ofLoadImage(tex, "tex.png");
    
    rotAcc = 4500;
    gravAcc = 13500;
    drag = 0.5;
    fieldMult = 30.0;

}

void testApp::updateParticles(){
    for (int y = 0; y < vectorField.getHeight(); y++) {
        for (int x = 0; x< vectorField.getWidth(); x++) {
            int index = vectorField.getPixelIndex(x, y);
            float angle = ofNoise(x / (float)vectorField.getWidth() * 4.0f, y / (float)vectorField.getHeight() * 4.0f, ofGetElapsedTimef() * 0.05f) * TWO_PI * 2.0f;
            ofVec2f dir(cos(angle), sin(angle));
            dir.normalize().scale(ofNoise(x / (float)vectorField.getWidth() * 4.0f, y / (float)vectorField.getHeight() * 4.0f, ofGetElapsedTimef() * 0.05 + 10.0f));
            vectorField.setColor(x, y, ofColor_<float>(dir.x, dir.y, 0.0f));
        }
    }
    
    // Add forces.
    float dt = MIN(ofGetLastFrameTime(), 1.0f / 10.0f);
    particleSystem.gravitateTo(centerPoint, gravAcc, 1.0f, 10.0f, false);
    particleSystem.rotateAround(centerPoint, rotAcc, 10.0f, false);
    particleSystem.applyVectorField(vectorField.getPixels(), vectorField.getWidth(), vectorField.getHeight(), vectorField.getNumChannels(), ofGetWindowRect(), fieldMult);
    
    particleSystem.update(dt, drag);
    
    particleSystem.addParticles(leftEmitter);
    particleSystem.addParticles(rightEmitter);
    
    prevMousePos = mousePos;
    prevMouseVel = mouseVel;

}

// ===== KINECT =====

void testApp::updateKinectData(){
    NIContext.update();
    NIdepthGen.update();
    NIuserGen.update();
    
    if (NIuserGen.getNumberOfTrackedUsers() > 0) {
        setUserDetected(true);
        ofxTrackedUser user = *NIuserGen.getTrackedUser(NIuserGen.getNumberOfTrackedUsers());
//        ofLogNotice() << "calibrating:" << user.skeletonCalibrating << " calibrated:" << user.skeletonCalibrated << " tracking:" <<  user.skeletonTracking;
//        ofLogNotice() << "needs pose for calibration: " << NIuserGen.needsPoseForCalibration();
        arduino.sendDigital(waitingForUserPin, ARD_LOW);
        if (NIuserGen.userCalibrated) {
            arduino.sendDigital(letsDancePin, ARD_HIGH);
            arduino.sendDigital(calibratingPin, ARD_LOW);
        }else{
            arduino.sendDigital(calibratingPin, ARD_HIGH);
            arduino.sendDigital(letsDancePin, ARD_LOW);
        }
        Rhand.x = user.right_lower_arm.position[1].X;
        Rhand.y = user.right_lower_arm.position[1].Y;
        Lhand.x = user.left_lower_arm.position[1].X;
        Lhand.y = user.left_lower_arm.position[1].Y;
    }else{
        setUserDetected(false);
        arduino.sendDigital(letsDancePin, ARD_LOW);
        arduino.sendDigital(calibratingPin, ARD_LOW);
        arduino.sendDigital(waitingForUserPin, ARD_HIGH);
    }
}

void testApp::setUserDetected(bool userDetected){
    if (userDetected != _userDetected) {
        _userDetected = userDetected;
        if (_userDetected) {
            midi.sendControlChange(KinectMidiChannel, 20, 127);
            midi.sendControlChange(VideoMidiChannel, 20, 127);
        }else{
            midi.sendControlChange(KinectMidiChannel, 21, 127);
            midi.sendControlChange(VideoMidiChannel, 20, 0);
        }
    }
}

// ===== ARDUINO =====

void testApp::setupArduino(const int &version){
    ofRemoveListener(arduino.EInitialized, this, &testApp::setupArduino);
    arduinoIsSetup = true;
    ofLogNotice() << arduino.getFirmwareName();
    ofLogNotice() << "firmata version " << arduino.getMajorFirmwareVersion() << "." << arduino.getMinorFirmwareVersion();
    
    //de arduino pinnen instellen op hoe ze zullen worden gebruikt...
    arduino.sendAnalogPinReporting(0, ARD_ANALOG);
    arduino.sendAnalogPinReporting(knockSensorPin, ARD_ANALOG);
    arduino.sendAnalogPinReporting(2, ARD_ANALOG);
    
    arduino.sendDigitalPinMode(13, ARD_OUTPUT);
    arduino.sendDigitalPinMode(calibratingPin, ARD_OUTPUT);
    arduino.sendDigitalPinMode(waitingForUserPin, ARD_OUTPUT);
    arduino.sendDigitalPinMode(letsDancePin, ARD_OUTPUT);
    arduino.sendDigitalPinMode(2, ARD_INPUT);
    
    
    // luistern naar de events of er iets gebeurt met de pinnen.
    ofAddListener(arduino.EAnalogPinChanged, this, &testApp::analogPinChanged);
    ofAddListener(arduino.EDigitalPinChanged, this, &testApp::digitalPinChanged);
    
}

void testApp::updateArduino(){
    arduino.update();
    
    if (arduinoIsSetup) {
        //hier pas dingen versturen naar de arduino. anders is de connectie nog niet tot stand te komen.
    }
    
}

void testApp::digitalPinChanged(const int &pinNum){
    switch (pinNum) {
        case 2:
            if (arduino.getDigital(2) != ARD_HIGH) {
                midi.sendControlChange(15, 20,127);
                arduino.sendDigital(13, ARD_HIGH);
            }else{
                midi.sendControlChange(15, 20, 0);
                arduino.sendDigital(13, ARD_LOW);
            }
            break;
            
        default:
            //do nothing.
            break;
    }
}

void testApp::analogPinChanged(const int &pinNum){
    switch (pinNum) {
        case 0:
            {
            float potval = arduino.getAnalog(0);
            int ccval = (potval/1024)*128;
                ofLogNotice() << "ccval: " << ccval;
            midi.sendControlChange(VideoMidiChannel, 1, ccval);
            }
            break;
            
        default:
            {
                int sensorval = arduino.getAnalog(pinNum);
                ofLogNotice() << "knocksensor: " << sensorval;
                if (sensorval > knockTreshold) {
                    KnockHandler(pinNum);
                }
            }
            break;
            

    }
}

void testApp::KnockHandler(int pinNum){
    midi.sendControlChange(VideoMidiChannel, (30+pinNum), 127);
    
    //TODO: muzieknoten versturen naar live.
}

// ===== MIDI =====

void testApp::sendMidiSignals(){
    //leap vingers. Elke midi noot die een aansignaal krijgt moet ook weer uitgezet worden. Blijven commando's sturen is uit den boze!
    for (int i = 0; i < 10; i++) {
        if (i < fingersYpos.size()) {
            if (!playingChordNotes[i]) {
                midi.sendNoteOn(i+1, chordNotes[i],127);
                playingChordNotes[i] = true;
            }
            midi.sendControlChange(16, i+1, (fingersYpos[i].y/480)*127);
        }else{
            if (playingChordNotes[i]) {
                midi.sendNoteOff(i+1, chordNotes[i]);
                playingChordNotes[i] = false;
            }
        }
    }
    
    //kinect signalen
    if (_userDetected) {
        ofLogNotice() << "rhand x: " << Rhand.x;
        midi.sendControlChange(KinectMidiChannel, 4, (Lhand.x/640) *127);
        midi.sendControlChange(KinectMidiChannel, 5, (Lhand.y/480) *127);

        midi.sendControlChange(KinectMidiChannel, 7, (Rhand.x/640) *127);
        midi.sendControlChange(KinectMidiChannel, 8, (Rhand.y/480) *127);
        
        midi.sendControlChange(VideoMidiChannel, 1, (Lhand.x/640) *127);
        
        midi.sendControlChange(VideoMidiChannel, 2, (Rhand.x/640) *127);
        midi.sendControlChange(VideoMidiChannel, 3, (Rhand.y/480) *127);

    }
}


// ===== HELPER FUNCTIONS =====
//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch (key) {
        case 's':
            showData = !showData;
            break;
        case 'r':
            cam.reset();
            break;
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    ofLogNotice() << "GotMessage: " << msg.message;
}

