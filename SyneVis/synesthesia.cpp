#include "stdafx.h"
#include "synesthesia.h"

#include "wavutils.h"
#include "consts.h"

using namespace cv;
using namespace std;

static Mat srv_input[5], *qput = srv_input, *forgray = srv_input + 1, *forhsv = srv_input + 2, *foresmat = srv_input + 3, *qpop = srv_input + 4;

static bool calc_need_wait = true, sum_mode_brightness = true;
bool syne_brightness_mode = true;

static mutex var_change, pasunccalc;
static condition_variable syne_prep_th;

VideoCapture camera_capture_syne;

static WavUtils::SynGenData SynesthesyData;

static short *scurrpos[SYNE_FRAME_HEIGHT], *sendpos[SYNE_FRAME_HEIGHT];

static short colnum;

static float *prew_coef, *prew_coef_end, syne_exp[256], syne_invh[180];

static char syne_sign_offset, syne_sign_delim;

static Mat column;

static unsigned short brightness_divider;

static int SynesthesyGenerator(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
	short *out = (short*)outputBuffer;
	short **csines, **esines = SynesthesyData.sines + SynesthesyData.sinesnum, **currpos, **endpos;
	float *curr_prew, tmp;
	int summ, div_res;
	var_change.lock();
	if (colnum >= 0) {
		column = qpop->col(colnum);
		MatIterator_<float> start = column.begin<float>(), end, icurrpos;
		if (sum_mode_brightness) {
			for (unsigned short i = 0; i != framesPerBuffer; ++i, ++out) {
				summ = 0;
				for (csines = SynesthesyData.sines, currpos = scurrpos, endpos = sendpos, icurrpos = start, curr_prew = prew_coef; csines != esines; ++csines, ++currpos, ++endpos, ++icurrpos, ++curr_prew) {
					if (std::abs(*curr_prew - *icurrpos) > SYNE_FRAME_VIS_DELTA) {
						if (*curr_prew < *icurrpos) {
							tmp = *curr_prew += SYNE_FRAME_VIS_DELTA;
							summ += (short)(**currpos * tmp);
						} else {
							tmp = *curr_prew -= SYNE_FRAME_VIS_DELTA;
							summ += (short)(**currpos * tmp);
						}
					} else {
						summ += (short)(**currpos * *icurrpos);
						*curr_prew = *icurrpos;
					}
					++*currpos;
					if (*currpos == *endpos) *currpos = *csines;
				}
				div_res = summ / brightness_divider;
				if (div_res < 0) *out = (short)(div_res > -32767 ? div_res : -32767);
				else *out = (short)(div_res < 32767 ? div_res : 32767);
			}
		} else {
			for (unsigned short i = 0; i != framesPerBuffer; ++i, ++out) {
				summ = 0;
				for (csines = SynesthesyData.sines, currpos = scurrpos, endpos = sendpos, icurrpos = start, curr_prew = prew_coef; csines != esines; ++csines, ++currpos, ++endpos, ++icurrpos, ++curr_prew) {
					if (std::abs(*curr_prew - *icurrpos) > SYNE_FRAME_VIS_DELTA) {
						if (*curr_prew < *icurrpos) {
							tmp = *curr_prew += SYNE_FRAME_VIS_DELTA;
							summ += (short)(**currpos * tmp);
						} else {
							tmp = *curr_prew -= SYNE_FRAME_VIS_DELTA;
							summ += (short)(**currpos * tmp);
						}
					} else {
						summ += (short)(**currpos * *icurrpos);
						*curr_prew = *icurrpos;
					}
					++*currpos;
					if (*currpos == *endpos) *currpos = *csines;
				}
				div_res = summ / SYNE_FRAME_HEIGHT;
				if (div_res < 0) *out = (short)(div_res > -32767 ? div_res : -32767);
				else *out = (short)(div_res < 32767 ? div_res : 32767);
			}
		}
		if (++(colnum) == SYNE_FRAME_WIDTH) {
			calc_need_wait = false;
			syne_prep_th.notify_one();
			colnum = -2;
		}
	}
	else {
		for (unsigned short i = 0; i != framesPerBuffer; ++i, ++out) {
			summ = 0;
			for (csines = SynesthesyData.sines + syne_sign_offset, currpos = scurrpos + syne_sign_offset, endpos = sendpos + syne_sign_offset, curr_prew = prew_coef + syne_sign_offset; csines != esines; csines += 12, currpos += 12, endpos += 12, curr_prew += 12) {
				if (1 - *curr_prew > SYNE_FRAME_VIS_DELTA) {
					if (*curr_prew < 1) {
						tmp = *curr_prew += SYNE_FRAME_VIS_DELTA;
						summ += (short)(**currpos * tmp);
					}
					else {
						tmp = *curr_prew -= SYNE_FRAME_VIS_DELTA;
						summ += (short)(**currpos * tmp);
					}
				}
				else {
					summ += (short)(**currpos);
					*curr_prew = 1;
				}
				++*currpos;
				if (*currpos == *endpos) *currpos = *csines;
			}
			*out = summ / syne_sign_delim;
		}
		++colnum;
	}
	var_change.unlock();
	return paContinue;
}


static void pa_asunc_calc() {
	MatIterator_<float> curr, end;
	Mat tmp[3];
	while (true)
	{
		unique_lock<mutex> locker(pasunccalc);
		while (calc_need_wait) syne_prep_th.wait(locker);
		var_change.lock();
		camera_capture_syne >> *qput;
		if (syne_brightness_mode) {
			cvtColor(*qput, *forgray, COLOR_BGR2GRAY);
			resize(*forgray, *foresmat, cv::Size(SYNE_FRAME_WIDTH, SYNE_FRAME_HEIGHT), 0, 0, InterpolationFlags::INTER_NEAREST);
			foresmat->convertTo(*qpop, CV_32FC1);
			for (curr = qpop->begin<float>(), end = qpop->end<float>(); curr != end; ++curr)
				*curr = syne_exp[(unsigned char)*curr];
			sum_mode_brightness = true;
		} else {
			cvtColor(*qput, *forhsv, COLOR_BGR2HSV);
			split(*forhsv, tmp);
			resize(*tmp, *foresmat, cv::Size(SYNE_FRAME_WIDTH, SYNE_FRAME_HEIGHT), 0, 0, InterpolationFlags::INTER_NEAREST);
			foresmat->convertTo(*qpop, CV_32FC1);
			for (curr = qpop->begin<float>(), end = qpop->end<float>(); curr != end; ++curr)
				*curr = syne_invh[(unsigned char)*curr];
			sum_mode_brightness = false;
		}
		calc_need_wait = true;
		var_change.unlock();
	}
}

static PaStream *syne_stream;

char Syne_Initialise() {
	if (!WavUtils::LoadSNDPacket("synesthesia\\collect.txt", "synesthesia\\sndfolder", SynesthesyData)) return 1;
	if (SYNE_FRAME_HEIGHT != SynesthesyData.sinesnum) return 2;
	unsigned int *sinesp = SynesthesyData.sinessizes, *sinespe = SynesthesyData.sinessizes + SYNE_FRAME_HEIGHT;
	short **sinesbp = SynesthesyData.sines, **currpp = scurrpos, **endpp = sendpos;
	for (; sinesp != sinespe; ++sinesp, ++currpp, ++endpp, ++sinesbp) {
		*currpp = *sinesbp; *endpp = *sinesbp + *sinesp;
	}
	colnum = -2;
	*qpop = Mat::zeros(SYNE_FRAME_HEIGHT, SYNE_FRAME_WIDTH, CV_32FC1);
	prew_coef = new float[SYNE_FRAME_HEIGHT];
	float *cprewc = prew_coef; prew_coef_end = cprewc + SYNE_FRAME_HEIGHT;
	for (; cprewc != prew_coef_end; ++cprewc)
		*cprewc = 0;
	for (unsigned short i = 0; i != 256; ++i) {
		syne_exp[i] = (i * i) / 65025.0F;
	}
	for (unsigned char i = 0; i != 180; ++i) {
		syne_invh[i] = (179 - i) / 179.0F;
	}
	syne_sign_offset = SYNE_FRAME_HEIGHT % 12;
	syne_sign_delim = (SYNE_FRAME_HEIGHT - syne_sign_offset) / 3;
	brightness_divider = (unsigned short)(SYNE_FRAME_HEIGHT / 1.5);

	Pa_OpenDefaultStream(&syne_stream, 0, 1, paInt16, SAMPLE_RATE, 167, SynesthesyGenerator, nullptr);

	thread paAsuncCalc(pa_asunc_calc); //Starting paCallback asunc calc thread
	paAsuncCalc.detach();

	return 0;
};

void Syne_Start() { 
	Pa_StartStream(syne_stream); 
}

void Syne_Stop() { 
	Pa_StopStream(syne_stream); 
}
