#include "testApp.h"

// ===== OPEN FRAMEWORKS =====
void testApp::setup(){
    ofBackground(0);
    ofSetFrameRate(25);
    ofSetCircleResolution(50);
    
    //Midisetup: poorten oplijsten + virtuele poort aanmaken + de constante waarden aanmaken
    midi.listPorts();
    midi.openVirtualPort("MaanSteen");
    for (int i = 0; i < 6; i++) {
        playingPads.push_back(false);
    }
    
    
    //start de leapmotion
    leap.open();
    
    //setup the arduino over firmata
//    arduino.connect("/dev/tty.usbmodem14531",57600);
    ofAddListener(arduino.EInitialized, this, &testApp::setupArduino);
    arduinoIsSetup = false;
    treshold.setup("Pads Treshold", _treshold, 0, 80);
    treshold.setPosition(600, 330);
    treshold.addListener(this, &testApp::setTreshold);
    
    //kinect device aanspreken voor beeldhoek
    kinect.setup();
    KinectAngleSlider.setup("Kinect Angle", kinect.getTiltAngle(), -30, 30);
    KinectAngleSlider.setPosition(300, 330);
    KinectAngleSlider.addListener(this, &testApp::setKinectAngle);
    //kinect oproepen via openNI
    NIContext.setup();
    NIdepthGen.setup(&NIContext);
    NIuserGen.setup(&NIContext);
    NIuserGen.setMaxNumberOfUsers(1);
    NIContext.registerViewport();
    _userDetected = false;
    Rhand.x = 0;
    Rhand.y = 0;
    Lhand.x = 0;
    Lhand.y = 0;
    
    
    //syphon server opstarten voor het doorsturen van de beelden naar het syphon framework
    syphonOutput.setName("Maansteen");
    
    //particles setup
    setupParticles();
    
}

void testApp::update(){
    GetLeapData();
    updateArduino();
    updateKinectData();
    
    sendMidiSignals();
}

void testApp::draw(){
    ofEnableBlendMode(OF_BLENDMODE_ADD);

    cam.begin();
    // Draw system.
    ofSetLineWidth(2.0);
    particleSystem.draw(tex);
    cam.end();

    // hiervoor alle drawing die naar Syphon wordt gestuurd.
    syphonOutput.publishScreen();
    
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);
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
        NIdepthGen.draw(0,0,440,330);
        NIuserGen.draw(440,330);
        ofDrawBitmapString("Kinect DepthImage + Users", 5, 340);
        if (_userDetected) {
            ofSetColor(255, 255, 50);
            ofCircle((Lhand.x / 640)*440, (Lhand.y / 480)*330, 10);
            ofCircle((Rhand.x / 640)*440, (Rhand.y / 480)*330, 10);
            ofSetColor(255);
            ofDrawBitmapString("L-X: "+ofToString(Lhand.x), 440, 40);
            ofDrawBitmapString("L-Y: "+ofToString(Lhand.y), 440, 70);
            ofDrawBitmapString("R-X: "+ofToString(Rhand.x), 440, 100);
            ofDrawBitmapString("R-Y: "+ofToString(Rhand.y), 440, 130);
        }
        ofSetColor(255);
        KinectAngleSlider.draw();
        ofLine(0, 350, 1024, 350);
        
        //padsdata
        treshold.draw();
        ofLine(550, 0, 550, 350);
        ofDrawBitmapString("Drumpads (test met nummers 1-6)", 560, 14);
        for (int i = 0; i < playingPads.size(); i++) {
            if (playingPads[i]) {
                ofSetColor(0,200,30);
            }else{
                ofSetColor(255);
            }
            ofCircle(650 + (i * 120) - (round(i/3) * 360), 100 + (round(i/3) * 120), 40);
        }

    }
    ofSetColor(255);
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
    leftEmitter.posSpread = ofVec3f(1, 1, 1);
    leftEmitter.velSpread = ofVec3f(200,200,200);
    leftEmitter.life = 2;
    leftEmitter.lifeSpread = 10;
    leftEmitter.numPars = 1;
    leftEmitter.color = ofColor(230, 255, 50);
    leftEmitter.colorSpread = ofColor(50, 50, 50);
    leftEmitter.size = 22;
    
    rightEmitter.setPosition(ofVec3f(100, 100));
    rightEmitter.setVelocity(ofVec3f(-150.0, 150.0, 150.0));
    rightEmitter.posSpread = ofVec3f(10, 10, 10);
    rightEmitter.velSpread = ofVec3f(200,200,200);
    rightEmitter.life = 2;
    rightEmitter.lifeSpread = 10;
    rightEmitter.numPars = 1;
    rightEmitter.size = 22;
    rightEmitter.color = ofColor(0, 200, 255);
    rightEmitter.colorSpread = ofColor(50, 50, 50);
    
    vectorField.allocate(128, 128, 3);
    ofLoadImage(tex, "tex.png");
    
    rotAcc = 4500;
    gravAcc = 13500;
    drag = 0.5;
    fieldMult = 30.0;

}

void testApp::updateParticles(){
    switch (simplehands.size()) {
        case 0:
            midi.sendControlChange(LeapMidiChannel, 20, 0);
            midi.sendControlChange(LeapMidiChannel, 21, 0);
            leftEmitter.numPars = 0;
            rightEmitter.numPars = 0;
            break;
            
        case 1:
            midi.sendControlChange(LeapMidiChannel, 20, 127);
            midi.sendControlChange(LeapMidiChannel, 21, 0);
            leftEmitter.numPars = floor((abs(prevLX - simplehands[0].handPos.x)+abs(prevLY - simplehands[0].handPos.y))/3);
            rightEmitter.numPars = 0;
            leftEmitter.setPosition(ofVec3f(simplehands[0].handPos.x*2, simplehands[0].handPos.y*2 - 400));
            prevLX = simplehands[0].handPos.x;
            prevLY = simplehands[0].handPos.y;
            break;
            
        case 2:
            midi.sendControlChange(LeapMidiChannel, 20, 127);
            midi.sendControlChange(LeapMidiChannel, 21, 127);
            leftEmitter.numPars = floor((abs(prevLX - simplehands[0].handPos.x)+abs(prevLY - simplehands[0].handPos.y))/3);
            rightEmitter.numPars = floor((abs(prevRX - simplehands[1].handPos.x)+abs(prevRY - simplehands[1].handPos.y))/3);
            leftEmitter.setPosition(ofVec3f(simplehands[0].handPos.x*2, simplehands[0].handPos.y*2 -400));
            rightEmitter.setPosition(ofVec3f(simplehands[1].handPos.x*2, simplehands[1].handPos.y*2 -400));
            prevLX = simplehands[0].handPos.x;
            prevLY = simplehands[0].handPos.y;
            prevRX = simplehands[1].handPos.x;
            prevRY = simplehands[1].handPos.y;
            break;
            
        default:
            break;
    }
    
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
        midi.sendControlChange(KinectMidiChannel, 3, (user.neck.position[1].X /640) *127);
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
            midi.sendControlChange(KinectMidiChannel, 1, 127);
            midi.sendControlChange(VideoMidiChannel, 20, 127);
        }else{
            midi.sendControlChange(KinectMidiChannel, 1, 0);
            midi.sendControlChange(VideoMidiChannel, 20, 0);
        }
    }
}

void testApp::setKinectAngle(int & angle){
    kinect.setTiltAngle(angle);
}

// ===== ARDUINO =====

void testApp::setupArduino(const int &version){
    ofRemoveListener(arduino.EInitialized, this, &testApp::setupArduino);
    arduinoIsSetup = true;
    ofLogNotice() << arduino.getFirmwareName();
    ofLogNotice() << "firmata version " << arduino.getMajorFirmwareVersion() << "." << arduino.getMinorFirmwareVersion();
    
    //de arduino pinnen instellen op hoe ze zullen worden gebruikt...
    arduino.sendAnalogPinReporting(0, ARD_ANALOG);
    arduino.sendAnalogPinReporting(1, ARD_ANALOG);
    arduino.sendAnalogPinReporting(2, ARD_ANALOG);
    arduino.sendAnalogPinReporting(3, ARD_ANALOG);
    arduino.sendAnalogPinReporting(4, ARD_ANALOG);
    arduino.sendAnalogPinReporting(5, ARD_ANALOG);

    
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
    int sensorval = arduino.getAnalog(pinNum);
//    ofLogNotice() << "knocksensor " << pinNum <<": " << sensorval;
    if (sensorval >> _treshold) {
        setPad(pinNum, true);
    }else{
        setPad(pinNum, false);
    }
}

void testApp::setTreshold(int & treshold){
    _treshold = treshold;
}

void testApp::setPad(int padNum, bool val){
    if (playingPads[padNum] != val) {
        playingPads[padNum] = val;
        if (val) {
            arduino.sendDigital(13, ARD_HIGH);
            midi.sendNoteOn(ArduinoMidiChannel, padNum);
        }else{
            arduino.sendDigital(13, ARD_LOW);
            midi.sendNoteOff(ArduinoMidiChannel, padNum);
        }
    }
}

// ===== MIDI =====

void testApp::sendMidiSignals(){
    //leap vingers. Elke midi noot die een aansignaal krijgt moet ook weer uitgezet worden. Blijven commando's sturen is uit den boze!
    for (int i = 0; i < simplehands.size(); i++) {
        int midiStartValue;
        if (i == 1) {
            midiStartValue = 15;
        }else{
            midiStartValue = 10;
        }
        
        for (int j = 0; j < simplehands[i].fingers.size(); j++) {
            midi.sendControlChange(LeapMidiChannel, midiStartValue + j, (simplehands[i].fingers[j].pos.y /500)*127);
        }
    }
    
    //kinect signalen
    if (_userDetected) {
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
        case '1':
            setPad(0, true);
            break;
        case '2':
            setPad(1, true);
            break;
        case '3':
            setPad(2, true);
            break;
        case '4':
            setPad(3, true);
            break;
        case '5':
            setPad(4, true);
            break;
        case '6':
            setPad(5, true);
            break;
        case 'u':
            if (kinect.getTiltAngle() <= 25) {
                kinect.setTiltAngle(kinect.getTiltAngle() + 2);
                kinect.update();
            }
            break;
        case 'd':
            kinect.setTiltAngle(kinect.getTiltAngle() - 2);
            kinect.update();
            break;
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
    switch (key) {
        case '1':
            setPad(0, false);
            break;
        case '2':
            setPad(1, false);
            break;
        case '3':
            setPad(2, false);
            break;
        case '4':
            setPad(3, false);
            break;
        case '5':
            setPad(4, false);
            break;
        case '6':
            setPad(5, false);
            break;
    }
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    ofLogNotice() << "GotMessage: " << msg.message;
}

