//
//  loopFindThread.h
//  loopFindr
//
//  Created by Collin Burger on 5/3/14.
//
//
//#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "loopFindEvents.h"

//#ifndef loopFindr_loopFindThread_h
//#define loopFindr_loopFindThread_h

class loopFindThread : public ofThread {
public:
    vector< vector<ofImage> > *loops;
    vector< vector<int> > *loopIndeces;
    vector< vector<ofImage*> > *displayLoops;
    vector< cv::Mat > *loopStartMats;
    vector<int> matchIndeces;
    vector<int> *loopLengths;
    vector<int> *loopPlayIdx;
    vector<float> *loopQuality;
    float loopWidth;
    float loopHeight;
    
    int minPeriod;
    int maxPeriod;
    int frameStart;
    int potentialEndIdx;
    
    vector< cv::Mat > potentialLoopEnds;
    vector<cv::Mat> bestMatches;
    
    //cv::Size imResize;
    
    float minMovementThresh;
    float maxMovementThresh;
    float loopThresh;
    float minChangeRatio;
    bool minMovementBool;
    bool maxMovementBool;
    ofQTKitPlayer vidPlayer;
    string videoPath;
    bool isSetup;
    
    ofEvent<loopFoundEventArgs> newloopEvent;
    loopFindEvents loopEvents;

     /*vector< vector<ofImage> > loops = ofApp.loops;
     vector< vector<int> > loopIndeces = ofApp.loopIndeces;
     vector< vector<int> > loopIndeces = ofApp.displayLoops;
     vector< cv::Mat > loopStartMats = ofApp.loopStartMats;
     vector<int> matchIndeces;
     vector<int> loopLengths = ofApp.loopLengths;
     vector<int> loopPlayIdx = ofApp.loopPlayIdx;
     vector<float> loopQuality = ofApp.loopQuality;
     float loopWidth = ofApp.loopWidth;
     float loopHeight = ofApp.loopWidth;
     
     int minPeriod = ofApp.minPeriod;
     int maxPeriod = ofApp.maxPeriod;
     int frameStart = ofApp.frameStart;
     int potentialEndIdx;
     
     vector< cv::Mat > potentialLoopEnds;
     vector<cv::Mat> bestMatches;
     
     cv::Size imResize;
     
     float minMovementThresh = ofApp.minMovementThresh;
     float maxMovementThresh = ofApp.maxMovementThresh;
     float loopThresh = ofApp.loopThresh;
     float minChangeRatio = ofApp.minChangeRatio;
     bool minMovementBool = ofApp.minMovementBool;
     bool maxMovementBool = ofApp.maxMovementBool;
    ofQTKitPlayer vidPlayer = ofApp.vidPlayer;*/
public:
    void setup(vector< vector<ofImage> > *loops, vector< vector<int> > *loopIndeces,vector< vector<ofImage*> > *displayLoops,vector< cv::Mat > *loopStartMats,vector<int> *loopLengths,vector<int> *loopPlayIdx,vector<float> *loopQuality,float loopWidth,float loopHeight,int minPeriod,int maxPeriod,int frameStart,vector< cv::Mat > *potentialLoopEnds,/*cv::Size imResize,*/float minMovementThresh,float maxMovementThresh,float loopThresh,float minChangeRatio,bool minMovementBool,bool maxMovementBool, string videoPath){
        this->loops = loops;
        this->loopIndeces = loopIndeces;
        this->displayLoops = displayLoops;
        this->loopStartMats = loopStartMats;
        this->loopLengths = loopLengths;
        this->loopPlayIdx = loopPlayIdx;
        this->loopQuality = loopQuality;
        this->loopWidth = loopWidth;
        this->loopHeight = loopHeight;
        this->minPeriod = minPeriod;
        this->maxPeriod = maxPeriod;
        this->frameStart = frameStart;
        this->potentialLoopEnds = *potentialLoopEnds;
        //this->imResize = imResize;
        this->minMovementThresh = minMovementThresh;
        this->maxMovementThresh = maxMovementThresh;
        this->loopThresh = loopThresh;
        this->minChangeRatio = minChangeRatio;
        this->minMovementBool = minMovementBool;
        this->maxMovementBool = maxMovementBool;
        //this->vidPlayer = vidPlayer;
        this->videoPath = videoPath;
        isSetup = true;
        
    }
    
    // the thread function
    void threadedFunction() {
        
        // start
        
        //while(isThreadRunning()) {
            lock();
        if(isSetup){
            vidPlayer.loadMovie(videoPath, OF_QTKIT_DECODE_PIXELS_ONLY);
            //vidPlayer.setUseTexture(false);
            checkForLoop();
        }
            unlock();
        //}
        
        isSetup = false;
        // done
    }
    
    //------------------------------------------------------------------------------------
    bool checkForLoop(){
        if (potentialLoopEnds.size() == 0)
            return false;
        cv::Mat start;
        potentialLoopEnds.at(0).copyTo(start);
        cv::Scalar startMean = cv::mean(start);
        cv::subtract(start, startMean, start);
        float startSum = cv::sum(start)[0] + 1; //add one to get rid of division by 0 if screen is black
        cv::Mat currEnd;
        for (int i = minPeriod; i < potentialLoopEnds.size(); i++) {
            cv::Mat currEnd;
            potentialLoopEnds.at(i).copyTo(currEnd);
            cv::subtract(currEnd, startMean, currEnd);
            cv::Mat diff;
            cv::absdiff(start, currEnd, diff);
            float endDiff = cv::sum(diff)[0] + 1;
            float changeRatio = endDiff/startSum;
            
            
            if (changeRatio < minChangeRatio) {
                minChangeRatio = changeRatio;
                //cout << "min Ratio: " << minChangeRatio << endl;
            }
            
            cout << "frameStart: " << frameStart << endl;
            cout << "compare Frame: " << frameStart + i << endl;
            cout << "startSum: " << startSum << endl;
            cout << "endSum: " << cv::sum(currEnd)[0] + 1 << endl;
            cout << "endDiff: " << endDiff << endl;
            cout << "change Ratio: " << changeRatio*100 << endl;
            cout << "test Thresh: " << 100 - loopThresh << endl;
            cout << endl;
            
            if ((changeRatio*100) < (100 - loopThresh)){//set to change percentage
                potentialEndIdx = frameStart + i;
                //cout << "potentialEnd: " << potentialEndIdx << endl;
                if (minMovementBool || maxMovementBool){
                    if  (hasMovement()){
                        matchIndeces.push_back(potentialEndIdx);
                        bestMatches.push_back(currEnd);
                    }
                    else{
                        //timeline.setCurrentFrame(potentialEndIdx);
                        //frameStart = potentialEndIdx;
                        //timeline.setCurrentFrame(frameStart);
                        //vidPlayer.setFrame(timeline.getCurrentFrame());
                        //vidPlayer.setFrame(frameStart);
                        //vidPlayer.update();
                        return false;
                    }
                }
                else{
                    matchIndeces.push_back(potentialEndIdx);
                    bestMatches.push_back(currEnd);
                }
            }
        }
        
        if (bestMatches.size() > 0) {
            //vidPlayer.setFrame(frameStart);
            //vidPlayer.update();
            getBestLoop(start,startMean);
            //vidPlayer.setFrame(frameStart);
            //vidPlayer.update();
            return true;
        }
        
        //vidPlayer.setFrame(frameStart);
        //vidPlayer.update();
        return false;
    }
    
    //------------------------------------------------------------------------------------
    bool hasMovement(){
        float sumDiff = 0;
        
        cv::Mat prevFrame;
        potentialLoopEnds.at(0).copyTo(prevFrame);
        cv::Scalar prevMean = cv::mean(prevFrame); //DO MEAN INSIDE LOOP?
        cv::subtract(prevFrame,prevMean,prevFrame);
        float prevSum = cv::sum(prevFrame)[0] + 1; //add one to get rid of division by 0 if screen is black
        
        cv::Mat currFrame;
        
        //for (int i = 1; i <= potentialEndIdx - timeline.getCurrentFrame(); i++) {
        for (int i = 1; i <= potentialEndIdx - frameStart; i++) {
            cv::Mat currFrame;
            potentialLoopEnds.at(i).copyTo(currFrame);
            cv::subtract(currFrame, prevMean, currFrame);
            cv::Mat diff;
            cv::absdiff(currFrame,prevFrame,diff);
            float endDiff = cv::sum(diff)[0] + 1;
            float changeRatio = endDiff;///prevSum;
            //cout << "movement Change Ratio: " << changeRatio << endl;
            sumDiff += changeRatio;
            prevFrame = currFrame;
            //prevSum = cv::sum(prevFrame)[0] + 1;
        }
        sumDiff /= prevSum;
        //cout << "sum Diff: " << sumDiff << endl;
        //cout << "minThreshold: " << minMovementThresh*(potentialEndIdx - frameStart)/100 << endl;
        if (minMovementBool) {
            //if(sumDiff < minMovementThresh*(potentialEndIdx - frameStart)/100 ){
            if(sumDiff < minMovementThresh/100 ){
                cout << "not enough movement" << endl;
                return false;
            }
        }
        else if (maxMovementBool){
            //if(sumDiff > maxMovementThresh*(potentialEndIdx - frameStart)/100 ){
            if(sumDiff > maxMovementThresh/100 ){
                cout << "too much movement" << endl;
                return false;
            }
        }
        else if (minMovementBool && maxMovementBool){
            //if(sumDiff > maxMovementThresh*(potentialEndIdx - frameStart)/100 && sumDiff < minMovementThresh*(potentialEndIdx - frameStart)/100){
            if(sumDiff > maxMovementThresh/100 && sumDiff < minMovementThresh/100){
                cout << "not enough movement or too much" << endl;
                return false;
            }
        }
        
        return true;
    }
    
    //------------------------------------------------------------------------------------
    void getBestLoop(cv::Mat start,cv::Scalar startMean){
        float minChange = MAXFLOAT;
        int bestEnd = -1;
        float startSum = cv::sum(start)[0] + 1; //add one to get rid of division by 0 if screen is black
        
        for (int i = 0; i < bestMatches.size(); i++) {
            cv::Mat currEnd;
            potentialLoopEnds.at(i).copyTo(currEnd);
            cv::subtract(currEnd,startMean,currEnd);
            cv::Mat diff;
            cv::absdiff(start, currEnd, diff);
            float endDiff = cv::sum(diff)[0] + 1;
            float changeRatio = endDiff/startSum;
            
            //cout << "end of loop frame: " << matchIndeces.at(i) << endl;
            //add flow test here
            if (changeRatio <= minChange) {
                minChange = changeRatio;
                bestEnd = i;
            }
        }
        
        //ofQTKitDecodeMode decodeMode = OF_QTKIT_DECODE_PIXELS_ONLY;
        //ofQTKitPlayer loopFrameGrabber;
        //loopFrameGrabber.loadMovie(videoFilePath,decodeMode);
        if (bestEnd >= 0) {
            vector< ofImage*> display;
            vidPlayer.setPaused(true);
            vidPlayer.setFrame(frameStart);
            vidPlayer.update();
            
            for (int j = frameStart; j <= matchIndeces.at(bestEnd); j++) {
                //loopFrameGrabber.setFrame(j);
                //loopFrameGrabber.update();
                //vidPlayer.setFrame(j);
                //vidPlayer.update();
                ofImage loopee;
                loopee.setUseTexture(false);
                loopee.setFromPixels(vidPlayer.getPixels(), vidPlayer.getWidth(), vidPlayer.getHeight(), OF_IMAGE_COLOR);
                //loopee.setFromPixels(loopFrameGrabber.getPixels(), loopFrameGrabber.getWidth(), loopFrameGrabber.getHeight(), OF_IMAGE_COLOR);
                loopee.update();
                ofImage *displayIm = new ofImage();
                displayIm->clone(loopee);
                displayIm->resize(loopWidth,loopHeight);
                //displayIm.resize(vidPlayer.getWidth()/numLoopsInRow, vidPlayer.getHeight()/numLoopsInRow);
                displayIm->update();
                //displayIm.resize(loopFrameGrabber.getWidth()/numLoopsInRow, loopFrameGrabber.getHeight()/numLoopsInRow);
                display.push_back(displayIm);
                vidPlayer.nextFrame();
                cout << "Updating Vid Player" << endl;
                vidPlayer.update();
            }
            //vidPlayer.setFrame(frameStart);
            //cout << "Updating Vid Player" << endl;
            //vidPlayer.update();
            //loopFrameGrabber.close();
            
            //ofVec2f indices = ofVec2f(frameStart, matchIndeces.at(bestEnd));
            //int indices[2] = {frameStart,matchIndeces.at(bestEnd)};
            vector<int> indices;
            
            indices.push_back(frameStart);
            indices.push_back(matchIndeces.at(bestEnd));
            loopIndeces->push_back(indices);
            loopLengths->push_back(display.size());
            displayLoops->push_back(display);
            loopQuality->push_back(minChange);
            //loopQuality.push_back(1 - (minChange/100));
            loopPlayIdx->push_back(0);
            loopStartMats->push_back(start);
            
            loopFoundEventArgs args;
            args.loopThread = this;
            args.indices = indices;
            ofNotifyEvent(loopEvents.loopFoundEvent, args);
    
            //ditchSimilarLoop();
        }
        
        bestMatches.clear();
        matchIndeces.clear();
    }

    
};


//#endif
