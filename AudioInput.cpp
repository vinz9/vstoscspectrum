#include <math.h>
#include <stdlib.h>
#include "AudioInput.h"
#include "VstPlugin.h"

//----------------------------------------------------------------------------
AudioInput::AudioInput(float sRate, int bufferSize, int iNBands, float fResponse,
					   int iAmpScale, int iFreqScale, int iResampling, float fXScale, float fYScale, AudioEffect* effect):
effect(effect),level(0.0f),sampleRate(sRate),bufferSize(bufferSize),nBands(iNBands),halfLife(fResponse),
curFftScaling(iAmpScale),curFreqScaling(iFreqScale),curResampling(iResampling),xScale(fXScale), yScale(fYScale),maxFreq(0),maxMagni(0)
//AudioInput::AudioInput(float sRate, int bufferSize, int iNBands, float fResponse, int iAmpScale):
//level(0.0f),sampleRate(sRate),bufferSize(bufferSize),nBands(iNBands),halfLife(fResponse),curFftScaling(iAmpScale),maxFreq(0),maxMagni(0)

{
	cursor = 0;
	
	buffer = new float[bufferSize];
	memset(buffer, 0, bufferSize*sizeof(float));
	magnitude = NULL;
	audiobars = NULL;
	temp_bars = NULL;

	
	gain = 1/32768.0;

	nSamples = bufferSize;

	nfftResult = (int)(nSamples*0.5+1);

	fftin = new float[nSamples];
	memset(fftin, 0, nSamples*sizeof(float));

	fftout = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * nSamples);
	plan = fftwf_plan_dft_r2c_1d(nSamples, fftin, fftout, FFTW_MEASURE);
	fftMag = new float[nfftResult];
	memset(fftMag, 0, nfftResult*sizeof(float));

	fftBands = new float[nBands];
	memset(fftBands, 0, nBands*sizeof(float));

	fftBandsTemp = new float[nBands];
	memset(fftBandsTemp, 0, nBands*sizeof(float));

	powerN = log10((double)nfftResult)/nBands;

	curResponse = 0.90f;

	lock = false;

	initFilters(nBands);

}

//----------------------------------------------------------------------------
AudioInput::AudioInput():level(0.0f),sampleRate(44100.0f),bufferSize(bufferSize),halfLife(0.01f),maxFreq(0),maxMagni(0)
{
	buffer = new float[bufferSize];
	magnitude = new float[bufferSize*0.5];
	audiobars = NULL;
	temp_bars = NULL;


}

//----------------------------------------------------------------------------
AudioInput::~AudioInput()
{
	if (buffer)
	delete[] buffer;

	if (magnitude != NULL)
		delete[] magnitude;

	if (audiobars != NULL)
		delete[] audiobars;

	if (temp_bars != NULL)
	{
		delete[] temp_bars;

		 /*for (int i=0; i<nof_bands; i++) 
		{
		
			delete bp_filter[i];
		}*/
		//free(bp_filter);
		delete[] bp_filter;
	}

	fftwf_destroy_plan(plan);
	fftwf_free(fftout);
	delete[] fftin;
	delete[] fftMag;

	delete[] fftBands;
	delete[] fftBandsTemp;


}
//---------------------------------------------------------------------------------------------

void AudioInput::updateNSamples(int newBufferSize){

	//int tempSamples = (int)(pow((double)2, 9+newNSamples));
	//if (tempSamples != nSamples) {
		//nSamples = tempSamples;

		nSamples = newBufferSize;

		delete[] buffer;

		fftwf_destroy_plan(plan);
		fftwf_free(fftout);
		delete[] fftin;
		delete[] fftMag;

		//delete[] fftBands;
		//delete[] fftBandsTemp;

		nfftResult = (int)(nSamples*0.5+1);
		//fftRawArray.setLength(nfftResult);

		fftin = new float[nSamples];
		fftout = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * nSamples);
		//p = fftwf_plan_dft_r2c_1d(nSamples, fftin, out, FFTW_ESTIMATE);
		plan = fftwf_plan_dft_r2c_1d(nSamples, fftin, fftout, FFTW_MEASURE);
		fftMag = new float[nfftResult];

		//fftBands = new float[nBands];
		//fftBandsTemp = new float[nBands];
		//powerN = log10((double)nfftResult)/nBands;

//		chunkSize = nSamples*nBytes*nChannels;

		//buffer = new short[nSamples*nChannels];
		buffer = new float[newBufferSize];

}

//----------------------------------------------------------------------------
void AudioInput::peak()
{
	float scalar = pow( 0.5f, 1.0f/(halfLife * sampleRate));
	float in = 0;

	for(int i = 0; i<bufferSize; i++)
	{	
		in = fabs(buffer[i]);
		if ( in >= level )
		{
			level = in;
		}
		else
		{
			level = level * scalar;
			if( level < 0.00001 ) level = 0.0;
		}
	}
}


//---------------------------------------------------------------------------
void AudioInput::fftw() {

	float scalar = pow(pow( 0.5f, 1.0f/(halfLife * sampleRate)), 2048.0f);

	double window, imag, real;

	lock = true;

	for (int k = 0; k < nSamples; k++) {
		window = -.5f*cos(2.*M_PI*k/nSamples)+.5f;
		
//		fftin[k] = (float)((float)buffer->interpolate(k,0)*gain * window);
		fftin[k] = (float)(buffer[k]  * window);
		
	}

	lock = false;

	fftwf_execute(plan);
	float temp;
	float max = 0;

	for (int k = 0; k < nfftResult; k++) {

		real = fftout[k][0];
		imag = fftout[k][1];

		//fftMag[k] = (float)sqrt(real*real + imag*imag);
		temp = (float)sqrt(real*real + imag*imag);
		if (temp >fftMag[k])
			fftMag[k]=temp;
		else
			fftMag[k]=fftMag[k]*scalar;
		if (fftMag[k] < 0.00001) fftMag[k] = 0;

		if (temp>max)
			max = temp;

	}

	//max 80 normal, 150 loop, 300 morceau compresse

	int freq = 0;
	int freqlast = 0;
	float result = 0;

	((VstPlugin*)effect)->lockBands = true;

	//maxsum dépasse pas max
	float maxsum = 0;

	for(int i=1; i<=nBands; i++)
	{

		switch(curResampling) {
			case kDecim: 
				if(curFreqScaling == kNoScaling) {
					freq = (int)(i*(float)nfftResult/(float)nBands)-1;
					result = fftMag[freq];

				} else if(curFreqScaling ==kLog) {
					double ftemp = (pow((double)10, i*powerN));
					freq = (int)ftemp;

					if(freq >= nfftResult){
						result = fftMag[nfftResult-1];
					} else {
						result = (float)((1-(ftemp-freq))*fftMag[freq-1]+(ftemp-freq)*fftMag[freq]);
					}
				}
				
				if(curFftScaling == kNoScaling) {
					//result = pow((result/1.0f),yScale) * pow((float)i/nBands,xScale);
					result = result*pow((float)i/nBands,xScale);
					result = result * 0.05;
					result = pow(result,yScale);

				} else if (curFftScaling == kLog) {
					if (result > 0.01) {
						result = (float)(log((double)result) + 4.6);
						result = result*pow((float)i/nBands,xScale);
						result = result * 0.15;
						result = pow(result,yScale);
					}
					else {
						result = 0;
					}

				}
				break;

			case kSum:
				freqlast = freq;

				if(curFreqScaling == kNoScaling) {
					freq = (int)(i*(float)nfftResult/(float)nBands)-1;

				} else if(curFreqScaling ==kLog) {
					double ftemp = (pow((double)10, i*powerN));
					freq = (int)ftemp;

				}

				float sum = 0.0f;
				float sumAvg = 0.0f;
				float logsum = 0.0f;
				
				if (freq >= nfftResult) freq = nfftResult - 1;

				if (freqlast == freq) freqlast = freq-1;
				int bsize = freq-freqlast;
				float bsizetemp = pow((float)bsize,xScale);

				for(;freqlast < freq; freqlast++){
					sum += fftMag[freqlast];
				}

				if (sum >maxsum)
					maxsum = sum;

				// sum entre 100 et 300

				if (curFftScaling == kNoScaling) {
					sum = (float)(sum/bsizetemp);
					result = sum*0.004f;
					result = pow(result,yScale);

				} else if (curFftScaling == kLog) {

					sumAvg = (float)sum/bsizetemp;

					// log(0.01) = 4.6
					if (sumAvg > 0.01)
						logsum = (float)(log((double)sumAvg) + 4.6);
					else
						logsum = 0;

					result = (float)(logsum*0.1);
					result = pow(result,yScale);

				}

				break;
		}


		//if ( result > fftBands[i-1])
			fftBands[i-1] = result;
		/*else{
				fftBands[i-1] = fftBands[i-1]*0.9;
				if( fftBands[i-1] < 0.00001 ) fftBands[i-1] = 0.0;
		}*/
	}
	((VstPlugin*)effect)->lockBands = false;

}

//----------------------------------------------------------------------------
NOTCH_FILTER *  AudioInput::initNotch(float cutoff)
{
    //NOTCH_FILTER * l=(NOTCH_FILTER*)malloc(sizeof(NOTCH_FILTER));

	NOTCH_FILTER * l= new NOTCH_FILTER;

    float steep = 0.999f;
    float r = steep * 0.99609375f;
    float f = (float)(cos(M_PI * cutoff / (sampleRate*0.5)));
    l->cutoff = cutoff;
    l->a0 = (1 - r) * sqrt( r * (r - 4 * (f * f) + 2) + 1);
    l->b1 = 2 * f * r;
    l->b2 = -(r * r);

    l->x1 = 0.0;
    l->x2 = 0.0;
    return l;
}

void AudioInput::updateNotch(NOTCH_FILTER * l, float cutoff) {

	float steep = 0.999f;
    float r = steep * 0.99609375f;
    float f = (float)(cos(M_PI * cutoff / (sampleRate*0.5)));
    l->cutoff = cutoff;
    l->a0 = (1 - r) * sqrt( r * (r - 4 * (f * f) + 2) + 1);
    l->b1 = 2 * f * r;
    l->b2 = -(r * r);

    l->x1 = 0.0;
    l->x2 = 0.0;

}

//----------------------------------------------------------------------------
float AudioInput::processNotch(NOTCH_FILTER * l, float x0)
{
    float outp = l->a0 * x0 + l->b1 * l->x1 + l->b2 * l->x2;
    l->x2 = l->x1;
    l->x1 = outp;

	if (outp < 0.001) outp = 0.0;

    return outp;
}

//----------------------------------------------------------------------------
void AudioInput::initFilters(int nbands)
{
	float b;
	audiobars = new float[nbands];
	temp_bars = new float[nbands];
	nof_bands=nbands;
	//bp_filter = (NOTCH_FILTER*)malloc(sizeof(NOTCH_FILTER) * nBands);
	bp_filter = new NOTCH_FILTER[nbands];

    for (int i=0; i<nof_bands; i++) 
	{
		b=80.0f+i*(22000.0f-80.0f)/nof_bands;
		//bp_filter[i]=initNotch(b);
		updateNotch(&(bp_filter[i]),b);

	}
}

#define HEIGHT 1.0
#define D 0.45
#define TAU 0.25
#define DIF 5.0

//----------------------------------------------------------------------------
void AudioInput::processFilters()
{
	float scalar = pow( 0.5f, 1.0f/(halfLife * sampleRate));
	//scalar = 0.9;

	float f;


	float lk=2.0;
	float l0=2.0;

	double scale = HEIGHT / ( log((1.0f - D) / D) * 2.0f );
	double x00 = (float)(D*D*1.0f/(2 * D - 1));
	double y00 = -log(-x00) * scale;
	double y;

	for (int i=0; i<bufferSize; i++) {
		for (int b=0; b<nof_bands; b++)
		{
			//temp_bars[b]=0.0;

			f=processNotch(&(bp_filter[b]),buffer[i]);

			/*if (fabs(f)>audiobars[b])
			{
				audiobars[b]=fabs(f);
			}
			else {
				audiobars[b] = audiobars[b] * scalar;
				if( audiobars[b] < 0.00001 ) audiobars[b] = 0.0;
			}*/

			if (fabs(f)>temp_bars[b])
			{
				temp_bars[b]=fabs(f);
			}

			else {
				temp_bars[b] = temp_bars[b] * scalar;
				if( temp_bars[b] < 0.00001 ) temp_bars[b] = 0.0;
			}

			if (temp_bars[b]>0)
			{
			y=temp_bars[b];
			y=y*(b*lk+l0);

			y = ( log(y - x00) * scale + y00 ); 

			y = ( (DIF-2.0)*y +
				(b==0       ? 0 : temp_bars[b-1]) +
				(b==31 ? 0 : temp_bars[b+1])) / DIF;

			y=(1.0-TAU)*audiobars[b]+TAU*y;
			audiobars[b]=y;
			
			}
			else audiobars[b]=temp_bars[b];

			//y = y* pow((float)(i/nof_bands),xScale);
			//audiobars[b] = pow((float)audiobars[b],yScale);

		}
	}
}

//----------------------------------------------------------------------------
bool AudioInput::hasBeenTriggered()
{
	if (level > 0.1) activation = TRUE;
	else
	{
		activation = FALSE; 
		activated = FALSE;
	}
	return activation && !activated;
}

//----------------------------------------------------------------------------
bool AudioInput::isBeingTriggered()
{
	if (level > 0.05) activation = TRUE;
	else
	{
		activated = FALSE;
	}
	return activation && !activated;
}

//----------------------------------------------------------------------------
void AudioInput::updateTrigger()
{
		activation = FALSE;
		activated = TRUE;
}

//----------------------------------------------------------------------------
int AudioInput::getNote()
{
	int freq = -1;

	if (6<=maxFreq && maxFreq<12)
	{
		freq = (maxFreq-6)*2.0;
	}
	if (12<=maxFreq && maxFreq<24)
	{
		freq = maxFreq-12;
	}
	if (24<=maxFreq && maxFreq<48)
	{
		freq = (maxFreq-24)*0.5;
	}
 return freq;
}

//----------------------------------------------------------------------------------

void AudioInput::updateBands(int nb){
	
	delete[] fftBands;
	delete[] fftBandsTemp;

	nBands = nb;

	fftBands = new float[nBands];
	fftBandsTemp = new float[nBands];
	powerN = log10((double)nfftResult)/nBands;

	memset(fftBands, 0, nBands*sizeof(float));
	memset(fftBandsTemp, 0, nBands*sizeof(float));


	if (audiobars != NULL)
		delete[] audiobars;

	if (temp_bars != NULL)
	{
		delete[] temp_bars;

		/*for (int i=0; i<nof_bands; i++) 
		{
		
			delete bp_filter[i];
		}*/
		//free(bp_filter);
		delete[] bp_filter;
	}

	initFilters(nBands);

}
