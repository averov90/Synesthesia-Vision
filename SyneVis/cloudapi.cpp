#include "stdafx.h"
#include "cloudapi.h"

#include "consts.h"


using namespace std;
using namespace cv;

static string tts_url(string("https://texttospeech.googleapis.com/v1beta1/text:synthesize?key=") + CLOUD_TTS_AUTH_KEY), 
	imdesc_url(string("https://vision.googleapis.com/v1/images:annotate?key=") + CLOUD_ImageDescriptor_AUTH_KEY),
	tts_req_part("'},'voice':{'languageCode':'en-US','name':'en-GB-Standard-D','ssmlGender':'MALE'},'audioConfig':{'audioEncoding':'LINEAR16','sampleRateHertz':" + to_string(SAMPLE_RATE) + ",'speakingRate':0.65,'pitch':1}}"),
	tmp;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

bool Cloud_TTS(string &text) {
	CURL *curl = curl_easy_init();
	if (!curl) return false;
	curl_slist * list = curl_slist_append(nullptr, "Content-Type: application/json");
	tmp = "{'input':{'text':'" + text + tts_req_part;
	curl_easy_setopt(curl, CURLOPT_URL, tts_url.c_str());
	curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, tmp.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	text = "";
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &text);
	if (curl_easy_perform(curl) != CURLcode::CURLE_OK) return false;
	curl_easy_cleanup(curl);
	unsigned int fpos = (unsigned int)text.find_first_of(':') + 3;
	text = base64_decode(text.substr(fpos, text.find_last_of('=') - fpos + 1));
	return true;
}

bool Cloud_ImageDescriptor(const cv::Mat &image, string &result) {
	unsigned char *jpegBuf = nullptr;
	unsigned long jpegSize = 0;
	tjhandle tjInstance = tjInitCompress();
	if (!tjInstance) return false;
	if (tjCompress2(tjInstance, image.data, image.cols, 0, image.rows, TJPF_BGR, &jpegBuf, &jpegSize, TJSAMP_422, 85, TJFLAG_FASTDCT) < 0) return false;
	tjDestroy(tjInstance);
	std::string img_encoded = base64_encode(jpegBuf, (unsigned int)jpegSize);
	tjFree(jpegBuf);
	CURL *curl = curl_easy_init();
	if (!curl) return false;
	curl_slist *list = curl_slist_append(nullptr, "Content-Type: application/json");
	tmp = "{'requests':[{'image':{'content':'" + img_encoded + "'},'features':[{'type':'LABEL_DETECTION'}]}]}";
	curl_easy_setopt(curl, CURLOPT_URL, imdesc_url.c_str());
	curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, tmp.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	string resp;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
	if (curl_easy_perform(curl) != CURLcode::CURLE_OK) return false;
	curl_easy_cleanup(curl);

	result = "Found: ";
	unsigned int pos1 = (unsigned int)resp.find("n\":") + 5, pos2 = (unsigned int)resp.find_first_of('\"', pos1); //Finding "description" by latest symbols
	result += resp.substr(pos1, pos2 - pos1);
	pos1 = (unsigned int)resp.find("e\":", pos2) + 4; pos2 = (unsigned int)resp.find_first_of(',', pos1); //Finding "score" by latest symbols
	float msc = stof(resp.substr(pos1, pos2 - pos1));
	for (char i = 0; i != 10; ++i)
	{
		pos1 = (unsigned int)resp.find("n\":", pos2) + 5; pos2 = (unsigned int)resp.find_first_of('\"', pos1); //Finding "description" by latest symbols
		tmp = resp.substr(pos1, pos2 - pos1);
		pos1 = (unsigned int)resp.find("e\":", pos2) + 4; pos2 = (unsigned int)resp.find_first_of(',', pos1); //Finding "score" by latest symbols
		if (msc - stof(resp.substr(pos1, pos2 - pos1)) < 0.1)
			result += ", or " + tmp;
		else break;
	}
	return true;
}





