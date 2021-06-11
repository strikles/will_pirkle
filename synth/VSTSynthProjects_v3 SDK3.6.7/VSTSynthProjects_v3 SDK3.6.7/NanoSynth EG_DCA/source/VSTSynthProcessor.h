// -----------------------------------------------------------------------------
// Project     : NANO SYNTH
// Version     : 3.6.6
// Created by  : Will Pirkle
// Description : NanoSynth with OSC, EG, DCA
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

#include "synthfunctions.h"

#define MAX_VOICES 16
#define OUTPUT_CHANNELS 2 // stereo only!

// --- synth objects
#include "QBLimitedOscillator.h"
#include "LFO.h"
#include "EnvelopeGenerator.h"
#include "DCA.h"

namespace Steinberg {
namespace Vst {
namespace NanoSynth {

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
	// --- NanoSynth Components
	CQBLimitedOscillator m_Osc1;
	CQBLimitedOscillator m_Osc2;
	CLFO m_LFO1;

	// NS3
	CEnvelopeGenerator m_EG1;
	CDCA m_DCA;

	// 1P --- Declare
	UINT m_uOscWaveform;
	UINT m_uLFO1Waveform;
	double m_dLFO1Rate;
	double m_dLFO1Amplitude;
	UINT m_uLFO1Mode;

	double m_dAttackTime_mSec;
	double m_dDecayTime_mSec;
	double m_dSustainLevel;
	double m_dReleaseTime_mSec;
	double m_dPanControl;
	double m_dVolume_dB;
	double m_dEG1DCAIntensity;
	UINT m_uResetToZero;
	UINT m_uLegatoMode;

	// ------------------------------------------

	// --- functions to reduce size of process()
	bool doControlUpdate(ProcessData& data);

	// --- for MIDI note-on/off
	bool doProcessEvent(Event& vstEvent);

	// updates all voices at once
	void update();

	// for portamento
	double m_dLastNoteFrequency;

	// MIDI variables
	bool m_bSustainPedal;

	// --- our MIDI receive channel
	UINT m_uMidiRxChannel;

	// these are VST3 specific variables for non-note MIDI messages!
	double m_dMIDIPitchBend; 
	UINT m_uMIDIModWheel;
	UINT m_uMIDIVolumeCC7;
	UINT m_uMIDIPanCC10;
	UINT m_uMIDIExpressionCC11;
};

}}} // namespaces

#endif
