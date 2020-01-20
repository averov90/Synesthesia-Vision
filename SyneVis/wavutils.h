#pragma once

namespace WavUtils {
	struct SegmentSineProps
	{
		unsigned int bufflen;
		float period;
	};
	struct WaveSmallInfo
	{
		unsigned short channels;
		unsigned short sampleRate;
	};
	struct Freqs {
		float *freqs = nullptr;
		unsigned short count = 0;
		Freqs(const Freqs &c):freqs(c.freqs), count(c.count) {
			copyed = true;
		}
		Freqs() {}
		float &operator [](const unsigned short index) {
			return freqs[index];
		}
		~Freqs() {
			if (copyed) 
				delete[] freqs;
		}
	private:
		bool copyed = false;
	};
	struct SynGenData
	{
		short **sines;
		unsigned int *sinessizes;
		unsigned short sinesnum;
	};
	void GenPeriodicalSine(short* buffer, unsigned short lenght, float tone, short volume = 32767);
	Freqs GetPeriodicalSineFreqs(unsigned short count, unsigned short offset = 0);
	unsigned short GetPeriodicalSineBuffLen(float tone);

	/*@accuracy
	Value range: 0-4
	*/
	SegmentSineProps GetSegmetSineBuffLen(float tone, char accuracy);
	/*@accuracy
	Value range: 0-4
	*/
	Freqs GetSegmetSineFreqs(float min_freq, float max_freq, char accuracy, unsigned short buffmultiplicity);
	void GenSegmentSine(short* buffer, const SegmentSineProps &props, short volume = 32767);

	bool SaveSND(const char *filepath, const short* buffer, unsigned int bufflen);
	short *LoadSND(const char *filepath, unsigned int &bufflen);
	short *LoadWAVFromBuffer(const char *buffer, unsigned int &outbufflen, WaveSmallInfo *info = nullptr);

	/*@accuracy
	Value range: 0-4
	*/
	bool GenerateSinesByFreqFile(const char* freqfilename, char freqaccuracy, SynGenData &data);
	bool SaveSNDPacket(const char* collectorFilename, const char* SNDfileFolder, const SynGenData &data);
	bool LoadSNDPacket(const char* collectorFilename, const char* SNDfileFolder, SynGenData &data);

	struct WAV_HEADER
	{
		//Содержит символы "RIFF" в ASCII кодировке
		//(0x52494646 в big-endian представлении)
		char chunkId[4];

		// 36 + subchunk2Size, или более точно:
		// 4 + (8 + subchunk1Size) + (8 + subchunk2Size)
		// Это оставшийся размер цепочки, начиная с этой позиции.
		// Иначе говоря, это размер файла - 8, то есть,
		// исключены поля chunkId и chunkSize.
		unsigned __int32 chunkSize;

		// Содержит символы "WAVE"
		// (0x57415645 в big-endian представлении)
		char format[4];

		// Формат "WAVE" состоит из двух подцепочек: "fmt " и "data":
		// Подцепочка "fmt " описывает формат звуковых данных:

		// Содержит символы "fmt "
		// (0x666d7420 в big-endian представлении)
		char subchunk1Id[4];

		// 16 для формата PCM.
		// Это оставшийся размер подцепочки, начиная с этой позиции.
		unsigned __int32 subchunk1Size;

		// Для PCM = 1 (то есть, Линейное квантование).
		// Значения, отличающиеся от 1, обозначают некоторый формат сжатия.
		unsigned __int16 audioFormat;

		// Количество каналов. Моно = 1, Стерео = 2 и т.д.
		unsigned __int16 numChannels;

		// Частота дискретизации. 8000 Гц, 44100 Гц и т.д.
		unsigned __int32 sampleRate;

		// sampleRate * numChannels * bitsPerSample/8
		unsigned __int32 byteRate;

		// numChannels * bitsPerSample/8
		// Количество байт для одного сэмпла, включая все каналы.
		unsigned __int16 blockAlign;

		// Так называемая "глубиная" или точность звучания. 8 бит, 16 бит и т.д.
		unsigned __int16 bitsPerSample;

		// Подцепочка "data" содержит аудио-данные и их размер.

		// Содержит символы "data"
		// (0x64617461 в big-endian представлении)
		char subchunk2Id[4];

		// numSamples * numChannels * bitsPerSample/8
		// Количество байт в области данных.
		unsigned __int32 subchunk2Size;

		// Далее следуют непосредственно Wav данные.
	};
	/*
	ofstream str;
	str.open("test.wav", ofstream::out | fstream::binary);
	WavUtils::WAV_HEADER wh;

	memcpy(wh.chunkId, "RIFF", 4);
	wh.chunkSize=12036;
	memcpy(wh.format, "WAVE", 4);
	memcpy(wh.subchunk1Id, "fmt ", 4);
	wh.subchunk1Size=16;
	wh.audioFormat=1;
	wh.numChannels=1;
	wh.sampleRate= WavUtils::SAMPLE_RATE;
	wh.byteRate= WavUtils::SAMPLE_RATE*2;
	wh.blockAlign=2;
	wh.bitsPerSample=16;
	memcpy(wh.subchunk2Id, "data", 4);
	wh.subchunk2Size=24000;
	str.write((char*)&wh, sizeof(WavUtils::WAV_HEADER));

	//float *af = a, *bf = b, *ae = a+50, *be=b+19;
	for (unsigned int i1 = 0; i1 != 1000; ++i1)

		for (unsigned int i = 0; i != 12; ++i)
		{
			str.write((char*)(a+i), 2);
			//cout << a[i] << endl;
		}
	str.close();
	cout << "end!" << endl;
	*/
}