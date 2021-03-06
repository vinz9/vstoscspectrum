//	VstPlugin.cpp - Definition of the plugin class.
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

#include "VstPlugin.h"
//#include "SpectrumOSC.h"
#include "GuiEditor.h"
#include <math.h>

using namespace std;

//----------------------------------------------------------------------------
AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{

	return new VstPlugin(audioMaster);
}

//----------------------------------------------------------------------------
VstPlugin::VstPlugin(audioMasterCallback audioMaster):
AudioEffectX(audioMaster, numPrograms, numParameters),
//programs(0),
samplerate(44100.0f),
tempo(120.0f),
tempEvents(0),
numEvents(0),
numPendingEvents(0),
frames(0),
effectName("SpectrumOSC"),
vendorName("Foliativ")
{
	//setInitialDelay(40192);
	int i;

	//Initialise parameters.
	for(i=0;i<numParameters;++i)
		parameters[i] = 0.0f;

	//Setup event queue.
	for(i=0;i<maxNumEvents;++i)
	{
		midiEvent[i] = new VstMidiEvent;

		midiEvent[i]->type = 1;
		midiEvent[i]->midiData[3] = 0;
		midiEvent[i]->reserved1 = 99;
		midiEvent[i]->deltaFrames = -99;
		midiEvent[i]->noteLength = 0;
		midiEvent[i]->noteOffset = 0;

		eventNumArray[i] = -1;
	}
	tempEvents = new VstEvents;
	tempEvents->numEvents = 1;
	tempEvents->events[0] = (VstEvent *)midiEvent[0];

	programsAreChunks(true);

	//PluginProgramBank bank;

	//Setup programs here.
	//bank.programs = new PluginProgram[numPrograms];

	updatePluginProgram(&(bank.programs[0]),"FFTsum", 2048, 0.7f, 32, 0.2f, kFFTAverage,kLog, kBands, 0.2,1,kNoScaling,kSum,"out1","localhost",10000);
	updatePluginProgram(&(bank.programs[1]),"FFTdecim", 2048, 3.0f, 32, 0.2f, kFFTAverage,kLog, kLines, 2.5,1,kNoScaling,kDecim,"out1","localhost",10000);
	updatePluginProgram(&(bank.programs[2]),"Levels", 2048, 1.0f, 32, 0.2f, kLevels,kLog, kBands, 1,1,kNoScaling,kSum,"out1","localhost",10000);
	updatePluginProgram(&(bank.programs[3]),"FiltersBank", 2048, 1.0f, 32, 0.2f, kFiltersBank,kLog, kBands, 1,1,kNoScaling,kSum,"out1","localhost",10000);

	updatePluginProgram(&(bank.programs[4]),"FFTsum", 2048, 0.7f, 32, 0.2f, kFFTAverage,kLog, kBands, 0.2,1,kNoScaling,kSum,"out1","localhost",10000);
	updatePluginProgram(&(bank.programs[5]),"FFTdecim", 2048, 3.0f, 32, 0.2f, kFFTAverage,kLog, kLines, 2.5,1,kNoScaling,kDecim,"out1","localhost",10000);
	updatePluginProgram(&(bank.programs[6]),"Levels", 2048, 1.0f, 32, 0.2f, kLevels,kLog, kBands, 1,1,kNoScaling,kSum,"out1","localhost",10000);
	updatePluginProgram(&(bank.programs[7]),"FiltersBank", 2048, 1.0f, 32, 0.2f, kFiltersBank,kLog, kBands, 1,1,kNoScaling,kSum,"out1","localhost",10000);
	

	//Set various constants.
    setNumInputs(NUMINPUTS);
    setNumOutputs(NUMINPUTS);
    canProcessReplacing(true);
    isSynth(false);
    setUniqueID('sOSC');


	//Construct editor here.
	//editor = new SpectrumOSC(this);
	temps = new littleTimer();

	lockBands = false;
	editor = new GuiEditor (this);

	cursor = 0;
	bufferSize = bank.programs[0].iBufferSize;
	bufferok = true;
	int nBands = bank.programs[0].iNBands;
	float response = bank.programs[0].fResponse;
	int ampScale = bank.programs[0].iAmpScale;
	int freqScale = bank.programs[0].iFreqScale;
	int resampling = bank.programs[0].iResampling;
	float xScale = bank.programs[0].fXScale;
	float yScale = bank.programs[0].fYScale;


	for (int i = 0; i<NUMINPUTS; i++)
	{
		audioInputs[i] = NULL;
		audioInputs[i] = new AudioInput(sampleRate,bufferSize,nBands,response,ampScale, freqScale, resampling,xScale,yScale,this);
		buffer[i] = NULL;
		buffer[i] = new float[bufferSize];
		//memset(buffer[i], 0, bufferSize*sizeof(float));
	}

	//setProgram(0);
	((GuiEditor*)editor)->updateGuiProg(&(bank.programs[curProgram]));

}

//----------------------------------------------------------------------------
VstPlugin::~VstPlugin()
{
	int i;

	//Delete event queue.
	for(i=0;i<maxNumEvents;++i)
	{
		if(midiEvent[i])
			delete midiEvent[i];
	}
	if(tempEvents)
		delete tempEvents;

	//Delete programs.
	//if(programs)
	//	delete [] programs;

	for (int i = 0; i<NUMINPUTS; i++)
	{
		if(audioInputs[i]!=NULL)
			delete audioInputs[i];
		if(buffer[i]!=NULL)
			delete[] buffer[i];
	}

	//delete temps;
}

//---------------------------------------------------------------------------
void VstPlugin::updatePluginProgram(PluginProgram* plugprog, std::string n, int bufferSize, float gain, int bands,
									float response, int type, int freqscale, int display, float xscale, float yscale,
									int ampscale, int resampling, std::string address, std::string host, int port) {

	plugprog->name = n;
	plugprog->iBufferSize = bufferSize;
	plugprog->iType = type;
	plugprog->fGain = gain;
	plugprog->iNBands = bands;
	plugprog->iDisplay = display;
	plugprog->iAmpScale = ampscale;
	plugprog->fXScale = xscale;
	plugprog->fYScale = yscale;
	plugprog->iFreqScale = freqscale;
	plugprog->iResampling = resampling;
	plugprog->fResponse = response;
	plugprog->sAddress = address;
	plugprog->sHost = host;
	plugprog->iPort = port;


}

//---------------------------------------------------------------------------
void VstPlugin::updateBufferSize(int newBufferSize){

	bufferok = false;
	((GuiEditor*)editor)->bufok = false;

	for (int i = 0; i<NUMINPUTS; i++)
	{
		if(buffer[i]!=NULL)
			delete[] buffer[i];
		//audioInputs[i]->updateNSamples(newBufferSize);
		if(audioInputs[i]!=NULL)
			delete audioInputs[i];


	}

	int nBands = ((GuiEditor*)editor)->myProg.iNBands;
	float response = ((GuiEditor*)editor)->myProg.fResponse;
	int ampScale = ((GuiEditor*)editor)->myProg.iAmpScale;
	int freqScale = ((GuiEditor*)editor)->myProg.iFreqScale;
	int resampling = ((GuiEditor*)editor)->myProg.iResampling;
	float xScale = ((GuiEditor*)editor)->myProg.fXScale;
	float yScale = ((GuiEditor*)editor)->myProg.fYScale;

	bufferSize = newBufferSize;

	for (int i = 0; i<NUMINPUTS; i++)
	{
		buffer[i] = new float[newBufferSize];
		memset(buffer[i], 0, newBufferSize*sizeof(float));
		audioInputs[i] = new AudioInput(sampleRate,newBufferSize,nBands,response,ampScale,freqScale,resampling,xScale,yScale,this);
		//audioInputs[i] = new AudioInput(sampleRate,newBufferSize,nBands,response,AmpScale);
		audioInputs[i]->cursor = 0;
	}

	bufferok = true;
	((GuiEditor*)editor)->bufok = true;


}

//----------------------------------------------------------------------------
void VstPlugin::process(float **inputs, float **outputs, VstInt32 sampleFrames)
{
	
VstInt32 i;

	frames = sampleFrames;

	//Calculate audio.
	for(i=0;i<frames;++i)
	{
		//Process MIDI events,if there are any.
		if(numEvents>0)
			processMIDI(i);

		outputs[0][i] += inputs[0][i];
		outputs[1][i] += inputs[1][i];
	}
	//If there are events remaining in the queue, update their delta values.
	if(numPendingEvents > 0)
	{
		for(i=0;i<numPendingEvents;++i)
			midiEvent[eventNumArray[i]]->deltaFrames -= frames;
	}	
}

//----------------------------------------------------------------------------
void VstPlugin::processReplacing(float **inputs,
								 float **outputs,
								 VstInt32 sampleFrames)
{
	for(int i=0; i<sampleFrames; ++i)
	{
		for (int j=0; j<NUMINPUTS; j++)
		{
			if(bufferok)
			{
				
				if (audioInputs[j]->lock == false)
				{
					//if(j==0)
					//buffer[j][audioInputs[j]->cursor] = 0.5*(inputs[j][i]+inputs[j+1][i]);
					//outputs[j][i] = 0;
					//else
					buffer[j][audioInputs[j]->cursor] = inputs[j][i];
			
					if (audioInputs[j]->cursor >= bufferSize-1)
					{
						memcpy(audioInputs[j]->buffer,buffer[j],bufferSize*sizeof(float));
						audioInputs[j]->cursor = 0;
					}
					
				}
				if(audioInputs[j]->cursor < bufferSize-1) {
						audioInputs[j]->cursor = (audioInputs[j]->cursor)+1;
					}

			} 
			outputs[j][i] = inputs[j][i];

		}
	}
	
	
	
	
	
	/*for(int i=0; i<sampleFrames;++i){
		for (int j=0; j<NUMINPUTS; j++){
			if(bufferok) {
				if (audioInputs[j]->cursor >= bufferSize-1){
					//if(audioInputs[j]->lock == true)
					//	int tada = i;
					//if (audioInputs[j]->lock == false) {
						memcpy(audioInputs[j]->buffer,buffer[j],bufferSize*sizeof(float));
						audioInputs[j]->cursor = 0;
					//}	
				}
				
				//if (audioInputs[j]->lock == false){
					buffer[j][audioInputs[j]->cursor] = inputs[j][i];
					audioInputs[j]->cursor = (audioInputs[j]->cursor)+1;
				//}
			}
			outputs[j][i] = inputs[j][i];
			//outputs[j][i] = 0;
		}

	}*/
	
	
	if(numPendingEvents > 0)
	{
		for(int i=0;i<numPendingEvents;++i)
			midiEvent[eventNumArray[i]]->deltaFrames -= frames;
	}
}

//----------------------------------------------------------------------------
VstInt32 VstPlugin::processEvents(VstEvents *events)
{
	int i, j, k;
	VstMidiEvent *event;

	//Go through each event in events.
	for(i=0;i<events->numEvents;++i)
	{
		//Only interested in MIDI events.
		if(events->events[i]->type == kVstMidiType)
		{
			event = (VstMidiEvent *)events->events[i];
			j = -1;

			//Find a space for it in the midiEvent queue.
			for(k=1;k<maxNumEvents;++k)
			{
				if(midiEvent[k]->deltaFrames == -99)
				{
					eventNumArray[numPendingEvents] = k;
					++numPendingEvents;

					j = k;
					break;
				}
			}
			//Add it to the queue if there's still room.
			if((j > 0)&&(numEvents < maxNumEvents))
			{
				numEvents++;
				midiEvent[j]->midiData[0] =		event->midiData[0];
				midiEvent[j]->midiData[1] =		event->midiData[1];
				midiEvent[j]->midiData[2] =		event->midiData[2];
				midiEvent[j]->midiData[3] =		event->midiData[3];
				midiEvent[j]->type =			event->type;
				midiEvent[j]->byteSize =		event->byteSize;
				midiEvent[j]->deltaFrames =		event->deltaFrames;
				midiEvent[j]->flags =			event->flags;
				midiEvent[j]->noteLength =		event->noteLength;
				midiEvent[j]->noteOffset =		event->noteOffset;
				midiEvent[j]->detune =			event->detune;
				midiEvent[j]->noteOffVelocity = event->noteOffVelocity;
				midiEvent[j]->reserved1 =		99;
				midiEvent[j]->reserved2 =		event->reserved2;
			}
		}
	}

	return 1;
}

//----------------------------------------------------------------------------
void VstPlugin::resume()
{
	//Let old hosts know we want to receive MIDI events.
    AudioEffectX::resume();
}

//----------------------------------------------------------------------------
void VstPlugin::suspend()
{
    
}

//----------------------------------------------------------------------------
void VstPlugin::setProgram(VstInt32 program)
{
	//int i;

	curProgram = program;

	//for(i=0;i<numParameters;++i)
	//	setParameter(i, programs[curProgram].parameters[i]);*/


	((GuiEditor*)editor)->updateGuiProg(&(bank.programs[curProgram]));

if (editor)	
	((GuiEditor*)editor)->updateDisplay();

	updateBufferSize(bank.programs[curProgram].iBufferSize);
	((GuiEditor*)editor)->updateAudioInput();


}

//----------------------------------------------------------------------------
void VstPlugin::setProgramName(char *name)
{
	bank.programs[curProgram].name = name;
}


//----------------------------------------------------------------------------
void VstPlugin::getProgramName(char *name)
{
	strcpy(name, bank.programs[curProgram].name.c_str());
}

//----------------------------------------------------------------------------
bool VstPlugin::getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text)
{
	bool retval = false;

    if(index < numPrograms)
    {
		strcpy(text, bank.programs[index].name.c_str());
		retval = true;
    }

	return retval;
}

//----------------------------------------------------------------------------
bool VstPlugin::copyProgram(VstInt32 destination)
{
	bool retval = false;

    if(destination < numPrograms)
    {
		bank.programs[destination] = bank.programs[curProgram];
        retval = true;
    }

    return retval;
}

//----------------------------------------------------------------------------
void VstPlugin::setParameter(VstInt32 index, float value)
{
	parameters[index] = value;
	bank.programs[curProgram].parameters[index] = parameters[index];


	if (editor)
		((AEffGUIEditor*)editor)->setParameter (index, value);
}

//----------------------------------------------------------------------------
float VstPlugin::getParameter(VstInt32 index)
{
	return parameters[index];
}

//----------------------------------------------------------------------------
void VstPlugin::getParameterLabel(VstInt32 index, char *label)
{
	strcpy(label, " ");

	/*switch (index) {
		case kBufferSize : strcpy(label, "choice"); break;
	}*/

}

//----------------------------------------------------------------------------
void VstPlugin::getParameterDisplay(VstInt32 index, char *text)
{
	float2string(parameters[index], text, 7);

	/*switch (index) {
		case kBufferSize : int2string(parameters[kBufferSize]*4096, text, 5); break;
	}*/
}

//----------------------------------------------------------------------------
void VstPlugin::getParameterName(VstInt32 index, char *label)
{
	char tempch[8];

	sprintf(tempch, "p%d", static_cast<int>(index));
	strcpy(label, tempch);

	/*switch (index) {
		case kBufferSize : strcpy(label, "Buffer Size"); break;
	}*/


}

//----------------------------------------------------------------------------
VstInt32 VstPlugin::getChunk (void** data, bool isPreset){

	

	if (isPreset) {
        *data = &(bank.programs[curProgram]);
        return (sizeof(PluginProgram));
    } else {
        *data = &(bank);
        return (sizeof(PluginProgramBank));
    } 



	  //  *data = &(((GuiEditor*)editor)->myProg);
      //  return (sizeof(PlugProgram)); 

	//return 0;

}

//----------------------------------------------------------------------------
VstInt32 VstPlugin::setChunk (void* data, VstInt32 byteSize, bool isPreset){

	if (isPreset)
    {
        //if (byteSize != sizeof(PluginProgram)) return 0;
        bank.programs[curProgram] = *(PluginProgram*)data;

    }
    else
    {
        //if (byteSize != sizeof(PluginProgramBank)) return 0;
        bank = *(PluginProgramBank*)data;
    }

	/*((GuiEditor*)editor)->updateGuiProg(&(bank.programs[curProgram]));

	((GuiEditor*)editor)->updateDisplay();
	updateBufferSize(bank.programs[curProgram].iBufferSize);
	((GuiEditor*)editor)->updateAudioInput();*/
    return 1; 
	
	

		//return 1;

}


//----------------------------------------------------------------------------
VstInt32 VstPlugin::canDo(char *text)
{
	if(!strcmp(text, "sendVstEvents")) return 1;
	if(!strcmp(text, "sendVstMidiEvent")) return 1; //because the plugin acts as a MIDI through
	if(!strcmp(text, "sendVstTimeInfo")) return -1;
	if(!strcmp(text, "receiveVstEvents")) return 1;
	if(!strcmp(text, "receiveVstMidiEvent")) return 1;
	if(!strcmp(text, "receiveVstTimeInfo")) return 1;
	if(!strcmp(text, "offline")) return -1;
	if(!strcmp(text, "plugAsChannelInsert")) return -1;
	if(!strcmp(text, "plugAsSend")) return -1;
	if(!strcmp(text, "mixDryWet")) return -1;
	if(!strcmp(text, "noRealTime")) return -1;
	if(!strcmp(text, "multipass")) return -1;
	if(!strcmp(text, "metapass")) return -1;
	if(!strcmp(text, "1in1out")) return -1;
	if(!strcmp(text, "1in2out")) return -1;
	if(!strcmp(text, "2in1out")) return -1;
	if(!strcmp(text, "2in2out")) return -1;
	if(!strcmp(text, "2in4out")) return -1;
	if(!strcmp(text, "4in2out")) return -1;
	if(!strcmp(text, "4in4out")) return -1;
	if(!strcmp(text, "4in8out")) return -1;					// 4:2 matrix to surround bus
	if(!strcmp(text, "8in4out")) return -1;					// surround bus to 4:2 matrix
	if(!strcmp(text, "8in8out")) return -1;
	if(!strcmp(text, "midiProgramNames")) return -1;
	if(!strcmp(text, "conformsToWindowRules") ) return -1;	// mac: doesn't mess with grafport. general: may want
															// to call sizeWindow (). if you want to use sizeWindow (),
															// you must return true (1) in canDo ("conformsToWindowRules")
	if(!strcmp(text, "bypass")) return -1;

	return -1;
}

//----------------------------------------------------------------------------
float VstPlugin::getVu()
{
	return 0.0f;
}

//----------------------------------------------------------------------------
bool VstPlugin::getEffectName(char* name)
{
    strcpy(name, effectName.c_str());

    return true;
}

//----------------------------------------------------------------------------
bool VstPlugin::getVendorString(char* text)
{
    strcpy(text, vendorName.c_str());

    return true;
}

//----------------------------------------------------------------------------
bool VstPlugin::getProductString(char* text)
{
    strcpy(text, effectName.c_str());

    return true;
}

//----------------------------------------------------------------------------
VstInt32 VstPlugin::getVendorVersion()
{
    return versionNumber;
}

//----------------------------------------------------------------------------
VstPlugCategory VstPlugin::getPlugCategory()
{
    return(kPlugCategEffect);
}

//----------------------------------------------------------------------------
bool VstPlugin::getInputProperties(VstInt32 index, VstPinProperties* properties)
{
	bool retval = false;

	if(index == 0)
	{
		sprintf(properties->label, "%s Left Input 1", effectName.c_str());
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		retval = true;
	}
	else if(index == 1)
	{
		sprintf(properties->label, "%s Right Input 1", effectName.c_str());
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		retval = true;
	}

	return retval;
}

//----------------------------------------------------------------------------
bool VstPlugin::getOutputProperties(VstInt32 index, VstPinProperties* properties)
{
	bool retval = false;

	if(index == 0)
	{
		sprintf(properties->label, "%s Left Output 1", effectName.c_str());
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		retval = true;
	}
	else if(index == 1)
	{
		sprintf(properties->label, "%s Right Output 1", effectName.c_str());
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		retval = true;
	}

	return retval;
}

//----------------------------------------------------------------------------
VstInt32 VstPlugin::getGetTailSize()
{
	return 1; //1=no tail, 0=don't know, everything else=tail size
}

//----------------------------------------------------------------------------
void VstPlugin::processMIDI(VstInt32 pos)
{
	int data1, data2;
	int status, ch, delta;
	int note;
	int i, j;

	for(i=0;i<numPendingEvents;++i)
	{
		if((midiEvent[eventNumArray[i]]->deltaFrames%frames) == pos)
		{
			//--pass on/act on event--
			delta = 0; //because we're at pos frames into the buffer...
			ch = (midiEvent[eventNumArray[i]]->midiData[0] & 0x0F);
			status = (midiEvent[eventNumArray[i]]->midiData[0] & 0xF0);
			data1 = (midiEvent[eventNumArray[i]]->midiData[1] & 0x7F);
			data2 = (midiEvent[eventNumArray[i]]->midiData[2] & 0x7F);

			note = data1;

			switch(status)
			{
				case 0x90:
					if(data2 > 0)
						MIDI_NoteOn(ch, data1, data2, delta);
					else
						MIDI_NoteOff(ch, data1, data2, delta);
					break;
				case 0x80:
					MIDI_NoteOff(ch, data1, data2, delta);
					break;
				case 0xA0:
					MIDI_PolyAftertouch(ch, data1, data2, delta);
					break;
				case 0xB0:
					MIDI_CC(ch, data1, data2, delta);
					break;
				case 0xC0:
					MIDI_ProgramChange(ch, data1, delta);
					break;
				case 0xD0:
					MIDI_ChannelAftertouch(ch, data1, delta);
					break;
				case 0xE0:
					MIDI_PitchBend(ch, data1, data2, delta);
					break;
			}
			midiEvent[eventNumArray[i]]->deltaFrames = -99;
			--numEvents;

			//--reset EventNumArray--
			for(j=(i+1);j<numPendingEvents;++j)
				eventNumArray[(j-1)] = eventNumArray[j];
			--numPendingEvents;
			//break;
		}
	}
}

//----------------------------------------------------------------------------
void VstPlugin::MIDI_NoteOn(int ch, int note, int val, int delta)
{
	midiEvent[0]->midiData[0] = 0x90 + ch;
	midiEvent[0]->midiData[1] = note;
	midiEvent[0]->midiData[2] = val;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
void VstPlugin::MIDI_NoteOff(int ch, int note, int val, int delta)
{
	midiEvent[0]->midiData[0] = 0x80 + ch;
	midiEvent[0]->midiData[1] = note;
	midiEvent[0]->midiData[2] = val;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
void VstPlugin::MIDI_PolyAftertouch(int ch, int note, int val, int delta)
{
	midiEvent[0]->midiData[0] = 0xA0 + ch;
	midiEvent[0]->midiData[1] = note;
	midiEvent[0]->midiData[2] = val;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
void VstPlugin::MIDI_CC(int ch, int num, int val, int delta)
{
	float tempf;

	tempf = static_cast<float>(val)/127.0f;	// CC data

	midiEvent[0]->midiData[0] = 0xB0 + ch;
	midiEvent[0]->midiData[1] = num;
	midiEvent[0]->midiData[2] = val;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
void VstPlugin::MIDI_ProgramChange(int ch, int val, int delta)
{
	if(val < numPrograms)
		setProgram(val);

	midiEvent[0]->midiData[0] = 0xD0 + ch;
	midiEvent[0]->midiData[1] = val;
	midiEvent[0]->midiData[2] = 0;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
void VstPlugin::MIDI_ChannelAftertouch(int ch, int val, int delta)
{
	midiEvent[0]->midiData[0] = 0xD0 + ch;
	midiEvent[0]->midiData[1] = val;
	midiEvent[0]->midiData[2] = 0;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
void VstPlugin::MIDI_PitchBend(int ch, int x1, int x2, int delta)
{
	midiEvent[0]->midiData[0] = 0xE0 + ch;
	midiEvent[0]->midiData[1] = x1;
	midiEvent[0]->midiData[2] = x2;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

static void peak(float in, float sampleRate, float* output)
{

	float input = in;
	float halfLife = 1; //time in seconds for output to decay to half value after an impulse

	float scalar = pow( 0.5f, 1.0f/(halfLife * sampleRate));

	if( input < 0.0 )
		input = -input;  /* Absolute value. */



	if ( input >= *output )
	{
		*output = input;
	}
	else
	{
		*output = *output * scalar;
		if( *output < 0.00001 ) *output = 0.0;
	}
//	return *output;

}