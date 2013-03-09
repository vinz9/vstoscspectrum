#ifndef AUDIOINPUT_H_
#define AUDIOINPUT_H_

#define M_PI 3.14159265358979323846

#include <windows.h>

#include "fftw3.h"
#include "audioeffect.h"
//#include "VstPlugin.h"

typedef struct {
    float cutoff;
    float a0;
    float b1;
    float b2;
    float x1;
    float x2;
} NOTCH_FILTER;

//AmpScale FreqScale
enum {
	kNoScaling,
	kLog,
};

//Resampling
enum {
	kDecim,
	kSum
};


class AudioInput
{
	public:

	AudioInput();
	AudioInput(float sRate, int bufferSize, int iNBands, float fResponse,
		int iAmpScale, int iFreqScale, int iResampling, float fXScale, float fYScale, AudioEffect* effect);
	//AudioInput(float sRate, int bufferSize, int iNBands, float fResponse, int iAmpScale);
	~AudioInput();

	AudioEffect* effect;

	const int bufferSize;

	float sampleRate;

	float* buffer;

	bool lock;

	int cursor;

	float level;
	float* audiobars;
	float* temp_bars;
	float* magnitude;
	float halfLife;

	//C3 : 12
	//C4 : 24
	int maxFreq;
	float maxMagni;
	int getNote();

	int nof_bands;

	void peak();

	
	void initFilters(int nbands);
	void processFilters();

	bool hasBeenTriggered();
	bool isBeingTriggered();
	void updateTrigger();

	//private:

	bool activated;
	bool activation;
	NOTCH_FILTER * bp_filter;

	void fftw();

	float processNotch(NOTCH_FILTER * l, float x0);
	NOTCH_FILTER *  initNotch(float cutoff);

	float gain;
	int nSamples;

	float* fftin;
	fftwf_complex *fftout;
	fftwf_plan plan;
	float* fftMag;

	double powerN;
	int nfftResult;
	int nBands;
	float* fftBands;
	float* fftBandsTemp;
	float curResponse;

	float xScale;
	float yScale;

	int curFftScaling;
	int curFreqScaling;
	int curResampling;

	void updateBands(int nb);
	void updateNSamples(int newBufferSize);
	void updateNotch(NOTCH_FILTER * l, float cutoff);
};

#endif