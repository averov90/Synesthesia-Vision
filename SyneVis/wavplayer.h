#pragma once

bool WavePlayNowaitD(const char *WAV_file);
bool WavePlayWaitD(const char *WAV_file);

bool SNDPlayNowaitD(const short *SND_file, unsigned int len);
bool SNDPlayWaitD(const short *SND_file, unsigned int len);