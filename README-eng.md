# Synesthesia Vision
[![License](https://img.shields.io/badge/LICENSE-MIT-green?style=flat-square)](/LICENSE)  [![Version](https://img.shields.io/badge/VERSION-DEBUG%20--%20demonstrable-yellow?style=flat-square)](https://github.com/averov90/Synesthesia-Vision/releases)
### :small_orange_diamond: [Русская версия](/README.md)

###### If you don't know what synesthesia is, follow the [link](https://en.wikipedia.org/wiki/Synesthesia#Chromesthesia).

This program converts the image coming from the camera into sound, or rather, one of the image channels turns into sound: color or brightness.

## Synesthesia
The program takes a picture, then changes its size to 67x50. A picture of this size is voiced in columns by changing the intensity of a frequency in the output sound.

The algorithm is quite simple:
1. Reduce the image to 67x50
2. Take one column from the image (1x50)
3. For each pixel in this column, map the frequency (for example, 8000 Hz for 1 pixel, 16000 Hz for 2 pixels, and so on).)
4. Multiply the sinusoid of the corresponding frequency by a factor (from 0 to 1, pixel brightness 0-coefficient = 0, brightness 255-coefficient = 1)
5. Add the sinusoids multiplied by the coefficient.

With the help of synesthesia, you can learn to " see " with your ears because the image begins to sound. Even an untrained person will be able to distinguish by ear (blindfolded) where the light is, what shape it is, what brightness it is. If you train more, you can learn to " see " a b/w picture through your ears.
*No more than a 3-dimensional vector can be transmitted through 1 synesthesia channel: X, Y, B-pixel position by X, Y, and some value in the pixel. The sweep, as you might guess, is the following: Y is represented by the frequency, B is the intensity of the frequency when mixing, and X is represented by the playing time of the column (all columns are played in turn cyclically from left to right).*

#### Files
- [synesthesia.h](https://github.com/averov90/Synesthesia-Vision/blob/master/SyneVis/synesthesia.h)
- [synesthesia.cpp](https://github.com/averov90/Synesthesia-Vision/blob/master/SyneVis/synesthesia.cpp)

The synesthesia block requires the presence of the **wavutils**block. Other project blocks are not required for this block. 
The synesthesia function was written for the **portaudio** library. The block also uses classes from **opencv**, but it is easy to replace them with analogues.

## WavUtils
This block contains functions for working with audio files, in particular - *wave*. This contains a class for getting a wave from *.wav* and its further use. 
Also, this block introduces a special audio storage format - **SND**, as well as means for single and batch generation of frequency files (which are used in the synesthesia block).

#### Files
- [wavutils.h](https://github.com/averov90/Synesthesia-Vision/blob/master/SyneVis/wavutils.h)
- [wavutils.cpp](https://github.com/averov90/Synesthesia-Vision/blob/master/SyneVis/wavutils.cpp)

The name of the namespace - *WavUtils*.

**NOTE**: *frequency* in this context refers to an array of absolute amplitude values.

#### Structures:
- **WAV_HEADER** - the *wav*file header template contains its parameters (sample rate, channels, etc.).
- **WaveSmallInfo** - the structure returned in addition (optionally) by the Load WAV From Buffer function. This structure contains information about the number of channels and the sampling rate .WAV-file.
- **Freqs** - the structure returned by the *GetPeriodicalSineFreqs* and *GetSegmetSineFreqs*functions. Contains an array of frequencies in the specified range that can be generated at the specified parameters using the *GenPeriodicalSine* and *GenSegmentSine* functions respectively. This structure supports operator[] and has an automatic deletion function.
- **SegmentSineProps** - the structure required for generating the frequency by the *GetSegmetSineFreqs * function. Contains the size of the frequency buffer and the period to generate. From this structure, you will need to take the buffer size value.
- **SynGenData** - the structure containing many frequencies. It is convenient to use it when you need to upload many files at a time (for example, 50 frequency files).

#### Functions:
- **GetPeriodicalSineFreqs** - allows you to get a list of frequencies supported in the "periodic" mode, i.e. frequencies that are multiples of the sample rate. The "periodic" mode is a special case of the "segmented" mode. The *count * argument is the number of frequencies you want to get, and *offset* is the offset in the array of supported frequencies. For example, to get frequencies from the 4th to the 9th, you need *count*=5, *offset*=4.
- **GetPeriodicalSineBuffLen** - gets the short buffer size required for frequency storage. The argument specifies the frequency from the list of supported frequencies.
- **GenPeriodicalSine** - generates a frequency. *buffer* - pointer to the buffer of the desired size, *length* - buffer size, *tone* - frequency (1/T, where T is the period), *volume* the maximum absolute value in the array of absolute amplitude values.
- **GetSegmetSineFreqs** - allows you to get a list of frequencies for the *GenSegmentSine* function. *min_freq* - initial frequency (800 Hz, for example), *max_freq* - final frequency (1600 Hz, for example), *accuracy* - degree of rounding during generation (0-generation equivalent to "periodic", 4-generation with the maximum possible number of frequencies for *GetSegmetSine*). If easier - *accuracy* is responsible for the number of frequencies between *min_freq* and *max_freq*. * buffmultiplicity* - the maximum number of repetitions of the period. For "periodic", 1 is always sufficient because "periodic" is always a frequency divider without a remainder, which means that the array of its values for correct playback can contain only one maximum and one minimum value (one period). For any frequency other than "periodic", one repetition will not be enough. As you can guess, *buffmultiplicity* for a specific frequency, you need such that the frequency period *buffmultiplicity* is a divisor of the descretization frequency. However, you should not bother with the exact moment of calculation because for a particular frequency, the buffer may be too large. It is better to choose the maximum allowed number of repetitions, so as not to exceed a single buffer of gigabytes :)
- **GetSegmetSineBuffLen** - gets the structure *SegmentSineProps*, *accuracy* must be specified for calculating the period, otherwise the specified frequency would be rounded to "periodic" - corresponding.
- **GenSegmentSine** - generates a frequency using *SegmentSineProps*. 
- **LoadWAVFromBuffer** - gets the frequency from .wav of a certain type (not float-point, for example). *buffer* - pointer to the buffer containing the wave file. *outbuflen* - returns the size of the created frequency storage buffer. Additionally, you can pass a pointer to *WaveSmallInfo* if necessary. The function returns the frequency (an array of absolute amplitude values).
- **SaveSND** - saves the frequency in .smd file (smaller than .wav). *file path* - path to save the file, *buffer* - frequency, *buflen* - length of frequency (array of absolute amplitude values). Returns *true* if successful.
- **LoadSND** - loads a frequency from a file. Returns the frequency, and through the argument *buff len* - the size of the array.
- **GenerateSinesByFreqFile** - generates a set of frequencies (for example, 50) using a generation file (text, ASCII). The first line is the number of frequencies, and the other lines are frequencies (F = 1/T).
<details>
<summary>Example of the contents of a frequency file</summary>
  
```
3  
879.89  
659  
155.7 
```
</details>

- **SaveSNDPacket** - saves multiple frequencies to the *SNDfileFolder * folder, and also saves a text file listing the frequency files in the *SNDfileFolder* folder. 
<details>
<summary>Example of the contents of a file listing files with frequencies</summary>
  
```
3
aug_0.snd 118
aug_1.snd 626
aug_2.snd 662
```
aug_0.snd 118 - "aug_0.snd" - name of file, 118 - the size of the file data in short. The size in bytes here is equal to 118 * 2 + 4 (4 - size of uint at the beginning).
</details>

- **LoadSNDPacket** - loads many files.

## WavPlayer
Allows you to play a wave file synchronously or asynchronously. Calling from multiple threads and starting asynchronous playback of multiple files works correctly. This block requires the *WavUtils* block to work.

#### Files
- [wavplayer.h](https://github.com/averov90/Synesthesia-Vision/blob/master/SyneVis/wavplayer.h)
- [wavplayer.cpp](https://github.com/averov90/Synesthesia-Vision/blob/master/SyneVis/wavplayer.cpp)

#### Functions:
- **WavePlayWaitD** - starts synchronous playback (waiting for playback to finish) of a wav file located in the *WAV_file* buffer.
- **WavePlayNowaitD** - starts asynchronous (does not wait for playback to finish) playback of the wav file.
- **SNDPlayWaitD** - starts synchronous playback of the snd file.
- **SNDPlayNowaitD** - starts asynchronous playback of the snd file.
<details>
<summary>What does "D" mean at the end of the function name?</summary>
The "D" at the end of the function name means that this function uses the preset parameters of the wav file when playing (sampleRate, bitDepth). This is useful when playing wav files of only a certain format.
</details>

## CloudAPI
Allows you to make requests to the *Google-Vision-API* and to the *Google-TTS-API*.

#### Functions:
- **Cloud_TTS** - a function that uses the *Google-TTS-API*. The input is given a string for voice-over. If successful, the function returns *true* and replaces the contents of the *text* argument with the bytes of the wave file that came from the cloud.
- **Cloud_ImageDescriptor** - a function that uses the * Google-Vision-API*. The input is an image (argument *image*) in which you want to find objects. The second argument contains a string in which 1-10 tags describing the image (for example, ```sofa, armchair, cotton```) will be placed in case of successful execution (the value *true* is returned). Tags are arranged in descending order of their "relevance". The function returns a string like ```Found: tag1, tag2, tag3```. If necessary, the word "Found" can be removed in the source code of the function.
This function also uses the function to create a jpeg image in ram using the function from **libjpeg-turbo** instead of the non-optimal *imdecode* from **opencv**.

*More features of the Google-TTS-API and Google-Vision-API are presented in the project, which is referenced below.*

### *There is a project created based on this: [Raspberry Pi - SyneVis_Proj](https://github.com/averov90/RaspberryPi-SyneVis_Proj).*
