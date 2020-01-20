#include "stdafx.h"
#include "wavutils.h"

#include "consts.h"

namespace WavUtils
{
	static const float sample_rate_div2 = SAMPLE_RATE / 2.0F;

	void GenPeriodicalSine(short* buffer, unsigned short lenght, float tone, short volume) {
		unsigned short period = (unsigned short)roundf(sample_rate_div2 / tone); //44100 / 2
		unsigned short i = 0;
		short *buff = buffer, *buff_e = buffer + lenght; //segment = round(44100 / 2 / tone)*2

		for (; buff != buff_e; ++buff, ++i)
			*buff = (short)(volume * sin(i * M_PI / period));
	}
	unsigned short GetPeriodicalSineBuffLen(float tone) {
		return (unsigned short)(round(sample_rate_div2 / tone)) << 1;
	}
	Freqs GetPeriodicalSineFreqs(unsigned short count, unsigned short offset) {
		Freqs f;
		f.count = count;
		f.freqs = new float[count];
		float *fc = f.freqs, *fe = f.freqs + count;
		for (unsigned short i = 1 + offset; fc != fe; ++fc, ++i)
			*fc = sample_rate_div2 / i;
		return f;
	}

	inline unsigned int NOD(unsigned int  x, unsigned int  y) {
		if (x == 0) return y;
		else if (y == 0) return x;
		if (x >= y) return NOD(x%y, y);
		else return NOD(x, y%x);
	}
	Freqs GetSegmetSineFreqs(float min_freq, float max_freq, char accuracy, unsigned short buffmultiplicity) {
		unsigned short acc = (unsigned short)powf(10, accuracy);
		unsigned int max_f = (unsigned int)(max_freq * acc + 1), min_f = (unsigned int)(min_freq * acc);
		float *freqa = new float[max_f - min_f], *wp = freqa;
		Freqs f;
		unsigned int bufflen, prew = 0;
		for (unsigned int i = min_f; i != max_f; ++i)
		{
			bufflen = (unsigned int)roundf(sample_rate_div2 / (i / (float)acc) * acc);
			if (bufflen / acc / NOD(bufflen, acc) <= buffmultiplicity && bufflen != prew) {
				*wp = i / (float)acc; ++wp; prew = bufflen;
			}
		}
		f.count = (unsigned short)(wp - freqa);
		f.freqs = new float[f.count];
		memcpy(f.freqs, freqa, f.count * sizeof(float));
		delete[] freqa;
		return f;
	}
	SegmentSineProps GetSegmetSineBuffLen(float tone, char accuracy) {
		unsigned short acc = (unsigned short)powf(10, accuracy);
		unsigned int expper = (unsigned int)roundf(sample_rate_div2 / tone * acc);
		SegmentSineProps sqp;
		sqp.bufflen = (expper << 1) / NOD(expper, acc);
		sqp.period = expper / (float)acc;
		return sqp;
	}
	void GenSegmentSine(short* buffer, const SegmentSineProps &props, short volume) {
		unsigned int i = 0;
		short *buff = buffer, *buff_e = buffer + props.bufflen; 
		for (; buff != buff_e; ++buff, ++i)
			*buff = (short)(volume * sin(i * M_PI / props.period));
	}

	bool SaveSND(const char *filepath, const short* buffer, unsigned int bufflen) {
		std::ofstream str(filepath, std::fstream::out | std::fstream::binary);
		if (str.fail()) return false;
		str.write((char*)&bufflen, sizeof(int));
		str.write((char*)buffer, bufflen * sizeof(short));
		str.close();
		return true;
	}
	short *LoadSND(const char *filepath, unsigned int &bufflen) {
		std::ifstream str(filepath, std::fstream::in | std::fstream::binary);
		if (str.fail()) return nullptr;
		str.read((char*)&bufflen, sizeof(int));
		short *buffer = new short[bufflen];
		str.read((char*)buffer, bufflen * sizeof(short));
		str.close();
		return buffer;
	}
	short *LoadWAVFromBuffer(const char *buffer, unsigned int &outbufflen, WaveSmallInfo *info) {
		WAV_HEADER wh;
		memcpy(&wh, buffer, 44);
		outbufflen = (wh.chunkSize - 36) >> 1;
		short *wavedata = new short[outbufflen];
		buffer += 44;
		memcpy(wavedata, buffer, outbufflen << 1);
		if (info) {
			info->channels = wh.numChannels;
			info->sampleRate = wh.sampleRate;
		}
		return wavedata;
	}

	bool GenerateSinesByFreqFile(const char* freqfilename, char freqaccuracy, SynGenData &data) {
		std::ifstream str(freqfilename, std::fstream::in);
		if (str.fail()) return false;
		std::string cis; std::getline(str, cis);
		data.sinesnum = (unsigned short)std::stoi(cis);
		WavUtils::SegmentSineProps *segprops = new WavUtils::SegmentSineProps[data.sinesnum], *csgpos = segprops, *csepos = segprops + data.sinesnum;
		data.sinessizes = new unsigned int[data.sinesnum]; unsigned int *sinessc = data.sinessizes, fullbuffsize = 0;
		for (; csgpos != csepos; ++csgpos, ++sinessc) {
			std::getline(str, cis); 
			*csgpos = WavUtils::GetSegmetSineBuffLen(std::stof(cis), freqaccuracy);
			fullbuffsize += csgpos->bufflen;
			*sinessc = csgpos->bufflen;
		}
		str.close();
		short *sinesbuff = new short[fullbuffsize]; csgpos = segprops;
		data.sines = new short*[data.sinesnum]; short **sinespartc = data.sines;
		for (; csgpos != csepos; ++csgpos, ++sinespartc) {
			*sinespartc = sinesbuff;
			WavUtils::GenSegmentSine(sinesbuff, *csgpos);
			sinesbuff += csgpos->bufflen;
		}
		delete[] segprops;
		return true;
	}
	bool SaveSNDPacket(const char* collectorFilename, const char* SNDfileFolder, const SynGenData &data) {
		std::ofstream cll(collectorFilename, std::fstream::out);
		if (cll.fail()) return false;
		std::string sndfolder(SNDfileFolder); sndfolder += "\\aug_";
		unsigned int *cssize = data.sinessizes;
		short **currsine = data.sines;
		cll << data.sinesnum << std::endl;
		for (unsigned short i = 0; i != data.sinesnum; ++i, ++cssize, ++currsine) {
			std::ofstream sndfile(sndfolder + std::to_string(i) + ".snd", std::fstream::out | std::fstream::binary);
			if (sndfile.fail()) return false;
			cll << "aug_" << i << ".snd " << *cssize << std::endl;
			sndfile.write((char*)cssize, sizeof(int));
			sndfile.write((char*)*currsine, *cssize * sizeof(short));
			sndfile.close();
		}
		cll.close();
		return true;
	}

	bool LoadSNDPacket(const char* collectorFilename, const char* SNDfileFolder, SynGenData &data) {
		std::ifstream cll(collectorFilename, std::fstream::in);
		if (cll.fail()) return false;
		std::string cis; std::getline(cll, cis); data.sinesnum = std::stoi(cis);
		data.sinessizes = new unsigned int[data.sinesnum]; unsigned int *ssizespos = data.sinessizes, *ssizesep = data.sinessizes + data.sinesnum; 
		std::string *cllines = new std::string[data.sinesnum], *cllinc = cllines;
		unsigned int fullbuffsz = 0;
		unsigned short pos;
		for (; ssizespos != ssizesep; ++ssizespos, ++cllinc) {
			std::getline(cll, cis);
			pos = (unsigned short)cis.find_first_of(' ');
			*cllinc = cis.substr(0, pos);
			*ssizespos = (unsigned int)std::stoi(cis.substr(pos+1));
			fullbuffsz += *ssizespos;
		}
		cll.close();
		short *fullbuff = new short[fullbuffsz]; ssizespos = data.sinessizes; cllinc = cllines;
		data.sines = new short*[data.sinesnum]; short **sinespartc = data.sines;
		unsigned int tmp;
		for (; ssizespos != ssizesep; ++ssizespos, ++cllinc, ++sinespartc) {
			*sinespartc = fullbuff;
			std::ifstream sndfile(std::string(SNDfileFolder) + "\\" + *cllinc, std::fstream::in | std::fstream::binary);
			if (sndfile.fail()) { delete[] cllines; delete[] fullbuff; delete[] data.sinessizes; delete[] data.sines; return false; }
			sndfile.read((char*)&tmp, sizeof(int));
			sndfile.read((char*)fullbuff, *ssizespos * sizeof(short));
			sndfile.close();
			fullbuff += *ssizespos;
			
		}
		delete[] cllines;
		return true;
	}
}


