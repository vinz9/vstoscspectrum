#ifndef GUIEDITOR_H_
#define GUIEDITOR_H_

//#ifndef __vstcontrols__
//#include "vstcontrols.h"
//#endif

/*#ifndef __vstgui__
#include "vstgui.h"
#endif*/

#include "aeffguieditor.h"

#include "VstPlugin.h"
#include "VSTGLTimer.h"
#include "ofxOsc.h"

#include "SpectrumView.h";


struct PlugProgram
{

	int iBufferSize;
	int iType;
	float fGain;
	int iNBands;
	int iDisplay;
	int iAmpScale;
	float fXScale;
	float fYScale;
	int iFreqScale;
	int iResampling;
	float fResponse;
	std::string sAddress;
	std::string sHost;
	int iPort;


};

static int const maxBands = 64;
static int const maxGain = 5;
static int const maxXScale = 3;
static int const maxYScale = 3;


class GuiEditor : public AEffGUIEditor, CControlListener, Timer
{
public:
	GuiEditor (void* effect);
	virtual ~GuiEditor ();


	enum {

		kBufferSize,
		kVoices,
		kGain,
		kGainText,
		kBands,
		kBandsText,
		kDisplay,
		kAmpScale,
		kXScale,
		kXScaleText,
		kYScale,
		kYScaleText,
		kFreqScale,
		kResponse,
		kResampling,
		kResponseText,
		kType,
		kAddress,
		kHost,
		kPort

	};

	//void setTabView (CFrame* frame, const CRect& r, long position);

//protected:
	virtual bool open (void *ptr);
	virtual void close ();
	//virtual void idle ();

	virtual void setParameter (VstInt32 index, float value);
	virtual void valueChanged (CControl* control);


	void timerCallback();

	bool bufok;
	bool opened;

	int dWidth;
	int dBottom;

	COptionMenu* bufferMenu;

	CHorizontalSlider* gainFader;
	CTextEdit* gainDisplay;

	CHorizontalSlider* xScaleFader;
	CTextEdit* xScaleDisplay;

	CHorizontalSlider* yScaleFader;
	CTextEdit* yScaleDisplay;

	CHorizontalSlider* responseFader;
	CTextEdit* responseDisplay;

	CHorizontalSlider* bandsFader;
	CTextEdit* bandsDisplay;

	COptionMenu* displayMenu;
	COptionMenu* ampScaleMenu;
	COptionMenu* freqScaleMenu;
	COptionMenu* resamplingMenu;
	COptionMenu* typeMenu;

	CTextEdit* addressDisplay;
	CTextEdit* portDisplay;
	CTextEdit* hostDisplay;


	SpectrumView* mySpectrumView;

	std::string intToString(int n);
	std::string floatToString(float f);
	int charToInt(const char* c);
	float charToFloat(const char* c);


	//void updateParameters();
	void updateDisplay();

	void updateBufferSize(int b);
	void updateBands(int nb);
	void updateGain(float g);
	void updateXScale(float s);
	void updateYScale(float s);
	void updateResponse(float r);
	void updateAmpScale(int s);
	void updateFreqScale(int s);
	void updateDisplayt(int s);
	void updateResampling(int s);
	void updateType(int s); 
	void updateAddress(string a);
	void updateHost(string a);
	void updatePort(int p);

	void sendBands(int type);
	
	PlugProgram myProg;
	void updateGuiProg(PluginProgram* plugprog);

	ofxOscSender sender;

	void updateAudioInput();

	//void bandsStringConvert(float value, char* string);

};

#endif