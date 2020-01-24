#include "stdafx.h"
#include "wavplayer.h"

#include "consts.h"
#include "wavutils.h"

struct WaveFileUniv {
	short *currpos, *endpos;
};

struct WaveFileW : WaveFileUniv {
	std::mutex lock;
	std::condition_variable wait_lk;
	bool needtosleep = true;
};

struct WaveFileNoW : WaveFileUniv {
	short *buffer;
	PaStream **stream;
};

static int PlayCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
	short *out = (short*)outputBuffer;
	WaveFileUniv *data = (WaveFileUniv*)userData;
	unsigned long i;
	for (i = 0; i != framesPerBuffer && data->currpos != data->endpos; ++i, ++data->currpos, ++out) {
		*out = *data->currpos;
	}
	for (; i != framesPerBuffer; ++i, ++out) *out = 0;
	if (data->currpos != data->endpos) return paContinue;
	return paComplete;
}

static void PlayFinishedCallbackNoW(void *userData) {
	WaveFileNoW *data = (WaveFileNoW*)userData;
	Pa_CloseStream(*data->stream);
	delete[] data->buffer;
	delete data->stream;
	delete data;
}

static void PlayFinishedCallbackW(void *userData) {
	WaveFileW *data = (WaveFileW*)userData;
	data->needtosleep = false;
	data->wait_lk.notify_one();
}

bool WavePlayNowaitD(const char *WAV_file) {
	WaveFileNoW *wavinfo = new WaveFileNoW;
	unsigned int bufflen;
	wavinfo->currpos = wavinfo->buffer = WavUtils::LoadWAVFromBuffer(WAV_file, bufflen);
	wavinfo->endpos = wavinfo->buffer + bufflen;
	wavinfo->stream = new PaStream*;
	if (Pa_OpenDefaultStream(wavinfo->stream, 0, 1, paInt16, SAMPLE_RATE, paFramesPerBufferUnspecified, PlayCallback, wavinfo)) { 
		delete[] wavinfo->buffer;
		delete wavinfo->stream;
		delete wavinfo;
		return false; 
	}
	Pa_SetStreamFinishedCallback(*wavinfo->stream, PlayFinishedCallbackNoW);
	if (Pa_StartStream(*wavinfo->stream)) {
		delete[] wavinfo->buffer;
		delete wavinfo->stream;
		delete wavinfo;
		return false;
	}
	return true;
};

bool WavePlayWaitD(const char *WAV_file) {
	WaveFileW wavinfo;
	unsigned int bufflen;
	short *buffer = WavUtils::LoadWAVFromBuffer(WAV_file, bufflen);
	wavinfo.currpos = buffer;
	wavinfo.endpos = buffer + bufflen;
	PaStream *stream;
	if (Pa_OpenDefaultStream(&stream, 0, 1, paInt16, SAMPLE_RATE, paFramesPerBufferUnspecified, PlayCallback, &wavinfo)) {
		delete[] buffer;
		return false;
	}
	Pa_SetStreamFinishedCallback(stream, PlayFinishedCallbackW);
	if (Pa_StartStream(stream)) {
		delete[] buffer;
		return false;
	}
	std::unique_lock<std::mutex> locker(wavinfo.lock);
	while (wavinfo.needtosleep) wavinfo.wait_lk.wait(locker);
	Pa_CloseStream(stream);
	delete[] buffer;
	return true;
};

bool SNDPlayNowaitD(const short *SND_file, unsigned int len) {
	WaveFileNoW *wavinfo = new WaveFileNoW;
	wavinfo->currpos = wavinfo->buffer = new short[len];
	memcpy(wavinfo->buffer, SND_file, len << 1);
	wavinfo->endpos = wavinfo->buffer + len;
	wavinfo->stream = new PaStream*;
	if (Pa_OpenDefaultStream(wavinfo->stream, 0, 1, paInt16, SAMPLE_RATE, paFramesPerBufferUnspecified, PlayCallback, wavinfo)) {
		delete[] wavinfo->buffer;
		delete wavinfo->stream;
		delete wavinfo;
		return false;
	}
	Pa_SetStreamFinishedCallback(*wavinfo->stream, PlayFinishedCallbackNoW);
	if (Pa_StartStream(*wavinfo->stream)) {
		delete[] wavinfo->buffer;
		delete wavinfo->stream;
		delete wavinfo;
		return false;
	}
	return true;
};

bool SNDPlayWaitD(const short *SND_file, unsigned int len) {
	WaveFileW wavinfo;
	short *buffer = new short[len];
	memcpy(buffer, SND_file, len << 1);
	wavinfo.currpos = buffer;
	wavinfo.endpos = buffer + len;
	PaStream *stream;
	if (Pa_OpenDefaultStream(&stream, 0, 1, paInt16, SAMPLE_RATE, paFramesPerBufferUnspecified, PlayCallback, &wavinfo)) {
		delete[] buffer;
		return false;
	}
	Pa_SetStreamFinishedCallback(stream, PlayFinishedCallbackW);
	if (Pa_StartStream(stream)) {
		delete[] buffer;
		return false;
	}
	std::unique_lock<std::mutex> locker(wavinfo.lock);
	while (wavinfo.needtosleep) wavinfo.wait_lk.wait(locker);
	Pa_CloseStream(stream);
	delete[] buffer;
	return true;
};
