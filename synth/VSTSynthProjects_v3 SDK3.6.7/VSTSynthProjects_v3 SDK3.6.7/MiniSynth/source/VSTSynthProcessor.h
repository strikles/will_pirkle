//-----------------------------------------------------------------------------
// Project     : MINI SYNTH
// Version     : 3.6.6
// Created by  : Will Pirkle
// Description : Analog-Style Synth
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

#ifndef __vst_synth_processor__
#define __vst_synth_processor__

#include "public.sdk/source/vst/vst2wrapper/vst2wrapper.h"
#include "vstgui/vstgui.h"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "public.sdk/source/vst/vstparameters.h"
#include "pluginterfaces/vst/ivstevents.h"

#include "MiniSynthVoice.h"
#include "StereoDelayFX.h"

#define MAX_VOICES 16
#define OUTPUT_CHANNELS 2 // stereo only!

namespace Steinberg {
namespace Vst {
namespace MiniSynth {

/*
	The  Processor handles the audio processing, in this case voice rendering. It is derived from AudioEffect
	which has much of the functionality hidden in it.
*/
class Processor : public AudioEffect
{
public:
	// --- constructor
	Processor();

	// --- destructor
	~Processor();


	// --- One time init to define our I/O 
	tresult PLUGIN_API initialize(FUnknown* context);

	// --- Define the audio I/O we support
	tresult PLUGIN_API setBusArrangements(SpeakerArrangement* inputs, int32 numIns, SpeakerArrangement* outputs, int32 numOuts);
	
	// --- Define our word-length capabilities (currently 32 bit only)
	tresult PLUGIN_API canProcessSampleSize(int32 symbolicSampleSize);

	// --- Turn on/off; this is equivalent to prepareForPlay() in RAFX
	tresult PLUGIN_API setActive(TBool state);

	// --- Save and load presets from a file stream
	tresult PLUGIN_API setState(IBStream* fileStream);
	tresult PLUGIN_API getState(IBStream* fileStream);

	// --- The all important process method where the audio is rendered
	tresult PLUGIN_API process(ProcessData& data);
	
	// --- our COM creation method
	static FUnknown* createInstance (void*) { return (IAudioProcessor*)new Processor (); }

	// --- our Globally Unique ID
	static FUID cid;

protected:
	// --- our voice array
	CMiniSynthVoice* m_pVoiceArray[MAX_VOICES];
	
	// -- MmM
	CModulationMatrix m_GlobalModMatrix;

	// --- global params
	globalSynthParams m_GlobalSynthParams;

	// --- helper functions for note on/off/voice steal
	void incrementVoiceTimestamps();
	CMiniSynthVoice* getOldestVoice();
	CMiniSynthVoice* getOldestVoiceWithNote(UINT uMIDINote);
	int getNoteOnCount();

	// --- functions to reduce size of process()
	bool doControlUpdate(ProcessData& data);

	// --- for MIDI note-on/off
	bool doProcessEvent(Event& vstEvent);

	// --- updates all voices at once
	void update();

	// --- for portamento
	double m_dLastNoteFrequency;

	// --- our recieve channel (optional)
	UINT m_uMidiRxChannel;
	
	// for converting the log filter control value
	Parameter* m_pFilterLogParam;

	// --- NEW  DELAY FX
	CStereoDelayFX m_DelayFX;

	// --- synth parameters; initialized in constructor! WP
	double m_dNoiseOsc_dB;
	double m_dPulseWidth_Pct;
	double m_dHSRatio;
	double m_dEG1OscIntensity;
	double m_dFcControl;
	double m_dQControl;
	double m_dEG1FilterIntensity;
	double m_dFilterKeyTrackIntensity;
	double m_dAttackTime_mSec;
	double m_dDecayReleaseTime_mSec;
	double m_dSustainLevel;
	UINT m_uLFO1Waveform;
	double m_dLFO1Rate;
	double m_dLFO1Amplitude;
	double m_dLFO1FilterFcIntensity;
	double m_dLFO1OscPitchIntensity;
	double m_dLFO1AmpIntensity;
	double m_dLFO1PanIntensity;
	double m_dVolume_dB;
	double m_dEG1DCAIntensity;
	UINT m_uVoiceMode;
	double m_dDetune_cents;
	double m_dPortamentoTime_mSec;
	int m_nOctave;
	UINT m_uVelocityToAttackScaling;
	UINT m_uNoteNumberToDecayScaling;
	UINT m_uResetToZero;
	UINT m_uFilterKeyTrack;
	UINT m_uLegatoMode;
	int m_nPitchBendRange;

	// these are VST3 specific variables for non-note MIDI messages!
	double m_dMIDIPitchBend; // -1 to +1
	UINT m_uMIDIModWheel;
	UINT m_uMIDIVolumeCC7;
	UINT m_uMIDIPanCC10;
	UINT m_uMIDIExpressionCC11;
	bool m_bSustainPedal;

	// --- delay FX
	double m_dDelayTime_mSec;
	double m_dFeedback_Pct;
	double m_dDelayRatio;
	double m_dWetMix;
	UINT m_uDelayMode;


};

}}} // namespaces

#endif
