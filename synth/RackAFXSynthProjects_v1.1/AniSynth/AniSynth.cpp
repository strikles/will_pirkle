/*
	RackAFX(TM)
	Applications Programming Interface
	Derived Class Object Implementation
*/


#include "AniSynth.h"
#include "trace.h"

#define LOG_MIDI 1

/* constructor()
	You can initialize variables here.
	You can also allocate memory here as long is it does not
	require the plugin to be fully instantiated. If so, allocate in init()

*/
CAniSynth::CAniSynth()
{
	// Added by RackAFX - DO NOT REMOVE
	//
	// initUI() for GUI controls: this must be called before initializing/using any GUI variables
	initUI();
	// END initUI()

	// built in initialization
	m_PlugInName = "AniSynth";

	// Default to Stereo Operation:
	// Change this if you want to support more/less channels
	m_uMaxInputChannels = 2;
	m_uMaxOutputChannels = 2;

	// use of MIDI controllers to adjust sliders/knobs
	m_bEnableMIDIControl = true;		// by default this is enabled

	// custom GUI stuff
	m_bLinkGUIRowsAndButtons = false;	// change this if you want to force-link

	// DO NOT CHANGE let RackAFX change it for you; use Edit Project to alter
	m_bUseCustomVSTGUI = false;

	// for a user (not RackAFX) generated GUI - advanced you must compile your own resources
	// DO NOT CHANGE let RackAFX change it for you; use Edit Project to alter
	m_bUserCustomGUI = false;

	// output only - SYNTH - plugin DO NOT CHANGE let RackAFX change it for you; use Edit Project to alter
	m_bOutputOnlyPlugIn = true;

	// un-comment this for VST/AU Buffer-style processing
	// m_bWantVSTBuffers = true;

	// Finish initializations here
	// NEED THIS!
	m_bWantAllMIDIMessages = true;

	// Finish initializations here
	m_dLastNoteFrequency = 0.0;

	// receive on all channels
	m_uMidiRxChannel = MIDI_CH_ALL;

	// load up voices
	for(int i=0; i<MAX_VOICES; i++)
	{
		// --- create voice
		m_pVoiceArray[i] = new CAniSynthVoice;

		// --- should never happen
		if(!m_pVoiceArray[i]) return;

		// --- global params (MUST BE DONE before setting up mod matrix!)
		m_pVoiceArray[i]->initGlobalParameters(&m_GlobalSynthParams);

		// --- VectorSynth addition RAFX ONLY
		//
		// CJoystickProgram(m_pVectorJSProgram, <- pointer to our JS Program Array (programmed in RAFX, no code)
		//                  m_uVectorPathMode)  <- initialize the path mode
		m_pVoiceArray[i]->m_pJSProgram = new CJoystickProgram(m_pVectorJSProgram, m_uVectorPathMode);
	}

	// --- use the first voice to setup the MmM
	m_pVoiceArray[0]->initializeModMatrix(&m_GlobalModMatrix);

	// --- then set the mod
	for(int i=0; i<MAX_VOICES; i++)
	{
		// --- mod matrix
		m_pVoiceArray[i]->setModMatrixCore(m_GlobalModMatrix.getModMatrixCore());
	}
}


/* destructor()
	Destroy variables allocated in the contructor()

*/
CAniSynth::~CAniSynth(void)
{
	// --- delete on master ONLY
	m_GlobalModMatrix.deleteModMatrix();

	// --- delete voices
	for(int i=0; i<MAX_VOICES; i++)
	{
		if(m_pVoiceArray[i])
			delete m_pVoiceArray[i];
	}
}

/*
initialize()
	Called by the client after creation; the parent window handle is now valid
	so you can use the Plug-In -> Host functions here (eg sendUpdateUI())
	See the website www.willpirkle.com for more details
*/
bool __stdcall CAniSynth::initialize()
{
	return loadSamples();
}

bool CAniSynth::loadSamples()
{
	for(int i=0; i<MAX_VOICES; i++)
	{
		CAniSynthVoice* pVoice = m_pVoiceArray[i];
		if(!pVoice) return false;
		
		// --- create the array
		pVoice->createOscArray(5); // (N+1)^2 = 36 samples

		// --- initialize the oscillators
		int nIndex = 0;
		char* pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0001\\AKWF_0001.wav");
		pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless // true = single cycle sample
		delete [] pPath;

		pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0001\\AKWF_0002.wav");
		pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
		delete [] pPath;

		pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0001\\AKWF_0003.wav");
		pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
		delete [] pPath;

		pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0001\\AKWF_0004.wav");
		pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
		delete [] pPath;

		pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0002\\AKWF_0101.wav");
		pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
		delete [] pPath;

		pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0002\\AKWF_0102.wav");
		pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
		delete [] pPath;

		pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0002\\AKWF_0103.wav");
		pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
		delete [] pPath;

		pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0002\\AKWF_0104.wav");
		pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
		delete [] pPath;

		pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0003\\AKWF_0201.wav");
		pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
		delete [] pPath;

		pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0003\\AKWF_0202.wav");
		pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
		delete [] pPath;

		pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0003\\AKWF_0203.wav");
		pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
		delete [] pPath;

		pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0003\\AKWF_0204.wav");
		pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
		delete [] pPath;

		pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0004\\AKWF_0301.wav");
		pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
		delete [] pPath;

		pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0004\\AKWF_0302.wav");
		pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
		delete [] pPath;

		pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0004\\AKWF_0303.wav");
		pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
		delete [] pPath;

		pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0004\\AKWF_0304.wav");
		pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
		delete [] pPath;

		// for N=5 set if(true) for N=3 set if(false)
		if(true)
		{
			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0005\\AKWF_0421.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0005\\AKWF_0422.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0005\\AKWF_0431.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0005\\AKWF_0442.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0006\\AKWF_0513.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0006\\AKWF_0522.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0006\\AKWF_0534.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0006\\AKWF_0542.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0007\\AKWF_0615.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0007\\AKWF_0624.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0007\\AKWF_0613.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0007\\AKWF_0622.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0008\\AKWF_0737.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0008\\AKWF_0746.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0008\\AKWF_0714.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0008\\AKWF_0723.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0009\\AKWF_0832.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0009\\AKWF_0841.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0009\\AKWF_0817.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;

			pPath = addStrings(getMyDLLDirectory(),"\\Samples\\AKWF_0009\\AKWF_0828.wav");
			pVoice->initOscWithFilePath(nIndex++, pPath, true, false); // true: single sample, false: pitchless
			delete [] pPath;
		}
	}

	return true;

}



/* prepareForPlay()
	Called by the client after Play() is initiated but before audio streams

	You can perform buffer flushes and per-run intializations.
	You can check the following variables and use them if needed:

	m_nNumWAVEChannels;
	m_nSampleRate;
	m_nBitDepth;

	NOTE: the above values are only valid during prepareForPlay() and
		  processAudioFrame() because the user might change to another wave file,
		  or use the sound card, oscillators, or impulse response mechanisms

    NOTE: if you allocte memory in this function, destroy it in ::destroy() above
*/
bool __stdcall CAniSynth::prepareForPlay()
{
	// Add your code here:

	for(int i=0; i<MAX_VOICES; i++)
	{
		if(m_pVoiceArray[i])
		{
			m_pVoiceArray[i]->setSampleRate((double)m_nSampleRate);
			m_pVoiceArray[i]->prepareForPlay();
		}
	}

	// mass update
	update();

	// clear
	m_dLastNoteFrequency = 0.0;

	return true;
}

void CAniSynth::update()
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
	m_GlobalSynthParams.osc2Params.dAmplitude = dOscAmplitude;

	dOscAmplitude = m_dOsc2Amplitude_dB == -96.0 ? 0.0 : pow(10.0, m_dOsc2Amplitude_dB/20.0);
	m_GlobalSynthParams.osc3Params.dAmplitude = dOscAmplitude;
	m_GlobalSynthParams.osc4Params.dAmplitude = dOscAmplitude;

	// --- octave
	m_GlobalSynthParams.osc1Params.nOctave = m_nOctave;
	m_GlobalSynthParams.osc2Params.nOctave = m_nOctave;
	m_GlobalSynthParams.osc3Params.nOctave = m_nOctave;
	m_GlobalSynthParams.osc4Params.nOctave = m_nOctave;

	// --- detuning for AniSynth
	m_GlobalSynthParams.osc1Params.nCents = m_dDetune_cents;
	m_GlobalSynthParams.osc2Params.nCents = m_dDetune_cents;
	m_GlobalSynthParams.osc3Params.nCents = m_dDetune_cents;
	m_GlobalSynthParams.osc4Params.nCents = m_dDetune_cents;

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

/* processAudioFrame

// ALL VALUES IN AND OUT ON THE RANGE OF -1.0 TO + 1.0

LEFT INPUT = pInputBuffer[0];
RIGHT INPUT = pInputBuffer[1]

LEFT INPUT = pInputBuffer[0]
RIGHT OUTPUT = pOutputBuffer[1]

*/
bool __stdcall CAniSynth::processAudioFrame(float* pInputBuffer, float* pOutputBuffer, UINT uNumInputChannels, UINT uNumOutputChannels)
{
	// Do LEFT (MONO) Channel; there is always at least one input/one output
	double dLeftAccum = 0.0;
	double dRightAccum = 0.0;

	// --- 12dB HR
	float fMix = 1.0; // samples usually are already attenuated for headroom;
	double dLeft = 0.0;
	double dRight = 0.0;

	// --- loop and accumulate voices
	for(int i=0; i<MAX_VOICES; i++)
	{
		// --- get voice
		if(m_pVoiceArray[i])
			// --- render synth
			m_pVoiceArray[i]->doVoice(dLeft, dRight);

		// --- accumulate and scale
		dLeftAccum += fMix*dLeft;
		dRightAccum += fMix*dRight;
	}

	pOutputBuffer[0] = dLeftAccum;

	// Mono-In, Stereo-Out (AUX Effect)
	if(uNumInputChannels == 1 && uNumOutputChannels == 2)
		pOutputBuffer[1] = dLeftAccum;

	// Stereo-In, Stereo-Out (INSERT Effect)
	if(uNumInputChannels == 2 && uNumOutputChannels == 2)
		pOutputBuffer[1] = dRightAccum;


	return true;
}


/* ADDED BY RACKAFX -- DO NOT EDIT THIS CODE!!! ----------------------------------- //
   	**--0x2983--**

	Variable Name                    Index
-----------------------------------------------
	m_uVoiceMode                      0
	m_dDetune_cents                   1
	m_dFcControl                      2
	m_dLFO1Rate                       3
	m_dAttackTime_mSec                4
	m_dOrbitX                         5
	m_dOsc1Amplitude_dB               6
	m_nOctave                         10
	m_dPortamentoTime_mSec            11
	m_dQControl                       12
	m_dLFO1OscPitchIntensity          13
	m_dDecayReleaseTime_mSec          14
	m_dOrbitY                         15
	m_dOsc2Amplitude_dB               16
	m_uVectorPathMode                 20
	m_dEG1OscIntensity                21
	m_dEG1FilterIntensity             22
	m_dLFO1FilterFcIntensity          23
	m_dSustainLevel                   24
	m_dRotorRate                      25
	m_dLFO1Amplitude                  30
	m_dLFO1AmpIntensity               31
	m_dLFO1PanIntensity               32
	m_uLFO1Waveform                   33
	m_dEG1DCAIntensity                34
	m_uRotorWaveform                  35
	m_dVolume_dB                      100
	m_uLegatoMode                     101
	m_nPitchBendRange                 102
	m_uResetToZero                    103
	m_uFilterKeyTrack                 104
	m_dFilterKeyTrackIntensity        105
	m_uVelocityToAttackScaling        106
	m_uNoteNumberToDecayScaling       107

	Assignable Buttons               Index
-----------------------------------------------
	B1                                50
	B2                                51
	B3                                52

-----------------------------------------------
Joystick Drop List Boxes          Index
-----------------------------------------------
	 Drop List A                     60
	 Drop List B                     61
	 Drop List C                     62
	 Drop List D                     63

-----------------------------------------------

	**--0xFFDD--**
// ------------------------------------------------------------------------------- */
// Add your UI Handler code here ------------------------------------------------- //
//
bool __stdcall CAniSynth::userInterfaceChange(int nControlIndex)
{
	update();

	// decode the control index, or delete the switch and use brute force calls
	switch(nControlIndex)
	{
		case 0:
		{
			break;
		}

		default:
			break;
	}

	return true;
}


/* joystickControlChange

	Indicates the user moved the joystick point; the variables are the relative mixes
	of each axis; the values will add up to 1.0

			B
			|
		A -	x -	C
			|
			D

	The point in the very center (x) would be:
	fControlA = 0.25
	fControlB = 0.25
	fControlC = 0.25
	fControlD = 0.25

	AC Mix = projection on X Axis (0 -> 1)
	BD Mix = projection on Y Axis (0 -> 1)
*/
bool __stdcall CAniSynth::joystickControlChange(float fControlA, float fControlB, float fControlC, float fControlD, float fACMix, float fBDMix)
{
	// --- update globals
	m_GlobalSynthParams.voiceParams.dAmplitude_A = fControlA;
	m_GlobalSynthParams.voiceParams.dAmplitude_B = fControlB;
	m_GlobalSynthParams.voiceParams.dAmplitude_C = fControlC;
	m_GlobalSynthParams.voiceParams.dAmplitude_D = fControlD;
	m_GlobalSynthParams.voiceParams.dAmplitude_ACmix = fACMix;
	m_GlobalSynthParams.voiceParams.dAmplitude_BDmix = fBDMix;

	return true;
}

/* processAudioBuffer

	// ALL VALUES IN AND OUT ON THE RANGE OF -1.0 TO + 1.0

	The I/O buffers are interleaved depending on the number of channels. If uNumChannels = 2, then the
	buffer is L/R/L/R/L/R etc...

	if uNumChannels = 6 then the buffer is L/R/C/Sub/BL/BR etc...

	It is up to you to decode and de-interleave the data.

	To use this function set m_bWantBuffers = true in your constructor.

	******************************
	********* IMPORTANT! *********
	******************************
	If you are going to ultimately make this a VST Compatible Plug-In and you want to process
	buffers, you need to override the NEXT function below:

	processVSTAudioBuffer()


	This function (processRackAFXAudioBuffer) is not supported in the VST wrapper because
	the VST buffer sizes no maximum value. This would require the use of dynamic buffering
	in the callback which is not acceptable for performance!
*/
bool __stdcall CAniSynth::processRackAFXAudioBuffer(float* pInputBuffer, float* pOutputBuffer,
													   UINT uNumInputChannels, UINT uNumOutputChannels,
													   UINT uBufferSize)
{

	for(UINT i=0; i<uBufferSize; i++)
	{
		// pass through code
		pOutputBuffer[i] = pInputBuffer[i];
	}


	return true;
}



/* processVSTAudioBuffer

	// ALL VALUES IN AND OUT ON THE RANGE OF -1.0 TO + 1.0

	NOTE: You do not have to implement this function if you don't want to; the processAudioFrame()
	will still work; however this using function will be more CPU efficient for your plug-in, and will
	override processAudioFrame().

	To use this function set m_bWantVSTBuffers = true in your constructor.

	The VST input and output buffers are pointers-to-pointers. The pp buffers are the same depth as uNumChannels, so
	if uNumChannels = 2, then ppInputs would contain two pointers,

		inBuffer[0] = a pointer to the LEFT buffer of data
		inBuffer[1] = a pointer to the RIGHT buffer of data

	Similarly, outBuffer would have 2 pointers, one for left and one for right.

	For 5.1 audio you would get 6 pointers in each buffer.

*/
bool __stdcall CAniSynth::processVSTAudioBuffer(float** inBuffer, float** outBuffer, UINT uNumChannels, int inFramesToProcess)
{
	// PASS Through example
	// MONO First
	float* pInputL  = inBuffer[0];
	float* pOutputL = outBuffer[0];
	float* pInputR  = NULL;
	float* pOutputR = NULL;

	// if STEREO,
	if(inBuffer[1])
		pInputR = inBuffer[1];

	if(outBuffer[1])
		pOutputR = outBuffer[1];

	// Process audio by de-referencing ptrs
	// this is siple pass through code
	while (--inFramesToProcess >= 0)
	{
		// Left channel processing
		*pOutputL = *pInputL;

		// If there is a right channel
		if(pInputR)
			*pOutputR = *pInputR;

		// advance pointers
		pInputL++;
		pOutputL++;
		if(pInputR) pInputR++;
		if(pOutputR) pOutputR++;
	}
	// all OK
	return true;
}

// --- increment the timestamp for new note events
void CAniSynth::incrementVoiceTimestamps()
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
CAniSynthVoice* CAniSynth::getOldestVoice()
{
	int nTimeStamp = -1;
	CAniSynthVoice* pFoundVoice = NULL;
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
CAniSynthVoice* CAniSynth::getOldestVoiceWithNote(UINT uMIDINote)
{
	int nTimeStamp = -1;
	CAniSynthVoice* pFoundVoice = NULL;
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

bool __stdcall CAniSynth::midiNoteOn(UINT uChannel, UINT uMIDINote, UINT uVelocity)
{
	// test channel/ignore
	if(m_uMidiRxChannel != MIDI_CH_ALL && uChannel != m_uMidiRxChannel)
		return false;

	bool bStealNote = true;
	for(int i=0; i<MAX_VOICES; i++)
	{
		CAniSynthVoice* pVoice = m_pVoiceArray[i];
		if(!pVoice) return false; // should never happen

		// if we have a free voice, turn on
		if(!pVoice->m_bNoteOn)
		{
			// do this first
			incrementVoiceTimestamps();

			// then note on
			pVoice->noteOn(uMIDINote, uVelocity, m_MIDIFreqTable[uMIDINote], m_dLastNoteFrequency);

#ifdef LOG_MIDI
		TRACE("-- Note On Ch:%d Note:%d Vel:%d \n", uChannel, uMIDINote, uVelocity);
#endif
			// save
			m_dLastNoteFrequency = m_MIDIFreqTable[uMIDINote];
			bStealNote = false;
			break;
		}
	}

	if(bStealNote)
	{
		// steal oldest note
		CAniSynthVoice* pVoice = getOldestVoice();
		if(pVoice)
		{
			// do this first
			incrementVoiceTimestamps();

			// then note on
			pVoice->noteOn(uMIDINote, uVelocity, m_MIDIFreqTable[uMIDINote], m_dLastNoteFrequency);

#ifdef LOG_MIDI
			TRACE("-- Stole note from Voice with Note Number:%d, TO Note:%d \n", pVoice->m_uMIDINoteNumber, uMIDINote);
#endif
		}

		// save
		m_dLastNoteFrequency = m_MIDIFreqTable[uMIDINote];
	}
	return true;
}

bool __stdcall CAniSynth::midiNoteOff(UINT uChannel, UINT uMIDINote, UINT uVelocity, bool bAllNotesOff)
{
	// test channel/ignore
	if(m_uMidiRxChannel != MIDI_CH_ALL && uChannel != m_uMidiRxChannel)
	{
		return false;
	}

	#ifdef LOG_MIDI
		if(bAllNotesOff)
			TRACE("-- All Notes OFF Ch:%d \n", uChannel);
		else
			TRACE("-- Note Off Ch:%d Note:%d Vel:%d \n", uChannel, uMIDINote, uVelocity);
	#endif

	if(bAllNotesOff)
	{
		// force all off
		for(int i=0; i<MAX_VOICES; i++)
		{
			if(m_pVoiceArray[i])
				m_pVoiceArray[i]->noteOff(m_pVoiceArray[i]->m_uMIDINoteNumber);
		}

		return true;
	}

	// find and turn off
	// may have multiple notes sustaining; this ensures the oldest
	// note gets the event by starting at top of stack
	for(int i=0; i<MAX_VOICES; i++)
	{
		CAniSynthVoice* pVoice = getOldestVoiceWithNote(uMIDINote);
		if(pVoice)
		{
			pVoice->noteOff(uMIDINote);
			break;
		}
	}

	return true;
}

// uModValue = 0->127
bool __stdcall CAniSynth::midiModWheel(UINT uChannel, UINT uModValue)
{
	// test channel/ignore
	if(m_uMidiRxChannel != MIDI_CH_ALL && uChannel != m_uMidiRxChannel)
		return false;

#ifdef LOG_MIDI
		TRACE("-- Mod Wheel Ch:%d Value:%d \n", uChannel, uModValue);
#endif

	for(int i=0; i<MAX_VOICES; i++)
	{
		 if(m_pVoiceArray[i])
			 m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_MODWHEEL] = uModValue;
	}

	return true;
}

// nActualPitchBendValue 		= -8192 -> +8191, 0 at center
// fNormalizedPitchBendValue 	= -1.0  -> +1.0,  0 at center
bool __stdcall CAniSynth::midiPitchBend(UINT uChannel, int nActualPitchBendValue, float fNormalizedPitchBendValue)
{
	// test channel/ignore
	if(m_uMidiRxChannel != MIDI_CH_ALL && uChannel != m_uMidiRxChannel)
		return false;

#ifdef LOG_MIDI
		TRACE("-- Pitch Bend Ch:%d int:%d float:%f \n", uChannel, nActualPitchBendValue, fNormalizedPitchBendValue);
#endif

	for(int i=0; i<MAX_VOICES; i++)
	{
		// --- send to matrix
		if(m_pVoiceArray[i])
			m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_PITCHBEND] = fNormalizedPitchBendValue;
	}

	return true;
}

// MIDI Clock
// http://home.roadrunner.com/~jgglatt/tech/midispec/clock.htm
/* There are 24 MIDI Clocks in every quarter note. (12 MIDI Clocks in an eighth note, 6 MIDI Clocks in a 16th, etc).
   Therefore, when a slave device counts down the receipt of 24 MIDI Clock messages, it knows that one quarter note
   has passed. When the slave counts off another 24 MIDI Clock messages, it knows that another quarter note has passed.
   Etc. Of course, the rate that the master sends these messages is based upon the master's tempo.

   For example, for a tempo of 120 BPM (ie, there are 120 quarter notes in every minute), the master sends a MIDI clock
   every 20833 microseconds. (ie, There are 1,000,000 microseconds in a second. Therefore, there are 60,000,000
   microseconds in a minute. At a tempo of 120 BPM, there are 120 quarter notes per minute. There are 24 MIDI clocks
   in each quarter note. Therefore, there should be 24 * 120 MIDI Clocks per minute.
   So, each MIDI Clock is sent at a rate of 60,000,000/(24 * 120) microseconds).
*/
bool __stdcall CAniSynth::midiClock()
{

	return true;
}

// any midi message other than note on, note off, pitchbend, mod wheel or clock
bool __stdcall CAniSynth::midiMessage(unsigned char cChannel, unsigned char cStatus, unsigned char
						   				  cData1, unsigned char cData2)
{
	// test channel/ignore
	if(m_uMidiRxChannel != MIDI_CH_ALL && (UINT)cChannel != m_uMidiRxChannel)
		return false;

	// test channel/ignore
	if(m_uMidiRxChannel != MIDI_CH_ALL && (UINT)cChannel != m_uMidiRxChannel)
		return false;

	switch(cStatus)
	{
		case POLY_PRESSURE:
		{
			#ifdef LOG_MIDI
				TRACE("-- Poly Pressure Ch:%d Note:%d Value:%d \n", (UINT)cChannel, (UINT)cData1, (UINT)cData2);
			#endif

			break;
		}
		case CONTROL_CHANGE:
		{
			switch(cData1)
			{
				case VOLUME_CC07:
				{
					#ifdef LOG_MIDI
						TRACE("-- Volume Ch:%d Value:%d \n", (UINT)cChannel, (UINT)cData2);
					#endif

					// --- send to matrix
					for(int i=0; i<MAX_VOICES; i++)
					{
						 if(m_pVoiceArray[i])
							 m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_MIDI_VOLUME_CC07] = (UINT)cData2;
					}

					break;
				}
				case PAN_CC10:
				{
					#ifdef LOG_MIDI
						TRACE("-- Pan Ch:%d Value:%d \n", (UINT)cChannel, (UINT)cData2);
					#endif

					// --- send to matrix
					for(int i=0; i<MAX_VOICES; i++)
					{
						 if(m_pVoiceArray[i])
							 m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_MIDI_PAN_CC10] = (UINT)cData2;
					}

					break;
				}
				case EXPRESSION_CC11:
				{
					#ifdef LOG_MIDI
						TRACE("-- Expression Ch:%d Value:%d \n", (UINT)cChannel, (UINT)cData2);
					#endif

					// --- send to matrix
					for(int i=0; i<MAX_VOICES; i++)
					{
						 if(m_pVoiceArray[i])
							 m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_MIDI_EXPRESSION_CC11] = (UINT)cData2;
					}
					break;
				}

				case SUSTAIN_PEDAL:
				{
					// --- 64 or greater = ON for all switch messages
					bool sus = (UINT)cData2 > 63 ? true : false;

					#ifdef LOG_MIDI
						if(sus)
							TRACE("-- Sustain Pedal ON\n");
						else
							TRACE("-- Sustain Pedal OFF\n");
					#endif

					for(int i=0; i<MAX_VOICES; i++)
					{
						 if(m_pVoiceArray[i])
							 m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_SUSTAIN_PEDAL] = (UINT)cData2;
					}

					break;
				}
				case ALL_NOTES_OFF:
				{
					// handled in midiNoteOff() for RackAFX
					break;
				}
				case MOD_WHEEL:
				{
					// handled separately
					break;
				}
				// --- all other controllers
				default:
				{
					#ifdef LOG_MIDI
					if((UINT)cData1 != RESET_ALL_CONTROLLERS) // ignore these
						TRACE("-- CC Ch:%d Num:%d Value:%d \n", (UINT)cChannel, (UINT)cData1, (UINT)cData2);
					#endif

					break;
				}
			}

			break;
		}

		case PROGRAM_CHANGE:
		{
			#ifdef LOG_MIDI
				TRACE("-- Program Change Num Ch:%d Value:%d \n", (UINT)cChannel, (UINT)cData1);
			#endif

			break;
		}
		case CHANNEL_PRESSURE:
		{
			#ifdef LOG_MIDI
				TRACE("-- Channel Pressure Value Ch:%d Value:%d \n", (UINT)cChannel, (UINT)cData1);
			#endif

			break;
		}

		default:
			break;
	}

	return true;
}


// DO NOT DELETE THIS FUNCTION --------------------------------------------------- //
bool __stdcall CAniSynth::initUI()
{
	// ADDED BY RACKAFX -- DO NOT EDIT THIS CODE!!! ------------------------------ //
	if(m_UIControlList.count() > 0)
		return true;

// **--0xDEA7--**

	m_uVoiceMode = 0;
	CUICtrl ui0;
	ui0.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui0.uControlId = 0;
	ui0.bLogSlider = false;
	ui0.bExpSlider = false;
	ui0.fUserDisplayDataLoLimit = 0.000000;
	ui0.fUserDisplayDataHiLimit = 1.000000;
	ui0.uUserDataType = UINTData;
	ui0.fInitUserIntValue = 0;
	ui0.fInitUserFloatValue = 0;
	ui0.fInitUserDoubleValue = 0;
	ui0.fInitUserUINTValue = 0.000000;
	ui0.m_pUserCookedIntData = NULL;
	ui0.m_pUserCookedFloatData = NULL;
	ui0.m_pUserCookedDoubleData = NULL;
	ui0.m_pUserCookedUINTData = &m_uVoiceMode;
	ui0.cControlUnits = "                                                                ";
	ui0.cVariableName = "m_uVoiceMode";
	ui0.cEnumeratedList = "manual,program";
	ui0.dPresetData[0] = 0.000000;ui0.dPresetData[1] = 0.000000;ui0.dPresetData[2] = 0.000000;ui0.dPresetData[3] = 0.000000;ui0.dPresetData[4] = 0.000000;ui0.dPresetData[5] = 0.000000;ui0.dPresetData[6] = 0.000000;ui0.dPresetData[7] = 0.000000;ui0.dPresetData[8] = 0.000000;ui0.dPresetData[9] = 0.000000;ui0.dPresetData[10] = 0.000000;ui0.dPresetData[11] = 0.000000;ui0.dPresetData[12] = 0.000000;ui0.dPresetData[13] = 0.000000;ui0.dPresetData[14] = 0.000000;ui0.dPresetData[15] = 0.000000;
	ui0.cControlName = "Voice Mode";
	ui0.bOwnerControl = false;
	ui0.bMIDIControl = false;
	ui0.uMIDIControlCommand = 176;
	ui0.uMIDIControlName = 3;
	ui0.uMIDIControlChannel = 0;
	ui0.nGUIRow = -1;
	ui0.nGUIColumn = -1;
	ui0.uControlTheme[0] = 0; ui0.uControlTheme[1] = 0; ui0.uControlTheme[2] = 0; ui0.uControlTheme[3] = 0; ui0.uControlTheme[4] = 0; ui0.uControlTheme[5] = 0; ui0.uControlTheme[6] = 0; ui0.uControlTheme[7] = 0; ui0.uControlTheme[8] = 0; ui0.uControlTheme[9] = 0; ui0.uControlTheme[10] = 0; ui0.uControlTheme[11] = 0; ui0.uControlTheme[12] = 0; ui0.uControlTheme[13] = 0; ui0.uControlTheme[14] = 0; ui0.uControlTheme[15] = 0; ui0.uControlTheme[16] = 0; ui0.uControlTheme[17] = 0; ui0.uControlTheme[18] = 0; ui0.uControlTheme[19] = 0; ui0.uControlTheme[20] = 0; ui0.uControlTheme[21] = 0; ui0.uControlTheme[22] = 0; ui0.uControlTheme[23] = 0; ui0.uControlTheme[24] = 0; ui0.uControlTheme[25] = 0; ui0.uControlTheme[26] = 0; ui0.uControlTheme[27] = 0; ui0.uControlTheme[28] = 0; ui0.uControlTheme[29] = 0; ui0.uControlTheme[30] = 0; ui0.uControlTheme[31] = 0;
	ui0.uFluxCapControl[0] = 0; ui0.uFluxCapControl[1] = 0; ui0.uFluxCapControl[2] = 0; ui0.uFluxCapControl[3] = 0; ui0.uFluxCapControl[4] = 0; ui0.uFluxCapControl[5] = 0; ui0.uFluxCapControl[6] = 0; ui0.uFluxCapControl[7] = 0; ui0.uFluxCapControl[8] = 0; ui0.uFluxCapControl[9] = 0; ui0.uFluxCapControl[10] = 0; ui0.uFluxCapControl[11] = 0; ui0.uFluxCapControl[12] = 0; ui0.uFluxCapControl[13] = 0; ui0.uFluxCapControl[14] = 0; ui0.uFluxCapControl[15] = 0; ui0.uFluxCapControl[16] = 0; ui0.uFluxCapControl[17] = 0; ui0.uFluxCapControl[18] = 0; ui0.uFluxCapControl[19] = 0; ui0.uFluxCapControl[20] = 0; ui0.uFluxCapControl[21] = 0; ui0.uFluxCapControl[22] = 0; ui0.uFluxCapControl[23] = 0; ui0.uFluxCapControl[24] = 0; ui0.uFluxCapControl[25] = 0; ui0.uFluxCapControl[26] = 0; ui0.uFluxCapControl[27] = 0; ui0.uFluxCapControl[28] = 0; ui0.uFluxCapControl[29] = 0; ui0.uFluxCapControl[30] = 0; ui0.uFluxCapControl[31] = 0; ui0.uFluxCapControl[32] = 0; ui0.uFluxCapControl[33] = 0; ui0.uFluxCapControl[34] = 0; ui0.uFluxCapControl[35] = 0; ui0.uFluxCapControl[36] = 0; ui0.uFluxCapControl[37] = 0; ui0.uFluxCapControl[38] = 0; ui0.uFluxCapControl[39] = 0; ui0.uFluxCapControl[40] = 0; ui0.uFluxCapControl[41] = 0; ui0.uFluxCapControl[42] = 0; ui0.uFluxCapControl[43] = 0; ui0.uFluxCapControl[44] = 0; ui0.uFluxCapControl[45] = 0; ui0.uFluxCapControl[46] = 0; ui0.uFluxCapControl[47] = 0; ui0.uFluxCapControl[48] = 0; ui0.uFluxCapControl[49] = 0; ui0.uFluxCapControl[50] = 0; ui0.uFluxCapControl[51] = 0; ui0.uFluxCapControl[52] = 0; ui0.uFluxCapControl[53] = 0; ui0.uFluxCapControl[54] = 0; ui0.uFluxCapControl[55] = 0; ui0.uFluxCapControl[56] = 0; ui0.uFluxCapControl[57] = 0; ui0.uFluxCapControl[58] = 0; ui0.uFluxCapControl[59] = 0; ui0.uFluxCapControl[60] = 0; ui0.uFluxCapControl[61] = 0; ui0.uFluxCapControl[62] = 0; ui0.uFluxCapControl[63] = 0;
	ui0.fFluxCapData[0] = 0.000000; ui0.fFluxCapData[1] = 0.000000; ui0.fFluxCapData[2] = 0.000000; ui0.fFluxCapData[3] = 0.000000; ui0.fFluxCapData[4] = 0.000000; ui0.fFluxCapData[5] = 0.000000; ui0.fFluxCapData[6] = 0.000000; ui0.fFluxCapData[7] = 0.000000; ui0.fFluxCapData[8] = 0.000000; ui0.fFluxCapData[9] = 0.000000; ui0.fFluxCapData[10] = 0.000000; ui0.fFluxCapData[11] = 0.000000; ui0.fFluxCapData[12] = 0.000000; ui0.fFluxCapData[13] = 0.000000; ui0.fFluxCapData[14] = 0.000000; ui0.fFluxCapData[15] = 0.000000; ui0.fFluxCapData[16] = 0.000000; ui0.fFluxCapData[17] = 0.000000; ui0.fFluxCapData[18] = 0.000000; ui0.fFluxCapData[19] = 0.000000; ui0.fFluxCapData[20] = 0.000000; ui0.fFluxCapData[21] = 0.000000; ui0.fFluxCapData[22] = 0.000000; ui0.fFluxCapData[23] = 0.000000; ui0.fFluxCapData[24] = 0.000000; ui0.fFluxCapData[25] = 0.000000; ui0.fFluxCapData[26] = 0.000000; ui0.fFluxCapData[27] = 0.000000; ui0.fFluxCapData[28] = 0.000000; ui0.fFluxCapData[29] = 0.000000; ui0.fFluxCapData[30] = 0.000000; ui0.fFluxCapData[31] = 0.000000; ui0.fFluxCapData[32] = 0.000000; ui0.fFluxCapData[33] = 0.000000; ui0.fFluxCapData[34] = 0.000000; ui0.fFluxCapData[35] = 0.000000; ui0.fFluxCapData[36] = 0.000000; ui0.fFluxCapData[37] = 0.000000; ui0.fFluxCapData[38] = 0.000000; ui0.fFluxCapData[39] = 0.000000; ui0.fFluxCapData[40] = 0.000000; ui0.fFluxCapData[41] = 0.000000; ui0.fFluxCapData[42] = 0.000000; ui0.fFluxCapData[43] = 0.000000; ui0.fFluxCapData[44] = 0.000000; ui0.fFluxCapData[45] = 0.000000; ui0.fFluxCapData[46] = 0.000000; ui0.fFluxCapData[47] = 0.000000; ui0.fFluxCapData[48] = 0.000000; ui0.fFluxCapData[49] = 0.000000; ui0.fFluxCapData[50] = 0.000000; ui0.fFluxCapData[51] = 0.000000; ui0.fFluxCapData[52] = 0.000000; ui0.fFluxCapData[53] = 0.000000; ui0.fFluxCapData[54] = 0.000000; ui0.fFluxCapData[55] = 0.000000; ui0.fFluxCapData[56] = 0.000000; ui0.fFluxCapData[57] = 0.000000; ui0.fFluxCapData[58] = 0.000000; ui0.fFluxCapData[59] = 0.000000; ui0.fFluxCapData[60] = 0.000000; ui0.fFluxCapData[61] = 0.000000; ui0.fFluxCapData[62] = 0.000000; ui0.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui0);


	m_dDetune_cents = 0.000000;
	CUICtrl ui1;
	ui1.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui1.uControlId = 1;
	ui1.bLogSlider = false;
	ui1.bExpSlider = false;
	ui1.fUserDisplayDataLoLimit = -100.000000;
	ui1.fUserDisplayDataHiLimit = 100.000000;
	ui1.uUserDataType = doubleData;
	ui1.fInitUserIntValue = 0;
	ui1.fInitUserFloatValue = 0;
	ui1.fInitUserDoubleValue = 0.000000;
	ui1.fInitUserUINTValue = 0;
	ui1.m_pUserCookedIntData = NULL;
	ui1.m_pUserCookedFloatData = NULL;
	ui1.m_pUserCookedDoubleData = &m_dDetune_cents;
	ui1.m_pUserCookedUINTData = NULL;
	ui1.cControlUnits = "cnts                                                            ";
	ui1.cVariableName = "m_dDetune_cents";
	ui1.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui1.dPresetData[0] = 0.000000;ui1.dPresetData[1] = 0.000000;ui1.dPresetData[2] = 0.000000;ui1.dPresetData[3] = 0.000000;ui1.dPresetData[4] = 0.000000;ui1.dPresetData[5] = 0.000000;ui1.dPresetData[6] = 0.000000;ui1.dPresetData[7] = 0.000000;ui1.dPresetData[8] = 0.000000;ui1.dPresetData[9] = 0.000000;ui1.dPresetData[10] = 0.000000;ui1.dPresetData[11] = 0.000000;ui1.dPresetData[12] = 0.000000;ui1.dPresetData[13] = 0.000000;ui1.dPresetData[14] = 0.000000;ui1.dPresetData[15] = 0.000000;
	ui1.cControlName = "Detune";
	ui1.bOwnerControl = false;
	ui1.bMIDIControl = false;
	ui1.uMIDIControlCommand = 176;
	ui1.uMIDIControlName = 3;
	ui1.uMIDIControlChannel = 0;
	ui1.nGUIRow = -1;
	ui1.nGUIColumn = -1;
	ui1.uControlTheme[0] = 0; ui1.uControlTheme[1] = 0; ui1.uControlTheme[2] = 0; ui1.uControlTheme[3] = 0; ui1.uControlTheme[4] = 0; ui1.uControlTheme[5] = 0; ui1.uControlTheme[6] = 0; ui1.uControlTheme[7] = 0; ui1.uControlTheme[8] = 0; ui1.uControlTheme[9] = 0; ui1.uControlTheme[10] = 0; ui1.uControlTheme[11] = 0; ui1.uControlTheme[12] = 0; ui1.uControlTheme[13] = 0; ui1.uControlTheme[14] = 0; ui1.uControlTheme[15] = 0; ui1.uControlTheme[16] = 0; ui1.uControlTheme[17] = 0; ui1.uControlTheme[18] = 0; ui1.uControlTheme[19] = 0; ui1.uControlTheme[20] = 0; ui1.uControlTheme[21] = 0; ui1.uControlTheme[22] = 0; ui1.uControlTheme[23] = 0; ui1.uControlTheme[24] = 0; ui1.uControlTheme[25] = 0; ui1.uControlTheme[26] = 0; ui1.uControlTheme[27] = 0; ui1.uControlTheme[28] = 0; ui1.uControlTheme[29] = 0; ui1.uControlTheme[30] = 0; ui1.uControlTheme[31] = 0;
	ui1.uFluxCapControl[0] = 0; ui1.uFluxCapControl[1] = 0; ui1.uFluxCapControl[2] = 0; ui1.uFluxCapControl[3] = 0; ui1.uFluxCapControl[4] = 0; ui1.uFluxCapControl[5] = 0; ui1.uFluxCapControl[6] = 0; ui1.uFluxCapControl[7] = 0; ui1.uFluxCapControl[8] = 0; ui1.uFluxCapControl[9] = 0; ui1.uFluxCapControl[10] = 0; ui1.uFluxCapControl[11] = 0; ui1.uFluxCapControl[12] = 0; ui1.uFluxCapControl[13] = 0; ui1.uFluxCapControl[14] = 0; ui1.uFluxCapControl[15] = 0; ui1.uFluxCapControl[16] = 0; ui1.uFluxCapControl[17] = 0; ui1.uFluxCapControl[18] = 0; ui1.uFluxCapControl[19] = 0; ui1.uFluxCapControl[20] = 0; ui1.uFluxCapControl[21] = 0; ui1.uFluxCapControl[22] = 0; ui1.uFluxCapControl[23] = 0; ui1.uFluxCapControl[24] = 0; ui1.uFluxCapControl[25] = 0; ui1.uFluxCapControl[26] = 0; ui1.uFluxCapControl[27] = 0; ui1.uFluxCapControl[28] = 0; ui1.uFluxCapControl[29] = 0; ui1.uFluxCapControl[30] = 0; ui1.uFluxCapControl[31] = 0; ui1.uFluxCapControl[32] = 0; ui1.uFluxCapControl[33] = 0; ui1.uFluxCapControl[34] = 0; ui1.uFluxCapControl[35] = 0; ui1.uFluxCapControl[36] = 0; ui1.uFluxCapControl[37] = 0; ui1.uFluxCapControl[38] = 0; ui1.uFluxCapControl[39] = 0; ui1.uFluxCapControl[40] = 0; ui1.uFluxCapControl[41] = 0; ui1.uFluxCapControl[42] = 0; ui1.uFluxCapControl[43] = 0; ui1.uFluxCapControl[44] = 0; ui1.uFluxCapControl[45] = 0; ui1.uFluxCapControl[46] = 0; ui1.uFluxCapControl[47] = 0; ui1.uFluxCapControl[48] = 0; ui1.uFluxCapControl[49] = 0; ui1.uFluxCapControl[50] = 0; ui1.uFluxCapControl[51] = 0; ui1.uFluxCapControl[52] = 0; ui1.uFluxCapControl[53] = 0; ui1.uFluxCapControl[54] = 0; ui1.uFluxCapControl[55] = 0; ui1.uFluxCapControl[56] = 0; ui1.uFluxCapControl[57] = 0; ui1.uFluxCapControl[58] = 0; ui1.uFluxCapControl[59] = 0; ui1.uFluxCapControl[60] = 0; ui1.uFluxCapControl[61] = 0; ui1.uFluxCapControl[62] = 0; ui1.uFluxCapControl[63] = 0;
	ui1.fFluxCapData[0] = 0.000000; ui1.fFluxCapData[1] = 0.000000; ui1.fFluxCapData[2] = 0.000000; ui1.fFluxCapData[3] = 0.000000; ui1.fFluxCapData[4] = 0.000000; ui1.fFluxCapData[5] = 0.000000; ui1.fFluxCapData[6] = 0.000000; ui1.fFluxCapData[7] = 0.000000; ui1.fFluxCapData[8] = 0.000000; ui1.fFluxCapData[9] = 0.000000; ui1.fFluxCapData[10] = 0.000000; ui1.fFluxCapData[11] = 0.000000; ui1.fFluxCapData[12] = 0.000000; ui1.fFluxCapData[13] = 0.000000; ui1.fFluxCapData[14] = 0.000000; ui1.fFluxCapData[15] = 0.000000; ui1.fFluxCapData[16] = 0.000000; ui1.fFluxCapData[17] = 0.000000; ui1.fFluxCapData[18] = 0.000000; ui1.fFluxCapData[19] = 0.000000; ui1.fFluxCapData[20] = 0.000000; ui1.fFluxCapData[21] = 0.000000; ui1.fFluxCapData[22] = 0.000000; ui1.fFluxCapData[23] = 0.000000; ui1.fFluxCapData[24] = 0.000000; ui1.fFluxCapData[25] = 0.000000; ui1.fFluxCapData[26] = 0.000000; ui1.fFluxCapData[27] = 0.000000; ui1.fFluxCapData[28] = 0.000000; ui1.fFluxCapData[29] = 0.000000; ui1.fFluxCapData[30] = 0.000000; ui1.fFluxCapData[31] = 0.000000; ui1.fFluxCapData[32] = 0.000000; ui1.fFluxCapData[33] = 0.000000; ui1.fFluxCapData[34] = 0.000000; ui1.fFluxCapData[35] = 0.000000; ui1.fFluxCapData[36] = 0.000000; ui1.fFluxCapData[37] = 0.000000; ui1.fFluxCapData[38] = 0.000000; ui1.fFluxCapData[39] = 0.000000; ui1.fFluxCapData[40] = 0.000000; ui1.fFluxCapData[41] = 0.000000; ui1.fFluxCapData[42] = 0.000000; ui1.fFluxCapData[43] = 0.000000; ui1.fFluxCapData[44] = 0.000000; ui1.fFluxCapData[45] = 0.000000; ui1.fFluxCapData[46] = 0.000000; ui1.fFluxCapData[47] = 0.000000; ui1.fFluxCapData[48] = 0.000000; ui1.fFluxCapData[49] = 0.000000; ui1.fFluxCapData[50] = 0.000000; ui1.fFluxCapData[51] = 0.000000; ui1.fFluxCapData[52] = 0.000000; ui1.fFluxCapData[53] = 0.000000; ui1.fFluxCapData[54] = 0.000000; ui1.fFluxCapData[55] = 0.000000; ui1.fFluxCapData[56] = 0.000000; ui1.fFluxCapData[57] = 0.000000; ui1.fFluxCapData[58] = 0.000000; ui1.fFluxCapData[59] = 0.000000; ui1.fFluxCapData[60] = 0.000000; ui1.fFluxCapData[61] = 0.000000; ui1.fFluxCapData[62] = 0.000000; ui1.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui1);


	m_dFcControl = 10000.000000;
	CUICtrl ui2;
	ui2.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui2.uControlId = 2;
	ui2.bLogSlider = false;
	ui2.bExpSlider = true;
	ui2.fUserDisplayDataLoLimit = 80.000000;
	ui2.fUserDisplayDataHiLimit = 18000.000000;
	ui2.uUserDataType = doubleData;
	ui2.fInitUserIntValue = 0;
	ui2.fInitUserFloatValue = 0;
	ui2.fInitUserDoubleValue = 10000.000000;
	ui2.fInitUserUINTValue = 0;
	ui2.m_pUserCookedIntData = NULL;
	ui2.m_pUserCookedFloatData = NULL;
	ui2.m_pUserCookedDoubleData = &m_dFcControl;
	ui2.m_pUserCookedUINTData = NULL;
	ui2.cControlUnits = "Hz                                                              ";
	ui2.cVariableName = "m_dFcControl";
	ui2.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui2.dPresetData[0] = 10000.000000;ui2.dPresetData[1] = 0.000000;ui2.dPresetData[2] = 0.000000;ui2.dPresetData[3] = 0.000000;ui2.dPresetData[4] = 0.000000;ui2.dPresetData[5] = 0.000000;ui2.dPresetData[6] = 0.000000;ui2.dPresetData[7] = 0.000000;ui2.dPresetData[8] = 0.000000;ui2.dPresetData[9] = 0.000000;ui2.dPresetData[10] = 0.000000;ui2.dPresetData[11] = 0.000000;ui2.dPresetData[12] = 0.000000;ui2.dPresetData[13] = 0.000000;ui2.dPresetData[14] = 0.000000;ui2.dPresetData[15] = 0.000000;
	ui2.cControlName = "Filter fc";
	ui2.bOwnerControl = false;
	ui2.bMIDIControl = false;
	ui2.uMIDIControlCommand = 176;
	ui2.uMIDIControlName = 3;
	ui2.uMIDIControlChannel = 0;
	ui2.nGUIRow = -1;
	ui2.nGUIColumn = -1;
	ui2.uControlTheme[0] = 0; ui2.uControlTheme[1] = 0; ui2.uControlTheme[2] = 0; ui2.uControlTheme[3] = 0; ui2.uControlTheme[4] = 0; ui2.uControlTheme[5] = 0; ui2.uControlTheme[6] = 0; ui2.uControlTheme[7] = 0; ui2.uControlTheme[8] = 0; ui2.uControlTheme[9] = 0; ui2.uControlTheme[10] = 0; ui2.uControlTheme[11] = 0; ui2.uControlTheme[12] = 0; ui2.uControlTheme[13] = 0; ui2.uControlTheme[14] = 0; ui2.uControlTheme[15] = 0; ui2.uControlTheme[16] = 0; ui2.uControlTheme[17] = 0; ui2.uControlTheme[18] = 0; ui2.uControlTheme[19] = 0; ui2.uControlTheme[20] = 0; ui2.uControlTheme[21] = 0; ui2.uControlTheme[22] = 0; ui2.uControlTheme[23] = 0; ui2.uControlTheme[24] = 0; ui2.uControlTheme[25] = 0; ui2.uControlTheme[26] = 0; ui2.uControlTheme[27] = 0; ui2.uControlTheme[28] = 0; ui2.uControlTheme[29] = 0; ui2.uControlTheme[30] = 0; ui2.uControlTheme[31] = 0;
	ui2.uFluxCapControl[0] = 0; ui2.uFluxCapControl[1] = 0; ui2.uFluxCapControl[2] = 0; ui2.uFluxCapControl[3] = 0; ui2.uFluxCapControl[4] = 0; ui2.uFluxCapControl[5] = 0; ui2.uFluxCapControl[6] = 0; ui2.uFluxCapControl[7] = 0; ui2.uFluxCapControl[8] = 0; ui2.uFluxCapControl[9] = 0; ui2.uFluxCapControl[10] = 0; ui2.uFluxCapControl[11] = 0; ui2.uFluxCapControl[12] = 0; ui2.uFluxCapControl[13] = 0; ui2.uFluxCapControl[14] = 0; ui2.uFluxCapControl[15] = 0; ui2.uFluxCapControl[16] = 0; ui2.uFluxCapControl[17] = 0; ui2.uFluxCapControl[18] = 0; ui2.uFluxCapControl[19] = 0; ui2.uFluxCapControl[20] = 0; ui2.uFluxCapControl[21] = 0; ui2.uFluxCapControl[22] = 0; ui2.uFluxCapControl[23] = 0; ui2.uFluxCapControl[24] = 0; ui2.uFluxCapControl[25] = 0; ui2.uFluxCapControl[26] = 0; ui2.uFluxCapControl[27] = 0; ui2.uFluxCapControl[28] = 0; ui2.uFluxCapControl[29] = 0; ui2.uFluxCapControl[30] = 0; ui2.uFluxCapControl[31] = 0; ui2.uFluxCapControl[32] = 0; ui2.uFluxCapControl[33] = 0; ui2.uFluxCapControl[34] = 0; ui2.uFluxCapControl[35] = 0; ui2.uFluxCapControl[36] = 0; ui2.uFluxCapControl[37] = 0; ui2.uFluxCapControl[38] = 0; ui2.uFluxCapControl[39] = 0; ui2.uFluxCapControl[40] = 0; ui2.uFluxCapControl[41] = 0; ui2.uFluxCapControl[42] = 0; ui2.uFluxCapControl[43] = 0; ui2.uFluxCapControl[44] = 0; ui2.uFluxCapControl[45] = 0; ui2.uFluxCapControl[46] = 0; ui2.uFluxCapControl[47] = 0; ui2.uFluxCapControl[48] = 0; ui2.uFluxCapControl[49] = 0; ui2.uFluxCapControl[50] = 0; ui2.uFluxCapControl[51] = 0; ui2.uFluxCapControl[52] = 0; ui2.uFluxCapControl[53] = 0; ui2.uFluxCapControl[54] = 0; ui2.uFluxCapControl[55] = 0; ui2.uFluxCapControl[56] = 0; ui2.uFluxCapControl[57] = 0; ui2.uFluxCapControl[58] = 0; ui2.uFluxCapControl[59] = 0; ui2.uFluxCapControl[60] = 0; ui2.uFluxCapControl[61] = 0; ui2.uFluxCapControl[62] = 0; ui2.uFluxCapControl[63] = 0;
	ui2.fFluxCapData[0] = 0.000000; ui2.fFluxCapData[1] = 0.000000; ui2.fFluxCapData[2] = 0.000000; ui2.fFluxCapData[3] = 0.000000; ui2.fFluxCapData[4] = 0.000000; ui2.fFluxCapData[5] = 0.000000; ui2.fFluxCapData[6] = 0.000000; ui2.fFluxCapData[7] = 0.000000; ui2.fFluxCapData[8] = 0.000000; ui2.fFluxCapData[9] = 0.000000; ui2.fFluxCapData[10] = 0.000000; ui2.fFluxCapData[11] = 0.000000; ui2.fFluxCapData[12] = 0.000000; ui2.fFluxCapData[13] = 0.000000; ui2.fFluxCapData[14] = 0.000000; ui2.fFluxCapData[15] = 0.000000; ui2.fFluxCapData[16] = 0.000000; ui2.fFluxCapData[17] = 0.000000; ui2.fFluxCapData[18] = 0.000000; ui2.fFluxCapData[19] = 0.000000; ui2.fFluxCapData[20] = 0.000000; ui2.fFluxCapData[21] = 0.000000; ui2.fFluxCapData[22] = 0.000000; ui2.fFluxCapData[23] = 0.000000; ui2.fFluxCapData[24] = 0.000000; ui2.fFluxCapData[25] = 0.000000; ui2.fFluxCapData[26] = 0.000000; ui2.fFluxCapData[27] = 0.000000; ui2.fFluxCapData[28] = 0.000000; ui2.fFluxCapData[29] = 0.000000; ui2.fFluxCapData[30] = 0.000000; ui2.fFluxCapData[31] = 0.000000; ui2.fFluxCapData[32] = 0.000000; ui2.fFluxCapData[33] = 0.000000; ui2.fFluxCapData[34] = 0.000000; ui2.fFluxCapData[35] = 0.000000; ui2.fFluxCapData[36] = 0.000000; ui2.fFluxCapData[37] = 0.000000; ui2.fFluxCapData[38] = 0.000000; ui2.fFluxCapData[39] = 0.000000; ui2.fFluxCapData[40] = 0.000000; ui2.fFluxCapData[41] = 0.000000; ui2.fFluxCapData[42] = 0.000000; ui2.fFluxCapData[43] = 0.000000; ui2.fFluxCapData[44] = 0.000000; ui2.fFluxCapData[45] = 0.000000; ui2.fFluxCapData[46] = 0.000000; ui2.fFluxCapData[47] = 0.000000; ui2.fFluxCapData[48] = 0.000000; ui2.fFluxCapData[49] = 0.000000; ui2.fFluxCapData[50] = 0.000000; ui2.fFluxCapData[51] = 0.000000; ui2.fFluxCapData[52] = 0.000000; ui2.fFluxCapData[53] = 0.000000; ui2.fFluxCapData[54] = 0.000000; ui2.fFluxCapData[55] = 0.000000; ui2.fFluxCapData[56] = 0.000000; ui2.fFluxCapData[57] = 0.000000; ui2.fFluxCapData[58] = 0.000000; ui2.fFluxCapData[59] = 0.000000; ui2.fFluxCapData[60] = 0.000000; ui2.fFluxCapData[61] = 0.000000; ui2.fFluxCapData[62] = 0.000000; ui2.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui2);


	m_dLFO1Rate = 0.500000;
	CUICtrl ui3;
	ui3.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui3.uControlId = 3;
	ui3.bLogSlider = false;
	ui3.bExpSlider = false;
	ui3.fUserDisplayDataLoLimit = 0.020000;
	ui3.fUserDisplayDataHiLimit = 20.000000;
	ui3.uUserDataType = doubleData;
	ui3.fInitUserIntValue = 0;
	ui3.fInitUserFloatValue = 0;
	ui3.fInitUserDoubleValue = 0.500000;
	ui3.fInitUserUINTValue = 0;
	ui3.m_pUserCookedIntData = NULL;
	ui3.m_pUserCookedFloatData = NULL;
	ui3.m_pUserCookedDoubleData = &m_dLFO1Rate;
	ui3.m_pUserCookedUINTData = NULL;
	ui3.cControlUnits = "Hz                                                              ";
	ui3.cVariableName = "m_dLFO1Rate";
	ui3.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui3.dPresetData[0] = 0.500000;ui3.dPresetData[1] = 0.000000;ui3.dPresetData[2] = 0.000000;ui3.dPresetData[3] = 0.000000;ui3.dPresetData[4] = 0.000000;ui3.dPresetData[5] = 0.000000;ui3.dPresetData[6] = 0.000000;ui3.dPresetData[7] = 0.000000;ui3.dPresetData[8] = 0.000000;ui3.dPresetData[9] = 0.000000;ui3.dPresetData[10] = 0.000000;ui3.dPresetData[11] = 0.000000;ui3.dPresetData[12] = 0.000000;ui3.dPresetData[13] = 0.000000;ui3.dPresetData[14] = 0.000000;ui3.dPresetData[15] = 0.000000;
	ui3.cControlName = "LFO Rate";
	ui3.bOwnerControl = false;
	ui3.bMIDIControl = false;
	ui3.uMIDIControlCommand = 176;
	ui3.uMIDIControlName = 3;
	ui3.uMIDIControlChannel = 0;
	ui3.nGUIRow = -1;
	ui3.nGUIColumn = -1;
	ui3.uControlTheme[0] = 0; ui3.uControlTheme[1] = 0; ui3.uControlTheme[2] = 0; ui3.uControlTheme[3] = 0; ui3.uControlTheme[4] = 0; ui3.uControlTheme[5] = 0; ui3.uControlTheme[6] = 0; ui3.uControlTheme[7] = 0; ui3.uControlTheme[8] = 0; ui3.uControlTheme[9] = 0; ui3.uControlTheme[10] = 0; ui3.uControlTheme[11] = 0; ui3.uControlTheme[12] = 0; ui3.uControlTheme[13] = 0; ui3.uControlTheme[14] = 0; ui3.uControlTheme[15] = 0; ui3.uControlTheme[16] = 0; ui3.uControlTheme[17] = 0; ui3.uControlTheme[18] = 0; ui3.uControlTheme[19] = 0; ui3.uControlTheme[20] = 0; ui3.uControlTheme[21] = 0; ui3.uControlTheme[22] = 0; ui3.uControlTheme[23] = 0; ui3.uControlTheme[24] = 0; ui3.uControlTheme[25] = 0; ui3.uControlTheme[26] = 0; ui3.uControlTheme[27] = 0; ui3.uControlTheme[28] = 0; ui3.uControlTheme[29] = 0; ui3.uControlTheme[30] = 0; ui3.uControlTheme[31] = 0;
	ui3.uFluxCapControl[0] = 0; ui3.uFluxCapControl[1] = 0; ui3.uFluxCapControl[2] = 0; ui3.uFluxCapControl[3] = 0; ui3.uFluxCapControl[4] = 0; ui3.uFluxCapControl[5] = 0; ui3.uFluxCapControl[6] = 0; ui3.uFluxCapControl[7] = 0; ui3.uFluxCapControl[8] = 0; ui3.uFluxCapControl[9] = 0; ui3.uFluxCapControl[10] = 0; ui3.uFluxCapControl[11] = 0; ui3.uFluxCapControl[12] = 0; ui3.uFluxCapControl[13] = 0; ui3.uFluxCapControl[14] = 0; ui3.uFluxCapControl[15] = 0; ui3.uFluxCapControl[16] = 0; ui3.uFluxCapControl[17] = 0; ui3.uFluxCapControl[18] = 0; ui3.uFluxCapControl[19] = 0; ui3.uFluxCapControl[20] = 0; ui3.uFluxCapControl[21] = 0; ui3.uFluxCapControl[22] = 0; ui3.uFluxCapControl[23] = 0; ui3.uFluxCapControl[24] = 0; ui3.uFluxCapControl[25] = 0; ui3.uFluxCapControl[26] = 0; ui3.uFluxCapControl[27] = 0; ui3.uFluxCapControl[28] = 0; ui3.uFluxCapControl[29] = 0; ui3.uFluxCapControl[30] = 0; ui3.uFluxCapControl[31] = 0; ui3.uFluxCapControl[32] = 0; ui3.uFluxCapControl[33] = 0; ui3.uFluxCapControl[34] = 0; ui3.uFluxCapControl[35] = 0; ui3.uFluxCapControl[36] = 0; ui3.uFluxCapControl[37] = 0; ui3.uFluxCapControl[38] = 0; ui3.uFluxCapControl[39] = 0; ui3.uFluxCapControl[40] = 0; ui3.uFluxCapControl[41] = 0; ui3.uFluxCapControl[42] = 0; ui3.uFluxCapControl[43] = 0; ui3.uFluxCapControl[44] = 0; ui3.uFluxCapControl[45] = 0; ui3.uFluxCapControl[46] = 0; ui3.uFluxCapControl[47] = 0; ui3.uFluxCapControl[48] = 0; ui3.uFluxCapControl[49] = 0; ui3.uFluxCapControl[50] = 0; ui3.uFluxCapControl[51] = 0; ui3.uFluxCapControl[52] = 0; ui3.uFluxCapControl[53] = 0; ui3.uFluxCapControl[54] = 0; ui3.uFluxCapControl[55] = 0; ui3.uFluxCapControl[56] = 0; ui3.uFluxCapControl[57] = 0; ui3.uFluxCapControl[58] = 0; ui3.uFluxCapControl[59] = 0; ui3.uFluxCapControl[60] = 0; ui3.uFluxCapControl[61] = 0; ui3.uFluxCapControl[62] = 0; ui3.uFluxCapControl[63] = 0;
	ui3.fFluxCapData[0] = 0.000000; ui3.fFluxCapData[1] = 0.000000; ui3.fFluxCapData[2] = 0.000000; ui3.fFluxCapData[3] = 0.000000; ui3.fFluxCapData[4] = 0.000000; ui3.fFluxCapData[5] = 0.000000; ui3.fFluxCapData[6] = 0.000000; ui3.fFluxCapData[7] = 0.000000; ui3.fFluxCapData[8] = 0.000000; ui3.fFluxCapData[9] = 0.000000; ui3.fFluxCapData[10] = 0.000000; ui3.fFluxCapData[11] = 0.000000; ui3.fFluxCapData[12] = 0.000000; ui3.fFluxCapData[13] = 0.000000; ui3.fFluxCapData[14] = 0.000000; ui3.fFluxCapData[15] = 0.000000; ui3.fFluxCapData[16] = 0.000000; ui3.fFluxCapData[17] = 0.000000; ui3.fFluxCapData[18] = 0.000000; ui3.fFluxCapData[19] = 0.000000; ui3.fFluxCapData[20] = 0.000000; ui3.fFluxCapData[21] = 0.000000; ui3.fFluxCapData[22] = 0.000000; ui3.fFluxCapData[23] = 0.000000; ui3.fFluxCapData[24] = 0.000000; ui3.fFluxCapData[25] = 0.000000; ui3.fFluxCapData[26] = 0.000000; ui3.fFluxCapData[27] = 0.000000; ui3.fFluxCapData[28] = 0.000000; ui3.fFluxCapData[29] = 0.000000; ui3.fFluxCapData[30] = 0.000000; ui3.fFluxCapData[31] = 0.000000; ui3.fFluxCapData[32] = 0.000000; ui3.fFluxCapData[33] = 0.000000; ui3.fFluxCapData[34] = 0.000000; ui3.fFluxCapData[35] = 0.000000; ui3.fFluxCapData[36] = 0.000000; ui3.fFluxCapData[37] = 0.000000; ui3.fFluxCapData[38] = 0.000000; ui3.fFluxCapData[39] = 0.000000; ui3.fFluxCapData[40] = 0.000000; ui3.fFluxCapData[41] = 0.000000; ui3.fFluxCapData[42] = 0.000000; ui3.fFluxCapData[43] = 0.000000; ui3.fFluxCapData[44] = 0.000000; ui3.fFluxCapData[45] = 0.000000; ui3.fFluxCapData[46] = 0.000000; ui3.fFluxCapData[47] = 0.000000; ui3.fFluxCapData[48] = 0.000000; ui3.fFluxCapData[49] = 0.000000; ui3.fFluxCapData[50] = 0.000000; ui3.fFluxCapData[51] = 0.000000; ui3.fFluxCapData[52] = 0.000000; ui3.fFluxCapData[53] = 0.000000; ui3.fFluxCapData[54] = 0.000000; ui3.fFluxCapData[55] = 0.000000; ui3.fFluxCapData[56] = 0.000000; ui3.fFluxCapData[57] = 0.000000; ui3.fFluxCapData[58] = 0.000000; ui3.fFluxCapData[59] = 0.000000; ui3.fFluxCapData[60] = 0.000000; ui3.fFluxCapData[61] = 0.000000; ui3.fFluxCapData[62] = 0.000000; ui3.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui3);


	m_dAttackTime_mSec = 100.000000;
	CUICtrl ui4;
	ui4.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui4.uControlId = 4;
	ui4.bLogSlider = false;
	ui4.bExpSlider = false;
	ui4.fUserDisplayDataLoLimit = 0.000000;
	ui4.fUserDisplayDataHiLimit = 5000.000000;
	ui4.uUserDataType = doubleData;
	ui4.fInitUserIntValue = 0;
	ui4.fInitUserFloatValue = 0;
	ui4.fInitUserDoubleValue = 100.000000;
	ui4.fInitUserUINTValue = 0;
	ui4.m_pUserCookedIntData = NULL;
	ui4.m_pUserCookedFloatData = NULL;
	ui4.m_pUserCookedDoubleData = &m_dAttackTime_mSec;
	ui4.m_pUserCookedUINTData = NULL;
	ui4.cControlUnits = "mS                                                              ";
	ui4.cVariableName = "m_dAttackTime_mSec";
	ui4.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui4.dPresetData[0] = 100.000000;ui4.dPresetData[1] = 0.000000;ui4.dPresetData[2] = 0.000000;ui4.dPresetData[3] = 0.000000;ui4.dPresetData[4] = 0.000000;ui4.dPresetData[5] = 0.000000;ui4.dPresetData[6] = 0.000000;ui4.dPresetData[7] = 0.000000;ui4.dPresetData[8] = 0.000000;ui4.dPresetData[9] = 0.000000;ui4.dPresetData[10] = 0.000000;ui4.dPresetData[11] = 0.000000;ui4.dPresetData[12] = 0.000000;ui4.dPresetData[13] = 0.000000;ui4.dPresetData[14] = 0.000000;ui4.dPresetData[15] = 0.000000;
	ui4.cControlName = "Attack";
	ui4.bOwnerControl = false;
	ui4.bMIDIControl = false;
	ui4.uMIDIControlCommand = 176;
	ui4.uMIDIControlName = 3;
	ui4.uMIDIControlChannel = 0;
	ui4.nGUIRow = -1;
	ui4.nGUIColumn = -1;
	ui4.uControlTheme[0] = 0; ui4.uControlTheme[1] = 0; ui4.uControlTheme[2] = 0; ui4.uControlTheme[3] = 0; ui4.uControlTheme[4] = 0; ui4.uControlTheme[5] = 0; ui4.uControlTheme[6] = 0; ui4.uControlTheme[7] = 0; ui4.uControlTheme[8] = 0; ui4.uControlTheme[9] = 0; ui4.uControlTheme[10] = 0; ui4.uControlTheme[11] = 0; ui4.uControlTheme[12] = 0; ui4.uControlTheme[13] = 0; ui4.uControlTheme[14] = 0; ui4.uControlTheme[15] = 0; ui4.uControlTheme[16] = 0; ui4.uControlTheme[17] = 0; ui4.uControlTheme[18] = 0; ui4.uControlTheme[19] = 0; ui4.uControlTheme[20] = 0; ui4.uControlTheme[21] = 0; ui4.uControlTheme[22] = 0; ui4.uControlTheme[23] = 0; ui4.uControlTheme[24] = 0; ui4.uControlTheme[25] = 0; ui4.uControlTheme[26] = 0; ui4.uControlTheme[27] = 0; ui4.uControlTheme[28] = 0; ui4.uControlTheme[29] = 0; ui4.uControlTheme[30] = 0; ui4.uControlTheme[31] = 0;
	ui4.uFluxCapControl[0] = 0; ui4.uFluxCapControl[1] = 0; ui4.uFluxCapControl[2] = 0; ui4.uFluxCapControl[3] = 0; ui4.uFluxCapControl[4] = 0; ui4.uFluxCapControl[5] = 0; ui4.uFluxCapControl[6] = 0; ui4.uFluxCapControl[7] = 0; ui4.uFluxCapControl[8] = 0; ui4.uFluxCapControl[9] = 0; ui4.uFluxCapControl[10] = 0; ui4.uFluxCapControl[11] = 0; ui4.uFluxCapControl[12] = 0; ui4.uFluxCapControl[13] = 0; ui4.uFluxCapControl[14] = 0; ui4.uFluxCapControl[15] = 0; ui4.uFluxCapControl[16] = 0; ui4.uFluxCapControl[17] = 0; ui4.uFluxCapControl[18] = 0; ui4.uFluxCapControl[19] = 0; ui4.uFluxCapControl[20] = 0; ui4.uFluxCapControl[21] = 0; ui4.uFluxCapControl[22] = 0; ui4.uFluxCapControl[23] = 0; ui4.uFluxCapControl[24] = 0; ui4.uFluxCapControl[25] = 0; ui4.uFluxCapControl[26] = 0; ui4.uFluxCapControl[27] = 0; ui4.uFluxCapControl[28] = 0; ui4.uFluxCapControl[29] = 0; ui4.uFluxCapControl[30] = 0; ui4.uFluxCapControl[31] = 0; ui4.uFluxCapControl[32] = 0; ui4.uFluxCapControl[33] = 0; ui4.uFluxCapControl[34] = 0; ui4.uFluxCapControl[35] = 0; ui4.uFluxCapControl[36] = 0; ui4.uFluxCapControl[37] = 0; ui4.uFluxCapControl[38] = 0; ui4.uFluxCapControl[39] = 0; ui4.uFluxCapControl[40] = 0; ui4.uFluxCapControl[41] = 0; ui4.uFluxCapControl[42] = 0; ui4.uFluxCapControl[43] = 0; ui4.uFluxCapControl[44] = 0; ui4.uFluxCapControl[45] = 0; ui4.uFluxCapControl[46] = 0; ui4.uFluxCapControl[47] = 0; ui4.uFluxCapControl[48] = 0; ui4.uFluxCapControl[49] = 0; ui4.uFluxCapControl[50] = 0; ui4.uFluxCapControl[51] = 0; ui4.uFluxCapControl[52] = 0; ui4.uFluxCapControl[53] = 0; ui4.uFluxCapControl[54] = 0; ui4.uFluxCapControl[55] = 0; ui4.uFluxCapControl[56] = 0; ui4.uFluxCapControl[57] = 0; ui4.uFluxCapControl[58] = 0; ui4.uFluxCapControl[59] = 0; ui4.uFluxCapControl[60] = 0; ui4.uFluxCapControl[61] = 0; ui4.uFluxCapControl[62] = 0; ui4.uFluxCapControl[63] = 0;
	ui4.fFluxCapData[0] = 0.000000; ui4.fFluxCapData[1] = 0.000000; ui4.fFluxCapData[2] = 0.000000; ui4.fFluxCapData[3] = 0.000000; ui4.fFluxCapData[4] = 0.000000; ui4.fFluxCapData[5] = 0.000000; ui4.fFluxCapData[6] = 0.000000; ui4.fFluxCapData[7] = 0.000000; ui4.fFluxCapData[8] = 0.000000; ui4.fFluxCapData[9] = 0.000000; ui4.fFluxCapData[10] = 0.000000; ui4.fFluxCapData[11] = 0.000000; ui4.fFluxCapData[12] = 0.000000; ui4.fFluxCapData[13] = 0.000000; ui4.fFluxCapData[14] = 0.000000; ui4.fFluxCapData[15] = 0.000000; ui4.fFluxCapData[16] = 0.000000; ui4.fFluxCapData[17] = 0.000000; ui4.fFluxCapData[18] = 0.000000; ui4.fFluxCapData[19] = 0.000000; ui4.fFluxCapData[20] = 0.000000; ui4.fFluxCapData[21] = 0.000000; ui4.fFluxCapData[22] = 0.000000; ui4.fFluxCapData[23] = 0.000000; ui4.fFluxCapData[24] = 0.000000; ui4.fFluxCapData[25] = 0.000000; ui4.fFluxCapData[26] = 0.000000; ui4.fFluxCapData[27] = 0.000000; ui4.fFluxCapData[28] = 0.000000; ui4.fFluxCapData[29] = 0.000000; ui4.fFluxCapData[30] = 0.000000; ui4.fFluxCapData[31] = 0.000000; ui4.fFluxCapData[32] = 0.000000; ui4.fFluxCapData[33] = 0.000000; ui4.fFluxCapData[34] = 0.000000; ui4.fFluxCapData[35] = 0.000000; ui4.fFluxCapData[36] = 0.000000; ui4.fFluxCapData[37] = 0.000000; ui4.fFluxCapData[38] = 0.000000; ui4.fFluxCapData[39] = 0.000000; ui4.fFluxCapData[40] = 0.000000; ui4.fFluxCapData[41] = 0.000000; ui4.fFluxCapData[42] = 0.000000; ui4.fFluxCapData[43] = 0.000000; ui4.fFluxCapData[44] = 0.000000; ui4.fFluxCapData[45] = 0.000000; ui4.fFluxCapData[46] = 0.000000; ui4.fFluxCapData[47] = 0.000000; ui4.fFluxCapData[48] = 0.000000; ui4.fFluxCapData[49] = 0.000000; ui4.fFluxCapData[50] = 0.000000; ui4.fFluxCapData[51] = 0.000000; ui4.fFluxCapData[52] = 0.000000; ui4.fFluxCapData[53] = 0.000000; ui4.fFluxCapData[54] = 0.000000; ui4.fFluxCapData[55] = 0.000000; ui4.fFluxCapData[56] = 0.000000; ui4.fFluxCapData[57] = 0.000000; ui4.fFluxCapData[58] = 0.000000; ui4.fFluxCapData[59] = 0.000000; ui4.fFluxCapData[60] = 0.000000; ui4.fFluxCapData[61] = 0.000000; ui4.fFluxCapData[62] = 0.000000; ui4.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui4);


	m_dOrbitX = 0.000000;
	CUICtrl ui5;
	ui5.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui5.uControlId = 5;
	ui5.bLogSlider = false;
	ui5.bExpSlider = false;
	ui5.fUserDisplayDataLoLimit = 0.000000;
	ui5.fUserDisplayDataHiLimit = 1.000000;
	ui5.uUserDataType = doubleData;
	ui5.fInitUserIntValue = 0;
	ui5.fInitUserFloatValue = 0;
	ui5.fInitUserDoubleValue = 0.000000;
	ui5.fInitUserUINTValue = 0;
	ui5.m_pUserCookedIntData = NULL;
	ui5.m_pUserCookedFloatData = NULL;
	ui5.m_pUserCookedDoubleData = &m_dOrbitX;
	ui5.m_pUserCookedUINTData = NULL;
	ui5.cControlUnits = "                                                                ";
	ui5.cVariableName = "m_dOrbitX";
	ui5.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui5.dPresetData[0] = 0.000000;ui5.dPresetData[1] = 0.000000;ui5.dPresetData[2] = 0.000000;ui5.dPresetData[3] = 0.000000;ui5.dPresetData[4] = 0.000000;ui5.dPresetData[5] = 0.000000;ui5.dPresetData[6] = 0.000000;ui5.dPresetData[7] = 0.000000;ui5.dPresetData[8] = 0.000000;ui5.dPresetData[9] = 0.000000;ui5.dPresetData[10] = 0.000000;ui5.dPresetData[11] = 0.000000;ui5.dPresetData[12] = 0.000000;ui5.dPresetData[13] = 0.000000;ui5.dPresetData[14] = 0.000000;ui5.dPresetData[15] = 0.000000;
	ui5.cControlName = "Orbit X";
	ui5.bOwnerControl = false;
	ui5.bMIDIControl = false;
	ui5.uMIDIControlCommand = 176;
	ui5.uMIDIControlName = 3;
	ui5.uMIDIControlChannel = 0;
	ui5.nGUIRow = -1;
	ui5.nGUIColumn = -1;
	ui5.uControlTheme[0] = 0; ui5.uControlTheme[1] = 0; ui5.uControlTheme[2] = 0; ui5.uControlTheme[3] = 0; ui5.uControlTheme[4] = 0; ui5.uControlTheme[5] = 0; ui5.uControlTheme[6] = 0; ui5.uControlTheme[7] = 0; ui5.uControlTheme[8] = 0; ui5.uControlTheme[9] = 0; ui5.uControlTheme[10] = 0; ui5.uControlTheme[11] = 0; ui5.uControlTheme[12] = 0; ui5.uControlTheme[13] = 0; ui5.uControlTheme[14] = 0; ui5.uControlTheme[15] = 0; ui5.uControlTheme[16] = 0; ui5.uControlTheme[17] = 0; ui5.uControlTheme[18] = 0; ui5.uControlTheme[19] = 0; ui5.uControlTheme[20] = 0; ui5.uControlTheme[21] = 0; ui5.uControlTheme[22] = 0; ui5.uControlTheme[23] = 0; ui5.uControlTheme[24] = 0; ui5.uControlTheme[25] = 0; ui5.uControlTheme[26] = 0; ui5.uControlTheme[27] = 0; ui5.uControlTheme[28] = 0; ui5.uControlTheme[29] = 0; ui5.uControlTheme[30] = 0; ui5.uControlTheme[31] = 0;
	ui5.uFluxCapControl[0] = 0; ui5.uFluxCapControl[1] = 0; ui5.uFluxCapControl[2] = 0; ui5.uFluxCapControl[3] = 0; ui5.uFluxCapControl[4] = 0; ui5.uFluxCapControl[5] = 0; ui5.uFluxCapControl[6] = 0; ui5.uFluxCapControl[7] = 0; ui5.uFluxCapControl[8] = 0; ui5.uFluxCapControl[9] = 0; ui5.uFluxCapControl[10] = 0; ui5.uFluxCapControl[11] = 0; ui5.uFluxCapControl[12] = 0; ui5.uFluxCapControl[13] = 0; ui5.uFluxCapControl[14] = 0; ui5.uFluxCapControl[15] = 0; ui5.uFluxCapControl[16] = 0; ui5.uFluxCapControl[17] = 0; ui5.uFluxCapControl[18] = 0; ui5.uFluxCapControl[19] = 0; ui5.uFluxCapControl[20] = 0; ui5.uFluxCapControl[21] = 0; ui5.uFluxCapControl[22] = 0; ui5.uFluxCapControl[23] = 0; ui5.uFluxCapControl[24] = 0; ui5.uFluxCapControl[25] = 0; ui5.uFluxCapControl[26] = 0; ui5.uFluxCapControl[27] = 0; ui5.uFluxCapControl[28] = 0; ui5.uFluxCapControl[29] = 0; ui5.uFluxCapControl[30] = 0; ui5.uFluxCapControl[31] = 0; ui5.uFluxCapControl[32] = 0; ui5.uFluxCapControl[33] = 0; ui5.uFluxCapControl[34] = 0; ui5.uFluxCapControl[35] = 0; ui5.uFluxCapControl[36] = 0; ui5.uFluxCapControl[37] = 0; ui5.uFluxCapControl[38] = 0; ui5.uFluxCapControl[39] = 0; ui5.uFluxCapControl[40] = 0; ui5.uFluxCapControl[41] = 0; ui5.uFluxCapControl[42] = 0; ui5.uFluxCapControl[43] = 0; ui5.uFluxCapControl[44] = 0; ui5.uFluxCapControl[45] = 0; ui5.uFluxCapControl[46] = 0; ui5.uFluxCapControl[47] = 0; ui5.uFluxCapControl[48] = 0; ui5.uFluxCapControl[49] = 0; ui5.uFluxCapControl[50] = 0; ui5.uFluxCapControl[51] = 0; ui5.uFluxCapControl[52] = 0; ui5.uFluxCapControl[53] = 0; ui5.uFluxCapControl[54] = 0; ui5.uFluxCapControl[55] = 0; ui5.uFluxCapControl[56] = 0; ui5.uFluxCapControl[57] = 0; ui5.uFluxCapControl[58] = 0; ui5.uFluxCapControl[59] = 0; ui5.uFluxCapControl[60] = 0; ui5.uFluxCapControl[61] = 0; ui5.uFluxCapControl[62] = 0; ui5.uFluxCapControl[63] = 0;
	ui5.fFluxCapData[0] = 0.000000; ui5.fFluxCapData[1] = 0.000000; ui5.fFluxCapData[2] = 0.000000; ui5.fFluxCapData[3] = 0.000000; ui5.fFluxCapData[4] = 0.000000; ui5.fFluxCapData[5] = 0.000000; ui5.fFluxCapData[6] = 0.000000; ui5.fFluxCapData[7] = 0.000000; ui5.fFluxCapData[8] = 0.000000; ui5.fFluxCapData[9] = 0.000000; ui5.fFluxCapData[10] = 0.000000; ui5.fFluxCapData[11] = 0.000000; ui5.fFluxCapData[12] = 0.000000; ui5.fFluxCapData[13] = 0.000000; ui5.fFluxCapData[14] = 0.000000; ui5.fFluxCapData[15] = 0.000000; ui5.fFluxCapData[16] = 0.000000; ui5.fFluxCapData[17] = 0.000000; ui5.fFluxCapData[18] = 0.000000; ui5.fFluxCapData[19] = 0.000000; ui5.fFluxCapData[20] = 0.000000; ui5.fFluxCapData[21] = 0.000000; ui5.fFluxCapData[22] = 0.000000; ui5.fFluxCapData[23] = 0.000000; ui5.fFluxCapData[24] = 0.000000; ui5.fFluxCapData[25] = 0.000000; ui5.fFluxCapData[26] = 0.000000; ui5.fFluxCapData[27] = 0.000000; ui5.fFluxCapData[28] = 0.000000; ui5.fFluxCapData[29] = 0.000000; ui5.fFluxCapData[30] = 0.000000; ui5.fFluxCapData[31] = 0.000000; ui5.fFluxCapData[32] = 0.000000; ui5.fFluxCapData[33] = 0.000000; ui5.fFluxCapData[34] = 0.000000; ui5.fFluxCapData[35] = 0.000000; ui5.fFluxCapData[36] = 0.000000; ui5.fFluxCapData[37] = 0.000000; ui5.fFluxCapData[38] = 0.000000; ui5.fFluxCapData[39] = 0.000000; ui5.fFluxCapData[40] = 0.000000; ui5.fFluxCapData[41] = 0.000000; ui5.fFluxCapData[42] = 0.000000; ui5.fFluxCapData[43] = 0.000000; ui5.fFluxCapData[44] = 0.000000; ui5.fFluxCapData[45] = 0.000000; ui5.fFluxCapData[46] = 0.000000; ui5.fFluxCapData[47] = 0.000000; ui5.fFluxCapData[48] = 0.000000; ui5.fFluxCapData[49] = 0.000000; ui5.fFluxCapData[50] = 0.000000; ui5.fFluxCapData[51] = 0.000000; ui5.fFluxCapData[52] = 0.000000; ui5.fFluxCapData[53] = 0.000000; ui5.fFluxCapData[54] = 0.000000; ui5.fFluxCapData[55] = 0.000000; ui5.fFluxCapData[56] = 0.000000; ui5.fFluxCapData[57] = 0.000000; ui5.fFluxCapData[58] = 0.000000; ui5.fFluxCapData[59] = 0.000000; ui5.fFluxCapData[60] = 0.000000; ui5.fFluxCapData[61] = 0.000000; ui5.fFluxCapData[62] = 0.000000; ui5.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui5);


	m_dOsc1Amplitude_dB = 0.000000;
	CUICtrl ui6;
	ui6.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui6.uControlId = 6;
	ui6.bLogSlider = false;
	ui6.bExpSlider = false;
	ui6.fUserDisplayDataLoLimit = -96.000000;
	ui6.fUserDisplayDataHiLimit = 24.000000;
	ui6.uUserDataType = doubleData;
	ui6.fInitUserIntValue = 0;
	ui6.fInitUserFloatValue = 0;
	ui6.fInitUserDoubleValue = 0.000000;
	ui6.fInitUserUINTValue = 0;
	ui6.m_pUserCookedIntData = NULL;
	ui6.m_pUserCookedFloatData = NULL;
	ui6.m_pUserCookedDoubleData = &m_dOsc1Amplitude_dB;
	ui6.m_pUserCookedUINTData = NULL;
	ui6.cControlUnits = "dB                                                              ";
	ui6.cVariableName = "m_dOsc1Amplitude_dB";
	ui6.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui6.dPresetData[0] = 0.000000;ui6.dPresetData[1] = 0.000000;ui6.dPresetData[2] = 0.000000;ui6.dPresetData[3] = 0.000000;ui6.dPresetData[4] = 0.000000;ui6.dPresetData[5] = 0.000000;ui6.dPresetData[6] = 0.000000;ui6.dPresetData[7] = 0.000000;ui6.dPresetData[8] = 0.000000;ui6.dPresetData[9] = 0.000000;ui6.dPresetData[10] = 0.000000;ui6.dPresetData[11] = 0.000000;ui6.dPresetData[12] = 0.000000;ui6.dPresetData[13] = 0.000000;ui6.dPresetData[14] = 0.000000;ui6.dPresetData[15] = 0.000000;
	ui6.cControlName = "A-C Output";
	ui6.bOwnerControl = false;
	ui6.bMIDIControl = false;
	ui6.uMIDIControlCommand = 176;
	ui6.uMIDIControlName = 3;
	ui6.uMIDIControlChannel = 0;
	ui6.nGUIRow = -1;
	ui6.nGUIColumn = -1;
	ui6.uControlTheme[0] = 0; ui6.uControlTheme[1] = 0; ui6.uControlTheme[2] = 0; ui6.uControlTheme[3] = 0; ui6.uControlTheme[4] = 0; ui6.uControlTheme[5] = 0; ui6.uControlTheme[6] = 0; ui6.uControlTheme[7] = 0; ui6.uControlTheme[8] = 0; ui6.uControlTheme[9] = 0; ui6.uControlTheme[10] = 0; ui6.uControlTheme[11] = 0; ui6.uControlTheme[12] = 0; ui6.uControlTheme[13] = 0; ui6.uControlTheme[14] = 0; ui6.uControlTheme[15] = 0; ui6.uControlTheme[16] = 0; ui6.uControlTheme[17] = 0; ui6.uControlTheme[18] = 0; ui6.uControlTheme[19] = 0; ui6.uControlTheme[20] = 0; ui6.uControlTheme[21] = 0; ui6.uControlTheme[22] = 0; ui6.uControlTheme[23] = 0; ui6.uControlTheme[24] = 0; ui6.uControlTheme[25] = 0; ui6.uControlTheme[26] = 0; ui6.uControlTheme[27] = 0; ui6.uControlTheme[28] = 0; ui6.uControlTheme[29] = 0; ui6.uControlTheme[30] = 0; ui6.uControlTheme[31] = 0;
	ui6.uFluxCapControl[0] = 0; ui6.uFluxCapControl[1] = 0; ui6.uFluxCapControl[2] = 0; ui6.uFluxCapControl[3] = 0; ui6.uFluxCapControl[4] = 0; ui6.uFluxCapControl[5] = 0; ui6.uFluxCapControl[6] = 0; ui6.uFluxCapControl[7] = 0; ui6.uFluxCapControl[8] = 0; ui6.uFluxCapControl[9] = 0; ui6.uFluxCapControl[10] = 0; ui6.uFluxCapControl[11] = 0; ui6.uFluxCapControl[12] = 0; ui6.uFluxCapControl[13] = 0; ui6.uFluxCapControl[14] = 0; ui6.uFluxCapControl[15] = 0; ui6.uFluxCapControl[16] = 0; ui6.uFluxCapControl[17] = 0; ui6.uFluxCapControl[18] = 0; ui6.uFluxCapControl[19] = 0; ui6.uFluxCapControl[20] = 0; ui6.uFluxCapControl[21] = 0; ui6.uFluxCapControl[22] = 0; ui6.uFluxCapControl[23] = 0; ui6.uFluxCapControl[24] = 0; ui6.uFluxCapControl[25] = 0; ui6.uFluxCapControl[26] = 0; ui6.uFluxCapControl[27] = 0; ui6.uFluxCapControl[28] = 0; ui6.uFluxCapControl[29] = 0; ui6.uFluxCapControl[30] = 0; ui6.uFluxCapControl[31] = 0; ui6.uFluxCapControl[32] = 0; ui6.uFluxCapControl[33] = 0; ui6.uFluxCapControl[34] = 0; ui6.uFluxCapControl[35] = 0; ui6.uFluxCapControl[36] = 0; ui6.uFluxCapControl[37] = 0; ui6.uFluxCapControl[38] = 0; ui6.uFluxCapControl[39] = 0; ui6.uFluxCapControl[40] = 0; ui6.uFluxCapControl[41] = 0; ui6.uFluxCapControl[42] = 0; ui6.uFluxCapControl[43] = 0; ui6.uFluxCapControl[44] = 0; ui6.uFluxCapControl[45] = 0; ui6.uFluxCapControl[46] = 0; ui6.uFluxCapControl[47] = 0; ui6.uFluxCapControl[48] = 0; ui6.uFluxCapControl[49] = 0; ui6.uFluxCapControl[50] = 0; ui6.uFluxCapControl[51] = 0; ui6.uFluxCapControl[52] = 0; ui6.uFluxCapControl[53] = 0; ui6.uFluxCapControl[54] = 0; ui6.uFluxCapControl[55] = 0; ui6.uFluxCapControl[56] = 0; ui6.uFluxCapControl[57] = 0; ui6.uFluxCapControl[58] = 0; ui6.uFluxCapControl[59] = 0; ui6.uFluxCapControl[60] = 0; ui6.uFluxCapControl[61] = 0; ui6.uFluxCapControl[62] = 0; ui6.uFluxCapControl[63] = 0;
	ui6.fFluxCapData[0] = 0.000000; ui6.fFluxCapData[1] = 0.000000; ui6.fFluxCapData[2] = 0.000000; ui6.fFluxCapData[3] = 0.000000; ui6.fFluxCapData[4] = 0.000000; ui6.fFluxCapData[5] = 0.000000; ui6.fFluxCapData[6] = 0.000000; ui6.fFluxCapData[7] = 0.000000; ui6.fFluxCapData[8] = 0.000000; ui6.fFluxCapData[9] = 0.000000; ui6.fFluxCapData[10] = 0.000000; ui6.fFluxCapData[11] = 0.000000; ui6.fFluxCapData[12] = 0.000000; ui6.fFluxCapData[13] = 0.000000; ui6.fFluxCapData[14] = 0.000000; ui6.fFluxCapData[15] = 0.000000; ui6.fFluxCapData[16] = 0.000000; ui6.fFluxCapData[17] = 0.000000; ui6.fFluxCapData[18] = 0.000000; ui6.fFluxCapData[19] = 0.000000; ui6.fFluxCapData[20] = 0.000000; ui6.fFluxCapData[21] = 0.000000; ui6.fFluxCapData[22] = 0.000000; ui6.fFluxCapData[23] = 0.000000; ui6.fFluxCapData[24] = 0.000000; ui6.fFluxCapData[25] = 0.000000; ui6.fFluxCapData[26] = 0.000000; ui6.fFluxCapData[27] = 0.000000; ui6.fFluxCapData[28] = 0.000000; ui6.fFluxCapData[29] = 0.000000; ui6.fFluxCapData[30] = 0.000000; ui6.fFluxCapData[31] = 0.000000; ui6.fFluxCapData[32] = 0.000000; ui6.fFluxCapData[33] = 0.000000; ui6.fFluxCapData[34] = 0.000000; ui6.fFluxCapData[35] = 0.000000; ui6.fFluxCapData[36] = 0.000000; ui6.fFluxCapData[37] = 0.000000; ui6.fFluxCapData[38] = 0.000000; ui6.fFluxCapData[39] = 0.000000; ui6.fFluxCapData[40] = 0.000000; ui6.fFluxCapData[41] = 0.000000; ui6.fFluxCapData[42] = 0.000000; ui6.fFluxCapData[43] = 0.000000; ui6.fFluxCapData[44] = 0.000000; ui6.fFluxCapData[45] = 0.000000; ui6.fFluxCapData[46] = 0.000000; ui6.fFluxCapData[47] = 0.000000; ui6.fFluxCapData[48] = 0.000000; ui6.fFluxCapData[49] = 0.000000; ui6.fFluxCapData[50] = 0.000000; ui6.fFluxCapData[51] = 0.000000; ui6.fFluxCapData[52] = 0.000000; ui6.fFluxCapData[53] = 0.000000; ui6.fFluxCapData[54] = 0.000000; ui6.fFluxCapData[55] = 0.000000; ui6.fFluxCapData[56] = 0.000000; ui6.fFluxCapData[57] = 0.000000; ui6.fFluxCapData[58] = 0.000000; ui6.fFluxCapData[59] = 0.000000; ui6.fFluxCapData[60] = 0.000000; ui6.fFluxCapData[61] = 0.000000; ui6.fFluxCapData[62] = 0.000000; ui6.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui6);


	m_nOctave = 0;
	CUICtrl ui7;
	ui7.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui7.uControlId = 10;
	ui7.bLogSlider = false;
	ui7.bExpSlider = false;
	ui7.fUserDisplayDataLoLimit = -4.000000;
	ui7.fUserDisplayDataHiLimit = 4.000000;
	ui7.uUserDataType = intData;
	ui7.fInitUserIntValue = 0.000000;
	ui7.fInitUserFloatValue = 0;
	ui7.fInitUserDoubleValue = 0;
	ui7.fInitUserUINTValue = 0;
	ui7.m_pUserCookedIntData = &m_nOctave;
	ui7.m_pUserCookedFloatData = NULL;
	ui7.m_pUserCookedDoubleData = NULL;
	ui7.m_pUserCookedUINTData = NULL;
	ui7.cControlUnits = "                                                                ";
	ui7.cVariableName = "m_nOctave";
	ui7.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui7.dPresetData[0] = 0.000000;ui7.dPresetData[1] = 0.000000;ui7.dPresetData[2] = 0.000000;ui7.dPresetData[3] = 0.000000;ui7.dPresetData[4] = 0.000000;ui7.dPresetData[5] = 0.000000;ui7.dPresetData[6] = 0.000000;ui7.dPresetData[7] = 0.000000;ui7.dPresetData[8] = 0.000000;ui7.dPresetData[9] = 0.000000;ui7.dPresetData[10] = 0.000000;ui7.dPresetData[11] = 0.000000;ui7.dPresetData[12] = 0.000000;ui7.dPresetData[13] = 0.000000;ui7.dPresetData[14] = 0.000000;ui7.dPresetData[15] = 0.000000;
	ui7.cControlName = "Octave";
	ui7.bOwnerControl = false;
	ui7.bMIDIControl = false;
	ui7.uMIDIControlCommand = 176;
	ui7.uMIDIControlName = 3;
	ui7.uMIDIControlChannel = 0;
	ui7.nGUIRow = -1;
	ui7.nGUIColumn = -1;
	ui7.uControlTheme[0] = 0; ui7.uControlTheme[1] = 0; ui7.uControlTheme[2] = 0; ui7.uControlTheme[3] = 0; ui7.uControlTheme[4] = 0; ui7.uControlTheme[5] = 0; ui7.uControlTheme[6] = 0; ui7.uControlTheme[7] = 0; ui7.uControlTheme[8] = 0; ui7.uControlTheme[9] = 0; ui7.uControlTheme[10] = 0; ui7.uControlTheme[11] = 0; ui7.uControlTheme[12] = 0; ui7.uControlTheme[13] = 0; ui7.uControlTheme[14] = 0; ui7.uControlTheme[15] = 0; ui7.uControlTheme[16] = 0; ui7.uControlTheme[17] = 0; ui7.uControlTheme[18] = 0; ui7.uControlTheme[19] = 0; ui7.uControlTheme[20] = 0; ui7.uControlTheme[21] = 0; ui7.uControlTheme[22] = 0; ui7.uControlTheme[23] = 0; ui7.uControlTheme[24] = 0; ui7.uControlTheme[25] = 0; ui7.uControlTheme[26] = 0; ui7.uControlTheme[27] = 0; ui7.uControlTheme[28] = 0; ui7.uControlTheme[29] = 0; ui7.uControlTheme[30] = 0; ui7.uControlTheme[31] = 0;
	ui7.uFluxCapControl[0] = 0; ui7.uFluxCapControl[1] = 0; ui7.uFluxCapControl[2] = 0; ui7.uFluxCapControl[3] = 0; ui7.uFluxCapControl[4] = 0; ui7.uFluxCapControl[5] = 0; ui7.uFluxCapControl[6] = 0; ui7.uFluxCapControl[7] = 0; ui7.uFluxCapControl[8] = 0; ui7.uFluxCapControl[9] = 0; ui7.uFluxCapControl[10] = 0; ui7.uFluxCapControl[11] = 0; ui7.uFluxCapControl[12] = 0; ui7.uFluxCapControl[13] = 0; ui7.uFluxCapControl[14] = 0; ui7.uFluxCapControl[15] = 0; ui7.uFluxCapControl[16] = 0; ui7.uFluxCapControl[17] = 0; ui7.uFluxCapControl[18] = 0; ui7.uFluxCapControl[19] = 0; ui7.uFluxCapControl[20] = 0; ui7.uFluxCapControl[21] = 0; ui7.uFluxCapControl[22] = 0; ui7.uFluxCapControl[23] = 0; ui7.uFluxCapControl[24] = 0; ui7.uFluxCapControl[25] = 0; ui7.uFluxCapControl[26] = 0; ui7.uFluxCapControl[27] = 0; ui7.uFluxCapControl[28] = 0; ui7.uFluxCapControl[29] = 0; ui7.uFluxCapControl[30] = 0; ui7.uFluxCapControl[31] = 0; ui7.uFluxCapControl[32] = 0; ui7.uFluxCapControl[33] = 0; ui7.uFluxCapControl[34] = 0; ui7.uFluxCapControl[35] = 0; ui7.uFluxCapControl[36] = 0; ui7.uFluxCapControl[37] = 0; ui7.uFluxCapControl[38] = 0; ui7.uFluxCapControl[39] = 0; ui7.uFluxCapControl[40] = 0; ui7.uFluxCapControl[41] = 0; ui7.uFluxCapControl[42] = 0; ui7.uFluxCapControl[43] = 0; ui7.uFluxCapControl[44] = 0; ui7.uFluxCapControl[45] = 0; ui7.uFluxCapControl[46] = 0; ui7.uFluxCapControl[47] = 0; ui7.uFluxCapControl[48] = 0; ui7.uFluxCapControl[49] = 0; ui7.uFluxCapControl[50] = 0; ui7.uFluxCapControl[51] = 0; ui7.uFluxCapControl[52] = 0; ui7.uFluxCapControl[53] = 0; ui7.uFluxCapControl[54] = 0; ui7.uFluxCapControl[55] = 0; ui7.uFluxCapControl[56] = 0; ui7.uFluxCapControl[57] = 0; ui7.uFluxCapControl[58] = 0; ui7.uFluxCapControl[59] = 0; ui7.uFluxCapControl[60] = 0; ui7.uFluxCapControl[61] = 0; ui7.uFluxCapControl[62] = 0; ui7.uFluxCapControl[63] = 0;
	ui7.fFluxCapData[0] = 0.000000; ui7.fFluxCapData[1] = 0.000000; ui7.fFluxCapData[2] = 0.000000; ui7.fFluxCapData[3] = 0.000000; ui7.fFluxCapData[4] = 0.000000; ui7.fFluxCapData[5] = 0.000000; ui7.fFluxCapData[6] = 0.000000; ui7.fFluxCapData[7] = 0.000000; ui7.fFluxCapData[8] = 0.000000; ui7.fFluxCapData[9] = 0.000000; ui7.fFluxCapData[10] = 0.000000; ui7.fFluxCapData[11] = 0.000000; ui7.fFluxCapData[12] = 0.000000; ui7.fFluxCapData[13] = 0.000000; ui7.fFluxCapData[14] = 0.000000; ui7.fFluxCapData[15] = 0.000000; ui7.fFluxCapData[16] = 0.000000; ui7.fFluxCapData[17] = 0.000000; ui7.fFluxCapData[18] = 0.000000; ui7.fFluxCapData[19] = 0.000000; ui7.fFluxCapData[20] = 0.000000; ui7.fFluxCapData[21] = 0.000000; ui7.fFluxCapData[22] = 0.000000; ui7.fFluxCapData[23] = 0.000000; ui7.fFluxCapData[24] = 0.000000; ui7.fFluxCapData[25] = 0.000000; ui7.fFluxCapData[26] = 0.000000; ui7.fFluxCapData[27] = 0.000000; ui7.fFluxCapData[28] = 0.000000; ui7.fFluxCapData[29] = 0.000000; ui7.fFluxCapData[30] = 0.000000; ui7.fFluxCapData[31] = 0.000000; ui7.fFluxCapData[32] = 0.000000; ui7.fFluxCapData[33] = 0.000000; ui7.fFluxCapData[34] = 0.000000; ui7.fFluxCapData[35] = 0.000000; ui7.fFluxCapData[36] = 0.000000; ui7.fFluxCapData[37] = 0.000000; ui7.fFluxCapData[38] = 0.000000; ui7.fFluxCapData[39] = 0.000000; ui7.fFluxCapData[40] = 0.000000; ui7.fFluxCapData[41] = 0.000000; ui7.fFluxCapData[42] = 0.000000; ui7.fFluxCapData[43] = 0.000000; ui7.fFluxCapData[44] = 0.000000; ui7.fFluxCapData[45] = 0.000000; ui7.fFluxCapData[46] = 0.000000; ui7.fFluxCapData[47] = 0.000000; ui7.fFluxCapData[48] = 0.000000; ui7.fFluxCapData[49] = 0.000000; ui7.fFluxCapData[50] = 0.000000; ui7.fFluxCapData[51] = 0.000000; ui7.fFluxCapData[52] = 0.000000; ui7.fFluxCapData[53] = 0.000000; ui7.fFluxCapData[54] = 0.000000; ui7.fFluxCapData[55] = 0.000000; ui7.fFluxCapData[56] = 0.000000; ui7.fFluxCapData[57] = 0.000000; ui7.fFluxCapData[58] = 0.000000; ui7.fFluxCapData[59] = 0.000000; ui7.fFluxCapData[60] = 0.000000; ui7.fFluxCapData[61] = 0.000000; ui7.fFluxCapData[62] = 0.000000; ui7.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui7);


	m_dPortamentoTime_mSec = 0.000000;
	CUICtrl ui8;
	ui8.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui8.uControlId = 11;
	ui8.bLogSlider = false;
	ui8.bExpSlider = false;
	ui8.fUserDisplayDataLoLimit = 0.000000;
	ui8.fUserDisplayDataHiLimit = 5000.000000;
	ui8.uUserDataType = doubleData;
	ui8.fInitUserIntValue = 0;
	ui8.fInitUserFloatValue = 0;
	ui8.fInitUserDoubleValue = 0.000000;
	ui8.fInitUserUINTValue = 0;
	ui8.m_pUserCookedIntData = NULL;
	ui8.m_pUserCookedFloatData = NULL;
	ui8.m_pUserCookedDoubleData = &m_dPortamentoTime_mSec;
	ui8.m_pUserCookedUINTData = NULL;
	ui8.cControlUnits = "mS                                                              ";
	ui8.cVariableName = "m_dPortamentoTime_mSec";
	ui8.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui8.dPresetData[0] = 0.000000;ui8.dPresetData[1] = 0.000000;ui8.dPresetData[2] = 0.000000;ui8.dPresetData[3] = 0.000000;ui8.dPresetData[4] = 0.000000;ui8.dPresetData[5] = 0.000000;ui8.dPresetData[6] = 0.000000;ui8.dPresetData[7] = 0.000000;ui8.dPresetData[8] = 0.000000;ui8.dPresetData[9] = 0.000000;ui8.dPresetData[10] = 0.000000;ui8.dPresetData[11] = 0.000000;ui8.dPresetData[12] = 0.000000;ui8.dPresetData[13] = 0.000000;ui8.dPresetData[14] = 0.000000;ui8.dPresetData[15] = 0.000000;
	ui8.cControlName = "Portamento";
	ui8.bOwnerControl = false;
	ui8.bMIDIControl = false;
	ui8.uMIDIControlCommand = 176;
	ui8.uMIDIControlName = 3;
	ui8.uMIDIControlChannel = 0;
	ui8.nGUIRow = -1;
	ui8.nGUIColumn = -1;
	ui8.uControlTheme[0] = 0; ui8.uControlTheme[1] = 0; ui8.uControlTheme[2] = 0; ui8.uControlTheme[3] = 0; ui8.uControlTheme[4] = 0; ui8.uControlTheme[5] = 0; ui8.uControlTheme[6] = 0; ui8.uControlTheme[7] = 0; ui8.uControlTheme[8] = 0; ui8.uControlTheme[9] = 0; ui8.uControlTheme[10] = 0; ui8.uControlTheme[11] = 0; ui8.uControlTheme[12] = 0; ui8.uControlTheme[13] = 0; ui8.uControlTheme[14] = 0; ui8.uControlTheme[15] = 0; ui8.uControlTheme[16] = 0; ui8.uControlTheme[17] = 0; ui8.uControlTheme[18] = 0; ui8.uControlTheme[19] = 0; ui8.uControlTheme[20] = 0; ui8.uControlTheme[21] = 0; ui8.uControlTheme[22] = 0; ui8.uControlTheme[23] = 0; ui8.uControlTheme[24] = 0; ui8.uControlTheme[25] = 0; ui8.uControlTheme[26] = 0; ui8.uControlTheme[27] = 0; ui8.uControlTheme[28] = 0; ui8.uControlTheme[29] = 0; ui8.uControlTheme[30] = 0; ui8.uControlTheme[31] = 0;
	ui8.uFluxCapControl[0] = 0; ui8.uFluxCapControl[1] = 0; ui8.uFluxCapControl[2] = 0; ui8.uFluxCapControl[3] = 0; ui8.uFluxCapControl[4] = 0; ui8.uFluxCapControl[5] = 0; ui8.uFluxCapControl[6] = 0; ui8.uFluxCapControl[7] = 0; ui8.uFluxCapControl[8] = 0; ui8.uFluxCapControl[9] = 0; ui8.uFluxCapControl[10] = 0; ui8.uFluxCapControl[11] = 0; ui8.uFluxCapControl[12] = 0; ui8.uFluxCapControl[13] = 0; ui8.uFluxCapControl[14] = 0; ui8.uFluxCapControl[15] = 0; ui8.uFluxCapControl[16] = 0; ui8.uFluxCapControl[17] = 0; ui8.uFluxCapControl[18] = 0; ui8.uFluxCapControl[19] = 0; ui8.uFluxCapControl[20] = 0; ui8.uFluxCapControl[21] = 0; ui8.uFluxCapControl[22] = 0; ui8.uFluxCapControl[23] = 0; ui8.uFluxCapControl[24] = 0; ui8.uFluxCapControl[25] = 0; ui8.uFluxCapControl[26] = 0; ui8.uFluxCapControl[27] = 0; ui8.uFluxCapControl[28] = 0; ui8.uFluxCapControl[29] = 0; ui8.uFluxCapControl[30] = 0; ui8.uFluxCapControl[31] = 0; ui8.uFluxCapControl[32] = 0; ui8.uFluxCapControl[33] = 0; ui8.uFluxCapControl[34] = 0; ui8.uFluxCapControl[35] = 0; ui8.uFluxCapControl[36] = 0; ui8.uFluxCapControl[37] = 0; ui8.uFluxCapControl[38] = 0; ui8.uFluxCapControl[39] = 0; ui8.uFluxCapControl[40] = 0; ui8.uFluxCapControl[41] = 0; ui8.uFluxCapControl[42] = 0; ui8.uFluxCapControl[43] = 0; ui8.uFluxCapControl[44] = 0; ui8.uFluxCapControl[45] = 0; ui8.uFluxCapControl[46] = 0; ui8.uFluxCapControl[47] = 0; ui8.uFluxCapControl[48] = 0; ui8.uFluxCapControl[49] = 0; ui8.uFluxCapControl[50] = 0; ui8.uFluxCapControl[51] = 0; ui8.uFluxCapControl[52] = 0; ui8.uFluxCapControl[53] = 0; ui8.uFluxCapControl[54] = 0; ui8.uFluxCapControl[55] = 0; ui8.uFluxCapControl[56] = 0; ui8.uFluxCapControl[57] = 0; ui8.uFluxCapControl[58] = 0; ui8.uFluxCapControl[59] = 0; ui8.uFluxCapControl[60] = 0; ui8.uFluxCapControl[61] = 0; ui8.uFluxCapControl[62] = 0; ui8.uFluxCapControl[63] = 0;
	ui8.fFluxCapData[0] = 0.000000; ui8.fFluxCapData[1] = 0.000000; ui8.fFluxCapData[2] = 0.000000; ui8.fFluxCapData[3] = 0.000000; ui8.fFluxCapData[4] = 0.000000; ui8.fFluxCapData[5] = 0.000000; ui8.fFluxCapData[6] = 0.000000; ui8.fFluxCapData[7] = 0.000000; ui8.fFluxCapData[8] = 0.000000; ui8.fFluxCapData[9] = 0.000000; ui8.fFluxCapData[10] = 0.000000; ui8.fFluxCapData[11] = 0.000000; ui8.fFluxCapData[12] = 0.000000; ui8.fFluxCapData[13] = 0.000000; ui8.fFluxCapData[14] = 0.000000; ui8.fFluxCapData[15] = 0.000000; ui8.fFluxCapData[16] = 0.000000; ui8.fFluxCapData[17] = 0.000000; ui8.fFluxCapData[18] = 0.000000; ui8.fFluxCapData[19] = 0.000000; ui8.fFluxCapData[20] = 0.000000; ui8.fFluxCapData[21] = 0.000000; ui8.fFluxCapData[22] = 0.000000; ui8.fFluxCapData[23] = 0.000000; ui8.fFluxCapData[24] = 0.000000; ui8.fFluxCapData[25] = 0.000000; ui8.fFluxCapData[26] = 0.000000; ui8.fFluxCapData[27] = 0.000000; ui8.fFluxCapData[28] = 0.000000; ui8.fFluxCapData[29] = 0.000000; ui8.fFluxCapData[30] = 0.000000; ui8.fFluxCapData[31] = 0.000000; ui8.fFluxCapData[32] = 0.000000; ui8.fFluxCapData[33] = 0.000000; ui8.fFluxCapData[34] = 0.000000; ui8.fFluxCapData[35] = 0.000000; ui8.fFluxCapData[36] = 0.000000; ui8.fFluxCapData[37] = 0.000000; ui8.fFluxCapData[38] = 0.000000; ui8.fFluxCapData[39] = 0.000000; ui8.fFluxCapData[40] = 0.000000; ui8.fFluxCapData[41] = 0.000000; ui8.fFluxCapData[42] = 0.000000; ui8.fFluxCapData[43] = 0.000000; ui8.fFluxCapData[44] = 0.000000; ui8.fFluxCapData[45] = 0.000000; ui8.fFluxCapData[46] = 0.000000; ui8.fFluxCapData[47] = 0.000000; ui8.fFluxCapData[48] = 0.000000; ui8.fFluxCapData[49] = 0.000000; ui8.fFluxCapData[50] = 0.000000; ui8.fFluxCapData[51] = 0.000000; ui8.fFluxCapData[52] = 0.000000; ui8.fFluxCapData[53] = 0.000000; ui8.fFluxCapData[54] = 0.000000; ui8.fFluxCapData[55] = 0.000000; ui8.fFluxCapData[56] = 0.000000; ui8.fFluxCapData[57] = 0.000000; ui8.fFluxCapData[58] = 0.000000; ui8.fFluxCapData[59] = 0.000000; ui8.fFluxCapData[60] = 0.000000; ui8.fFluxCapData[61] = 0.000000; ui8.fFluxCapData[62] = 0.000000; ui8.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui8);


	m_dQControl = 1.000000;
	CUICtrl ui9;
	ui9.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui9.uControlId = 12;
	ui9.bLogSlider = false;
	ui9.bExpSlider = true;
	ui9.fUserDisplayDataLoLimit = 1.000000;
	ui9.fUserDisplayDataHiLimit = 10.000000;
	ui9.uUserDataType = doubleData;
	ui9.fInitUserIntValue = 0;
	ui9.fInitUserFloatValue = 0;
	ui9.fInitUserDoubleValue = 1.000000;
	ui9.fInitUserUINTValue = 0;
	ui9.m_pUserCookedIntData = NULL;
	ui9.m_pUserCookedFloatData = NULL;
	ui9.m_pUserCookedDoubleData = &m_dQControl;
	ui9.m_pUserCookedUINTData = NULL;
	ui9.cControlUnits = "                                                                ";
	ui9.cVariableName = "m_dQControl";
	ui9.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui9.dPresetData[0] = 1.000000;ui9.dPresetData[1] = 0.000000;ui9.dPresetData[2] = 0.000000;ui9.dPresetData[3] = 0.000000;ui9.dPresetData[4] = 0.000000;ui9.dPresetData[5] = 0.000000;ui9.dPresetData[6] = 0.000000;ui9.dPresetData[7] = 0.000000;ui9.dPresetData[8] = 0.000000;ui9.dPresetData[9] = 0.000000;ui9.dPresetData[10] = 0.000000;ui9.dPresetData[11] = 0.000000;ui9.dPresetData[12] = 0.000000;ui9.dPresetData[13] = 0.000000;ui9.dPresetData[14] = 0.000000;ui9.dPresetData[15] = 0.000000;
	ui9.cControlName = "Filter Q";
	ui9.bOwnerControl = false;
	ui9.bMIDIControl = false;
	ui9.uMIDIControlCommand = 176;
	ui9.uMIDIControlName = 3;
	ui9.uMIDIControlChannel = 0;
	ui9.nGUIRow = -1;
	ui9.nGUIColumn = -1;
	ui9.uControlTheme[0] = 0; ui9.uControlTheme[1] = 0; ui9.uControlTheme[2] = 0; ui9.uControlTheme[3] = 0; ui9.uControlTheme[4] = 0; ui9.uControlTheme[5] = 0; ui9.uControlTheme[6] = 0; ui9.uControlTheme[7] = 0; ui9.uControlTheme[8] = 0; ui9.uControlTheme[9] = 0; ui9.uControlTheme[10] = 0; ui9.uControlTheme[11] = 0; ui9.uControlTheme[12] = 0; ui9.uControlTheme[13] = 0; ui9.uControlTheme[14] = 0; ui9.uControlTheme[15] = 0; ui9.uControlTheme[16] = 0; ui9.uControlTheme[17] = 0; ui9.uControlTheme[18] = 0; ui9.uControlTheme[19] = 0; ui9.uControlTheme[20] = 0; ui9.uControlTheme[21] = 0; ui9.uControlTheme[22] = 0; ui9.uControlTheme[23] = 0; ui9.uControlTheme[24] = 0; ui9.uControlTheme[25] = 0; ui9.uControlTheme[26] = 0; ui9.uControlTheme[27] = 0; ui9.uControlTheme[28] = 0; ui9.uControlTheme[29] = 0; ui9.uControlTheme[30] = 0; ui9.uControlTheme[31] = 0;
	ui9.uFluxCapControl[0] = 0; ui9.uFluxCapControl[1] = 0; ui9.uFluxCapControl[2] = 0; ui9.uFluxCapControl[3] = 0; ui9.uFluxCapControl[4] = 0; ui9.uFluxCapControl[5] = 0; ui9.uFluxCapControl[6] = 0; ui9.uFluxCapControl[7] = 0; ui9.uFluxCapControl[8] = 0; ui9.uFluxCapControl[9] = 0; ui9.uFluxCapControl[10] = 0; ui9.uFluxCapControl[11] = 0; ui9.uFluxCapControl[12] = 0; ui9.uFluxCapControl[13] = 0; ui9.uFluxCapControl[14] = 0; ui9.uFluxCapControl[15] = 0; ui9.uFluxCapControl[16] = 0; ui9.uFluxCapControl[17] = 0; ui9.uFluxCapControl[18] = 0; ui9.uFluxCapControl[19] = 0; ui9.uFluxCapControl[20] = 0; ui9.uFluxCapControl[21] = 0; ui9.uFluxCapControl[22] = 0; ui9.uFluxCapControl[23] = 0; ui9.uFluxCapControl[24] = 0; ui9.uFluxCapControl[25] = 0; ui9.uFluxCapControl[26] = 0; ui9.uFluxCapControl[27] = 0; ui9.uFluxCapControl[28] = 0; ui9.uFluxCapControl[29] = 0; ui9.uFluxCapControl[30] = 0; ui9.uFluxCapControl[31] = 0; ui9.uFluxCapControl[32] = 0; ui9.uFluxCapControl[33] = 0; ui9.uFluxCapControl[34] = 0; ui9.uFluxCapControl[35] = 0; ui9.uFluxCapControl[36] = 0; ui9.uFluxCapControl[37] = 0; ui9.uFluxCapControl[38] = 0; ui9.uFluxCapControl[39] = 0; ui9.uFluxCapControl[40] = 0; ui9.uFluxCapControl[41] = 0; ui9.uFluxCapControl[42] = 0; ui9.uFluxCapControl[43] = 0; ui9.uFluxCapControl[44] = 0; ui9.uFluxCapControl[45] = 0; ui9.uFluxCapControl[46] = 0; ui9.uFluxCapControl[47] = 0; ui9.uFluxCapControl[48] = 0; ui9.uFluxCapControl[49] = 0; ui9.uFluxCapControl[50] = 0; ui9.uFluxCapControl[51] = 0; ui9.uFluxCapControl[52] = 0; ui9.uFluxCapControl[53] = 0; ui9.uFluxCapControl[54] = 0; ui9.uFluxCapControl[55] = 0; ui9.uFluxCapControl[56] = 0; ui9.uFluxCapControl[57] = 0; ui9.uFluxCapControl[58] = 0; ui9.uFluxCapControl[59] = 0; ui9.uFluxCapControl[60] = 0; ui9.uFluxCapControl[61] = 0; ui9.uFluxCapControl[62] = 0; ui9.uFluxCapControl[63] = 0;
	ui9.fFluxCapData[0] = 0.000000; ui9.fFluxCapData[1] = 0.000000; ui9.fFluxCapData[2] = 0.000000; ui9.fFluxCapData[3] = 0.000000; ui9.fFluxCapData[4] = 0.000000; ui9.fFluxCapData[5] = 0.000000; ui9.fFluxCapData[6] = 0.000000; ui9.fFluxCapData[7] = 0.000000; ui9.fFluxCapData[8] = 0.000000; ui9.fFluxCapData[9] = 0.000000; ui9.fFluxCapData[10] = 0.000000; ui9.fFluxCapData[11] = 0.000000; ui9.fFluxCapData[12] = 0.000000; ui9.fFluxCapData[13] = 0.000000; ui9.fFluxCapData[14] = 0.000000; ui9.fFluxCapData[15] = 0.000000; ui9.fFluxCapData[16] = 0.000000; ui9.fFluxCapData[17] = 0.000000; ui9.fFluxCapData[18] = 0.000000; ui9.fFluxCapData[19] = 0.000000; ui9.fFluxCapData[20] = 0.000000; ui9.fFluxCapData[21] = 0.000000; ui9.fFluxCapData[22] = 0.000000; ui9.fFluxCapData[23] = 0.000000; ui9.fFluxCapData[24] = 0.000000; ui9.fFluxCapData[25] = 0.000000; ui9.fFluxCapData[26] = 0.000000; ui9.fFluxCapData[27] = 0.000000; ui9.fFluxCapData[28] = 0.000000; ui9.fFluxCapData[29] = 0.000000; ui9.fFluxCapData[30] = 0.000000; ui9.fFluxCapData[31] = 0.000000; ui9.fFluxCapData[32] = 0.000000; ui9.fFluxCapData[33] = 0.000000; ui9.fFluxCapData[34] = 0.000000; ui9.fFluxCapData[35] = 0.000000; ui9.fFluxCapData[36] = 0.000000; ui9.fFluxCapData[37] = 0.000000; ui9.fFluxCapData[38] = 0.000000; ui9.fFluxCapData[39] = 0.000000; ui9.fFluxCapData[40] = 0.000000; ui9.fFluxCapData[41] = 0.000000; ui9.fFluxCapData[42] = 0.000000; ui9.fFluxCapData[43] = 0.000000; ui9.fFluxCapData[44] = 0.000000; ui9.fFluxCapData[45] = 0.000000; ui9.fFluxCapData[46] = 0.000000; ui9.fFluxCapData[47] = 0.000000; ui9.fFluxCapData[48] = 0.000000; ui9.fFluxCapData[49] = 0.000000; ui9.fFluxCapData[50] = 0.000000; ui9.fFluxCapData[51] = 0.000000; ui9.fFluxCapData[52] = 0.000000; ui9.fFluxCapData[53] = 0.000000; ui9.fFluxCapData[54] = 0.000000; ui9.fFluxCapData[55] = 0.000000; ui9.fFluxCapData[56] = 0.000000; ui9.fFluxCapData[57] = 0.000000; ui9.fFluxCapData[58] = 0.000000; ui9.fFluxCapData[59] = 0.000000; ui9.fFluxCapData[60] = 0.000000; ui9.fFluxCapData[61] = 0.000000; ui9.fFluxCapData[62] = 0.000000; ui9.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui9);


	m_dLFO1OscPitchIntensity = 0.000000;
	CUICtrl ui10;
	ui10.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui10.uControlId = 13;
	ui10.bLogSlider = false;
	ui10.bExpSlider = false;
	ui10.fUserDisplayDataLoLimit = -1.000000;
	ui10.fUserDisplayDataHiLimit = 1.000000;
	ui10.uUserDataType = doubleData;
	ui10.fInitUserIntValue = 0;
	ui10.fInitUserFloatValue = 0;
	ui10.fInitUserDoubleValue = 0.000000;
	ui10.fInitUserUINTValue = 0;
	ui10.m_pUserCookedIntData = NULL;
	ui10.m_pUserCookedFloatData = NULL;
	ui10.m_pUserCookedDoubleData = &m_dLFO1OscPitchIntensity;
	ui10.m_pUserCookedUINTData = NULL;
	ui10.cControlUnits = "                                                                ";
	ui10.cVariableName = "m_dLFO1OscPitchIntensity";
	ui10.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui10.dPresetData[0] = 0.000000;ui10.dPresetData[1] = 0.000000;ui10.dPresetData[2] = 0.000000;ui10.dPresetData[3] = 0.000000;ui10.dPresetData[4] = 0.000000;ui10.dPresetData[5] = 0.000000;ui10.dPresetData[6] = 0.000000;ui10.dPresetData[7] = 0.000000;ui10.dPresetData[8] = 0.000000;ui10.dPresetData[9] = 0.000000;ui10.dPresetData[10] = 0.000000;ui10.dPresetData[11] = 0.000000;ui10.dPresetData[12] = 0.000000;ui10.dPresetData[13] = 0.000000;ui10.dPresetData[14] = 0.000000;ui10.dPresetData[15] = 0.000000;
	ui10.cControlName = "LFO Pitch Int";
	ui10.bOwnerControl = false;
	ui10.bMIDIControl = false;
	ui10.uMIDIControlCommand = 176;
	ui10.uMIDIControlName = 3;
	ui10.uMIDIControlChannel = 0;
	ui10.nGUIRow = -1;
	ui10.nGUIColumn = -1;
	ui10.uControlTheme[0] = 0; ui10.uControlTheme[1] = 0; ui10.uControlTheme[2] = 0; ui10.uControlTheme[3] = 0; ui10.uControlTheme[4] = 0; ui10.uControlTheme[5] = 0; ui10.uControlTheme[6] = 0; ui10.uControlTheme[7] = 0; ui10.uControlTheme[8] = 0; ui10.uControlTheme[9] = 0; ui10.uControlTheme[10] = 0; ui10.uControlTheme[11] = 0; ui10.uControlTheme[12] = 0; ui10.uControlTheme[13] = 0; ui10.uControlTheme[14] = 0; ui10.uControlTheme[15] = 0; ui10.uControlTheme[16] = 0; ui10.uControlTheme[17] = 0; ui10.uControlTheme[18] = 0; ui10.uControlTheme[19] = 0; ui10.uControlTheme[20] = 0; ui10.uControlTheme[21] = 0; ui10.uControlTheme[22] = 0; ui10.uControlTheme[23] = 0; ui10.uControlTheme[24] = 0; ui10.uControlTheme[25] = 0; ui10.uControlTheme[26] = 0; ui10.uControlTheme[27] = 0; ui10.uControlTheme[28] = 0; ui10.uControlTheme[29] = 0; ui10.uControlTheme[30] = 0; ui10.uControlTheme[31] = 0;
	ui10.uFluxCapControl[0] = 0; ui10.uFluxCapControl[1] = 0; ui10.uFluxCapControl[2] = 0; ui10.uFluxCapControl[3] = 0; ui10.uFluxCapControl[4] = 0; ui10.uFluxCapControl[5] = 0; ui10.uFluxCapControl[6] = 0; ui10.uFluxCapControl[7] = 0; ui10.uFluxCapControl[8] = 0; ui10.uFluxCapControl[9] = 0; ui10.uFluxCapControl[10] = 0; ui10.uFluxCapControl[11] = 0; ui10.uFluxCapControl[12] = 0; ui10.uFluxCapControl[13] = 0; ui10.uFluxCapControl[14] = 0; ui10.uFluxCapControl[15] = 0; ui10.uFluxCapControl[16] = 0; ui10.uFluxCapControl[17] = 0; ui10.uFluxCapControl[18] = 0; ui10.uFluxCapControl[19] = 0; ui10.uFluxCapControl[20] = 0; ui10.uFluxCapControl[21] = 0; ui10.uFluxCapControl[22] = 0; ui10.uFluxCapControl[23] = 0; ui10.uFluxCapControl[24] = 0; ui10.uFluxCapControl[25] = 0; ui10.uFluxCapControl[26] = 0; ui10.uFluxCapControl[27] = 0; ui10.uFluxCapControl[28] = 0; ui10.uFluxCapControl[29] = 0; ui10.uFluxCapControl[30] = 0; ui10.uFluxCapControl[31] = 0; ui10.uFluxCapControl[32] = 0; ui10.uFluxCapControl[33] = 0; ui10.uFluxCapControl[34] = 0; ui10.uFluxCapControl[35] = 0; ui10.uFluxCapControl[36] = 0; ui10.uFluxCapControl[37] = 0; ui10.uFluxCapControl[38] = 0; ui10.uFluxCapControl[39] = 0; ui10.uFluxCapControl[40] = 0; ui10.uFluxCapControl[41] = 0; ui10.uFluxCapControl[42] = 0; ui10.uFluxCapControl[43] = 0; ui10.uFluxCapControl[44] = 0; ui10.uFluxCapControl[45] = 0; ui10.uFluxCapControl[46] = 0; ui10.uFluxCapControl[47] = 0; ui10.uFluxCapControl[48] = 0; ui10.uFluxCapControl[49] = 0; ui10.uFluxCapControl[50] = 0; ui10.uFluxCapControl[51] = 0; ui10.uFluxCapControl[52] = 0; ui10.uFluxCapControl[53] = 0; ui10.uFluxCapControl[54] = 0; ui10.uFluxCapControl[55] = 0; ui10.uFluxCapControl[56] = 0; ui10.uFluxCapControl[57] = 0; ui10.uFluxCapControl[58] = 0; ui10.uFluxCapControl[59] = 0; ui10.uFluxCapControl[60] = 0; ui10.uFluxCapControl[61] = 0; ui10.uFluxCapControl[62] = 0; ui10.uFluxCapControl[63] = 0;
	ui10.fFluxCapData[0] = 0.000000; ui10.fFluxCapData[1] = 0.000000; ui10.fFluxCapData[2] = 0.000000; ui10.fFluxCapData[3] = 0.000000; ui10.fFluxCapData[4] = 0.000000; ui10.fFluxCapData[5] = 0.000000; ui10.fFluxCapData[6] = 0.000000; ui10.fFluxCapData[7] = 0.000000; ui10.fFluxCapData[8] = 0.000000; ui10.fFluxCapData[9] = 0.000000; ui10.fFluxCapData[10] = 0.000000; ui10.fFluxCapData[11] = 0.000000; ui10.fFluxCapData[12] = 0.000000; ui10.fFluxCapData[13] = 0.000000; ui10.fFluxCapData[14] = 0.000000; ui10.fFluxCapData[15] = 0.000000; ui10.fFluxCapData[16] = 0.000000; ui10.fFluxCapData[17] = 0.000000; ui10.fFluxCapData[18] = 0.000000; ui10.fFluxCapData[19] = 0.000000; ui10.fFluxCapData[20] = 0.000000; ui10.fFluxCapData[21] = 0.000000; ui10.fFluxCapData[22] = 0.000000; ui10.fFluxCapData[23] = 0.000000; ui10.fFluxCapData[24] = 0.000000; ui10.fFluxCapData[25] = 0.000000; ui10.fFluxCapData[26] = 0.000000; ui10.fFluxCapData[27] = 0.000000; ui10.fFluxCapData[28] = 0.000000; ui10.fFluxCapData[29] = 0.000000; ui10.fFluxCapData[30] = 0.000000; ui10.fFluxCapData[31] = 0.000000; ui10.fFluxCapData[32] = 0.000000; ui10.fFluxCapData[33] = 0.000000; ui10.fFluxCapData[34] = 0.000000; ui10.fFluxCapData[35] = 0.000000; ui10.fFluxCapData[36] = 0.000000; ui10.fFluxCapData[37] = 0.000000; ui10.fFluxCapData[38] = 0.000000; ui10.fFluxCapData[39] = 0.000000; ui10.fFluxCapData[40] = 0.000000; ui10.fFluxCapData[41] = 0.000000; ui10.fFluxCapData[42] = 0.000000; ui10.fFluxCapData[43] = 0.000000; ui10.fFluxCapData[44] = 0.000000; ui10.fFluxCapData[45] = 0.000000; ui10.fFluxCapData[46] = 0.000000; ui10.fFluxCapData[47] = 0.000000; ui10.fFluxCapData[48] = 0.000000; ui10.fFluxCapData[49] = 0.000000; ui10.fFluxCapData[50] = 0.000000; ui10.fFluxCapData[51] = 0.000000; ui10.fFluxCapData[52] = 0.000000; ui10.fFluxCapData[53] = 0.000000; ui10.fFluxCapData[54] = 0.000000; ui10.fFluxCapData[55] = 0.000000; ui10.fFluxCapData[56] = 0.000000; ui10.fFluxCapData[57] = 0.000000; ui10.fFluxCapData[58] = 0.000000; ui10.fFluxCapData[59] = 0.000000; ui10.fFluxCapData[60] = 0.000000; ui10.fFluxCapData[61] = 0.000000; ui10.fFluxCapData[62] = 0.000000; ui10.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui10);


	m_dDecayReleaseTime_mSec = 1000.000000;
	CUICtrl ui11;
	ui11.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui11.uControlId = 14;
	ui11.bLogSlider = false;
	ui11.bExpSlider = false;
	ui11.fUserDisplayDataLoLimit = 0.000000;
	ui11.fUserDisplayDataHiLimit = 10000.000000;
	ui11.uUserDataType = doubleData;
	ui11.fInitUserIntValue = 0;
	ui11.fInitUserFloatValue = 0;
	ui11.fInitUserDoubleValue = 1000.000000;
	ui11.fInitUserUINTValue = 0;
	ui11.m_pUserCookedIntData = NULL;
	ui11.m_pUserCookedFloatData = NULL;
	ui11.m_pUserCookedDoubleData = &m_dDecayReleaseTime_mSec;
	ui11.m_pUserCookedUINTData = NULL;
	ui11.cControlUnits = "mS                                                              ";
	ui11.cVariableName = "m_dDecayReleaseTime_mSec";
	ui11.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui11.dPresetData[0] = 1000.000000;ui11.dPresetData[1] = 0.000000;ui11.dPresetData[2] = 0.000000;ui11.dPresetData[3] = 0.000000;ui11.dPresetData[4] = 0.000000;ui11.dPresetData[5] = 0.000000;ui11.dPresetData[6] = 0.000000;ui11.dPresetData[7] = 0.000000;ui11.dPresetData[8] = 0.000000;ui11.dPresetData[9] = 0.000000;ui11.dPresetData[10] = 0.000000;ui11.dPresetData[11] = 0.000000;ui11.dPresetData[12] = 0.000000;ui11.dPresetData[13] = 0.000000;ui11.dPresetData[14] = 0.000000;ui11.dPresetData[15] = 0.000000;
	ui11.cControlName = "Decay/Release";
	ui11.bOwnerControl = false;
	ui11.bMIDIControl = false;
	ui11.uMIDIControlCommand = 176;
	ui11.uMIDIControlName = 3;
	ui11.uMIDIControlChannel = 0;
	ui11.nGUIRow = -1;
	ui11.nGUIColumn = -1;
	ui11.uControlTheme[0] = 0; ui11.uControlTheme[1] = 0; ui11.uControlTheme[2] = 0; ui11.uControlTheme[3] = 0; ui11.uControlTheme[4] = 0; ui11.uControlTheme[5] = 0; ui11.uControlTheme[6] = 0; ui11.uControlTheme[7] = 0; ui11.uControlTheme[8] = 0; ui11.uControlTheme[9] = 0; ui11.uControlTheme[10] = 0; ui11.uControlTheme[11] = 0; ui11.uControlTheme[12] = 0; ui11.uControlTheme[13] = 0; ui11.uControlTheme[14] = 0; ui11.uControlTheme[15] = 0; ui11.uControlTheme[16] = 0; ui11.uControlTheme[17] = 0; ui11.uControlTheme[18] = 0; ui11.uControlTheme[19] = 0; ui11.uControlTheme[20] = 0; ui11.uControlTheme[21] = 0; ui11.uControlTheme[22] = 0; ui11.uControlTheme[23] = 0; ui11.uControlTheme[24] = 0; ui11.uControlTheme[25] = 0; ui11.uControlTheme[26] = 0; ui11.uControlTheme[27] = 0; ui11.uControlTheme[28] = 0; ui11.uControlTheme[29] = 0; ui11.uControlTheme[30] = 0; ui11.uControlTheme[31] = 0;
	ui11.uFluxCapControl[0] = 0; ui11.uFluxCapControl[1] = 0; ui11.uFluxCapControl[2] = 0; ui11.uFluxCapControl[3] = 0; ui11.uFluxCapControl[4] = 0; ui11.uFluxCapControl[5] = 0; ui11.uFluxCapControl[6] = 0; ui11.uFluxCapControl[7] = 0; ui11.uFluxCapControl[8] = 0; ui11.uFluxCapControl[9] = 0; ui11.uFluxCapControl[10] = 0; ui11.uFluxCapControl[11] = 0; ui11.uFluxCapControl[12] = 0; ui11.uFluxCapControl[13] = 0; ui11.uFluxCapControl[14] = 0; ui11.uFluxCapControl[15] = 0; ui11.uFluxCapControl[16] = 0; ui11.uFluxCapControl[17] = 0; ui11.uFluxCapControl[18] = 0; ui11.uFluxCapControl[19] = 0; ui11.uFluxCapControl[20] = 0; ui11.uFluxCapControl[21] = 0; ui11.uFluxCapControl[22] = 0; ui11.uFluxCapControl[23] = 0; ui11.uFluxCapControl[24] = 0; ui11.uFluxCapControl[25] = 0; ui11.uFluxCapControl[26] = 0; ui11.uFluxCapControl[27] = 0; ui11.uFluxCapControl[28] = 0; ui11.uFluxCapControl[29] = 0; ui11.uFluxCapControl[30] = 0; ui11.uFluxCapControl[31] = 0; ui11.uFluxCapControl[32] = 0; ui11.uFluxCapControl[33] = 0; ui11.uFluxCapControl[34] = 0; ui11.uFluxCapControl[35] = 0; ui11.uFluxCapControl[36] = 0; ui11.uFluxCapControl[37] = 0; ui11.uFluxCapControl[38] = 0; ui11.uFluxCapControl[39] = 0; ui11.uFluxCapControl[40] = 0; ui11.uFluxCapControl[41] = 0; ui11.uFluxCapControl[42] = 0; ui11.uFluxCapControl[43] = 0; ui11.uFluxCapControl[44] = 0; ui11.uFluxCapControl[45] = 0; ui11.uFluxCapControl[46] = 0; ui11.uFluxCapControl[47] = 0; ui11.uFluxCapControl[48] = 0; ui11.uFluxCapControl[49] = 0; ui11.uFluxCapControl[50] = 0; ui11.uFluxCapControl[51] = 0; ui11.uFluxCapControl[52] = 0; ui11.uFluxCapControl[53] = 0; ui11.uFluxCapControl[54] = 0; ui11.uFluxCapControl[55] = 0; ui11.uFluxCapControl[56] = 0; ui11.uFluxCapControl[57] = 0; ui11.uFluxCapControl[58] = 0; ui11.uFluxCapControl[59] = 0; ui11.uFluxCapControl[60] = 0; ui11.uFluxCapControl[61] = 0; ui11.uFluxCapControl[62] = 0; ui11.uFluxCapControl[63] = 0;
	ui11.fFluxCapData[0] = 0.000000; ui11.fFluxCapData[1] = 0.000000; ui11.fFluxCapData[2] = 0.000000; ui11.fFluxCapData[3] = 0.000000; ui11.fFluxCapData[4] = 0.000000; ui11.fFluxCapData[5] = 0.000000; ui11.fFluxCapData[6] = 0.000000; ui11.fFluxCapData[7] = 0.000000; ui11.fFluxCapData[8] = 0.000000; ui11.fFluxCapData[9] = 0.000000; ui11.fFluxCapData[10] = 0.000000; ui11.fFluxCapData[11] = 0.000000; ui11.fFluxCapData[12] = 0.000000; ui11.fFluxCapData[13] = 0.000000; ui11.fFluxCapData[14] = 0.000000; ui11.fFluxCapData[15] = 0.000000; ui11.fFluxCapData[16] = 0.000000; ui11.fFluxCapData[17] = 0.000000; ui11.fFluxCapData[18] = 0.000000; ui11.fFluxCapData[19] = 0.000000; ui11.fFluxCapData[20] = 0.000000; ui11.fFluxCapData[21] = 0.000000; ui11.fFluxCapData[22] = 0.000000; ui11.fFluxCapData[23] = 0.000000; ui11.fFluxCapData[24] = 0.000000; ui11.fFluxCapData[25] = 0.000000; ui11.fFluxCapData[26] = 0.000000; ui11.fFluxCapData[27] = 0.000000; ui11.fFluxCapData[28] = 0.000000; ui11.fFluxCapData[29] = 0.000000; ui11.fFluxCapData[30] = 0.000000; ui11.fFluxCapData[31] = 0.000000; ui11.fFluxCapData[32] = 0.000000; ui11.fFluxCapData[33] = 0.000000; ui11.fFluxCapData[34] = 0.000000; ui11.fFluxCapData[35] = 0.000000; ui11.fFluxCapData[36] = 0.000000; ui11.fFluxCapData[37] = 0.000000; ui11.fFluxCapData[38] = 0.000000; ui11.fFluxCapData[39] = 0.000000; ui11.fFluxCapData[40] = 0.000000; ui11.fFluxCapData[41] = 0.000000; ui11.fFluxCapData[42] = 0.000000; ui11.fFluxCapData[43] = 0.000000; ui11.fFluxCapData[44] = 0.000000; ui11.fFluxCapData[45] = 0.000000; ui11.fFluxCapData[46] = 0.000000; ui11.fFluxCapData[47] = 0.000000; ui11.fFluxCapData[48] = 0.000000; ui11.fFluxCapData[49] = 0.000000; ui11.fFluxCapData[50] = 0.000000; ui11.fFluxCapData[51] = 0.000000; ui11.fFluxCapData[52] = 0.000000; ui11.fFluxCapData[53] = 0.000000; ui11.fFluxCapData[54] = 0.000000; ui11.fFluxCapData[55] = 0.000000; ui11.fFluxCapData[56] = 0.000000; ui11.fFluxCapData[57] = 0.000000; ui11.fFluxCapData[58] = 0.000000; ui11.fFluxCapData[59] = 0.000000; ui11.fFluxCapData[60] = 0.000000; ui11.fFluxCapData[61] = 0.000000; ui11.fFluxCapData[62] = 0.000000; ui11.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui11);


	m_dOrbitY = 0.000000;
	CUICtrl ui12;
	ui12.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui12.uControlId = 15;
	ui12.bLogSlider = false;
	ui12.bExpSlider = false;
	ui12.fUserDisplayDataLoLimit = 0.000000;
	ui12.fUserDisplayDataHiLimit = 1.000000;
	ui12.uUserDataType = doubleData;
	ui12.fInitUserIntValue = 0;
	ui12.fInitUserFloatValue = 0;
	ui12.fInitUserDoubleValue = 0.000000;
	ui12.fInitUserUINTValue = 0;
	ui12.m_pUserCookedIntData = NULL;
	ui12.m_pUserCookedFloatData = NULL;
	ui12.m_pUserCookedDoubleData = &m_dOrbitY;
	ui12.m_pUserCookedUINTData = NULL;
	ui12.cControlUnits = "                                                                ";
	ui12.cVariableName = "m_dOrbitY";
	ui12.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui12.dPresetData[0] = 0.000000;ui12.dPresetData[1] = 0.000000;ui12.dPresetData[2] = 0.000000;ui12.dPresetData[3] = 0.000000;ui12.dPresetData[4] = 0.000000;ui12.dPresetData[5] = 0.000000;ui12.dPresetData[6] = 0.000000;ui12.dPresetData[7] = 0.000000;ui12.dPresetData[8] = 0.000000;ui12.dPresetData[9] = 0.000000;ui12.dPresetData[10] = 0.000000;ui12.dPresetData[11] = 0.000000;ui12.dPresetData[12] = 0.000000;ui12.dPresetData[13] = 0.000000;ui12.dPresetData[14] = 0.000000;ui12.dPresetData[15] = 0.000000;
	ui12.cControlName = "Orbit Y";
	ui12.bOwnerControl = false;
	ui12.bMIDIControl = false;
	ui12.uMIDIControlCommand = 176;
	ui12.uMIDIControlName = 3;
	ui12.uMIDIControlChannel = 0;
	ui12.nGUIRow = -1;
	ui12.nGUIColumn = -1;
	ui12.uControlTheme[0] = 0; ui12.uControlTheme[1] = 0; ui12.uControlTheme[2] = 0; ui12.uControlTheme[3] = 0; ui12.uControlTheme[4] = 0; ui12.uControlTheme[5] = 0; ui12.uControlTheme[6] = 0; ui12.uControlTheme[7] = 0; ui12.uControlTheme[8] = 0; ui12.uControlTheme[9] = 0; ui12.uControlTheme[10] = 0; ui12.uControlTheme[11] = 0; ui12.uControlTheme[12] = 0; ui12.uControlTheme[13] = 0; ui12.uControlTheme[14] = 0; ui12.uControlTheme[15] = 0; ui12.uControlTheme[16] = 0; ui12.uControlTheme[17] = 0; ui12.uControlTheme[18] = 0; ui12.uControlTheme[19] = 0; ui12.uControlTheme[20] = 0; ui12.uControlTheme[21] = 0; ui12.uControlTheme[22] = 0; ui12.uControlTheme[23] = 0; ui12.uControlTheme[24] = 0; ui12.uControlTheme[25] = 0; ui12.uControlTheme[26] = 0; ui12.uControlTheme[27] = 0; ui12.uControlTheme[28] = 0; ui12.uControlTheme[29] = 0; ui12.uControlTheme[30] = 0; ui12.uControlTheme[31] = 0;
	ui12.uFluxCapControl[0] = 0; ui12.uFluxCapControl[1] = 0; ui12.uFluxCapControl[2] = 0; ui12.uFluxCapControl[3] = 0; ui12.uFluxCapControl[4] = 0; ui12.uFluxCapControl[5] = 0; ui12.uFluxCapControl[6] = 0; ui12.uFluxCapControl[7] = 0; ui12.uFluxCapControl[8] = 0; ui12.uFluxCapControl[9] = 0; ui12.uFluxCapControl[10] = 0; ui12.uFluxCapControl[11] = 0; ui12.uFluxCapControl[12] = 0; ui12.uFluxCapControl[13] = 0; ui12.uFluxCapControl[14] = 0; ui12.uFluxCapControl[15] = 0; ui12.uFluxCapControl[16] = 0; ui12.uFluxCapControl[17] = 0; ui12.uFluxCapControl[18] = 0; ui12.uFluxCapControl[19] = 0; ui12.uFluxCapControl[20] = 0; ui12.uFluxCapControl[21] = 0; ui12.uFluxCapControl[22] = 0; ui12.uFluxCapControl[23] = 0; ui12.uFluxCapControl[24] = 0; ui12.uFluxCapControl[25] = 0; ui12.uFluxCapControl[26] = 0; ui12.uFluxCapControl[27] = 0; ui12.uFluxCapControl[28] = 0; ui12.uFluxCapControl[29] = 0; ui12.uFluxCapControl[30] = 0; ui12.uFluxCapControl[31] = 0; ui12.uFluxCapControl[32] = 0; ui12.uFluxCapControl[33] = 0; ui12.uFluxCapControl[34] = 0; ui12.uFluxCapControl[35] = 0; ui12.uFluxCapControl[36] = 0; ui12.uFluxCapControl[37] = 0; ui12.uFluxCapControl[38] = 0; ui12.uFluxCapControl[39] = 0; ui12.uFluxCapControl[40] = 0; ui12.uFluxCapControl[41] = 0; ui12.uFluxCapControl[42] = 0; ui12.uFluxCapControl[43] = 0; ui12.uFluxCapControl[44] = 0; ui12.uFluxCapControl[45] = 0; ui12.uFluxCapControl[46] = 0; ui12.uFluxCapControl[47] = 0; ui12.uFluxCapControl[48] = 0; ui12.uFluxCapControl[49] = 0; ui12.uFluxCapControl[50] = 0; ui12.uFluxCapControl[51] = 0; ui12.uFluxCapControl[52] = 0; ui12.uFluxCapControl[53] = 0; ui12.uFluxCapControl[54] = 0; ui12.uFluxCapControl[55] = 0; ui12.uFluxCapControl[56] = 0; ui12.uFluxCapControl[57] = 0; ui12.uFluxCapControl[58] = 0; ui12.uFluxCapControl[59] = 0; ui12.uFluxCapControl[60] = 0; ui12.uFluxCapControl[61] = 0; ui12.uFluxCapControl[62] = 0; ui12.uFluxCapControl[63] = 0;
	ui12.fFluxCapData[0] = 0.000000; ui12.fFluxCapData[1] = 0.000000; ui12.fFluxCapData[2] = 0.000000; ui12.fFluxCapData[3] = 0.000000; ui12.fFluxCapData[4] = 0.000000; ui12.fFluxCapData[5] = 0.000000; ui12.fFluxCapData[6] = 0.000000; ui12.fFluxCapData[7] = 0.000000; ui12.fFluxCapData[8] = 0.000000; ui12.fFluxCapData[9] = 0.000000; ui12.fFluxCapData[10] = 0.000000; ui12.fFluxCapData[11] = 0.000000; ui12.fFluxCapData[12] = 0.000000; ui12.fFluxCapData[13] = 0.000000; ui12.fFluxCapData[14] = 0.000000; ui12.fFluxCapData[15] = 0.000000; ui12.fFluxCapData[16] = 0.000000; ui12.fFluxCapData[17] = 0.000000; ui12.fFluxCapData[18] = 0.000000; ui12.fFluxCapData[19] = 0.000000; ui12.fFluxCapData[20] = 0.000000; ui12.fFluxCapData[21] = 0.000000; ui12.fFluxCapData[22] = 0.000000; ui12.fFluxCapData[23] = 0.000000; ui12.fFluxCapData[24] = 0.000000; ui12.fFluxCapData[25] = 0.000000; ui12.fFluxCapData[26] = 0.000000; ui12.fFluxCapData[27] = 0.000000; ui12.fFluxCapData[28] = 0.000000; ui12.fFluxCapData[29] = 0.000000; ui12.fFluxCapData[30] = 0.000000; ui12.fFluxCapData[31] = 0.000000; ui12.fFluxCapData[32] = 0.000000; ui12.fFluxCapData[33] = 0.000000; ui12.fFluxCapData[34] = 0.000000; ui12.fFluxCapData[35] = 0.000000; ui12.fFluxCapData[36] = 0.000000; ui12.fFluxCapData[37] = 0.000000; ui12.fFluxCapData[38] = 0.000000; ui12.fFluxCapData[39] = 0.000000; ui12.fFluxCapData[40] = 0.000000; ui12.fFluxCapData[41] = 0.000000; ui12.fFluxCapData[42] = 0.000000; ui12.fFluxCapData[43] = 0.000000; ui12.fFluxCapData[44] = 0.000000; ui12.fFluxCapData[45] = 0.000000; ui12.fFluxCapData[46] = 0.000000; ui12.fFluxCapData[47] = 0.000000; ui12.fFluxCapData[48] = 0.000000; ui12.fFluxCapData[49] = 0.000000; ui12.fFluxCapData[50] = 0.000000; ui12.fFluxCapData[51] = 0.000000; ui12.fFluxCapData[52] = 0.000000; ui12.fFluxCapData[53] = 0.000000; ui12.fFluxCapData[54] = 0.000000; ui12.fFluxCapData[55] = 0.000000; ui12.fFluxCapData[56] = 0.000000; ui12.fFluxCapData[57] = 0.000000; ui12.fFluxCapData[58] = 0.000000; ui12.fFluxCapData[59] = 0.000000; ui12.fFluxCapData[60] = 0.000000; ui12.fFluxCapData[61] = 0.000000; ui12.fFluxCapData[62] = 0.000000; ui12.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui12);


	m_dOsc2Amplitude_dB = 0.000000;
	CUICtrl ui13;
	ui13.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui13.uControlId = 16;
	ui13.bLogSlider = false;
	ui13.bExpSlider = false;
	ui13.fUserDisplayDataLoLimit = -96.000000;
	ui13.fUserDisplayDataHiLimit = 24.000000;
	ui13.uUserDataType = doubleData;
	ui13.fInitUserIntValue = 0;
	ui13.fInitUserFloatValue = 0;
	ui13.fInitUserDoubleValue = 0.000000;
	ui13.fInitUserUINTValue = 0;
	ui13.m_pUserCookedIntData = NULL;
	ui13.m_pUserCookedFloatData = NULL;
	ui13.m_pUserCookedDoubleData = &m_dOsc2Amplitude_dB;
	ui13.m_pUserCookedUINTData = NULL;
	ui13.cControlUnits = "dB                                                              ";
	ui13.cVariableName = "m_dOsc2Amplitude_dB";
	ui13.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui13.dPresetData[0] = 0.000000;ui13.dPresetData[1] = 0.000000;ui13.dPresetData[2] = 0.000000;ui13.dPresetData[3] = 0.000000;ui13.dPresetData[4] = 0.000000;ui13.dPresetData[5] = 0.000000;ui13.dPresetData[6] = 0.000000;ui13.dPresetData[7] = 0.000000;ui13.dPresetData[8] = 0.000000;ui13.dPresetData[9] = 0.000000;ui13.dPresetData[10] = 0.000000;ui13.dPresetData[11] = 0.000000;ui13.dPresetData[12] = 0.000000;ui13.dPresetData[13] = 0.000000;ui13.dPresetData[14] = 0.000000;ui13.dPresetData[15] = 0.000000;
	ui13.cControlName = "B-D Output";
	ui13.bOwnerControl = false;
	ui13.bMIDIControl = false;
	ui13.uMIDIControlCommand = 176;
	ui13.uMIDIControlName = 3;
	ui13.uMIDIControlChannel = 0;
	ui13.nGUIRow = -1;
	ui13.nGUIColumn = -1;
	ui13.uControlTheme[0] = 0; ui13.uControlTheme[1] = 0; ui13.uControlTheme[2] = 0; ui13.uControlTheme[3] = 0; ui13.uControlTheme[4] = 0; ui13.uControlTheme[5] = 0; ui13.uControlTheme[6] = 0; ui13.uControlTheme[7] = 0; ui13.uControlTheme[8] = 0; ui13.uControlTheme[9] = 0; ui13.uControlTheme[10] = 0; ui13.uControlTheme[11] = 0; ui13.uControlTheme[12] = 0; ui13.uControlTheme[13] = 0; ui13.uControlTheme[14] = 0; ui13.uControlTheme[15] = 0; ui13.uControlTheme[16] = 0; ui13.uControlTheme[17] = 0; ui13.uControlTheme[18] = 0; ui13.uControlTheme[19] = 0; ui13.uControlTheme[20] = 0; ui13.uControlTheme[21] = 0; ui13.uControlTheme[22] = 0; ui13.uControlTheme[23] = 0; ui13.uControlTheme[24] = 0; ui13.uControlTheme[25] = 0; ui13.uControlTheme[26] = 0; ui13.uControlTheme[27] = 0; ui13.uControlTheme[28] = 0; ui13.uControlTheme[29] = 0; ui13.uControlTheme[30] = 0; ui13.uControlTheme[31] = 0;
	ui13.uFluxCapControl[0] = 0; ui13.uFluxCapControl[1] = 0; ui13.uFluxCapControl[2] = 0; ui13.uFluxCapControl[3] = 0; ui13.uFluxCapControl[4] = 0; ui13.uFluxCapControl[5] = 0; ui13.uFluxCapControl[6] = 0; ui13.uFluxCapControl[7] = 0; ui13.uFluxCapControl[8] = 0; ui13.uFluxCapControl[9] = 0; ui13.uFluxCapControl[10] = 0; ui13.uFluxCapControl[11] = 0; ui13.uFluxCapControl[12] = 0; ui13.uFluxCapControl[13] = 0; ui13.uFluxCapControl[14] = 0; ui13.uFluxCapControl[15] = 0; ui13.uFluxCapControl[16] = 0; ui13.uFluxCapControl[17] = 0; ui13.uFluxCapControl[18] = 0; ui13.uFluxCapControl[19] = 0; ui13.uFluxCapControl[20] = 0; ui13.uFluxCapControl[21] = 0; ui13.uFluxCapControl[22] = 0; ui13.uFluxCapControl[23] = 0; ui13.uFluxCapControl[24] = 0; ui13.uFluxCapControl[25] = 0; ui13.uFluxCapControl[26] = 0; ui13.uFluxCapControl[27] = 0; ui13.uFluxCapControl[28] = 0; ui13.uFluxCapControl[29] = 0; ui13.uFluxCapControl[30] = 0; ui13.uFluxCapControl[31] = 0; ui13.uFluxCapControl[32] = 0; ui13.uFluxCapControl[33] = 0; ui13.uFluxCapControl[34] = 0; ui13.uFluxCapControl[35] = 0; ui13.uFluxCapControl[36] = 0; ui13.uFluxCapControl[37] = 0; ui13.uFluxCapControl[38] = 0; ui13.uFluxCapControl[39] = 0; ui13.uFluxCapControl[40] = 0; ui13.uFluxCapControl[41] = 0; ui13.uFluxCapControl[42] = 0; ui13.uFluxCapControl[43] = 0; ui13.uFluxCapControl[44] = 0; ui13.uFluxCapControl[45] = 0; ui13.uFluxCapControl[46] = 0; ui13.uFluxCapControl[47] = 0; ui13.uFluxCapControl[48] = 0; ui13.uFluxCapControl[49] = 0; ui13.uFluxCapControl[50] = 0; ui13.uFluxCapControl[51] = 0; ui13.uFluxCapControl[52] = 0; ui13.uFluxCapControl[53] = 0; ui13.uFluxCapControl[54] = 0; ui13.uFluxCapControl[55] = 0; ui13.uFluxCapControl[56] = 0; ui13.uFluxCapControl[57] = 0; ui13.uFluxCapControl[58] = 0; ui13.uFluxCapControl[59] = 0; ui13.uFluxCapControl[60] = 0; ui13.uFluxCapControl[61] = 0; ui13.uFluxCapControl[62] = 0; ui13.uFluxCapControl[63] = 0;
	ui13.fFluxCapData[0] = 0.000000; ui13.fFluxCapData[1] = 0.000000; ui13.fFluxCapData[2] = 0.000000; ui13.fFluxCapData[3] = 0.000000; ui13.fFluxCapData[4] = 0.000000; ui13.fFluxCapData[5] = 0.000000; ui13.fFluxCapData[6] = 0.000000; ui13.fFluxCapData[7] = 0.000000; ui13.fFluxCapData[8] = 0.000000; ui13.fFluxCapData[9] = 0.000000; ui13.fFluxCapData[10] = 0.000000; ui13.fFluxCapData[11] = 0.000000; ui13.fFluxCapData[12] = 0.000000; ui13.fFluxCapData[13] = 0.000000; ui13.fFluxCapData[14] = 0.000000; ui13.fFluxCapData[15] = 0.000000; ui13.fFluxCapData[16] = 0.000000; ui13.fFluxCapData[17] = 0.000000; ui13.fFluxCapData[18] = 0.000000; ui13.fFluxCapData[19] = 0.000000; ui13.fFluxCapData[20] = 0.000000; ui13.fFluxCapData[21] = 0.000000; ui13.fFluxCapData[22] = 0.000000; ui13.fFluxCapData[23] = 0.000000; ui13.fFluxCapData[24] = 0.000000; ui13.fFluxCapData[25] = 0.000000; ui13.fFluxCapData[26] = 0.000000; ui13.fFluxCapData[27] = 0.000000; ui13.fFluxCapData[28] = 0.000000; ui13.fFluxCapData[29] = 0.000000; ui13.fFluxCapData[30] = 0.000000; ui13.fFluxCapData[31] = 0.000000; ui13.fFluxCapData[32] = 0.000000; ui13.fFluxCapData[33] = 0.000000; ui13.fFluxCapData[34] = 0.000000; ui13.fFluxCapData[35] = 0.000000; ui13.fFluxCapData[36] = 0.000000; ui13.fFluxCapData[37] = 0.000000; ui13.fFluxCapData[38] = 0.000000; ui13.fFluxCapData[39] = 0.000000; ui13.fFluxCapData[40] = 0.000000; ui13.fFluxCapData[41] = 0.000000; ui13.fFluxCapData[42] = 0.000000; ui13.fFluxCapData[43] = 0.000000; ui13.fFluxCapData[44] = 0.000000; ui13.fFluxCapData[45] = 0.000000; ui13.fFluxCapData[46] = 0.000000; ui13.fFluxCapData[47] = 0.000000; ui13.fFluxCapData[48] = 0.000000; ui13.fFluxCapData[49] = 0.000000; ui13.fFluxCapData[50] = 0.000000; ui13.fFluxCapData[51] = 0.000000; ui13.fFluxCapData[52] = 0.000000; ui13.fFluxCapData[53] = 0.000000; ui13.fFluxCapData[54] = 0.000000; ui13.fFluxCapData[55] = 0.000000; ui13.fFluxCapData[56] = 0.000000; ui13.fFluxCapData[57] = 0.000000; ui13.fFluxCapData[58] = 0.000000; ui13.fFluxCapData[59] = 0.000000; ui13.fFluxCapData[60] = 0.000000; ui13.fFluxCapData[61] = 0.000000; ui13.fFluxCapData[62] = 0.000000; ui13.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui13);


	m_uVectorPathMode = 0;
	CUICtrl ui14;
	ui14.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui14.uControlId = 20;
	ui14.bLogSlider = false;
	ui14.bExpSlider = false;
	ui14.fUserDisplayDataLoLimit = 0.000000;
	ui14.fUserDisplayDataHiLimit = 3.000000;
	ui14.uUserDataType = UINTData;
	ui14.fInitUserIntValue = 0;
	ui14.fInitUserFloatValue = 0;
	ui14.fInitUserDoubleValue = 0;
	ui14.fInitUserUINTValue = 0.000000;
	ui14.m_pUserCookedIntData = NULL;
	ui14.m_pUserCookedFloatData = NULL;
	ui14.m_pUserCookedDoubleData = NULL;
	ui14.m_pUserCookedUINTData = &m_uVectorPathMode;
	ui14.cControlUnits = "                                                                ";
	ui14.cVariableName = "m_uVectorPathMode";
	ui14.cEnumeratedList = "ONCE,LOOP,SUSTN,BckFrth";
	ui14.dPresetData[0] = 0.000000;ui14.dPresetData[1] = 0.000000;ui14.dPresetData[2] = 0.000000;ui14.dPresetData[3] = 0.000000;ui14.dPresetData[4] = 0.000000;ui14.dPresetData[5] = 0.000000;ui14.dPresetData[6] = 0.000000;ui14.dPresetData[7] = 0.000000;ui14.dPresetData[8] = 0.000000;ui14.dPresetData[9] = 0.000000;ui14.dPresetData[10] = 0.000000;ui14.dPresetData[11] = 0.000000;ui14.dPresetData[12] = 0.000000;ui14.dPresetData[13] = 0.000000;ui14.dPresetData[14] = 0.000000;ui14.dPresetData[15] = 0.000000;
	ui14.cControlName = "Path Mode";
	ui14.bOwnerControl = false;
	ui14.bMIDIControl = false;
	ui14.uMIDIControlCommand = 176;
	ui14.uMIDIControlName = 3;
	ui14.uMIDIControlChannel = 0;
	ui14.nGUIRow = -1;
	ui14.nGUIColumn = -1;
	ui14.uControlTheme[0] = 0; ui14.uControlTheme[1] = 0; ui14.uControlTheme[2] = 0; ui14.uControlTheme[3] = 0; ui14.uControlTheme[4] = 0; ui14.uControlTheme[5] = 0; ui14.uControlTheme[6] = 0; ui14.uControlTheme[7] = 0; ui14.uControlTheme[8] = 0; ui14.uControlTheme[9] = 0; ui14.uControlTheme[10] = 0; ui14.uControlTheme[11] = 0; ui14.uControlTheme[12] = 0; ui14.uControlTheme[13] = 0; ui14.uControlTheme[14] = 0; ui14.uControlTheme[15] = 0; ui14.uControlTheme[16] = 0; ui14.uControlTheme[17] = 0; ui14.uControlTheme[18] = 0; ui14.uControlTheme[19] = 0; ui14.uControlTheme[20] = 0; ui14.uControlTheme[21] = 0; ui14.uControlTheme[22] = 0; ui14.uControlTheme[23] = 0; ui14.uControlTheme[24] = 0; ui14.uControlTheme[25] = 0; ui14.uControlTheme[26] = 0; ui14.uControlTheme[27] = 0; ui14.uControlTheme[28] = 0; ui14.uControlTheme[29] = 0; ui14.uControlTheme[30] = 0; ui14.uControlTheme[31] = 0;
	ui14.uFluxCapControl[0] = 0; ui14.uFluxCapControl[1] = 0; ui14.uFluxCapControl[2] = 0; ui14.uFluxCapControl[3] = 0; ui14.uFluxCapControl[4] = 0; ui14.uFluxCapControl[5] = 0; ui14.uFluxCapControl[6] = 0; ui14.uFluxCapControl[7] = 0; ui14.uFluxCapControl[8] = 0; ui14.uFluxCapControl[9] = 0; ui14.uFluxCapControl[10] = 0; ui14.uFluxCapControl[11] = 0; ui14.uFluxCapControl[12] = 0; ui14.uFluxCapControl[13] = 0; ui14.uFluxCapControl[14] = 0; ui14.uFluxCapControl[15] = 0; ui14.uFluxCapControl[16] = 0; ui14.uFluxCapControl[17] = 0; ui14.uFluxCapControl[18] = 0; ui14.uFluxCapControl[19] = 0; ui14.uFluxCapControl[20] = 0; ui14.uFluxCapControl[21] = 0; ui14.uFluxCapControl[22] = 0; ui14.uFluxCapControl[23] = 0; ui14.uFluxCapControl[24] = 0; ui14.uFluxCapControl[25] = 0; ui14.uFluxCapControl[26] = 0; ui14.uFluxCapControl[27] = 0; ui14.uFluxCapControl[28] = 0; ui14.uFluxCapControl[29] = 0; ui14.uFluxCapControl[30] = 0; ui14.uFluxCapControl[31] = 0; ui14.uFluxCapControl[32] = 0; ui14.uFluxCapControl[33] = 0; ui14.uFluxCapControl[34] = 0; ui14.uFluxCapControl[35] = 0; ui14.uFluxCapControl[36] = 0; ui14.uFluxCapControl[37] = 0; ui14.uFluxCapControl[38] = 0; ui14.uFluxCapControl[39] = 0; ui14.uFluxCapControl[40] = 0; ui14.uFluxCapControl[41] = 0; ui14.uFluxCapControl[42] = 0; ui14.uFluxCapControl[43] = 0; ui14.uFluxCapControl[44] = 0; ui14.uFluxCapControl[45] = 0; ui14.uFluxCapControl[46] = 0; ui14.uFluxCapControl[47] = 0; ui14.uFluxCapControl[48] = 0; ui14.uFluxCapControl[49] = 0; ui14.uFluxCapControl[50] = 0; ui14.uFluxCapControl[51] = 0; ui14.uFluxCapControl[52] = 0; ui14.uFluxCapControl[53] = 0; ui14.uFluxCapControl[54] = 0; ui14.uFluxCapControl[55] = 0; ui14.uFluxCapControl[56] = 0; ui14.uFluxCapControl[57] = 0; ui14.uFluxCapControl[58] = 0; ui14.uFluxCapControl[59] = 0; ui14.uFluxCapControl[60] = 0; ui14.uFluxCapControl[61] = 0; ui14.uFluxCapControl[62] = 0; ui14.uFluxCapControl[63] = 0;
	ui14.fFluxCapData[0] = 0.000000; ui14.fFluxCapData[1] = 0.000000; ui14.fFluxCapData[2] = 0.000000; ui14.fFluxCapData[3] = 0.000000; ui14.fFluxCapData[4] = 0.000000; ui14.fFluxCapData[5] = 0.000000; ui14.fFluxCapData[6] = 0.000000; ui14.fFluxCapData[7] = 0.000000; ui14.fFluxCapData[8] = 0.000000; ui14.fFluxCapData[9] = 0.000000; ui14.fFluxCapData[10] = 0.000000; ui14.fFluxCapData[11] = 0.000000; ui14.fFluxCapData[12] = 0.000000; ui14.fFluxCapData[13] = 0.000000; ui14.fFluxCapData[14] = 0.000000; ui14.fFluxCapData[15] = 0.000000; ui14.fFluxCapData[16] = 0.000000; ui14.fFluxCapData[17] = 0.000000; ui14.fFluxCapData[18] = 0.000000; ui14.fFluxCapData[19] = 0.000000; ui14.fFluxCapData[20] = 0.000000; ui14.fFluxCapData[21] = 0.000000; ui14.fFluxCapData[22] = 0.000000; ui14.fFluxCapData[23] = 0.000000; ui14.fFluxCapData[24] = 0.000000; ui14.fFluxCapData[25] = 0.000000; ui14.fFluxCapData[26] = 0.000000; ui14.fFluxCapData[27] = 0.000000; ui14.fFluxCapData[28] = 0.000000; ui14.fFluxCapData[29] = 0.000000; ui14.fFluxCapData[30] = 0.000000; ui14.fFluxCapData[31] = 0.000000; ui14.fFluxCapData[32] = 0.000000; ui14.fFluxCapData[33] = 0.000000; ui14.fFluxCapData[34] = 0.000000; ui14.fFluxCapData[35] = 0.000000; ui14.fFluxCapData[36] = 0.000000; ui14.fFluxCapData[37] = 0.000000; ui14.fFluxCapData[38] = 0.000000; ui14.fFluxCapData[39] = 0.000000; ui14.fFluxCapData[40] = 0.000000; ui14.fFluxCapData[41] = 0.000000; ui14.fFluxCapData[42] = 0.000000; ui14.fFluxCapData[43] = 0.000000; ui14.fFluxCapData[44] = 0.000000; ui14.fFluxCapData[45] = 0.000000; ui14.fFluxCapData[46] = 0.000000; ui14.fFluxCapData[47] = 0.000000; ui14.fFluxCapData[48] = 0.000000; ui14.fFluxCapData[49] = 0.000000; ui14.fFluxCapData[50] = 0.000000; ui14.fFluxCapData[51] = 0.000000; ui14.fFluxCapData[52] = 0.000000; ui14.fFluxCapData[53] = 0.000000; ui14.fFluxCapData[54] = 0.000000; ui14.fFluxCapData[55] = 0.000000; ui14.fFluxCapData[56] = 0.000000; ui14.fFluxCapData[57] = 0.000000; ui14.fFluxCapData[58] = 0.000000; ui14.fFluxCapData[59] = 0.000000; ui14.fFluxCapData[60] = 0.000000; ui14.fFluxCapData[61] = 0.000000; ui14.fFluxCapData[62] = 0.000000; ui14.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui14);


	m_dEG1OscIntensity = 0.000000;
	CUICtrl ui15;
	ui15.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui15.uControlId = 21;
	ui15.bLogSlider = false;
	ui15.bExpSlider = false;
	ui15.fUserDisplayDataLoLimit = -1.000000;
	ui15.fUserDisplayDataHiLimit = 1.000000;
	ui15.uUserDataType = doubleData;
	ui15.fInitUserIntValue = 0;
	ui15.fInitUserFloatValue = 0;
	ui15.fInitUserDoubleValue = 0.000000;
	ui15.fInitUserUINTValue = 0;
	ui15.m_pUserCookedIntData = NULL;
	ui15.m_pUserCookedFloatData = NULL;
	ui15.m_pUserCookedDoubleData = &m_dEG1OscIntensity;
	ui15.m_pUserCookedUINTData = NULL;
	ui15.cControlUnits = "                                                                ";
	ui15.cVariableName = "m_dEG1OscIntensity";
	ui15.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui15.dPresetData[0] = 0.000000;ui15.dPresetData[1] = 0.000000;ui15.dPresetData[2] = 0.000000;ui15.dPresetData[3] = 0.000000;ui15.dPresetData[4] = 0.000000;ui15.dPresetData[5] = 0.000000;ui15.dPresetData[6] = 0.000000;ui15.dPresetData[7] = 0.000000;ui15.dPresetData[8] = 0.000000;ui15.dPresetData[9] = 0.000000;ui15.dPresetData[10] = 0.000000;ui15.dPresetData[11] = 0.000000;ui15.dPresetData[12] = 0.000000;ui15.dPresetData[13] = 0.000000;ui15.dPresetData[14] = 0.000000;ui15.dPresetData[15] = 0.000000;
	ui15.cControlName = "Osc EG Int";
	ui15.bOwnerControl = false;
	ui15.bMIDIControl = false;
	ui15.uMIDIControlCommand = 176;
	ui15.uMIDIControlName = 3;
	ui15.uMIDIControlChannel = 0;
	ui15.nGUIRow = -1;
	ui15.nGUIColumn = -1;
	ui15.uControlTheme[0] = 0; ui15.uControlTheme[1] = 0; ui15.uControlTheme[2] = 0; ui15.uControlTheme[3] = 0; ui15.uControlTheme[4] = 0; ui15.uControlTheme[5] = 0; ui15.uControlTheme[6] = 0; ui15.uControlTheme[7] = 0; ui15.uControlTheme[8] = 0; ui15.uControlTheme[9] = 0; ui15.uControlTheme[10] = 0; ui15.uControlTheme[11] = 0; ui15.uControlTheme[12] = 0; ui15.uControlTheme[13] = 0; ui15.uControlTheme[14] = 0; ui15.uControlTheme[15] = 0; ui15.uControlTheme[16] = 0; ui15.uControlTheme[17] = 0; ui15.uControlTheme[18] = 0; ui15.uControlTheme[19] = 0; ui15.uControlTheme[20] = 0; ui15.uControlTheme[21] = 0; ui15.uControlTheme[22] = 0; ui15.uControlTheme[23] = 0; ui15.uControlTheme[24] = 0; ui15.uControlTheme[25] = 0; ui15.uControlTheme[26] = 0; ui15.uControlTheme[27] = 0; ui15.uControlTheme[28] = 0; ui15.uControlTheme[29] = 0; ui15.uControlTheme[30] = 0; ui15.uControlTheme[31] = 0;
	ui15.uFluxCapControl[0] = 0; ui15.uFluxCapControl[1] = 0; ui15.uFluxCapControl[2] = 0; ui15.uFluxCapControl[3] = 0; ui15.uFluxCapControl[4] = 0; ui15.uFluxCapControl[5] = 0; ui15.uFluxCapControl[6] = 0; ui15.uFluxCapControl[7] = 0; ui15.uFluxCapControl[8] = 0; ui15.uFluxCapControl[9] = 0; ui15.uFluxCapControl[10] = 0; ui15.uFluxCapControl[11] = 0; ui15.uFluxCapControl[12] = 0; ui15.uFluxCapControl[13] = 0; ui15.uFluxCapControl[14] = 0; ui15.uFluxCapControl[15] = 0; ui15.uFluxCapControl[16] = 0; ui15.uFluxCapControl[17] = 0; ui15.uFluxCapControl[18] = 0; ui15.uFluxCapControl[19] = 0; ui15.uFluxCapControl[20] = 0; ui15.uFluxCapControl[21] = 0; ui15.uFluxCapControl[22] = 0; ui15.uFluxCapControl[23] = 0; ui15.uFluxCapControl[24] = 0; ui15.uFluxCapControl[25] = 0; ui15.uFluxCapControl[26] = 0; ui15.uFluxCapControl[27] = 0; ui15.uFluxCapControl[28] = 0; ui15.uFluxCapControl[29] = 0; ui15.uFluxCapControl[30] = 0; ui15.uFluxCapControl[31] = 0; ui15.uFluxCapControl[32] = 0; ui15.uFluxCapControl[33] = 0; ui15.uFluxCapControl[34] = 0; ui15.uFluxCapControl[35] = 0; ui15.uFluxCapControl[36] = 0; ui15.uFluxCapControl[37] = 0; ui15.uFluxCapControl[38] = 0; ui15.uFluxCapControl[39] = 0; ui15.uFluxCapControl[40] = 0; ui15.uFluxCapControl[41] = 0; ui15.uFluxCapControl[42] = 0; ui15.uFluxCapControl[43] = 0; ui15.uFluxCapControl[44] = 0; ui15.uFluxCapControl[45] = 0; ui15.uFluxCapControl[46] = 0; ui15.uFluxCapControl[47] = 0; ui15.uFluxCapControl[48] = 0; ui15.uFluxCapControl[49] = 0; ui15.uFluxCapControl[50] = 0; ui15.uFluxCapControl[51] = 0; ui15.uFluxCapControl[52] = 0; ui15.uFluxCapControl[53] = 0; ui15.uFluxCapControl[54] = 0; ui15.uFluxCapControl[55] = 0; ui15.uFluxCapControl[56] = 0; ui15.uFluxCapControl[57] = 0; ui15.uFluxCapControl[58] = 0; ui15.uFluxCapControl[59] = 0; ui15.uFluxCapControl[60] = 0; ui15.uFluxCapControl[61] = 0; ui15.uFluxCapControl[62] = 0; ui15.uFluxCapControl[63] = 0;
	ui15.fFluxCapData[0] = 0.000000; ui15.fFluxCapData[1] = 0.000000; ui15.fFluxCapData[2] = 0.000000; ui15.fFluxCapData[3] = 0.000000; ui15.fFluxCapData[4] = 0.000000; ui15.fFluxCapData[5] = 0.000000; ui15.fFluxCapData[6] = 0.000000; ui15.fFluxCapData[7] = 0.000000; ui15.fFluxCapData[8] = 0.000000; ui15.fFluxCapData[9] = 0.000000; ui15.fFluxCapData[10] = 0.000000; ui15.fFluxCapData[11] = 0.000000; ui15.fFluxCapData[12] = 0.000000; ui15.fFluxCapData[13] = 0.000000; ui15.fFluxCapData[14] = 0.000000; ui15.fFluxCapData[15] = 0.000000; ui15.fFluxCapData[16] = 0.000000; ui15.fFluxCapData[17] = 0.000000; ui15.fFluxCapData[18] = 0.000000; ui15.fFluxCapData[19] = 0.000000; ui15.fFluxCapData[20] = 0.000000; ui15.fFluxCapData[21] = 0.000000; ui15.fFluxCapData[22] = 0.000000; ui15.fFluxCapData[23] = 0.000000; ui15.fFluxCapData[24] = 0.000000; ui15.fFluxCapData[25] = 0.000000; ui15.fFluxCapData[26] = 0.000000; ui15.fFluxCapData[27] = 0.000000; ui15.fFluxCapData[28] = 0.000000; ui15.fFluxCapData[29] = 0.000000; ui15.fFluxCapData[30] = 0.000000; ui15.fFluxCapData[31] = 0.000000; ui15.fFluxCapData[32] = 0.000000; ui15.fFluxCapData[33] = 0.000000; ui15.fFluxCapData[34] = 0.000000; ui15.fFluxCapData[35] = 0.000000; ui15.fFluxCapData[36] = 0.000000; ui15.fFluxCapData[37] = 0.000000; ui15.fFluxCapData[38] = 0.000000; ui15.fFluxCapData[39] = 0.000000; ui15.fFluxCapData[40] = 0.000000; ui15.fFluxCapData[41] = 0.000000; ui15.fFluxCapData[42] = 0.000000; ui15.fFluxCapData[43] = 0.000000; ui15.fFluxCapData[44] = 0.000000; ui15.fFluxCapData[45] = 0.000000; ui15.fFluxCapData[46] = 0.000000; ui15.fFluxCapData[47] = 0.000000; ui15.fFluxCapData[48] = 0.000000; ui15.fFluxCapData[49] = 0.000000; ui15.fFluxCapData[50] = 0.000000; ui15.fFluxCapData[51] = 0.000000; ui15.fFluxCapData[52] = 0.000000; ui15.fFluxCapData[53] = 0.000000; ui15.fFluxCapData[54] = 0.000000; ui15.fFluxCapData[55] = 0.000000; ui15.fFluxCapData[56] = 0.000000; ui15.fFluxCapData[57] = 0.000000; ui15.fFluxCapData[58] = 0.000000; ui15.fFluxCapData[59] = 0.000000; ui15.fFluxCapData[60] = 0.000000; ui15.fFluxCapData[61] = 0.000000; ui15.fFluxCapData[62] = 0.000000; ui15.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui15);


	m_dEG1FilterIntensity = 0.000000;
	CUICtrl ui16;
	ui16.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui16.uControlId = 22;
	ui16.bLogSlider = false;
	ui16.bExpSlider = false;
	ui16.fUserDisplayDataLoLimit = -1.000000;
	ui16.fUserDisplayDataHiLimit = 1.000000;
	ui16.uUserDataType = doubleData;
	ui16.fInitUserIntValue = 0;
	ui16.fInitUserFloatValue = 0;
	ui16.fInitUserDoubleValue = 0.000000;
	ui16.fInitUserUINTValue = 0;
	ui16.m_pUserCookedIntData = NULL;
	ui16.m_pUserCookedFloatData = NULL;
	ui16.m_pUserCookedDoubleData = &m_dEG1FilterIntensity;
	ui16.m_pUserCookedUINTData = NULL;
	ui16.cControlUnits = "                                                                ";
	ui16.cVariableName = "m_dEG1FilterIntensity";
	ui16.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui16.dPresetData[0] = 0.000000;ui16.dPresetData[1] = 0.000000;ui16.dPresetData[2] = 0.000000;ui16.dPresetData[3] = 0.000000;ui16.dPresetData[4] = 0.000000;ui16.dPresetData[5] = 0.000000;ui16.dPresetData[6] = 0.000000;ui16.dPresetData[7] = 0.000000;ui16.dPresetData[8] = 0.000000;ui16.dPresetData[9] = 0.000000;ui16.dPresetData[10] = 0.000000;ui16.dPresetData[11] = 0.000000;ui16.dPresetData[12] = 0.000000;ui16.dPresetData[13] = 0.000000;ui16.dPresetData[14] = 0.000000;ui16.dPresetData[15] = 0.000000;
	ui16.cControlName = "Filter  EG Int";
	ui16.bOwnerControl = false;
	ui16.bMIDIControl = false;
	ui16.uMIDIControlCommand = 176;
	ui16.uMIDIControlName = 3;
	ui16.uMIDIControlChannel = 0;
	ui16.nGUIRow = -1;
	ui16.nGUIColumn = -1;
	ui16.uControlTheme[0] = 0; ui16.uControlTheme[1] = 0; ui16.uControlTheme[2] = 0; ui16.uControlTheme[3] = 0; ui16.uControlTheme[4] = 0; ui16.uControlTheme[5] = 0; ui16.uControlTheme[6] = 0; ui16.uControlTheme[7] = 0; ui16.uControlTheme[8] = 0; ui16.uControlTheme[9] = 0; ui16.uControlTheme[10] = 0; ui16.uControlTheme[11] = 0; ui16.uControlTheme[12] = 0; ui16.uControlTheme[13] = 0; ui16.uControlTheme[14] = 0; ui16.uControlTheme[15] = 0; ui16.uControlTheme[16] = 0; ui16.uControlTheme[17] = 0; ui16.uControlTheme[18] = 0; ui16.uControlTheme[19] = 0; ui16.uControlTheme[20] = 0; ui16.uControlTheme[21] = 0; ui16.uControlTheme[22] = 0; ui16.uControlTheme[23] = 0; ui16.uControlTheme[24] = 0; ui16.uControlTheme[25] = 0; ui16.uControlTheme[26] = 0; ui16.uControlTheme[27] = 0; ui16.uControlTheme[28] = 0; ui16.uControlTheme[29] = 0; ui16.uControlTheme[30] = 0; ui16.uControlTheme[31] = 0;
	ui16.uFluxCapControl[0] = 0; ui16.uFluxCapControl[1] = 0; ui16.uFluxCapControl[2] = 0; ui16.uFluxCapControl[3] = 0; ui16.uFluxCapControl[4] = 0; ui16.uFluxCapControl[5] = 0; ui16.uFluxCapControl[6] = 0; ui16.uFluxCapControl[7] = 0; ui16.uFluxCapControl[8] = 0; ui16.uFluxCapControl[9] = 0; ui16.uFluxCapControl[10] = 0; ui16.uFluxCapControl[11] = 0; ui16.uFluxCapControl[12] = 0; ui16.uFluxCapControl[13] = 0; ui16.uFluxCapControl[14] = 0; ui16.uFluxCapControl[15] = 0; ui16.uFluxCapControl[16] = 0; ui16.uFluxCapControl[17] = 0; ui16.uFluxCapControl[18] = 0; ui16.uFluxCapControl[19] = 0; ui16.uFluxCapControl[20] = 0; ui16.uFluxCapControl[21] = 0; ui16.uFluxCapControl[22] = 0; ui16.uFluxCapControl[23] = 0; ui16.uFluxCapControl[24] = 0; ui16.uFluxCapControl[25] = 0; ui16.uFluxCapControl[26] = 0; ui16.uFluxCapControl[27] = 0; ui16.uFluxCapControl[28] = 0; ui16.uFluxCapControl[29] = 0; ui16.uFluxCapControl[30] = 0; ui16.uFluxCapControl[31] = 0; ui16.uFluxCapControl[32] = 0; ui16.uFluxCapControl[33] = 0; ui16.uFluxCapControl[34] = 0; ui16.uFluxCapControl[35] = 0; ui16.uFluxCapControl[36] = 0; ui16.uFluxCapControl[37] = 0; ui16.uFluxCapControl[38] = 0; ui16.uFluxCapControl[39] = 0; ui16.uFluxCapControl[40] = 0; ui16.uFluxCapControl[41] = 0; ui16.uFluxCapControl[42] = 0; ui16.uFluxCapControl[43] = 0; ui16.uFluxCapControl[44] = 0; ui16.uFluxCapControl[45] = 0; ui16.uFluxCapControl[46] = 0; ui16.uFluxCapControl[47] = 0; ui16.uFluxCapControl[48] = 0; ui16.uFluxCapControl[49] = 0; ui16.uFluxCapControl[50] = 0; ui16.uFluxCapControl[51] = 0; ui16.uFluxCapControl[52] = 0; ui16.uFluxCapControl[53] = 0; ui16.uFluxCapControl[54] = 0; ui16.uFluxCapControl[55] = 0; ui16.uFluxCapControl[56] = 0; ui16.uFluxCapControl[57] = 0; ui16.uFluxCapControl[58] = 0; ui16.uFluxCapControl[59] = 0; ui16.uFluxCapControl[60] = 0; ui16.uFluxCapControl[61] = 0; ui16.uFluxCapControl[62] = 0; ui16.uFluxCapControl[63] = 0;
	ui16.fFluxCapData[0] = 0.000000; ui16.fFluxCapData[1] = 0.000000; ui16.fFluxCapData[2] = 0.000000; ui16.fFluxCapData[3] = 0.000000; ui16.fFluxCapData[4] = 0.000000; ui16.fFluxCapData[5] = 0.000000; ui16.fFluxCapData[6] = 0.000000; ui16.fFluxCapData[7] = 0.000000; ui16.fFluxCapData[8] = 0.000000; ui16.fFluxCapData[9] = 0.000000; ui16.fFluxCapData[10] = 0.000000; ui16.fFluxCapData[11] = 0.000000; ui16.fFluxCapData[12] = 0.000000; ui16.fFluxCapData[13] = 0.000000; ui16.fFluxCapData[14] = 0.000000; ui16.fFluxCapData[15] = 0.000000; ui16.fFluxCapData[16] = 0.000000; ui16.fFluxCapData[17] = 0.000000; ui16.fFluxCapData[18] = 0.000000; ui16.fFluxCapData[19] = 0.000000; ui16.fFluxCapData[20] = 0.000000; ui16.fFluxCapData[21] = 0.000000; ui16.fFluxCapData[22] = 0.000000; ui16.fFluxCapData[23] = 0.000000; ui16.fFluxCapData[24] = 0.000000; ui16.fFluxCapData[25] = 0.000000; ui16.fFluxCapData[26] = 0.000000; ui16.fFluxCapData[27] = 0.000000; ui16.fFluxCapData[28] = 0.000000; ui16.fFluxCapData[29] = 0.000000; ui16.fFluxCapData[30] = 0.000000; ui16.fFluxCapData[31] = 0.000000; ui16.fFluxCapData[32] = 0.000000; ui16.fFluxCapData[33] = 0.000000; ui16.fFluxCapData[34] = 0.000000; ui16.fFluxCapData[35] = 0.000000; ui16.fFluxCapData[36] = 0.000000; ui16.fFluxCapData[37] = 0.000000; ui16.fFluxCapData[38] = 0.000000; ui16.fFluxCapData[39] = 0.000000; ui16.fFluxCapData[40] = 0.000000; ui16.fFluxCapData[41] = 0.000000; ui16.fFluxCapData[42] = 0.000000; ui16.fFluxCapData[43] = 0.000000; ui16.fFluxCapData[44] = 0.000000; ui16.fFluxCapData[45] = 0.000000; ui16.fFluxCapData[46] = 0.000000; ui16.fFluxCapData[47] = 0.000000; ui16.fFluxCapData[48] = 0.000000; ui16.fFluxCapData[49] = 0.000000; ui16.fFluxCapData[50] = 0.000000; ui16.fFluxCapData[51] = 0.000000; ui16.fFluxCapData[52] = 0.000000; ui16.fFluxCapData[53] = 0.000000; ui16.fFluxCapData[54] = 0.000000; ui16.fFluxCapData[55] = 0.000000; ui16.fFluxCapData[56] = 0.000000; ui16.fFluxCapData[57] = 0.000000; ui16.fFluxCapData[58] = 0.000000; ui16.fFluxCapData[59] = 0.000000; ui16.fFluxCapData[60] = 0.000000; ui16.fFluxCapData[61] = 0.000000; ui16.fFluxCapData[62] = 0.000000; ui16.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui16);


	m_dLFO1FilterFcIntensity = 0.000000;
	CUICtrl ui17;
	ui17.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui17.uControlId = 23;
	ui17.bLogSlider = false;
	ui17.bExpSlider = false;
	ui17.fUserDisplayDataLoLimit = -1.000000;
	ui17.fUserDisplayDataHiLimit = 1.000000;
	ui17.uUserDataType = doubleData;
	ui17.fInitUserIntValue = 0;
	ui17.fInitUserFloatValue = 0;
	ui17.fInitUserDoubleValue = 0.000000;
	ui17.fInitUserUINTValue = 0;
	ui17.m_pUserCookedIntData = NULL;
	ui17.m_pUserCookedFloatData = NULL;
	ui17.m_pUserCookedDoubleData = &m_dLFO1FilterFcIntensity;
	ui17.m_pUserCookedUINTData = NULL;
	ui17.cControlUnits = "                                                                ";
	ui17.cVariableName = "m_dLFO1FilterFcIntensity";
	ui17.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui17.dPresetData[0] = 0.000000;ui17.dPresetData[1] = 0.000000;ui17.dPresetData[2] = 0.000000;ui17.dPresetData[3] = 0.000000;ui17.dPresetData[4] = 0.000000;ui17.dPresetData[5] = 0.000000;ui17.dPresetData[6] = 0.000000;ui17.dPresetData[7] = 0.000000;ui17.dPresetData[8] = 0.000000;ui17.dPresetData[9] = 0.000000;ui17.dPresetData[10] = 0.000000;ui17.dPresetData[11] = 0.000000;ui17.dPresetData[12] = 0.000000;ui17.dPresetData[13] = 0.000000;ui17.dPresetData[14] = 0.000000;ui17.dPresetData[15] = 0.000000;
	ui17.cControlName = "LFO Cutoff Int";
	ui17.bOwnerControl = false;
	ui17.bMIDIControl = false;
	ui17.uMIDIControlCommand = 176;
	ui17.uMIDIControlName = 3;
	ui17.uMIDIControlChannel = 0;
	ui17.nGUIRow = -1;
	ui17.nGUIColumn = -1;
	ui17.uControlTheme[0] = 0; ui17.uControlTheme[1] = 0; ui17.uControlTheme[2] = 0; ui17.uControlTheme[3] = 0; ui17.uControlTheme[4] = 0; ui17.uControlTheme[5] = 0; ui17.uControlTheme[6] = 0; ui17.uControlTheme[7] = 0; ui17.uControlTheme[8] = 0; ui17.uControlTheme[9] = 0; ui17.uControlTheme[10] = 0; ui17.uControlTheme[11] = 0; ui17.uControlTheme[12] = 0; ui17.uControlTheme[13] = 0; ui17.uControlTheme[14] = 0; ui17.uControlTheme[15] = 0; ui17.uControlTheme[16] = 0; ui17.uControlTheme[17] = 0; ui17.uControlTheme[18] = 0; ui17.uControlTheme[19] = 0; ui17.uControlTheme[20] = 0; ui17.uControlTheme[21] = 0; ui17.uControlTheme[22] = 0; ui17.uControlTheme[23] = 0; ui17.uControlTheme[24] = 0; ui17.uControlTheme[25] = 0; ui17.uControlTheme[26] = 0; ui17.uControlTheme[27] = 0; ui17.uControlTheme[28] = 0; ui17.uControlTheme[29] = 0; ui17.uControlTheme[30] = 0; ui17.uControlTheme[31] = 0;
	ui17.uFluxCapControl[0] = 0; ui17.uFluxCapControl[1] = 0; ui17.uFluxCapControl[2] = 0; ui17.uFluxCapControl[3] = 0; ui17.uFluxCapControl[4] = 0; ui17.uFluxCapControl[5] = 0; ui17.uFluxCapControl[6] = 0; ui17.uFluxCapControl[7] = 0; ui17.uFluxCapControl[8] = 0; ui17.uFluxCapControl[9] = 0; ui17.uFluxCapControl[10] = 0; ui17.uFluxCapControl[11] = 0; ui17.uFluxCapControl[12] = 0; ui17.uFluxCapControl[13] = 0; ui17.uFluxCapControl[14] = 0; ui17.uFluxCapControl[15] = 0; ui17.uFluxCapControl[16] = 0; ui17.uFluxCapControl[17] = 0; ui17.uFluxCapControl[18] = 0; ui17.uFluxCapControl[19] = 0; ui17.uFluxCapControl[20] = 0; ui17.uFluxCapControl[21] = 0; ui17.uFluxCapControl[22] = 0; ui17.uFluxCapControl[23] = 0; ui17.uFluxCapControl[24] = 0; ui17.uFluxCapControl[25] = 0; ui17.uFluxCapControl[26] = 0; ui17.uFluxCapControl[27] = 0; ui17.uFluxCapControl[28] = 0; ui17.uFluxCapControl[29] = 0; ui17.uFluxCapControl[30] = 0; ui17.uFluxCapControl[31] = 0; ui17.uFluxCapControl[32] = 0; ui17.uFluxCapControl[33] = 0; ui17.uFluxCapControl[34] = 0; ui17.uFluxCapControl[35] = 0; ui17.uFluxCapControl[36] = 0; ui17.uFluxCapControl[37] = 0; ui17.uFluxCapControl[38] = 0; ui17.uFluxCapControl[39] = 0; ui17.uFluxCapControl[40] = 0; ui17.uFluxCapControl[41] = 0; ui17.uFluxCapControl[42] = 0; ui17.uFluxCapControl[43] = 0; ui17.uFluxCapControl[44] = 0; ui17.uFluxCapControl[45] = 0; ui17.uFluxCapControl[46] = 0; ui17.uFluxCapControl[47] = 0; ui17.uFluxCapControl[48] = 0; ui17.uFluxCapControl[49] = 0; ui17.uFluxCapControl[50] = 0; ui17.uFluxCapControl[51] = 0; ui17.uFluxCapControl[52] = 0; ui17.uFluxCapControl[53] = 0; ui17.uFluxCapControl[54] = 0; ui17.uFluxCapControl[55] = 0; ui17.uFluxCapControl[56] = 0; ui17.uFluxCapControl[57] = 0; ui17.uFluxCapControl[58] = 0; ui17.uFluxCapControl[59] = 0; ui17.uFluxCapControl[60] = 0; ui17.uFluxCapControl[61] = 0; ui17.uFluxCapControl[62] = 0; ui17.uFluxCapControl[63] = 0;
	ui17.fFluxCapData[0] = 0.000000; ui17.fFluxCapData[1] = 0.000000; ui17.fFluxCapData[2] = 0.000000; ui17.fFluxCapData[3] = 0.000000; ui17.fFluxCapData[4] = 0.000000; ui17.fFluxCapData[5] = 0.000000; ui17.fFluxCapData[6] = 0.000000; ui17.fFluxCapData[7] = 0.000000; ui17.fFluxCapData[8] = 0.000000; ui17.fFluxCapData[9] = 0.000000; ui17.fFluxCapData[10] = 0.000000; ui17.fFluxCapData[11] = 0.000000; ui17.fFluxCapData[12] = 0.000000; ui17.fFluxCapData[13] = 0.000000; ui17.fFluxCapData[14] = 0.000000; ui17.fFluxCapData[15] = 0.000000; ui17.fFluxCapData[16] = 0.000000; ui17.fFluxCapData[17] = 0.000000; ui17.fFluxCapData[18] = 0.000000; ui17.fFluxCapData[19] = 0.000000; ui17.fFluxCapData[20] = 0.000000; ui17.fFluxCapData[21] = 0.000000; ui17.fFluxCapData[22] = 0.000000; ui17.fFluxCapData[23] = 0.000000; ui17.fFluxCapData[24] = 0.000000; ui17.fFluxCapData[25] = 0.000000; ui17.fFluxCapData[26] = 0.000000; ui17.fFluxCapData[27] = 0.000000; ui17.fFluxCapData[28] = 0.000000; ui17.fFluxCapData[29] = 0.000000; ui17.fFluxCapData[30] = 0.000000; ui17.fFluxCapData[31] = 0.000000; ui17.fFluxCapData[32] = 0.000000; ui17.fFluxCapData[33] = 0.000000; ui17.fFluxCapData[34] = 0.000000; ui17.fFluxCapData[35] = 0.000000; ui17.fFluxCapData[36] = 0.000000; ui17.fFluxCapData[37] = 0.000000; ui17.fFluxCapData[38] = 0.000000; ui17.fFluxCapData[39] = 0.000000; ui17.fFluxCapData[40] = 0.000000; ui17.fFluxCapData[41] = 0.000000; ui17.fFluxCapData[42] = 0.000000; ui17.fFluxCapData[43] = 0.000000; ui17.fFluxCapData[44] = 0.000000; ui17.fFluxCapData[45] = 0.000000; ui17.fFluxCapData[46] = 0.000000; ui17.fFluxCapData[47] = 0.000000; ui17.fFluxCapData[48] = 0.000000; ui17.fFluxCapData[49] = 0.000000; ui17.fFluxCapData[50] = 0.000000; ui17.fFluxCapData[51] = 0.000000; ui17.fFluxCapData[52] = 0.000000; ui17.fFluxCapData[53] = 0.000000; ui17.fFluxCapData[54] = 0.000000; ui17.fFluxCapData[55] = 0.000000; ui17.fFluxCapData[56] = 0.000000; ui17.fFluxCapData[57] = 0.000000; ui17.fFluxCapData[58] = 0.000000; ui17.fFluxCapData[59] = 0.000000; ui17.fFluxCapData[60] = 0.000000; ui17.fFluxCapData[61] = 0.000000; ui17.fFluxCapData[62] = 0.000000; ui17.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui17);


	m_dSustainLevel = 0.500000;
	CUICtrl ui18;
	ui18.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui18.uControlId = 24;
	ui18.bLogSlider = false;
	ui18.bExpSlider = false;
	ui18.fUserDisplayDataLoLimit = 0.000000;
	ui18.fUserDisplayDataHiLimit = 1.000000;
	ui18.uUserDataType = doubleData;
	ui18.fInitUserIntValue = 0;
	ui18.fInitUserFloatValue = 0;
	ui18.fInitUserDoubleValue = 0.500000;
	ui18.fInitUserUINTValue = 0;
	ui18.m_pUserCookedIntData = NULL;
	ui18.m_pUserCookedFloatData = NULL;
	ui18.m_pUserCookedDoubleData = &m_dSustainLevel;
	ui18.m_pUserCookedUINTData = NULL;
	ui18.cControlUnits = "                                                                ";
	ui18.cVariableName = "m_dSustainLevel";
	ui18.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui18.dPresetData[0] = 0.500000;ui18.dPresetData[1] = 0.000000;ui18.dPresetData[2] = 0.000000;ui18.dPresetData[3] = 0.000000;ui18.dPresetData[4] = 0.000000;ui18.dPresetData[5] = 0.000000;ui18.dPresetData[6] = 0.000000;ui18.dPresetData[7] = 0.000000;ui18.dPresetData[8] = 0.000000;ui18.dPresetData[9] = 0.000000;ui18.dPresetData[10] = 0.000000;ui18.dPresetData[11] = 0.000000;ui18.dPresetData[12] = 0.000000;ui18.dPresetData[13] = 0.000000;ui18.dPresetData[14] = 0.000000;ui18.dPresetData[15] = 0.000000;
	ui18.cControlName = "Sustain";
	ui18.bOwnerControl = false;
	ui18.bMIDIControl = false;
	ui18.uMIDIControlCommand = 176;
	ui18.uMIDIControlName = 3;
	ui18.uMIDIControlChannel = 0;
	ui18.nGUIRow = -1;
	ui18.nGUIColumn = -1;
	ui18.uControlTheme[0] = 0; ui18.uControlTheme[1] = 0; ui18.uControlTheme[2] = 0; ui18.uControlTheme[3] = 0; ui18.uControlTheme[4] = 0; ui18.uControlTheme[5] = 0; ui18.uControlTheme[6] = 0; ui18.uControlTheme[7] = 0; ui18.uControlTheme[8] = 0; ui18.uControlTheme[9] = 0; ui18.uControlTheme[10] = 0; ui18.uControlTheme[11] = 0; ui18.uControlTheme[12] = 0; ui18.uControlTheme[13] = 0; ui18.uControlTheme[14] = 0; ui18.uControlTheme[15] = 0; ui18.uControlTheme[16] = 0; ui18.uControlTheme[17] = 0; ui18.uControlTheme[18] = 0; ui18.uControlTheme[19] = 0; ui18.uControlTheme[20] = 0; ui18.uControlTheme[21] = 0; ui18.uControlTheme[22] = 0; ui18.uControlTheme[23] = 0; ui18.uControlTheme[24] = 0; ui18.uControlTheme[25] = 0; ui18.uControlTheme[26] = 0; ui18.uControlTheme[27] = 0; ui18.uControlTheme[28] = 0; ui18.uControlTheme[29] = 0; ui18.uControlTheme[30] = 0; ui18.uControlTheme[31] = 0;
	ui18.uFluxCapControl[0] = 0; ui18.uFluxCapControl[1] = 0; ui18.uFluxCapControl[2] = 0; ui18.uFluxCapControl[3] = 0; ui18.uFluxCapControl[4] = 0; ui18.uFluxCapControl[5] = 0; ui18.uFluxCapControl[6] = 0; ui18.uFluxCapControl[7] = 0; ui18.uFluxCapControl[8] = 0; ui18.uFluxCapControl[9] = 0; ui18.uFluxCapControl[10] = 0; ui18.uFluxCapControl[11] = 0; ui18.uFluxCapControl[12] = 0; ui18.uFluxCapControl[13] = 0; ui18.uFluxCapControl[14] = 0; ui18.uFluxCapControl[15] = 0; ui18.uFluxCapControl[16] = 0; ui18.uFluxCapControl[17] = 0; ui18.uFluxCapControl[18] = 0; ui18.uFluxCapControl[19] = 0; ui18.uFluxCapControl[20] = 0; ui18.uFluxCapControl[21] = 0; ui18.uFluxCapControl[22] = 0; ui18.uFluxCapControl[23] = 0; ui18.uFluxCapControl[24] = 0; ui18.uFluxCapControl[25] = 0; ui18.uFluxCapControl[26] = 0; ui18.uFluxCapControl[27] = 0; ui18.uFluxCapControl[28] = 0; ui18.uFluxCapControl[29] = 0; ui18.uFluxCapControl[30] = 0; ui18.uFluxCapControl[31] = 0; ui18.uFluxCapControl[32] = 0; ui18.uFluxCapControl[33] = 0; ui18.uFluxCapControl[34] = 0; ui18.uFluxCapControl[35] = 0; ui18.uFluxCapControl[36] = 0; ui18.uFluxCapControl[37] = 0; ui18.uFluxCapControl[38] = 0; ui18.uFluxCapControl[39] = 0; ui18.uFluxCapControl[40] = 0; ui18.uFluxCapControl[41] = 0; ui18.uFluxCapControl[42] = 0; ui18.uFluxCapControl[43] = 0; ui18.uFluxCapControl[44] = 0; ui18.uFluxCapControl[45] = 0; ui18.uFluxCapControl[46] = 0; ui18.uFluxCapControl[47] = 0; ui18.uFluxCapControl[48] = 0; ui18.uFluxCapControl[49] = 0; ui18.uFluxCapControl[50] = 0; ui18.uFluxCapControl[51] = 0; ui18.uFluxCapControl[52] = 0; ui18.uFluxCapControl[53] = 0; ui18.uFluxCapControl[54] = 0; ui18.uFluxCapControl[55] = 0; ui18.uFluxCapControl[56] = 0; ui18.uFluxCapControl[57] = 0; ui18.uFluxCapControl[58] = 0; ui18.uFluxCapControl[59] = 0; ui18.uFluxCapControl[60] = 0; ui18.uFluxCapControl[61] = 0; ui18.uFluxCapControl[62] = 0; ui18.uFluxCapControl[63] = 0;
	ui18.fFluxCapData[0] = 0.000000; ui18.fFluxCapData[1] = 0.000000; ui18.fFluxCapData[2] = 0.000000; ui18.fFluxCapData[3] = 0.000000; ui18.fFluxCapData[4] = 0.000000; ui18.fFluxCapData[5] = 0.000000; ui18.fFluxCapData[6] = 0.000000; ui18.fFluxCapData[7] = 0.000000; ui18.fFluxCapData[8] = 0.000000; ui18.fFluxCapData[9] = 0.000000; ui18.fFluxCapData[10] = 0.000000; ui18.fFluxCapData[11] = 0.000000; ui18.fFluxCapData[12] = 0.000000; ui18.fFluxCapData[13] = 0.000000; ui18.fFluxCapData[14] = 0.000000; ui18.fFluxCapData[15] = 0.000000; ui18.fFluxCapData[16] = 0.000000; ui18.fFluxCapData[17] = 0.000000; ui18.fFluxCapData[18] = 0.000000; ui18.fFluxCapData[19] = 0.000000; ui18.fFluxCapData[20] = 0.000000; ui18.fFluxCapData[21] = 0.000000; ui18.fFluxCapData[22] = 0.000000; ui18.fFluxCapData[23] = 0.000000; ui18.fFluxCapData[24] = 0.000000; ui18.fFluxCapData[25] = 0.000000; ui18.fFluxCapData[26] = 0.000000; ui18.fFluxCapData[27] = 0.000000; ui18.fFluxCapData[28] = 0.000000; ui18.fFluxCapData[29] = 0.000000; ui18.fFluxCapData[30] = 0.000000; ui18.fFluxCapData[31] = 0.000000; ui18.fFluxCapData[32] = 0.000000; ui18.fFluxCapData[33] = 0.000000; ui18.fFluxCapData[34] = 0.000000; ui18.fFluxCapData[35] = 0.000000; ui18.fFluxCapData[36] = 0.000000; ui18.fFluxCapData[37] = 0.000000; ui18.fFluxCapData[38] = 0.000000; ui18.fFluxCapData[39] = 0.000000; ui18.fFluxCapData[40] = 0.000000; ui18.fFluxCapData[41] = 0.000000; ui18.fFluxCapData[42] = 0.000000; ui18.fFluxCapData[43] = 0.000000; ui18.fFluxCapData[44] = 0.000000; ui18.fFluxCapData[45] = 0.000000; ui18.fFluxCapData[46] = 0.000000; ui18.fFluxCapData[47] = 0.000000; ui18.fFluxCapData[48] = 0.000000; ui18.fFluxCapData[49] = 0.000000; ui18.fFluxCapData[50] = 0.000000; ui18.fFluxCapData[51] = 0.000000; ui18.fFluxCapData[52] = 0.000000; ui18.fFluxCapData[53] = 0.000000; ui18.fFluxCapData[54] = 0.000000; ui18.fFluxCapData[55] = 0.000000; ui18.fFluxCapData[56] = 0.000000; ui18.fFluxCapData[57] = 0.000000; ui18.fFluxCapData[58] = 0.000000; ui18.fFluxCapData[59] = 0.000000; ui18.fFluxCapData[60] = 0.000000; ui18.fFluxCapData[61] = 0.000000; ui18.fFluxCapData[62] = 0.000000; ui18.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui18);


	m_dRotorRate = 0.500000;
	CUICtrl ui19;
	ui19.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui19.uControlId = 25;
	ui19.bLogSlider = false;
	ui19.bExpSlider = false;
	ui19.fUserDisplayDataLoLimit = 0.010000;
	ui19.fUserDisplayDataHiLimit = 2.000000;
	ui19.uUserDataType = doubleData;
	ui19.fInitUserIntValue = 0;
	ui19.fInitUserFloatValue = 0;
	ui19.fInitUserDoubleValue = 0.500000;
	ui19.fInitUserUINTValue = 0;
	ui19.m_pUserCookedIntData = NULL;
	ui19.m_pUserCookedFloatData = NULL;
	ui19.m_pUserCookedDoubleData = &m_dRotorRate;
	ui19.m_pUserCookedUINTData = NULL;
	ui19.cControlUnits = "Hz                                                              ";
	ui19.cVariableName = "m_dRotorRate";
	ui19.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui19.dPresetData[0] = 0.500000;ui19.dPresetData[1] = 0.000000;ui19.dPresetData[2] = 0.000000;ui19.dPresetData[3] = 0.000000;ui19.dPresetData[4] = 0.000000;ui19.dPresetData[5] = 0.000000;ui19.dPresetData[6] = 0.000000;ui19.dPresetData[7] = 0.000000;ui19.dPresetData[8] = 0.000000;ui19.dPresetData[9] = 0.000000;ui19.dPresetData[10] = 0.000000;ui19.dPresetData[11] = 0.000000;ui19.dPresetData[12] = 0.000000;ui19.dPresetData[13] = 0.000000;ui19.dPresetData[14] = 0.000000;ui19.dPresetData[15] = 0.000000;
	ui19.cControlName = "Rotor Rate";
	ui19.bOwnerControl = false;
	ui19.bMIDIControl = false;
	ui19.uMIDIControlCommand = 176;
	ui19.uMIDIControlName = 3;
	ui19.uMIDIControlChannel = 0;
	ui19.nGUIRow = -1;
	ui19.nGUIColumn = -1;
	ui19.uControlTheme[0] = 0; ui19.uControlTheme[1] = 0; ui19.uControlTheme[2] = 0; ui19.uControlTheme[3] = 0; ui19.uControlTheme[4] = 0; ui19.uControlTheme[5] = 0; ui19.uControlTheme[6] = 0; ui19.uControlTheme[7] = 0; ui19.uControlTheme[8] = 0; ui19.uControlTheme[9] = 0; ui19.uControlTheme[10] = 0; ui19.uControlTheme[11] = 0; ui19.uControlTheme[12] = 0; ui19.uControlTheme[13] = 0; ui19.uControlTheme[14] = 0; ui19.uControlTheme[15] = 0; ui19.uControlTheme[16] = 0; ui19.uControlTheme[17] = 0; ui19.uControlTheme[18] = 0; ui19.uControlTheme[19] = 0; ui19.uControlTheme[20] = 0; ui19.uControlTheme[21] = 0; ui19.uControlTheme[22] = 0; ui19.uControlTheme[23] = 0; ui19.uControlTheme[24] = 0; ui19.uControlTheme[25] = 0; ui19.uControlTheme[26] = 0; ui19.uControlTheme[27] = 0; ui19.uControlTheme[28] = 0; ui19.uControlTheme[29] = 0; ui19.uControlTheme[30] = 0; ui19.uControlTheme[31] = 0;
	ui19.uFluxCapControl[0] = 0; ui19.uFluxCapControl[1] = 0; ui19.uFluxCapControl[2] = 0; ui19.uFluxCapControl[3] = 0; ui19.uFluxCapControl[4] = 0; ui19.uFluxCapControl[5] = 0; ui19.uFluxCapControl[6] = 0; ui19.uFluxCapControl[7] = 0; ui19.uFluxCapControl[8] = 0; ui19.uFluxCapControl[9] = 0; ui19.uFluxCapControl[10] = 0; ui19.uFluxCapControl[11] = 0; ui19.uFluxCapControl[12] = 0; ui19.uFluxCapControl[13] = 0; ui19.uFluxCapControl[14] = 0; ui19.uFluxCapControl[15] = 0; ui19.uFluxCapControl[16] = 0; ui19.uFluxCapControl[17] = 0; ui19.uFluxCapControl[18] = 0; ui19.uFluxCapControl[19] = 0; ui19.uFluxCapControl[20] = 0; ui19.uFluxCapControl[21] = 0; ui19.uFluxCapControl[22] = 0; ui19.uFluxCapControl[23] = 0; ui19.uFluxCapControl[24] = 0; ui19.uFluxCapControl[25] = 0; ui19.uFluxCapControl[26] = 0; ui19.uFluxCapControl[27] = 0; ui19.uFluxCapControl[28] = 0; ui19.uFluxCapControl[29] = 0; ui19.uFluxCapControl[30] = 0; ui19.uFluxCapControl[31] = 0; ui19.uFluxCapControl[32] = 0; ui19.uFluxCapControl[33] = 0; ui19.uFluxCapControl[34] = 0; ui19.uFluxCapControl[35] = 0; ui19.uFluxCapControl[36] = 0; ui19.uFluxCapControl[37] = 0; ui19.uFluxCapControl[38] = 0; ui19.uFluxCapControl[39] = 0; ui19.uFluxCapControl[40] = 0; ui19.uFluxCapControl[41] = 0; ui19.uFluxCapControl[42] = 0; ui19.uFluxCapControl[43] = 0; ui19.uFluxCapControl[44] = 0; ui19.uFluxCapControl[45] = 0; ui19.uFluxCapControl[46] = 0; ui19.uFluxCapControl[47] = 0; ui19.uFluxCapControl[48] = 0; ui19.uFluxCapControl[49] = 0; ui19.uFluxCapControl[50] = 0; ui19.uFluxCapControl[51] = 0; ui19.uFluxCapControl[52] = 0; ui19.uFluxCapControl[53] = 0; ui19.uFluxCapControl[54] = 0; ui19.uFluxCapControl[55] = 0; ui19.uFluxCapControl[56] = 0; ui19.uFluxCapControl[57] = 0; ui19.uFluxCapControl[58] = 0; ui19.uFluxCapControl[59] = 0; ui19.uFluxCapControl[60] = 0; ui19.uFluxCapControl[61] = 0; ui19.uFluxCapControl[62] = 0; ui19.uFluxCapControl[63] = 0;
	ui19.fFluxCapData[0] = 0.000000; ui19.fFluxCapData[1] = 0.000000; ui19.fFluxCapData[2] = 0.000000; ui19.fFluxCapData[3] = 0.000000; ui19.fFluxCapData[4] = 0.000000; ui19.fFluxCapData[5] = 0.000000; ui19.fFluxCapData[6] = 0.000000; ui19.fFluxCapData[7] = 0.000000; ui19.fFluxCapData[8] = 0.000000; ui19.fFluxCapData[9] = 0.000000; ui19.fFluxCapData[10] = 0.000000; ui19.fFluxCapData[11] = 0.000000; ui19.fFluxCapData[12] = 0.000000; ui19.fFluxCapData[13] = 0.000000; ui19.fFluxCapData[14] = 0.000000; ui19.fFluxCapData[15] = 0.000000; ui19.fFluxCapData[16] = 0.000000; ui19.fFluxCapData[17] = 0.000000; ui19.fFluxCapData[18] = 0.000000; ui19.fFluxCapData[19] = 0.000000; ui19.fFluxCapData[20] = 0.000000; ui19.fFluxCapData[21] = 0.000000; ui19.fFluxCapData[22] = 0.000000; ui19.fFluxCapData[23] = 0.000000; ui19.fFluxCapData[24] = 0.000000; ui19.fFluxCapData[25] = 0.000000; ui19.fFluxCapData[26] = 0.000000; ui19.fFluxCapData[27] = 0.000000; ui19.fFluxCapData[28] = 0.000000; ui19.fFluxCapData[29] = 0.000000; ui19.fFluxCapData[30] = 0.000000; ui19.fFluxCapData[31] = 0.000000; ui19.fFluxCapData[32] = 0.000000; ui19.fFluxCapData[33] = 0.000000; ui19.fFluxCapData[34] = 0.000000; ui19.fFluxCapData[35] = 0.000000; ui19.fFluxCapData[36] = 0.000000; ui19.fFluxCapData[37] = 0.000000; ui19.fFluxCapData[38] = 0.000000; ui19.fFluxCapData[39] = 0.000000; ui19.fFluxCapData[40] = 0.000000; ui19.fFluxCapData[41] = 0.000000; ui19.fFluxCapData[42] = 0.000000; ui19.fFluxCapData[43] = 0.000000; ui19.fFluxCapData[44] = 0.000000; ui19.fFluxCapData[45] = 0.000000; ui19.fFluxCapData[46] = 0.000000; ui19.fFluxCapData[47] = 0.000000; ui19.fFluxCapData[48] = 0.000000; ui19.fFluxCapData[49] = 0.000000; ui19.fFluxCapData[50] = 0.000000; ui19.fFluxCapData[51] = 0.000000; ui19.fFluxCapData[52] = 0.000000; ui19.fFluxCapData[53] = 0.000000; ui19.fFluxCapData[54] = 0.000000; ui19.fFluxCapData[55] = 0.000000; ui19.fFluxCapData[56] = 0.000000; ui19.fFluxCapData[57] = 0.000000; ui19.fFluxCapData[58] = 0.000000; ui19.fFluxCapData[59] = 0.000000; ui19.fFluxCapData[60] = 0.000000; ui19.fFluxCapData[61] = 0.000000; ui19.fFluxCapData[62] = 0.000000; ui19.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui19);


	m_dLFO1Amplitude = 0.000000;
	CUICtrl ui20;
	ui20.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui20.uControlId = 30;
	ui20.bLogSlider = false;
	ui20.bExpSlider = false;
	ui20.fUserDisplayDataLoLimit = 0.000000;
	ui20.fUserDisplayDataHiLimit = 1.000000;
	ui20.uUserDataType = doubleData;
	ui20.fInitUserIntValue = 0;
	ui20.fInitUserFloatValue = 0;
	ui20.fInitUserDoubleValue = 0.000000;
	ui20.fInitUserUINTValue = 0;
	ui20.m_pUserCookedIntData = NULL;
	ui20.m_pUserCookedFloatData = NULL;
	ui20.m_pUserCookedDoubleData = &m_dLFO1Amplitude;
	ui20.m_pUserCookedUINTData = NULL;
	ui20.cControlUnits = "                                                                ";
	ui20.cVariableName = "m_dLFO1Amplitude";
	ui20.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui20.dPresetData[0] = 0.000000;ui20.dPresetData[1] = 0.000000;ui20.dPresetData[2] = 0.000000;ui20.dPresetData[3] = 0.000000;ui20.dPresetData[4] = 0.000000;ui20.dPresetData[5] = 0.000000;ui20.dPresetData[6] = 0.000000;ui20.dPresetData[7] = 0.000000;ui20.dPresetData[8] = 0.000000;ui20.dPresetData[9] = 0.000000;ui20.dPresetData[10] = 0.000000;ui20.dPresetData[11] = 0.000000;ui20.dPresetData[12] = 0.000000;ui20.dPresetData[13] = 0.000000;ui20.dPresetData[14] = 0.000000;ui20.dPresetData[15] = 0.000000;
	ui20.cControlName = "LFO Depth";
	ui20.bOwnerControl = false;
	ui20.bMIDIControl = false;
	ui20.uMIDIControlCommand = 176;
	ui20.uMIDIControlName = 3;
	ui20.uMIDIControlChannel = 0;
	ui20.nGUIRow = -1;
	ui20.nGUIColumn = -1;
	ui20.uControlTheme[0] = 0; ui20.uControlTheme[1] = 0; ui20.uControlTheme[2] = 0; ui20.uControlTheme[3] = 0; ui20.uControlTheme[4] = 0; ui20.uControlTheme[5] = 0; ui20.uControlTheme[6] = 0; ui20.uControlTheme[7] = 0; ui20.uControlTheme[8] = 0; ui20.uControlTheme[9] = 0; ui20.uControlTheme[10] = 0; ui20.uControlTheme[11] = 0; ui20.uControlTheme[12] = 0; ui20.uControlTheme[13] = 0; ui20.uControlTheme[14] = 0; ui20.uControlTheme[15] = 0; ui20.uControlTheme[16] = 0; ui20.uControlTheme[17] = 0; ui20.uControlTheme[18] = 0; ui20.uControlTheme[19] = 0; ui20.uControlTheme[20] = 0; ui20.uControlTheme[21] = 0; ui20.uControlTheme[22] = 0; ui20.uControlTheme[23] = 0; ui20.uControlTheme[24] = 0; ui20.uControlTheme[25] = 0; ui20.uControlTheme[26] = 0; ui20.uControlTheme[27] = 0; ui20.uControlTheme[28] = 0; ui20.uControlTheme[29] = 0; ui20.uControlTheme[30] = 0; ui20.uControlTheme[31] = 0;
	ui20.uFluxCapControl[0] = 0; ui20.uFluxCapControl[1] = 0; ui20.uFluxCapControl[2] = 0; ui20.uFluxCapControl[3] = 0; ui20.uFluxCapControl[4] = 0; ui20.uFluxCapControl[5] = 0; ui20.uFluxCapControl[6] = 0; ui20.uFluxCapControl[7] = 0; ui20.uFluxCapControl[8] = 0; ui20.uFluxCapControl[9] = 0; ui20.uFluxCapControl[10] = 0; ui20.uFluxCapControl[11] = 0; ui20.uFluxCapControl[12] = 0; ui20.uFluxCapControl[13] = 0; ui20.uFluxCapControl[14] = 0; ui20.uFluxCapControl[15] = 0; ui20.uFluxCapControl[16] = 0; ui20.uFluxCapControl[17] = 0; ui20.uFluxCapControl[18] = 0; ui20.uFluxCapControl[19] = 0; ui20.uFluxCapControl[20] = 0; ui20.uFluxCapControl[21] = 0; ui20.uFluxCapControl[22] = 0; ui20.uFluxCapControl[23] = 0; ui20.uFluxCapControl[24] = 0; ui20.uFluxCapControl[25] = 0; ui20.uFluxCapControl[26] = 0; ui20.uFluxCapControl[27] = 0; ui20.uFluxCapControl[28] = 0; ui20.uFluxCapControl[29] = 0; ui20.uFluxCapControl[30] = 0; ui20.uFluxCapControl[31] = 0; ui20.uFluxCapControl[32] = 0; ui20.uFluxCapControl[33] = 0; ui20.uFluxCapControl[34] = 0; ui20.uFluxCapControl[35] = 0; ui20.uFluxCapControl[36] = 0; ui20.uFluxCapControl[37] = 0; ui20.uFluxCapControl[38] = 0; ui20.uFluxCapControl[39] = 0; ui20.uFluxCapControl[40] = 0; ui20.uFluxCapControl[41] = 0; ui20.uFluxCapControl[42] = 0; ui20.uFluxCapControl[43] = 0; ui20.uFluxCapControl[44] = 0; ui20.uFluxCapControl[45] = 0; ui20.uFluxCapControl[46] = 0; ui20.uFluxCapControl[47] = 0; ui20.uFluxCapControl[48] = 0; ui20.uFluxCapControl[49] = 0; ui20.uFluxCapControl[50] = 0; ui20.uFluxCapControl[51] = 0; ui20.uFluxCapControl[52] = 0; ui20.uFluxCapControl[53] = 0; ui20.uFluxCapControl[54] = 0; ui20.uFluxCapControl[55] = 0; ui20.uFluxCapControl[56] = 0; ui20.uFluxCapControl[57] = 0; ui20.uFluxCapControl[58] = 0; ui20.uFluxCapControl[59] = 0; ui20.uFluxCapControl[60] = 0; ui20.uFluxCapControl[61] = 0; ui20.uFluxCapControl[62] = 0; ui20.uFluxCapControl[63] = 0;
	ui20.fFluxCapData[0] = 0.000000; ui20.fFluxCapData[1] = 0.000000; ui20.fFluxCapData[2] = 0.000000; ui20.fFluxCapData[3] = 0.000000; ui20.fFluxCapData[4] = 0.000000; ui20.fFluxCapData[5] = 0.000000; ui20.fFluxCapData[6] = 0.000000; ui20.fFluxCapData[7] = 0.000000; ui20.fFluxCapData[8] = 0.000000; ui20.fFluxCapData[9] = 0.000000; ui20.fFluxCapData[10] = 0.000000; ui20.fFluxCapData[11] = 0.000000; ui20.fFluxCapData[12] = 0.000000; ui20.fFluxCapData[13] = 0.000000; ui20.fFluxCapData[14] = 0.000000; ui20.fFluxCapData[15] = 0.000000; ui20.fFluxCapData[16] = 0.000000; ui20.fFluxCapData[17] = 0.000000; ui20.fFluxCapData[18] = 0.000000; ui20.fFluxCapData[19] = 0.000000; ui20.fFluxCapData[20] = 0.000000; ui20.fFluxCapData[21] = 0.000000; ui20.fFluxCapData[22] = 0.000000; ui20.fFluxCapData[23] = 0.000000; ui20.fFluxCapData[24] = 0.000000; ui20.fFluxCapData[25] = 0.000000; ui20.fFluxCapData[26] = 0.000000; ui20.fFluxCapData[27] = 0.000000; ui20.fFluxCapData[28] = 0.000000; ui20.fFluxCapData[29] = 0.000000; ui20.fFluxCapData[30] = 0.000000; ui20.fFluxCapData[31] = 0.000000; ui20.fFluxCapData[32] = 0.000000; ui20.fFluxCapData[33] = 0.000000; ui20.fFluxCapData[34] = 0.000000; ui20.fFluxCapData[35] = 0.000000; ui20.fFluxCapData[36] = 0.000000; ui20.fFluxCapData[37] = 0.000000; ui20.fFluxCapData[38] = 0.000000; ui20.fFluxCapData[39] = 0.000000; ui20.fFluxCapData[40] = 0.000000; ui20.fFluxCapData[41] = 0.000000; ui20.fFluxCapData[42] = 0.000000; ui20.fFluxCapData[43] = 0.000000; ui20.fFluxCapData[44] = 0.000000; ui20.fFluxCapData[45] = 0.000000; ui20.fFluxCapData[46] = 0.000000; ui20.fFluxCapData[47] = 0.000000; ui20.fFluxCapData[48] = 0.000000; ui20.fFluxCapData[49] = 0.000000; ui20.fFluxCapData[50] = 0.000000; ui20.fFluxCapData[51] = 0.000000; ui20.fFluxCapData[52] = 0.000000; ui20.fFluxCapData[53] = 0.000000; ui20.fFluxCapData[54] = 0.000000; ui20.fFluxCapData[55] = 0.000000; ui20.fFluxCapData[56] = 0.000000; ui20.fFluxCapData[57] = 0.000000; ui20.fFluxCapData[58] = 0.000000; ui20.fFluxCapData[59] = 0.000000; ui20.fFluxCapData[60] = 0.000000; ui20.fFluxCapData[61] = 0.000000; ui20.fFluxCapData[62] = 0.000000; ui20.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui20);


	m_dLFO1AmpIntensity = 0.000000;
	CUICtrl ui21;
	ui21.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui21.uControlId = 31;
	ui21.bLogSlider = false;
	ui21.bExpSlider = false;
	ui21.fUserDisplayDataLoLimit = 0.000000;
	ui21.fUserDisplayDataHiLimit = 1.000000;
	ui21.uUserDataType = doubleData;
	ui21.fInitUserIntValue = 0;
	ui21.fInitUserFloatValue = 0;
	ui21.fInitUserDoubleValue = 0.000000;
	ui21.fInitUserUINTValue = 0;
	ui21.m_pUserCookedIntData = NULL;
	ui21.m_pUserCookedFloatData = NULL;
	ui21.m_pUserCookedDoubleData = &m_dLFO1AmpIntensity;
	ui21.m_pUserCookedUINTData = NULL;
	ui21.cControlUnits = "                                                                ";
	ui21.cVariableName = "m_dLFO1AmpIntensity";
	ui21.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui21.dPresetData[0] = 0.000000;ui21.dPresetData[1] = 0.000000;ui21.dPresetData[2] = 0.000000;ui21.dPresetData[3] = 0.000000;ui21.dPresetData[4] = 0.000000;ui21.dPresetData[5] = 0.000000;ui21.dPresetData[6] = 0.000000;ui21.dPresetData[7] = 0.000000;ui21.dPresetData[8] = 0.000000;ui21.dPresetData[9] = 0.000000;ui21.dPresetData[10] = 0.000000;ui21.dPresetData[11] = 0.000000;ui21.dPresetData[12] = 0.000000;ui21.dPresetData[13] = 0.000000;ui21.dPresetData[14] = 0.000000;ui21.dPresetData[15] = 0.000000;
	ui21.cControlName = "LFO Amp Int";
	ui21.bOwnerControl = false;
	ui21.bMIDIControl = false;
	ui21.uMIDIControlCommand = 176;
	ui21.uMIDIControlName = 3;
	ui21.uMIDIControlChannel = 0;
	ui21.nGUIRow = -1;
	ui21.nGUIColumn = -1;
	ui21.uControlTheme[0] = 0; ui21.uControlTheme[1] = 0; ui21.uControlTheme[2] = 0; ui21.uControlTheme[3] = 0; ui21.uControlTheme[4] = 0; ui21.uControlTheme[5] = 0; ui21.uControlTheme[6] = 0; ui21.uControlTheme[7] = 0; ui21.uControlTheme[8] = 0; ui21.uControlTheme[9] = 0; ui21.uControlTheme[10] = 0; ui21.uControlTheme[11] = 0; ui21.uControlTheme[12] = 0; ui21.uControlTheme[13] = 0; ui21.uControlTheme[14] = 0; ui21.uControlTheme[15] = 0; ui21.uControlTheme[16] = 0; ui21.uControlTheme[17] = 0; ui21.uControlTheme[18] = 0; ui21.uControlTheme[19] = 0; ui21.uControlTheme[20] = 0; ui21.uControlTheme[21] = 0; ui21.uControlTheme[22] = 0; ui21.uControlTheme[23] = 0; ui21.uControlTheme[24] = 0; ui21.uControlTheme[25] = 0; ui21.uControlTheme[26] = 0; ui21.uControlTheme[27] = 0; ui21.uControlTheme[28] = 0; ui21.uControlTheme[29] = 0; ui21.uControlTheme[30] = 0; ui21.uControlTheme[31] = 0;
	ui21.uFluxCapControl[0] = 0; ui21.uFluxCapControl[1] = 0; ui21.uFluxCapControl[2] = 0; ui21.uFluxCapControl[3] = 0; ui21.uFluxCapControl[4] = 0; ui21.uFluxCapControl[5] = 0; ui21.uFluxCapControl[6] = 0; ui21.uFluxCapControl[7] = 0; ui21.uFluxCapControl[8] = 0; ui21.uFluxCapControl[9] = 0; ui21.uFluxCapControl[10] = 0; ui21.uFluxCapControl[11] = 0; ui21.uFluxCapControl[12] = 0; ui21.uFluxCapControl[13] = 0; ui21.uFluxCapControl[14] = 0; ui21.uFluxCapControl[15] = 0; ui21.uFluxCapControl[16] = 0; ui21.uFluxCapControl[17] = 0; ui21.uFluxCapControl[18] = 0; ui21.uFluxCapControl[19] = 0; ui21.uFluxCapControl[20] = 0; ui21.uFluxCapControl[21] = 0; ui21.uFluxCapControl[22] = 0; ui21.uFluxCapControl[23] = 0; ui21.uFluxCapControl[24] = 0; ui21.uFluxCapControl[25] = 0; ui21.uFluxCapControl[26] = 0; ui21.uFluxCapControl[27] = 0; ui21.uFluxCapControl[28] = 0; ui21.uFluxCapControl[29] = 0; ui21.uFluxCapControl[30] = 0; ui21.uFluxCapControl[31] = 0; ui21.uFluxCapControl[32] = 0; ui21.uFluxCapControl[33] = 0; ui21.uFluxCapControl[34] = 0; ui21.uFluxCapControl[35] = 0; ui21.uFluxCapControl[36] = 0; ui21.uFluxCapControl[37] = 0; ui21.uFluxCapControl[38] = 0; ui21.uFluxCapControl[39] = 0; ui21.uFluxCapControl[40] = 0; ui21.uFluxCapControl[41] = 0; ui21.uFluxCapControl[42] = 0; ui21.uFluxCapControl[43] = 0; ui21.uFluxCapControl[44] = 0; ui21.uFluxCapControl[45] = 0; ui21.uFluxCapControl[46] = 0; ui21.uFluxCapControl[47] = 0; ui21.uFluxCapControl[48] = 0; ui21.uFluxCapControl[49] = 0; ui21.uFluxCapControl[50] = 0; ui21.uFluxCapControl[51] = 0; ui21.uFluxCapControl[52] = 0; ui21.uFluxCapControl[53] = 0; ui21.uFluxCapControl[54] = 0; ui21.uFluxCapControl[55] = 0; ui21.uFluxCapControl[56] = 0; ui21.uFluxCapControl[57] = 0; ui21.uFluxCapControl[58] = 0; ui21.uFluxCapControl[59] = 0; ui21.uFluxCapControl[60] = 0; ui21.uFluxCapControl[61] = 0; ui21.uFluxCapControl[62] = 0; ui21.uFluxCapControl[63] = 0;
	ui21.fFluxCapData[0] = 0.000000; ui21.fFluxCapData[1] = 0.000000; ui21.fFluxCapData[2] = 0.000000; ui21.fFluxCapData[3] = 0.000000; ui21.fFluxCapData[4] = 0.000000; ui21.fFluxCapData[5] = 0.000000; ui21.fFluxCapData[6] = 0.000000; ui21.fFluxCapData[7] = 0.000000; ui21.fFluxCapData[8] = 0.000000; ui21.fFluxCapData[9] = 0.000000; ui21.fFluxCapData[10] = 0.000000; ui21.fFluxCapData[11] = 0.000000; ui21.fFluxCapData[12] = 0.000000; ui21.fFluxCapData[13] = 0.000000; ui21.fFluxCapData[14] = 0.000000; ui21.fFluxCapData[15] = 0.000000; ui21.fFluxCapData[16] = 0.000000; ui21.fFluxCapData[17] = 0.000000; ui21.fFluxCapData[18] = 0.000000; ui21.fFluxCapData[19] = 0.000000; ui21.fFluxCapData[20] = 0.000000; ui21.fFluxCapData[21] = 0.000000; ui21.fFluxCapData[22] = 0.000000; ui21.fFluxCapData[23] = 0.000000; ui21.fFluxCapData[24] = 0.000000; ui21.fFluxCapData[25] = 0.000000; ui21.fFluxCapData[26] = 0.000000; ui21.fFluxCapData[27] = 0.000000; ui21.fFluxCapData[28] = 0.000000; ui21.fFluxCapData[29] = 0.000000; ui21.fFluxCapData[30] = 0.000000; ui21.fFluxCapData[31] = 0.000000; ui21.fFluxCapData[32] = 0.000000; ui21.fFluxCapData[33] = 0.000000; ui21.fFluxCapData[34] = 0.000000; ui21.fFluxCapData[35] = 0.000000; ui21.fFluxCapData[36] = 0.000000; ui21.fFluxCapData[37] = 0.000000; ui21.fFluxCapData[38] = 0.000000; ui21.fFluxCapData[39] = 0.000000; ui21.fFluxCapData[40] = 0.000000; ui21.fFluxCapData[41] = 0.000000; ui21.fFluxCapData[42] = 0.000000; ui21.fFluxCapData[43] = 0.000000; ui21.fFluxCapData[44] = 0.000000; ui21.fFluxCapData[45] = 0.000000; ui21.fFluxCapData[46] = 0.000000; ui21.fFluxCapData[47] = 0.000000; ui21.fFluxCapData[48] = 0.000000; ui21.fFluxCapData[49] = 0.000000; ui21.fFluxCapData[50] = 0.000000; ui21.fFluxCapData[51] = 0.000000; ui21.fFluxCapData[52] = 0.000000; ui21.fFluxCapData[53] = 0.000000; ui21.fFluxCapData[54] = 0.000000; ui21.fFluxCapData[55] = 0.000000; ui21.fFluxCapData[56] = 0.000000; ui21.fFluxCapData[57] = 0.000000; ui21.fFluxCapData[58] = 0.000000; ui21.fFluxCapData[59] = 0.000000; ui21.fFluxCapData[60] = 0.000000; ui21.fFluxCapData[61] = 0.000000; ui21.fFluxCapData[62] = 0.000000; ui21.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui21);


	m_dLFO1PanIntensity = 0.000000;
	CUICtrl ui22;
	ui22.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui22.uControlId = 32;
	ui22.bLogSlider = false;
	ui22.bExpSlider = false;
	ui22.fUserDisplayDataLoLimit = 0.000000;
	ui22.fUserDisplayDataHiLimit = 1.000000;
	ui22.uUserDataType = doubleData;
	ui22.fInitUserIntValue = 0;
	ui22.fInitUserFloatValue = 0;
	ui22.fInitUserDoubleValue = 0.000000;
	ui22.fInitUserUINTValue = 0;
	ui22.m_pUserCookedIntData = NULL;
	ui22.m_pUserCookedFloatData = NULL;
	ui22.m_pUserCookedDoubleData = &m_dLFO1PanIntensity;
	ui22.m_pUserCookedUINTData = NULL;
	ui22.cControlUnits = "                                                                ";
	ui22.cVariableName = "m_dLFO1PanIntensity";
	ui22.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui22.dPresetData[0] = 0.000000;ui22.dPresetData[1] = 0.000000;ui22.dPresetData[2] = 0.000000;ui22.dPresetData[3] = 0.000000;ui22.dPresetData[4] = 0.000000;ui22.dPresetData[5] = 0.000000;ui22.dPresetData[6] = 0.000000;ui22.dPresetData[7] = 0.000000;ui22.dPresetData[8] = 0.000000;ui22.dPresetData[9] = 0.000000;ui22.dPresetData[10] = 0.000000;ui22.dPresetData[11] = 0.000000;ui22.dPresetData[12] = 0.000000;ui22.dPresetData[13] = 0.000000;ui22.dPresetData[14] = 0.000000;ui22.dPresetData[15] = 0.000000;
	ui22.cControlName = "LFO Pan Int";
	ui22.bOwnerControl = false;
	ui22.bMIDIControl = false;
	ui22.uMIDIControlCommand = 176;
	ui22.uMIDIControlName = 3;
	ui22.uMIDIControlChannel = 0;
	ui22.nGUIRow = -1;
	ui22.nGUIColumn = -1;
	ui22.uControlTheme[0] = 0; ui22.uControlTheme[1] = 0; ui22.uControlTheme[2] = 0; ui22.uControlTheme[3] = 0; ui22.uControlTheme[4] = 0; ui22.uControlTheme[5] = 0; ui22.uControlTheme[6] = 0; ui22.uControlTheme[7] = 0; ui22.uControlTheme[8] = 0; ui22.uControlTheme[9] = 0; ui22.uControlTheme[10] = 0; ui22.uControlTheme[11] = 0; ui22.uControlTheme[12] = 0; ui22.uControlTheme[13] = 0; ui22.uControlTheme[14] = 0; ui22.uControlTheme[15] = 0; ui22.uControlTheme[16] = 0; ui22.uControlTheme[17] = 0; ui22.uControlTheme[18] = 0; ui22.uControlTheme[19] = 0; ui22.uControlTheme[20] = 0; ui22.uControlTheme[21] = 0; ui22.uControlTheme[22] = 0; ui22.uControlTheme[23] = 0; ui22.uControlTheme[24] = 0; ui22.uControlTheme[25] = 0; ui22.uControlTheme[26] = 0; ui22.uControlTheme[27] = 0; ui22.uControlTheme[28] = 0; ui22.uControlTheme[29] = 0; ui22.uControlTheme[30] = 0; ui22.uControlTheme[31] = 0;
	ui22.uFluxCapControl[0] = 0; ui22.uFluxCapControl[1] = 0; ui22.uFluxCapControl[2] = 0; ui22.uFluxCapControl[3] = 0; ui22.uFluxCapControl[4] = 0; ui22.uFluxCapControl[5] = 0; ui22.uFluxCapControl[6] = 0; ui22.uFluxCapControl[7] = 0; ui22.uFluxCapControl[8] = 0; ui22.uFluxCapControl[9] = 0; ui22.uFluxCapControl[10] = 0; ui22.uFluxCapControl[11] = 0; ui22.uFluxCapControl[12] = 0; ui22.uFluxCapControl[13] = 0; ui22.uFluxCapControl[14] = 0; ui22.uFluxCapControl[15] = 0; ui22.uFluxCapControl[16] = 0; ui22.uFluxCapControl[17] = 0; ui22.uFluxCapControl[18] = 0; ui22.uFluxCapControl[19] = 0; ui22.uFluxCapControl[20] = 0; ui22.uFluxCapControl[21] = 0; ui22.uFluxCapControl[22] = 0; ui22.uFluxCapControl[23] = 0; ui22.uFluxCapControl[24] = 0; ui22.uFluxCapControl[25] = 0; ui22.uFluxCapControl[26] = 0; ui22.uFluxCapControl[27] = 0; ui22.uFluxCapControl[28] = 0; ui22.uFluxCapControl[29] = 0; ui22.uFluxCapControl[30] = 0; ui22.uFluxCapControl[31] = 0; ui22.uFluxCapControl[32] = 0; ui22.uFluxCapControl[33] = 0; ui22.uFluxCapControl[34] = 0; ui22.uFluxCapControl[35] = 0; ui22.uFluxCapControl[36] = 0; ui22.uFluxCapControl[37] = 0; ui22.uFluxCapControl[38] = 0; ui22.uFluxCapControl[39] = 0; ui22.uFluxCapControl[40] = 0; ui22.uFluxCapControl[41] = 0; ui22.uFluxCapControl[42] = 0; ui22.uFluxCapControl[43] = 0; ui22.uFluxCapControl[44] = 0; ui22.uFluxCapControl[45] = 0; ui22.uFluxCapControl[46] = 0; ui22.uFluxCapControl[47] = 0; ui22.uFluxCapControl[48] = 0; ui22.uFluxCapControl[49] = 0; ui22.uFluxCapControl[50] = 0; ui22.uFluxCapControl[51] = 0; ui22.uFluxCapControl[52] = 0; ui22.uFluxCapControl[53] = 0; ui22.uFluxCapControl[54] = 0; ui22.uFluxCapControl[55] = 0; ui22.uFluxCapControl[56] = 0; ui22.uFluxCapControl[57] = 0; ui22.uFluxCapControl[58] = 0; ui22.uFluxCapControl[59] = 0; ui22.uFluxCapControl[60] = 0; ui22.uFluxCapControl[61] = 0; ui22.uFluxCapControl[62] = 0; ui22.uFluxCapControl[63] = 0;
	ui22.fFluxCapData[0] = 0.000000; ui22.fFluxCapData[1] = 0.000000; ui22.fFluxCapData[2] = 0.000000; ui22.fFluxCapData[3] = 0.000000; ui22.fFluxCapData[4] = 0.000000; ui22.fFluxCapData[5] = 0.000000; ui22.fFluxCapData[6] = 0.000000; ui22.fFluxCapData[7] = 0.000000; ui22.fFluxCapData[8] = 0.000000; ui22.fFluxCapData[9] = 0.000000; ui22.fFluxCapData[10] = 0.000000; ui22.fFluxCapData[11] = 0.000000; ui22.fFluxCapData[12] = 0.000000; ui22.fFluxCapData[13] = 0.000000; ui22.fFluxCapData[14] = 0.000000; ui22.fFluxCapData[15] = 0.000000; ui22.fFluxCapData[16] = 0.000000; ui22.fFluxCapData[17] = 0.000000; ui22.fFluxCapData[18] = 0.000000; ui22.fFluxCapData[19] = 0.000000; ui22.fFluxCapData[20] = 0.000000; ui22.fFluxCapData[21] = 0.000000; ui22.fFluxCapData[22] = 0.000000; ui22.fFluxCapData[23] = 0.000000; ui22.fFluxCapData[24] = 0.000000; ui22.fFluxCapData[25] = 0.000000; ui22.fFluxCapData[26] = 0.000000; ui22.fFluxCapData[27] = 0.000000; ui22.fFluxCapData[28] = 0.000000; ui22.fFluxCapData[29] = 0.000000; ui22.fFluxCapData[30] = 0.000000; ui22.fFluxCapData[31] = 0.000000; ui22.fFluxCapData[32] = 0.000000; ui22.fFluxCapData[33] = 0.000000; ui22.fFluxCapData[34] = 0.000000; ui22.fFluxCapData[35] = 0.000000; ui22.fFluxCapData[36] = 0.000000; ui22.fFluxCapData[37] = 0.000000; ui22.fFluxCapData[38] = 0.000000; ui22.fFluxCapData[39] = 0.000000; ui22.fFluxCapData[40] = 0.000000; ui22.fFluxCapData[41] = 0.000000; ui22.fFluxCapData[42] = 0.000000; ui22.fFluxCapData[43] = 0.000000; ui22.fFluxCapData[44] = 0.000000; ui22.fFluxCapData[45] = 0.000000; ui22.fFluxCapData[46] = 0.000000; ui22.fFluxCapData[47] = 0.000000; ui22.fFluxCapData[48] = 0.000000; ui22.fFluxCapData[49] = 0.000000; ui22.fFluxCapData[50] = 0.000000; ui22.fFluxCapData[51] = 0.000000; ui22.fFluxCapData[52] = 0.000000; ui22.fFluxCapData[53] = 0.000000; ui22.fFluxCapData[54] = 0.000000; ui22.fFluxCapData[55] = 0.000000; ui22.fFluxCapData[56] = 0.000000; ui22.fFluxCapData[57] = 0.000000; ui22.fFluxCapData[58] = 0.000000; ui22.fFluxCapData[59] = 0.000000; ui22.fFluxCapData[60] = 0.000000; ui22.fFluxCapData[61] = 0.000000; ui22.fFluxCapData[62] = 0.000000; ui22.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui22);


	m_uLFO1Waveform = 0;
	CUICtrl ui23;
	ui23.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui23.uControlId = 33;
	ui23.bLogSlider = false;
	ui23.bExpSlider = false;
	ui23.fUserDisplayDataLoLimit = 0.000000;
	ui23.fUserDisplayDataHiLimit = 7.000000;
	ui23.uUserDataType = UINTData;
	ui23.fInitUserIntValue = 0;
	ui23.fInitUserFloatValue = 0;
	ui23.fInitUserDoubleValue = 0;
	ui23.fInitUserUINTValue = 0.000000;
	ui23.m_pUserCookedIntData = NULL;
	ui23.m_pUserCookedFloatData = NULL;
	ui23.m_pUserCookedDoubleData = NULL;
	ui23.m_pUserCookedUINTData = &m_uLFO1Waveform;
	ui23.cControlUnits = "                                                                ";
	ui23.cVariableName = "m_uLFO1Waveform";
	ui23.cEnumeratedList = "sine,usaw,dsaw,tri,square,expo,rsh,qrsh";
	ui23.dPresetData[0] = 0.000000;ui23.dPresetData[1] = 0.000000;ui23.dPresetData[2] = 0.000000;ui23.dPresetData[3] = 0.000000;ui23.dPresetData[4] = 0.000000;ui23.dPresetData[5] = 0.000000;ui23.dPresetData[6] = 0.000000;ui23.dPresetData[7] = 0.000000;ui23.dPresetData[8] = 0.000000;ui23.dPresetData[9] = 0.000000;ui23.dPresetData[10] = 0.000000;ui23.dPresetData[11] = 0.000000;ui23.dPresetData[12] = 0.000000;ui23.dPresetData[13] = 0.000000;ui23.dPresetData[14] = 0.000000;ui23.dPresetData[15] = 0.000000;
	ui23.cControlName = "LFO Waveform";
	ui23.bOwnerControl = false;
	ui23.bMIDIControl = false;
	ui23.uMIDIControlCommand = 176;
	ui23.uMIDIControlName = 3;
	ui23.uMIDIControlChannel = 0;
	ui23.nGUIRow = -1;
	ui23.nGUIColumn = -1;
	ui23.uControlTheme[0] = 0; ui23.uControlTheme[1] = 0; ui23.uControlTheme[2] = 0; ui23.uControlTheme[3] = 0; ui23.uControlTheme[4] = 0; ui23.uControlTheme[5] = 0; ui23.uControlTheme[6] = 0; ui23.uControlTheme[7] = 0; ui23.uControlTheme[8] = 0; ui23.uControlTheme[9] = 0; ui23.uControlTheme[10] = 0; ui23.uControlTheme[11] = 0; ui23.uControlTheme[12] = 0; ui23.uControlTheme[13] = 0; ui23.uControlTheme[14] = 0; ui23.uControlTheme[15] = 0; ui23.uControlTheme[16] = 0; ui23.uControlTheme[17] = 0; ui23.uControlTheme[18] = 0; ui23.uControlTheme[19] = 0; ui23.uControlTheme[20] = 0; ui23.uControlTheme[21] = 0; ui23.uControlTheme[22] = 0; ui23.uControlTheme[23] = 0; ui23.uControlTheme[24] = 0; ui23.uControlTheme[25] = 0; ui23.uControlTheme[26] = 0; ui23.uControlTheme[27] = 0; ui23.uControlTheme[28] = 0; ui23.uControlTheme[29] = 0; ui23.uControlTheme[30] = 0; ui23.uControlTheme[31] = 0;
	ui23.uFluxCapControl[0] = 0; ui23.uFluxCapControl[1] = 0; ui23.uFluxCapControl[2] = 0; ui23.uFluxCapControl[3] = 0; ui23.uFluxCapControl[4] = 0; ui23.uFluxCapControl[5] = 0; ui23.uFluxCapControl[6] = 0; ui23.uFluxCapControl[7] = 0; ui23.uFluxCapControl[8] = 0; ui23.uFluxCapControl[9] = 0; ui23.uFluxCapControl[10] = 0; ui23.uFluxCapControl[11] = 0; ui23.uFluxCapControl[12] = 0; ui23.uFluxCapControl[13] = 0; ui23.uFluxCapControl[14] = 0; ui23.uFluxCapControl[15] = 0; ui23.uFluxCapControl[16] = 0; ui23.uFluxCapControl[17] = 0; ui23.uFluxCapControl[18] = 0; ui23.uFluxCapControl[19] = 0; ui23.uFluxCapControl[20] = 0; ui23.uFluxCapControl[21] = 0; ui23.uFluxCapControl[22] = 0; ui23.uFluxCapControl[23] = 0; ui23.uFluxCapControl[24] = 0; ui23.uFluxCapControl[25] = 0; ui23.uFluxCapControl[26] = 0; ui23.uFluxCapControl[27] = 0; ui23.uFluxCapControl[28] = 0; ui23.uFluxCapControl[29] = 0; ui23.uFluxCapControl[30] = 0; ui23.uFluxCapControl[31] = 0; ui23.uFluxCapControl[32] = 0; ui23.uFluxCapControl[33] = 0; ui23.uFluxCapControl[34] = 0; ui23.uFluxCapControl[35] = 0; ui23.uFluxCapControl[36] = 0; ui23.uFluxCapControl[37] = 0; ui23.uFluxCapControl[38] = 0; ui23.uFluxCapControl[39] = 0; ui23.uFluxCapControl[40] = 0; ui23.uFluxCapControl[41] = 0; ui23.uFluxCapControl[42] = 0; ui23.uFluxCapControl[43] = 0; ui23.uFluxCapControl[44] = 0; ui23.uFluxCapControl[45] = 0; ui23.uFluxCapControl[46] = 0; ui23.uFluxCapControl[47] = 0; ui23.uFluxCapControl[48] = 0; ui23.uFluxCapControl[49] = 0; ui23.uFluxCapControl[50] = 0; ui23.uFluxCapControl[51] = 0; ui23.uFluxCapControl[52] = 0; ui23.uFluxCapControl[53] = 0; ui23.uFluxCapControl[54] = 0; ui23.uFluxCapControl[55] = 0; ui23.uFluxCapControl[56] = 0; ui23.uFluxCapControl[57] = 0; ui23.uFluxCapControl[58] = 0; ui23.uFluxCapControl[59] = 0; ui23.uFluxCapControl[60] = 0; ui23.uFluxCapControl[61] = 0; ui23.uFluxCapControl[62] = 0; ui23.uFluxCapControl[63] = 0;
	ui23.fFluxCapData[0] = 0.000000; ui23.fFluxCapData[1] = 0.000000; ui23.fFluxCapData[2] = 0.000000; ui23.fFluxCapData[3] = 0.000000; ui23.fFluxCapData[4] = 0.000000; ui23.fFluxCapData[5] = 0.000000; ui23.fFluxCapData[6] = 0.000000; ui23.fFluxCapData[7] = 0.000000; ui23.fFluxCapData[8] = 0.000000; ui23.fFluxCapData[9] = 0.000000; ui23.fFluxCapData[10] = 0.000000; ui23.fFluxCapData[11] = 0.000000; ui23.fFluxCapData[12] = 0.000000; ui23.fFluxCapData[13] = 0.000000; ui23.fFluxCapData[14] = 0.000000; ui23.fFluxCapData[15] = 0.000000; ui23.fFluxCapData[16] = 0.000000; ui23.fFluxCapData[17] = 0.000000; ui23.fFluxCapData[18] = 0.000000; ui23.fFluxCapData[19] = 0.000000; ui23.fFluxCapData[20] = 0.000000; ui23.fFluxCapData[21] = 0.000000; ui23.fFluxCapData[22] = 0.000000; ui23.fFluxCapData[23] = 0.000000; ui23.fFluxCapData[24] = 0.000000; ui23.fFluxCapData[25] = 0.000000; ui23.fFluxCapData[26] = 0.000000; ui23.fFluxCapData[27] = 0.000000; ui23.fFluxCapData[28] = 0.000000; ui23.fFluxCapData[29] = 0.000000; ui23.fFluxCapData[30] = 0.000000; ui23.fFluxCapData[31] = 0.000000; ui23.fFluxCapData[32] = 0.000000; ui23.fFluxCapData[33] = 0.000000; ui23.fFluxCapData[34] = 0.000000; ui23.fFluxCapData[35] = 0.000000; ui23.fFluxCapData[36] = 0.000000; ui23.fFluxCapData[37] = 0.000000; ui23.fFluxCapData[38] = 0.000000; ui23.fFluxCapData[39] = 0.000000; ui23.fFluxCapData[40] = 0.000000; ui23.fFluxCapData[41] = 0.000000; ui23.fFluxCapData[42] = 0.000000; ui23.fFluxCapData[43] = 0.000000; ui23.fFluxCapData[44] = 0.000000; ui23.fFluxCapData[45] = 0.000000; ui23.fFluxCapData[46] = 0.000000; ui23.fFluxCapData[47] = 0.000000; ui23.fFluxCapData[48] = 0.000000; ui23.fFluxCapData[49] = 0.000000; ui23.fFluxCapData[50] = 0.000000; ui23.fFluxCapData[51] = 0.000000; ui23.fFluxCapData[52] = 0.000000; ui23.fFluxCapData[53] = 0.000000; ui23.fFluxCapData[54] = 0.000000; ui23.fFluxCapData[55] = 0.000000; ui23.fFluxCapData[56] = 0.000000; ui23.fFluxCapData[57] = 0.000000; ui23.fFluxCapData[58] = 0.000000; ui23.fFluxCapData[59] = 0.000000; ui23.fFluxCapData[60] = 0.000000; ui23.fFluxCapData[61] = 0.000000; ui23.fFluxCapData[62] = 0.000000; ui23.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui23);


	m_dEG1DCAIntensity = 1.000000;
	CUICtrl ui24;
	ui24.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui24.uControlId = 34;
	ui24.bLogSlider = false;
	ui24.bExpSlider = false;
	ui24.fUserDisplayDataLoLimit = -1.000000;
	ui24.fUserDisplayDataHiLimit = 1.000000;
	ui24.uUserDataType = doubleData;
	ui24.fInitUserIntValue = 0;
	ui24.fInitUserFloatValue = 0;
	ui24.fInitUserDoubleValue = 1.000000;
	ui24.fInitUserUINTValue = 0;
	ui24.m_pUserCookedIntData = NULL;
	ui24.m_pUserCookedFloatData = NULL;
	ui24.m_pUserCookedDoubleData = &m_dEG1DCAIntensity;
	ui24.m_pUserCookedUINTData = NULL;
	ui24.cControlUnits = "                                                                ";
	ui24.cVariableName = "m_dEG1DCAIntensity";
	ui24.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui24.dPresetData[0] = 1.000000;ui24.dPresetData[1] = 0.000000;ui24.dPresetData[2] = 0.000000;ui24.dPresetData[3] = 0.000000;ui24.dPresetData[4] = 0.000000;ui24.dPresetData[5] = 0.000000;ui24.dPresetData[6] = 0.000000;ui24.dPresetData[7] = 0.000000;ui24.dPresetData[8] = 0.000000;ui24.dPresetData[9] = 0.000000;ui24.dPresetData[10] = 0.000000;ui24.dPresetData[11] = 0.000000;ui24.dPresetData[12] = 0.000000;ui24.dPresetData[13] = 0.000000;ui24.dPresetData[14] = 0.000000;ui24.dPresetData[15] = 0.000000;
	ui24.cControlName = "DCA EG Int";
	ui24.bOwnerControl = false;
	ui24.bMIDIControl = false;
	ui24.uMIDIControlCommand = 176;
	ui24.uMIDIControlName = 3;
	ui24.uMIDIControlChannel = 0;
	ui24.nGUIRow = -1;
	ui24.nGUIColumn = -1;
	ui24.uControlTheme[0] = 0; ui24.uControlTheme[1] = 0; ui24.uControlTheme[2] = 0; ui24.uControlTheme[3] = 0; ui24.uControlTheme[4] = 0; ui24.uControlTheme[5] = 0; ui24.uControlTheme[6] = 0; ui24.uControlTheme[7] = 0; ui24.uControlTheme[8] = 0; ui24.uControlTheme[9] = 0; ui24.uControlTheme[10] = 0; ui24.uControlTheme[11] = 0; ui24.uControlTheme[12] = 0; ui24.uControlTheme[13] = 0; ui24.uControlTheme[14] = 0; ui24.uControlTheme[15] = 0; ui24.uControlTheme[16] = 0; ui24.uControlTheme[17] = 0; ui24.uControlTheme[18] = 0; ui24.uControlTheme[19] = 0; ui24.uControlTheme[20] = 0; ui24.uControlTheme[21] = 0; ui24.uControlTheme[22] = 0; ui24.uControlTheme[23] = 0; ui24.uControlTheme[24] = 0; ui24.uControlTheme[25] = 0; ui24.uControlTheme[26] = 0; ui24.uControlTheme[27] = 0; ui24.uControlTheme[28] = 0; ui24.uControlTheme[29] = 0; ui24.uControlTheme[30] = 0; ui24.uControlTheme[31] = 0;
	ui24.uFluxCapControl[0] = 0; ui24.uFluxCapControl[1] = 0; ui24.uFluxCapControl[2] = 0; ui24.uFluxCapControl[3] = 0; ui24.uFluxCapControl[4] = 0; ui24.uFluxCapControl[5] = 0; ui24.uFluxCapControl[6] = 0; ui24.uFluxCapControl[7] = 0; ui24.uFluxCapControl[8] = 0; ui24.uFluxCapControl[9] = 0; ui24.uFluxCapControl[10] = 0; ui24.uFluxCapControl[11] = 0; ui24.uFluxCapControl[12] = 0; ui24.uFluxCapControl[13] = 0; ui24.uFluxCapControl[14] = 0; ui24.uFluxCapControl[15] = 0; ui24.uFluxCapControl[16] = 0; ui24.uFluxCapControl[17] = 0; ui24.uFluxCapControl[18] = 0; ui24.uFluxCapControl[19] = 0; ui24.uFluxCapControl[20] = 0; ui24.uFluxCapControl[21] = 0; ui24.uFluxCapControl[22] = 0; ui24.uFluxCapControl[23] = 0; ui24.uFluxCapControl[24] = 0; ui24.uFluxCapControl[25] = 0; ui24.uFluxCapControl[26] = 0; ui24.uFluxCapControl[27] = 0; ui24.uFluxCapControl[28] = 0; ui24.uFluxCapControl[29] = 0; ui24.uFluxCapControl[30] = 0; ui24.uFluxCapControl[31] = 0; ui24.uFluxCapControl[32] = 0; ui24.uFluxCapControl[33] = 0; ui24.uFluxCapControl[34] = 0; ui24.uFluxCapControl[35] = 0; ui24.uFluxCapControl[36] = 0; ui24.uFluxCapControl[37] = 0; ui24.uFluxCapControl[38] = 0; ui24.uFluxCapControl[39] = 0; ui24.uFluxCapControl[40] = 0; ui24.uFluxCapControl[41] = 0; ui24.uFluxCapControl[42] = 0; ui24.uFluxCapControl[43] = 0; ui24.uFluxCapControl[44] = 0; ui24.uFluxCapControl[45] = 0; ui24.uFluxCapControl[46] = 0; ui24.uFluxCapControl[47] = 0; ui24.uFluxCapControl[48] = 0; ui24.uFluxCapControl[49] = 0; ui24.uFluxCapControl[50] = 0; ui24.uFluxCapControl[51] = 0; ui24.uFluxCapControl[52] = 0; ui24.uFluxCapControl[53] = 0; ui24.uFluxCapControl[54] = 0; ui24.uFluxCapControl[55] = 0; ui24.uFluxCapControl[56] = 0; ui24.uFluxCapControl[57] = 0; ui24.uFluxCapControl[58] = 0; ui24.uFluxCapControl[59] = 0; ui24.uFluxCapControl[60] = 0; ui24.uFluxCapControl[61] = 0; ui24.uFluxCapControl[62] = 0; ui24.uFluxCapControl[63] = 0;
	ui24.fFluxCapData[0] = 0.000000; ui24.fFluxCapData[1] = 0.000000; ui24.fFluxCapData[2] = 0.000000; ui24.fFluxCapData[3] = 0.000000; ui24.fFluxCapData[4] = 0.000000; ui24.fFluxCapData[5] = 0.000000; ui24.fFluxCapData[6] = 0.000000; ui24.fFluxCapData[7] = 0.000000; ui24.fFluxCapData[8] = 0.000000; ui24.fFluxCapData[9] = 0.000000; ui24.fFluxCapData[10] = 0.000000; ui24.fFluxCapData[11] = 0.000000; ui24.fFluxCapData[12] = 0.000000; ui24.fFluxCapData[13] = 0.000000; ui24.fFluxCapData[14] = 0.000000; ui24.fFluxCapData[15] = 0.000000; ui24.fFluxCapData[16] = 0.000000; ui24.fFluxCapData[17] = 0.000000; ui24.fFluxCapData[18] = 0.000000; ui24.fFluxCapData[19] = 0.000000; ui24.fFluxCapData[20] = 0.000000; ui24.fFluxCapData[21] = 0.000000; ui24.fFluxCapData[22] = 0.000000; ui24.fFluxCapData[23] = 0.000000; ui24.fFluxCapData[24] = 0.000000; ui24.fFluxCapData[25] = 0.000000; ui24.fFluxCapData[26] = 0.000000; ui24.fFluxCapData[27] = 0.000000; ui24.fFluxCapData[28] = 0.000000; ui24.fFluxCapData[29] = 0.000000; ui24.fFluxCapData[30] = 0.000000; ui24.fFluxCapData[31] = 0.000000; ui24.fFluxCapData[32] = 0.000000; ui24.fFluxCapData[33] = 0.000000; ui24.fFluxCapData[34] = 0.000000; ui24.fFluxCapData[35] = 0.000000; ui24.fFluxCapData[36] = 0.000000; ui24.fFluxCapData[37] = 0.000000; ui24.fFluxCapData[38] = 0.000000; ui24.fFluxCapData[39] = 0.000000; ui24.fFluxCapData[40] = 0.000000; ui24.fFluxCapData[41] = 0.000000; ui24.fFluxCapData[42] = 0.000000; ui24.fFluxCapData[43] = 0.000000; ui24.fFluxCapData[44] = 0.000000; ui24.fFluxCapData[45] = 0.000000; ui24.fFluxCapData[46] = 0.000000; ui24.fFluxCapData[47] = 0.000000; ui24.fFluxCapData[48] = 0.000000; ui24.fFluxCapData[49] = 0.000000; ui24.fFluxCapData[50] = 0.000000; ui24.fFluxCapData[51] = 0.000000; ui24.fFluxCapData[52] = 0.000000; ui24.fFluxCapData[53] = 0.000000; ui24.fFluxCapData[54] = 0.000000; ui24.fFluxCapData[55] = 0.000000; ui24.fFluxCapData[56] = 0.000000; ui24.fFluxCapData[57] = 0.000000; ui24.fFluxCapData[58] = 0.000000; ui24.fFluxCapData[59] = 0.000000; ui24.fFluxCapData[60] = 0.000000; ui24.fFluxCapData[61] = 0.000000; ui24.fFluxCapData[62] = 0.000000; ui24.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui24);


	m_uRotorWaveform = 0;
	CUICtrl ui25;
	ui25.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui25.uControlId = 35;
	ui25.bLogSlider = false;
	ui25.bExpSlider = false;
	ui25.fUserDisplayDataLoLimit = 0.000000;
	ui25.fUserDisplayDataHiLimit = 7.000000;
	ui25.uUserDataType = UINTData;
	ui25.fInitUserIntValue = 0;
	ui25.fInitUserFloatValue = 0;
	ui25.fInitUserDoubleValue = 0;
	ui25.fInitUserUINTValue = 0.000000;
	ui25.m_pUserCookedIntData = NULL;
	ui25.m_pUserCookedFloatData = NULL;
	ui25.m_pUserCookedDoubleData = NULL;
	ui25.m_pUserCookedUINTData = &m_uRotorWaveform;
	ui25.cControlUnits = "                                                                ";
	ui25.cVariableName = "m_uRotorWaveform";
	ui25.cEnumeratedList = "sine,usaw,dsaw,tri,square,expo,rsh,qrsh";
	ui25.dPresetData[0] = 0.000000;ui25.dPresetData[1] = 0.000000;ui25.dPresetData[2] = 0.000000;ui25.dPresetData[3] = 0.000000;ui25.dPresetData[4] = 0.000000;ui25.dPresetData[5] = 0.000000;ui25.dPresetData[6] = 0.000000;ui25.dPresetData[7] = 0.000000;ui25.dPresetData[8] = 0.000000;ui25.dPresetData[9] = 0.000000;ui25.dPresetData[10] = 0.000000;ui25.dPresetData[11] = 0.000000;ui25.dPresetData[12] = 0.000000;ui25.dPresetData[13] = 0.000000;ui25.dPresetData[14] = 0.000000;ui25.dPresetData[15] = 0.000000;
	ui25.cControlName = "Rotor Waveform";
	ui25.bOwnerControl = false;
	ui25.bMIDIControl = false;
	ui25.uMIDIControlCommand = 176;
	ui25.uMIDIControlName = 3;
	ui25.uMIDIControlChannel = 0;
	ui25.nGUIRow = -1;
	ui25.nGUIColumn = -1;
	ui25.uControlTheme[0] = 0; ui25.uControlTheme[1] = 0; ui25.uControlTheme[2] = 0; ui25.uControlTheme[3] = 0; ui25.uControlTheme[4] = 0; ui25.uControlTheme[5] = 0; ui25.uControlTheme[6] = 0; ui25.uControlTheme[7] = 0; ui25.uControlTheme[8] = 0; ui25.uControlTheme[9] = 0; ui25.uControlTheme[10] = 0; ui25.uControlTheme[11] = 0; ui25.uControlTheme[12] = 0; ui25.uControlTheme[13] = 0; ui25.uControlTheme[14] = 0; ui25.uControlTheme[15] = 0; ui25.uControlTheme[16] = 0; ui25.uControlTheme[17] = 0; ui25.uControlTheme[18] = 0; ui25.uControlTheme[19] = 0; ui25.uControlTheme[20] = 0; ui25.uControlTheme[21] = 0; ui25.uControlTheme[22] = 0; ui25.uControlTheme[23] = 0; ui25.uControlTheme[24] = 0; ui25.uControlTheme[25] = 0; ui25.uControlTheme[26] = 0; ui25.uControlTheme[27] = 0; ui25.uControlTheme[28] = 0; ui25.uControlTheme[29] = 0; ui25.uControlTheme[30] = 0; ui25.uControlTheme[31] = 0;
	ui25.uFluxCapControl[0] = 0; ui25.uFluxCapControl[1] = 0; ui25.uFluxCapControl[2] = 0; ui25.uFluxCapControl[3] = 0; ui25.uFluxCapControl[4] = 0; ui25.uFluxCapControl[5] = 0; ui25.uFluxCapControl[6] = 0; ui25.uFluxCapControl[7] = 0; ui25.uFluxCapControl[8] = 0; ui25.uFluxCapControl[9] = 0; ui25.uFluxCapControl[10] = 0; ui25.uFluxCapControl[11] = 0; ui25.uFluxCapControl[12] = 0; ui25.uFluxCapControl[13] = 0; ui25.uFluxCapControl[14] = 0; ui25.uFluxCapControl[15] = 0; ui25.uFluxCapControl[16] = 0; ui25.uFluxCapControl[17] = 0; ui25.uFluxCapControl[18] = 0; ui25.uFluxCapControl[19] = 0; ui25.uFluxCapControl[20] = 0; ui25.uFluxCapControl[21] = 0; ui25.uFluxCapControl[22] = 0; ui25.uFluxCapControl[23] = 0; ui25.uFluxCapControl[24] = 0; ui25.uFluxCapControl[25] = 0; ui25.uFluxCapControl[26] = 0; ui25.uFluxCapControl[27] = 0; ui25.uFluxCapControl[28] = 0; ui25.uFluxCapControl[29] = 0; ui25.uFluxCapControl[30] = 0; ui25.uFluxCapControl[31] = 0; ui25.uFluxCapControl[32] = 0; ui25.uFluxCapControl[33] = 0; ui25.uFluxCapControl[34] = 0; ui25.uFluxCapControl[35] = 0; ui25.uFluxCapControl[36] = 0; ui25.uFluxCapControl[37] = 0; ui25.uFluxCapControl[38] = 0; ui25.uFluxCapControl[39] = 0; ui25.uFluxCapControl[40] = 0; ui25.uFluxCapControl[41] = 0; ui25.uFluxCapControl[42] = 0; ui25.uFluxCapControl[43] = 0; ui25.uFluxCapControl[44] = 0; ui25.uFluxCapControl[45] = 0; ui25.uFluxCapControl[46] = 0; ui25.uFluxCapControl[47] = 0; ui25.uFluxCapControl[48] = 0; ui25.uFluxCapControl[49] = 0; ui25.uFluxCapControl[50] = 0; ui25.uFluxCapControl[51] = 0; ui25.uFluxCapControl[52] = 0; ui25.uFluxCapControl[53] = 0; ui25.uFluxCapControl[54] = 0; ui25.uFluxCapControl[55] = 0; ui25.uFluxCapControl[56] = 0; ui25.uFluxCapControl[57] = 0; ui25.uFluxCapControl[58] = 0; ui25.uFluxCapControl[59] = 0; ui25.uFluxCapControl[60] = 0; ui25.uFluxCapControl[61] = 0; ui25.uFluxCapControl[62] = 0; ui25.uFluxCapControl[63] = 0;
	ui25.fFluxCapData[0] = 0.000000; ui25.fFluxCapData[1] = 0.000000; ui25.fFluxCapData[2] = 0.000000; ui25.fFluxCapData[3] = 0.000000; ui25.fFluxCapData[4] = 0.000000; ui25.fFluxCapData[5] = 0.000000; ui25.fFluxCapData[6] = 0.000000; ui25.fFluxCapData[7] = 0.000000; ui25.fFluxCapData[8] = 0.000000; ui25.fFluxCapData[9] = 0.000000; ui25.fFluxCapData[10] = 0.000000; ui25.fFluxCapData[11] = 0.000000; ui25.fFluxCapData[12] = 0.000000; ui25.fFluxCapData[13] = 0.000000; ui25.fFluxCapData[14] = 0.000000; ui25.fFluxCapData[15] = 0.000000; ui25.fFluxCapData[16] = 0.000000; ui25.fFluxCapData[17] = 0.000000; ui25.fFluxCapData[18] = 0.000000; ui25.fFluxCapData[19] = 0.000000; ui25.fFluxCapData[20] = 0.000000; ui25.fFluxCapData[21] = 0.000000; ui25.fFluxCapData[22] = 0.000000; ui25.fFluxCapData[23] = 0.000000; ui25.fFluxCapData[24] = 0.000000; ui25.fFluxCapData[25] = 0.000000; ui25.fFluxCapData[26] = 0.000000; ui25.fFluxCapData[27] = 0.000000; ui25.fFluxCapData[28] = 0.000000; ui25.fFluxCapData[29] = 0.000000; ui25.fFluxCapData[30] = 0.000000; ui25.fFluxCapData[31] = 0.000000; ui25.fFluxCapData[32] = 0.000000; ui25.fFluxCapData[33] = 0.000000; ui25.fFluxCapData[34] = 0.000000; ui25.fFluxCapData[35] = 0.000000; ui25.fFluxCapData[36] = 0.000000; ui25.fFluxCapData[37] = 0.000000; ui25.fFluxCapData[38] = 0.000000; ui25.fFluxCapData[39] = 0.000000; ui25.fFluxCapData[40] = 0.000000; ui25.fFluxCapData[41] = 0.000000; ui25.fFluxCapData[42] = 0.000000; ui25.fFluxCapData[43] = 0.000000; ui25.fFluxCapData[44] = 0.000000; ui25.fFluxCapData[45] = 0.000000; ui25.fFluxCapData[46] = 0.000000; ui25.fFluxCapData[47] = 0.000000; ui25.fFluxCapData[48] = 0.000000; ui25.fFluxCapData[49] = 0.000000; ui25.fFluxCapData[50] = 0.000000; ui25.fFluxCapData[51] = 0.000000; ui25.fFluxCapData[52] = 0.000000; ui25.fFluxCapData[53] = 0.000000; ui25.fFluxCapData[54] = 0.000000; ui25.fFluxCapData[55] = 0.000000; ui25.fFluxCapData[56] = 0.000000; ui25.fFluxCapData[57] = 0.000000; ui25.fFluxCapData[58] = 0.000000; ui25.fFluxCapData[59] = 0.000000; ui25.fFluxCapData[60] = 0.000000; ui25.fFluxCapData[61] = 0.000000; ui25.fFluxCapData[62] = 0.000000; ui25.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui25);


	m_dVolume_dB = 0.000000;
	CUICtrl ui26;
	ui26.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui26.uControlId = 100;
	ui26.bLogSlider = false;
	ui26.bExpSlider = false;
	ui26.fUserDisplayDataLoLimit = -96.000000;
	ui26.fUserDisplayDataHiLimit = 24.000000;
	ui26.uUserDataType = doubleData;
	ui26.fInitUserIntValue = 0;
	ui26.fInitUserFloatValue = 0;
	ui26.fInitUserDoubleValue = 0.000000;
	ui26.fInitUserUINTValue = 0;
	ui26.m_pUserCookedIntData = NULL;
	ui26.m_pUserCookedFloatData = NULL;
	ui26.m_pUserCookedDoubleData = &m_dVolume_dB;
	ui26.m_pUserCookedUINTData = NULL;
	ui26.cControlUnits = "dB                                                              ";
	ui26.cVariableName = "m_dVolume_dB";
	ui26.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui26.dPresetData[0] = 0.000000;ui26.dPresetData[1] = 0.000000;ui26.dPresetData[2] = 0.000000;ui26.dPresetData[3] = 0.000000;ui26.dPresetData[4] = 0.000000;ui26.dPresetData[5] = 0.000000;ui26.dPresetData[6] = 0.000000;ui26.dPresetData[7] = 0.000000;ui26.dPresetData[8] = 0.000000;ui26.dPresetData[9] = 0.000000;ui26.dPresetData[10] = 0.000000;ui26.dPresetData[11] = 0.000000;ui26.dPresetData[12] = 0.000000;ui26.dPresetData[13] = 0.000000;ui26.dPresetData[14] = 0.000000;ui26.dPresetData[15] = 0.000000;
	ui26.cControlName = "Volume";
	ui26.bOwnerControl = false;
	ui26.bMIDIControl = false;
	ui26.uMIDIControlCommand = 176;
	ui26.uMIDIControlName = 3;
	ui26.uMIDIControlChannel = 0;
	ui26.nGUIRow = -1;
	ui26.nGUIColumn = -1;
	ui26.uControlTheme[0] = 0; ui26.uControlTheme[1] = 0; ui26.uControlTheme[2] = 0; ui26.uControlTheme[3] = 0; ui26.uControlTheme[4] = 0; ui26.uControlTheme[5] = 0; ui26.uControlTheme[6] = 0; ui26.uControlTheme[7] = 0; ui26.uControlTheme[8] = 0; ui26.uControlTheme[9] = 0; ui26.uControlTheme[10] = 0; ui26.uControlTheme[11] = 0; ui26.uControlTheme[12] = 0; ui26.uControlTheme[13] = 0; ui26.uControlTheme[14] = 0; ui26.uControlTheme[15] = 0; ui26.uControlTheme[16] = 0; ui26.uControlTheme[17] = 0; ui26.uControlTheme[18] = 0; ui26.uControlTheme[19] = 0; ui26.uControlTheme[20] = 0; ui26.uControlTheme[21] = 0; ui26.uControlTheme[22] = 0; ui26.uControlTheme[23] = 0; ui26.uControlTheme[24] = 0; ui26.uControlTheme[25] = 0; ui26.uControlTheme[26] = 0; ui26.uControlTheme[27] = 0; ui26.uControlTheme[28] = 0; ui26.uControlTheme[29] = 0; ui26.uControlTheme[30] = 0; ui26.uControlTheme[31] = 0;
	ui26.uFluxCapControl[0] = 1; ui26.uFluxCapControl[1] = 0; ui26.uFluxCapControl[2] = 0; ui26.uFluxCapControl[3] = 0; ui26.uFluxCapControl[4] = 0; ui26.uFluxCapControl[5] = 0; ui26.uFluxCapControl[6] = 0; ui26.uFluxCapControl[7] = 0; ui26.uFluxCapControl[8] = 0; ui26.uFluxCapControl[9] = 0; ui26.uFluxCapControl[10] = 0; ui26.uFluxCapControl[11] = 0; ui26.uFluxCapControl[12] = 0; ui26.uFluxCapControl[13] = 0; ui26.uFluxCapControl[14] = 0; ui26.uFluxCapControl[15] = 0; ui26.uFluxCapControl[16] = 0; ui26.uFluxCapControl[17] = 0; ui26.uFluxCapControl[18] = 0; ui26.uFluxCapControl[19] = 0; ui26.uFluxCapControl[20] = 0; ui26.uFluxCapControl[21] = 0; ui26.uFluxCapControl[22] = 0; ui26.uFluxCapControl[23] = 0; ui26.uFluxCapControl[24] = 0; ui26.uFluxCapControl[25] = 0; ui26.uFluxCapControl[26] = 0; ui26.uFluxCapControl[27] = 0; ui26.uFluxCapControl[28] = 0; ui26.uFluxCapControl[29] = 0; ui26.uFluxCapControl[30] = 0; ui26.uFluxCapControl[31] = 0; ui26.uFluxCapControl[32] = 0; ui26.uFluxCapControl[33] = 0; ui26.uFluxCapControl[34] = 0; ui26.uFluxCapControl[35] = 0; ui26.uFluxCapControl[36] = 0; ui26.uFluxCapControl[37] = 0; ui26.uFluxCapControl[38] = 0; ui26.uFluxCapControl[39] = 0; ui26.uFluxCapControl[40] = 0; ui26.uFluxCapControl[41] = 0; ui26.uFluxCapControl[42] = 0; ui26.uFluxCapControl[43] = 0; ui26.uFluxCapControl[44] = 0; ui26.uFluxCapControl[45] = 0; ui26.uFluxCapControl[46] = 0; ui26.uFluxCapControl[47] = 0; ui26.uFluxCapControl[48] = 0; ui26.uFluxCapControl[49] = 0; ui26.uFluxCapControl[50] = 0; ui26.uFluxCapControl[51] = 0; ui26.uFluxCapControl[52] = 0; ui26.uFluxCapControl[53] = 0; ui26.uFluxCapControl[54] = 0; ui26.uFluxCapControl[55] = 0; ui26.uFluxCapControl[56] = 0; ui26.uFluxCapControl[57] = 0; ui26.uFluxCapControl[58] = 0; ui26.uFluxCapControl[59] = 0; ui26.uFluxCapControl[60] = 0; ui26.uFluxCapControl[61] = 0; ui26.uFluxCapControl[62] = 0; ui26.uFluxCapControl[63] = 0;
	ui26.fFluxCapData[0] = 0.000000; ui26.fFluxCapData[1] = 0.000000; ui26.fFluxCapData[2] = 0.000000; ui26.fFluxCapData[3] = 0.000000; ui26.fFluxCapData[4] = 0.000000; ui26.fFluxCapData[5] = 0.000000; ui26.fFluxCapData[6] = 0.000000; ui26.fFluxCapData[7] = 0.000000; ui26.fFluxCapData[8] = 0.000000; ui26.fFluxCapData[9] = 0.000000; ui26.fFluxCapData[10] = 0.000000; ui26.fFluxCapData[11] = 0.000000; ui26.fFluxCapData[12] = 0.000000; ui26.fFluxCapData[13] = 0.000000; ui26.fFluxCapData[14] = 0.000000; ui26.fFluxCapData[15] = 0.000000; ui26.fFluxCapData[16] = 0.000000; ui26.fFluxCapData[17] = 0.000000; ui26.fFluxCapData[18] = 0.000000; ui26.fFluxCapData[19] = 0.000000; ui26.fFluxCapData[20] = 0.000000; ui26.fFluxCapData[21] = 0.000000; ui26.fFluxCapData[22] = 0.000000; ui26.fFluxCapData[23] = 0.000000; ui26.fFluxCapData[24] = 0.000000; ui26.fFluxCapData[25] = 0.000000; ui26.fFluxCapData[26] = 0.000000; ui26.fFluxCapData[27] = 0.000000; ui26.fFluxCapData[28] = 0.000000; ui26.fFluxCapData[29] = 0.000000; ui26.fFluxCapData[30] = 0.000000; ui26.fFluxCapData[31] = 0.000000; ui26.fFluxCapData[32] = 0.000000; ui26.fFluxCapData[33] = 0.000000; ui26.fFluxCapData[34] = 0.000000; ui26.fFluxCapData[35] = 0.000000; ui26.fFluxCapData[36] = 0.000000; ui26.fFluxCapData[37] = 0.000000; ui26.fFluxCapData[38] = 0.000000; ui26.fFluxCapData[39] = 0.000000; ui26.fFluxCapData[40] = 0.000000; ui26.fFluxCapData[41] = 0.000000; ui26.fFluxCapData[42] = 0.000000; ui26.fFluxCapData[43] = 0.000000; ui26.fFluxCapData[44] = 0.000000; ui26.fFluxCapData[45] = 0.000000; ui26.fFluxCapData[46] = 0.000000; ui26.fFluxCapData[47] = 0.000000; ui26.fFluxCapData[48] = 0.000000; ui26.fFluxCapData[49] = 0.000000; ui26.fFluxCapData[50] = 0.000000; ui26.fFluxCapData[51] = 0.000000; ui26.fFluxCapData[52] = 0.000000; ui26.fFluxCapData[53] = 0.000000; ui26.fFluxCapData[54] = 0.000000; ui26.fFluxCapData[55] = 0.000000; ui26.fFluxCapData[56] = 0.000000; ui26.fFluxCapData[57] = 0.000000; ui26.fFluxCapData[58] = 0.000000; ui26.fFluxCapData[59] = 0.000000; ui26.fFluxCapData[60] = 0.000000; ui26.fFluxCapData[61] = 0.000000; ui26.fFluxCapData[62] = 0.000000; ui26.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui26);


	m_uLegatoMode = 0;
	CUICtrl ui27;
	ui27.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui27.uControlId = 101;
	ui27.bLogSlider = false;
	ui27.bExpSlider = false;
	ui27.fUserDisplayDataLoLimit = 0.000000;
	ui27.fUserDisplayDataHiLimit = 1.000000;
	ui27.uUserDataType = UINTData;
	ui27.fInitUserIntValue = 0;
	ui27.fInitUserFloatValue = 0;
	ui27.fInitUserDoubleValue = 0;
	ui27.fInitUserUINTValue = 0.000000;
	ui27.m_pUserCookedIntData = NULL;
	ui27.m_pUserCookedFloatData = NULL;
	ui27.m_pUserCookedDoubleData = NULL;
	ui27.m_pUserCookedUINTData = &m_uLegatoMode;
	ui27.cControlUnits = "                                                                ";
	ui27.cVariableName = "m_uLegatoMode";
	ui27.cEnumeratedList = "mono,legato";
	ui27.dPresetData[0] = 0.000000;ui27.dPresetData[1] = 0.000000;ui27.dPresetData[2] = 0.000000;ui27.dPresetData[3] = 0.000000;ui27.dPresetData[4] = 0.000000;ui27.dPresetData[5] = 0.000000;ui27.dPresetData[6] = 0.000000;ui27.dPresetData[7] = 0.000000;ui27.dPresetData[8] = 0.000000;ui27.dPresetData[9] = 0.000000;ui27.dPresetData[10] = 0.000000;ui27.dPresetData[11] = 0.000000;ui27.dPresetData[12] = 0.000000;ui27.dPresetData[13] = 0.000000;ui27.dPresetData[14] = 0.000000;ui27.dPresetData[15] = 0.000000;
	ui27.cControlName = "Legato Mode";
	ui27.bOwnerControl = false;
	ui27.bMIDIControl = false;
	ui27.uMIDIControlCommand = 176;
	ui27.uMIDIControlName = 3;
	ui27.uMIDIControlChannel = 0;
	ui27.nGUIRow = -1;
	ui27.nGUIColumn = -1;
	ui27.uControlTheme[0] = 0; ui27.uControlTheme[1] = 0; ui27.uControlTheme[2] = 0; ui27.uControlTheme[3] = 0; ui27.uControlTheme[4] = 0; ui27.uControlTheme[5] = 0; ui27.uControlTheme[6] = 0; ui27.uControlTheme[7] = 0; ui27.uControlTheme[8] = 0; ui27.uControlTheme[9] = 0; ui27.uControlTheme[10] = 0; ui27.uControlTheme[11] = 0; ui27.uControlTheme[12] = 0; ui27.uControlTheme[13] = 0; ui27.uControlTheme[14] = 0; ui27.uControlTheme[15] = 0; ui27.uControlTheme[16] = 0; ui27.uControlTheme[17] = 0; ui27.uControlTheme[18] = 0; ui27.uControlTheme[19] = 0; ui27.uControlTheme[20] = 0; ui27.uControlTheme[21] = 0; ui27.uControlTheme[22] = 0; ui27.uControlTheme[23] = 0; ui27.uControlTheme[24] = 0; ui27.uControlTheme[25] = 0; ui27.uControlTheme[26] = 0; ui27.uControlTheme[27] = 0; ui27.uControlTheme[28] = 0; ui27.uControlTheme[29] = 0; ui27.uControlTheme[30] = 0; ui27.uControlTheme[31] = 0;
	ui27.uFluxCapControl[0] = 1; ui27.uFluxCapControl[1] = 1; ui27.uFluxCapControl[2] = 0; ui27.uFluxCapControl[3] = 0; ui27.uFluxCapControl[4] = 0; ui27.uFluxCapControl[5] = 0; ui27.uFluxCapControl[6] = 0; ui27.uFluxCapControl[7] = 0; ui27.uFluxCapControl[8] = 0; ui27.uFluxCapControl[9] = 0; ui27.uFluxCapControl[10] = 0; ui27.uFluxCapControl[11] = 0; ui27.uFluxCapControl[12] = 0; ui27.uFluxCapControl[13] = 0; ui27.uFluxCapControl[14] = 0; ui27.uFluxCapControl[15] = 0; ui27.uFluxCapControl[16] = 0; ui27.uFluxCapControl[17] = 0; ui27.uFluxCapControl[18] = 0; ui27.uFluxCapControl[19] = 0; ui27.uFluxCapControl[20] = 0; ui27.uFluxCapControl[21] = 0; ui27.uFluxCapControl[22] = 0; ui27.uFluxCapControl[23] = 0; ui27.uFluxCapControl[24] = 0; ui27.uFluxCapControl[25] = 0; ui27.uFluxCapControl[26] = 0; ui27.uFluxCapControl[27] = 0; ui27.uFluxCapControl[28] = 0; ui27.uFluxCapControl[29] = 0; ui27.uFluxCapControl[30] = 0; ui27.uFluxCapControl[31] = 0; ui27.uFluxCapControl[32] = 0; ui27.uFluxCapControl[33] = 0; ui27.uFluxCapControl[34] = 0; ui27.uFluxCapControl[35] = 0; ui27.uFluxCapControl[36] = 0; ui27.uFluxCapControl[37] = 0; ui27.uFluxCapControl[38] = 0; ui27.uFluxCapControl[39] = 0; ui27.uFluxCapControl[40] = 0; ui27.uFluxCapControl[41] = 0; ui27.uFluxCapControl[42] = 0; ui27.uFluxCapControl[43] = 0; ui27.uFluxCapControl[44] = 0; ui27.uFluxCapControl[45] = 0; ui27.uFluxCapControl[46] = 0; ui27.uFluxCapControl[47] = 0; ui27.uFluxCapControl[48] = 0; ui27.uFluxCapControl[49] = 0; ui27.uFluxCapControl[50] = 0; ui27.uFluxCapControl[51] = 0; ui27.uFluxCapControl[52] = 0; ui27.uFluxCapControl[53] = 0; ui27.uFluxCapControl[54] = 0; ui27.uFluxCapControl[55] = 0; ui27.uFluxCapControl[56] = 0; ui27.uFluxCapControl[57] = 0; ui27.uFluxCapControl[58] = 0; ui27.uFluxCapControl[59] = 0; ui27.uFluxCapControl[60] = 0; ui27.uFluxCapControl[61] = 0; ui27.uFluxCapControl[62] = 0; ui27.uFluxCapControl[63] = 0;
	ui27.fFluxCapData[0] = 0.000000; ui27.fFluxCapData[1] = 0.000000; ui27.fFluxCapData[2] = 0.000000; ui27.fFluxCapData[3] = 0.000000; ui27.fFluxCapData[4] = 0.000000; ui27.fFluxCapData[5] = 0.000000; ui27.fFluxCapData[6] = 0.000000; ui27.fFluxCapData[7] = 0.000000; ui27.fFluxCapData[8] = 0.000000; ui27.fFluxCapData[9] = 0.000000; ui27.fFluxCapData[10] = 0.000000; ui27.fFluxCapData[11] = 0.000000; ui27.fFluxCapData[12] = 0.000000; ui27.fFluxCapData[13] = 0.000000; ui27.fFluxCapData[14] = 0.000000; ui27.fFluxCapData[15] = 0.000000; ui27.fFluxCapData[16] = 0.000000; ui27.fFluxCapData[17] = 0.000000; ui27.fFluxCapData[18] = 0.000000; ui27.fFluxCapData[19] = 0.000000; ui27.fFluxCapData[20] = 0.000000; ui27.fFluxCapData[21] = 0.000000; ui27.fFluxCapData[22] = 0.000000; ui27.fFluxCapData[23] = 0.000000; ui27.fFluxCapData[24] = 0.000000; ui27.fFluxCapData[25] = 0.000000; ui27.fFluxCapData[26] = 0.000000; ui27.fFluxCapData[27] = 0.000000; ui27.fFluxCapData[28] = 0.000000; ui27.fFluxCapData[29] = 0.000000; ui27.fFluxCapData[30] = 0.000000; ui27.fFluxCapData[31] = 0.000000; ui27.fFluxCapData[32] = 0.000000; ui27.fFluxCapData[33] = 0.000000; ui27.fFluxCapData[34] = 0.000000; ui27.fFluxCapData[35] = 0.000000; ui27.fFluxCapData[36] = 0.000000; ui27.fFluxCapData[37] = 0.000000; ui27.fFluxCapData[38] = 0.000000; ui27.fFluxCapData[39] = 0.000000; ui27.fFluxCapData[40] = 0.000000; ui27.fFluxCapData[41] = 0.000000; ui27.fFluxCapData[42] = 0.000000; ui27.fFluxCapData[43] = 0.000000; ui27.fFluxCapData[44] = 0.000000; ui27.fFluxCapData[45] = 0.000000; ui27.fFluxCapData[46] = 0.000000; ui27.fFluxCapData[47] = 0.000000; ui27.fFluxCapData[48] = 0.000000; ui27.fFluxCapData[49] = 0.000000; ui27.fFluxCapData[50] = 0.000000; ui27.fFluxCapData[51] = 0.000000; ui27.fFluxCapData[52] = 0.000000; ui27.fFluxCapData[53] = 0.000000; ui27.fFluxCapData[54] = 0.000000; ui27.fFluxCapData[55] = 0.000000; ui27.fFluxCapData[56] = 0.000000; ui27.fFluxCapData[57] = 0.000000; ui27.fFluxCapData[58] = 0.000000; ui27.fFluxCapData[59] = 0.000000; ui27.fFluxCapData[60] = 0.000000; ui27.fFluxCapData[61] = 0.000000; ui27.fFluxCapData[62] = 0.000000; ui27.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui27);


	m_nPitchBendRange = 1;
	CUICtrl ui28;
	ui28.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui28.uControlId = 102;
	ui28.bLogSlider = false;
	ui28.bExpSlider = false;
	ui28.fUserDisplayDataLoLimit = 1.000000;
	ui28.fUserDisplayDataHiLimit = 12.000000;
	ui28.uUserDataType = intData;
	ui28.fInitUserIntValue = 1.000000;
	ui28.fInitUserFloatValue = 0;
	ui28.fInitUserDoubleValue = 0;
	ui28.fInitUserUINTValue = 0;
	ui28.m_pUserCookedIntData = &m_nPitchBendRange;
	ui28.m_pUserCookedFloatData = NULL;
	ui28.m_pUserCookedDoubleData = NULL;
	ui28.m_pUserCookedUINTData = NULL;
	ui28.cControlUnits = "semitones                                                       ";
	ui28.cVariableName = "m_nPitchBendRange";
	ui28.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui28.dPresetData[0] = 1.000000;ui28.dPresetData[1] = 0.000000;ui28.dPresetData[2] = 0.000000;ui28.dPresetData[3] = 0.000000;ui28.dPresetData[4] = 0.000000;ui28.dPresetData[5] = 0.000000;ui28.dPresetData[6] = 0.000000;ui28.dPresetData[7] = 0.000000;ui28.dPresetData[8] = 0.000000;ui28.dPresetData[9] = 0.000000;ui28.dPresetData[10] = 0.000000;ui28.dPresetData[11] = 0.000000;ui28.dPresetData[12] = 0.000000;ui28.dPresetData[13] = 0.000000;ui28.dPresetData[14] = 0.000000;ui28.dPresetData[15] = 0.000000;
	ui28.cControlName = "PBend Range";
	ui28.bOwnerControl = false;
	ui28.bMIDIControl = false;
	ui28.uMIDIControlCommand = 176;
	ui28.uMIDIControlName = 3;
	ui28.uMIDIControlChannel = 0;
	ui28.nGUIRow = -1;
	ui28.nGUIColumn = -1;
	ui28.uControlTheme[0] = 0; ui28.uControlTheme[1] = 0; ui28.uControlTheme[2] = 0; ui28.uControlTheme[3] = 0; ui28.uControlTheme[4] = 0; ui28.uControlTheme[5] = 0; ui28.uControlTheme[6] = 0; ui28.uControlTheme[7] = 0; ui28.uControlTheme[8] = 0; ui28.uControlTheme[9] = 0; ui28.uControlTheme[10] = 0; ui28.uControlTheme[11] = 0; ui28.uControlTheme[12] = 0; ui28.uControlTheme[13] = 0; ui28.uControlTheme[14] = 0; ui28.uControlTheme[15] = 0; ui28.uControlTheme[16] = 0; ui28.uControlTheme[17] = 0; ui28.uControlTheme[18] = 0; ui28.uControlTheme[19] = 0; ui28.uControlTheme[20] = 0; ui28.uControlTheme[21] = 0; ui28.uControlTheme[22] = 0; ui28.uControlTheme[23] = 0; ui28.uControlTheme[24] = 0; ui28.uControlTheme[25] = 0; ui28.uControlTheme[26] = 0; ui28.uControlTheme[27] = 0; ui28.uControlTheme[28] = 0; ui28.uControlTheme[29] = 0; ui28.uControlTheme[30] = 0; ui28.uControlTheme[31] = 0;
	ui28.uFluxCapControl[0] = 1; ui28.uFluxCapControl[1] = 3; ui28.uFluxCapControl[2] = 0; ui28.uFluxCapControl[3] = 0; ui28.uFluxCapControl[4] = 0; ui28.uFluxCapControl[5] = 0; ui28.uFluxCapControl[6] = 0; ui28.uFluxCapControl[7] = 0; ui28.uFluxCapControl[8] = 0; ui28.uFluxCapControl[9] = 0; ui28.uFluxCapControl[10] = 0; ui28.uFluxCapControl[11] = 0; ui28.uFluxCapControl[12] = 0; ui28.uFluxCapControl[13] = 0; ui28.uFluxCapControl[14] = 0; ui28.uFluxCapControl[15] = 0; ui28.uFluxCapControl[16] = 0; ui28.uFluxCapControl[17] = 0; ui28.uFluxCapControl[18] = 0; ui28.uFluxCapControl[19] = 0; ui28.uFluxCapControl[20] = 0; ui28.uFluxCapControl[21] = 0; ui28.uFluxCapControl[22] = 0; ui28.uFluxCapControl[23] = 0; ui28.uFluxCapControl[24] = 0; ui28.uFluxCapControl[25] = 0; ui28.uFluxCapControl[26] = 0; ui28.uFluxCapControl[27] = 0; ui28.uFluxCapControl[28] = 0; ui28.uFluxCapControl[29] = 0; ui28.uFluxCapControl[30] = 0; ui28.uFluxCapControl[31] = 0; ui28.uFluxCapControl[32] = 0; ui28.uFluxCapControl[33] = 0; ui28.uFluxCapControl[34] = 0; ui28.uFluxCapControl[35] = 0; ui28.uFluxCapControl[36] = 0; ui28.uFluxCapControl[37] = 0; ui28.uFluxCapControl[38] = 0; ui28.uFluxCapControl[39] = 0; ui28.uFluxCapControl[40] = 0; ui28.uFluxCapControl[41] = 0; ui28.uFluxCapControl[42] = 0; ui28.uFluxCapControl[43] = 0; ui28.uFluxCapControl[44] = 0; ui28.uFluxCapControl[45] = 0; ui28.uFluxCapControl[46] = 0; ui28.uFluxCapControl[47] = 0; ui28.uFluxCapControl[48] = 0; ui28.uFluxCapControl[49] = 0; ui28.uFluxCapControl[50] = 0; ui28.uFluxCapControl[51] = 0; ui28.uFluxCapControl[52] = 0; ui28.uFluxCapControl[53] = 0; ui28.uFluxCapControl[54] = 0; ui28.uFluxCapControl[55] = 0; ui28.uFluxCapControl[56] = 0; ui28.uFluxCapControl[57] = 0; ui28.uFluxCapControl[58] = 0; ui28.uFluxCapControl[59] = 0; ui28.uFluxCapControl[60] = 0; ui28.uFluxCapControl[61] = 0; ui28.uFluxCapControl[62] = 0; ui28.uFluxCapControl[63] = 0;
	ui28.fFluxCapData[0] = 0.000000; ui28.fFluxCapData[1] = 0.000000; ui28.fFluxCapData[2] = 0.000000; ui28.fFluxCapData[3] = 0.000000; ui28.fFluxCapData[4] = 0.000000; ui28.fFluxCapData[5] = 0.000000; ui28.fFluxCapData[6] = 0.000000; ui28.fFluxCapData[7] = 0.000000; ui28.fFluxCapData[8] = 0.000000; ui28.fFluxCapData[9] = 0.000000; ui28.fFluxCapData[10] = 0.000000; ui28.fFluxCapData[11] = 0.000000; ui28.fFluxCapData[12] = 0.000000; ui28.fFluxCapData[13] = 0.000000; ui28.fFluxCapData[14] = 0.000000; ui28.fFluxCapData[15] = 0.000000; ui28.fFluxCapData[16] = 0.000000; ui28.fFluxCapData[17] = 0.000000; ui28.fFluxCapData[18] = 0.000000; ui28.fFluxCapData[19] = 0.000000; ui28.fFluxCapData[20] = 0.000000; ui28.fFluxCapData[21] = 0.000000; ui28.fFluxCapData[22] = 0.000000; ui28.fFluxCapData[23] = 0.000000; ui28.fFluxCapData[24] = 0.000000; ui28.fFluxCapData[25] = 0.000000; ui28.fFluxCapData[26] = 0.000000; ui28.fFluxCapData[27] = 0.000000; ui28.fFluxCapData[28] = 0.000000; ui28.fFluxCapData[29] = 0.000000; ui28.fFluxCapData[30] = 0.000000; ui28.fFluxCapData[31] = 0.000000; ui28.fFluxCapData[32] = 0.000000; ui28.fFluxCapData[33] = 0.000000; ui28.fFluxCapData[34] = 0.000000; ui28.fFluxCapData[35] = 0.000000; ui28.fFluxCapData[36] = 0.000000; ui28.fFluxCapData[37] = 0.000000; ui28.fFluxCapData[38] = 0.000000; ui28.fFluxCapData[39] = 0.000000; ui28.fFluxCapData[40] = 0.000000; ui28.fFluxCapData[41] = 0.000000; ui28.fFluxCapData[42] = 0.000000; ui28.fFluxCapData[43] = 0.000000; ui28.fFluxCapData[44] = 0.000000; ui28.fFluxCapData[45] = 0.000000; ui28.fFluxCapData[46] = 0.000000; ui28.fFluxCapData[47] = 0.000000; ui28.fFluxCapData[48] = 0.000000; ui28.fFluxCapData[49] = 0.000000; ui28.fFluxCapData[50] = 0.000000; ui28.fFluxCapData[51] = 0.000000; ui28.fFluxCapData[52] = 0.000000; ui28.fFluxCapData[53] = 0.000000; ui28.fFluxCapData[54] = 0.000000; ui28.fFluxCapData[55] = 0.000000; ui28.fFluxCapData[56] = 0.000000; ui28.fFluxCapData[57] = 0.000000; ui28.fFluxCapData[58] = 0.000000; ui28.fFluxCapData[59] = 0.000000; ui28.fFluxCapData[60] = 0.000000; ui28.fFluxCapData[61] = 0.000000; ui28.fFluxCapData[62] = 0.000000; ui28.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui28);


	m_uResetToZero = 0;
	CUICtrl ui29;
	ui29.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui29.uControlId = 103;
	ui29.bLogSlider = false;
	ui29.bExpSlider = false;
	ui29.fUserDisplayDataLoLimit = 0.000000;
	ui29.fUserDisplayDataHiLimit = 1.000000;
	ui29.uUserDataType = UINTData;
	ui29.fInitUserIntValue = 0;
	ui29.fInitUserFloatValue = 0;
	ui29.fInitUserDoubleValue = 0;
	ui29.fInitUserUINTValue = 0.000000;
	ui29.m_pUserCookedIntData = NULL;
	ui29.m_pUserCookedFloatData = NULL;
	ui29.m_pUserCookedDoubleData = NULL;
	ui29.m_pUserCookedUINTData = &m_uResetToZero;
	ui29.cControlUnits = "                                                                ";
	ui29.cVariableName = "m_uResetToZero";
	ui29.cEnumeratedList = "OFF,ON";
	ui29.dPresetData[0] = 0.000000;ui29.dPresetData[1] = 0.000000;ui29.dPresetData[2] = 0.000000;ui29.dPresetData[3] = 0.000000;ui29.dPresetData[4] = 0.000000;ui29.dPresetData[5] = 0.000000;ui29.dPresetData[6] = 0.000000;ui29.dPresetData[7] = 0.000000;ui29.dPresetData[8] = 0.000000;ui29.dPresetData[9] = 0.000000;ui29.dPresetData[10] = 0.000000;ui29.dPresetData[11] = 0.000000;ui29.dPresetData[12] = 0.000000;ui29.dPresetData[13] = 0.000000;ui29.dPresetData[14] = 0.000000;ui29.dPresetData[15] = 0.000000;
	ui29.cControlName = "Reset to Zero";
	ui29.bOwnerControl = false;
	ui29.bMIDIControl = false;
	ui29.uMIDIControlCommand = 176;
	ui29.uMIDIControlName = 3;
	ui29.uMIDIControlChannel = 0;
	ui29.nGUIRow = -1;
	ui29.nGUIColumn = -1;
	ui29.uControlTheme[0] = 0; ui29.uControlTheme[1] = 0; ui29.uControlTheme[2] = 0; ui29.uControlTheme[3] = 0; ui29.uControlTheme[4] = 0; ui29.uControlTheme[5] = 0; ui29.uControlTheme[6] = 0; ui29.uControlTheme[7] = 0; ui29.uControlTheme[8] = 0; ui29.uControlTheme[9] = 0; ui29.uControlTheme[10] = 0; ui29.uControlTheme[11] = 0; ui29.uControlTheme[12] = 0; ui29.uControlTheme[13] = 0; ui29.uControlTheme[14] = 0; ui29.uControlTheme[15] = 0; ui29.uControlTheme[16] = 0; ui29.uControlTheme[17] = 0; ui29.uControlTheme[18] = 0; ui29.uControlTheme[19] = 0; ui29.uControlTheme[20] = 0; ui29.uControlTheme[21] = 0; ui29.uControlTheme[22] = 0; ui29.uControlTheme[23] = 0; ui29.uControlTheme[24] = 0; ui29.uControlTheme[25] = 0; ui29.uControlTheme[26] = 0; ui29.uControlTheme[27] = 0; ui29.uControlTheme[28] = 0; ui29.uControlTheme[29] = 0; ui29.uControlTheme[30] = 0; ui29.uControlTheme[31] = 0;
	ui29.uFluxCapControl[0] = 1; ui29.uFluxCapControl[1] = 2; ui29.uFluxCapControl[2] = 0; ui29.uFluxCapControl[3] = 0; ui29.uFluxCapControl[4] = 0; ui29.uFluxCapControl[5] = 0; ui29.uFluxCapControl[6] = 0; ui29.uFluxCapControl[7] = 0; ui29.uFluxCapControl[8] = 0; ui29.uFluxCapControl[9] = 0; ui29.uFluxCapControl[10] = 0; ui29.uFluxCapControl[11] = 0; ui29.uFluxCapControl[12] = 0; ui29.uFluxCapControl[13] = 0; ui29.uFluxCapControl[14] = 0; ui29.uFluxCapControl[15] = 0; ui29.uFluxCapControl[16] = 0; ui29.uFluxCapControl[17] = 0; ui29.uFluxCapControl[18] = 0; ui29.uFluxCapControl[19] = 0; ui29.uFluxCapControl[20] = 0; ui29.uFluxCapControl[21] = 0; ui29.uFluxCapControl[22] = 0; ui29.uFluxCapControl[23] = 0; ui29.uFluxCapControl[24] = 0; ui29.uFluxCapControl[25] = 0; ui29.uFluxCapControl[26] = 0; ui29.uFluxCapControl[27] = 0; ui29.uFluxCapControl[28] = 0; ui29.uFluxCapControl[29] = 0; ui29.uFluxCapControl[30] = 0; ui29.uFluxCapControl[31] = 0; ui29.uFluxCapControl[32] = 0; ui29.uFluxCapControl[33] = 0; ui29.uFluxCapControl[34] = 0; ui29.uFluxCapControl[35] = 0; ui29.uFluxCapControl[36] = 0; ui29.uFluxCapControl[37] = 0; ui29.uFluxCapControl[38] = 0; ui29.uFluxCapControl[39] = 0; ui29.uFluxCapControl[40] = 0; ui29.uFluxCapControl[41] = 0; ui29.uFluxCapControl[42] = 0; ui29.uFluxCapControl[43] = 0; ui29.uFluxCapControl[44] = 0; ui29.uFluxCapControl[45] = 0; ui29.uFluxCapControl[46] = 0; ui29.uFluxCapControl[47] = 0; ui29.uFluxCapControl[48] = 0; ui29.uFluxCapControl[49] = 0; ui29.uFluxCapControl[50] = 0; ui29.uFluxCapControl[51] = 0; ui29.uFluxCapControl[52] = 0; ui29.uFluxCapControl[53] = 0; ui29.uFluxCapControl[54] = 0; ui29.uFluxCapControl[55] = 0; ui29.uFluxCapControl[56] = 0; ui29.uFluxCapControl[57] = 0; ui29.uFluxCapControl[58] = 0; ui29.uFluxCapControl[59] = 0; ui29.uFluxCapControl[60] = 0; ui29.uFluxCapControl[61] = 0; ui29.uFluxCapControl[62] = 0; ui29.uFluxCapControl[63] = 0;
	ui29.fFluxCapData[0] = 0.000000; ui29.fFluxCapData[1] = 0.000000; ui29.fFluxCapData[2] = 0.000000; ui29.fFluxCapData[3] = 0.000000; ui29.fFluxCapData[4] = 0.000000; ui29.fFluxCapData[5] = 0.000000; ui29.fFluxCapData[6] = 0.000000; ui29.fFluxCapData[7] = 0.000000; ui29.fFluxCapData[8] = 0.000000; ui29.fFluxCapData[9] = 0.000000; ui29.fFluxCapData[10] = 0.000000; ui29.fFluxCapData[11] = 0.000000; ui29.fFluxCapData[12] = 0.000000; ui29.fFluxCapData[13] = 0.000000; ui29.fFluxCapData[14] = 0.000000; ui29.fFluxCapData[15] = 0.000000; ui29.fFluxCapData[16] = 0.000000; ui29.fFluxCapData[17] = 0.000000; ui29.fFluxCapData[18] = 0.000000; ui29.fFluxCapData[19] = 0.000000; ui29.fFluxCapData[20] = 0.000000; ui29.fFluxCapData[21] = 0.000000; ui29.fFluxCapData[22] = 0.000000; ui29.fFluxCapData[23] = 0.000000; ui29.fFluxCapData[24] = 0.000000; ui29.fFluxCapData[25] = 0.000000; ui29.fFluxCapData[26] = 0.000000; ui29.fFluxCapData[27] = 0.000000; ui29.fFluxCapData[28] = 0.000000; ui29.fFluxCapData[29] = 0.000000; ui29.fFluxCapData[30] = 0.000000; ui29.fFluxCapData[31] = 0.000000; ui29.fFluxCapData[32] = 0.000000; ui29.fFluxCapData[33] = 0.000000; ui29.fFluxCapData[34] = 0.000000; ui29.fFluxCapData[35] = 0.000000; ui29.fFluxCapData[36] = 0.000000; ui29.fFluxCapData[37] = 0.000000; ui29.fFluxCapData[38] = 0.000000; ui29.fFluxCapData[39] = 0.000000; ui29.fFluxCapData[40] = 0.000000; ui29.fFluxCapData[41] = 0.000000; ui29.fFluxCapData[42] = 0.000000; ui29.fFluxCapData[43] = 0.000000; ui29.fFluxCapData[44] = 0.000000; ui29.fFluxCapData[45] = 0.000000; ui29.fFluxCapData[46] = 0.000000; ui29.fFluxCapData[47] = 0.000000; ui29.fFluxCapData[48] = 0.000000; ui29.fFluxCapData[49] = 0.000000; ui29.fFluxCapData[50] = 0.000000; ui29.fFluxCapData[51] = 0.000000; ui29.fFluxCapData[52] = 0.000000; ui29.fFluxCapData[53] = 0.000000; ui29.fFluxCapData[54] = 0.000000; ui29.fFluxCapData[55] = 0.000000; ui29.fFluxCapData[56] = 0.000000; ui29.fFluxCapData[57] = 0.000000; ui29.fFluxCapData[58] = 0.000000; ui29.fFluxCapData[59] = 0.000000; ui29.fFluxCapData[60] = 0.000000; ui29.fFluxCapData[61] = 0.000000; ui29.fFluxCapData[62] = 0.000000; ui29.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui29);


	m_uFilterKeyTrack = 0;
	CUICtrl ui30;
	ui30.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui30.uControlId = 104;
	ui30.bLogSlider = false;
	ui30.bExpSlider = false;
	ui30.fUserDisplayDataLoLimit = 0.000000;
	ui30.fUserDisplayDataHiLimit = 1.000000;
	ui30.uUserDataType = UINTData;
	ui30.fInitUserIntValue = 0;
	ui30.fInitUserFloatValue = 0;
	ui30.fInitUserDoubleValue = 0;
	ui30.fInitUserUINTValue = 0.000000;
	ui30.m_pUserCookedIntData = NULL;
	ui30.m_pUserCookedFloatData = NULL;
	ui30.m_pUserCookedDoubleData = NULL;
	ui30.m_pUserCookedUINTData = &m_uFilterKeyTrack;
	ui30.cControlUnits = "                                                                ";
	ui30.cVariableName = "m_uFilterKeyTrack";
	ui30.cEnumeratedList = "OFF,ON";
	ui30.dPresetData[0] = 0.000000;ui30.dPresetData[1] = 0.000000;ui30.dPresetData[2] = 0.000000;ui30.dPresetData[3] = 0.000000;ui30.dPresetData[4] = 0.000000;ui30.dPresetData[5] = 0.000000;ui30.dPresetData[6] = 0.000000;ui30.dPresetData[7] = 0.000000;ui30.dPresetData[8] = 0.000000;ui30.dPresetData[9] = 0.000000;ui30.dPresetData[10] = 0.000000;ui30.dPresetData[11] = 0.000000;ui30.dPresetData[12] = 0.000000;ui30.dPresetData[13] = 0.000000;ui30.dPresetData[14] = 0.000000;ui30.dPresetData[15] = 0.000000;
	ui30.cControlName = "Filter KeyTrack";
	ui30.bOwnerControl = false;
	ui30.bMIDIControl = false;
	ui30.uMIDIControlCommand = 176;
	ui30.uMIDIControlName = 3;
	ui30.uMIDIControlChannel = 0;
	ui30.nGUIRow = -1;
	ui30.nGUIColumn = -1;
	ui30.uControlTheme[0] = 0; ui30.uControlTheme[1] = 0; ui30.uControlTheme[2] = 0; ui30.uControlTheme[3] = 0; ui30.uControlTheme[4] = 0; ui30.uControlTheme[5] = 0; ui30.uControlTheme[6] = 0; ui30.uControlTheme[7] = 0; ui30.uControlTheme[8] = 0; ui30.uControlTheme[9] = 0; ui30.uControlTheme[10] = 0; ui30.uControlTheme[11] = 0; ui30.uControlTheme[12] = 0; ui30.uControlTheme[13] = 0; ui30.uControlTheme[14] = 0; ui30.uControlTheme[15] = 0; ui30.uControlTheme[16] = 0; ui30.uControlTheme[17] = 0; ui30.uControlTheme[18] = 0; ui30.uControlTheme[19] = 0; ui30.uControlTheme[20] = 0; ui30.uControlTheme[21] = 0; ui30.uControlTheme[22] = 0; ui30.uControlTheme[23] = 0; ui30.uControlTheme[24] = 0; ui30.uControlTheme[25] = 0; ui30.uControlTheme[26] = 0; ui30.uControlTheme[27] = 0; ui30.uControlTheme[28] = 0; ui30.uControlTheme[29] = 0; ui30.uControlTheme[30] = 0; ui30.uControlTheme[31] = 0;
	ui30.uFluxCapControl[0] = 1; ui30.uFluxCapControl[1] = 4; ui30.uFluxCapControl[2] = 0; ui30.uFluxCapControl[3] = 0; ui30.uFluxCapControl[4] = 0; ui30.uFluxCapControl[5] = 0; ui30.uFluxCapControl[6] = 0; ui30.uFluxCapControl[7] = 0; ui30.uFluxCapControl[8] = 0; ui30.uFluxCapControl[9] = 0; ui30.uFluxCapControl[10] = 0; ui30.uFluxCapControl[11] = 0; ui30.uFluxCapControl[12] = 0; ui30.uFluxCapControl[13] = 0; ui30.uFluxCapControl[14] = 0; ui30.uFluxCapControl[15] = 0; ui30.uFluxCapControl[16] = 0; ui30.uFluxCapControl[17] = 0; ui30.uFluxCapControl[18] = 0; ui30.uFluxCapControl[19] = 0; ui30.uFluxCapControl[20] = 0; ui30.uFluxCapControl[21] = 0; ui30.uFluxCapControl[22] = 0; ui30.uFluxCapControl[23] = 0; ui30.uFluxCapControl[24] = 0; ui30.uFluxCapControl[25] = 0; ui30.uFluxCapControl[26] = 0; ui30.uFluxCapControl[27] = 0; ui30.uFluxCapControl[28] = 0; ui30.uFluxCapControl[29] = 0; ui30.uFluxCapControl[30] = 0; ui30.uFluxCapControl[31] = 0; ui30.uFluxCapControl[32] = 0; ui30.uFluxCapControl[33] = 0; ui30.uFluxCapControl[34] = 0; ui30.uFluxCapControl[35] = 0; ui30.uFluxCapControl[36] = 0; ui30.uFluxCapControl[37] = 0; ui30.uFluxCapControl[38] = 0; ui30.uFluxCapControl[39] = 0; ui30.uFluxCapControl[40] = 0; ui30.uFluxCapControl[41] = 0; ui30.uFluxCapControl[42] = 0; ui30.uFluxCapControl[43] = 0; ui30.uFluxCapControl[44] = 0; ui30.uFluxCapControl[45] = 0; ui30.uFluxCapControl[46] = 0; ui30.uFluxCapControl[47] = 0; ui30.uFluxCapControl[48] = 0; ui30.uFluxCapControl[49] = 0; ui30.uFluxCapControl[50] = 0; ui30.uFluxCapControl[51] = 0; ui30.uFluxCapControl[52] = 0; ui30.uFluxCapControl[53] = 0; ui30.uFluxCapControl[54] = 0; ui30.uFluxCapControl[55] = 0; ui30.uFluxCapControl[56] = 0; ui30.uFluxCapControl[57] = 0; ui30.uFluxCapControl[58] = 0; ui30.uFluxCapControl[59] = 0; ui30.uFluxCapControl[60] = 0; ui30.uFluxCapControl[61] = 0; ui30.uFluxCapControl[62] = 0; ui30.uFluxCapControl[63] = 0;
	ui30.fFluxCapData[0] = 0.000000; ui30.fFluxCapData[1] = 0.000000; ui30.fFluxCapData[2] = 0.000000; ui30.fFluxCapData[3] = 0.000000; ui30.fFluxCapData[4] = 0.000000; ui30.fFluxCapData[5] = 0.000000; ui30.fFluxCapData[6] = 0.000000; ui30.fFluxCapData[7] = 0.000000; ui30.fFluxCapData[8] = 0.000000; ui30.fFluxCapData[9] = 0.000000; ui30.fFluxCapData[10] = 0.000000; ui30.fFluxCapData[11] = 0.000000; ui30.fFluxCapData[12] = 0.000000; ui30.fFluxCapData[13] = 0.000000; ui30.fFluxCapData[14] = 0.000000; ui30.fFluxCapData[15] = 0.000000; ui30.fFluxCapData[16] = 0.000000; ui30.fFluxCapData[17] = 0.000000; ui30.fFluxCapData[18] = 0.000000; ui30.fFluxCapData[19] = 0.000000; ui30.fFluxCapData[20] = 0.000000; ui30.fFluxCapData[21] = 0.000000; ui30.fFluxCapData[22] = 0.000000; ui30.fFluxCapData[23] = 0.000000; ui30.fFluxCapData[24] = 0.000000; ui30.fFluxCapData[25] = 0.000000; ui30.fFluxCapData[26] = 0.000000; ui30.fFluxCapData[27] = 0.000000; ui30.fFluxCapData[28] = 0.000000; ui30.fFluxCapData[29] = 0.000000; ui30.fFluxCapData[30] = 0.000000; ui30.fFluxCapData[31] = 0.000000; ui30.fFluxCapData[32] = 0.000000; ui30.fFluxCapData[33] = 0.000000; ui30.fFluxCapData[34] = 0.000000; ui30.fFluxCapData[35] = 0.000000; ui30.fFluxCapData[36] = 0.000000; ui30.fFluxCapData[37] = 0.000000; ui30.fFluxCapData[38] = 0.000000; ui30.fFluxCapData[39] = 0.000000; ui30.fFluxCapData[40] = 0.000000; ui30.fFluxCapData[41] = 0.000000; ui30.fFluxCapData[42] = 0.000000; ui30.fFluxCapData[43] = 0.000000; ui30.fFluxCapData[44] = 0.000000; ui30.fFluxCapData[45] = 0.000000; ui30.fFluxCapData[46] = 0.000000; ui30.fFluxCapData[47] = 0.000000; ui30.fFluxCapData[48] = 0.000000; ui30.fFluxCapData[49] = 0.000000; ui30.fFluxCapData[50] = 0.000000; ui30.fFluxCapData[51] = 0.000000; ui30.fFluxCapData[52] = 0.000000; ui30.fFluxCapData[53] = 0.000000; ui30.fFluxCapData[54] = 0.000000; ui30.fFluxCapData[55] = 0.000000; ui30.fFluxCapData[56] = 0.000000; ui30.fFluxCapData[57] = 0.000000; ui30.fFluxCapData[58] = 0.000000; ui30.fFluxCapData[59] = 0.000000; ui30.fFluxCapData[60] = 0.000000; ui30.fFluxCapData[61] = 0.000000; ui30.fFluxCapData[62] = 0.000000; ui30.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui30);


	m_dFilterKeyTrackIntensity = 1.000000;
	CUICtrl ui31;
	ui31.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui31.uControlId = 105;
	ui31.bLogSlider = false;
	ui31.bExpSlider = false;
	ui31.fUserDisplayDataLoLimit = 0.500000;
	ui31.fUserDisplayDataHiLimit = 2.000000;
	ui31.uUserDataType = doubleData;
	ui31.fInitUserIntValue = 0;
	ui31.fInitUserFloatValue = 0;
	ui31.fInitUserDoubleValue = 1.000000;
	ui31.fInitUserUINTValue = 0;
	ui31.m_pUserCookedIntData = NULL;
	ui31.m_pUserCookedFloatData = NULL;
	ui31.m_pUserCookedDoubleData = &m_dFilterKeyTrackIntensity;
	ui31.m_pUserCookedUINTData = NULL;
	ui31.cControlUnits = "%                                                               ";
	ui31.cVariableName = "m_dFilterKeyTrackIntensity";
	ui31.cEnumeratedList = "SEL1,SEL2,SEL3";
	ui31.dPresetData[0] = 1.000000;ui31.dPresetData[1] = 0.000000;ui31.dPresetData[2] = 0.000000;ui31.dPresetData[3] = 0.000000;ui31.dPresetData[4] = 0.000000;ui31.dPresetData[5] = 0.000000;ui31.dPresetData[6] = 0.000000;ui31.dPresetData[7] = 0.000000;ui31.dPresetData[8] = 0.000000;ui31.dPresetData[9] = 0.000000;ui31.dPresetData[10] = 0.000000;ui31.dPresetData[11] = 0.000000;ui31.dPresetData[12] = 0.000000;ui31.dPresetData[13] = 0.000000;ui31.dPresetData[14] = 0.000000;ui31.dPresetData[15] = 0.000000;
	ui31.cControlName = "Filter KeyTrack Int";
	ui31.bOwnerControl = false;
	ui31.bMIDIControl = false;
	ui31.uMIDIControlCommand = 176;
	ui31.uMIDIControlName = 3;
	ui31.uMIDIControlChannel = 0;
	ui31.nGUIRow = -1;
	ui31.nGUIColumn = -1;
	ui31.uControlTheme[0] = 0; ui31.uControlTheme[1] = 0; ui31.uControlTheme[2] = 0; ui31.uControlTheme[3] = 0; ui31.uControlTheme[4] = 0; ui31.uControlTheme[5] = 0; ui31.uControlTheme[6] = 0; ui31.uControlTheme[7] = 0; ui31.uControlTheme[8] = 0; ui31.uControlTheme[9] = 0; ui31.uControlTheme[10] = 0; ui31.uControlTheme[11] = 0; ui31.uControlTheme[12] = 0; ui31.uControlTheme[13] = 0; ui31.uControlTheme[14] = 0; ui31.uControlTheme[15] = 0; ui31.uControlTheme[16] = 0; ui31.uControlTheme[17] = 0; ui31.uControlTheme[18] = 0; ui31.uControlTheme[19] = 0; ui31.uControlTheme[20] = 0; ui31.uControlTheme[21] = 0; ui31.uControlTheme[22] = 0; ui31.uControlTheme[23] = 0; ui31.uControlTheme[24] = 0; ui31.uControlTheme[25] = 0; ui31.uControlTheme[26] = 0; ui31.uControlTheme[27] = 0; ui31.uControlTheme[28] = 0; ui31.uControlTheme[29] = 0; ui31.uControlTheme[30] = 0; ui31.uControlTheme[31] = 0;
	ui31.uFluxCapControl[0] = 1; ui31.uFluxCapControl[1] = 5; ui31.uFluxCapControl[2] = 0; ui31.uFluxCapControl[3] = 0; ui31.uFluxCapControl[4] = 0; ui31.uFluxCapControl[5] = 0; ui31.uFluxCapControl[6] = 0; ui31.uFluxCapControl[7] = 0; ui31.uFluxCapControl[8] = 0; ui31.uFluxCapControl[9] = 0; ui31.uFluxCapControl[10] = 0; ui31.uFluxCapControl[11] = 0; ui31.uFluxCapControl[12] = 0; ui31.uFluxCapControl[13] = 0; ui31.uFluxCapControl[14] = 0; ui31.uFluxCapControl[15] = 0; ui31.uFluxCapControl[16] = 0; ui31.uFluxCapControl[17] = 0; ui31.uFluxCapControl[18] = 0; ui31.uFluxCapControl[19] = 0; ui31.uFluxCapControl[20] = 0; ui31.uFluxCapControl[21] = 0; ui31.uFluxCapControl[22] = 0; ui31.uFluxCapControl[23] = 0; ui31.uFluxCapControl[24] = 0; ui31.uFluxCapControl[25] = 0; ui31.uFluxCapControl[26] = 0; ui31.uFluxCapControl[27] = 0; ui31.uFluxCapControl[28] = 0; ui31.uFluxCapControl[29] = 0; ui31.uFluxCapControl[30] = 0; ui31.uFluxCapControl[31] = 0; ui31.uFluxCapControl[32] = 0; ui31.uFluxCapControl[33] = 0; ui31.uFluxCapControl[34] = 0; ui31.uFluxCapControl[35] = 0; ui31.uFluxCapControl[36] = 0; ui31.uFluxCapControl[37] = 0; ui31.uFluxCapControl[38] = 0; ui31.uFluxCapControl[39] = 0; ui31.uFluxCapControl[40] = 0; ui31.uFluxCapControl[41] = 0; ui31.uFluxCapControl[42] = 0; ui31.uFluxCapControl[43] = 0; ui31.uFluxCapControl[44] = 0; ui31.uFluxCapControl[45] = 0; ui31.uFluxCapControl[46] = 0; ui31.uFluxCapControl[47] = 0; ui31.uFluxCapControl[48] = 0; ui31.uFluxCapControl[49] = 0; ui31.uFluxCapControl[50] = 0; ui31.uFluxCapControl[51] = 0; ui31.uFluxCapControl[52] = 0; ui31.uFluxCapControl[53] = 0; ui31.uFluxCapControl[54] = 0; ui31.uFluxCapControl[55] = 0; ui31.uFluxCapControl[56] = 0; ui31.uFluxCapControl[57] = 0; ui31.uFluxCapControl[58] = 0; ui31.uFluxCapControl[59] = 0; ui31.uFluxCapControl[60] = 0; ui31.uFluxCapControl[61] = 0; ui31.uFluxCapControl[62] = 0; ui31.uFluxCapControl[63] = 0;
	ui31.fFluxCapData[0] = 0.000000; ui31.fFluxCapData[1] = 0.000000; ui31.fFluxCapData[2] = 0.000000; ui31.fFluxCapData[3] = 0.000000; ui31.fFluxCapData[4] = 0.000000; ui31.fFluxCapData[5] = 0.000000; ui31.fFluxCapData[6] = 0.000000; ui31.fFluxCapData[7] = 0.000000; ui31.fFluxCapData[8] = 0.000000; ui31.fFluxCapData[9] = 0.000000; ui31.fFluxCapData[10] = 0.000000; ui31.fFluxCapData[11] = 0.000000; ui31.fFluxCapData[12] = 0.000000; ui31.fFluxCapData[13] = 0.000000; ui31.fFluxCapData[14] = 0.000000; ui31.fFluxCapData[15] = 0.000000; ui31.fFluxCapData[16] = 0.000000; ui31.fFluxCapData[17] = 0.000000; ui31.fFluxCapData[18] = 0.000000; ui31.fFluxCapData[19] = 0.000000; ui31.fFluxCapData[20] = 0.000000; ui31.fFluxCapData[21] = 0.000000; ui31.fFluxCapData[22] = 0.000000; ui31.fFluxCapData[23] = 0.000000; ui31.fFluxCapData[24] = 0.000000; ui31.fFluxCapData[25] = 0.000000; ui31.fFluxCapData[26] = 0.000000; ui31.fFluxCapData[27] = 0.000000; ui31.fFluxCapData[28] = 0.000000; ui31.fFluxCapData[29] = 0.000000; ui31.fFluxCapData[30] = 0.000000; ui31.fFluxCapData[31] = 0.000000; ui31.fFluxCapData[32] = 0.000000; ui31.fFluxCapData[33] = 0.000000; ui31.fFluxCapData[34] = 0.000000; ui31.fFluxCapData[35] = 0.000000; ui31.fFluxCapData[36] = 0.000000; ui31.fFluxCapData[37] = 0.000000; ui31.fFluxCapData[38] = 0.000000; ui31.fFluxCapData[39] = 0.000000; ui31.fFluxCapData[40] = 0.000000; ui31.fFluxCapData[41] = 0.000000; ui31.fFluxCapData[42] = 0.000000; ui31.fFluxCapData[43] = 0.000000; ui31.fFluxCapData[44] = 0.000000; ui31.fFluxCapData[45] = 0.000000; ui31.fFluxCapData[46] = 0.000000; ui31.fFluxCapData[47] = 0.000000; ui31.fFluxCapData[48] = 0.000000; ui31.fFluxCapData[49] = 0.000000; ui31.fFluxCapData[50] = 0.000000; ui31.fFluxCapData[51] = 0.000000; ui31.fFluxCapData[52] = 0.000000; ui31.fFluxCapData[53] = 0.000000; ui31.fFluxCapData[54] = 0.000000; ui31.fFluxCapData[55] = 0.000000; ui31.fFluxCapData[56] = 0.000000; ui31.fFluxCapData[57] = 0.000000; ui31.fFluxCapData[58] = 0.000000; ui31.fFluxCapData[59] = 0.000000; ui31.fFluxCapData[60] = 0.000000; ui31.fFluxCapData[61] = 0.000000; ui31.fFluxCapData[62] = 0.000000; ui31.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui31);


	m_uVelocityToAttackScaling = 0;
	CUICtrl ui32;
	ui32.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui32.uControlId = 106;
	ui32.bLogSlider = false;
	ui32.bExpSlider = false;
	ui32.fUserDisplayDataLoLimit = 0.000000;
	ui32.fUserDisplayDataHiLimit = 1.000000;
	ui32.uUserDataType = UINTData;
	ui32.fInitUserIntValue = 0;
	ui32.fInitUserFloatValue = 0;
	ui32.fInitUserDoubleValue = 0;
	ui32.fInitUserUINTValue = 0.000000;
	ui32.m_pUserCookedIntData = NULL;
	ui32.m_pUserCookedFloatData = NULL;
	ui32.m_pUserCookedDoubleData = NULL;
	ui32.m_pUserCookedUINTData = &m_uVelocityToAttackScaling;
	ui32.cControlUnits = "                                                                ";
	ui32.cVariableName = "m_uVelocityToAttackScaling";
	ui32.cEnumeratedList = "OFF,ON";
	ui32.dPresetData[0] = 0.000000;ui32.dPresetData[1] = 0.000000;ui32.dPresetData[2] = 0.000000;ui32.dPresetData[3] = 0.000000;ui32.dPresetData[4] = 0.000000;ui32.dPresetData[5] = 0.000000;ui32.dPresetData[6] = 0.000000;ui32.dPresetData[7] = 0.000000;ui32.dPresetData[8] = 0.000000;ui32.dPresetData[9] = 0.000000;ui32.dPresetData[10] = 0.000000;ui32.dPresetData[11] = 0.000000;ui32.dPresetData[12] = 0.000000;ui32.dPresetData[13] = 0.000000;ui32.dPresetData[14] = 0.000000;ui32.dPresetData[15] = 0.000000;
	ui32.cControlName = "Vel->Att Scale";
	ui32.bOwnerControl = false;
	ui32.bMIDIControl = false;
	ui32.uMIDIControlCommand = 176;
	ui32.uMIDIControlName = 3;
	ui32.uMIDIControlChannel = 0;
	ui32.nGUIRow = -1;
	ui32.nGUIColumn = -1;
	ui32.uControlTheme[0] = 0; ui32.uControlTheme[1] = 0; ui32.uControlTheme[2] = 0; ui32.uControlTheme[3] = 0; ui32.uControlTheme[4] = 0; ui32.uControlTheme[5] = 0; ui32.uControlTheme[6] = 0; ui32.uControlTheme[7] = 0; ui32.uControlTheme[8] = 0; ui32.uControlTheme[9] = 0; ui32.uControlTheme[10] = 0; ui32.uControlTheme[11] = 0; ui32.uControlTheme[12] = 0; ui32.uControlTheme[13] = 0; ui32.uControlTheme[14] = 0; ui32.uControlTheme[15] = 0; ui32.uControlTheme[16] = 0; ui32.uControlTheme[17] = 0; ui32.uControlTheme[18] = 0; ui32.uControlTheme[19] = 0; ui32.uControlTheme[20] = 0; ui32.uControlTheme[21] = 0; ui32.uControlTheme[22] = 0; ui32.uControlTheme[23] = 0; ui32.uControlTheme[24] = 0; ui32.uControlTheme[25] = 0; ui32.uControlTheme[26] = 0; ui32.uControlTheme[27] = 0; ui32.uControlTheme[28] = 0; ui32.uControlTheme[29] = 0; ui32.uControlTheme[30] = 0; ui32.uControlTheme[31] = 0;
	ui32.uFluxCapControl[0] = 1; ui32.uFluxCapControl[1] = 6; ui32.uFluxCapControl[2] = 0; ui32.uFluxCapControl[3] = 0; ui32.uFluxCapControl[4] = 0; ui32.uFluxCapControl[5] = 0; ui32.uFluxCapControl[6] = 0; ui32.uFluxCapControl[7] = 0; ui32.uFluxCapControl[8] = 0; ui32.uFluxCapControl[9] = 0; ui32.uFluxCapControl[10] = 0; ui32.uFluxCapControl[11] = 0; ui32.uFluxCapControl[12] = 0; ui32.uFluxCapControl[13] = 0; ui32.uFluxCapControl[14] = 0; ui32.uFluxCapControl[15] = 0; ui32.uFluxCapControl[16] = 0; ui32.uFluxCapControl[17] = 0; ui32.uFluxCapControl[18] = 0; ui32.uFluxCapControl[19] = 0; ui32.uFluxCapControl[20] = 0; ui32.uFluxCapControl[21] = 0; ui32.uFluxCapControl[22] = 0; ui32.uFluxCapControl[23] = 0; ui32.uFluxCapControl[24] = 0; ui32.uFluxCapControl[25] = 0; ui32.uFluxCapControl[26] = 0; ui32.uFluxCapControl[27] = 0; ui32.uFluxCapControl[28] = 0; ui32.uFluxCapControl[29] = 0; ui32.uFluxCapControl[30] = 0; ui32.uFluxCapControl[31] = 0; ui32.uFluxCapControl[32] = 0; ui32.uFluxCapControl[33] = 0; ui32.uFluxCapControl[34] = 0; ui32.uFluxCapControl[35] = 0; ui32.uFluxCapControl[36] = 0; ui32.uFluxCapControl[37] = 0; ui32.uFluxCapControl[38] = 0; ui32.uFluxCapControl[39] = 0; ui32.uFluxCapControl[40] = 0; ui32.uFluxCapControl[41] = 0; ui32.uFluxCapControl[42] = 0; ui32.uFluxCapControl[43] = 0; ui32.uFluxCapControl[44] = 0; ui32.uFluxCapControl[45] = 0; ui32.uFluxCapControl[46] = 0; ui32.uFluxCapControl[47] = 0; ui32.uFluxCapControl[48] = 0; ui32.uFluxCapControl[49] = 0; ui32.uFluxCapControl[50] = 0; ui32.uFluxCapControl[51] = 0; ui32.uFluxCapControl[52] = 0; ui32.uFluxCapControl[53] = 0; ui32.uFluxCapControl[54] = 0; ui32.uFluxCapControl[55] = 0; ui32.uFluxCapControl[56] = 0; ui32.uFluxCapControl[57] = 0; ui32.uFluxCapControl[58] = 0; ui32.uFluxCapControl[59] = 0; ui32.uFluxCapControl[60] = 0; ui32.uFluxCapControl[61] = 0; ui32.uFluxCapControl[62] = 0; ui32.uFluxCapControl[63] = 0;
	ui32.fFluxCapData[0] = 0.000000; ui32.fFluxCapData[1] = 0.000000; ui32.fFluxCapData[2] = 0.000000; ui32.fFluxCapData[3] = 0.000000; ui32.fFluxCapData[4] = 0.000000; ui32.fFluxCapData[5] = 0.000000; ui32.fFluxCapData[6] = 0.000000; ui32.fFluxCapData[7] = 0.000000; ui32.fFluxCapData[8] = 0.000000; ui32.fFluxCapData[9] = 0.000000; ui32.fFluxCapData[10] = 0.000000; ui32.fFluxCapData[11] = 0.000000; ui32.fFluxCapData[12] = 0.000000; ui32.fFluxCapData[13] = 0.000000; ui32.fFluxCapData[14] = 0.000000; ui32.fFluxCapData[15] = 0.000000; ui32.fFluxCapData[16] = 0.000000; ui32.fFluxCapData[17] = 0.000000; ui32.fFluxCapData[18] = 0.000000; ui32.fFluxCapData[19] = 0.000000; ui32.fFluxCapData[20] = 0.000000; ui32.fFluxCapData[21] = 0.000000; ui32.fFluxCapData[22] = 0.000000; ui32.fFluxCapData[23] = 0.000000; ui32.fFluxCapData[24] = 0.000000; ui32.fFluxCapData[25] = 0.000000; ui32.fFluxCapData[26] = 0.000000; ui32.fFluxCapData[27] = 0.000000; ui32.fFluxCapData[28] = 0.000000; ui32.fFluxCapData[29] = 0.000000; ui32.fFluxCapData[30] = 0.000000; ui32.fFluxCapData[31] = 0.000000; ui32.fFluxCapData[32] = 0.000000; ui32.fFluxCapData[33] = 0.000000; ui32.fFluxCapData[34] = 0.000000; ui32.fFluxCapData[35] = 0.000000; ui32.fFluxCapData[36] = 0.000000; ui32.fFluxCapData[37] = 0.000000; ui32.fFluxCapData[38] = 0.000000; ui32.fFluxCapData[39] = 0.000000; ui32.fFluxCapData[40] = 0.000000; ui32.fFluxCapData[41] = 0.000000; ui32.fFluxCapData[42] = 0.000000; ui32.fFluxCapData[43] = 0.000000; ui32.fFluxCapData[44] = 0.000000; ui32.fFluxCapData[45] = 0.000000; ui32.fFluxCapData[46] = 0.000000; ui32.fFluxCapData[47] = 0.000000; ui32.fFluxCapData[48] = 0.000000; ui32.fFluxCapData[49] = 0.000000; ui32.fFluxCapData[50] = 0.000000; ui32.fFluxCapData[51] = 0.000000; ui32.fFluxCapData[52] = 0.000000; ui32.fFluxCapData[53] = 0.000000; ui32.fFluxCapData[54] = 0.000000; ui32.fFluxCapData[55] = 0.000000; ui32.fFluxCapData[56] = 0.000000; ui32.fFluxCapData[57] = 0.000000; ui32.fFluxCapData[58] = 0.000000; ui32.fFluxCapData[59] = 0.000000; ui32.fFluxCapData[60] = 0.000000; ui32.fFluxCapData[61] = 0.000000; ui32.fFluxCapData[62] = 0.000000; ui32.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui32);


	m_uNoteNumberToDecayScaling = 0;
	CUICtrl ui33;
	ui33.uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui33.uControlId = 107;
	ui33.bLogSlider = false;
	ui33.bExpSlider = false;
	ui33.fUserDisplayDataLoLimit = 0.000000;
	ui33.fUserDisplayDataHiLimit = 1.000000;
	ui33.uUserDataType = UINTData;
	ui33.fInitUserIntValue = 0;
	ui33.fInitUserFloatValue = 0;
	ui33.fInitUserDoubleValue = 0;
	ui33.fInitUserUINTValue = 0.000000;
	ui33.m_pUserCookedIntData = NULL;
	ui33.m_pUserCookedFloatData = NULL;
	ui33.m_pUserCookedDoubleData = NULL;
	ui33.m_pUserCookedUINTData = &m_uNoteNumberToDecayScaling;
	ui33.cControlUnits = "                                                                ";
	ui33.cVariableName = "m_uNoteNumberToDecayScaling";
	ui33.cEnumeratedList = "OFF,ON";
	ui33.dPresetData[0] = 0.000000;ui33.dPresetData[1] = 0.000000;ui33.dPresetData[2] = 0.000000;ui33.dPresetData[3] = 0.000000;ui33.dPresetData[4] = 0.000000;ui33.dPresetData[5] = 0.000000;ui33.dPresetData[6] = 0.000000;ui33.dPresetData[7] = 0.000000;ui33.dPresetData[8] = 0.000000;ui33.dPresetData[9] = 0.000000;ui33.dPresetData[10] = 0.000000;ui33.dPresetData[11] = 0.000000;ui33.dPresetData[12] = 0.000000;ui33.dPresetData[13] = 0.000000;ui33.dPresetData[14] = 0.000000;ui33.dPresetData[15] = 0.000000;
	ui33.cControlName = "Note->Dcy Scale";
	ui33.bOwnerControl = false;
	ui33.bMIDIControl = false;
	ui33.uMIDIControlCommand = 176;
	ui33.uMIDIControlName = 3;
	ui33.uMIDIControlChannel = 0;
	ui33.nGUIRow = -1;
	ui33.nGUIColumn = -1;
	ui33.uControlTheme[0] = 0; ui33.uControlTheme[1] = 0; ui33.uControlTheme[2] = 0; ui33.uControlTheme[3] = 0; ui33.uControlTheme[4] = 0; ui33.uControlTheme[5] = 0; ui33.uControlTheme[6] = 0; ui33.uControlTheme[7] = 0; ui33.uControlTheme[8] = 0; ui33.uControlTheme[9] = 0; ui33.uControlTheme[10] = 0; ui33.uControlTheme[11] = 0; ui33.uControlTheme[12] = 0; ui33.uControlTheme[13] = 0; ui33.uControlTheme[14] = 0; ui33.uControlTheme[15] = 0; ui33.uControlTheme[16] = 0; ui33.uControlTheme[17] = 0; ui33.uControlTheme[18] = 0; ui33.uControlTheme[19] = 0; ui33.uControlTheme[20] = 0; ui33.uControlTheme[21] = 0; ui33.uControlTheme[22] = 0; ui33.uControlTheme[23] = 0; ui33.uControlTheme[24] = 0; ui33.uControlTheme[25] = 0; ui33.uControlTheme[26] = 0; ui33.uControlTheme[27] = 0; ui33.uControlTheme[28] = 0; ui33.uControlTheme[29] = 0; ui33.uControlTheme[30] = 0; ui33.uControlTheme[31] = 0;
	ui33.uFluxCapControl[0] = 1; ui33.uFluxCapControl[1] = 7; ui33.uFluxCapControl[2] = 0; ui33.uFluxCapControl[3] = 0; ui33.uFluxCapControl[4] = 0; ui33.uFluxCapControl[5] = 0; ui33.uFluxCapControl[6] = 0; ui33.uFluxCapControl[7] = 0; ui33.uFluxCapControl[8] = 0; ui33.uFluxCapControl[9] = 0; ui33.uFluxCapControl[10] = 0; ui33.uFluxCapControl[11] = 0; ui33.uFluxCapControl[12] = 0; ui33.uFluxCapControl[13] = 0; ui33.uFluxCapControl[14] = 0; ui33.uFluxCapControl[15] = 0; ui33.uFluxCapControl[16] = 0; ui33.uFluxCapControl[17] = 0; ui33.uFluxCapControl[18] = 0; ui33.uFluxCapControl[19] = 0; ui33.uFluxCapControl[20] = 0; ui33.uFluxCapControl[21] = 0; ui33.uFluxCapControl[22] = 0; ui33.uFluxCapControl[23] = 0; ui33.uFluxCapControl[24] = 0; ui33.uFluxCapControl[25] = 0; ui33.uFluxCapControl[26] = 0; ui33.uFluxCapControl[27] = 0; ui33.uFluxCapControl[28] = 0; ui33.uFluxCapControl[29] = 0; ui33.uFluxCapControl[30] = 0; ui33.uFluxCapControl[31] = 0; ui33.uFluxCapControl[32] = 0; ui33.uFluxCapControl[33] = 0; ui33.uFluxCapControl[34] = 0; ui33.uFluxCapControl[35] = 0; ui33.uFluxCapControl[36] = 0; ui33.uFluxCapControl[37] = 0; ui33.uFluxCapControl[38] = 0; ui33.uFluxCapControl[39] = 0; ui33.uFluxCapControl[40] = 0; ui33.uFluxCapControl[41] = 0; ui33.uFluxCapControl[42] = 0; ui33.uFluxCapControl[43] = 0; ui33.uFluxCapControl[44] = 0; ui33.uFluxCapControl[45] = 0; ui33.uFluxCapControl[46] = 0; ui33.uFluxCapControl[47] = 0; ui33.uFluxCapControl[48] = 0; ui33.uFluxCapControl[49] = 0; ui33.uFluxCapControl[50] = 0; ui33.uFluxCapControl[51] = 0; ui33.uFluxCapControl[52] = 0; ui33.uFluxCapControl[53] = 0; ui33.uFluxCapControl[54] = 0; ui33.uFluxCapControl[55] = 0; ui33.uFluxCapControl[56] = 0; ui33.uFluxCapControl[57] = 0; ui33.uFluxCapControl[58] = 0; ui33.uFluxCapControl[59] = 0; ui33.uFluxCapControl[60] = 0; ui33.uFluxCapControl[61] = 0; ui33.uFluxCapControl[62] = 0; ui33.uFluxCapControl[63] = 0;
	ui33.fFluxCapData[0] = 0.000000; ui33.fFluxCapData[1] = 0.000000; ui33.fFluxCapData[2] = 0.000000; ui33.fFluxCapData[3] = 0.000000; ui33.fFluxCapData[4] = 0.000000; ui33.fFluxCapData[5] = 0.000000; ui33.fFluxCapData[6] = 0.000000; ui33.fFluxCapData[7] = 0.000000; ui33.fFluxCapData[8] = 0.000000; ui33.fFluxCapData[9] = 0.000000; ui33.fFluxCapData[10] = 0.000000; ui33.fFluxCapData[11] = 0.000000; ui33.fFluxCapData[12] = 0.000000; ui33.fFluxCapData[13] = 0.000000; ui33.fFluxCapData[14] = 0.000000; ui33.fFluxCapData[15] = 0.000000; ui33.fFluxCapData[16] = 0.000000; ui33.fFluxCapData[17] = 0.000000; ui33.fFluxCapData[18] = 0.000000; ui33.fFluxCapData[19] = 0.000000; ui33.fFluxCapData[20] = 0.000000; ui33.fFluxCapData[21] = 0.000000; ui33.fFluxCapData[22] = 0.000000; ui33.fFluxCapData[23] = 0.000000; ui33.fFluxCapData[24] = 0.000000; ui33.fFluxCapData[25] = 0.000000; ui33.fFluxCapData[26] = 0.000000; ui33.fFluxCapData[27] = 0.000000; ui33.fFluxCapData[28] = 0.000000; ui33.fFluxCapData[29] = 0.000000; ui33.fFluxCapData[30] = 0.000000; ui33.fFluxCapData[31] = 0.000000; ui33.fFluxCapData[32] = 0.000000; ui33.fFluxCapData[33] = 0.000000; ui33.fFluxCapData[34] = 0.000000; ui33.fFluxCapData[35] = 0.000000; ui33.fFluxCapData[36] = 0.000000; ui33.fFluxCapData[37] = 0.000000; ui33.fFluxCapData[38] = 0.000000; ui33.fFluxCapData[39] = 0.000000; ui33.fFluxCapData[40] = 0.000000; ui33.fFluxCapData[41] = 0.000000; ui33.fFluxCapData[42] = 0.000000; ui33.fFluxCapData[43] = 0.000000; ui33.fFluxCapData[44] = 0.000000; ui33.fFluxCapData[45] = 0.000000; ui33.fFluxCapData[46] = 0.000000; ui33.fFluxCapData[47] = 0.000000; ui33.fFluxCapData[48] = 0.000000; ui33.fFluxCapData[49] = 0.000000; ui33.fFluxCapData[50] = 0.000000; ui33.fFluxCapData[51] = 0.000000; ui33.fFluxCapData[52] = 0.000000; ui33.fFluxCapData[53] = 0.000000; ui33.fFluxCapData[54] = 0.000000; ui33.fFluxCapData[55] = 0.000000; ui33.fFluxCapData[56] = 0.000000; ui33.fFluxCapData[57] = 0.000000; ui33.fFluxCapData[58] = 0.000000; ui33.fFluxCapData[59] = 0.000000; ui33.fFluxCapData[60] = 0.000000; ui33.fFluxCapData[61] = 0.000000; ui33.fFluxCapData[62] = 0.000000; ui33.fFluxCapData[63] = 0.000000;
	m_UIControlList.append(ui33);


	m_uX_TrackPadIndex = -1;
	m_uY_TrackPadIndex = -1;

	m_AssignButton1Name = "B1";
	m_AssignButton2Name = "B2";
	m_AssignButton3Name = "B3";

	m_bLatchingAssignButton1 = false;
	m_bLatchingAssignButton2 = false;
	m_bLatchingAssignButton3 = false;

	m_nGUIType = -1;
	m_nGUIThemeID = -1;
	m_bUseCustomVSTGUI = false;

	m_uControlTheme[0] = 0; m_uControlTheme[1] = 0; m_uControlTheme[2] = 0; m_uControlTheme[3] = 0; m_uControlTheme[4] = 0; m_uControlTheme[5] = 0; m_uControlTheme[6] = 0; m_uControlTheme[7] = 0; m_uControlTheme[8] = 0; m_uControlTheme[9] = 0; m_uControlTheme[10] = 0; m_uControlTheme[11] = 0; m_uControlTheme[12] = 0; m_uControlTheme[13] = 0; m_uControlTheme[14] = 0; m_uControlTheme[15] = 0; m_uControlTheme[16] = 0; m_uControlTheme[17] = 0; m_uControlTheme[18] = 0; m_uControlTheme[19] = 0; m_uControlTheme[20] = 0; m_uControlTheme[21] = 0; m_uControlTheme[22] = 0; m_uControlTheme[23] = 0; m_uControlTheme[24] = 0; m_uControlTheme[25] = 0; m_uControlTheme[26] = 0; m_uControlTheme[27] = 0; m_uControlTheme[28] = 0; m_uControlTheme[29] = 0; m_uControlTheme[30] = 0; m_uControlTheme[31] = 0; m_uControlTheme[32] = 0; m_uControlTheme[33] = 0; m_uControlTheme[34] = 0; m_uControlTheme[35] = 0; m_uControlTheme[36] = 0; m_uControlTheme[37] = 0; m_uControlTheme[38] = 0; m_uControlTheme[39] = 0; m_uControlTheme[40] = 0; m_uControlTheme[41] = 0; m_uControlTheme[42] = 0; m_uControlTheme[43] = 0; m_uControlTheme[44] = 0; m_uControlTheme[45] = 0; m_uControlTheme[46] = 0; m_uControlTheme[47] = 0; m_uControlTheme[48] = 0; m_uControlTheme[49] = 0; m_uControlTheme[50] = 0; m_uControlTheme[51] = 0; m_uControlTheme[52] = 0; m_uControlTheme[53] = 0; m_uControlTheme[54] = 0; m_uControlTheme[55] = 0; m_uControlTheme[56] = 0; m_uControlTheme[57] = 0; m_uControlTheme[58] = 0; m_uControlTheme[59] = 0; m_uControlTheme[60] = 0; m_uControlTheme[61] = 0; m_uControlTheme[62] = 0; m_uControlTheme[63] = 0;

	m_uPlugInEx[0] = 0; m_uPlugInEx[1] = 0; m_uPlugInEx[2] = 0; m_uPlugInEx[3] = 0; m_uPlugInEx[4] = 0; m_uPlugInEx[5] = 0; m_uPlugInEx[6] = 0; m_uPlugInEx[7] = 0; m_uPlugInEx[8] = 0; m_uPlugInEx[9] = 0; m_uPlugInEx[10] = 0; m_uPlugInEx[11] = 0; m_uPlugInEx[12] = 0; m_uPlugInEx[13] = 0; m_uPlugInEx[14] = 0; m_uPlugInEx[15] = 0; m_uPlugInEx[16] = 0; m_uPlugInEx[17] = 0; m_uPlugInEx[18] = 0; m_uPlugInEx[19] = 0; m_uPlugInEx[20] = 0; m_uPlugInEx[21] = 0; m_uPlugInEx[22] = 0; m_uPlugInEx[23] = 0; m_uPlugInEx[24] = 0; m_uPlugInEx[25] = 0; m_uPlugInEx[26] = 0; m_uPlugInEx[27] = 0; m_uPlugInEx[28] = 0; m_uPlugInEx[29] = 0; m_uPlugInEx[30] = 0; m_uPlugInEx[31] = 0; m_uPlugInEx[32] = 0; m_uPlugInEx[33] = 0; m_uPlugInEx[34] = 0; m_uPlugInEx[35] = 0; m_uPlugInEx[36] = 0; m_uPlugInEx[37] = 0; m_uPlugInEx[38] = 0; m_uPlugInEx[39] = 0; m_uPlugInEx[40] = 0; m_uPlugInEx[41] = 0; m_uPlugInEx[42] = 0; m_uPlugInEx[43] = 0; m_uPlugInEx[44] = 0; m_uPlugInEx[45] = 0; m_uPlugInEx[46] = 0; m_uPlugInEx[47] = 0; m_uPlugInEx[48] = 0; m_uPlugInEx[49] = 0; m_uPlugInEx[50] = 0; m_uPlugInEx[51] = 0; m_uPlugInEx[52] = 0; m_uPlugInEx[53] = 0; m_uPlugInEx[54] = 0; m_uPlugInEx[55] = 0; m_uPlugInEx[56] = 0; m_uPlugInEx[57] = 0; m_uPlugInEx[58] = 0; m_uPlugInEx[59] = 0; m_uPlugInEx[60] = 0; m_uPlugInEx[61] = 0; m_uPlugInEx[62] = 0; m_uPlugInEx[63] = 0;
	m_fPlugInEx[0] = 0.000000; m_fPlugInEx[1] = 0.000000; m_fPlugInEx[2] = 0.000000; m_fPlugInEx[3] = 0.000000; m_fPlugInEx[4] = 0.000000; m_fPlugInEx[5] = 0.000000; m_fPlugInEx[6] = 0.000000; m_fPlugInEx[7] = 0.000000; m_fPlugInEx[8] = 0.000000; m_fPlugInEx[9] = 0.000000; m_fPlugInEx[10] = 0.000000; m_fPlugInEx[11] = 0.000000; m_fPlugInEx[12] = 0.000000; m_fPlugInEx[13] = 0.000000; m_fPlugInEx[14] = 0.000000; m_fPlugInEx[15] = 0.000000; m_fPlugInEx[16] = 0.000000; m_fPlugInEx[17] = 0.000000; m_fPlugInEx[18] = 0.000000; m_fPlugInEx[19] = 0.000000; m_fPlugInEx[20] = 0.000000; m_fPlugInEx[21] = 0.000000; m_fPlugInEx[22] = 0.000000; m_fPlugInEx[23] = 0.000000; m_fPlugInEx[24] = 0.000000; m_fPlugInEx[25] = 0.000000; m_fPlugInEx[26] = 0.000000; m_fPlugInEx[27] = 0.000000; m_fPlugInEx[28] = 0.000000; m_fPlugInEx[29] = 0.000000; m_fPlugInEx[30] = 0.000000; m_fPlugInEx[31] = 0.000000; m_fPlugInEx[32] = 0.000000; m_fPlugInEx[33] = 0.000000; m_fPlugInEx[34] = 0.000000; m_fPlugInEx[35] = 0.000000; m_fPlugInEx[36] = 0.000000; m_fPlugInEx[37] = 0.000000; m_fPlugInEx[38] = 0.000000; m_fPlugInEx[39] = 0.000000; m_fPlugInEx[40] = 0.000000; m_fPlugInEx[41] = 0.000000; m_fPlugInEx[42] = 0.000000; m_fPlugInEx[43] = 0.000000; m_fPlugInEx[44] = 0.000000; m_fPlugInEx[45] = 0.000000; m_fPlugInEx[46] = 0.000000; m_fPlugInEx[47] = 0.000000; m_fPlugInEx[48] = 0.000000; m_fPlugInEx[49] = 0.000000; m_fPlugInEx[50] = 0.000000; m_fPlugInEx[51] = 0.000000; m_fPlugInEx[52] = 0.000000; m_fPlugInEx[53] = 0.000000; m_fPlugInEx[54] = 0.000000; m_fPlugInEx[55] = 0.000000; m_fPlugInEx[56] = 0.000000; m_fPlugInEx[57] = 0.000000; m_fPlugInEx[58] = 0.000000; m_fPlugInEx[59] = 0.000000; m_fPlugInEx[60] = 0.000000; m_fPlugInEx[61] = 0.000000; m_fPlugInEx[62] = 0.000000; m_fPlugInEx[63] = 0.000000;

	m_TextLabels[0] = ""; m_TextLabels[1] = ""; m_TextLabels[2] = ""; m_TextLabels[3] = ""; m_TextLabels[4] = ""; m_TextLabels[5] = ""; m_TextLabels[6] = ""; m_TextLabels[7] = ""; m_TextLabels[8] = ""; m_TextLabels[9] = ""; m_TextLabels[10] = ""; m_TextLabels[11] = ""; m_TextLabels[12] = ""; m_TextLabels[13] = ""; m_TextLabels[14] = ""; m_TextLabels[15] = ""; m_TextLabels[16] = ""; m_TextLabels[17] = ""; m_TextLabels[18] = ""; m_TextLabels[19] = ""; m_TextLabels[20] = ""; m_TextLabels[21] = ""; m_TextLabels[22] = ""; m_TextLabels[23] = ""; m_TextLabels[24] = ""; m_TextLabels[25] = ""; m_TextLabels[26] = ""; m_TextLabels[27] = ""; m_TextLabels[28] = ""; m_TextLabels[29] = ""; m_TextLabels[30] = ""; m_TextLabels[31] = ""; m_TextLabels[32] = ""; m_TextLabels[33] = ""; m_TextLabels[34] = ""; m_TextLabels[35] = ""; m_TextLabels[36] = ""; m_TextLabels[37] = ""; m_TextLabels[38] = ""; m_TextLabels[39] = ""; m_TextLabels[40] = ""; m_TextLabels[41] = ""; m_TextLabels[42] = ""; m_TextLabels[43] = ""; m_TextLabels[44] = ""; m_TextLabels[45] = ""; m_TextLabels[46] = ""; m_TextLabels[47] = ""; m_TextLabels[48] = ""; m_TextLabels[49] = ""; m_TextLabels[50] = ""; m_TextLabels[51] = ""; m_TextLabels[52] = ""; m_TextLabels[53] = ""; m_TextLabels[54] = ""; m_TextLabels[55] = ""; m_TextLabels[56] = ""; m_TextLabels[57] = ""; m_TextLabels[58] = ""; m_TextLabels[59] = ""; m_TextLabels[60] = ""; m_TextLabels[61] = ""; m_TextLabels[62] = ""; m_TextLabels[63] = "";

	m_uLabelCX[0] = 0; m_uLabelCX[1] = 0; m_uLabelCX[2] = 0; m_uLabelCX[3] = 0; m_uLabelCX[4] = 0; m_uLabelCX[5] = 0; m_uLabelCX[6] = 0; m_uLabelCX[7] = 0; m_uLabelCX[8] = 0; m_uLabelCX[9] = 0; m_uLabelCX[10] = 0; m_uLabelCX[11] = 0; m_uLabelCX[12] = 0; m_uLabelCX[13] = 0; m_uLabelCX[14] = 0; m_uLabelCX[15] = 0; m_uLabelCX[16] = 0; m_uLabelCX[17] = 0; m_uLabelCX[18] = 0; m_uLabelCX[19] = 0; m_uLabelCX[20] = 0; m_uLabelCX[21] = 0; m_uLabelCX[22] = 0; m_uLabelCX[23] = 0; m_uLabelCX[24] = 0; m_uLabelCX[25] = 0; m_uLabelCX[26] = 0; m_uLabelCX[27] = 0; m_uLabelCX[28] = 0; m_uLabelCX[29] = 0; m_uLabelCX[30] = 0; m_uLabelCX[31] = 0; m_uLabelCX[32] = 0; m_uLabelCX[33] = 0; m_uLabelCX[34] = 0; m_uLabelCX[35] = 0; m_uLabelCX[36] = 0; m_uLabelCX[37] = 0; m_uLabelCX[38] = 0; m_uLabelCX[39] = 0; m_uLabelCX[40] = 0; m_uLabelCX[41] = 0; m_uLabelCX[42] = 0; m_uLabelCX[43] = 0; m_uLabelCX[44] = 0; m_uLabelCX[45] = 0; m_uLabelCX[46] = 0; m_uLabelCX[47] = 0; m_uLabelCX[48] = 0; m_uLabelCX[49] = 0; m_uLabelCX[50] = 0; m_uLabelCX[51] = 0; m_uLabelCX[52] = 0; m_uLabelCX[53] = 0; m_uLabelCX[54] = 0; m_uLabelCX[55] = 0; m_uLabelCX[56] = 0; m_uLabelCX[57] = 0; m_uLabelCX[58] = 0; m_uLabelCX[59] = 0; m_uLabelCX[60] = 0; m_uLabelCX[61] = 0; m_uLabelCX[62] = 0; m_uLabelCX[63] = 0;
	m_uLabelCY[0] = 0; m_uLabelCY[1] = 0; m_uLabelCY[2] = 0; m_uLabelCY[3] = 0; m_uLabelCY[4] = 0; m_uLabelCY[5] = 0; m_uLabelCY[6] = 0; m_uLabelCY[7] = 0; m_uLabelCY[8] = 0; m_uLabelCY[9] = 0; m_uLabelCY[10] = 0; m_uLabelCY[11] = 0; m_uLabelCY[12] = 0; m_uLabelCY[13] = 0; m_uLabelCY[14] = 0; m_uLabelCY[15] = 0; m_uLabelCY[16] = 0; m_uLabelCY[17] = 0; m_uLabelCY[18] = 0; m_uLabelCY[19] = 0; m_uLabelCY[20] = 0; m_uLabelCY[21] = 0; m_uLabelCY[22] = 0; m_uLabelCY[23] = 0; m_uLabelCY[24] = 0; m_uLabelCY[25] = 0; m_uLabelCY[26] = 0; m_uLabelCY[27] = 0; m_uLabelCY[28] = 0; m_uLabelCY[29] = 0; m_uLabelCY[30] = 0; m_uLabelCY[31] = 0; m_uLabelCY[32] = 0; m_uLabelCY[33] = 0; m_uLabelCY[34] = 0; m_uLabelCY[35] = 0; m_uLabelCY[36] = 0; m_uLabelCY[37] = 0; m_uLabelCY[38] = 0; m_uLabelCY[39] = 0; m_uLabelCY[40] = 0; m_uLabelCY[41] = 0; m_uLabelCY[42] = 0; m_uLabelCY[43] = 0; m_uLabelCY[44] = 0; m_uLabelCY[45] = 0; m_uLabelCY[46] = 0; m_uLabelCY[47] = 0; m_uLabelCY[48] = 0; m_uLabelCY[49] = 0; m_uLabelCY[50] = 0; m_uLabelCY[51] = 0; m_uLabelCY[52] = 0; m_uLabelCY[53] = 0; m_uLabelCY[54] = 0; m_uLabelCY[55] = 0; m_uLabelCY[56] = 0; m_uLabelCY[57] = 0; m_uLabelCY[58] = 0; m_uLabelCY[59] = 0; m_uLabelCY[60] = 0; m_uLabelCY[61] = 0; m_uLabelCY[62] = 0; m_uLabelCY[63] = 0;

	m_pVectorJSProgram[JS_PROG_INDEX(0,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,6)] = 0.0000;


	m_JS_XCtrl.cControlName = "MIDI JS X";
	m_JS_XCtrl.uControlId = 0;
	m_JS_XCtrl.bMIDIControl = false;
	m_JS_XCtrl.uMIDIControlCommand = 176;
	m_JS_XCtrl.uMIDIControlName = 16;
	m_JS_XCtrl.uMIDIControlChannel = 0;


	m_JS_YCtrl.cControlName = "MIDI JS Y";
	m_JS_YCtrl.uControlId = 0;
	m_JS_YCtrl.bMIDIControl = false;
	m_JS_YCtrl.uMIDIControlCommand = 176;
	m_JS_YCtrl.uMIDIControlName = 17;
	m_JS_YCtrl.uMIDIControlChannel = 0;


	float* pJSProg = NULL;
	m_PresetNames[0] = "Factory Preset";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[0] = pJSProg;

	m_PresetNames[1] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[1] = pJSProg;

	m_PresetNames[2] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[2] = pJSProg;

	m_PresetNames[3] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[3] = pJSProg;

	m_PresetNames[4] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[4] = pJSProg;

	m_PresetNames[5] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[5] = pJSProg;

	m_PresetNames[6] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[6] = pJSProg;

	m_PresetNames[7] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[7] = pJSProg;

	m_PresetNames[8] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[8] = pJSProg;

	m_PresetNames[9] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[9] = pJSProg;

	m_PresetNames[10] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[10] = pJSProg;

	m_PresetNames[11] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[11] = pJSProg;

	m_PresetNames[12] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[12] = pJSProg;

	m_PresetNames[13] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[13] = pJSProg;

	m_PresetNames[14] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[14] = pJSProg;

	m_PresetNames[15] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[15] = pJSProg;

	// Additional Preset Support (avanced)


	// **--0xEDA5--**
// ------------------------------------------------------------------------------- //

	return true;

}

































