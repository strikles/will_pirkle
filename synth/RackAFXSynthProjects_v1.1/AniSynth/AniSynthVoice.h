#pragma once
#include "vectorsynthvoice.h"
#include "DiodeLadderFilter.h"

// --- RAFX only
#define _RAFX

class CAniSynthVoice : public CVectorSynthVoice
{
public:
	CAniSynthVoice(void);
	~CAniSynthVoice(void);
	
	// --- using DiodeLadder Filter here
	CDiodeLadderFilter m_LeftDiodeFilter;
	CDiodeLadderFilter m_RightDiodeFilter;

protected:
	// assumption is rectangular grid flattened out row by row
	CSampleOscillator** m_ppOscArray;
	int m_nCellsPerSide;
	int m_nCells;
	int m_nCurrentCell;

public:
	// --- Overrides

	// --- init mod matrix
	virtual void initializeModMatrix(CModulationMatrix* pMatrix);

	// --- init golbals
	inline virtual void initGlobalParameters(globalSynthParams* pGlobalParams)
	{
		// --- always call base class first
		CVectorSynthVoice::initGlobalParameters(pGlobalParams);

		// --- additional inits here
	}

	virtual void prepareForPlay();

	// NOTE: m_nCellsPerSide MUST be ODD
	bool createOscArray(int nCellsPerSide);

	inline bool initOscWithFilePath(int nOscIndex, char* pPath,  bool bSingleCycleSample, bool bPitchlessSample)
	{
		if(!m_ppOscArray) return false;
			
		((CSampleOscillator*)m_ppOscArray[nOscIndex])->m_bSingleCycleSample = bSingleCycleSample;
		((CSampleOscillator*)m_ppOscArray[nOscIndex])->m_bPitchlessSample = bPitchlessSample;

		return m_ppOscArray[nOscIndex]->initWithFilePath(pPath);
	}
	inline bool initOscWithFolderPath(int nOscIndex, char* pPath,  bool bSingleCycleSample, bool bPitchlessSample)
	{
		if(!m_ppOscArray) return false;
		
		((CSampleOscillator*)m_ppOscArray[nOscIndex])->m_bSingleCycleSample = bSingleCycleSample;
		((CSampleOscillator*)m_ppOscArray[nOscIndex])->m_bPitchlessSample = bPitchlessSample;

		return m_ppOscArray[nOscIndex]->initWithFolderPath(pPath);
	}

	inline void setActiveCell(int nCell)
	{
		int nTargetRow = nCell/m_nCellsPerSide;
		int nSkip = m_nCellsPerSide - 1;

		// A nTargetRow
		((CSampleOscillator*)m_pOsc1)->setWaveDataPtr(m_ppOscArray[nCell+nTargetRow]->getWaveData());

		// B nTargetRow+1
		((CSampleOscillator*)m_pOsc2)->setWaveDataPtr(m_ppOscArray[nCell+nTargetRow+1]->getWaveData());

		// C nTargetRow+5 (yes, 3)
		((CSampleOscillator*)m_pOsc3)->setWaveDataPtr(m_ppOscArray[nCell+nTargetRow+nSkip+3]->getWaveData());
		
		// D nTargetRow+2
		((CSampleOscillator*)m_pOsc4)->setWaveDataPtr(m_ppOscArray[nCell+nTargetRow+nSkip+2]->getWaveData());
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
		m_LeftDiodeFilter.update();
		m_RightDiodeFilter.update();

		// --- update ocillators
		m_OscA.update();
		m_OscB.update();
		m_OscC.update();
		m_OscD.update();

		// do the voice
		double dLeftA,dRightA;
		double dLeftB,dRightB;
		double dLeftC,dRightC;
		double dLeftD,dRightD;

		// if in program mode, run the JS or VPG
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

		// these are from LFO2
		double m_dRotorX = m_ModulationMatrix.m_dDestinations[DEST_VS_AC_AXIS];
		double m_dRotorY = m_ModulationMatrix.m_dDestinations[DEST_VS_BD_AXIS];

		// --- this is the different chunk for AniSynth 
		// --- making copies here because translateRotorPointToGridCell 
		//     will alter the arguments (for other manifestations not in the book)
		double dJSX = m_dVectorCenterX;
		double dJSY = m_dVectorCenterY;

		// --- calc the rotor x,y
		double dRotorX = m_dRotorX*m_dOrbitXAmp;
		double dRotorY = m_dRotorY*m_dOrbitYAmp;

		// --- find the cell for this x,y location
		int cell = translateRotorPointToGridCell(dJSX, dJSY, dRotorX, dRotorY, m_nCellsPerSide, true);

		// --- set active cell; changes pointers to samples
		if(m_nCurrentCell != cell)
		{
			m_nCurrentCell = cell;
			setActiveCell(m_nCurrentCell);
		}

		// --- AniSynth uses rotated coordinates
		bool bJoystickCoords = false;

		// --- calculate the final vector mix values based on path and rotor
		calculateVectorMixValues(0.0, 0.0,
								 dRotorX, dRotorY,
								 m_dAmplitude_A, m_dAmplitude_B, m_dAmplitude_C, m_dAmplitude_D,
								 dRotorACMix, dRotorBDMix, m_nCellsPerSide, bJoystickCoords); 
		
		// --- get our 4 osc outputs
		dLeftA = m_pOsc1->doOscillate(&dRightA);
		dLeftB = m_pOsc2->doOscillate(&dRightB);
		dLeftC = m_pOsc3->doOscillate(&dRightC);
		dLeftD = m_pOsc4->doOscillate(&dRightD);

		// --- do the massive mix
		double dLeftVectorMix = m_dAmplitude_A*dLeftA + m_dAmplitude_B*dLeftB + m_dAmplitude_C*dLeftC + m_dAmplitude_D*dLeftD;
		double dRightVectorMix = m_dAmplitude_A*dRightA + m_dAmplitude_B*dRightB + m_dAmplitude_C*dRightC + m_dAmplitude_D*dRightD;

		// --- Challenge: apply AC and BD mix to left and right filter cutoffs!
		dLeftOutput = m_LeftDiodeFilter.doFilter(dLeftVectorMix);
		dRightOutput = m_RightDiodeFilter.doFilter(dRightVectorMix);

		// --- apply the DCA
		m_DCA.doDCA(dLeftOutput, dRightOutput, dLeftOutput, dRightOutput);

		return true;
	}
};
