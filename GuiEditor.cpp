#include "GuiEditor.h"

#include <sstream>
#include <iomanip>


enum {
	// bitmaps
	kHFaderBodyId = 1,
	kHFaderHandleId,
	kVFaderBodyId,
	kVFaderHandleId,
};

enum {

	k512,
	k1024,
	k2048,
	k4096,
	k8192,
};


GuiEditor::GuiEditor(void* effect)
 : AEffGUIEditor ((AudioEffect*)effect),Timer(30) {
	 opened = false;
	//backgroundBitmap = new CBitmap (kBackgroundBitmap);
	// setup size of editor
	rect.left   = 0;
	rect.top    = 0;

	dWidth = 830;
	dBottom = 290;

	rect.right = dWidth;
	rect.bottom = dBottom;

	myProg.iBufferSize = 2048;
	myProg.fGain = 1.0;
	myProg.iNBands = 32;
	myProg.fResponse = 0.2;
	myProg.iType = kBands;
	myProg.iFreqScale = kLog;
	myProg.iDisplay = 0;
	myProg.fXScale = 2.0f;
	myProg.fYScale = 2.0f;
	myProg.iAmpScale = kNoScaling;
	myProg.iResampling = kDecim;
	/*myProg.fXScale = 0.2f;
	myProg.fYScale = 1.0f;
	myProg.iAmpScale = kNoScaling;
	myProg.iResampling = kSum;*/
	myProg.sAddress = "out1";
	myProg.sHost = "locahost";
	myProg.iPort = 10000;

	sender.setup("localhost", 10000);

	bufok = true;


}

GuiEditor::~GuiEditor ()
{
	// free the background bitmap
	/*if (hBackground)
		hBackground->forget ();
	hBackground = 0;*/

}


bool GuiEditor::open (void *ptr)
{
	AEffGUIEditor::open (ptr);
	CRect size (rect.left , rect.top, rect.right, rect.bottom);


	CBitmap* hHFaderBody   = new CBitmap (kHFaderBodyId);
	CBitmap* hHFaderHandle = new CBitmap (kHFaderHandleId);



	CFrame* lFrame = new CFrame(size, ptr, this);
	//lFrame->setBackgroundColor(kGreyCColor);

	int middle = 150;

	size(0,0,dWidth,middle);

	CViewContainer* graphicsView = new CViewContainer(size, lFrame);
	graphicsView->setBackgroundColor(kBlackCColor);

	lFrame->addView(graphicsView);

	size(0,middle,dWidth,dBottom);
	CViewContainer* controlsView = new CViewContainer(size, lFrame);
	controlsView->setBackgroundColor(kGreyCColor);

	lFrame->addView(controlsView);


	size(0,0,700,150);
	mySpectrumView = new SpectrumView(size,effect);
	graphicsView->addView(mySpectrumView);


	int left, top, right, bottom, minPos, maxPos;

	//------------------------------------------------------------------------------
	left = 20;
	top = 10;
	right = left + 70;
	bottom = top + 16;

	size(left, top, right, bottom);
	CTextLabel* bufferLabel = new CTextLabel(size,"BufferSize");
	bufferLabel->setFont (kNormalFontSmall);
	bufferLabel->setFontColor (kWhiteCColor);
	bufferLabel->setBackColor (kBlackCColor);
	bufferLabel->setFrameColor (kBlackCColor);
	controlsView->addView(bufferLabel);

	//------------------------------------------------------------------------------
	left = right + 10;
	top = 10;
	right = left + 70;
	bottom = top + 16;

	size(left, top, right, bottom);
	bufferMenu = new COptionMenu(size, this,kBufferSize);
	bufferMenu->setFont (kNormalFontSmall);
	bufferMenu->setFontColor (kWhiteCColor);
	bufferMenu->setBackColor (kBlackCColor);
	bufferMenu->setFrameColor (kWhiteCColor);

	bufferMenu->addEntry("512",0);
	bufferMenu->addEntry("1024",1);
	bufferMenu->addEntry("2048",2);
	bufferMenu->addEntry("4096",3);
	bufferMenu->addEntry("8192",4);
	bufferMenu->setCurrent(3);

	controlsView->addView(bufferMenu);

	//----------------------------------------------------------------------------

	left = right + 10;
	right = left + 60;


		size(left, top, right, bottom);
	CTextLabel* displayLabel = new CTextLabel(size,"Display");
	displayLabel->setFont (kNormalFontSmall);
	displayLabel->setFontColor (kWhiteCColor);
	displayLabel->setBackColor (kBlackCColor);
	displayLabel->setFrameColor (kBlackCColor);
	controlsView->addView(displayLabel);


	left = right + 10;
	right = left + 50;

	size(left, top, right, bottom);
	displayMenu = new COptionMenu(size, this,kDisplay);
	displayMenu->setFont (kNormalFontSmall);
	displayMenu->setFontColor (kWhiteCColor);
	displayMenu->setBackColor (kBlackCColor);
	displayMenu->setFrameColor (kWhiteCColor);

	displayMenu->addEntry("Bands",0);
	displayMenu->addEntry("Lines",1);
	displayMenu->setCurrent(0);

	controlsView->addView(displayMenu);


	//------------------------------------------------------------------------------

	left = 20;
	top = bottom + 10;
	right = left + 70;
	bottom = top + 16;

	size(left, top, right, bottom);
	CTextLabel* gainLabel = new CTextLabel(size,"Gain");
	gainLabel->setFont (kNormalFontSmall);
	gainLabel->setFontColor (kWhiteCColor);
	gainLabel->setBackColor (kBlackCColor);
	gainLabel->setFrameColor (kBlackCColor);
	controlsView->addView(gainLabel);

	left = right + 10;
	right = left + hHFaderBody->getWidth ();
	bottom = top + hHFaderBody->getHeight ();

	minPos = left;
	maxPos = right - hHFaderHandle->getWidth () - 1;
	CPoint point (0, 0);

	size (left, top, right, bottom);
	gainFader = new CHorizontalSlider (size, this, kGain, minPos, maxPos, hHFaderHandle, hHFaderBody, point, kLeft);
	controlsView->addView(gainFader);

	left = right + 10;
	right = left + 30;
	

	size (left, top, right, bottom);
	gainDisplay = new CTextEdit(size, this, kGainText, "0");
	gainDisplay->setFont (kNormalFontSmall);
	gainDisplay->setFontColor (kWhiteCColor);
	gainDisplay->setBackColor (kBlackCColor);
	gainDisplay->setFrameColor (kWhiteCColor);
	controlsView->addView (gainDisplay);

	//---------------------------------------------------------------------------
	
	left = 20;
	top = bottom + 10;
	right = left + 70;
	bottom = top + 16;

	size(left, top, right, bottom);
	CTextLabel* responseLabel = new CTextLabel(size,"Response");
	responseLabel->setFont (kNormalFontSmall);
	responseLabel->setFontColor (kWhiteCColor);
	responseLabel->setBackColor (kBlackCColor);
	responseLabel->setFrameColor (kBlackCColor);
	controlsView->addView(responseLabel);

	left = right + 10;
	right = left + hHFaderBody->getWidth ();
	bottom = top + hHFaderBody->getHeight ();

	minPos = left;
	maxPos = right - hHFaderHandle->getWidth () - 1;
	point (0, 0);

	size (left, top, right, bottom);
	responseFader = new CHorizontalSlider (size, this, kResponse, minPos, maxPos, hHFaderHandle, hHFaderBody, point, kLeft);
	controlsView->addView(responseFader);

	left = right + 10;
	right = left + 30;
	

	size (left, top, right, bottom);
	responseDisplay = new CTextEdit (size, this, kResponseText, "0");
	responseDisplay->setFont (kNormalFontSmall);
	responseDisplay->setFontColor (kWhiteCColor);
	responseDisplay->setBackColor (kBlackCColor);
	responseDisplay->setFrameColor (kWhiteCColor);
	controlsView->addView (responseDisplay);


	//---------------------------------------------------------------------------
	left = 20;
	top = bottom + 10;
	right = left + 70;
	bottom = top + 16;

		size(left, top, right, bottom);
	CTextLabel* bandsLabel = new CTextLabel(size,"Bands");
	bandsLabel->setFont (kNormalFontSmall);
	bandsLabel->setFontColor (kWhiteCColor);
	bandsLabel->setBackColor (kBlackCColor);
	bandsLabel->setFrameColor (kBlackCColor);
	controlsView->addView(bandsLabel);

	left = right + 10;
	right = left + hHFaderBody->getWidth ();
	bottom = top + hHFaderBody->getHeight ();

	minPos = left;
	maxPos = right - hHFaderHandle->getWidth () - 1;
	point (0, 0);

	size (left, top, right, bottom);
	bandsFader = new CHorizontalSlider(size, this, kBands, minPos, maxPos, hHFaderHandle, hHFaderBody, point, kLeft);
	controlsView->addView(bandsFader);
	bandsFader->setValue(0.5);

	left = right + 10;
	right = left + 30;
	

	size (left, top, right, bottom);
	bandsDisplay = new CTextEdit (size, this, kBandsText, "32");
	bandsDisplay->setFont (kNormalFontSmall);
	bandsDisplay->setFontColor (kWhiteCColor);
	bandsDisplay->setBackColor (kBlackCColor);
	bandsDisplay->setFrameColor (kWhiteCColor);
	//bandsDisplay->setStringConvert(bandsStringConvert);
	controlsView->addView (bandsDisplay);


	//---------------------------------------------------------------------------
	int left2 = right + 40;
	left = left2;
	top = 10;
	right = left + 70;
	bottom = top + 16;

	size(left, top, right, bottom);
	CTextLabel* ampScaleLabel = new CTextLabel(size,"Amp Scale");
	ampScaleLabel->setFont (kNormalFontSmall);
	ampScaleLabel->setFontColor (kWhiteCColor);
	ampScaleLabel->setBackColor (kBlackCColor);
	ampScaleLabel->setFrameColor (kBlackCColor);
	controlsView->addView(ampScaleLabel);


	left = right + 10;
	right = left + 70;

	size(left, top, right, bottom);
	ampScaleMenu = new COptionMenu(size, this,kAmpScale);
	ampScaleMenu->setFont (kNormalFontSmall);
	ampScaleMenu->setFontColor (kWhiteCColor);
	ampScaleMenu->setBackColor (kBlackCColor);
	ampScaleMenu->setFrameColor (kWhiteCColor);

	ampScaleMenu->addEntry("No Scaling",0);
	ampScaleMenu->addEntry("Log",1);
	ampScaleMenu->setCurrent(1);

	controlsView->addView(ampScaleMenu);


	//------------------------------------------------------------------------------

	left = left2;
	top = bottom + 10;
	right = left + 70;
	bottom = top + 16;

		size(left, top, right, bottom);
	CTextLabel* freqScaleLabel = new CTextLabel(size,"Freq Scale");
	freqScaleLabel->setFont (kNormalFontSmall);
	freqScaleLabel->setFontColor (kWhiteCColor);
	freqScaleLabel->setBackColor (kBlackCColor);
	freqScaleLabel->setFrameColor (kBlackCColor);
	controlsView->addView(freqScaleLabel);

	left = right +10;
	right = left + 70;

	size(left, top, right, bottom);
	freqScaleMenu = new COptionMenu(size, this,kFreqScale);
	freqScaleMenu->setFont (kNormalFontSmall);
	freqScaleMenu->setFontColor (kWhiteCColor);
	freqScaleMenu->setBackColor (kBlackCColor);
	freqScaleMenu->setFrameColor (kWhiteCColor);

	freqScaleMenu->addEntry("No Scaling",0);
	freqScaleMenu->addEntry("Log",1);
	freqScaleMenu->setCurrent(1);

	controlsView->addView(freqScaleMenu);

	


	//------------------------------------------------------------------------------

		left = left2;
	top = bottom + 10;
	right = left + 70;
	bottom = top + 16;

		size(left, top, right, bottom);
	CTextLabel* resamplingLabel = new CTextLabel(size,"Resampling");
	resamplingLabel->setFont (kNormalFontSmall);
	resamplingLabel->setFontColor (kWhiteCColor);
	resamplingLabel->setBackColor (kBlackCColor);
	resamplingLabel->setFrameColor (kBlackCColor);
	controlsView->addView(resamplingLabel);

	left = right +10;
	right = left + 70;

	size(left, top, right, bottom);
	resamplingMenu = new COptionMenu(size, this,kResampling);
	resamplingMenu->setFont (kNormalFontSmall);
	resamplingMenu->setFontColor (kWhiteCColor);
	resamplingMenu->setBackColor (kBlackCColor);
	resamplingMenu->setFrameColor (kWhiteCColor);

	resamplingMenu->addEntry("Decim",0);
	resamplingMenu->addEntry("Sum",1);
	resamplingMenu->setCurrent(0);

	controlsView->addView(resamplingMenu);

	
	//------------------------------------------------------------------------------

	left = left2;
	top = bottom + 10;
	right = left + 70;
	bottom = top + 16;

	size(left, top, right, bottom);
	CTextLabel* xScaleLabel = new CTextLabel(size,"X Scale");
	xScaleLabel->setFont (kNormalFontSmall);
	xScaleLabel->setFontColor (kWhiteCColor);
	xScaleLabel->setBackColor (kBlackCColor);
	xScaleLabel->setFrameColor (kBlackCColor);
	controlsView->addView(xScaleLabel);

	left = right + 10;
	right = left + hHFaderBody->getWidth ();
	bottom = top + hHFaderBody->getHeight ();

	minPos = left;
	maxPos = right - hHFaderHandle->getWidth () - 1;
	point (0, 0);

	size (left, top, right, bottom);
	xScaleFader = new CHorizontalSlider (size, this, kXScale, minPos, maxPos, hHFaderHandle, hHFaderBody, point, kLeft);
	controlsView->addView(xScaleFader);

	left = right + 10;
	right = left + 30;
	

	size (left, top, right, bottom);
	xScaleDisplay = new CTextEdit (size, this, kXScaleText, "0");
	xScaleDisplay->setFont (kNormalFontSmall);
	xScaleDisplay->setFontColor (kWhiteCColor);
	xScaleDisplay->setBackColor (kBlackCColor);
	xScaleDisplay->setFrameColor (kWhiteCColor);
	controlsView->addView (xScaleDisplay);


	//---------------------------------------------------------------------------

		left = left2;
	top = bottom + 10;
	right = left + 70;
	bottom = top + 16;

	size(left, top, right, bottom);
	CTextLabel* yScaleLabel = new CTextLabel(size,"Y Scale");
	yScaleLabel->setFont (kNormalFontSmall);
	yScaleLabel->setFontColor (kWhiteCColor);
	yScaleLabel->setBackColor (kBlackCColor);
	yScaleLabel->setFrameColor (kBlackCColor);
	controlsView->addView(yScaleLabel);

	left = right + 10;
	right = left + hHFaderBody->getWidth ();
	bottom = top + hHFaderBody->getHeight ();

	minPos = left;
	maxPos = right - hHFaderHandle->getWidth () - 1;
	point (0, 0);

	size (left, top, right, bottom);
	yScaleFader = new CHorizontalSlider (size, this, kYScale, minPos, maxPos, hHFaderHandle, hHFaderBody, point, kLeft);
	controlsView->addView(yScaleFader);

	left = right + 10;
	right = left + 30;
	

	size (left, top, right, bottom);
	yScaleDisplay = new CTextEdit (size, this, kYScaleText, "0");
	yScaleDisplay->setFont (kNormalFontSmall);
	yScaleDisplay->setFontColor (kWhiteCColor);
	yScaleDisplay->setBackColor (kBlackCColor);
	yScaleDisplay->setFrameColor (kWhiteCColor);
	controlsView->addView (yScaleDisplay);


	//---------------------------------------------------------------------------

	int left3 = right + 40;
	left = left3;
	top = 10;
	right = left + 70;
	bottom = top + 16;

	
		size(left, top, right, bottom);
	CTextLabel* typeLabel = new CTextLabel(size,"Type");
	typeLabel->setFont (kNormalFontSmall);
	typeLabel->setFontColor (kWhiteCColor);
	typeLabel->setBackColor (kBlackCColor);
	typeLabel->setFrameColor (kBlackCColor);
	controlsView->addView(typeLabel);


	left = right + 10;
	right = left + 70;

	size(left, top, right, bottom);
	typeMenu = new COptionMenu(size, this,kType);
	typeMenu->setFont (kNormalFontSmall);
	typeMenu->setFontColor (kWhiteCColor);
	typeMenu->setBackColor (kBlackCColor);
	typeMenu->setFrameColor (kWhiteCColor);

	typeMenu->addEntry("FFT Mono",0);
	typeMenu->addEntry("FFT Average",1);
	typeMenu->addEntry("Filtersbank",2);
	typeMenu->addEntry("Levels",3);
	typeMenu->setCurrent(0);

	controlsView->addView(typeMenu);

	//--------------------------------------------------------------------


	left = left3;
	top = bottom + 10;
	right = left + 70;
	bottom = top + 16;

	size(left, top, right, bottom);
	CTextLabel* addressLabel = new CTextLabel(size,"Adress");
	addressLabel->setFont (kNormalFontSmall);
	addressLabel->setFontColor (kWhiteCColor);
	addressLabel->setBackColor (kBlackCColor);
	addressLabel->setFrameColor (kBlackCColor);
	controlsView->addView(addressLabel);

	left = right +10;
	right = left + 70;

	size (left, top, right, bottom);
	addressDisplay = new CTextEdit(size, this, kAddress, "out1");
	addressDisplay->setFont (kNormalFontSmall);
	addressDisplay->setFontColor (kWhiteCColor);
	addressDisplay->setBackColor (kBlackCColor);
	addressDisplay->setFrameColor (kWhiteCColor);
	controlsView->addView (addressDisplay);

	//------------------------------------------------------------------------------

	left = left3;
	top = bottom + 10;
	right = left + 70;
	bottom = top + 16;

	size(left, top, right, bottom);
	CTextLabel* portLabel = new CTextLabel(size,"Port");
	portLabel->setFont (kNormalFontSmall);
	portLabel->setFontColor (kWhiteCColor);
	portLabel->setBackColor (kBlackCColor);
	portLabel->setFrameColor (kBlackCColor);
	controlsView->addView(portLabel);

	left = right +10;
	right = left + 70;

	size (left, top, right, bottom);
	portDisplay = new CTextEdit (size, this, kPort, "12000");
	portDisplay->setFont (kNormalFontSmall);
	portDisplay->setFontColor (kWhiteCColor);
	portDisplay->setBackColor (kBlackCColor);
	portDisplay->setFrameColor (kWhiteCColor);
	controlsView->addView (portDisplay);

	//------------------------------------------------------------------------------

	left = left3;
	top = bottom + 10;
	right = left + 70;
	bottom = top + 16;

	size(left, top, right, bottom);
	CTextLabel* hostLabel = new CTextLabel(size,"host");
	hostLabel->setFont (kNormalFontSmall);
	hostLabel->setFontColor (kWhiteCColor);
	hostLabel->setBackColor (kBlackCColor);
	hostLabel->setFrameColor (kBlackCColor);
	controlsView->addView(hostLabel);

	left = right +10;
	right = left + 70;

	size (left, top, right, bottom);
	hostDisplay = new CTextEdit (size, this, kHost, "localhost");
	hostDisplay->setFont (kNormalFontSmall);
	hostDisplay->setFontColor (kWhiteCColor);
	hostDisplay->setBackColor (kBlackCColor);
	hostDisplay->setFrameColor (kWhiteCColor);
	controlsView->addView (hostDisplay);

	//------------------------------------------------------------------------------


	hHFaderBody->forget ();
	hHFaderHandle->forget ();

	opened = true;

	updateDisplay();

	this->frame = lFrame;


	start();


	return true;
}

void GuiEditor::close ()
{
	// don't forget to remove the frame !!
	if (frame)
		delete frame;
	frame = 0;

	opened = false;

	/*if (mySpectrumView)
		delete mySpectrumView;
	mySpectrumView = 0;*/
}


void GuiEditor::setParameter (VstInt32 index, float value)
{

	/*if (frame && index < VstPlugin::numParameters)
	{
		controls[index]->setValue (value);
		controls[index+VstPlugin::numParameters]->setValue (value);

	}*/

}

//-----------------------------------------------------------------------------
void GuiEditor::valueChanged (CControl* control)
{
	//effect->setParameterAutomated (control->getTag (), control->getValue ());

	float value;
	int ival;
	const char* text;
	std::string result;
	char* tempt;

	switch (control->getTag())
	{

		case kBufferSize:
			((COptionMenu*)(control))->getCurrent(tempt);
			ival = ((COptionMenu*)(control))->getIndex(tempt);
			updateBufferSize(ival);
			break;
	
		case kGain:
			value = control->getValue();
			result = floatToString(value*maxGain);
			gainDisplay->setText(result.c_str());
			updateGain(value*maxGain);
			break;

		case kGainText:
			text = ((CTextEdit*)(control))->getText();
			value = charToFloat(text);
			gainFader->setValue(value/(float)maxGain);
			updateGain(value);
			break;

		case kXScale:
			value = control->getValue();
			result = floatToString(value*maxXScale);
			xScaleDisplay->setText(result.c_str());
			updateXScale(value*maxXScale);
			break;

		case kXScaleText:
			text = ((CTextEdit*)(control))->getText();
			value = charToFloat(text);
			xScaleFader->setValue(value/(float)maxXScale);
			updateXScale(value);
			break;

		case kYScale:
			value = control->getValue();
			result = floatToString(value*maxYScale);
			yScaleDisplay->setText(result.c_str());
			updateYScale(value*maxYScale);
			break;

		case kYScaleText:
			text = ((CTextEdit*)(control))->getText();
			value = charToFloat(text);
			yScaleFader->setValue(value/(float)maxYScale);
			updateYScale(value);
			break;

		case kResponse:
			value = control->getValue();
			result = floatToString(value);
			responseDisplay->setText(result.c_str());
			updateResponse(value);
			break;

		case kResponseText:
			text = ((CTextEdit*)(control))->getText();
			value = charToFloat(text);
			responseFader->setValue(value);
			updateResponse(value);
			break;

		case kBands:
			ival = (int)(control->getValue()*maxBands);
			result = intToString(ival);
			bandsDisplay->setText(result.c_str());
			bandsFader->setValue(((float)ival)/(float)maxBands);
			updateBands(ival);
			break;

		case kBandsText:
			text = ((CTextEdit*)(control))->getText();
			ival = charToInt(text);
			bandsFader->setValue((float)value/(float)maxBands);
			updateBands(ival);
			break;

		case kAmpScale:
			((COptionMenu*)(control))->getCurrent(tempt);
			ival = ((COptionMenu*)(control))->getIndex(tempt);
			updateAmpScale(ival);
			break;

		case kType:
			((COptionMenu*)(control))->getCurrent(tempt);
			ival = ((COptionMenu*)(control))->getIndex(tempt);
			updateType(ival);
			break;

		case kDisplay:
			((COptionMenu*)(control))->getCurrent(tempt);
			ival = ((COptionMenu*)(control))->getIndex(tempt);
			updateDisplayt(ival);
			break;

		case kResampling:
			((COptionMenu*)(control))->getCurrent(tempt);
			ival = ((COptionMenu*)(control))->getIndex(tempt);
			updateResampling(ival);
			break;

		case kFreqScale:
			((COptionMenu*)(control))->getCurrent(tempt);
			ival = ((COptionMenu*)(control))->getIndex(tempt);
			updateFreqScale(ival);
			break;



		case kAddress:
			text = ((CTextEdit*)(control))->getText();
			updateAddress(text);
			break;

		case kPort:
			text = ((CTextEdit*)(control))->getText();
			ival = charToInt(text);
			updatePort(ival);
			break;

		case kHost:
			text = ((CTextEdit*)(control))->getText();
			updateHost(text);
			break;
	}

}

std::string GuiEditor::intToString(int n){
	std::ostringstream oss;
	oss << n;
	std::string result = oss.str();
	return result;
}

std::string GuiEditor::floatToString(float f){
	std::ostringstream oss;
	oss << std::setprecision(2) << f;
	//oss << f ;
	std::string result = oss.str();
	return result;
}

int GuiEditor::charToInt(const char* c){

	std::istringstream iss(c);
	int number;
	iss >> number; 
	return number;
}

float GuiEditor::charToFloat(const char* c){

	std::istringstream iss(c);
	float number;
	iss >> number; 
	return number;
}



void GuiEditor::timerCallback() {

	if (bufok) {

		switch(myProg.iType) {
			
				case kFFTMono:
					((VstPlugin*)effect)->audioInputs[0]->fftw();
					break;

				case kFFTAverage:
					((VstPlugin*)effect)->audioInputs[0]->fftw();
					((VstPlugin*)effect)->audioInputs[1]->fftw();
					break;

				case kFiltersBank:
					((VstPlugin*)effect)->audioInputs[0]->processFilters();
					((VstPlugin*)effect)->audioInputs[1]->processFilters();
					break;

				case kLevels:
					((VstPlugin*)effect)->audioInputs[0]->peak();
					((VstPlugin*)effect)->audioInputs[1]->peak();
					break;
	
		}
		sendBands(myProg.iType);
		//((VstPlugin*)effect)->audioInputs[0]->peak();
		if (mySpectrumView->nBands>0)
			mySpectrumView->setDirty();
	}
}


void GuiEditor::updateBands(int nb) {

	int cur = ((VstPlugin*)effect)->curProgram;
	((VstPlugin*)effect)->bank.programs[cur].iNBands = nb;
	myProg.iNBands = nb;
	((VstPlugin*)effect)->audioInputs[0]->updateBands(nb);
	((VstPlugin*)effect)->audioInputs[1]->updateBands(nb);
	mySpectrumView->nBands = nb;
}


void GuiEditor::updateGain(float g) {

	int cur = ((VstPlugin*)effect)->curProgram;
	((VstPlugin*)effect)->bank.programs[cur].fGain = g;
	myProg.fGain = g;
	mySpectrumView->gain = g;
}

void GuiEditor::updateXScale(float s) {

	int cur = ((VstPlugin*)effect)->curProgram;
	((VstPlugin*)effect)->bank.programs[cur].fXScale = s;
	myProg.fXScale = s;
	((VstPlugin*)effect)->audioInputs[0]->xScale = s;
	((VstPlugin*)effect)->audioInputs[1]->xScale = s;
}

void GuiEditor::updateYScale(float s) {

	int cur = ((VstPlugin*)effect)->curProgram;
	((VstPlugin*)effect)->bank.programs[cur].fYScale = s;
	myProg.fYScale = s;
	((VstPlugin*)effect)->audioInputs[0]->yScale = s;
	((VstPlugin*)effect)->audioInputs[1]->yScale = s;
}

void GuiEditor::updateResponse(float r) {

	int cur = ((VstPlugin*)effect)->curProgram;
	((VstPlugin*)effect)->bank.programs[cur].fResponse = r;
	myProg.fResponse = r;
	((VstPlugin*)effect)->audioInputs[0]->halfLife=r;
	((VstPlugin*)effect)->audioInputs[1]->halfLife=r;
}

void GuiEditor::updateAmpScale(int s) {

	int cur = ((VstPlugin*)effect)->curProgram;
	((VstPlugin*)effect)->bank.programs[cur].iAmpScale = s;
	myProg.iAmpScale = s;
	((VstPlugin*)effect)->audioInputs[0]->curFftScaling = s;
	((VstPlugin*)effect)->audioInputs[1]->curFftScaling = s;
}

void GuiEditor::updateFreqScale(int s) {

	int cur = ((VstPlugin*)effect)->curProgram;
	((VstPlugin*)effect)->bank.programs[cur].iFreqScale = s;
	myProg.iFreqScale = s;
	((VstPlugin*)effect)->audioInputs[0]->curFreqScaling = s;
	((VstPlugin*)effect)->audioInputs[1]->curFreqScaling = s;
}

void GuiEditor::updateDisplayt(int s) {

	int cur = ((VstPlugin*)effect)->curProgram;
	((VstPlugin*)effect)->bank.programs[cur].iDisplay = s;
	myProg.iDisplay = s;
	mySpectrumView->display = s;
}
void GuiEditor::updateResampling(int s) {

	int cur = ((VstPlugin*)effect)->curProgram;
	((VstPlugin*)effect)->bank.programs[cur].iResampling = s;
	myProg.iResampling = s;
	((VstPlugin*)effect)->audioInputs[0]->curResampling = s;
}

void GuiEditor::updateType(int s) {

	int cur = ((VstPlugin*)effect)->curProgram;
	((VstPlugin*)effect)->bank.programs[cur].iType = s;
	myProg.iType = s;
	mySpectrumView->displayType = s;
}





void GuiEditor::updateAddress(string a){

	int cur = ((VstPlugin*)effect)->curProgram;
	((VstPlugin*)effect)->bank.programs[cur].sAddress = a;
	myProg.sAddress = a;
}

void GuiEditor::updateHost(string a){

	int cur = ((VstPlugin*)effect)->curProgram;
	((VstPlugin*)effect)->bank.programs[cur].sHost = a;
	myProg.sHost = a;
	int port = myProg.iPort;
	sender.setup(a, port);
}


void GuiEditor::updatePort(int p){

	int cur = ((VstPlugin*)effect)->curProgram;
	((VstPlugin*)effect)->bank.programs[cur].iPort = p;
	myProg.iPort = p;
	sender.setup(myProg.sHost, p);

}

void GuiEditor::updateBufferSize(int b){

	int cur = ((VstPlugin*)effect)->curProgram;
	
	switch (b){
		case k512:
			myProg.iBufferSize = 512;
			((VstPlugin*)effect)->bank.programs[cur].iBufferSize = 512;
			break;

		case k1024:
			myProg.iBufferSize = 1024;
			((VstPlugin*)effect)->bank.programs[cur].iBufferSize = 1024;
			break;

		case k2048:
			myProg.iBufferSize = 2048;
			((VstPlugin*)effect)->bank.programs[cur].iBufferSize = 2048;
			break;

		case k4096:
			myProg.iBufferSize = 4096;
			((VstPlugin*)effect)->bank.programs[cur].iBufferSize = 4096;
			break;

		case k8192:
			myProg.iBufferSize = 8192;
			((VstPlugin*)effect)->bank.programs[cur].iBufferSize = 8192;
			break;

	}

	((VstPlugin*)effect)->updateBufferSize(myProg.iBufferSize);
	

}

void GuiEditor::sendBands(int type) {

	int size = 0;
	float curBand;
	ofxOscMessage m;
	m.setAddress("/"+myProg.sAddress+"/");

	float* bands0;
	float* bands1;

	switch(type){
		case kFFTMono:
			size = myProg.iNBands;
			bands0 = ((VstPlugin*)effect)->audioInputs[0]->fftBands;
			for (int i = 0; i<myProg.iNBands; i++) {
				curBand = bands0[i];
				m.addFloatArg(curBand*myProg.fGain);
			}

			break;

		case kFFTAverage:
			size = myProg.iNBands;
			bands0 = ((VstPlugin*)effect)->audioInputs[0]->fftBands;
			bands1 = ((VstPlugin*)effect)->audioInputs[1]->fftBands;

			for (int i = 0; i<myProg.iNBands; i++) {
				curBand = 0.5*(bands0[i]+bands1[i]);
				m.addFloatArg(curBand*myProg.fGain);
			}

			break;

		case kFiltersBank:
			size = myProg.iNBands;
			bands0 = ((VstPlugin*)effect)->audioInputs[0]->audiobars;
			bands1 = ((VstPlugin*)effect)->audioInputs[1]->audiobars;

			for (int i = 0; i<myProg.iNBands; i++) {
				curBand = 0.5*(bands0[i]+bands1[i]);
				m.addFloatArg(curBand*myProg.fGain);
			}

			break;

		case kLevels:
			float levelL = ((VstPlugin*)effect)->audioInputs[0]->level;
			float levelR = ((VstPlugin*)effect)->audioInputs[1]->level;

			m.addFloatArg(levelL*myProg.fGain);
			m.addFloatArg(levelR*myProg.fGain);

			break;


	}

	sender.sendMessage( m );


}

void GuiEditor::updateDisplay() {

	if(opened == true) {

	int bufsize = -1;
	switch(myProg.iBufferSize){
		case 512:
			bufsize = 0;
			break;

		case 1024:
			bufsize = 1;
			break;

		case 2048:
			bufsize = 2;
			break;

		case 4096:
			bufsize = 3;
			break;

		case 8192:
			bufsize = 4;
			break;
	}
	bufferMenu->setCurrent(bufsize);

	float val = myProg.fGain;

	gainFader->setValue(val/(float)maxGain);
	gainDisplay->setText(floatToString(val).c_str());
	mySpectrumView->gain = val;

	val = myProg.fResponse;
	responseFader->setValue(val);
	responseDisplay->setText(floatToString(val).c_str());

	val = myProg.fXScale;
	xScaleFader->setValue(val/(float)maxXScale);
	xScaleDisplay->setText(floatToString(val).c_str());

	val = myProg.fYScale;
	yScaleFader->setValue(val/(float)maxYScale);
	yScaleDisplay->setText(floatToString(val).c_str());

	int nBands = myProg.iNBands;

	bandsFader->setValue(nBands/(float)maxBands);
	bandsDisplay->setText(intToString(nBands).c_str());
	mySpectrumView->nBands = nBands;

	ampScaleMenu->setCurrent(myProg.iAmpScale);
	freqScaleMenu->setCurrent(myProg.iFreqScale);
	displayMenu->setCurrent(myProg.iDisplay);
	mySpectrumView->display = myProg.iDisplay;
	typeMenu->setCurrent(myProg.iType);
	mySpectrumView->displayType = myProg.iType;
	resamplingMenu->setCurrent(myProg.iResampling);


	std::string address = myProg.sAddress;
	//const char * c = address.c_str();
	addressDisplay->setText(address.c_str());
	//addressDisplay->setText(c);

	std::string host = myProg.sHost;
	//const char * d = host.c_str();
	hostDisplay->setText(host.c_str());


	int port = myProg.iPort;
	portDisplay->setText(intToString(port).c_str());

	sender.setup(myProg.sHost, myProg.iPort);
	
	}

}

void GuiEditor::updateAudioInput() {

	((VstPlugin*)effect)->audioInputs[0]->updateBands(myProg.iNBands);
	((VstPlugin*)effect)->audioInputs[0]->halfLife = myProg.fResponse;
	((VstPlugin*)effect)->audioInputs[0]->curFftScaling = myProg.iAmpScale;
	((VstPlugin*)effect)->audioInputs[0]->curFreqScaling = myProg.iFreqScale;
	((VstPlugin*)effect)->audioInputs[0]->curResampling = myProg.iResampling;
	((VstPlugin*)effect)->audioInputs[0]->xScale = myProg.fXScale;
	((VstPlugin*)effect)->audioInputs[0]->yScale = myProg.fYScale;

	((VstPlugin*)effect)->audioInputs[1]->updateBands(myProg.iNBands);
	((VstPlugin*)effect)->audioInputs[1]->halfLife = myProg.fResponse;
	((VstPlugin*)effect)->audioInputs[1]->curFftScaling = myProg.iAmpScale;
	((VstPlugin*)effect)->audioInputs[1]->curFreqScaling = myProg.iFreqScale;
	((VstPlugin*)effect)->audioInputs[1]->curResampling = myProg.iResampling;
	((VstPlugin*)effect)->audioInputs[1]->xScale = myProg.fXScale;
	((VstPlugin*)effect)->audioInputs[1]->yScale = myProg.fYScale;




}
void GuiEditor::updateGuiProg(PluginProgram* plugprog){

	myProg.iBufferSize = plugprog->iBufferSize;
	myProg.iType = plugprog->iType;
	myProg.fGain = plugprog->fGain;
	myProg.iNBands = plugprog->iNBands;
	myProg.iDisplay = plugprog->iDisplay;
	myProg.iAmpScale = plugprog->iAmpScale;
	myProg.fXScale = plugprog->fXScale;
	myProg.fYScale = plugprog->fYScale;
	myProg.iFreqScale = plugprog->iFreqScale;
	myProg.iResampling = plugprog->iResampling;
	myProg.fResponse = plugprog->fResponse;
	myProg.sAddress = plugprog->sAddress;
	myProg.sHost = plugprog->sHost;
	myProg.iPort = plugprog->iPort;

}