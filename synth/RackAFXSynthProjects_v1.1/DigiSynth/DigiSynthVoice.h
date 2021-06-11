#pragma once
#include "voice.h"
#include "SampleOscillator.h"
#include "SEMFilter.h"

class CDigiSynthVoice : public CVoice
{
public:
	CDigiSynthVoice(void);
	~CDigiSynthVoice(void);

protected:
	// --- our synth components
	//     2 oscillators for splits/layers
	CSampleOscillator m_Osc1;
	CSampleOscillator m_Osc2;

	// --- SEMFilter has multiple types; 
	//     useful for sample based manipulation
	CSEMFilter m_LeftSEMFilter;
	CSEMFilter m_RightSEMFilter;
	
	// --- for our voice mode
	enum {Osc1,Osc2,split,layer};

	// --- regions to hold our spli/layer information
	noteRegion m_Osc1NoteRegion;
	noteRegion m_Osc2NoteRegion;

	// --- test to see if in Osc Split Range
	inline bool inOscSplitRange(int nOscIndex)
	{
		if(nOscIndex == 0)
		{
			if(m_uMIDINoteNumber >= m_Osc1NoteRegion.uMIDINoteLow &&
				m_uMIDINoteNumber <= m_Osc1NoteRegion.uMIDINoteHigh)
				return true;
		}
		else if(nOscIndex == 1)
		{
			if(m_uMIDINoteNumber >= m_Osc2NoteRegion.uMIDINoteLow &&
				m_uMIDINoteNumber <= m_Osc2NoteRegion.uMIDINoteHigh)
				return true;
		}

		return false;
	}

	// --- test to see if in Osc Velocity Range
	inline bool inOscVelocityRange(int nOscIndex)
	{
		if(nOscIndex == 0)
		{
			if(m_uMIDIVelocity >= m_Osc1NoteRegion.uMIDIVelocityLow &&
				m_uMIDIVelocity <= m_Osc1NoteRegion.uMIDIVelocityHigh)
				return true;
		}
		else if(nOscIndex == 1)
		{
			if(m_uMIDIVelocity >= m_Osc2NoteRegion.uMIDIVelocityLow &&
				m_uMIDIVelocity <= m_Osc2NoteRegion.uMIDIVelocityHigh)
				return true;
		}
		return false;
	}

public:
	// --- Overrides
	virtual void prepareForPlay();
	virtual void setSampleRate(double dSampleRate);
	virtual void update();
	virtual void reset();
	
	inline bool isOscSingleCycleSample(UINT uOsc)
	{
		CSampleOscillator* pOsc = NULL;
		switch(uOsc)
		{
			case 0:
				pOsc = (CSampleOscillator*)m_pOsc1; break;
			case 1:
				pOsc = (CSampleOscillator*)m_pOsc2; break;
			case 2:
				pOsc = (CSampleOscillator*)m_pOsc3; break;
			case 3:
				pOsc = (CSampleOscillator*)m_pOsc4; break;
			default:
				break;
		}

		// --- return the flag
		if(pOsc)
			return pOsc->m_bSingleCycleSample;

		return false;
	}
	
	inline CWaveData* getOscWaveData(UINT uOsc)
	{
		CSampleOscillator* pOsc = NULL;
		switch(uOsc)
		{
			case 0:
				pOsc = (CSampleOscillator*)m_pOsc1; break;
			case 1:
				pOsc = (CSampleOscillator*)m_pOsc2; break;
			case 2:
				pOsc = (CSampleOscillator*)m_pOsc3; break;
			case 3:
				pOsc = (CSampleOscillator*)m_pOsc4; break;
			default:
				break;
		}

		// --- return the pointer from object
		if(pOsc)
			return pOsc->getWaveData();

		return NULL;
	}

	inline CWaveData** getOscWaveSamplePtrArray(UINT uOsc)
	{
		CSampleOscillator* pOsc = NULL;
		switch(uOsc)
		{
			case 0:
				pOsc = (CSampleOscillator*)m_pOsc1; break;
			case 1:
				pOsc = (CSampleOscillator*)m_pOsc2; break;
			case 2:
				pOsc = (CSampleOscillator*)m_pOsc3; break;
			case 3:
				pOsc = (CSampleOscillator*)m_pOsc4; break;
			default:
				break;
		}

		if(pOsc)
			return pOsc->getWaveSamplePtrArray();

		return NULL;
	}


	// --- for single samples
	inline bool initOscWithFilePath(UINT uOsc, char* pPath, bool bSingleCycleSample, bool bPitchlessSample)
	{
		CSampleOscillator* pOsc = NULL;

		switch(uOsc)
		{
			case 0:
				pOsc = (CSampleOscillator*)m_pOsc1; break;
			case 1:
				pOsc = (CSampleOscillator*)m_pOsc2; break;
			case 2:
				pOsc = (CSampleOscillator*)m_pOsc3; break;
			case 3:
				pOsc = (CSampleOscillator*)m_pOsc4; break;
			default:
				break;
		}

		// --- call the initializer function
		if(pOsc)
		{
			((CSampleOscillator*)pOsc)->m_bSingleCycleSample = bSingleCycleSample;
			((CSampleOscillator*)pOsc)->m_bPitchlessSample = bPitchlessSample;
			return pOsc->initWithFilePath(pPath);
		}

		return false;
	}

	// --- for multi-samples
	inline bool initOscWithFolderPath(UINT uOsc, char* pPath, bool bSingleCycleSample, bool bPitchlessSample)
	{
		CSampleOscillator* pOsc = NULL;
		switch(uOsc)
		{
			case 0:
				pOsc = (CSampleOscillator*)m_pOsc1; break;
			case 1:
				pOsc = (CSampleOscillator*)m_pOsc2; break;
			case 2:
				pOsc = (CSampleOscillator*)m_pOsc3; break;
			case 3:
				pOsc = (CSampleOscillator*)m_pOsc4; break;
			default:
				break;
		}

		// --- call the initializer function
		if(pOsc)
		{
			((CSampleOscillator*)pOsc)->m_bSingleCycleSample = bSingleCycleSample;
			((CSampleOscillator*)pOsc)->m_bPitchlessSample = bPitchlessSample;
			return ((CSampleOscillator*)pOsc)->initWithFolderPath(pPath);
		}

		return false;
	}

	// --- this copies the pointers so that samples can be shared to save load-time
	inline bool initAllOscWithDigiSynthVoice(CDigiSynthVoice* pVoice, bool bSingleCycleSample, bool bPitchlessSample)
	{
		if(pVoice->getOscWaveData(0))
			((CSampleOscillator*)m_pOsc1)->setWaveDataPtr(pVoice->getOscWaveData(0));
		if(pVoice->getOscWaveSamplePtrArray(0))
			((CSampleOscillator*)m_pOsc1)->setWaveSamplePtrArray(pVoice->getOscWaveSamplePtrArray(0));
		if(pVoice->getOscWaveData(1))
			((CSampleOscillator*)m_pOsc2)->setWaveDataPtr(pVoice->getOscWaveData(1));
		if(pVoice->getOscWaveSamplePtrArray(1))
			((CSampleOscillator*)m_pOsc2)->setWaveSamplePtrArray(pVoice->getOscWaveSamplePtrArray(1));
		if(pVoice->getOscWaveData(2))
			((CSampleOscillator*)m_pOsc3)->setWaveDataPtr(pVoice->getOscWaveData(2));
		if(pVoice->getOscWaveSamplePtrArray(2))
			((CSampleOscillator*)m_pOsc3)->setWaveSamplePtrArray(pVoice->getOscWaveSamplePtrArray(2));
		if(pVoice->getOscWaveData(3))
			((CSampleOscillator*)m_pOsc4)->setWaveDataPtr(pVoice->getOscWaveData(3));
		if(pVoice->getOscWaveSamplePtrArray(3))
			((CSampleOscillator*)m_pOsc4)->setWaveSamplePtrArray(pVoice->getOscWaveSamplePtrArray(3));

		if(m_pOsc1)
		{
			((CSampleOscillator*)m_pOsc1)->m_bSingleCycleSample = bSingleCycleSample;
			((CSampleOscillator*)m_pOsc1)->m_bPitchlessSample = bPitchlessSample;
		}
		if(m_pOsc2)
		{
			((CSampleOscillator*)m_pOsc2)->m_bSingleCycleSample = bSingleCycleSample;
			((CSampleOscillator*)m_pOsc2)->m_bPitchlessSample = bPitchlessSample;
		}
		if(m_pOsc3)
		{
			((CSampleOscillator*)m_pOsc3)->m_bSingleCycleSample = bSingleCycleSample;
			((CSampleOscillator*)m_pOsc3)->m_bPitchlessSample = bPitchlessSample;
		}
		if(m_pOsc4)
		{
			((CSampleOscillator*)m_pOsc4)->m_bSingleCycleSample = bSingleCycleSample;
			((CSampleOscillator*)m_pOsc4)->m_bPitchlessSample = bPitchlessSample;
		}
		return true;
	}

	
	// --- init mod matrix
	virtual void initializeModMatrix(CModulationMatrix* pMatrix);

	// --- init golbals
	inline virtual void initGlobalParameters(globalSynthParams* pGlobalParams)
	{
		// --- always call base class first
		CVoice::initGlobalParameters(pGlobalParams);

		// --- add any CThisVoice specific variables here
		//     (you need to add them to the global param struct first)
		if(m_pOsc1)((CSampleOscillator*)m_pOsc1)->initGlobalParameters(&pGlobalParams->osc1Params);
		if(m_pOsc2)((CSampleOscillator*)m_pOsc2)->initGlobalParameters(&pGlobalParams->osc2Params);
		if(m_pOsc3)((CSampleOscillator*)m_pOsc3)->initGlobalParameters(&pGlobalParams->osc3Params);
		if(m_pOsc4)((CSampleOscillator*)m_pOsc4)->initGlobalParameters(&pGlobalParams->osc4Params);

		if(m_pFilter1)((CSEMFilter*)m_pFilter1)->initGlobalParameters(&pGlobalParams->filter1Params);
		if(m_pFilter2)((CSEMFilter*)m_pFilter2)->initGlobalParameters(&pGlobalParams->filter2Params);

		// these default to 1.0 in case user doesnt have a GUI control for them
		//
		// NOTE: we only set the intensities we use in THIS VOICE
		m_pGlobalVoiceParams->dLFO1OscModIntensity = 1.0;
		m_pGlobalVoiceParams->dLFO1Filter1ModIntensity = 1.0;
		m_pGlobalVoiceParams->dLFO1Filter2ModIntensity = 1.0;
		m_pGlobalVoiceParams->dLFO1DCAPanModIntensity = 1.0;
		m_pGlobalVoiceParams->dLFO1DCAAmpModIntensity = 1.0;

		m_pGlobalVoiceParams->dEG1OscModIntensity = 1.0;
		m_pGlobalVoiceParams->dEG1Filter1ModIntensity = 1.0;
		m_pGlobalVoiceParams->dEG1Filter2ModIntensity = 1.0;
		m_pGlobalVoiceParams->dEG1DCAAmpModIntensity = 1.0;
	}

	inline virtual bool doVoice(double& dLeftOutput, double& dRightOutput)
	{
		// this does basic on/off work
		if(!CVoice::doVoice(dLeftOutput, dRightOutput))
			return false;

		// --- ARTICULATION BLOCK --- //
		// --- layer 0 modulators: velocity->attack
		//						   note number->decay
		m_ModulationMatrix.doModulationMatrix(0);

		// --- update layer 1 modulators
		m_EG1.update();
		m_LFO1.update();	

		// --- do layer 1 modulators
		m_EG1.doEnvelope();
		m_LFO1.doOscillate();

		// --- modulation matrix Layer 1
		m_ModulationMatrix.doModulationMatrix(1);
		
		// --- update Voice, DCA and Filter
		this->update();
		m_DCA.update();	
		m_LeftSEMFilter.update();
		m_RightSEMFilter.update();

		// --- update oscillators
		m_Osc1.update();
		m_Osc2.update();
		
		// --- DIGITAL AUDIO ENGINE BLOCK --- //
		double dLeftAccum = 0.0;
		double dRightAccum = 0.0;
		double dLeft1 = 0.0;
		double dRight1 = 0.0;
		double dLeft2 = 0.0 ;
		double dRight2 = 0.0;

		// --- check for velocity and split location
		if(m_pGlobalVoiceParams->uVoiceMode == Osc1)
			dLeftAccum = m_Osc1.doOscillate(&dRightAccum);
		else if(m_pGlobalVoiceParams->uVoiceMode == Osc2)
			dLeftAccum = m_Osc2.doOscillate(&dRightAccum);
		else if(m_pGlobalVoiceParams->uVoiceMode == split)
		{
			bool Osc1On = inOscSplitRange(0);
			bool Osc2On = inOscSplitRange(1);
			
			dLeft1 = m_Osc1.doOscillate(&dRight1);
			dLeft2 = m_Osc2.doOscillate(&dRight2);
			
			if(Osc1On && Osc2On)
			{
				dLeftAccum = 0.5*dLeft1 + 0.5*dLeft2;
				dRightAccum = 0.5*dRight1 + 0.5*dRight2;
			}
			else if(Osc1On)
			{
				dLeftAccum = dLeft1;
				dRightAccum = dRight1;
			}
			else if(Osc2On)
			{
				dLeftAccum = dLeft2;
				dRightAccum = dRight2;
			}
		}
		else if(m_pGlobalVoiceParams->uVoiceMode == layer)
		{
			bool Osc1On = inOscVelocityRange(0);
			bool Osc2On = inOscVelocityRange(1);

			dLeft1 = m_Osc1.doOscillate(&dRight1);
			dLeft2 = m_Osc2.doOscillate(&dRight2);
			
			if(Osc1On && Osc2On)
			{
				dLeftAccum = 0.5*dLeft1 + 0.5*dLeft2;
				dRightAccum = 0.5*dRight1 + 0.5*dRight2;
			}
			else if(Osc1On)
			{
				dLeftAccum = dLeft1;
				dRightAccum = dRight1;
			}
			else if(Osc2On)
			{
				dLeftAccum = dLeft2;
				dRightAccum = dRight2;
			}
		}

		// --- apply the filter
		dLeftOutput = m_LeftSEMFilter.doFilter(dLeftAccum);
		dRightOutput = m_RightSEMFilter.doFilter(dRightAccum);

		// --- apply the DCA
		m_DCA.doDCA(dLeftOutput, dRightOutput, dLeftOutput, dRightOutput);

		return true;
	}
};
