//-----------------------------------------------------------------------------
// Project     : NANOSYNTH MIDI
// Version     : 3.6.6
// Created by  : Will Pirkle
// Description : NanoSynth MIDI only
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2013, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "VSTSynthProcessor.h"
#include "VSTSynthController.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/base/ftypes.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/base/futils.h"
#include "logscale.h"
#include "SynthParamLimits.h"

// --- Synth Stuff
#define SYNTH_PROC_BLOCKSIZE 32 // 32 samples per processing block = 0.7 mSec = OK for tactile response WP

#define LOG_MIDI 1

// --- VST2 wrapper
::AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return Steinberg::Vst::Vst2Wrapper::create(GetPluginFactory(),	/* calls factory.cpp macro */
		Steinberg::Vst::NanoSynth::Processor::cid,	/* proc CID */
		'NaN0',	/* 10 dig code for NanoSynth */
		audioMaster);
}

namespace Steinberg {
namespace Vst {
namespace NanoSynth {

// --- for versioning in serialization
static uint64 NanoSynthVersion = 0;

// --- the unique identifier (use guidgen.exe to generate)
FUID Processor::cid (0xD57A3223, 0x18A14663, 0xB765124B, 0x38B37056);

// this defines a logarithmig scaling for the filter Fc control
LogScale<ParamValue> filterLogScale2 (0.0,		/* VST GUI Variable MIN */
									 1.0,		/* VST GUI Variable MAX */
									 80.0,		/* filter fc LOW */
									 18000.0,	/* filter fc HIGH */
									 0.5,		/* the value at position 0.5 will be: */
									 1800.0);	/* 1800 Hz */
Processor::Processor()
{
	// --- we are a Processor
	setControllerClass(Controller::cid);

	// --- our inits
	//
	// Finish initializations here
	m_dLastNoteFrequency = 0.0;

	// sus pedal support
	m_bSustainPedal = false;

	// receive on all channels
	m_uMidiRxChannel = MIDI_CH_ALL;

	// VST3 specific
	m_dMIDIPitchBend = DEFAULT_MIDI_PITCHBEND; // -1 to +1
	m_uMIDIModWheel = DEFAULT_MIDI_MODWHEEL; 
	m_uMIDIVolumeCC7 = DEFAULT_MIDI_VOLUME;  // note defaults to 127
	m_uMIDIPanCC10 = DEFAULT_MIDI_PAN;     // 64 = center pan
	m_uMIDIExpressionCC11 = DEFAULT_MIDI_EXPRESSION; 
}

// --- destructor
Processor::~Processor()
{

}

/* 
	Processor::initialize()
	Call the base class
	Add a Stereo Audio Output
	Add one Event Bus with 16 Channels
*/
tresult PLUGIN_API Processor::initialize(FUnknown* context)
{
	tresult result = AudioEffect::initialize(context);
	if (result == kResultTrue)
	{
		// stereo output bus
		addAudioOutput(STR16 ("Audio Output"), SpeakerArr::kStereo);

		// MIDI event input bus, 16 channels
		addEventInput(STR16 ("Event Input"), 16);
	}

	return result;
}
/* 
	Processor::setState()
	This is the READ part of the serialization process. We get the stream interface and use it
	to read from the filestream.

	NOTE: The datatypes/read order must EXACTLY match the getState() version or crashes may happen or variables
	      not initialized properly.
*/
tresult PLUGIN_API Processor::setState(IBStream* fileStream)
{
	IBStreamer s(fileStream, kLittleEndian);
	uint64 version = 0;

	// --- needed to convert to our UINT reads
	uint32 udata = 0;
	int32 data = 0;

	// --- read the version
	if(!s.readInt64u(version)) return kResultFalse;

	// --- do next version...
	if (version >= 1)
	{
		// add v1 stuff here
	}
	return kResultTrue;
}

/* 
	Processor::getState()
	This is the WRITE part of the serialization process. We get the stream interface and use it
	to write to the filestream. This is important because it is how the Factory Default is set
	at startup, as well as when writing presets.
*/
tresult PLUGIN_API Processor::getState(IBStream* fileStream)
{
	// --- get a stream I/F
	IBStreamer s(fileStream, kLittleEndian);

	// --- NanoSynthVersion - place this at top so versioning can be used during the READ operation
	if(!s.writeInt64u(NanoSynthVersion)) return kResultFalse;

	return kResultTrue;
}

/* 
	Processor::setBusArrangements()
	Client queries us for our supported Busses; this is where you can modify to support mono, surround, etc...
*/
tresult PLUGIN_API Processor::setBusArrangements(SpeakerArrangement* inputs, int32 numIns, 
												   SpeakerArrangement* outputs, int32 numOuts)
{
	// we only support one stereo output bus
	if (numIns == 0 && numOuts == 1 && outputs[0] == SpeakerArr::kStereo)
	{
		return AudioEffect::setBusArrangements (inputs, numIns, outputs, numOuts);
	}
	return kResultFalse;
}

/* 
	Processor::canProcessSampleSize()
	Client queries us for our supported sample lengths
*/
tresult PLUGIN_API Processor::canProcessSampleSize(int32 symbolicSampleSize)
{
	// this is a challenge in the book; here is where you say you support it but
	// you will need to deal with different buffers in the process() method
//	if (symbolicSampleSize == kSample32 || symbolicSampleSize == kSample64)

	// --- currently 32 bit only
	if (symbolicSampleSize == kSample32)
	{
		return kResultTrue;
	}
	return kResultFalse;
}

/* 
	Processor::setActive()
	This is the analog of prepareForPlay() in RAFX since the Sample Rate is now set.

	VST3 plugins may be turned on or off; you are supposed to dynamically delare stuff when activated
	then delete when de-activated. So, we handle our voice stack here. 
*/
tresult PLUGIN_API Processor::setActive(TBool state)
{
	if(state)
	{
		// --- do ON stuff
	}
	else
	{
		// --- do OFF stuff
	}

	// --- base class method call is last
	return AudioEffect::setActive (state);
}

/* 
	Processor::update()
	Our own function to update the voice(s) of the synth with UI Changes.
*/
void Processor::update()
{

}

/* 
	Processor::doControlUpdate()
	Find and issue Control Changes (same as userInterfaceChange() in RAFX)
	returns true if a control was changed
*/
bool Processor::doControlUpdate(ProcessData& data)
{
	bool paramChange = false;

	// --- check
	if(!data.inputParameterChanges)
		return paramChange;

	// --- get the param count and setup a loop for processing queue data
	int32 count = data.inputParameterChanges->getParameterCount();
	
	// --- make sure there is something there
	if(count <= 0)
		return paramChange;

	// --- loop
	for(int32 i=0; i<count; i++)
	{
		// get the message queue for ith parameter
		IParamValueQueue* queue = data.inputParameterChanges->getParameterData(i);

		if(queue)
		{
			// --- check for control points
			if(queue->getPointCount() <= 0) return false;

			int32 sampleOffset = 0.0;
			ParamValue value = 0.0;
			ParamID pid = queue->getParameterId();

			// this is the same as userInterfaceChange(); these only are updated if a change has 
			// occurred (a control got moved)
			//
			// NOTE: same as with VST sample synth, we are taking the last value in the queue: queue->getPointCount()-1
			//       the value parameter is [0..1] so MUST BE COOKED before using
			//
			// NOTE: These are NOT MIDI Events! You can't get the channel directly
			// TODO: maybe try to make this nearly sample accurate <- from Steinberg...

			// --- get the last point in queue
			if(queue->getPoint(queue->getPointCount()-1, /* last update point */
				sampleOffset,			/* sample offset */
				value) == kResultTrue)	/* value = [0..1] */
			{
				// --- at least one param changed
				paramChange = true;

				switch(pid) // same as RAFX uControlID
				{
					// --- MIDI messages
					case MIDI_PITCHBEND: // want -1 to +1
					{
						m_dMIDIPitchBend = unipolarToBipolar(value);
						#if(LOG_MIDI && _DEBUG)
							FDebugPrint("-- Pitch Bend: %f\n", m_dMIDIPitchBend);
						#endif
						break;
					}		
					case MIDI_MODWHEEL: // want 0 to 127
					{
						m_uMIDIModWheel = unipolarToMIDI(value);
						#if(LOG_MIDI && _DEBUG)
							FDebugPrint("-- Mod Wheel: %d\n", m_uMIDIModWheel);
						#endif
						break;
					}		
					case MIDI_VOLUME_CC7: // want 0 to 127
					{
						m_uMIDIVolumeCC7 = unipolarToMIDI(value);
						#if(LOG_MIDI && _DEBUG)
							FDebugPrint("-- Volume: %d\n", m_uMIDIVolumeCC7);
						#endif
						break;
					}		
					case MIDI_PAN_CC10: // want 0 to 127
					{
						m_uMIDIPanCC10 = unipolarToMIDI(value);
						#if(LOG_MIDI && _DEBUG)
							FDebugPrint("-- Pan: %d\n", m_uMIDIPanCC10);
						#endif
						break;
					}		
					case MIDI_EXPRESSION_CC11: // want 0 to 127
					{
						m_uMIDIExpressionCC11 = unipolarToMIDI(value);
						#if(LOG_MIDI && _DEBUG)
							FDebugPrint("-- Expression: %d\n", m_uMIDIExpressionCC11);
						#endif
						break;
					}		
					case MIDI_CHANNEL_PRESSURE: 
					{
						#if(LOG_MIDI && _DEBUG)
							FDebugPrint("-- Channel Pressure: %f\n", value);
						#endif
						break;
					}		
					case MIDI_SUSTAIN_PEDAL: // want 0 to 1
					{
						m_bSustainPedal = value > 0.5 ? true : false;
						#if(LOG_MIDI && _DEBUG)
						if(m_bSustainPedal)
							FDebugPrint("-- Sustain Pedal ON\n");
						else
							FDebugPrint("-- Sustain Pedal OFF\n");
						#endif
						break;
					}		
					case MIDI_ALL_NOTES_OFF: 
					{
						#if(LOG_MIDI && _DEBUG)
							FDebugPrint("-- All Notes OFF\n");
						#endif
						break;
					}		
				}	
			}
		}
	}

	// --- check and update
	// if(paramChange)
	//	update();

	return paramChange;
}

bool Processor::doProcessEvent(Event& vstEvent)
{
	bool noteEvent = false;
	
	// --- process Note On or Note Off messages here
	switch(vstEvent.type)
	{
		// --- NOTE ON
		case Event::kNoteOnEvent:
		{
			// --- get the channel/note/vel
			UINT uMIDIChannel = (UINT)vstEvent.noteOn.channel;
			UINT uMIDINote = (UINT)vstEvent.noteOn.pitch;
			UINT uMIDIVelocity = (UINT)(127.0*vstEvent.noteOn.velocity);

			// --- test channel/ignore
			if(m_uMidiRxChannel != MIDI_CH_ALL && uMIDIChannel != m_uMidiRxChannel)
				return false;
			
			// --- event occurred
			noteEvent = true;

			// --- fix noteID as per SDK
			if(vstEvent.noteOn.noteId == -1)
				vstEvent.noteOn.noteId = uMIDINote;

			#if(LOG_MIDI && _DEBUG)
				FDebugPrint("-- Note On Ch:%d Note:%d Vel:%d \n", uMIDIChannel, uMIDINote, uMIDIVelocity);
			#endif

			break;
		}

		// --- NOTE OFF
		case Event::kNoteOffEvent:
		{
			// --- get the channel/note/vel
			UINT uMIDIChannel = (UINT)vstEvent.noteOff.channel;
			UINT uMIDINote = (UINT)vstEvent.noteOff.pitch;
			UINT uMIDIVelocity = (UINT)(127.0*vstEvent.noteOff.velocity); // not used

			// --- test channel/ignore
			if(m_uMidiRxChannel != MIDI_CH_ALL && uMIDIChannel != m_uMidiRxChannel)
				return false;
			
			// --- event occurred
			noteEvent = true;

			// --- fix noteID as per SDK
			if(vstEvent.noteOff.noteId == -1)
				vstEvent.noteOff.noteId = uMIDINote;

			#if(LOG_MIDI && _DEBUG)
				FDebugPrint("-- Note Off Ch:%d Note:%d Vel:%d \n", uMIDIChannel, uMIDINote, uMIDIVelocity);
			#endif

			break;
		}

		// --- polyphonic aftertouch 0xAn
		case Event::kPolyPressureEvent:
		{			
			// --- get the channel
			UINT uMIDIChannel = (UINT)vstEvent.polyPressure.channel;
			UINT uMIDINote = (UINT)vstEvent.polyPressure.pitch;
			float fPressure = vstEvent.polyPressure.pressure;

			// --- test channel/ignore
			if(m_uMidiRxChannel != MIDI_CH_ALL && uMIDIChannel != m_uMidiRxChannel)
				return false;
			
			// --- note event did not occurr
			noteEvent = false;
	
			#if(LOG_MIDI && _DEBUG)
				FDebugPrint("-- Poly Pressure Ch:%d Note:%d Value:%d \n", uMIDIChannel, uMIDINote, fPressure);
			#endif

			break;
		}
	}

	// -- note event occurred?
	return noteEvent;
}

/* 
	Processor::process()
	The most important function handles:
		Control Changes (same as userInterfaceChange() in RAFX)
		Synth voice rendering
		Output GUI Changes (allows you to write back to the GUI, advanced.
*/
tresult PLUGIN_API Processor::process(ProcessData& data)
{
	// --- check for presence of output buffers
	if (!data.outputs)
		return kResultTrue;

	// --- check for conrol chages and update synth if needed
	doControlUpdate(data);

	// flush mode
	if (data.numOutputs < 1)
		return kResultTrue;
	else
	{
		// --- we process 32 samples at a time; MIDI events are then accurate to 0.7 mSec
		const int32 kBlockSize = SYNTH_PROC_BLOCKSIZE;
		
		// 32-bit is float
		// if doing a 64-bit version, you need to replace with double*
		// initialize audio output buffers
		float* buffers[OUTPUT_CHANNELS]; // Precision is float - need to change this do DOUBLE if supporting 64 bit

		// 32-bit is float
		// if doing a 64-bit version, you need to replace with double* here too
		for(int i = 0; i < OUTPUT_CHANNELS; i++)
		{
			// data.outputs[0] = BUS 0
			buffers[i] = (float*)data.outputs[0].channelBuffers32[i];
			memset (buffers[i], 0, data.numSamples * sizeof(float));
		}

		// --- total number of samples in the input Buffer
		int32 numSamples = data.numSamples;

		// --- this is used when we need to shove an event into the next block 
		int32 samplesProcessed = 0;

		// --- get our list of events
		IEventList* inputEvents = data.inputEvents;
		Event e = {0};
		Event* eventPtr = 0;
		int32 eventIndex = 0;

		// --- count of events
		int32 numEvents = inputEvents ? inputEvents->getEventCount () : 0;
		
		// get the first event
		if(numEvents)
		{
			inputEvents->getEvent (0, e);
			eventPtr = &e;
		}
		
		while(numSamples > 0)
		{
			// bound the samples to process to BLOCK SIZE (32)
			int32 samplesToProcess = std::min<int32> (kBlockSize, numSamples);
			
			while(eventPtr != 0)
			{
				// --- if the event is not in the current processing block 
				//     then adapt offset for next block
				if (e.sampleOffset > samplesToProcess)
				{
					e.sampleOffset -= samplesToProcess;
					break;
				}

				// --- find MIDI note-on/off and broadcast
				doProcessEvent(e);

				// --- get next event
				eventIndex++;
				if (eventIndex < numEvents)
				{
					if (inputEvents->getEvent (eventIndex, e) == kResultTrue)
					{
						e.sampleOffset -= samplesProcessed;
					}
					else
					{
						eventPtr = 0;
					}
				}
				else
				{
					eventPtr = 0;
				}
			}	

			// the loop - samplesToProcess is more like framesToProcess
			for(int32 j=0; j<samplesToProcess; j++)
			{
				// just clear buffers
				buffers[0][j] = 0.0;	// left
				buffers[1][j] = 0.0;	// right
			}

			// --- update the counter
			for(int i = 0; i < OUTPUT_CHANNELS; i++) 
				buffers[i] += samplesToProcess;

			// --- update the samples processed/to process
			numSamples -= samplesToProcess;
			samplesProcessed += samplesToProcess;

		} // end while (numSamples > 0)
	}

	// can write OUT to the GUI like this:
	if (data.outputParameterChanges)
	{

	}
	// --- set silence flags if no notes playing
	if(data.numOutputs > 0)
	{
		data.outputs[0].silenceFlags = 0x11; // left and right channel are silent
	}

	return kResultTrue;
}

}}} // namespaces
