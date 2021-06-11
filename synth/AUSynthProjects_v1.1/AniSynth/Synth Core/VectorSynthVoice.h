#pragma once
#include "DigiSynthVoice.h"

#include "SampleOscillator.h"
#include "SEMFilter.h"
#include "VectorPathGenerator.h"
#include "KThreeFiveFilter.h"

// --- RackAFX only!
#define _RAFX

class CVectorSynthVoice : public CDigiSynthVoice
{
public:
	CVectorSynthVoice(void);
	virtual ~CVectorSynthVoice(void);

	// --- our synth components
	// --- can use other oscillators too for VS!
	CSampleOscillator m_OscA; 
	CSampleOscillator m_OscB;
	CSampleOscillator m_OscC;
	CSampleOscillator m_OscD;

	// --- using K35 Filter here
	CKThreeFiveFilter m_LeftK35Filter;
	CKThreeFiveFilter m_RightK35Filter;

	// --- VS specific variables
	double m_dAmplitude_A;
	double m_dAmplitude_B;
	double m_dAmplitude_C;
	double m_dAmplitude_D;
	double m_dAmplitude_ACmix;
	double m_dAmplitude_BDmix;
	double m_dVectorCenterX;
	double m_dVectorCenterY;
	double m_dOrbitXAmp; // 0->1
	double m_dOrbitYAmp; // 0->1
	
	// RAFX ONLY: JS Program; need one copy per voice!
	CJoystickProgram* m_pJSProgram;
	
	// for anyone
	CVectorPathGenerator m_VPG;
	bool m_bEnableVPG;

//	UINT m_uJSProgramMode;
	enum{manual,program};

	UINT m_uVectorPathMode;
	enum{once,loop,sustain,bckfrth};

	// --- Overrides
	virtual void prepareForPlay();
	virtual void setSampleRate(double dSampleRate);
	virtual void update();
	virtual void reset();
	
	// --- RAFX ONLY
	inline void setJSProgram(CJoystickProgram* pProgram){m_pJSProgram = pProgram;}

	// --- anyone
	inline void setUseVPGProgram(bool b){m_bEnableVPG = b;}
		
	// --- init mod matrix
	virtual void initializeModMatrix(CModulationMatrix* pMatrix);

	// --- init golbals
	inline virtual void initGlobalParameters(globalSynthParams* pGlobalParams)
	{
		// --- always call base class first
		CDigiSynthVoice::initGlobalParameters(pGlobalParams);

		// --- add any CThisVoice specific variables here
		//     (you need to add them to the global param struct first)
		m_pGlobalVoiceParams->dOrbitXAmp = m_dOrbitXAmp;
		m_pGlobalVoiceParams->dOrbitYAmp = m_dOrbitYAmp;
		m_pGlobalVoiceParams->uVectorPathMode = m_uVectorPathMode;

		m_pGlobalVoiceParams->dAmplitude_A = m_dAmplitude_A;
		m_pGlobalVoiceParams->dAmplitude_B = m_dAmplitude_B;
		m_pGlobalVoiceParams->dAmplitude_C = m_dAmplitude_C;
		m_pGlobalVoiceParams->dAmplitude_D = m_dAmplitude_D;

		m_pGlobalVoiceParams->dAmplitude_ACmix = m_dAmplitude_ACmix;
		m_pGlobalVoiceParams->dAmplitude_BDmix = m_dAmplitude_BDmix;
	}

	// need to override noteOn and noteOff to deal with joystick
	inline virtual void noteOn(UINT uMIDINote, UINT uMIDIVelocity, double dFrequency, double dLastNoteFrequency)
	{
		CVoice::noteOn(uMIDINote, uMIDIVelocity, dFrequency, dLastNoteFrequency);

		if(m_uVoiceMode == program)
		{
			if(m_bEnableVPG)
				m_VPG.startProgram();
			else if(m_pJSProgram)
				m_pJSProgram->startProgram();
		}
	}
	
	inline virtual void noteOff(UINT uMIDINoteNumber)
	{
		CVoice::noteOff(uMIDINoteNumber);

		if(m_uVoiceMode == program && m_uVectorPathMode == sustain)
		{
			// need to restart the JSProgram; it will pause itself during the sustain portion
			if(m_bEnableVPG)
				m_VPG.resumeProgram();
			else if(m_pJSProgram)
				m_pJSProgram->resumeProgram();
		}
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
		m_LFO2.update();	

		// --- do layer 1 modulators
		m_EG1.doEnvelope();
		m_LFO1.doOscillate();
		m_LFO2.doOscillate(); // rotor

		// --- mod matrix Layer 1
		m_ModulationMatrix.doModulationMatrix(1);
		
		// --- update Voice, DCA and Filter
		this->update();
		m_DCA.update();	
		m_LeftK35Filter.update();
		m_RightK35Filter.update();

		// --- update ocillators
		m_OscA.update();
		m_OscB.update();
		m_OscC.update();
		m_OscD.update();

		// --- do the voice
		double dLeftA,dRightA;
		double dLeftB,dRightB;
		double dLeftC,dRightC;
		double dLeftD,dRightD;
		
		// --- if in program mode, run the JS or VPG 
		if(m_uVoiceMode == program)
		{
			// --- nudge timer; VPG "wins" if both present
			if(m_bEnableVPG)
			{
				m_VPG.incTimer();
				
				// --- uses pass-by-reference to set our amps with values
				m_VPG.getVectorMixValues(m_dAmplitude_A, m_dAmplitude_B, m_dAmplitude_C, m_dAmplitude_D);
				m_VPG.getVectorACBDMixes(m_dAmplitude_ACmix, m_dAmplitude_BDmix);
			}
			else if(m_pJSProgram)
			{
				// --- JS Program is older and uses floats
				//     since these are pass-by-reference need
				//     to declare and set here
				float fA, fB, fC, fD, fAC, fBD; 
				m_pJSProgram->incTimer();

				// --- uses pass-by-reference to set our amps with values
				m_pJSProgram->getVectorMixValues(fA, fB, fC, fD);
				m_pJSProgram->getVectorACBDMixes(fAC, fBD);
				
				//--- set on our variables
				m_dAmplitude_A = fA;
				m_dAmplitude_B = fB;
				m_dAmplitude_C = fC;
				m_dAmplitude_D = fD;
				m_dAmplitude_ACmix = fAC;
				m_dAmplitude_BDmix = fBD;
			}
	
			// --- find the current center location
			m_dVectorCenterX = unipolarToBipolar(m_dAmplitude_ACmix);
			m_dVectorCenterY = unipolarToBipolar(m_dAmplitude_BDmix);
		}

		double dRotorACMix, dRotorBDMix = 0.0;

		// --- these are from LFO2
		double dRotorX = m_ModulationMatrix.m_dDestinations[DEST_VS_AC_AXIS];
		double dRotorY = m_ModulationMatrix.m_dDestinations[DEST_VS_BD_AXIS];

		// --- for RAFX, use the Korg joystick coordinates
		bool bJoystickCoords = true;

		#ifndef _RAFX
			bJoystickCoords = false;
		#endif

		// --- calculate the final vector mix values based on path and rotor and joystick location
		calculateVectorMixValues(m_dVectorCenterX, m_dVectorCenterY, 
								 dRotorX*m_dOrbitXAmp, dRotorY*m_dOrbitYAmp, 
							     m_dAmplitude_A, m_dAmplitude_B, m_dAmplitude_C, m_dAmplitude_D, 
								 dRotorACMix, dRotorBDMix, 1, bJoystickCoords);
				
		// --- get our 4 osc outputs
		dLeftA = m_pOsc1->doOscillate(&dRightA);
		dLeftB = m_pOsc2->doOscillate(&dRightB);
		dLeftC = m_pOsc3->doOscillate(&dRightC);
		dLeftD = m_pOsc4->doOscillate(&dRightD);

		// --- do the massive mix
		double dLeftVectorMix = m_dAmplitude_A*dLeftA + m_dAmplitude_B*dLeftB + m_dAmplitude_C*dLeftC + m_dAmplitude_D*dLeftD;
		double dRightVectorMix = m_dAmplitude_A*dRightA + m_dAmplitude_B*dRightB + m_dAmplitude_C*dRightC + m_dAmplitude_D*dRightD;

		// --- Challenge: apply AC and BD mix to left and right filter cutoffs!
		dLeftOutput = m_LeftK35Filter.doFilter(dLeftVectorMix);
		dRightOutput = m_RightK35Filter.doFilter(dRightVectorMix);

		// --- apply the DCA
		m_DCA.doDCA(dLeftOutput, dRightOutput, dLeftOutput, dRightOutput);

		return true;
	}
};
