#include "VectorSynthVoice.h"

CVectorSynthVoice::CVectorSynthVoice(void)
{
	// --- declare your oscillators and filters
	m_pOsc1 = &m_OscA;
	m_pOsc2 = &m_OscB;
	m_pOsc3 = &m_OscC;
	m_pOsc4 = &m_OscD;
	    
	m_pFilter1 = &m_LeftK35Filter;
	m_pFilter2 = &m_RightK35Filter;
		
	// --- for voice stealing
	m_EG1.m_bResetToZero = true;
	m_EG2.m_bResetToZero = true;
	m_EG3.m_bResetToZero = true;
	m_EG4.m_bResetToZero = true;

	// --- experiment with NLP
	m_LeftK35Filter.m_uNLP = ON;
	m_RightK35Filter.m_uNLP = ON;

	m_EG1.setEGMode(analog);
	m_EG1.m_bOutputEG = true;

	// --- VS Specific
	m_dAmplitude_A = 0.25;
	m_dAmplitude_B = 0.25;
	m_dAmplitude_C = 0.25;
	m_dAmplitude_D = 0.25;
	m_dAmplitude_ACmix = 0.5;
	m_dAmplitude_BDmix = 0.5;
	m_dVectorCenterX = 0.0;
	m_dVectorCenterY = 0.0;
	m_dOrbitXAmp = 0.0;
	m_dOrbitYAmp = 0.0;

	// --- plug-in must set this RAFX only!!
	m_pJSProgram = NULL;
	m_uVoiceMode = manual;

	// example of programmatically creating a vector path
	// without RAFX program generator
	// 
	// -- Setup vector path generator; this just moves around from 
	//    one apex to the next.
	// start at 100% A morph to next step in 2 sec
	// start at 100% B morph to next step in 2 sec
	// start at 100% C morph to next step in 2 sec
	// start at 100% D morph to BEGINNING in 2 sec (if LOOP mode)	
	//
#ifdef _RAFX
	// -- the true argument means use thKorg/RAFX coordinates
	m_VPG.setProgramStep(0, -1.0, 0.0, 2000, true); 
	m_VPG.setProgramStep(1, 0.0, 1.0, 2000, true); 
	m_VPG.setProgramStep(2, 1.0, 0.0, 2000, true); 
	m_VPG.setProgramStep(3, 0.0, -1.0, 2000, true); 

	// --- RAFX version, VPG is disabled so you can use the easy to 
	//     program JoystickProgram in RAFX (hit the Program button)
	m_bEnableVPG = false;
	// m_bEnableVPG = true; //<-- uncomment to use VPG instead of JSProgram
#else
	// -- the false argument means use the rotated coordinates (Moog, VST3, AU versions)
	m_VPG.setProgramStep(0, -1.0, 1.0, 2000, false); 
	m_VPG.setProgramStep(1, 1.0, 1.0, 2000, false); 
	m_VPG.setProgramStep(2, 1.0, -1.0, 2000, false); 
	m_VPG.setProgramStep(3, -1.0, -1.0, 2000, false); 
		
	// --- for VST3/AU, there is no built-in program generator, so do it by hand here
	m_bEnableVPG = true;
#endif

}
void CVectorSynthVoice::initializeModMatrix(CModulationMatrix* pMatrix)
{
	// --- always first: call base class to create core and init with basic routings
	CDigiSynthVoice::initializeModMatrix(pMatrix);

	if(!pMatrix->getModMatrixCore()) return;

	modMatrixRow* pRow = NULL;

	// --- VS uses LFO2 as "Rotor"
	//
	// LFO2 -> VS AC Axis Mod
	pRow = createModMatrixRow(SOURCE_LFO2,
							  DEST_VS_AC_AXIS,
							  &m_dDefaultModIntensity,
							  &m_dDefaultModRange,
							  TRANSFORM_NONE,
							  true);
	pMatrix->addModMatrixRow(pRow);
	
	// LFO2Quad -> VS BD Axis Mod
	pRow = createModMatrixRow(SOURCE_LFO2Q,
							  DEST_VS_BD_AXIS,
							  &m_dDefaultModIntensity,
							  &m_dDefaultModRange,
							  TRANSFORM_NONE,
							  true);
	pMatrix->addModMatrixRow(pRow);
}

CVectorSynthVoice::~CVectorSynthVoice(void)
{
}

void CVectorSynthVoice::setSampleRate(double dSampleRate)
{
	CDigiSynthVoice::setSampleRate(dSampleRate);
	
	m_VPG.setSampleRate((float)dSampleRate);

	if(m_pJSProgram)
		m_pJSProgram->setSampleRate((int)dSampleRate);
}

void CVectorSynthVoice::prepareForPlay()
{
	CDigiSynthVoice::prepareForPlay();
}

void CVectorSynthVoice::reset()
{
	CDigiSynthVoice::reset();
}

void CVectorSynthVoice::update()
{
	// --- voice specific updates
	if(!m_pGlobalVoiceParams) return;

	// --- call base class first 
	CDigiSynthVoice::update();

	// --- VS Specific
	//
	// --- path mode
	m_uVectorPathMode = m_pGlobalVoiceParams->uVectorPathMode;
			
	// --- for RAFX
	if(m_pJSProgram)
		m_pJSProgram->setJSMode(m_uVectorPathMode);

	// --- for everyone else (or RAFX)
	m_VPG.setVPGPathMode(m_uVectorPathMode);

	// --- vector joystick params
	m_dAmplitude_A = m_pGlobalVoiceParams->dAmplitude_A;
	m_dAmplitude_B = m_pGlobalVoiceParams->dAmplitude_B;
	m_dAmplitude_C = m_pGlobalVoiceParams->dAmplitude_C;
	m_dAmplitude_D = m_pGlobalVoiceParams->dAmplitude_D;
	m_dAmplitude_ACmix = m_pGlobalVoiceParams->dAmplitude_ACmix;
	m_dAmplitude_BDmix = m_pGlobalVoiceParams->dAmplitude_BDmix;
	m_dVectorCenterX = unipolarToBipolar(m_dAmplitude_ACmix);
	m_dVectorCenterY = unipolarToBipolar(m_dAmplitude_BDmix);
	m_dOrbitXAmp = m_pGlobalVoiceParams->dOrbitXAmp;
	m_dOrbitYAmp = m_pGlobalVoiceParams->dOrbitYAmp;
}
