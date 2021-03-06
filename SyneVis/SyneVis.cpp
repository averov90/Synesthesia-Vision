#include "stdafx.h"

#include "synesthesia.h"
#include "wavutils.h"
#include "cloudapi.h"
#include "wavplayer.h"
#include "consts.h"

using namespace cv;
using namespace std;

extern VideoCapture camera_capture_syne; //Capture frpm synestesia.cpp
extern bool syne_brightness_mode; //Synesthesia mode


int main(int argc, char *argv[])
{
	curl_global_init(CURL_GLOBAL_DEFAULT); //Initialising Libcurl
	Pa_Initialize(); //Initialising PortAudio
	VideoCapture cap(0);
	Syne_Initialise(); //Initialise synestesia
	camera_capture_syne = cap;
	Mat frame;
	camera_capture_syne >> frame;
	Syne_Start(); //Start synestesia
	
	cout << "started" << endl;

	string str;
	
	
	/*Cloud_ImageDescriptor(frame, str);
	cout << str << endl;
	Cloud_TTS(str);
	WavePlayWaitD(str.c_str());*/
	
	while (true)
	{
		Pa_Sleep(500);
		cap >> frame;
		imshow("fr", frame);
		waitKey(1);
	}
	//Pa_Sleep(10 * 1000);
	cout << "stopped" << endl;

    return 0;
}