#pragma once

#include "voice.h"
#include "QBLimitedOscillator.h"
#include "MoogLadderFilter.h"

class CMiniSynthVoice : public CVoice
{
public:
	CMiniSynthVoice(void);
	~CMiniSynthVoice(void);

protected:
	// --- Four oscillators
	CQBLimitedOscillator m_Osc1;
	CQBLimitedOscillator m_Osc2;
	CQBLimitedOscillator m_Osc3;
	CQBLimitedOscillator m_Osc4; // noise
	
	// --- 1 filter
	CMoogLadderFilter m_MoogLadderFilter;

	// --- repeat of enum for easier coding
	enum {Saw3,Sqr3,Saw2Sqr,Tri2Saw,Tri2Sqr};

public:
	inline virtual void initGlobalParameters(globalSynthParams* pGlobalParams)
	{
		// --- always call base class first
		CVoice::initGlobalParameters(pGlobalParams);

		// --- add any CThisVoice specific variables here
		//     (you need to add them to the global param struct first)
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

	// --- Overrides
	virtual void initializeModMatrix(CModulationMatrix* pMatrix);
	virtual void prepareForPlay();
	virtual void setSampleRate(double dSampleRate);
	virtual void update();
	virtual void reset();

	inline virtual bool doVoice(double& dLeftOutput, double& dRightOutput)
	{

		// this does basic on/off work zero impact on speed
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
		m_MoogLadderFilter.update();

		// --- update oscillators
		m_Osc1.update();
		m_Osc2.update();
		m_Osc3.update();
		m_Osc4.update();

		// --- DIGITAL AUDIO ENGINE BLOCK --- //
		double dOscMix = 0.333*m_Osc1.doOscillate() + 
						 0.333*m_Osc2.doOscillate() +
						 0.333*m_Osc3.doOscillate() +
						 m_Osc4.doOscillate();
	    
		// --- apply the filter
		double dLPFOut = m_MoogLadderFilter.doFilter(dOscMix);

		// --- apply the DCA
		m_DCA.doDCA(dLPFOut, dLPFOut, dLeftOutput, dRightOutput);
	    
		return true;
	}
};
