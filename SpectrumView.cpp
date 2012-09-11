#include "SpectrumView.h"


#include <stdio.h>
#include <math.h>


SpectrumView::SpectrumView (const CRect& size, AudioEffect *effect)
: CView (size),effect(effect)
{
	setTransparency (true);

	//sender.setup("localhost", 10000);
}

void SpectrumView::draw (CDrawContext *context){

	float t = ((VstPlugin*)effect)->temps->getElapsedTime();

	CRect r (size);
	context->setFillColor (kBlackCColor);
	context->drawRect (r,kDrawFilled);

	width = size.getWidth();
	bottom = size.bottom;


	switch(displayType) {
		case kFFTMono:
			drawFFT(context, kMono);
			break;

		case kFFTAverage:
			drawFFT(context, kAverage);
			break;

		case kFiltersBank:
			drawFiltersBank(context);
			break;


		case kLevels:
			drawLevels(context);
			break;

	}
	

}

void SpectrumView::drawFFT(CDrawContext *context, int t) {
	
	float* bands0 = ((VstPlugin*)effect)->audioInputs[0]->fftBands;
	float* bands1 = ((VstPlugin*)effect)->audioInputs[1]->fftBands;
	//int nBands = 32;
	int bWidth = (width-50)/(float)nBands;
	//bWidth = 9;

	float curBand;

	switch(display) {
		case kBands:
			for (int i = 0; i<nBands; i++) {
				if (t == kMono) {
					curBand = bands0[i];
				}
				if (t == kAverage) {
					curBand = 0.5*(bands0[i]+bands1[i]);
				}
				CRect rect(10+i*bWidth,bottom-8-(int)(curBand*140*gain),10+(bWidth-5)+i*bWidth,bottom-5);
				//CRect rect(10+i*bWidth,bottom-8-(int)(1*140*gain),10+(bWidth-5)+i*bWidth,bottom-5);
				context->setFillColor (kWhiteCColor);
				context->drawRect (rect, kDrawFilledAndStroked);	
			}
			break;
		
		case kLines:
			context->setDrawMode(kAntialias);
			context->setFrameColor(kWhiteCColor);
			context->setLineWidth(2);

			context->moveTo (CPoint (10, bottom-5));
			for (int i = 1; i<=nBands; i++) {
				if (t == kMono) {
					curBand = bands0[i-1];
				}
				if (t == kAverage) {
					curBand = 0.5*(bands0[i-1]+bands1[i-1]);
				}
				context->lineTo (CPoint (10+i*bWidth, bottom-5-(int)(curBand*140*gain)));
			}
			break;
	}


}


void SpectrumView::drawLevels(CDrawContext *context) {

	float levelL = ((VstPlugin*)effect)->audioInputs[0]->level;
	float levelR = ((VstPlugin*)effect)->audioInputs[1]->level;

	int mul = 140;

	int bWidth = (width-50)/4.0f;

	CRect rect;

	switch(display) {
		case kBands:
			rect(10,bottom-8-(int)(levelL*mul*gain),10+(bWidth-5),bottom-5);
			context->setFillColor (kGreyCColor);
			context->drawRect (rect, kDrawFilledAndStroked);

			rect(10+bWidth,bottom-8-(int)(levelR*mul*gain),10+(bWidth-5)+bWidth,bottom-5);
			context->setFillColor (kRedCColor);
			context->drawRect (rect, kDrawFilledAndStroked);
			
			break;
		
		case kLines:

			context->setDrawMode(kAntialias);
			context->setFrameColor(kWhiteCColor);
			context->setLineWidth(2);

			context->moveTo (CPoint (10, bottom-5));

			context->lineTo (CPoint (10+0.5*bWidth, bottom-5-(int)(levelL*mul*gain)));
			context->lineTo (CPoint (10+bWidth, bottom-5));
			context->lineTo (CPoint (10+1.5*bWidth, bottom-5-(int)(levelR*mul*gain)));

			context->lineTo (CPoint (10+2*bWidth, bottom-5));
			break;

	}
}


void SpectrumView::drawFiltersBank(CDrawContext *context) {

	float* bands0 = ((VstPlugin*)effect)->audioInputs[0]->audiobars;
	float* bands1 = ((VstPlugin*)effect)->audioInputs[1]->audiobars;
	//int nBands = 32;
	int bWidth = (width-50)/(float)nBands;
	//bWidth = 9;

	float curBand;

	switch(display) {
		case kBands:
			for (int i = 0; i<nBands; i++) {
				
				curBand = 0.5*(bands0[i]+bands1[i]);

				CRect rect(10+i*bWidth,bottom-8-(int)(curBand*140*gain),10+(bWidth-5)+i*bWidth,bottom-5);

				context->setFillColor (kWhiteCColor);
				context->drawRect (rect, kDrawFilledAndStroked);	
			}
			break;
		
		case kLines:
			context->setDrawMode(kAntialias);
			context->setFrameColor(kWhiteCColor);
			context->setLineWidth(2);

			context->moveTo (CPoint (10, bottom-5));
			for (int i = 1; i<=nBands; i++) {
					curBand = 0.5*(bands0[i]+bands1[i]);

				context->lineTo (CPoint (10+i*bWidth, bottom-5-(int)(curBand*140*gain)));
			}
			break;
	}
}

