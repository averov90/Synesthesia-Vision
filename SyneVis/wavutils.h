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
		//Contains the characters "RIFF" in ASCII
		char chunkId[4];

		// 36 + subchunk2Size, or more precisely:
		// 4 + (8 + subchunk1Size) + (8 + subchunk2Size)
		// This is the remaining size of the chain starting from this position.
		// In other words, this is the file size-8, that is, the chunkId and chunkSize fields are excluded.
		unsigned __int32 chunkSize;

		// Contains characters
		char format[4];

		// The "WAVE" format consists of two subheadings: "fmt" and " data":
		// The "fmt" tag describes the audio data format:

		// Contains the characters "fmt "
		char subchunk1Id[4];

		// 16 for the PCM format.
		// This is the remaining size under the chain starting from this position.
		unsigned __int32 subchunk1Size;

		// For PCM = 1 (that is, Linear quantization).
		// Values other than 1 indicate some compression format.
		unsigned __int16 audioFormat;

		// Number of channels. Mono = 1, Stereo = 2, etc.
		unsigned __int16 numChannels;

		// Sampling frequency. 8000 Hz, 44100 Hz, etc.
		unsigned __int32 sampleRate;

		// sampleRate * numChannels * bitsPerSample/8
		unsigned __int32 byteRate;

		// numChannels * bitsPerSample/8
		// The number of bytes for a single sample, including all channels.
		unsigned __int16 blockAlign;

		// So-called "depth" or accuracy of sound. 8 bit, 16 bit, etc.
		unsigned __int16 bitsPerSample;

		// The "data" tag contains audio data and its size.

		// Contains the characters "data"
		char subchunk2Id[4];

		// numSamples * numChannels * bitsPerSample/8
		// The number of bytes in the data area.
		unsigned __int32 subchunk2Size;

		// This is followed directly by Wav data.
	};
}