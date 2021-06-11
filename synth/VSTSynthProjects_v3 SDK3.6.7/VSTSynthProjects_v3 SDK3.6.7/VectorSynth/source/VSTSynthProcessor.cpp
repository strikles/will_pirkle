//-----------------------------------------------------------------------------
// Project     : VECTOR SYNTH
// Version     : 3.6.6
// Created by  : Will Pirkle
// Description : Vector-Synth
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
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/base/futils.h"
#include "synthparamlimits.h" // param limits file
#include "logscale.h"

// --- Synth Stuff
#include <algorithm>
#include "pluginconstants.h"
#define SYNTH_PROC_BLOCKSIZE 32 // 32 samples per processing block = 0.7 mSec = OK for tactile response WP

// --- MIDI Logging -- comment out to disable
#define LOG_MIDI 1

// --- VST2 wrapper
::AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return Steinberg::Vst::Vst2Wrapper::create(GetPluginFactory(),	/* calls factory.cpp macro */
		Steinberg::Vst::VectorSynth::Processor::cid,	/* proc CID */
		'VeC0',	/* 10 dig code for VectorSynth */
		audioMaster);
}

namespace Steinberg {
namespace Vst {
namespace VectorSynth {

// --- for versioning in serialization
static uint64 VectorSynthVersion = 0;

// --- the unique identifier (use guidgen.exe to generate)
FUID Processor::cid (0x3FDF9942, 0xF6734013, 0xB88CF87A, 0xF88E6080);

// this defines a logarithmig scaling for the filter Fc control
LogScale<ParamValue> filterLogScale2 (0.0,		/* VST GUI Variable MIN */
									 1.0,		/* VST GUI Variable MAX */
									 80.0,		/* filter fc LOW */
									 18000.0,	/* filter fc HIGH */
									 0.5,		/* the value at position 0.5 will be: */
									 1800.0);	/* 1800 Hz */

Processor::Processor ()
{
	// --- we are a Processor
	setControllerClass(Controller::cid);

	// --- our inits
	m_dOsc1Amplitude_dB = DEFAULT_OUTPUT_AMPLITUDE_DB;
	m_dOsc2Amplitude_dB = DEFAULT_OUTPUT_AMPLITUDE_DB;
	m_dEG1OscIntensity = DEFAULT_BIPOLAR;

	m_dFcControl = DEFAULT_FILTER_FC;
	m_dQControl = DEFAULT_FILTER_Q;
	m_dEG1FilterIntensity = DEFAULT_BIPOLAR;
	m_dFilterKeyTrackIntensity = DEFAULT_FILTER_KEYTRACK_INTENSITY;

	m_dAttackTime_mSec = DEFAULT_EG_ATTACK_TIME;
	m_dDecayReleaseTime_mSec = DEFAULT_EG_DECAYRELEASE_TIME;
	m_dSustainLevel = DEFAULT_EG_SUSTAIN_LEVEL;

	m_uLFO1Waveform = DEFAULT_LFO_WAVEFORM;
	m_dLFO1Rate = DEFAULT_LFO_RATE;
	m_dLFO1Amplitude = DEFAULT_UNIPOLAR;

	m_dLFO1FilterFcIntensity = DEFAULT_BIPOLAR;
	m_dLFO1OscPitchIntensity = DEFAULT_BIPOLAR;
	m_dLFO1AmpIntensity = DEFAULT_UNIPOLAR;
	m_dLFO1PanIntensity = DEFAULT_UNIPOLAR;

	m_dVolume_dB = DEFAULT_OUTPUT_AMPLITUDE_DB;
	m_dEG1DCAIntensity = MAX_BIPOLAR; // note! 

	m_uVoiceMode = DEFAULT_VOICE_MODE;
	m_dDetune_cents = DEFAULT_DETUNE_CENTS;
	m_dPortamentoTime_mSec = DEFAULT_PORTAMENTO_TIME_MSEC;
	m_nOctave = DEFAULT_OCTAVE;
	
	m_uVelocityToAttackScaling = DEFAULT_VELOCITY_TO_ATTACK;
	m_uNoteNumberToDecayScaling = DEFAULT_NOTE_TO_DECAY;
	m_uResetToZero = DEFAULT_RESET_TO_ZERO;
	m_uFilterKeyTrack = DEFAULT_FILTER_KEYTRACK;
	m_uLegatoMode = DEFAULT_LEGATO_MODE;
	m_nPitchBendRange = DEFAULT_PITCHBEND_RANGE;
	m_uLoopMode = DEFAULT_LOOP_MODE;
	
	// vector joystick
	m_dJoystickX = DEFAULT_UNIPOLAR_HALF; // NOTE these are 0->1
	m_dJoystickY = DEFAULT_UNIPOLAR_HALF; // ditto
	m_uVectorPathMode = DEFAULT_PATH_MODE;
	m_dOrbitX = DEFAULT_BIPOLAR;
	m_dOrbitY = DEFAULT_BIPOLAR;
	m_dRotorRate = DEFAULT_SLOW_LFO_RATE;
	m_uRotorWaveform = DEFAULT_LFO_WAVEFORM; 

	// VST3 specific
	m_dMIDIPitchBend = DEFAULT_MIDI_PITCHBEND; // -1 to +1
	m_uMIDIModWheel = DEFAULT_MIDI_MODWHEEL; 
	m_uMIDIVolumeCC7 = DEFAULT_MIDI_VOLUME;  // note defaults to 127
	m_uMIDIPanCC10 = DEFAULT_MIDI_PAN;     // 64 = center pan
	m_uMIDIExpressionCC11 = DEFAULT_MIDI_EXPRESSION; 

	// Finish initializations here
	m_dLastNoteFrequency = 0.0;

	// sus pedal support
	m_bSustainPedal = false;

	// receive on all channels
	m_uMidiRxChannel = MIDI_CH_ALL;
	
	// this is created/d3estroyed in SetActive()
	m_pFilterLogParam = NULL;

	// load up voices
	for(int i=0; i<MAX_VOICES; i++)
	{
		// --- create voice
		m_pVoiceArray[i] = new CVectorSynthVoice;

		// --- global params (MUST BE DONE before setting up mod matrix!)
		m_pVoiceArray[i]->initGlobalParameters(&m_GlobalSynthParams);
	}
			
	// --- use the first voice to setup the MmM
	m_pVoiceArray[0]->initializeModMatrix(&m_GlobalModMatrix);

	// --- then set the mod matrix cores on the rest of the voices
	for(int i=0; i<MAX_VOICES; i++)
	{
		// --- all matrices share a common core array of matrix rows
		if(m_pVoiceArray[i])
			m_pVoiceArray[i]->setModMatrixCore(m_GlobalModMatrix.getModMatrixCore());		
	}
		
	// --- helper
	m_pFilterLogParam = new LogScaleParameter<ParamValue> (USTRING("Filter fc"), 
														    FILTER_FC, 
														    filterLogScale2, 
														    (USTRING("Hz")));

	// --- now load samples
	loadSamples();
}

// --- destructor
Processor::~Processor()
{
	// --- delete on master ONLY
	m_GlobalModMatrix.deleteModMatrix();

	// --- delete voices
	for(int i=0; i<MAX_VOICES; i++)
	{
		if(m_pVoiceArray[i])
		{
			delete m_pVoiceArray[i];
			m_pVoiceArray[i] = NULL;
		}
	}
			
	if(m_pFilterLogParam)
	{
		delete m_pFilterLogParam;
		m_pFilterLogParam = NULL;
	}
}

/* 
	Processor::initialize()
	Call the base class
	Add a Stereo Audio Output
	Add a MIDI event inputs (16: one for each channel)
*/
tresult PLUGIN_API Processor::initialize(FUnknown* context)
{
	tresult result = AudioEffect::initialize(context);
	if (result == kResultTrue)
	{
		// stereo output bus
		addAudioOutput(STR16 ("Audio Output"), SpeakerArr::kStereo);

		// MIDI event input bus, one channel
		addEventInput(STR16 ("Event Input"), 16);
	}

	return result;
}

/* 
	Processor::getState()
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

	// read the version
	if(!s.readInt64u(version)) return kResultFalse;

	// --- then read Version 0 params
	if(!s.readDouble(m_dOsc1Amplitude_dB)) return kResultFalse;
	if(!s.readDouble(m_dOsc2Amplitude_dB)) return kResultFalse;
	if(!s.readDouble(m_dEG1OscIntensity)) return kResultFalse;

	if(!s.readDouble(m_dFcControl)) return kResultFalse;
	if(!s.readDouble(m_dQControl)) return kResultFalse;
	if(!s.readDouble(m_dEG1FilterIntensity)) return kResultFalse;
	if(!s.readDouble(m_dFilterKeyTrackIntensity)) return kResultFalse;

	if(!s.readDouble(m_dAttackTime_mSec)) return kResultFalse;	
	if(!s.readDouble(m_dDecayReleaseTime_mSec)) return kResultFalse;
	if(!s.readDouble(m_dSustainLevel)) return kResultFalse;

	if(!s.readInt32u(udata)) return kResultFalse; else m_uLFO1Waveform = udata;
	if(!s.readDouble(m_dLFO1Rate)) return kResultFalse;
	if(!s.readDouble(m_dLFO1Amplitude)) return kResultFalse;

	if(!s.readDouble(m_dLFO1FilterFcIntensity)) return kResultFalse;
	if(!s.readDouble(m_dLFO1OscPitchIntensity)) return kResultFalse;
	if(!s.readDouble(m_dLFO1AmpIntensity)) return kResultFalse;
	if(!s.readDouble(m_dLFO1PanIntensity)) return kResultFalse;

	if(!s.readDouble(m_dVolume_dB)) return kResultFalse;
	if(!s.readDouble(m_dEG1DCAIntensity)) return kResultFalse;

	if(!s.readInt32u(udata)) return kResultFalse; else m_uVoiceMode = udata;
	if(!s.readDouble(m_dDetune_cents)) return kResultFalse;
	if(!s.readDouble(m_dPortamentoTime_mSec)) return kResultFalse;
	if(!s.readInt32(data)) return kResultFalse; else m_nOctave = data;

	if(!s.readInt32u(udata)) return kResultFalse; else m_uVelocityToAttackScaling = udata;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uNoteNumberToDecayScaling = udata;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uResetToZero = udata;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uFilterKeyTrack = udata;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uLegatoMode = udata;
	if(!s.readInt32(data)) return kResultFalse; else m_nPitchBendRange = data;
	
	// digisynth stuff
	if(!s.readInt32u(udata)) return kResultFalse; else m_uLoopMode = udata;

	// vector synth stuff
	if(!s.readDouble(m_dJoystickX)) return kResultFalse;
	if(!s.readDouble(m_dJoystickY)) return kResultFalse;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uVectorPathMode = udata;
	if(!s.readDouble(m_dOrbitX)) return kResultFalse;
	if(!s.readDouble(m_dOrbitY)) return kResultFalse;
	if(!s.readDouble(m_dRotorRate)) return kResultFalse;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uRotorWaveform = udata;

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
	// get a stream I/F
	IBStreamer s(fileStream, kLittleEndian);

	// --- VectorSynthVersion - place this at top so versioning can be used during the READ operation
	if(!s.writeInt64u(VectorSynthVersion)) return kResultFalse;

	// --- these follow the same order as the enum for the index values (they don't nec have to)
	if(!s.writeDouble(m_dOsc1Amplitude_dB)) return kResultFalse;
	if(!s.writeDouble(m_dOsc2Amplitude_dB)) return kResultFalse;
	if(!s.writeDouble(m_dEG1OscIntensity)) return kResultFalse;

	if(!s.writeDouble(m_dFcControl)) return kResultFalse;
	if(!s.writeDouble(m_dQControl)) return kResultFalse;
	if(!s.writeDouble(m_dEG1FilterIntensity)) return kResultFalse;
	if(!s.writeDouble(m_dFilterKeyTrackIntensity)) return kResultFalse;

	if(!s.writeDouble(m_dAttackTime_mSec)) return kResultFalse;	
	if(!s.writeDouble(m_dDecayReleaseTime_mSec)) return kResultFalse;
	if(!s.writeDouble(m_dSustainLevel)) return kResultFalse;

	if(!s.writeInt32u(m_uLFO1Waveform)) return kResultFalse;
	if(!s.writeDouble(m_dLFO1Rate)) return kResultFalse;
	if(!s.writeDouble(m_dLFO1Amplitude)) return kResultFalse;

	if(!s.writeDouble(m_dLFO1FilterFcIntensity)) return kResultFalse;
	if(!s.writeDouble(m_dLFO1OscPitchIntensity)) return kResultFalse;
	if(!s.writeDouble(m_dLFO1AmpIntensity)) return kResultFalse;
	if(!s.writeDouble(m_dLFO1PanIntensity)) return kResultFalse;

	if(!s.writeDouble(m_dVolume_dB)) return kResultFalse;
	if(!s.writeDouble(m_dEG1DCAIntensity)) return kResultFalse;

	if(!s.writeInt32u(m_uVoiceMode)) return kResultFalse;
	if(!s.writeDouble(m_dDetune_cents)) return kResultFalse;
	if(!s.writeDouble(m_dPortamentoTime_mSec)) return kResultFalse;	
	if(!s.writeInt32(m_nOctave)) return kResultFalse;

	if(!s.writeInt32u(m_uVelocityToAttackScaling)) return kResultFalse;
	if(!s.writeInt32u(m_uNoteNumberToDecayScaling)) return kResultFalse;
	if(!s.writeInt32u(m_uResetToZero)) return kResultFalse;
	if(!s.writeInt32u(m_uFilterKeyTrack)) return kResultFalse;
	if(!s.writeInt32u(m_uLegatoMode)) return kResultFalse;
	if(!s.writeInt32(m_nPitchBendRange)) return kResultFalse;

	// digisynth stuff
	if(!s.writeInt32u(m_uLoopMode)) return kResultFalse;

	// vector synth stuff
	if(!s.writeDouble(m_dJoystickX)) return kResultFalse;
	if(!s.writeDouble(m_dJoystickY)) return kResultFalse;
	if(!s.writeInt32u(m_uVectorPathMode)) return kResultFalse;
	if(!s.writeDouble(m_dOrbitX)) return kResultFalse;
	if(!s.writeDouble(m_dOrbitY)) return kResultFalse;
	if(!s.writeDouble(m_dRotorRate)) return kResultFalse;
	if(!s.writeInt32u(m_uRotorWaveform)) return kResultFalse;

	return kResultTrue;
}

/* 
	Processor::setBusArrangements()
	Client queries us for our supported Busses; this is where you can modify to support mono, surround, etc...
*/
tresult PLUGIN_API Processor::setBusArrangements (SpeakerArrangement* inputs, int32 numIns, 
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


bool Processor::loadSamples()
{
	// get our DLL path with my helper function; remember to delete when done
	char* pDLLPath = getMyDLLDirectory(USTRING("VectorSynth.vst3"));
	char* pPath0 = addStrings(pDLLPath,"\\MultiSamples\\Heaver");
	char* pPath1 = addStrings(pDLLPath,"\\MultiSamples\\OldFlatty");
	char* pPath2 = addStrings(pDLLPath,"\\MultiSamples\\Divider");
	char* pPath3 = addStrings(pDLLPath,"\\MultiSamples\\FuzzVibe");

	// show the wait cursor; this may be a lenghty process but we
	// are frontloading the file reads
	SetCursor(LoadCursor(NULL, IDC_WAIT));
	ShowCursor(true);

	// load 4 sets of multis
	// ------- FOR MULTI SAMPLES ------- //
	// --- to speed up sample loading, init the first voice here
	//     init(...false, false) = NOT single-sample, NOT pitchless sample
	if(!m_pVoiceArray[0]->initOscWithFolderPath(0, pPath0, false, false)) 
		return false;
	if(!m_pVoiceArray[0]->initOscWithFolderPath(1, pPath1, false, false))
		return false;
	if(!m_pVoiceArray[0]->initOscWithFolderPath(2, pPath2, false, false)) 
		return false;
	if(!m_pVoiceArray[0]->initOscWithFolderPath(3, pPath3, false, false)) 
		return false;

	// --- then use the other function to copy the sample pointerrs
	//     so they share pointers to same buffers of data
	for(int i=1; i<MAX_VOICES; i++)
	{
		// --- init false, false = NOT single-sample, 
		//     NOT pitchless sample
		m_pVoiceArray[i]->initAllOscWithDigiSynthVoice(m_pVoiceArray[0], false, false);
	}

	// always delete what comes back from addStrings()
	delete [] pPath0;
	delete [] pPath1;
	delete [] pPath2;
	delete [] pPath3;
	delete [] pDLLPath;

	return true;
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
		// --- reset
		m_dLastNoteFrequency = 0.0;

		// --- set sample rate; it is now valid (could have changed?)
		for(int i=0; i<MAX_VOICES; i++)
		{
			if(m_pVoiceArray[i])
			{
				m_pVoiceArray[i]->setSampleRate((double)processSetup.sampleRate);
				m_pVoiceArray[i]->prepareForPlay();
			}
		}
	
		// --- mass update
		update();
	}
	else
	{
		// --- for any customization you may do
	}

	// base class method call is last
	return AudioEffect::setActive (state);
}


/* 
	Processor::update()
	Our own function to update the voice(s) of the synth with UI Changes.
*/
void Processor::update()
{
	// --- update global parameters
	//
	// --- Voice:
	m_GlobalSynthParams.voiceParams.uVoiceMode = m_uVoiceMode;
	m_GlobalSynthParams.voiceParams.dPortamentoTime_mSec = m_dPortamentoTime_mSec;
	
	// --- VS Specific
	m_GlobalSynthParams.voiceParams.uVectorPathMode = m_uVectorPathMode;
	m_GlobalSynthParams.voiceParams.dOrbitXAmp = m_dOrbitX;
	m_GlobalSynthParams.voiceParams.dOrbitYAmp = m_dOrbitY;

	// --- VST3/AU ONLY ---
	// --- calculate the vector joystick mix values based on x,y location (x,y are 0->1)
	calculateVectorJoystickValues(m_dJoystickX, m_dJoystickY, 
								  m_GlobalSynthParams.voiceParams.dAmplitude_A, 
								  m_GlobalSynthParams.voiceParams.dAmplitude_B, 
								  m_GlobalSynthParams.voiceParams.dAmplitude_C, 
								  m_GlobalSynthParams.voiceParams.dAmplitude_D, 
								  m_GlobalSynthParams.voiceParams.dAmplitude_ACmix, 
								  m_GlobalSynthParams.voiceParams.dAmplitude_BDmix);
	// --- ranges
	m_GlobalSynthParams.voiceParams.dOscFoPitchBendModRange = m_nPitchBendRange;

	// --- intensities	
	m_GlobalSynthParams.voiceParams.dFilterKeyTrackIntensity = m_dFilterKeyTrackIntensity;
	m_GlobalSynthParams.voiceParams.dLFO1Filter1ModIntensity = m_dLFO1FilterFcIntensity;
	m_GlobalSynthParams.voiceParams.dLFO1Filter2ModIntensity = m_dLFO1FilterFcIntensity;
	m_GlobalSynthParams.voiceParams.dLFO1OscModIntensity = m_dLFO1OscPitchIntensity;
	m_GlobalSynthParams.voiceParams.dLFO1DCAAmpModIntensity = m_dLFO1AmpIntensity;
	m_GlobalSynthParams.voiceParams.dLFO1DCAPanModIntensity = m_dLFO1PanIntensity;

	m_GlobalSynthParams.voiceParams.dEG1OscModIntensity = m_dEG1OscIntensity;
	m_GlobalSynthParams.voiceParams.dEG1Filter1ModIntensity = m_dEG1FilterIntensity;
	m_GlobalSynthParams.voiceParams.dEG1Filter2ModIntensity = m_dEG1FilterIntensity;
	m_GlobalSynthParams.voiceParams.dEG1DCAAmpModIntensity = m_dEG1DCAIntensity;

	// --- Oscillators:
	double dOscAmplitude = m_dOsc1Amplitude_dB == -96.0 ? 0.0 : pow(10.0, m_dOsc1Amplitude_dB/20.0);
	m_GlobalSynthParams.osc1Params.dAmplitude = dOscAmplitude;
	
	dOscAmplitude = m_dOsc2Amplitude_dB == -96.0 ? 0.0 : pow(10.0, m_dOsc2Amplitude_dB/20.0);
	m_GlobalSynthParams.osc2Params.dAmplitude = dOscAmplitude;

	// --- octave
	m_GlobalSynthParams.osc1Params.nOctave = m_nOctave;
	m_GlobalSynthParams.osc2Params.nOctave = m_nOctave;

	// --- detuning for DigiSynth
	m_GlobalSynthParams.osc1Params.nCents = m_dDetune_cents;
	m_GlobalSynthParams.osc2Params.nCents = m_dDetune_cents;

	// --- Filter:
	m_GlobalSynthParams.filter1Params.dFcControl = m_dFcControl;
	m_GlobalSynthParams.filter1Params.dQControl = m_dQControl;
	m_GlobalSynthParams.filter2Params.dFcControl = m_dFcControl;
	m_GlobalSynthParams.filter2Params.dQControl = m_dQControl;

	// --- LFO1:
	m_GlobalSynthParams.lfo1Params.uWaveform = m_uLFO1Waveform;
	m_GlobalSynthParams.lfo1Params.dAmplitude = m_dLFO1Amplitude;
	m_GlobalSynthParams.lfo1Params.dOscFo = m_dLFO1Rate;
	
	// --- LFO2:
	m_GlobalSynthParams.lfo2Params.uWaveform = m_uRotorWaveform;
	m_GlobalSynthParams.lfo2Params.dOscFo = m_dRotorRate;

	// --- EG1:
	m_GlobalSynthParams.eg1Params.dAttackTime_mSec = m_dAttackTime_mSec;
	m_GlobalSynthParams.eg1Params.dDecayTime_mSec = m_dDecayReleaseTime_mSec;
	m_GlobalSynthParams.eg1Params.dSustainLevel = m_dSustainLevel;
	m_GlobalSynthParams.eg1Params.dReleaseTime_mSec = m_dDecayReleaseTime_mSec;
	m_GlobalSynthParams.eg1Params.bResetToZero = (bool)m_uResetToZero;
	m_GlobalSynthParams.eg1Params.bLegatoMode = (bool)m_uLegatoMode;

	// --- DCA:
	m_GlobalSynthParams.dcaParams.dAmplitude_dB = m_dVolume_dB;
	
	// --- enable/disable mod matrix stuff
	if(m_uVelocityToAttackScaling == 1)
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_VELOCITY, DEST_ALL_EG_ATTACK_SCALING, true); // enable
	else
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_VELOCITY, DEST_ALL_EG_ATTACK_SCALING, false);

	if(m_uNoteNumberToDecayScaling == 1)
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_MIDI_NOTE_NUM, DEST_ALL_EG_DECAY_SCALING, true); // enable
	else
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_MIDI_NOTE_NUM, DEST_ALL_EG_DECAY_SCALING, false);

	if(m_uFilterKeyTrack == 1)
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_MIDI_NOTE_NUM, DEST_ALL_FILTER_KEYTRACK, true); // enable
	else
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_MIDI_NOTE_NUM, DEST_ALL_FILTER_KEYTRACK, false);
}



// --- increment the timestamp for new note events
void Processor::incrementVoiceTimestamps()
{
	for(int i=0; i<MAX_VOICES; i++)
	{
		if(m_pVoiceArray[i])
		{
			// if on, inc
			if(m_pVoiceArray[i]->m_bNoteOn)
				m_pVoiceArray[i]->m_uTimeStamp++;
		}
	}
}
	
// --- get oldest note
CVectorSynthVoice* Processor::getOldestVoice()
{
	int nTimeStamp = -1;
	CVectorSynthVoice* pFoundVoice = NULL;
	for(int i=0; i<MAX_VOICES; i++)
	{
		if(m_pVoiceArray[i])
		{
			// if on and older, save
			// highest timestamp is oldest
			if(m_pVoiceArray[i]->m_bNoteOn && (int)m_pVoiceArray[i]->m_uTimeStamp > nTimeStamp)
			{
				pFoundVoice = m_pVoiceArray[i];
				nTimeStamp = (int)m_pVoiceArray[i]->m_uTimeStamp;
			}
		}
	}
	return pFoundVoice;
}

// --- get oldest voice with a MIDI note also
CVectorSynthVoice* Processor::getOldestVoiceWithNote(UINT uMIDINote)
{
	int nTimeStamp = -1;
	CVectorSynthVoice* pFoundVoice = NULL;
	for(int i=0; i<MAX_VOICES; i++)
	{
		if(m_pVoiceArray[i])
		{
			// if on and older and same MIDI note, save
			// highest timestamp is oldest
			if(m_pVoiceArray[i]->canNoteOff() && 
				(int)m_pVoiceArray[i]->m_uTimeStamp > nTimeStamp && 
				m_pVoiceArray[i]->m_uMIDINoteNumber == uMIDINote)
			{
				pFoundVoice = m_pVoiceArray[i];
				nTimeStamp = (int)m_pVoiceArray[i]->m_uTimeStamp;
			}
		}
	}
	return pFoundVoice;
}
/* 
	Processor::getNoteCount()
	returns number of notes currently playing
*/
int Processor::getNoteOnCount()
{
	int nCount = 0;
	for(int i=0; i<MAX_VOICES; i++)
	{
		if(m_pVoiceArray[i])
		{
			if(m_pVoiceArray[i]->m_bNoteOn)
				nCount++;
		}
	}	
	return nCount;
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
			// TODO: maybe try to make this nearly sample accurate <- from Steinberg...

			// --- get the last point in queue
			if(queue->getPoint(queue->getPointCount()-1, /* last update point */
				sampleOffset,			/* sample offset */
				value) == kResultTrue)	/* value = [0..1] */
			{
				// --- at least one param changed
				paramChange = true;

				switch (pid) // same as RAFX uControlID
				{
					// cookVSTGUIVariable(min, max, currentValue) <- cooks raw data into meaningful info for us 
					case OSC1_AMPLITUDE_DB:
					{
						m_dOsc1Amplitude_dB = cookVSTGUIVariable(MIN_OUTPUT_AMPLITUDE_DB, MAX_OUTPUT_AMPLITUDE_DB, value);
						break;
					}

					case OSC2_AMPLITUDE_DB:
					{
						m_dOsc2Amplitude_dB = cookVSTGUIVariable(MIN_OUTPUT_AMPLITUDE_DB, MAX_OUTPUT_AMPLITUDE_DB, value);
						break;
					}

					case EG1_TO_OSC_INTENSITY:
					{
						m_dEG1OscIntensity = cookVSTGUIVariable(MIN_BIPOLAR, MAX_BIPOLAR, value);
						break;
					}

					case FILTER_FC:
					{
						// --- fc control is log, but value is still linear so must convert
						if(m_pFilterLogParam)
							m_dFcControl = m_pFilterLogParam->toPlain(value);
						else
							m_dFcControl = cookVSTGUIVariable(MIN_FILTER_FC, MAX_FILTER_FC, value);
						break;
					}

					case FILTER_Q:
					{
						m_dQControl = cookVSTGUIVariable(MIN_FILTER_Q, MAX_FILTER_Q, value);
						break;
					}

					case EG1_TO_FILTER_INTENSITY:
					{
						m_dEG1FilterIntensity = cookVSTGUIVariable(MIN_BIPOLAR, MAX_BIPOLAR, value);
						break;
					}

					case FILTER_KEYTRACK_INTENSITY:
					{
						m_dFilterKeyTrackIntensity = cookVSTGUIVariable(MIN_FILTER_KEYTRACK_INTENSITY, MAX_FILTER_KEYTRACK_INTENSITY, value);
						break;
					}

					case EG1_ATTACK_MSEC:
					{
						m_dAttackTime_mSec = cookVSTGUIVariable(MIN_EG_ATTACK_TIME, MAX_EG_ATTACK_TIME, value);
						break;
					}

					case EG1_DECAY_RELEASE_MSEC:
					{
						m_dDecayReleaseTime_mSec = cookVSTGUIVariable(MIN_EG_DECAYRELEASE_TIME, MAX_EG_DECAYRELEASE_TIME, value);
						break;
					}

					case EG1_SUSTAIN_LEVEL:
					{
						m_dSustainLevel = cookVSTGUIVariable(MIN_EG_SUSTAIN_LEVEL, MAX_EG_SUSTAIN_LEVEL, value);
						break;
					}

					case LFO1_WAVEFORM:
					{
						m_uLFO1Waveform = (UINT)cookVSTGUIVariable(MIN_LFO_WAVEFORM, MAX_LFO_WAVEFORM, value);
						break;
					}
					
					case LFO1_RATE:
					{
						m_dLFO1Rate = cookVSTGUIVariable(MIN_LFO_RATE, MAX_LFO_RATE, value);
						break;
					}

					case LFO1_AMPLITUDE:
					{
						m_dLFO1Amplitude = cookVSTGUIVariable(MIN_UNIPOLAR, MAX_UNIPOLAR, value);
						break;
					}

					case LFO1_TO_FILTER_INTENSITY:
					{
						m_dLFO1FilterFcIntensity = cookVSTGUIVariable(MIN_BIPOLAR, MAX_BIPOLAR, value);
						break;
					}

					case LFO1_TO_OSC_INTENSITY:
					{
						m_dLFO1OscPitchIntensity = cookVSTGUIVariable(MIN_BIPOLAR, MAX_BIPOLAR, value);
						break;
					}

					case LFO1_TO_DCA_INTENSITY:
					{
						m_dLFO1AmpIntensity = cookVSTGUIVariable(MIN_UNIPOLAR, MAX_UNIPOLAR, value);
						break;
					}

					case LFO1_TO_PAN_INTENSITY:
					{
						m_dLFO1PanIntensity = cookVSTGUIVariable(MIN_UNIPOLAR, MAX_UNIPOLAR, value);
						break;
					}
					
					case OUTPUT_AMPLITUDE_DB:
					{
						m_dVolume_dB = cookVSTGUIVariable(MIN_OUTPUT_AMPLITUDE_DB, MAX_OUTPUT_AMPLITUDE_DB, value);
						break;
					}

					case EG1_TO_DCA_INTENSITY:
					{
						m_dEG1DCAIntensity = cookVSTGUIVariable(MIN_BIPOLAR, MAX_BIPOLAR, value);
						break;
					}

					case VOICE_MODE:
					{
						m_uVoiceMode = (UINT)cookVSTGUIVariable(MIN_VOICE_MODE, MAX_VOICE_MODE, value);
						break;
					}

					case DETUNE_CENTS:
					{
						m_dDetune_cents = cookVSTGUIVariable(MIN_DETUNE_CENTS, MAX_DETUNE_CENTS, value);
						break;
					}
					
					case PORTAMENTO_TIME_MSEC:
					{
						m_dPortamentoTime_mSec = cookVSTGUIVariable(MIN_PORTAMENTO_TIME_MSEC, MAX_PORTAMENTO_TIME_MSEC, value);
						break;
					}

					case OCTAVE:
					{
						m_nOctave = (int)(cookVSTGUIVariable(MIN_OCTAVE, MAX_OCTAVE, value));
						break;
					}

					case VELOCITY_TO_ATTACK:
					{
						m_uVelocityToAttackScaling = (UINT)cookVSTGUIVariable(MIN_ONOFF_SWITCH, MAX_ONOFF_SWITCH, value);
						break;
					}

					case NOTE_NUM_TO_DECAY:
					{
						m_uNoteNumberToDecayScaling = (UINT)cookVSTGUIVariable(MIN_ONOFF_SWITCH, MAX_ONOFF_SWITCH, value);
						break;
					}	

					case RESET_TO_ZERO:
					{
						m_uResetToZero = (UINT)cookVSTGUIVariable(MIN_ONOFF_SWITCH, MAX_ONOFF_SWITCH, value);
						break;
					}

					case FILTER_KEYTRACK:
					{
						m_uFilterKeyTrack = (UINT)cookVSTGUIVariable(MIN_ONOFF_SWITCH, MAX_ONOFF_SWITCH, value);
						break;
					}

					case LEGATO_MODE:
					{
						m_uLegatoMode = (UINT)cookVSTGUIVariable(MIN_ONOFF_SWITCH, MAX_ONOFF_SWITCH, value);
						break;
					}

					case PITCHBEND_RANGE:
					{
						m_nPitchBendRange = (int)cookVSTGUIVariable(MIN_PITCHBEND_RANGE, MAX_PITCHBEND_RANGE, value);
						break;
					}

					case VECTORJOYSTICK_X: 
					{
						m_dJoystickX = value;
						break;
					}		
					case VECTORJOYSTICK_Y: 
					{
						m_dJoystickY = value;
						break;
					}		

					case PATH_MODE:
					{
						m_uVectorPathMode = (UINT)cookVSTGUIVariable(MIN_PATH_MODE, MAX_PATH_MODE, value);
						break;
					}

					case ORBIT_X:
					{
						m_dOrbitX = value; 
						break;
					}		
					case ORBIT_Y: 
					{
						m_dOrbitY = value; 
						break;
					}		
					case ROTOR_RATE: 
					{
						m_dRotorRate = cookVSTGUIVariable(MIN_SLOW_LFO_RATE, MAX_SLOW_LFO_RATE, value);
						break;
					}		
					case ROTOR_WAVEFORM: 
					{
						m_uRotorWaveform = (UINT)cookVSTGUIVariable(MIN_LFO_WAVEFORM, MAX_LFO_WAVEFORM, value);
						break;
					}	

					// --- MIDI messages
					case MIDI_PITCHBEND: // want -1 to +1
					{
						m_dMIDIPitchBend = unipolarToBipolar(value);
						for(int i=0; i<MAX_VOICES; i++)
						{
							// --- send to matrix
							m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_PITCHBEND] = m_dMIDIPitchBend;
						}
						break;
					}
					case MIDI_MODWHEEL: // want 0 to 127
					{
						m_uMIDIModWheel = unipolarToMIDI(value);
						for(int i=0; i<MAX_VOICES; i++)
						{
							 m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_MODWHEEL] = m_uMIDIModWheel;
						}
						break;
					}
					case MIDI_VOLUME_CC7: // want 0 to 127
					{
						m_uMIDIVolumeCC7 = unipolarToMIDI(value);
						for(int i=0; i<MAX_VOICES; i++)
						{
							m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_MIDI_VOLUME_CC07] = m_uMIDIVolumeCC7;
						}
						break;
					}
					case MIDI_PAN_CC10: // want 0 to 127
					{
						m_uMIDIPanCC10 = unipolarToMIDI(value);
						for(int i=0; i<MAX_VOICES; i++)
						{
							m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_MIDI_PAN_CC10] = m_uMIDIPanCC10;
						}
						break;
					}
					case MIDI_EXPRESSION_CC11: // want 0 to 127
					{
						m_uMIDIExpressionCC11 = unipolarToMIDI(value);
						for(int i=0; i<MAX_VOICES; i++)
						{
							m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_MIDI_EXPRESSION_CC11] = m_uMIDIExpressionCC11;
						}
						break;
					}
					case MIDI_SUSTAIN_PEDAL: // want 0 to 1
					{
						UINT uMIDI = value > 0.5 ? 127 : 0;
						for(int i=0; i<MAX_VOICES; i++)
						{
							m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_SUSTAIN_PEDAL] = uMIDI;
						}

						break;
					}
					case MIDI_CHANNEL_PRESSURE:
					{
						// --- !CHALLENGE!
						break;
					}
					case MIDI_ALL_NOTES_OFF:
					{
						// force all off
						for(int i=0; i<MAX_VOICES; i++)
						{
							m_pVoiceArray[i]->noteOff(m_pVoiceArray[i]->m_uMIDINoteNumber);
						}
						break;
					}
				}
			}
		}
	}

	// --- check and update
	if(paramChange)
		update();

	return paramChange;
}

bool Processor::doProcessEvent(Event& vstEvent)
{
	bool noteEvent = false;
	// --- process Note On or Note Off messages here; these are individual functions in RAFX
	switch(vstEvent.type)
	{
		// --- NOTE ON
		case Event::kNoteOnEvent:
		{
			// --- get the channel/note/vel
			UINT uMIDIChannel = (UINT)vstEvent.noteOn.channel;
			UINT uMIDINote = (UINT)vstEvent.noteOn.pitch;
			UINT uVelocity = (UINT)(127.0*vstEvent.noteOn.velocity);

			// --- test channel/ignore
			if(m_uMidiRxChannel != MIDI_CH_ALL && uMIDIChannel != m_uMidiRxChannel)
				return false;

			// --- event occurred
			noteEvent = true;

			// --- fix noteID as per SDK
			if(vstEvent.noteOn.noteId == -1)
				vstEvent.noteOn.noteId = uMIDINote;

			bool bStealNote = true;
			for(int i=0; i<MAX_VOICES; i++)
			{
				// --- loop and find free voice
				if(m_pVoiceArray[i])
				{
					// -- if we have a free voice, turn on
					if(!m_pVoiceArray[i]->m_bNoteOn)
					{
						// --- do this first
						incrementVoiceTimestamps();

						// --- then note on
						m_pVoiceArray[i]->noteOn(uMIDINote, uVelocity, midiFreqTable[uMIDINote], m_dLastNoteFrequency);

						#if(LOG_MIDI && _DEBUG)
							FDebugPrint("-- Note On Ch:%d Note:%d Vel:%d \n", uMIDIChannel, uMIDINote, uVelocity);
						#endif

						// --- save
						m_dLastNoteFrequency = midiFreqTable[uMIDINote];
						bStealNote = false;
						break;
					}
				}
			}

			// --- need to steal
			if(bStealNote)
			{
				// --- steal oldest note
				CVectorSynthVoice* pVoice = getOldestVoice();
				if(pVoice)
				{
					// --- do this first
					incrementVoiceTimestamps();

					// --- then note on
					pVoice->noteOn(uMIDINote, uVelocity, midiFreqTable[uMIDINote],
							   m_dLastNoteFrequency);

					#if(LOG_MIDI && _DEBUG)
						FDebugPrint("-- Note Stolen! Ch:%d Note:%d Vel:%d \n", uMIDIChannel, uMIDINote, uVelocity);
					#endif
				}

				// --- save
				m_dLastNoteFrequency = midiFreqTable[uMIDINote];
			}

			break;
		}

		// --- NOTE OFF
		case Event::kNoteOffEvent:
		{
			// --- get the channel/note/vel
			UINT uMIDIChannel = (UINT)vstEvent.noteOff.channel;
			UINT uMIDINote = (UINT)vstEvent.noteOff.pitch;
			UINT uVelocity = (UINT)(127.0*vstEvent.noteOff.velocity); // not used

			// --- test channel/ignore
			if(m_uMidiRxChannel != MIDI_CH_ALL && uMIDIChannel != m_uMidiRxChannel)
				return false;

			// --- event occurred
			noteEvent = true;

			// --- fix noteID as per SDK
			if(vstEvent.noteOff.noteId == -1)
				vstEvent.noteOff.noteId = uMIDINote;

			for(int i=0; i<MAX_VOICES; i++)
			{
				CVectorSynthVoice* pVoice = getOldestVoiceWithNote(uMIDINote);
				if(pVoice)
				{
					pVoice->noteOff(uMIDINote);
				#if(LOG_MIDI && _DEBUG)
					FDebugPrint("-- Note Off Ch:%d Note:%d Vel:%d \n", uMIDIChannel, uMIDINote, uVelocity);
				#endif
					break;
				}
			}

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
			// if the event is not in the current processing block then adapt offset for next block
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
		// sine we will to a pair of samples at a time
		float fMix = 0.25; // 12dB headroom
		double dLeft = 0.0;
		double dRight = 0.0;
		double dLeftAccum = 0.0;
		double dRightAccum = 0.0;

		for(int32 j=0; j<samplesToProcess; j++)
		{
			// --- clear accumulators
			dLeftAccum = 0.0;
			dRightAccum = 0.0;

			for(int i=0; i<MAX_VOICES; i++)
			{
				// --- iterate and process; notice we process ALL voices
				//     if they are OFF they return 0.0 immediately
				//
				// --- render left and right
				if(m_pVoiceArray[i])
					m_pVoiceArray[i]->doVoice(dLeft, dRight);

				// --- accumulate notes
				dLeftAccum += fMix*dLeft;
				dRightAccum += fMix*dRight;
			}
			
			// write out to buffer
			buffers[0][j] = dLeftAccum;  // left
			buffers[1][j] = dRightAccum; // right
		}

		// --- update the counter
		for(int i = 0; i < OUTPUT_CHANNELS; i++) 
			buffers[i] += samplesToProcess;

		// --- update the samples processed/to process
		numSamples -= samplesToProcess;
		samplesProcessed += samplesToProcess;

	} // end while (numSamples > 0)
	
	// can write OUT to the GUI; see documentation
	if (data.outputParameterChanges)
	{
		// write to GUI or Parameters
	}

	// --- set silence flags if no notes playing
	if(getNoteOnCount() == 0 && data.numOutputs > 0)
	{
		data.outputs[0].silenceFlags = 0x11; // left and right channel are silent
	}

	return kResultTrue;
}

}}} // namespaces
