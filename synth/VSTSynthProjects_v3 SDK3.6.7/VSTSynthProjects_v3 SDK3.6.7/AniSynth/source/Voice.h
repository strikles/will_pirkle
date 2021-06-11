#pragma once
#include "synthfunctions.h"

#include "DCA.h"
#include "Oscillator.h"
#include "LFO.h"
#include "EnvelopeGenerator.h"
#include "Filter.h"
#include "ModulationMatrix.h"

class CVoice
{
public:
	CVoice(void);
	virtual ~CVoice(void);

	// NOTE: public; this is shared by sources and destinations
	CModulationMatrix m_ModulationMatrix;

protected:
	globalVoiceParams* m_pGlobalVoiceParams;
	globalSynthParams* m_pGlobalSynthParams;

	double m_dSampleRate;

	// this will be different for the voices
	UINT m_uVoiceMode;
	
	// hard sync is property of VOICE!
	double m_dHSRatio;
	
	// legato on/off
	UINT m_uLegatoMode;
	enum {mono,legato};

	// current pitch
	double m_dOscPitch;

	// pitch pending from a note-steal operation
	double m_dOscPitchPending;

	// for portamento: Starting Pitch
	double m_dPortamentoTime_mSec;
	double m_dPortamentoStart;

	// a counter/inc pair for the timebase
	double m_dModuloPortamento;
	double m_dPortamentoInc;

	// the amount of portamento in semitones
	double m_dPortamentoSemitones;

	// flag that a new note is pending (steal mode)
	bool m_bNotePending;
	
	// enum for EG state variable so we can query the EG	
	enum {off, attack, decay, sustain, release, shutdown};
	
	// for LFOs
	enum {sine,usaw,dsaw,tri,square,expo,rsh,qrsh};

	// for PITCHED Oscillators
	enum {SINE,SAW1,SAW2,SAW3,TRI,SQUARE,NOISE,PNOISE};
	
	// for NLP and other ON/OFF switches
	enum {OFF,ON}; 

	// for EG mode
	enum {analog, digital};
	
	// 4 oscillators
	COscillator* m_pOsc1;
	COscillator* m_pOsc2;
	COscillator* m_pOsc3;
	COscillator* m_pOsc4;

	// 2 filters; 2 mono or 1 stereo
	CFilter* m_pFilter1;
	CFilter* m_pFilter2;

	// 4 EGs
	CEnvelopeGenerator m_EG1;
	CEnvelopeGenerator m_EG2;
	CEnvelopeGenerator m_EG3;
	CEnvelopeGenerator m_EG4;
	
	// 2 LFOs	
	CLFO m_LFO1;
	CLFO m_LFO2;

	// 1 DCA (can be mono or stereo)
	CDCA m_DCA;

	// --- default mod matrix intensities
	double m_dDefaultModIntensity;	// always 1.0
	double m_dDefaultModRange;		// 1.0

public:
	inline virtual void initGlobalParameters(globalSynthParams* pGlobalParams)
	{
		// --- save Global Param Ptr
		m_pGlobalSynthParams = pGlobalParams;

		// --- Voice params
		m_pGlobalVoiceParams = &pGlobalParams->voiceParams;
		m_pGlobalVoiceParams->uVoiceMode = m_uVoiceMode;
		m_pGlobalVoiceParams->dHSRatio = m_dHSRatio;
		m_pGlobalVoiceParams->dPortamentoTime_mSec = m_dPortamentoTime_mSec;

		// --- Range Variables
		m_pGlobalVoiceParams->dOscFoPitchBendModRange = OSC_PITCHBEND_MOD_RANGE;
		m_pGlobalVoiceParams->dOscFoModRange = OSC_FO_MOD_RANGE;
		m_pGlobalVoiceParams->dOscHardSyncModRange = OSC_HARD_SYNC_RATIO_RANGE;
		m_pGlobalVoiceParams->dFilterModRange = FILTER_FC_MOD_RANGE;
		m_pGlobalVoiceParams->dAmpModRange = AMP_MOD_RANGE;

		// --- Intensity variables
		m_pGlobalVoiceParams->dFilterKeyTrackIntensity = 1.0;

		// --- init sub-components
		if(m_pOsc1)m_pOsc1->initGlobalParameters(&m_pGlobalSynthParams->osc1Params);
		if(m_pOsc2)m_pOsc2->initGlobalParameters(&m_pGlobalSynthParams->osc2Params);
		if(m_pOsc3)m_pOsc3->initGlobalParameters(&m_pGlobalSynthParams->osc3Params);
		if(m_pOsc4)m_pOsc4->initGlobalParameters(&m_pGlobalSynthParams->osc4Params);

		if(m_pFilter1)m_pFilter1->initGlobalParameters(&m_pGlobalSynthParams->filter1Params);
		if(m_pFilter2)m_pFilter2->initGlobalParameters(&m_pGlobalSynthParams->filter2Params);

		m_EG1.initGlobalParameters(&m_pGlobalSynthParams->eg1Params);
		m_EG2.initGlobalParameters(&m_pGlobalSynthParams->eg2Params);
		m_EG3.initGlobalParameters(&m_pGlobalSynthParams->eg3Params);
		m_EG4.initGlobalParameters(&m_pGlobalSynthParams->eg4Params);

		m_LFO1.initGlobalParameters(&m_pGlobalSynthParams->lfo1Params);
		m_LFO2.initGlobalParameters(&m_pGlobalSynthParams->lfo2Params);

		m_DCA.initGlobalParameters(&m_pGlobalSynthParams->dcaParams);
	}

	// --- our state; may be different from Oscillator states
	bool m_bNoteOn;
	UINT m_uTimeStamp;

	// --- MIDI stuff
	UINT m_uMIDINoteNumber;
	UINT m_uMIDINoteNumberPending;
	UINT m_uMIDIVelocity;
	UINT m_uMIDIVelocityPending;

	// DX uses 0->99 in a dB fashion which corresponds to DX controls
	/*inline void setDXOpAmplitude(UINT uOperator, double dDXLevel)
	{
		// if level = 0, leave it otherwise convert
		if(dDXLevel != 0.0)
		{
			// empirically found using a DX100
			dDXLevel -= 99.0;
			dDXLevel /= 1.32;

			// set dB-ish version
			dDXLevel = (pow(10.0, dDXLevel/20.0));
		}
		
		// call normal method
		setOscAmplitude(uOperator, dDXLevel);
	}*/

	// --- Modulation Matrix Sources
	//
	// --- set the core
	void setModMatrixCore(modMatrixRow** pModMatrix){m_ModulationMatrix.setModMatrixCore(pModMatrix);}
	
	// --- initialize a ModMatrix (voice specific)
	virtual void initializeModMatrix(CModulationMatrix* pMatrix);

	// WILL need to override this for FM Instrument or any parallel-osc algorithm with multiple output amp EGs
	inline virtual bool isActiveVoice()
	{	
		if(m_bNoteOn && m_EG1.isActive()) 
			return true;

		return false;
	}
	
	// we are in a state to accept a noteOff message
	// WILL need to override this for FM Instrument or any parallel-osc algorithm with multiple output amp EGs
	inline virtual bool canNoteOff()
	{	
		if(m_bNoteOn && m_EG1.canNoteOff())
			return true;

		return false;
	}
	
	// WILL need to override this for FM Instrument or any parallel-osc algorithm with multiple output amp EGs
	// this will be called to turn off m_bNoteOn so don't use it in this logic
	inline virtual bool isVoiceDone()
	{	
		if(m_EG1.getState() == OFF)
			return true;

		return false;
	}
	
	// MAY need to override this for FM Instrument or any parallel-osc algorithm with multiple output amp EGs
	inline virtual bool inLegatoMode()
	{	
		return m_EG1.m_bLegatoMode;
	}

	// --- overrides
	virtual void setSampleRate(double dSampleRate);
	virtual void prepareForPlay();
	virtual void update();
	virtual void reset();
	
	// --- MIDI methods
	inline void noteOn(UINT uMIDINote, UINT uMIDIVelocity, double dFrequency, double dLastNoteFrequency)
	{
		// save
		m_dOscPitch = dFrequency;

		// is our voice avaialble?
		if(!m_bNoteOn && !m_bNotePending)
		{
			// save the note number (for voice steal later)
			m_uMIDINoteNumber = uMIDINote;
			
			// the DCA needs this
			m_uMIDIVelocity = uMIDIVelocity;
			
			// set the velocity info 
			m_ModulationMatrix.m_dSources[SOURCE_VELOCITY] = (double)m_uMIDIVelocity;

			// set note number
			m_ModulationMatrix.m_dSources[SOURCE_MIDI_NOTE_NUM] = (double)uMIDINote;

			// update (globals)
			update();

			// set the OSC
			if(m_dPortamentoInc > 0.0 && dLastNoteFrequency > 0)
			{	
				m_dModuloPortamento = 0.0;
				m_dPortamentoSemitones = semitonesBetweenFrequencies(dLastNoteFrequency, dFrequency);
				m_dPortamentoStart = dLastNoteFrequency;
				
				if(m_pOsc1)m_pOsc1->m_dOscFo = m_dPortamentoStart;
				if(m_pOsc2)m_pOsc2->m_dOscFo = m_dPortamentoStart;
				if(m_pOsc3)m_pOsc3->m_dOscFo = m_dPortamentoStart;
				if(m_pOsc4)m_pOsc4->m_dOscFo = m_dPortamentoStart;
			}
			else
			{
				if(m_pOsc1)m_pOsc1->m_dOscFo = m_dOscPitch;
				if(m_pOsc2)m_pOsc2->m_dOscFo = m_dOscPitch;
				if(m_pOsc3)m_pOsc3->m_dOscFo = m_dOscPitch;
				if(m_pOsc4)m_pOsc4->m_dOscFo = m_dOscPitch;
			}

			// set MIDI note number (needed for Sample based osc)
			if(m_pOsc1)m_pOsc1->m_uMIDINoteNumber = m_uMIDINoteNumber;
			if(m_pOsc2)m_pOsc2->m_uMIDINoteNumber = m_uMIDINoteNumber;
			if(m_pOsc3)m_pOsc3->m_uMIDINoteNumber = m_uMIDINoteNumber;
			if(m_pOsc4)m_pOsc4->m_uMIDINoteNumber = m_uMIDINoteNumber;

			// start; NOTE this will reset and update()
			if(m_pOsc1)m_pOsc1->startOscillator();
			if(m_pOsc2)m_pOsc2->startOscillator();
			if(m_pOsc3)m_pOsc3->startOscillator();
			if(m_pOsc4)m_pOsc4->startOscillator();

			// start EGs
			m_EG1.startEG();
			m_EG2.startEG();
			m_EG3.startEG();
			m_EG4.startEG();

			// start LFOs
			m_LFO1.startOscillator();
			m_LFO2.startOscillator();

			// we are rendering!
			m_bNoteOn = true;
			m_uTimeStamp = 0; // start

			// bail
			return;
		}

		// IF we get here, we are playing a note and need to steal it
		// Save PENDING note number and velocity
		if(m_bNotePending && m_uMIDINoteNumberPending == uMIDINote)
			return;

		m_uMIDINoteNumberPending = uMIDINote;
			
		m_dOscPitchPending = dFrequency;
		m_uMIDIVelocityPending = uMIDIVelocity;
	
		// set the flag that we have a note pending
		m_bNotePending = true;

		// set portamento stuff
		if(m_dPortamentoInc > 0.0 && dLastNoteFrequency > 0)
		{
			if(m_dModuloPortamento > 0.0)
			{
				double dPortamentoPitchMult = pitchShiftMultiplier(m_dModuloPortamento*m_dPortamentoSemitones);
				m_dPortamentoStart = m_dPortamentoStart*dPortamentoPitchMult;
			}
			else
			{	
				m_dPortamentoStart = dLastNoteFrequency;
			}

			m_dModuloPortamento = 0.0;
			m_dPortamentoSemitones = semitonesBetweenFrequencies(m_dPortamentoStart, dFrequency);
		}

		// and shutdown the EGs
		m_EG1.shutDown();
		m_EG2.shutDown();
		m_EG3.shutDown();
		m_EG4.shutDown();
	}


	inline virtual void noteOff(UINT uMIDINoteNumber)
	{
		if(m_bNoteOn)
		{
			if(m_bNotePending && (uMIDINoteNumber == m_uMIDINoteNumberPending))
			{
				m_bNotePending = false;
				return;
			}
		
			if(uMIDINoteNumber != m_uMIDINoteNumber)
				return;

			// are we already in Release or Shutdown or Off?
			if(m_EG1.getState() == release || m_EG1.getState() == off)
				return;
			
			m_EG1.noteOff();
			m_EG2.noteOff();
			m_EG3.noteOff();
			m_EG4.noteOff();
		}
	}

	// returns true if voice is still ON
	inline virtual bool doVoice(double& dLeftOutput, double& dRightOutput)
	{
		// clear destinations
		dLeftOutput = 0.0;
		dRightOutput = 0.0;

		// bail if no note 
		if(!m_bNoteOn)
			return false;

		// did EG finish? - its the flag for us as a voice
		if(isVoiceDone() || m_bNotePending)
		{
			// did EG finish with NO note pending?
			if(isVoiceDone() && !m_bNotePending)
			{
				// shut off and reset everything
				if(m_pOsc1)m_pOsc1->stopOscillator();
				if(m_pOsc2)m_pOsc2->stopOscillator();
				if(m_pOsc3)m_pOsc3->stopOscillator();
				if(m_pOsc4)m_pOsc4->stopOscillator();

				// need this in case of steal mode
				if(m_pOsc1)m_pOsc1->reset();
				if(m_pOsc2)m_pOsc2->reset();
				if(m_pOsc3)m_pOsc3->reset();
				if(m_pOsc4)m_pOsc4->reset();

				// stop the LFOs
				m_LFO1.stopOscillator();
				m_LFO2.stopOscillator();

				// stop the EGs
				m_EG1.reset();
				m_EG2.reset();
				m_EG3.reset();
				m_EG4.reset();

				// not more note
				m_bNoteOn = false;

				// done
				return false;
			}
			else if(m_bNotePending && (isVoiceDone() || inLegatoMode()))// note pending so turn it on
			{
				// transfer information from PENDING values
				m_uMIDINoteNumber = m_uMIDINoteNumberPending;
				m_uMIDIVelocity = m_uMIDIVelocityPending;							
				m_dOscPitch = m_dOscPitchPending;
				m_uTimeStamp = 0; // start

				// set note number
				m_ModulationMatrix.m_dSources[SOURCE_MIDI_NOTE_NUM] = (double)m_uMIDINoteNumber;

				if(!inLegatoMode())
					// new velocity value
					m_ModulationMatrix.m_dSources[SOURCE_VELOCITY] = (double)m_uMIDIVelocity;
				
				// portamento on? use start pitch
				double dPitch = m_dPortamentoInc > 0.0 ? m_dPortamentoStart : m_dOscPitch;

				if(m_pOsc1)m_pOsc1->m_dOscFo = dPitch;
				if(m_pOsc2)m_pOsc2->m_dOscFo = dPitch;
				if(m_pOsc3)m_pOsc3->m_dOscFo = dPitch;
				if(m_pOsc4)m_pOsc4->m_dOscFo = dPitch;
		
				// MIDI note number
				if(m_pOsc1)m_pOsc1->m_uMIDINoteNumber = m_uMIDINoteNumber;
				if(m_pOsc2)m_pOsc2->m_uMIDINoteNumber = m_uMIDINoteNumber;
				if(m_pOsc3)m_pOsc3->m_uMIDINoteNumber = m_uMIDINoteNumber;
				if(m_pOsc4)m_pOsc4->m_uMIDINoteNumber = m_uMIDINoteNumber;

                // need this in case of steal mode for sample-based oscillators
                if(!m_uLegatoMode)
                {
                    if(m_pOsc1)m_pOsc1->reset();
                    if(m_pOsc2)m_pOsc2->reset();
                    if(m_pOsc3)m_pOsc3->reset();
                    if(m_pOsc4)m_pOsc4->reset();
                }
                
				// update so new pitch is used
				if(m_pOsc1)m_pOsc1->update();
				if(m_pOsc2)m_pOsc2->update();
				if(m_pOsc3)m_pOsc3->update();
				if(m_pOsc4)m_pOsc4->update();

				// crank the EGs back up
				m_EG1.startEG();
				m_EG2.startEG();
				m_EG3.startEG();
				m_EG4.startEG();

				// start LFOs
				m_LFO1.startOscillator();
				m_LFO2.startOscillator();

				// clear flag
				m_bNotePending = false;
			}
		}
		
		// --- PORTAMENTO BLOCK --- //
		if(m_dPortamentoInc > 0.0 && m_pOsc1->m_dOscFo != m_dOscPitch)
		{
			if(m_dModuloPortamento >= 1.0)
			{
				m_dModuloPortamento = 0.0;

				// target pitch has now been hit
				if(m_pOsc1) m_pOsc1->m_dOscFo = m_dOscPitch;
				if(m_pOsc2) m_pOsc2->m_dOscFo = m_dOscPitch;
				if(m_pOsc3) m_pOsc3->m_dOscFo = m_dOscPitch;
				if(m_pOsc4) m_pOsc4->m_dOscFo = m_dOscPitch;
			}
			else
			{
				double dPortamentoPitch = m_dPortamentoStart*pitchShiftMultiplier(m_dModuloPortamento*m_dPortamentoSemitones);
			
				if(m_pOsc1) m_pOsc1->m_dOscFo = dPortamentoPitch;
				if(m_pOsc2) m_pOsc2->m_dOscFo = dPortamentoPitch;
				if(m_pOsc3) m_pOsc3->m_dOscFo = dPortamentoPitch;
				if(m_pOsc4) m_pOsc4->m_dOscFo = dPortamentoPitch;
	
				m_dModuloPortamento += m_dPortamentoInc;
			}
		}

		// if we make it here, we are rendering
		// NOTE: Derived class method must call update() on oscillators
		//       for portamento to work
		return true;
	}
};
