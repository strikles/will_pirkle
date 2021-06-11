//-----------------------------------------------------------------------------
// Project     : NANO SYNTH MM3
// Version     : 3.6.6
// Created by  : Will Pirkle
// Description : NanoSynth with Mod Matrix III
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
		'Nan5',	/* 10 dig code for NanoSynth */
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

	// receive on all channels
	m_uMidiRxChannel = MIDI_CH_ALL;

	m_pFilterLogParam = NULL;

	// 2P --- initialize
	m_uOscWaveform = DEFAULT_PITCHED_OSC_WAVEFORM;
	m_uLFO1Waveform = DEFAULT_LFO_WAVEFORM;
	m_dLFO1Rate = DEFAULT_LFO_RATE;
	m_dLFO1Amplitude = DEFAULT_UNIPOLAR;
	m_uLFO1Mode = DEFAULT_LFO_MODE;

	// NS3
	m_dAttackTime_mSec = DEFAULT_EG_ATTACK_TIME;
	m_dDecayTime_mSec = DEFAULT_EG_DECAY_TIME;
	m_dSustainLevel = DEFAULT_EG_SUSTAIN_LEVEL;
	m_dReleaseTime_mSec = DEFAULT_EG_RELEASE_TIME;
	m_dPanControl = DEFAULT_PAN;
	m_dVolume_dB = DEFAULT_OUTPUT_AMPLITUDE_DB;
	m_dEG1DCAIntensity = MAX_BIPOLAR; // note!
	m_uResetToZero = DEFAULT_RESET_TO_ZERO;
	m_uLegatoMode = DEFAULT_LEGATO_MODE;

	// NS4
	m_dFcControl = DEFAULT_FILTER_FC;
	m_dQControl = DEFAULT_FILTER_Q;
	m_uFilterKeyTrack = DEFAULT_FILTER_KEYTRACK;
	m_dFilterKeyTrackIntensity = DEFAULT_FILTER_KEYTRACK_INTENSITY;
	m_dEG1OscIntensity = DEFAULT_BIPOLAR;

	// NS MM2
	m_uVelocityToAttackScaling = DEFAULT_VELOCITY_TO_ATTACK;
	m_uNoteNumberToDecayScaling = DEFAULT_NOTE_TO_DECAY;

	// NS MM3
	m_uLFO1Destination = DEFAULT_LFO1_DESTINATION;

	// --- set the core array on the master mod matrix
	m_dDefaultModIntensity = 1.0;
	m_dDefaultModRange = 1.0;
	m_dOscFoModRange = OSC_FO_MOD_RANGE; // OSC_FO_MOD_RANGE defined in oscillator'h
	m_dFilterModRange = FILTER_FC_MOD_RANGE; // FILTER_FC_MOD_RANGE defined in filter.h
	m_dOscFoPitchBendModRange = OSC_PITCHBEND_MOD_RANGE;
	m_dAmpModRange = AMP_MOD_RANGE;

	// VST3 specific
	m_dMIDIPitchBend = DEFAULT_MIDI_PITCHBEND; // -1 to +1
	m_uMIDIModWheel = DEFAULT_MIDI_MODWHEEL;
	m_uMIDIVolumeCC7 = DEFAULT_MIDI_VOLUME;  // note defaults to 127
	m_uMIDIPanCC10 = DEFAULT_MIDI_PAN;     // 64 = center pan
	m_uMIDIExpressionCC11 = DEFAULT_MIDI_EXPRESSION;

		
	// 	<< ** Code Listing 8.1 ** >>
	// --- The Mod matrix "wiring" for DEFAULTS for all synths
	//	   create a row for each source/destination pair; these all follow
	//     same pattern, see createModMatrixRow()
	// --- for individual rows
	modMatrixRow* pRow = NULL;

	// LFO1 -> ALL OSC1 FC
	pRow = createModMatrixRow(SOURCE_LFO1,
								DEST_ALL_OSC_FO,
								&m_dDefaultModIntensity,
								&m_dOscFoModRange,
								TRANSFORM_NONE,
								true);
	m_GlobalModMatrix.addModMatrixRow(pRow);

	// EG1 -> ALL OSC1 FC
	pRow = createModMatrixRow(SOURCE_BIASED_EG1,
								DEST_ALL_OSC_FO,
								&m_dEG1OscIntensity,
								&m_dOscFoModRange,
								TRANSFORM_NONE,
								true);
	m_GlobalModMatrix.addModMatrixRow(pRow);

	// EG1 -> FILTER1 FC
	pRow = createModMatrixRow(SOURCE_BIASED_EG1,
								DEST_ALL_FILTER_FC,
								&m_dDefaultModIntensity,
								&m_dFilterModRange,
								TRANSFORM_NONE,
								true);
	m_GlobalModMatrix.addModMatrixRow(pRow);

	// EG1 -> DCA EG
	pRow = createModMatrixRow(SOURCE_EG1,
								DEST_DCA_EG,
								&m_dEG1DCAIntensity,
								&m_dDefaultModRange,
								TRANSFORM_NONE,
								true);
	m_GlobalModMatrix.addModMatrixRow(pRow);

	// NOTE NUMBER -> FILTER Fc CONTROL
	pRow = createModMatrixRow(SOURCE_MIDI_NOTE_NUM,
								DEST_ALL_FILTER_KEYTRACK,
								&m_dFilterKeyTrackIntensity,
								&m_dDefaultModRange,
								TRANSFORM_NOTE_NUMBER_TO_FREQUENCY,
								false); /* DISABLED BY DEFAULT */
	m_GlobalModMatrix.addModMatrixRow(pRow);

	// << ** Code Listing 8.4 ** >>
	// VELOCITY -> DCA VEL
	pRow = createModMatrixRow(SOURCE_VELOCITY,
								DEST_DCA_VELOCITY,
								&m_dDefaultModIntensity,
								&m_dDefaultModRange,
								TRANSFORM_NONE,
								true);
	m_GlobalModMatrix.addModMatrixRow(pRow);

	// PITCHBEND -> PITCHBEND
	pRow = createModMatrixRow(SOURCE_PITCHBEND,
								DEST_ALL_OSC_FO,
								&m_dDefaultModIntensity,
								&m_dOscFoPitchBendModRange,
								TRANSFORM_NONE,
								true);
	m_GlobalModMatrix.addModMatrixRow(pRow);

	// MIDI Volume CC07
	pRow = createModMatrixRow(SOURCE_MIDI_VOLUME_CC07,
								DEST_DCA_AMP,
								&m_dDefaultModIntensity,
								&m_dAmpModRange,
								TRANSFORM_INVERT_MIDI_NORMALIZE,
								true);
	m_GlobalModMatrix.addModMatrixRow(pRow);

	// MIDI Pan CC10
	pRow = createModMatrixRow(SOURCE_MIDI_PAN_CC10,
								DEST_DCA_PAN,
								&m_dDefaultModIntensity,
								&m_dDefaultModRange,
								TRANSFORM_MIDI_TO_PAN,
								true);
	m_GlobalModMatrix.addModMatrixRow(pRow);

	// MIDI Sustain Pedal
	pRow = createModMatrixRow(SOURCE_SUSTAIN_PEDAL,
								DEST_ALL_EG_SUSTAIN_OVERRIDE,
								&m_dDefaultModIntensity,
								&m_dDefaultModRange,
								TRANSFORM_MIDI_SWITCH,
								true);
	m_GlobalModMatrix.addModMatrixRow(pRow);

	// VELOCITY -> EG ATTACK SOURCE_VELOCITY
	// 0 velocity -> scalar = 1, normal attack time
	// 128 velocity -> scalar = 0, fastest (0) attack time;
	// We use TRANSFORM_MIDI_NORMALIZE and the inversion takes
	// place in the EG update()
	pRow = createModMatrixRow(SOURCE_VELOCITY,
						DEST_ALL_EG_ATTACK_SCALING,
						&m_dDefaultModIntensity,
						&m_dDefaultModRange,
						TRANSFORM_MIDI_NORMALIZE,
						false); /* DISABLED BY DEFAULT */
	m_GlobalModMatrix.addModMatrixRow(pRow);

	// NOTE NUMBER -> EG DECAY SCALING
	// note#0 -> scalar = 1, normal decay time
	// note#128 -> scalar = 0, fastest (0) decay time
	// We use TRANSFORM_MIDI_NORMALIZE and the inversion takes
	// place in the EG update()
	pRow = createModMatrixRow(SOURCE_MIDI_NOTE_NUM,
						DEST_ALL_EG_DECAY_SCALING,
						&m_dDefaultModIntensity,
						&m_dDefaultModRange,
						TRANSFORM_MIDI_NORMALIZE,
						false); /* DISABLED BY DEFAULT */
	m_GlobalModMatrix.addModMatrixRow(pRow);

	// LFO1 -> FILTER1 FC
	pRow = createModMatrixRow(SOURCE_LFO1,
						DEST_ALL_FILTER_FC,
						&m_dDefaultModIntensity,
						&m_dFilterModRange,
						TRANSFORM_NONE,
						false); /* DISABLED BY DEFAULT */
	m_GlobalModMatrix.addModMatrixRow(pRow);


	// --- this sets up the DEFAULT CONNECTIONS!
	m_Osc1.m_pModulationMatrix = &m_GlobalModMatrix;

	// --- NOTE: Oscillator Source is a Destination of a modulator
	m_Osc1.m_uModSourceFo = DEST_OSC1_FO;
	m_Osc1.m_uModSourceAmp = DEST_OSC1_OUTPUT_AMP;

	// --- do same for Osc2
	m_Osc2.m_pModulationMatrix = &m_GlobalModMatrix;
	m_Osc2.m_uModSourceFo = DEST_OSC2_FO;
	m_Osc2.m_uModSourceAmp = DEST_OSC2_OUTPUT_AMP;

	m_Filter1.m_pModulationMatrix = &m_GlobalModMatrix;
	m_Filter1.m_uModSourceFc = DEST_FILTER1_FC;
	m_Filter1.m_uSourceFcControl = DEST_ALL_FILTER_KEYTRACK;

	// --- these are modulators: they write their outputs into what will be a Source for something else
	m_LFO1.m_pModulationMatrix = &m_GlobalModMatrix;
	m_LFO1.m_uModDestOutput1 = SOURCE_LFO1;
	m_LFO1.m_uModDestOutput2 = SOURCE_LFO1Q;

	m_EG1.m_pModulationMatrix = &m_GlobalModMatrix;
	m_EG1.m_uModDestEGOutput = SOURCE_EG1;
	m_EG1.m_uModDestBiasedEGOutput = SOURCE_BIASED_EG1;
	m_EG1.m_uModSourceEGAttackScaling = DEST_EG1_ATTACK_SCALING;
	m_EG1.m_uModSourceEGDecayScaling = DEST_EG1_DECAY_SCALING;
	m_EG1.m_uModSourceSustainOverride = DEST_EG1_SUSTAIN_OVERRIDE;

	// --- DCA Setup:
	m_DCA.m_pModulationMatrix = &m_GlobalModMatrix;
	m_DCA.m_uModSourceEG = DEST_DCA_EG;  //
	m_DCA.m_uModSourceAmp_dB = DEST_DCA_AMP;
	m_DCA.m_uModSourceVelocity = DEST_DCA_VELOCITY;
	m_DCA.m_uModSourcePan = DEST_DCA_PAN;

	// helper
	m_pFilterLogParam = new LogScaleParameter<ParamValue> (USTRING("Filter fc"),
														    FILTER_FC,
														    filterLogScale2,
														    (USTRING("Hz")));
}

// --- destructor
Processor::~Processor()
{
	// --- delete on master ONLY
	m_GlobalModMatrix.deleteModMatrix();
			
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

	// 3P --- read code
	if(!s.readInt32u(udata)) return kResultFalse; else m_uOscWaveform = udata;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uLFO1Waveform = udata;
	if(!s.readDouble(m_dLFO1Rate)) return kResultFalse;
	if(!s.readDouble(m_dLFO1Amplitude)) return kResultFalse;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uLFO1Mode = udata;

	// --- NS3
	if(!s.readDouble(m_dAttackTime_mSec)) return kResultFalse;
	if(!s.readDouble(m_dDecayTime_mSec)) return kResultFalse;
	if(!s.readDouble(m_dSustainLevel)) return kResultFalse;
	if(!s.readDouble(m_dReleaseTime_mSec)) return kResultFalse;
	if(!s.readDouble(m_dPanControl)) return kResultFalse;
	if(!s.readDouble(m_dVolume_dB)) return kResultFalse;
	if(!s.readDouble(m_dEG1DCAIntensity)) return kResultFalse;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uResetToZero = udata;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uLegatoMode = udata;

	// --- NS4
	if(!s.readDouble(m_dFcControl)) return kResultFalse;
	if(!s.readDouble(m_dQControl)) return kResultFalse;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uFilterKeyTrack = udata;
	if(!s.readDouble(m_dFilterKeyTrackIntensity)) return kResultFalse;
	if(!s.readDouble(m_dEG1OscIntensity)) return kResultFalse;

	// --- NS MM2
	if(!s.readInt32u(udata)) return kResultFalse; else m_uVelocityToAttackScaling = udata;
	if(!s.readInt32u(udata)) return kResultFalse; else m_uNoteNumberToDecayScaling = udata;

	// NS MM3
	if(!s.readInt32u(udata)) return kResultFalse; else m_uLFO1Destination = udata;

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

	// 4P --- write code
	if(!s.writeInt32u(m_uOscWaveform)) return kResultFalse;
	if(!s.writeInt32u(m_uLFO1Waveform)) return kResultFalse;
	if(!s.writeDouble(m_dLFO1Rate)) return kResultFalse;
	if(!s.writeDouble(m_dLFO1Amplitude)) return kResultFalse;
	if(!s.writeInt32u(m_uLFO1Mode)) return kResultFalse;

	// NS3
	if(!s.writeDouble(m_dAttackTime_mSec)) return kResultFalse;
	if(!s.writeDouble(m_dDecayTime_mSec)) return kResultFalse;
	if(!s.writeDouble(m_dSustainLevel)) return kResultFalse;
	if(!s.writeDouble(m_dReleaseTime_mSec)) return kResultFalse;
	if(!s.writeDouble(m_dPanControl)) return kResultFalse;
	if(!s.writeDouble(m_dVolume_dB)) return kResultFalse;
	if(!s.writeDouble(m_dEG1DCAIntensity)) return kResultFalse;
	if(!s.writeInt32u(m_uResetToZero)) return kResultFalse;
	if(!s.writeInt32u(m_uLegatoMode)) return kResultFalse;

	// NS4
	if(!s.writeDouble(m_dFcControl)) return kResultFalse;
	if(!s.writeDouble(m_dQControl)) return kResultFalse;
	if(!s.writeInt32u(m_uFilterKeyTrack)) return kResultFalse;
	if(!s.writeDouble(m_dFilterKeyTrackIntensity)) return kResultFalse;
	if(!s.writeDouble(m_dEG1OscIntensity)) return kResultFalse;

	// --- NS MM2
	if(!s.writeInt32u(m_uVelocityToAttackScaling)) return kResultFalse;
	if(!s.writeInt32u(m_uNoteNumberToDecayScaling)) return kResultFalse;

	// --- NS MM3
	if(!s.writeInt32u(m_uLFO1Destination)) return kResultFalse;

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
		// --- do ON stuff; dynamic allocations
		m_Osc1.setSampleRate((double)processSetup.sampleRate);
		m_Osc2.setSampleRate((double)processSetup.sampleRate);
		m_Osc2.m_nCents = 2.5; // +2.5 cents detuned

		m_LFO1.setSampleRate((double)processSetup.sampleRate);
		m_Filter1.setSampleRate((double)processSetup.sampleRate);
		m_EG1.setSampleRate((double)processSetup.sampleRate);
		m_EG1.m_bOutputEG = true;

		update();

		// --- default turn on volume and center the pan
		m_GlobalModMatrix.m_dSources[SOURCE_MIDI_VOLUME_CC07] = 127;
		m_GlobalModMatrix.m_dSources[SOURCE_MIDI_PAN_CC10] = 64;

		// clear
		m_dLastNoteFrequency = 0.0;


	}
	else
	{
	
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
	// --- set public attributes on objects
	//
	// --- Oscillators
	m_Osc1.m_uWaveform = m_uOscWaveform;
	m_Osc2.m_uWaveform = m_uOscWaveform;

	// --- Filter
	m_Filter1.m_dFcControl = m_dFcControl;
	m_Filter1.m_dQControl = m_dQControl;

	// --- LFO
	m_LFO1.m_uWaveform = m_uLFO1Waveform;
	m_LFO1.m_dAmplitude = m_dLFO1Amplitude;
	m_LFO1.m_dOscFo = m_dLFO1Rate;
	m_LFO1.m_uLFOMode = m_uLFO1Mode;

	// ---EG1
	m_EG1.setAttackTime_mSec(m_dAttackTime_mSec);
	m_EG1.setDecayTime_mSec(m_dDecayTime_mSec);
	m_EG1.setSustainLevel(m_dSustainLevel);
	m_EG1.setReleaseTime_mSec(m_dReleaseTime_mSec);
	m_EG1.m_bResetToZero = (bool)m_uResetToZero;
	m_EG1.m_bLegatoMode = (bool)m_uLegatoMode;

	// --- DCA
	m_DCA.setPanControl(m_dPanControl);
	m_DCA.setAmplitude_dB(m_dVolume_dB);

	// --- mod matrix stuff
	if(m_uFilterKeyTrack == 1)
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_MIDI_NOTE_NUM, DEST_ALL_FILTER_KEYTRACK, true); // enable
	else
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_MIDI_NOTE_NUM, DEST_ALL_FILTER_KEYTRACK, false);

	if(m_uVelocityToAttackScaling == 1)
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_VELOCITY, DEST_ALL_EG_ATTACK_SCALING, true); // enable
	else
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_VELOCITY, DEST_ALL_EG_ATTACK_SCALING, false);

	if(m_uNoteNumberToDecayScaling == 1)
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_MIDI_NOTE_NUM, DEST_ALL_EG_DECAY_SCALING, true); // enable
	else
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_MIDI_NOTE_NUM, DEST_ALL_EG_DECAY_SCALING, false);

	 // --- LFO routing
	if(m_uLFO1Destination == 0)
	{
		// --- enable/disable
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_ALL_OSC_FO, true); /* enable */
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_ALL_FILTER_FC, false); /* disable */
	}
	else if(m_uLFO1Destination == 1)
	{
		// --- disable/enable
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_ALL_OSC_FO, false);/* disable */
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_ALL_FILTER_FC, true); /* enable */
	}

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
					// 5P --- control code
					case OSC_WAVEFORM:
					{
						m_uOscWaveform = (UINT)cookVSTGUIVariable(MIN_PITCHED_OSC_WAVEFORM, MAX_PITCHED_OSC_WAVEFORM, value);
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

					case LFO1_MODE:
					{
						m_uLFO1Mode = cookVSTGUIVariable(MIN_LFO_MODE, MAX_LFO_MODE, value);
						break;
					}

					// --- NS MM3
					case LFO1_DESTINATION:
					{
						m_uLFO1Destination = cookVSTGUIVariable(MIN_LFO1_DESTINATION, MAX_LFO1_DESTINATION, value);
						break;
					}

					// -- ns3 additions
					case EG1_ATTACK_MSEC:
					{
						m_dAttackTime_mSec = cookVSTGUIVariable(MIN_EG_ATTACK_TIME, MAX_EG_ATTACK_TIME, value);
						break;
					}

					case EG1_DECAY_MSEC:
					{
						m_dDecayTime_mSec = cookVSTGUIVariable(MIN_EG_DECAY_TIME, MAX_EG_DECAY_TIME, value);
						break;
					}

					case EG1_SUSTAIN_LEVEL:
					{
						m_dSustainLevel = cookVSTGUIVariable(MIN_EG_SUSTAIN_LEVEL, MAX_EG_SUSTAIN_LEVEL, value);
						break;
					}

					case EG1_RELEASE_MSEC:
					{
						m_dReleaseTime_mSec = cookVSTGUIVariable(MIN_EG_RELEASE_TIME, MAX_EG_RELEASE_TIME, value);
						break;
					}


					case OUTPUT_PAN:
					{
						m_dPanControl = cookVSTGUIVariable(MIN_PAN, MAX_PAN, value);
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

					case RESET_TO_ZERO:
					{
						m_uResetToZero = (UINT)cookVSTGUIVariable(MIN_ONOFF_SWITCH, MAX_ONOFF_SWITCH, value);
						break;
					}

					case LEGATO_MODE:
					{
						m_uLegatoMode = (UINT)cookVSTGUIVariable(MIN_ONOFF_SWITCH, MAX_ONOFF_SWITCH, value);
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

					case FILTER_KEYTRACK:
					{
						m_uFilterKeyTrack = (UINT)cookVSTGUIVariable(MIN_ONOFF_SWITCH, MAX_ONOFF_SWITCH, value);
						break;
					}

					case FILTER_KEYTRACK_INTENSITY:
					{
						m_dFilterKeyTrackIntensity = cookVSTGUIVariable(MIN_FILTER_KEYTRACK_INTENSITY, MAX_FILTER_KEYTRACK_INTENSITY, value);
						break;
					}

					case EG1_TO_OSC_INTENSITY:
					{
						m_dEG1OscIntensity = cookVSTGUIVariable(MIN_BIPOLAR, MAX_BIPOLAR, value);
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

					// --- MIDI messages
					case MIDI_PITCHBEND: // want -1 to +1
					{
						m_dMIDIPitchBend = unipolarToBipolar(value);
						#if(LOG_MIDI && _DEBUG)
							FDebugPrint("-- Pitch Bend: %f\n", m_dMIDIPitchBend);
						#endif

						// --- send to matrix
						m_GlobalModMatrix.m_dSources[SOURCE_PITCHBEND] = m_dMIDIPitchBend;

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

						// --- send to matrix
						m_GlobalModMatrix.m_dSources[SOURCE_MIDI_VOLUME_CC07] = m_uMIDIVolumeCC7;

						break;
					}
					case MIDI_PAN_CC10: // want 0 to 127
					{
						m_uMIDIPanCC10 = unipolarToMIDI(value);
						#if(LOG_MIDI && _DEBUG)
							FDebugPrint("-- Pan: %d\n", m_uMIDIPanCC10);
						#endif

						// --- send to matrix
						m_GlobalModMatrix.m_dSources[SOURCE_MIDI_PAN_CC10] = m_uMIDIPanCC10;

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
						bool bSus = value > 0.5 ? true : false;
						#if(LOG_MIDI && _DEBUG)
						if(bSus)
							FDebugPrint("-- Sustain Pedal ON\n");
						else
							FDebugPrint("-- Sustain Pedal OFF\n");
						#endif

						// --- send to matrix (note - need actual MIDI here)
						m_GlobalModMatrix.m_dSources[SOURCE_SUSTAIN_PEDAL] = unipolarToMIDI(value);

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
	if(paramChange)
		update();

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
			UINT uVelocity = (UINT)(127.0*vstEvent.noteOn.velocity);

			// --- test channel/ignore
			if(m_uMidiRxChannel != MIDI_CH_ALL && uMIDIChannel != m_uMidiRxChannel)
				return false;

			// --- event occurred
			noteEvent = true;

			// --- fix noteID as per SDK
			if(vstEvent.noteOn.noteId == -1)
				vstEvent.noteOn.noteId = uMIDINote;

			#if(LOG_MIDI && _DEBUG)
				FDebugPrint("-- Note On Ch:%d Note:%d Vel:%d \n", uMIDIChannel, uMIDINote, uVelocity);
			#endif

			m_Osc1.m_uMIDINoteNumber = uMIDINote;
			m_Osc1.m_dOscFo = midiFreqTable[uMIDINote];
			m_Osc1.update();

			m_Osc2.m_uMIDINoteNumber = uMIDINote;
			m_Osc2.m_dOscFo = midiFreqTable[uMIDINote];
			m_Osc2.update();

			// if note is playing, just change the pitch
			if(!m_Osc1.m_bNoteOn)
			{
				m_Osc1.startOscillator();
				m_Osc2.startOscillator();
			}

			m_LFO1.startOscillator();
			m_EG1.startEG();

			// --- set the note number in the mod matrix for filter key track
			m_GlobalModMatrix.m_dSources[SOURCE_MIDI_NOTE_NUM] = uMIDINote;

			// --- for Vel->Att scaling
			m_GlobalModMatrix.m_dSources[SOURCE_VELOCITY] = uVelocity;

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

			#if(LOG_MIDI && _DEBUG)
				FDebugPrint("-- Note Off Ch:%d Note:%d Vel:%d \n", uMIDIChannel, uMIDINote, uVelocity);
			#endif

			// --- turn off IF this is the proper note (last note played)
			if(uMIDINote == m_Osc1.m_uMIDINoteNumber)
				m_EG1.noteOff();


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

			// --- output "accumulator"
			double dOutL = 0.0;
			double dOutR = 0.0;

			for(int32 j=0; j<samplesToProcess; j++)
			{
				// --- clear accumulators
				dOutL = 0.0;
				dOutR = 0.0;

				if(m_Osc1.m_bNoteOn)
				{
					// --- ARTICULATION BLOCK --- //
					// --- layer 0 modulators: velocity->attack
					//						   note number->decay
					m_GlobalModMatrix.doModulationMatrix(0);

					// --- update layer 1 modulators
					m_EG1.update();
					m_LFO1.update();

					// --- layer 1 modulators
					m_EG1.doEnvelope();
					m_LFO1.doOscillate();

					// --- modulation matrix Layer 1
					m_GlobalModMatrix.doModulationMatrix(1);

					// --- update DCA and Filter
					m_DCA.update();
					m_Filter1.update();

					// --- update oscillators
					m_Osc1.update();
					m_Osc2.update();

					// --- DIGITAL AUDIO ENGINE BLOCK --- //
					// (OSC1 + OSC2) --> FILTER --> DCA
					double dOscOut = 0.5*m_Osc1.doOscillate() + 0.5*m_Osc2.doOscillate();
					double dFilterOut = m_Filter1.doFilter(dOscOut);
					m_DCA.doDCA(dFilterOut, dFilterOut, dOutL, dOutR);

					// now check for note off
					if(m_EG1.getState() == 0) // 0 = off
					{
						m_Osc1.stopOscillator();
						m_Osc2.stopOscillator();
						m_LFO1.stopOscillator();
						m_EG1.stopEG();
					}
				}

				// write out to buffer
				buffers[0][j] = dOutL;  // left
				buffers[1][j] = dOutR; // right
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
	if(!m_Osc1.m_bNoteOn)
	{
		data.outputs[0].silenceFlags = 0x11; // left and right channel are silent
	}

	return kResultTrue;
}

}}} // namespaces
