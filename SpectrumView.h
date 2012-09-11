#ifndef SPECTRUMVIEW_H_
#define SPECTRUMVIEW_H_

#ifndef __vstgui__
#include "vstgui.h"
#endif

#include "ofxOsc.h"
#include "VstPlugin.h"

enum {
	kMono,
	kAverage
};

class SpectrumView : public CView {

public:

	SpectrumView (const CRect& size, AudioEffect* effect);


	void draw(CDrawContext *pContext);

	AudioEffect* effect;

	int nBands;
	int type;
	float gain;
	int displayType;
	int display;

	ofxOscSender sender;

	void drawFFT(CDrawContext *context, int t);
	void drawFiltersBank(CDrawContext *context);
	void drawLevels(CDrawContext *context);

	float* bands0;
	float* bands1;

	float width;
	float bottom;

	//void sendBands();


};


#endif