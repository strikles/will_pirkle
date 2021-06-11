#pragma once
#include "voice.h"
#include "WTOscillator.h"
#include "QBLimitedOscillator.h"

#define IMAX 4.0

class CDXSynthVoice : public CVoice
{
public:
	CDXSynthVoice(void);
	~CDXSynthVoice(void);

protected:
	// specific to this voice
	//
	// You can choose the oscillator you like here
	//CWTOscillator m_Op1;
	//CWTOscillator m_Op2;
	//CWTOscillator m_Op3;
	//CWTOscillator m_Op4;

	CQBLimitedOscillator m_Op1;
	CQBLimitedOscillator m_Op2;
	CQBLimitedOscillator m_Op3;
	CQBLimitedOscillator m_Op4;

	// --- Op4 feedback is the only one used in stock DX100
	double m_dOp1Feedback;
	double m_dOp2Feedback;
	double m_dOp3Feedback;
	double m_dOp4Feedback;

	// enum for LFO mod
	enum{None,AmpMod,Vibrato};

public:

	// inline void setOp4Feedback(double d){m_dOp4Feedback = d;}
	inline void setLFO1Destination(UINT uOperator, UINT uDestination)
	{
		switch(uOperator)
		{
			case 0:
			{
				if(uDestination == AmpMod && m_Op1.m_uModSourceAmp != DEST_OSC1_OUTPUT_AMP)
				{
					m_Op1.m_uModSourceAmp = DEST_OSC1_OUTPUT_AMP;
					m_Op1.m_uModSourceFo = DEST_NONE;
				}
				else if(uDestination == Vibrato && m_Op1.m_uModSourceFo != DEST_OSC1_FO)
				{
					m_Op1.m_uModSourceFo = DEST_OSC1_FO;
					m_Op1.m_uModSourceAmp = DEST_NONE;
				}
				else if(uDestination == None && (m_Op1.m_uModSourceAmp != DEST_NONE || m_Op1.m_uModSourceFo != DEST_NONE))
				{
					m_Op1.m_uModSourceAmp = DEST_NONE;
					m_Op1.m_uModSourceFo = DEST_NONE;
				}
				break;
			}

			case 1:
			{
				if(uDestination == AmpMod && m_Op2.m_uModSourceAmp != DEST_OSC2_OUTPUT_AMP)
				{
					m_Op2.m_uModSourceAmp = DEST_OSC2_OUTPUT_AMP;
					m_Op2.m_uModSourceFo = DEST_NONE;
				}
				else if(uDestination == Vibrato && m_Op2.m_uModSourceFo != DEST_OSC2_FO)
				{
					m_Op2.m_uModSourceFo = DEST_OSC2_FO;
					m_Op2.m_uModSourceAmp = DEST_NONE;
				}
				else if(uDestination == None && (m_Op2.m_uModSourceAmp != DEST_NONE || m_Op2.m_uModSourceFo != DEST_NONE))
				{
					m_Op2.m_uModSourceAmp = DEST_NONE;
					m_Op2.m_uModSourceFo = DEST_NONE;
				}
				break;
			}
			case 2:
			{
				if(uDestination == AmpMod && m_Op3.m_uModSourceAmp != DEST_OSC3_OUTPUT_AMP)
				{
					m_Op3.m_uModSourceAmp = DEST_OSC3_OUTPUT_AMP;
                    m_Op3.m_uModSourceFo = DEST_NONE;
				}
				else if(uDestination == Vibrato && m_Op3.m_uModSourceFo != DEST_OSC3_FO)
				{
					m_Op3.m_uModSourceFo = DEST_OSC3_FO;
                    m_Op3.m_uModSourceAmp = DEST_NONE;
				}
				else if(uDestination == None &&( m_Op3.m_uModSourceAmp != DEST_NONE || m_Op3.m_uModSourceFo != DEST_NONE))
				{
					m_Op3.m_uModSourceAmp = DEST_NONE;
                    m_Op3.m_uModSourceFo = DEST_NONE;
				}
				break;
			}
			case 3:
			{
				if(uDestination == AmpMod && m_Op4.m_uModSourceAmp != DEST_OSC4_OUTPUT_AMP)
				{
					m_Op4.m_uModSourceAmp = DEST_OSC4_OUTPUT_AMP;
                    m_Op3.m_uModSourceFo = DEST_NONE;
				}
				else if(uDestination == Vibrato && m_Op4.m_uModSourceFo != DEST_OSC4_FO)
				{
					m_Op4.m_uModSourceFo = DEST_OSC4_FO;
                    m_Op4.m_uModSourceAmp = DEST_NONE;
				}
				else if(uDestination == None && (m_Op4.m_uModSourceAmp != DEST_NONE || m_Op4.m_uModSourceFo != DEST_NONE))
				{
					m_Op4.m_uModSourceAmp = DEST_NONE;
                    m_Op4.m_uModSourceFo = DEST_NONE;
				}

				break;
			}

			default:
				break;
		}
	}

	
	inline virtual void initGlobalParameters(globalSynthParams* pGlobalParams)
	{
		// --- always call base class first
		CVoice::initGlobalParameters(pGlobalParams);

		// --- add any CThisVoice specific variables here
		//     (you need to add them to the global param struct first)
		// these default to 1.0 in case user doesnt have a GUI control for them
		//
		// NOTE: we only set the intensities we use in THIS VOICE
		m_pGlobalVoiceParams->dOp1Feedback = m_dOp1Feedback;
		m_pGlobalVoiceParams->dOp2Feedback = m_dOp2Feedback;
		m_pGlobalVoiceParams->dOp3Feedback = m_dOp3Feedback;
		m_pGlobalVoiceParams->dOp4Feedback = m_dOp4Feedback;

		// --- set to 1.0 in case user has no GUI control
		m_pGlobalVoiceParams->dLFO1OscModIntensity = 1.0;
	}

	// --- Overrides
	virtual void initializeModMatrix(CModulationMatrix* pMatrix);
	virtual void prepareForPlay();
	virtual void setSampleRate(double dSampleRate);
	virtual void update();
	virtual void reset();

	/// NEEED TO OVERRIDE NOTE OFF!!!! EGS!!!!!!!! WP 3.13.14

	// we are in a state to accept a noteOff message
	// for FM synth, this depends on algorithm, all
	// carrier oscillators must be in A, D or S state
	inline virtual bool canNoteOff()
	{
		bool bRet = false;
		if(!m_bNoteOn)
			return bRet;
		else
		{
			switch(m_uVoiceMode+1)
			{
				case 1:
				case 2:
				case 3:
				case 4:
				{
					if(m_EG1.canNoteOff())
						bRet = true;
					break;
				}

				case 5:
				{
					if(m_EG1.canNoteOff() && m_EG2.canNoteOff())
						bRet = true;
					break;
				}

				case 6:
				case 7:
				{
					if(m_EG1.canNoteOff() && m_EG2.canNoteOff() && m_EG3.canNoteOff())
						bRet = true;
					break;
				}

				case 8:
				{
					if(m_EG1.canNoteOff() && m_EG2.canNoteOff() && m_EG3.canNoteOff() && m_EG4.canNoteOff())
						bRet = true;
					break;
				}
			}

		}
		return bRet;
	}

	// this checks to see that the carrier EG(s) are in
	// the off state, meaning voice is done
	inline virtual bool isVoiceDone()
	{
		bool bRet = false;
		switch(m_uVoiceMode+1)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			{
				if(m_EG1.getState() == off)
					bRet = true;
				break;
			}

			case 5:
			{
				if(m_EG1.getState() == off && m_EG2.getState() == off)
					bRet = true;
				break;
			}

			case 6:
			case 7:
			{
				if(m_EG1.getState() == off && m_EG2.getState() == off && m_EG3.getState() == off)
					bRet = true;
				break;
			}

			case 8:
			{
				if(m_EG1.getState() == off && m_EG2.getState() == off && m_EG3.getState() == off && m_EG4.getState() == off)
					bRet = true;
				break;
			}
		}

		return bRet;
	}
	
	// we are in a state to accept a noteOff message
	// for FM synth, this depends on algorithm, all
	// carrier oscillators must be in A, D or S state
	inline void setOutputEGs()
	{
		m_EG1.m_bOutputEG = false;
		m_EG2.m_bOutputEG = false;
		m_EG3.m_bOutputEG = false;
		m_EG4.m_bOutputEG = false;
		switch(m_uVoiceMode+1)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			{
				m_EG1.m_bOutputEG = true;
				break;
			}

			case 5:
			{
				m_EG1.m_bOutputEG = true;
				m_EG2.m_bOutputEG = true;
				break;
			}

			case 6:
			case 7:
			{
				m_EG1.m_bOutputEG = true;
				m_EG2.m_bOutputEG = true;
				m_EG3.m_bOutputEG = true;
				break;
			}

			case 8:
			{
				m_EG1.m_bOutputEG = true;
				m_EG2.m_bOutputEG = true;
				m_EG3.m_bOutputEG = true;
				m_EG4.m_bOutputEG = true;
				break;
			}
		}
	}

	inline virtual bool doVoice(double& dLeftOutput, double& dRightOutput)
	{
		// this does basic on/off work
		if(!CVoice::doVoice(dLeftOutput, dRightOutput))
			return false;

/*		// portamento? Op1 is ALWAYS a carrier
		if(m_dPortamentoInc > 0.0 && m_Op1.m_dOscFo != m_dOscPitch)
		{
			// target pitch has now been hit
			if(m_dModuloPortamento >= 1.0)
			{
				m_dModuloPortamento = 0.0;
				m_Op1.m_dOscFo = m_dOscPitch;
				m_Op2.m_dOscFo = m_dOscPitch;
				m_Op3.m_dOscFo = m_dOscPitch;
				m_Op4.m_dOscFo = m_dOscPitch;
			}
			else
			{
				// calculate the pitch multiplier for this sample interval
				double dPortamentoPitchMult = pitchShiftMultiplier(m_dModuloPortamento*m_dPortamentoSemitones);

				// set it on our one and only one oscillator
				m_Op1.m_dOscFo = m_dPortamentoStart*dPortamentoPitchMult;
				m_Op2.m_dOscFo = m_dPortamentoStart*dPortamentoPitchMult;
				m_Op3.m_dOscFo = m_dPortamentoStart*dPortamentoPitchMult;
				m_Op4.m_dOscFo = m_dPortamentoStart*dPortamentoPitchMult;

				// inc the modulo
				m_dModuloPortamento += m_dPortamentoInc;
			}

			// oscillator is still running, so just update
			m_Op1.update();
			m_Op2.update();
			m_Op3.update();
			m_Op4.update();
		}*/

		double dOut1, dOut2, dOut3, dOut4 = 0.0;
		double dEG1, dEG2, dEG3, dEG4 = 0.0;
		double dOut = 0.0;

		// --- user could change algo on the fly
		setOutputEGs();
		
		// --- Layer 0 modulations include Vel->Att and Note->Dcy
		//     But those are not standard in the DX Synth
		m_ModulationMatrix.doModulationMatrix(0);

		// --- update
		m_EG1.update();
		m_EG2.update();
		m_EG3.update();
		m_EG4.update();

		// --- all algos use the same EG code; get EG outputs
		dEG1 = m_EG1.doEnvelope();
		dEG2 = m_EG2.doEnvelope();
		dEG3 = m_EG3.doEnvelope();
		dEG4 = m_EG4.doEnvelope();

		// LFO is MOD source
		m_LFO1.update();
		m_LFO1.doOscillate();

		// --- Layer 1 modulations (LFO->AM or LFO->Vibrato)
		m_ModulationMatrix.doModulationMatrix(1);

		// --- apply modulations
		this->update();
		m_Op1.update();
		m_Op2.update();
		m_Op3.update();
		m_Op4.update();

		// --- DCA
		m_DCA.update();

		// --- now decode algorithm (m_uVoiceMode+1) and implement
		//     NOTE: using the mod matrix for this could get messy
		switch(m_uVoiceMode+1)
		{
			case 1:
			{
				/* FM VERSION 
				// --- modulator level 3
				double dFreqDev = IMAX*m_Op4.m_dOscFo;
				dOut4 = dEG4*m_Op4.doOscillate();

				// --- self modulate
				m_Op4.setFoModLin(dFreqDev*dOut4*m_dOp4Feedback);
				m_Op4.update();

				// --- modulator level 2
				dFreqDev = IMAX*m_Op3.m_dOscFo;
				m_Op3.setFoModLin(dFreqDev*dOut4);
				m_Op3.update();

				// --- form Op3 Output 
				dOut3 = dEG3*m_Op3.doOscillate();

				// --- modulator level 1
				dFreqDev = IMAX*m_Op1.m_dOscFo;
				m_Op2.setFoModLin(dFreqDev*dOut3);
				m_Op2.update();
				
				// --- form Op2 Output 
				dOut2 = dEG2*m_Op2.doOscillate();

				// --- carrier level 1
				dFreqDev = IMAX*m_Op1.m_dOscFo;
				m_Op1.setFoModLin(dFreqDev*dOut2);
				m_Op1.update();

				// --- form Op1 Output 
				dOut1 = IMAX*dEG1*m_Op1.doOscillate();

				// --- single carrier is Op1
				dOut = dOut1;
				//}
				//else
				//{*/

				// --- modulator level 3
				dOut4 = IMAX*dEG4*m_Op4.doOscillate();

				// --- self modulate
				m_Op4.setPhaseMod(dOut4*m_dOp4Feedback);
				m_Op4.update();

				// --- modulator level 2
				m_Op3.setPhaseMod(dOut4);
				m_Op3.update();

				// --- form Op3 Output 
				dOut3 = IMAX*dEG3*m_Op3.doOscillate();

				// --- modulator level 1
				m_Op2.setPhaseMod(dOut3);
				m_Op2.update();
				
				// --- form Op2 Output 
				dOut2 = IMAX*dEG2*m_Op2.doOscillate();

				// --- carrier level 1
				m_Op1.setPhaseMod(dOut2);
				m_Op1.update();

				// --- form Op1 Output 
				dOut1 = IMAX*dEG1*m_Op1.doOscillate();

				// --- single carrier is Op1
				dOut = dOut1;

				break;
			}

			case 2:
			{
				// --- modulator level 2
				dOut4 = IMAX*dEG4*m_Op4.doOscillate();

				// --- self modulate
				m_Op4.setPhaseMod(dOut4*m_dOp4Feedback);
				m_Op4.update();

				// --- form Op3 output
				dOut3 = IMAX*dEG3*m_Op3.doOscillate();

				// --- modulator level 1
				m_Op2.setPhaseMod(dOut3 + dOut4);
				m_Op2.update();
				
				// --- form Op2 output
				dOut2 = IMAX*dEG2*m_Op2.doOscillate();

				// --- carrier level 1
				m_Op1.setPhaseMod(dOut2);
				m_Op1.update();

				// --- form Op1 output
				dOut1 = IMAX*dEG1*m_Op1.doOscillate();

				// --- single carrier
				dOut = dOut1;

				break;
			}

			case 3:
			{
				// --- modulator level 2
				dOut4 = IMAX*dEG4*m_Op4.doOscillate();

				// --- self modulate
				m_Op4.setPhaseMod(dOut4*m_dOp4Feedback);
				m_Op4.update();

				// --- form Op3 Output
				dOut3 = IMAX*dEG3*m_Op3.doOscillate();

				// --- modulator level 1
				m_Op2.setPhaseMod(dOut3);
				m_Op2.update();

				// --- form Op2 Output
				dOut2 = IMAX*dEG2*m_Op2.doOscillate();

				// --- carrier level 1
				m_Op1.setPhaseMod(dOut2 + dOut4);
				m_Op1.update();

				// --- form Op1 Output
				dOut1 = IMAX*dEG1*m_Op1.doOscillate();

				// --- single carrier
				dOut = dOut1;

				break;
			}

			case 4:
			{
				// --- modulator level 2
				dOut4 = IMAX*dEG4*m_Op4.doOscillate();

				// --- self modulate
				m_Op4.setPhaseMod(dOut4*m_dOp4Feedback);
				m_Op4.update();

				// --- set Op3
				m_Op3.setPhaseMod(dOut4);
				m_Op3.update();

				// --- form Op3 output
				dOut3 = IMAX*dEG3*m_Op3.doOscillate();

				// --- modulator level 1
				dOut2 = IMAX*dEG2*m_Op2.doOscillate();

				// --- carrier level 1
				m_Op1.setPhaseMod(dOut2 + dOut3);
				m_Op1.update();

				// --- form Op1 Output
				dOut1 = IMAX*dEG1*m_Op1.doOscillate();

				// --- single carrier
				dOut = dOut1;

				break;
			}

			case 5:
			{
				// --- modulator level 3
				dOut4 = IMAX*dEG4*m_Op4.doOscillate();

				// --- self modulate
				m_Op4.setPhaseMod(dOut4*m_dOp4Feedback);
				m_Op4.update();

				// --- modulator level 2
				dOut3 = IMAX*dEG3*m_Op3.doOscillate();

				// --- modulator level 1
				m_Op2.setPhaseMod(dOut4);
				m_Op2.update();

				// --- carrier level 1
				dOut2 = IMAX*dEG2*m_Op2.doOscillate();

				// --- carrier level 1
				m_Op1.setPhaseMod(dOut3);
				m_Op1.update();

				// --- form final output
				dOut1 = IMAX*dEG1*m_Op1.doOscillate();

				// --- double carrier
				dOut = 0.5*dOut1 + 0.5*dOut2;

				break;
			}

			case 6:
			{
				// --- modulator level 3
				dOut4 = IMAX*dEG4*m_Op4.doOscillate();

				// --- self modulate
				m_Op4.setPhaseMod(dOut4*m_dOp4Feedback);
				m_Op4.update();

				// --- modulator level 2
				m_Op3.setPhaseMod(dOut4);
				m_Op3.update();

				// --- form Op3 Output
				dOut3 = IMAX*dEG3*m_Op3.doOscillate();

				// --- modulator level 1
				m_Op2.setPhaseMod(dOut4);
				m_Op2.update();

				// --- form Op3 Output
				dOut2 = IMAX*dEG2*m_Op2.doOscillate();

				// -- carrier level 1
				m_Op1.setPhaseMod(dOut4);
				m_Op1.update();

				// --- form Op1 Output
				dOut1 = IMAX*dEG1*m_Op1.doOscillate();

				// --- triple carrier
				dOut = 0.33*dOut1 + 0.33*dOut2 + 0.33*dOut3;

				break;
			}

			case 7:
			{
				// --- modulator level 3
				dOut4 = IMAX*dEG4*m_Op4.doOscillate();

				// --- self modulate
				m_Op4.setPhaseMod(dOut4*m_dOp4Feedback);
				m_Op4.update();

				// --- modulator level 2
				m_Op3.setPhaseMod(dOut4);
				m_Op3.update();

				// --- form Op3 output
				dOut3 = IMAX*dEG3*m_Op3.doOscillate();

				// --- modulator level 1
				dOut2 = IMAX*dEG2*m_Op2.doOscillate();

				// --- carrier level 1
				dOut1 = IMAX*dEG1*m_Op1.doOscillate();

				// --- triple carrier
				dOut = 0.33*dOut1 + 0.33*dOut2 + 0.33*dOut3;

				break;
			}

			case 8:
			{
				// --- carrier and modulator level 3
				dOut4 = IMAX*dEG4*m_Op4.doOscillate();

				// --- self modulate
				m_Op4.setPhaseMod(dOut4*m_dOp4Feedback);
				m_Op4.update();

				// --- carrier 3
				dOut3 = IMAX*dEG3*m_Op3.doOscillate();

				// --- carrier 2
				dOut2 = IMAX*dEG2*m_Op2.doOscillate();

				// --- carrier 1
				dOut1 = IMAX*dEG1*m_Op1.doOscillate();

				// --- quadruple carrier
				dOut = 0.25*dOut1 + 0.25*dOut2 + 0.25*dOut3 + 0.25*dOut4;

				break;
			}

			default:
				break;
		}

		// --- do the DCA: we have a mono signal so repeating dOut, dOut as L/R
		m_DCA.doDCA(dOut, dOut, dLeftOutput, dRightOutput);

		return true;
	}
};
