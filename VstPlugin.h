//	VstPlugin.h - Declaration of the plugin class.
//	--------------------------------------------------------------------------
//	Copyright (c) 2005-2006 Niall Moody
//
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//	--------------------------------------------------------------------------

#ifndef VSTPLUGIN_H_
#define VSTPLUGIN_H_

#define NUMINPUTS 2

#include "audioeffectx.h"
#include <string>
#include "AudioInput.h"
#include "littleTimer.h"

struct PluginProgram;

//Trick to ensure inline functions get inlined properly.
#ifdef WIN32
#define strictinline __forceinline
#elif defined (__GNUC__)
#define strictinline inline __attribute__((always_inline))
#else
#define strictinline inline
#endif

enum {
	kBands,
	kLines

};

enum {
	kFFTMono,
	kFFTAverage,
	kFiltersBank,
	kLevels

};

//---------------------------------------------------------------------------
///	Simple struct representing a program of the plugin.
struct PluginProgram
{
  public:
	 public:
	///	Constructor.
	PluginProgram(): name("Default"),iBufferSize(2048),fGain(1.0f),iNBands(32),fResponse(0.2),iType(kFFTMono),
					iFreqScale(kLog),iDisplay(kBands),fXScale(0.2f),fYScale(1.0f),iAmpScale(kNoScaling),
					iResampling(kSum),sAddress("out1"),sHost("localhost"),iPort(10000)
	{

	};

	PluginProgram(std::string n, int bufferSize, float gain, int bands, float response, int type, int freqscale,
		int display, float xscale, float yscale, int ampscale, int resampling, std::string address, std::string host, int port): 
	name(n),iBufferSize(bufferSize),fGain(gain),iNBands(bands),fResponse(response),iType(type),
					iFreqScale(freqscale),iDisplay(display),fXScale(xscale),fYScale(yscale),iAmpScale(ampscale),
					iResampling(resampling),sAddress(address),sHost(host),iPort(10000)
	{

	};

	///	The parameters of the plugin.
	float parameters[1];
	///	The name of this program.
	std::string name;

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

//----------------------------------------------------------------------------
///	VST plugin class.
class VstPlugin : public AudioEffectX
{
public:

	//PluginProgram myProgram;
	int bufferSize;
	bool bufferok;

	AudioInput* audioInputs[NUMINPUTS];

	float* buffer[NUMINPUTS];
	long cursor;

	bool lockBands;

	littleTimer* temps;


	///	Constructor.
	/*!
		\param audioMaster This is a callback function used by AudioEffect to
		communicate with the host.  Subclasses of AudioEffect/AudioEffectX
		should not ever need to make use of it directly.
	 */
	VstPlugin(audioMasterCallback audioMaster);
	///	Destructor.
	~VstPlugin();

	void updatePluginProgram(PluginProgram* plugprog, std::string n, int bufferSize, float gain, int bands, float response, int type, int freqscale,
		int display, float xscale, float yscale, int ampscale, int resampling, std::string address, std::string host, int port);
	void updateBufferSize(int newBufferSize);

	///	Processes a block of audio, accumulating.
	/*!
		\param inputs Pointer to an array of an array of audio samples
		containing  the audio input to the plugin (well, it won't necessarily
		be an actual, contiguous array, but it's simpler to view it that way).
		\param outputs Pointer to an array of an array of audio samples which
		will contain the audio output of the plugin (again, won't necessarily
		be a contiguous array...).
		\param sampleFrames The number of samples to process for this block.
	 */
	void process(float **inputs, float **outputs, VstInt32 sampleFrames);
	///	Processes a block of audio, replacing.
	/*!
		\sa process()
	 */
	void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames);
	///	Called by the host to inform the plugin of certain events.
	/*!
		\param events Pointer to the events.

		The main events will be MIDI messages.
	 */
	VstInt32 processEvents(VstEvents* events);
	///	Called when audio processing begins.
	/*!
		Use this to set up buffers etc.
	 */
	void resume();
	///	Called when audio processing stops.
	void suspend();

	///	Called to set the plugin's current program.
	/*!
		\param program Index of the program to make current.
	 */
	void setProgram(VstInt32 program);


	//void updateProgram();

	///	Called to set the name of the current program.
	void setProgramName(char *name);
	///	Returns the name of the current program.
	void getProgramName(char *name);
	///	Returns the name of the current program.
	/*!
		\param category If your programs are arranged into categories?
		\param index Index of the program whose name the host wants to query.
		\param text String to put the return name into.

		\return True if successful, false otherwise?

		The aim of this method is to allow hosts to get the names for each
		program in a plugin, without having to call setProgram() first.
	 */
	bool getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text);
	///	Copies the current program to the destination program.
	bool copyProgram(VstInt32 destination);

	///	Called to set the plugin's parameters.
	void setParameter(VstInt32 index, float value);
	///	Returns the value of the indexed parameter.
	float getParameter(VstInt32 index);
	///	Returns the label (units) of the indexed parameter.
	void getParameterLabel(VstInt32 index, char *label);
	///	Returns the value of the indexed parameter as text.
	void getParameterDisplay(VstInt32 index, char *text);
	///	Returns the name of the indexed parameter.
	void getParameterName(VstInt32 index, char *text);

	VstInt32 getChunk (void** data, bool isPreset = false);
	VstInt32 setChunk (void* data, VstInt32 byteSize, bool isPreset = false);

	///	Called by the host to determine the plugin's capabilities.
	/*!
		\return 1=can do, -1=cannot do, 0=don't know.
	 */
	VstInt32 canDo(char* text);
	///	Returns the value of the plugin's VU meter?
	float getVu();
	///	Returns the name of the plugin.
	bool getEffectName(char* name);
	///	Returns the name of the plugin author.
	bool getVendorString(char* text);
	///	Returns the name of the plugin.
	bool getProductString(char* text);
	///	Returns the plugin's version.
	VstInt32 getVendorVersion();
	///	Returns the plugin's category.
	VstPlugCategory getPlugCategory();
	///	Returns certain information about the plugin's inputs.
	/*!
		This is used to assign names to the plugin's inputs.
	 */
	bool getInputProperties(VstInt32 index, VstPinProperties* properties);
	///	Returns certain information about the plugin's outputs.
	/*!
		\sa getInputProperties()
	 */
	bool getOutputProperties(VstInt32 index, VstPinProperties* properties);
	///	Stupid name method...
	VstInt32 getGetTailSize();

	//------------------------------------------
	///	Enum for enumerating the plugin's parameters.
	enum
	{
		param1,

		numParameters
	};
  //private:
	public:
	///	Called for every sample, to dispatch MIDI events appropriately.
	strictinline void processMIDI(VstInt32 pos);
	///	Called when a MIDI Note On message is received.
	strictinline void MIDI_NoteOn(int ch, int note, int val, int delta);
	///	Called when a MIDI Note Off message is received.
	strictinline void MIDI_NoteOff(int ch, int note, int val, int delta);
	///	Called when a MIDI Aftertouch message is received.
	strictinline void MIDI_PolyAftertouch(int ch, int note, int val, int delta);
	///	Called when a MIDI CC message is received.
	strictinline void MIDI_CC(int ch, int num, int val, int delta);
	///	Called when a MIDI Program Change message is received.
	strictinline void MIDI_ProgramChange(int ch, int val, int delta);
	///	Called when a MIDI Aftertouch message is received.
	strictinline void MIDI_ChannelAftertouch(int ch, int val, int delta);
	///	Called when a MIDI Pitchbend message is received.
	strictinline void MIDI_PitchBend(int ch, int x1, int x2, int delta);

	//-----------------------------------------
	///	Various constants.
	enum
	{
		numPrograms = 8,	///<Number of programs this plugin has.
		versionNumber = 100,///<The current version of the plugin.
		maxNumEvents = 250	///<The maximum number of events in our MIDI queue.
	};

	struct PluginProgramBank {
    PluginProgram programs[numPrograms];
	}; 

	PluginProgramBank bank;

    //PluginProgram *programs;				///	Array of our plugin's programs.
	
	float samplerate;						///	The current samplerate.
	float tempo;							///	The current tempo.

	float parameters[numParameters];		///	Array of our plugin's parameters.
	
	VstEvents *tempEvents;					///	VstEvents pointer for outputting events to the host.
	VstMidiEvent *midiEvent[maxNumEvents];	///	Our MIDI event queue.
	int numEvents;							///	The number of events in our event queue.

	///	Sorted array holding the indices of pending events (in midiEvent), for speed.
	int eventNumArray[maxNumEvents];
	///	Number of events in eventNumArray (same as numEvents?).
	int numPendingEvents;

	///	Number of samples in the current processing block.
	VstInt32 frames;
	//-----------------------------------------

	///	Name of the plugin.
	std::string effectName;
	///	Name of the plugin's author.
    std::string vendorName;
};




#endif
