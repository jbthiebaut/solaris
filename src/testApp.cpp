#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup(){


	#ifdef _USE_LIVE_VIDEO
        vidGrabber.setVerbose(true);
        vidGrabber.initGrabber(320,240);
	#else
		it=1;
	//videos.push_back("video24.mov");
	videos.push_back("video23.mov");
	videos.push_back("video12.mov");
	videos.push_back("video13.mov");
	videos.push_back("video14.mov");
	videos.push_back("video15.mov");
	videos.push_back("video16.mov");
	videos.push_back("video17.mov");
	videos.push_back("video18.mov");
	videos.push_back("video11.mov");
	videos.push_back("video10.mov");
	videos.push_back("video9.mov");
	videos.push_back("video8.mov");
	videos.push_back("video7.mov");
	videos.push_back("video5.mov");
	videos.push_back("video4.mov");
	videos.push_back("video3.mov");
	videos.push_back("video2.mov");
	videos.push_back("video1.mov");
	
        vidPlayer.loadMovie(videos[it]);
	frameCount = vidPlayer.getTotalNumFrames();
		it++;
		vidPlayer.setSpeed(0.3);
        vidPlayer.play();
	#endif

    colorImg.allocate(320,240);
    colorImg2.allocate(320,240);
	grayImage.allocate(320,240);
	grayBg.allocate(320,240);
	grayDiff.allocate(320,240);

	bLearnBakground = false;
	threshold = 150;
	
	// open an outgoing connection to HOST:PORT
	sender.setup( HOST, PORT );
}

//--------------------------------------------------------------
void testApp::update(){
	ofBackground(100,100,100);

    bool bNewFrame = false;

	#ifdef _USE_LIVE_VIDEO
       vidGrabber.grabFrame();
	   bNewFrame = vidGrabber.isFrameNew();
    #else
        vidPlayer.idleMovie();
//		go to next video if we reached the end of current one
		if (frameCount==vidPlayer.getCurrentFrame())
		{
			vidPlayer.loadMovie(videos[it]);
			frameCount = vidPlayer.getTotalNumFrames();
			vidPlayer.setSpeed(0.3);
			vidPlayer.play();
			it = it%18 +1;
			cout << it;
			// change vid
		}
        bNewFrame = vidPlayer.isFrameNew();
	#endif

	if (bNewFrame){

		#ifdef _USE_LIVE_VIDEO
            colorImg.setFromPixels(vidGrabber.getPixels(), 320,240);
	    #else
            colorImg.setFromPixels(vidPlayer.getPixels(), 320,240);
        #endif

        grayImage = colorImg;
		if (bLearnBakground == true){
			grayBg = grayImage;		// the = sign copys the pixels from grayImage into grayBg (operator overloading)
			bLearnBakground = false;
		}

		// take the abs value of the difference between background and incoming and then threshold:
		grayDiff.absDiff(grayBg, grayImage);
		grayDiff.threshold(threshold);

		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		contourFinder.findContours(grayDiff, 20, (340*240)/3, 10, true);	// find holes
		for (int i=0; i<contourFinder.blobs.size();i++)
		{
			//get blob rectangle in colorImage
			colorImg2=colorImg;
			colorImg2.setROI(contourFinder.blobs[i].boundingRect);
			//colorImg2.convertRgbToHsv();
			unsigned char* roi = colorImg2.getRoiPixels();
			colorImg2.resetROI();
			
			//calculate mean color for the retangle
			long sum=0;
			long hue=0;
			long sat=0;
			long light=0;

			while(roi[sum]!='\0')
			{
				hue+=roi[sum];
				sat+=roi[sum+1];
				light+=roi[sum+2];
				sum+=3;
			}
			//store mean color in blobs.meanColor
			if (sum>0)
			{
				contourFinder.blobs[i].meanColor.r = hue = hue/(sum/3);
				contourFinder.blobs[i].meanColor.g = sat =(sat*3)/sum;
				contourFinder.blobs[i].meanColor.b = light = (light*3)/sum;
				
			
				ofxOscMessage m;
				m.setAddress( "/blob" );
				m.addIntArg( i );
				m.addIntArg( hue );
				m.addIntArg( sat );
				m.addIntArg( light );
				m.addIntArg( sum/3 );
				m.addFloatArg(contourFinder.blobs[i].area);
				m.addFloatArg(contourFinder.blobs[i].length);
				m.addIntArg(contourFinder.blobs[i].centroid.x);
				m.addIntArg(contourFinder.blobs[i].centroid.y);
				m.addFloatArg( ofGetElapsedTimef() );
				sender.sendMessage( m );
			}
			
		}
	}

}

//--------------------------------------------------------------
void testApp::draw(){

	// draw the incoming, the grayscale, the bg and the thresholded difference
	ofSetColor(0xffffff);
	colorImg.draw(20,20);
//	grayImage.draw(360,20);
	//grayBg.draw(20,280);
//	grayDiff.draw(360,280);

	// then draw the contours:

	ofFill();
	ofSetColor(0x333333);
	ofRect(360,20,320,240);
	ofSetColor(0xffffff);

	// we could draw the whole contour finder
	//contourFinder.draw(20,280);

	// or, instead we can draw each blob individually,
	// this is how to get access to them:
	for (int i = 0; i < contourFinder.blobs.size(); i++){
        contourFinder.blobs[i].draw(360,20,true);
		
		//draw rectangles for the colour of the blobs
		ofFill();
		ofSetColor(contourFinder.blobs[i].meanColor.r,contourFinder.blobs[i].meanColor.g,contourFinder.blobs[i].meanColor.b);
		ofRect(60*i+20,300,40,40);
		
    }

	// finally, a report:

	ofSetColor(0xffffff);
	char reportStr[1024];
	sprintf(reportStr, "num blobs found %i, fps: %f \n ", contourFinder.blobs.size(), ofGetFrameRate());
	ofDrawBitmapString(reportStr, 20, 400);
	
	

}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){

	switch (key){
		case ' ':
			bLearnBakground = true;
			break;
		case '+':
			threshold ++;
			if (threshold > 255) threshold = 255;
			break;
		case '-':
			threshold --;
			if (threshold < 0) threshold = 0;
			break;
	}
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

