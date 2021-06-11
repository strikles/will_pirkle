//-----------------------------------------------------------------------------
// Project     : DX SYNTH
// Version     : 3.6.6
// Created by  : Will Pirkle
// Description : FM Synth
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
#include "pluginterfaces/vst/ivstevents.h"
#include "pluginterfaces/base/futils.h"

// --- Synth Stuff
#include "synthparamlimits.h" // param limits file

// --- MIDI Logging -- comment out to disable
#define LOG_MIDI 1

#define SYNTH_PROC_BLOCKSIZE 32 // 32 samples per processing block = 0.7 mSec = OK for tactile response WP

// --- VST2 wrapper
::AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
	return Steinberg::Vst::Vst2Wrapper::create(GetPluginFactory(),	/* calls factory.cpp macro */
		Steinberg::Vst::DXSynth::Processor::cid,	/* proc CID */
		'DXS0',	/* 10 dig code for DXSynth */
		audioMaster);
}

namespace Steinberg {
namespace Vst {
namespace DXSynth {

// --- for versioning in serialization
static uint64 DXSynthVersion = 0;

// --- the unique identifier (use guidgen.exe to generate)
FUID Processor::cid (0x52FB4CE9, 0x762F40CB, 0x82CE8DF2, 0xA10B65C0);

Processor::Processor ()
{
	// --- we are a Processor
	setControllerClass(Controller::cid);

	// our inits
	m_uVoiceMode = DEFAULT_VOICE_MODE;

	m_uLFO1Waveform = DEFAULT_LFO_WAVEFORM;
	m_dLFO1Intensity = DEFAULT_UNIPOLAR;
	m_dLFO1Rate = DEFAULT_LFO_RATE;

	m_uLFO1ModDest1 = DEFAULT_DX_LFO_DESTINATION;
	m_uLFO1ModDest2 = DEFAULT_DX_LFO_DESTINATION;
	m_uLFO1ModDest3 = DEFAULT_DX_LFO_DESTINATION;
	m_uLFO1ModDest4 = DEFAULT_DX_LFO_DESTINATION;

	m_dOp1Ratio = DEFAULT_OP_RATIO;
	m_dEG1Attack_mSec = DEFAULT_EG_ATTACK_TIME;
	m_dEG1Decay_mSec = DEFAULT_EG_DECAY_TIME;
	m_dEG1SustainLevel = DEFAULT_EG_SUSTAIN_LEVEL;
	m_dEG1Release_mSec = DEFAULT_EG_RELEASE_TIME;
	m_dOp1OutputLevel = DEFAULT_DX_OUTPUT_LEVEL;

	m_dOp2Ratio = DEFAULT_OP_RATIO;
	m_dEG2Attack_mSec = DEFAULT_EG_ATTACK_TIME;
	m_dEG2Decay_mSec = DEFAULT_EG_DECAY_TIME;
	m_dEG2SustainLevel = DEFAULT_EG_SUSTAIN_LEVEL;
	m_dEG2Release_mSec = DEFAULT_EG_RELEASE_TIME;
	m_dOp2OutputLevel = DEFAULT_DX_OUTPUT_LEVEL;

	m_dOp3Ratio = DEFAULT_OP_RATIO;
	m_dEG3Attack_mSec = DEFAULT_EG_ATTACK_TIME;
	m_dEG3Decay_mSec = DEFAULT_EG_DECAY_TIME;
	m_dEG3SustainLevel = DEFAULT_EG_SUSTAIN_LEVEL;
	m_dEG3Release_mSec = DEFAULT_EG_RELEASE_TIME;
	m_dOp3OutputLevel = DEFAULT_DX_OUTPUT_LEVEL;

	m_dOp4Feedback = 0.000000;
	m_dOp4Ratio = DEFAULT_OP_RATIO;
	m_dEG4Attack_mSec = DEFAULT_EG_ATTACK_TIME;
	m_dEG4Decay_mSec = DEFAULT_EG_DECAY_TIME;
	m_dEG4SustainLevel = DEFAULT_EG_SUSTAIN_LEVEL;
	m_dEG4Release_mSec = DEFAULT_EG_RELEASE_TIME;
	m_dOp4OutputLevel = DEFAULT_DX_OUTPUT_LEVEL;

	m_dPortamentoTime_mSec = DEFAULT_PORTAMENTO_TIME_MSEC;
	m_dVolume_dB = DEFAULT_OUTPUT_AMPLITUDE_DB;
	m_uLegatoMode = DEFAULT_LEGATO_MODE;
	m_uResetToZero = DEFAULT_RESET_TO_ZERO;
	m_nPitchBendRange = DEFAULT_PITCHBEND_RANGE;
	m_uVelocityToAttackScaling = DEFAULT_VELOCITY_TO_ATTACK;
	m_uNoteNumberToDecayScaling = DEFAULT_NOTE_TO_DECAY;

	// VST3 specific
	m_dMIDIPitchBend = DEFAULT_MIDI_PITCHBEND; // -1 to +1
	m_uMIDIModWheel = DEFAULT_MIDI_MODWHEEL;
	m_uMIDIVolumeCC7 = DEFAULT_MIDI_VOLUME;  // note defaults to 127
	m_uMIDIPanCC10 = DEFAULT_MIDI_PAN;     // 64 = center pan
	m_uMIDIExpressionCC11 = DEFAULT_MIDI_EXPRESSION;

	// Finish initializations here
	m_dLastNoteFrequency = 0.0;

	// receive on all channels
	m_uMidiRxChannel = MIDI_CH_ALL;

	// load up voices
	for(int i=0; i<MAX_VOICES; i++)
	{
		// --- create voice
		m_pVoiceArray[i] = new CDXSynthVoice;

		// --- global params (MUST BE DONE before setting up mod matrix!)
		m_pVoiceArray[i]->initGlobalParameters(&m_GlobalSynthParams);
	}

	// --- use the first voice to setup the MmM
	if(m_pVoiceArray[0])
		m_pVoiceArray[0]->initializeModMatrix(&m_GlobalModMatrix);

	// --- then set the mod matrix cores on the rest of the voices
	for(int i=0; i<MAX_VOICES; i++)
	{
		// --- all matrices share a common core array of matrix rows
		if(m_pVoiceArray[i])
			m_pVoiceArray[i]->setModMatrixCore(m_GlobalModMatrix.getModMatrixCore());
	}
}

// --- destructor
Processor::~Processor()
{
	// --- delete voices
	for(int i=0; i<MAX_VOICES; i++)
	{
		if(m_pVoiceArray[i])
		{
			delete m_pVoiceArray[i];
			m_pVoiceArray[i] = NULL;
		}
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

	//// read the version
	if(!s.readInt64u(version)) return kResultFalse;

	//// --- then read Version 0 params
	if(!s.readInt32u(udata)) return kResultFalse; else m_uVoiceMode = udata;

	if(!s.readInt32u(udata)) return kResultFalse; else m_uLFO1Waveform = udata;
	if(!s.readDouble(m_dLFO1Intensity)) return kResultFalse;
	if(!s.readDouble(m_dLFO1Rate)) return kResultFalse;

	// doing this udata->variable thing because cannot convert Steinberg var uint32->UINT
	// during a parameter pass in function
	if(!s.readInt32u(udata)) return kResultFalse; else m_uLFO1ModDest1 = udata;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uLFO1ModDest2 = udata;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uLFO1ModDest3 = udata;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uLFO1ModDest4 = udata;

	if(!s.readDouble(m_dOp1Ratio)) return kResultFalse;
	if(!s.readDouble(m_dEG1Attack_mSec)) return kResultFalse;
	if(!s.readDouble(m_dEG1Decay_mSec)) return kResultFalse;
	if(!s.readDouble(m_dEG1SustainLevel)) return kResultFalse;
	if(!s.readDouble(m_dEG1Release_mSec)) return kResultFalse;
	if(!s.readDouble(m_dOp1OutputLevel)) return kResultFalse;

	if(!s.readDouble(m_dOp2Ratio)) return kResultFalse;
	if(!s.readDouble(m_dEG2Attack_mSec)) return kResultFalse;
	if(!s.readDouble(m_dEG2Decay_mSec)) return kResultFalse;
	if(!s.readDouble(m_dEG2SustainLevel)) return kResultFalse;
	if(!s.readDouble(m_dEG2Release_mSec)) return kResultFalse;
	if(!s.readDouble(m_dOp2OutputLevel)) return kResultFalse;

	if(!s.readDouble(m_dOp3Ratio)) return kResultFalse;
	if(!s.readDouble(m_dEG3Attack_mSec)) return kResultFalse;
	if(!s.readDouble(m_dEG3Decay_mSec)) return kResultFalse;
	if(!s.readDouble(m_dEG3SustainLevel)) return kResultFalse;
	if(!s.readDouble(m_dEG3Release_mSec)) return kResultFalse;
	if(!s.readDouble(m_dOp3OutputLevel)) return kResultFalse;

	if(!s.readDouble(m_dOp4Feedback)) return kResultFalse;
	if(!s.readDouble(m_dOp4Ratio)) return kResultFalse;
	if(!s.readDouble(m_dEG4Attack_mSec)) return kResultFalse;
	if(!s.readDouble(m_dEG4Decay_mSec)) return kResultFalse;
	if(!s.readDouble(m_dEG4SustainLevel)) return kResultFalse;
	if(!s.readDouble(m_dEG4Release_mSec)) return kResultFalse;
	if(!s.readDouble(m_dOp4OutputLevel)) return kResultFalse;

	if(!s.readDouble(m_dPortamentoTime_mSec)) return kResultFalse;
	if(!s.readDouble(m_dVolume_dB)) return kResultFalse;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uLegatoMode = udata;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uResetToZero = udata;
	if(!s.readInt32(data)) return kResultFalse; else m_nPitchBendRange = data;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uVelocityToAttackScaling = udata;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uNoteNumberToDecayScaling = udata;

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
	if(!s.writeInt64u(DXSynthVersion)) return kResultFalse;

	if(!s.writeInt32u(m_uVoiceMode)) return kResultFalse;

	if(!s.writeInt32u(m_uLFO1Waveform)) return kResultFalse;
	if(!s.writeDouble(m_dLFO1Intensity)) return kResultFalse;
	if(!s.writeDouble(m_dLFO1Rate)) return kResultFalse;

	if(!s.writeInt32u(m_uLFO1ModDest1)) return kResultFalse;
	if(!s.writeInt32u(m_uLFO1ModDest2)) return kResultFalse;
	if(!s.writeInt32u(m_uLFO1ModDest3)) return kResultFalse;
	if(!s.writeInt32u(m_uLFO1ModDest4)) return kResultFalse;

	if(!s.writeDouble(m_dOp1Ratio)) return kResultFalse;
	if(!s.writeDouble(m_dEG1Attack_mSec)) return kResultFalse;
	if(!s.writeDouble(m_dEG1Decay_mSec)) return kResultFalse;
	if(!s.writeDouble(m_dEG1SustainLevel)) return kResultFalse;
	if(!s.writeDouble(m_dEG1Release_mSec)) return kResultFalse;
	if(!s.writeDouble(m_dOp1OutputLevel)) return kResultFalse;

	if(!s.writeDouble(m_dOp2Ratio)) return kResultFalse;
	if(!s.writeDouble(m_dEG2Attack_mSec)) return kResultFalse;
	if(!s.writeDouble(m_dEG2Decay_mSec)) return kResultFalse;
	if(!s.writeDouble(m_dEG2SustainLevel)) return kResultFalse;
	if(!s.writeDouble(m_dEG2Release_mSec)) return kResultFalse;
	if(!s.writeDouble(m_dOp2OutputLevel)) return kResultFalse;

	if(!s.writeDouble(m_dOp3Ratio)) return kResultFalse;
	if(!s.writeDouble(m_dEG3Attack_mSec)) return kResultFalse;
	if(!s.writeDouble(m_dEG3Decay_mSec)) return kResultFalse;
	if(!s.writeDouble(m_dEG3SustainLevel)) return kResultFalse;
	if(!s.writeDouble(m_dEG3Release_mSec)) return kResultFalse;
	if(!s.writeDouble(m_dOp3OutputLevel)) return kResultFalse;

	if(!s.writeDouble(m_dOp4Feedback)) return kResultFalse;
	if(!s.writeDouble(m_dOp4Ratio)) return kResultFalse;
	if(!s.writeDouble(m_dEG4Attack_mSec)) return kResultFalse;
	if(!s.writeDouble(m_dEG4Decay_mSec)) return kResultFalse;
	if(!s.writeDouble(m_dEG4SustainLevel)) return kResultFalse;
	if(!s.writeDouble(m_dEG4Release_mSec)) return kResultFalse;
	if(!s.writeDouble(m_dOp4OutputLevel)) return kResultFalse;

	if(!s.writeDouble(m_dPortamentoTime_mSec)) return kResultFalse;
	if(!s.writeDouble(m_dVolume_dB)) return kResultFalse;
	if(!s.writeInt32u(m_uLegatoMode)) return kResultFalse;
	if(!s.writeInt32u(m_uResetToZero)) return kResultFalse;
	if(!s.writeInt32(m_nPitchBendRange)) return kResultFalse;
	if(!s.writeInt32u(m_uVelocityToAttackScaling)) return kResultFalse;
	if(!s.writeInt32u(m_uNoteNumberToDecayScaling)) return kResultFalse;

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
	// for FM synth, Voice Mode = FM Algorithm
	m_GlobalSynthParams.voiceParams.uVoiceMode = m_uVoiceMode;
	m_GlobalSynthParams.voiceParams.dOp4Feedback = m_dOp4Feedback/100.0;
	m_GlobalSynthParams.voiceParams.dPortamentoTime_mSec = m_dPortamentoTime_mSec;

	// --- ranges
	m_GlobalSynthParams.voiceParams.dOscFoPitchBendModRange = m_nPitchBendRange;

	// --- intensities
	m_GlobalSynthParams.voiceParams.dLFO1OscModIntensity = m_dLFO1Intensity;

	// --- Oscillators:
	m_GlobalSynthParams.osc1Params.dAmplitude = calculateDXAmplitude(m_dOp1OutputLevel);
	m_GlobalSynthParams.osc2Params.dAmplitude = calculateDXAmplitude(m_dOp2OutputLevel);
	m_GlobalSynthParams.osc3Params.dAmplitude = calculateDXAmplitude(m_dOp3OutputLevel);
	m_GlobalSynthParams.osc4Params.dAmplitude = calculateDXAmplitude(m_dOp4OutputLevel);

	m_GlobalSynthParams.osc1Params.dFoRatio = m_dOp1Ratio;
	m_GlobalSynthParams.osc2Params.dFoRatio = m_dOp2Ratio;
	m_GlobalSynthParams.osc3Params.dFoRatio = m_dOp3Ratio;
	m_GlobalSynthParams.osc4Params.dFoRatio = m_dOp4Ratio;

	// --- EG1:
	m_GlobalSynthParams.eg1Params.dAttackTime_mSec = m_dEG1Attack_mSec;
	m_GlobalSynthParams.eg1Params.dDecayTime_mSec = m_dEG1Decay_mSec;
	m_GlobalSynthParams.eg1Params.dSustainLevel = m_dEG1SustainLevel;
	m_GlobalSynthParams.eg1Params.dReleaseTime_mSec = m_dEG1Release_mSec;
	m_GlobalSynthParams.eg1Params.bResetToZero = (bool)m_uResetToZero;
	m_GlobalSynthParams.eg1Params.bLegatoMode = (bool)m_uLegatoMode;

	// --- EG2:
	m_GlobalSynthParams.eg2Params.dAttackTime_mSec = m_dEG2Attack_mSec;
	m_GlobalSynthParams.eg2Params.dDecayTime_mSec = m_dEG2Decay_mSec;
	m_GlobalSynthParams.eg2Params.dSustainLevel = m_dEG2SustainLevel;
	m_GlobalSynthParams.eg2Params.dReleaseTime_mSec = m_dEG2Release_mSec;
	m_GlobalSynthParams.eg2Params.bResetToZero = (bool)m_uResetToZero;
	m_GlobalSynthParams.eg2Params.bLegatoMode = (bool)m_uLegatoMode;

	// --- EG3:
	m_GlobalSynthParams.eg3Params.dAttackTime_mSec = m_dEG3Attack_mSec;
	m_GlobalSynthParams.eg3Params.dDecayTime_mSec = m_dEG3Decay_mSec;
	m_GlobalSynthParams.eg3Params.dSustainLevel = m_dEG3SustainLevel;
	m_GlobalSynthParams.eg3Params.dReleaseTime_mSec = m_dEG3Release_mSec;
	m_GlobalSynthParams.eg3Params.bResetToZero = (bool)m_uResetToZero;
	m_GlobalSynthParams.eg3Params.bLegatoMode = (bool)m_uLegatoMode;

	// --- EG4:
	m_GlobalSynthParams.eg4Params.dAttackTime_mSec = m_dEG4Attack_mSec;
	m_GlobalSynthParams.eg4Params.dDecayTime_mSec = m_dEG4Decay_mSec;
	m_GlobalSynthParams.eg4Params.dSustainLevel = m_dEG4SustainLevel;
	m_GlobalSynthParams.eg4Params.dReleaseTime_mSec = m_dEG4Release_mSec;
	m_GlobalSynthParams.eg4Params.bResetToZero = (bool)m_uResetToZero;
	m_GlobalSynthParams.eg4Params.bLegatoMode = (bool)m_uLegatoMode;

	// --- LFO1:
	m_GlobalSynthParams.lfo1Params.uWaveform = m_uLFO1Waveform;
	m_GlobalSynthParams.lfo1Params.dOscFo = m_dLFO1Rate;

	// --- DCA:
	m_GlobalSynthParams.dcaParams.dAmplitude_dB = m_dVolume_dB;

	// --- LFO1 Destination 1
	if(m_uLFO1ModDest1 == None)
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC1_OUTPUT_AMP, false);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC1_FO, false);
	}
	else if(m_uLFO1ModDest1 == AmpMod)
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC1_OUTPUT_AMP, true);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC1_FO, false);
	}
	else // vibrato
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC1_OUTPUT_AMP, false);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC1_FO, true);
	}

	// --- LFO1 Destination 2
	if(m_uLFO1ModDest2 == None)
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC2_OUTPUT_AMP, false);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC2_FO, false);
	}
	else if(m_uLFO1ModDest2 == AmpMod)
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC2_OUTPUT_AMP, true);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC2_FO, false);
	}
	else // vibrato
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC2_OUTPUT_AMP, false);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC2_FO, true);
	}

	// --- LFO1 Destination 3
	if(m_uLFO1ModDest3 == None)
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC3_OUTPUT_AMP, false);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC3_FO, false);
	}
	else if(m_uLFO1ModDest3 == AmpMod)
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC3_OUTPUT_AMP, true);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC3_FO, false);
	}
	else // vibrato
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC3_OUTPUT_AMP, false);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC3_FO, true);
	}

	// --- LFO1 Destination 4
	if(m_uLFO1ModDest4 == None)
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC4_OUTPUT_AMP, false);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC4_FO, false);
	}
	else if(m_uLFO1ModDest4 == AmpMod)
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC4_OUTPUT_AMP, true);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC4_FO, false);
	}
	else // vibrato
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC4_OUTPUT_AMP, false);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC4_FO, true);
	}
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
CDXSynthVoice* Processor::getOldestVoice()
{
	int nTimeStamp = -1;
	CDXSynthVoice* pFoundVoice = NULL;
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
CDXSynthVoice* Processor::getOldestVoiceWithNote(UINT uMIDINote)
{
	int nTimeStamp = -1;
	CDXSynthVoice* pFoundVoice = NULL;
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
					case VOICE_MODE:
					{
						// cookVSTGUIVariable(min, max, currentValue) <- cooks raw data into meaningful info for us
						m_uVoiceMode = (UINT)cookVSTGUIVariable(MIN_VOICE_MODE, MAX_VOICE_MODE, value);
						break;
					}

					case LFO1_WAVEFORM:
					{
						m_uLFO1Waveform = (UINT)cookVSTGUIVariable(MIN_LFO_WAVEFORM, MAX_LFO_WAVEFORM, value);
						break;
					}

					case LFO1_AMPLITUDE:
					{
						m_dLFO1Intensity = value; // don't need to cook, unipolar
						break;
					}

					case LFO1_RATE:
					{
						m_dLFO1Rate = cookVSTGUIVariable(MIN_LFO_RATE, MAX_LFO_RATE, value);
						break;
					}

					case LFO1_DESTINATION_OP1:
					{
						m_uLFO1ModDest1 = (UINT)cookVSTGUIVariable(MIN_DX_LFO_DESTINATION, MAX_DX_LFO_DESTINATION, value);
						break;
					}

					case LFO1_DESTINATION_OP2:
					{
						m_uLFO1ModDest2 = (UINT)cookVSTGUIVariable(MIN_DX_LFO_DESTINATION, MAX_DX_LFO_DESTINATION, value);
						break;
					}

					case LFO1_DESTINATION_OP3:
					{
						m_uLFO1ModDest3 = (UINT)cookVSTGUIVariable(MIN_DX_LFO_DESTINATION, MAX_DX_LFO_DESTINATION, value);
						break;
					}

					case LFO1_DESTINATION_OP4:
					{
						m_uLFO1ModDest4 = (UINT)cookVSTGUIVariable(MIN_DX_LFO_DESTINATION, MAX_DX_LFO_DESTINATION, value);
						break;
					}

					case OP1_RATIO:
					{
						m_dOp1Ratio = cookVSTGUIVariable(MIN_OP_RATIO, MAX_OP_RATIO, value);
						break;
					}

					case EG1_ATTACK_MSEC:
					{
						m_dEG1Attack_mSec = cookVSTGUIVariable(MIN_EG_ATTACK_TIME, MAX_EG_ATTACK_TIME, value);
						break;
					}

					case EG1_DECAY_MSEC:
					{
						m_dEG1Decay_mSec = cookVSTGUIVariable(MIN_EG_DECAY_TIME, MAX_EG_DECAY_TIME, value);
						break;
					}

					case EG1_SUSTAIN_LEVEL:
					{
						m_dEG1SustainLevel = value;// don't need to cook, unipolar
						break;
					}

					case EG1_RELEASE_MSEC:
					{
						m_dEG1Release_mSec = cookVSTGUIVariable(MIN_EG_RELEASE_TIME, MAX_EG_RELEASE_TIME, value);
						break;
					}

					case OP2_RATIO:
					{
						m_dOp2Ratio = cookVSTGUIVariable(MIN_OP_RATIO, MAX_OP_RATIO, value);
						break;
					}

					case EG2_ATTACK_MSEC:
					{
						m_dEG2Attack_mSec = cookVSTGUIVariable(MIN_EG_ATTACK_TIME, MAX_EG_ATTACK_TIME, value);
						break;
					}

					case EG2_DECAY_MSEC:
					{
						m_dEG2Decay_mSec = cookVSTGUIVariable(MIN_EG_DECAY_TIME, MAX_EG_DECAY_TIME, value);
						break;
					}

					case EG2_SUSTAIN_LEVEL:
					{
						m_dEG2SustainLevel = value;// don't need to cook, unipolar
						break;
					}

					case EG2_RELEASE_MSEC:
					{
						m_dEG2Release_mSec = cookVSTGUIVariable(MIN_EG_RELEASE_TIME, MAX_EG_RELEASE_TIME, value);
						break;
					}

					case OP3_RATIO:
					{
						m_dOp3Ratio = cookVSTGUIVariable(MIN_OP_RATIO, MAX_OP_RATIO, value);
						break;
					}

					case EG3_ATTACK_MSEC:
					{
						m_dEG3Attack_mSec = cookVSTGUIVariable(MIN_EG_ATTACK_TIME, MAX_EG_ATTACK_TIME, value);
						break;
					}

					case EG3_DECAY_MSEC:
					{
						m_dEG3Decay_mSec = cookVSTGUIVariable(MIN_EG_DECAY_TIME, MAX_EG_DECAY_TIME, value);
						break;
					}

					case EG3_SUSTAIN_LEVEL:
					{
						m_dEG3SustainLevel = value;// don't need to cook, unipolar
						break;
					}

					case EG3_RELEASE_MSEC:
					{
						m_dEG3Release_mSec = cookVSTGUIVariable(MIN_EG_RELEASE_TIME, MAX_EG_RELEASE_TIME, value);
						break;
					}

					case OP4_FEEDBACK:
					{
						m_dOp4Feedback = cookVSTGUIVariable(MIN_OP_FEEDBACK, MAX_OP_FEEDBACK, value);
						break;
					}

					case OP4_RATIO:
					{
						m_dOp4Ratio = cookVSTGUIVariable(MIN_OP_RATIO, MAX_OP_RATIO, value);
						break;
					}

					case EG4_ATTACK_MSEC:
					{
						m_dEG4Attack_mSec = cookVSTGUIVariable(MIN_EG_ATTACK_TIME, MAX_EG_ATTACK_TIME, value);
						break;
					}

					case EG4_DECAY_MSEC:
					{
						m_dEG4Decay_mSec = cookVSTGUIVariable(MIN_EG_DECAY_TIME, MAX_EG_DECAY_TIME, value);
						break;
					}

					case EG4_SUSTAIN_LEVEL:
					{
						m_dEG4SustainLevel = value;// don't need to cook, unipolar
						break;
					}

					case EG4_RELEASE_MSEC:
					{
						m_dEG4Release_mSec = cookVSTGUIVariable(MIN_EG_RELEASE_TIME, MAX_EG_RELEASE_TIME, value);
						break;
					}

					case PORTAMENTO_TIME_MSEC:
					{
						m_dPortamentoTime_mSec = cookVSTGUIVariable(MIN_PORTAMENTO_TIME_MSEC, MAX_PORTAMENTO_TIME_MSEC, value);
						break;
					}

					case OUTPUT_AMPLITUDE_DB:
					{
						m_dVolume_dB = cookVSTGUIVariable(MIN_OUTPUT_AMPLITUDE_DB, MAX_OUTPUT_AMPLITUDE_DB, value);
						break;
					}

					case LEGATO_MODE:
					{
						m_uLegatoMode = value; // don't need to cook, unipolar
						break;
					}

					case RESET_TO_ZERO:
					{
						m_uResetToZero = value; // don't need to cook, unipolar
						break;
					}

					case PITCHBEND_RANGE:
					{
						m_nPitchBendRange = (int)cookVSTGUIVariable(MIN_PITCHBEND_RANGE, MAX_PITCHBEND_RANGE, value);
						break;
					}

					case VELOCITY_TO_ATTACK:
					{
						m_uVelocityToAttackScaling = value; // don't need to cook, unipolar
						break;
					}

					case NOTE_NUM_TO_DECAY:
					{
						m_uNoteNumberToDecayScaling = value; // don't need to cook, unipolar
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
				CDXSynthVoice* pVoice = getOldestVoice();
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
				CDXSynthVoice* pVoice = getOldestVoiceWithNote(uMIDINote);
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

	// --- sticky ON
	tresult result = kResultFalse;

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
