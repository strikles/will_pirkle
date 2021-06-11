#include "VectorPathGenerator.h"

CVectorPathGenerator::CVectorPathGenerator()
{
	m_bRunning = false;
	m_uSampleCount = 0;
	m_dTimerDurationMSec = 0;
	m_nCurrentProgramStep = 0;
	m_dA_Mix = 0.25;
	m_dB_Mix = 0.25;
	m_dC_Mix = 0.25;
	m_dD_Mix = 0.25;
	m_dAC_Mix = 0.0;
	m_dBD_Mix = 0.0;
	m_uJSMode = JS_ONESHOT;
	m_bForwardPath = true;

	// create and clear the table
	m_pVPGProgramTable = new double[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	memset(m_pVPGProgramTable, 0, MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS*sizeof(double));
}

CVectorPathGenerator::~CVectorPathGenerator(void)
{
	delete [] m_pVPGProgramTable;
}


// these are rotated from Joystick in "anisotropic" layout so that
/*	
	bJoystickCoords = FALSE:
	A = (-1,1)
	B = (1,1)
	C = (1,-1)
	D = (-1,-1)

	bJoystickCoords = TRUE:
	A = (-1,0)
	B = (0,1)
	C = (1,0)
	D = (0,-1)

	origin: (0,0)
*/ 
void CVectorPathGenerator::setProgramStep(int nIndex, double dX, double dY, double dTimeToNextStep_mSec, bool bJoystickCoords)
{
	if(nIndex > MAX_JS_PROGRAM_STEPS || nIndex < 0)
		return;

	double dA, dB, dC, dD, dAC, dBD = 0.0;

	// calculate them
	calculateVectorMixValues(0.0, 0.0, dX, dY, dA, dB, dC, dD, dAC, dBD, bJoystickCoords); 

	// add to table
	//	0:A_mix | 1:B_mix | 2:C_mix | 3:D_mix | 4:MorphTime to Next Step(mSec) | 5:AC_mix | 6:BD_mix
	m_pVPGProgramTable[JS_PROG_INDEX(nIndex,0)] = dA;
	m_pVPGProgramTable[JS_PROG_INDEX(nIndex,1)] = dB;
	m_pVPGProgramTable[JS_PROG_INDEX(nIndex,2)] = dC;
	m_pVPGProgramTable[JS_PROG_INDEX(nIndex,3)] = dD;
	m_pVPGProgramTable[JS_PROG_INDEX(nIndex,4)] = dTimeToNextStep_mSec;
	m_pVPGProgramTable[JS_PROG_INDEX(nIndex,5)] = dAC;
	m_pVPGProgramTable[JS_PROG_INDEX(nIndex,6)] = dBD;
}
