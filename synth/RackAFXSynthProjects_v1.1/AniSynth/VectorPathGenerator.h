#pragma once

#include "synthfunctions.h"

class CVectorPathGenerator
{
public:	// Functions
	//        
	// (1) Initialize here:
	CVectorPathGenerator();

	// these are rotated from Joystick in "anisotropic" layout so that
	/*	A = (-1,1)
		B = (1,1)
		C = (1,-1)
		D = (-1,-1)
		origin: (0,0)
	*/ 
	void setProgramStep(int nIndex, double dX, double dY, double dTimeToNextStep_mSec, bool bJoystickCoords);

	// One Time Destruction
	~CVectorPathGenerator(void);

protected:
	UINT m_uSampleCount;
	UINT m_uJSMode;
	bool m_bRunning;
	bool m_bForwardPath;

	double m_dTimerDurationMSec;
	double m_dSampleRate;
	int  m_nTimerDurationSamples;
	int m_nCurrentProgramStep;
	int m_nNumSteps;

	double m_dStartA_Mix;
	double m_dStartB_Mix;
	double m_dStartC_Mix;
	double m_dStartD_Mix;

	double m_dStartAC_Mix;
	double m_dStartBD_Mix;

	double m_dEndA_Mix;
	double m_dEndB_Mix;
	double m_dEndC_Mix;
	double m_dEndD_Mix;

	double m_dEndAC_Mix;
	double m_dEndBD_Mix;

	double m_dA_Mix;
	double m_dB_Mix;
	double m_dC_Mix;
	double m_dD_Mix;

	double m_dAC_Mix;
	double m_dBD_Mix;

	double* m_pVPGProgramTable;

public:
	/*
		The functions that access the program table use the MACRO

		JS_PROG_INDEX(i,j)

		i = row, j = column

		To implement a 2D Array in a flattened out manner which eliminiates
		issues with passing pointers to the array and makes it simple to set or
		get a cell in the array.

		The Rows are a STEP in the program.
		The Columns are:

		0:A_mix | 1:B_mix | 2:C_mix | 3:D_mix | 4:MorphTime to Next Step(mSec) | 5:AC_mix | 6:BD_mix
	*/

	inline void startProgram()
	{
		m_nNumSteps = 0;
		m_uSampleCount = 0;

		for(int i=1; i<MAX_JS_PROGRAM_STEPS; i++)
		{
			if(m_pVPGProgramTable[JS_PROG_INDEX(i,4)] > 0)
				m_nNumSteps++;
		}

		if(m_nNumSteps == 0)
			return;

		m_bForwardPath = true;
		m_nCurrentProgramStep = 1;

		m_dStartA_Mix = m_pVPGProgramTable[JS_PROG_INDEX(0,0)];
		m_dStartB_Mix = m_pVPGProgramTable[JS_PROG_INDEX(0,1)];
		m_dStartC_Mix = m_pVPGProgramTable[JS_PROG_INDEX(0,2)];
		m_dStartD_Mix = m_pVPGProgramTable[JS_PROG_INDEX(0,3)];
		m_dStartAC_Mix = m_pVPGProgramTable[JS_PROG_INDEX(0,5)];
		m_dStartBD_Mix = m_pVPGProgramTable[JS_PROG_INDEX(0,6)];

		m_dEndA_Mix = m_pVPGProgramTable[JS_PROG_INDEX(1,0)];
		m_dEndB_Mix = m_pVPGProgramTable[JS_PROG_INDEX(1,1)];
		m_dEndC_Mix = m_pVPGProgramTable[JS_PROG_INDEX(1,2)];
		m_dEndD_Mix = m_pVPGProgramTable[JS_PROG_INDEX(1,3)];
		m_dEndAC_Mix = m_pVPGProgramTable[JS_PROG_INDEX(1,5)];
		m_dEndBD_Mix = m_pVPGProgramTable[JS_PROG_INDEX(1,6)];

		m_dTimerDurationMSec = m_pVPGProgramTable[JS_PROG_INDEX(0,4)];
		m_nTimerDurationSamples = (int)((m_dTimerDurationMSec/1000.0)*(m_dSampleRate));

		m_bRunning = true;
	}

	inline void pauseProgram(){m_bRunning = false;}
	inline void resumeProgram(){m_bRunning = true;}
	
	inline void reset()
	{
		m_uSampleCount = 0; 
		m_bRunning = false; 
		m_bForwardPath = true; 
		m_dTimerDurationMSec = 0; 
		m_nCurrentProgramStep = 0;
	}
		
	inline void calculateCurrentVectorMix()
	{	
		float m = (m_dEndA_Mix - m_dStartA_Mix)/(float)m_nTimerDurationSamples;
		m_dA_Mix = m*(float)m_uSampleCount + m_dStartA_Mix;

		m = (m_dEndB_Mix - m_dStartB_Mix)/(float)m_nTimerDurationSamples;
		m_dB_Mix = m*(float)m_uSampleCount + m_dStartB_Mix;

		m = (m_dEndC_Mix - m_dStartC_Mix)/(float)m_nTimerDurationSamples;
		m_dC_Mix = m*(float)m_uSampleCount + m_dStartC_Mix;

		m = (m_dEndD_Mix - m_dStartD_Mix)/(float)m_nTimerDurationSamples;
		m_dD_Mix = m*(float)m_uSampleCount + m_dStartD_Mix;

		m = (m_dEndAC_Mix - m_dStartAC_Mix)/(float)m_nTimerDurationSamples;
		m_dAC_Mix = m*(float)m_uSampleCount + m_dStartAC_Mix;

		m = (m_dEndBD_Mix - m_dStartBD_Mix)/(float)m_nTimerDurationSamples;
		m_dBD_Mix = m*(float)m_uSampleCount + m_dStartBD_Mix;
	}

	inline void setVPGPathMode(UINT u)
	{
		m_uJSMode = u;
		m_bForwardPath = true;
	}

	inline int getCurrentStep(){return m_nCurrentProgramStep;}

	// (2) Set the sample rate in prepareForPlay()
	inline void setSampleRate(double d){m_dSampleRate = d;}

	// (3) call this once per sample period
	inline void incTimer()
	{
		if(!m_bRunning)
			return;

		m_uSampleCount++;

		calculateCurrentVectorMix();

		if(m_uSampleCount > (UINT)m_nTimerDurationSamples)
		{
			// goto the next step
			if(m_bForwardPath)
				m_nCurrentProgramStep++;
			else
				m_nCurrentProgramStep--;

			if(m_nCurrentProgramStep > m_nNumSteps || m_nCurrentProgramStep < 0)
			{
				if(m_uJSMode == JS_ONESHOT)
				{
					reset();
					return;
				}
				else if(m_uJSMode == JS_LOOP)
				{
					m_nCurrentProgramStep = 0;
				}
				else if(m_uJSMode == JS_LOOP_BACKANDFORTH)
				{
					m_bForwardPath = !m_bForwardPath;
					if(m_bForwardPath)
						m_nCurrentProgramStep +=2;
					else
						m_nCurrentProgramStep -= 2;
				}
				else if(m_uJSMode == JS_SUSTAIN)
				{
					m_bRunning = false;
					return; // just return
				}
			}
			else if(m_uJSMode == JS_SUSTAIN && m_nCurrentProgramStep == m_nNumSteps)
			{
				pauseProgram(); // until user restarts with a note-off event
			}

			// setup for next step
			m_uSampleCount = 0;

			m_dStartA_Mix = m_dEndA_Mix;
			m_dStartB_Mix = m_dEndB_Mix;
			m_dStartC_Mix = m_dEndC_Mix;
			m_dStartD_Mix = m_dEndD_Mix;
			m_dStartAC_Mix = m_dEndAC_Mix;
			m_dStartBD_Mix = m_dEndBD_Mix;

			m_dEndA_Mix = m_pVPGProgramTable[JS_PROG_INDEX(m_nCurrentProgramStep,0)];
			m_dEndB_Mix = m_pVPGProgramTable[JS_PROG_INDEX(m_nCurrentProgramStep,1)];
			m_dEndC_Mix = m_pVPGProgramTable[JS_PROG_INDEX(m_nCurrentProgramStep,2)];
			m_dEndD_Mix = m_pVPGProgramTable[JS_PROG_INDEX(m_nCurrentProgramStep,3)];
			m_dEndAC_Mix = m_pVPGProgramTable[JS_PROG_INDEX(m_nCurrentProgramStep,5)];
			m_dEndBD_Mix = m_pVPGProgramTable[JS_PROG_INDEX(m_nCurrentProgramStep,6)];

			if(m_bForwardPath)
				m_dTimerDurationMSec = m_nCurrentProgramStep > 0 ? m_pVPGProgramTable[JS_PROG_INDEX(m_nCurrentProgramStep-1,4)] : m_pVPGProgramTable[JS_PROG_INDEX(m_nNumSteps,4)];
			else
				m_dTimerDurationMSec = m_pVPGProgramTable[JS_PROG_INDEX(m_nCurrentProgramStep,4)];

			m_nTimerDurationSamples = (int)((m_dTimerDurationMSec/1000.0)*(m_dSampleRate));
		}
	}

	// (4) get the current vector mix ratios
	inline void getVectorMixValues(double& dA, double& dB, double& dC, double& dD)
	{
		dA = m_dA_Mix; dB = m_dB_Mix, dC = m_dC_Mix, dD = m_dD_Mix;
	}

	inline void getVectorACBDMixes(double& dAC, double& dBD)
	{
		dAC = m_dAC_Mix; dBD = m_dBD_Mix;
	}
};
