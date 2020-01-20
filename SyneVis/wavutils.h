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
		//�������� ������� "RIFF" � ASCII ���������
		//(0x52494646 � big-endian �������������)
		char chunkId[4];

		// 36 + subchunk2Size, ��� ����� �����:
		// 4 + (8 + subchunk1Size) + (8 + subchunk2Size)
		// ��� ���������� ������ �������, ������� � ���� �������.
		// ����� ������, ��� ������ ����� - 8, �� ����,
		// ��������� ���� chunkId � chunkSize.
		unsigned __int32 chunkSize;

		// �������� ������� "WAVE"
		// (0x57415645 � big-endian �������������)
		char format[4];

		// ������ "WAVE" ������� �� ���� ����������: "fmt " � "data":
		// ���������� "fmt " ��������� ������ �������� ������:

		// �������� ������� "fmt "
		// (0x666d7420 � big-endian �������������)
		char subchunk1Id[4];

		// 16 ��� ������� PCM.
		// ��� ���������� ������ ����������, ������� � ���� �������.
		unsigned __int32 subchunk1Size;

		// ��� PCM = 1 (�� ����, �������� �����������).
		// ��������, ������������ �� 1, ���������� ��������� ������ ������.
		unsigned __int16 audioFormat;

		// ���������� �������. ���� = 1, ������ = 2 � �.�.
		unsigned __int16 numChannels;

		// ������� �������������. 8000 ��, 44100 �� � �.�.
		unsigned __int32 sampleRate;

		// sampleRate * numChannels * bitsPerSample/8
		unsigned __int32 byteRate;

		// numChannels * bitsPerSample/8
		// ���������� ���� ��� ������ ������, ������� ��� ������.
		unsigned __int16 blockAlign;

		// ��� ���������� "��������" ��� �������� ��������. 8 ���, 16 ��� � �.�.
		unsigned __int16 bitsPerSample;

		// ���������� "data" �������� �����-������ � �� ������.

		// �������� ������� "data"
		// (0x64617461 � big-endian �������������)
		char subchunk2Id[4];

		// numSamples * numChannels * bitsPerSample/8
		// ���������� ���� � ������� ������.
		unsigned __int32 subchunk2Size;

		// ����� ������� ��������������� Wav ������.
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