#include "EnvelopeGenerator.h"

CEnvelopeGenerator::CEnvelopeGenerator(void)
{	
	m_dSampleRate = 44100;

	m_dAttackTime_mSec = EG_DEFAULT_STATE_TIME;
	m_dDecayTime_mSec = EG_DEFAULT_STATE_TIME;
	m_dReleaseTime_mSec = EG_DEFAULT_STATE_TIME;

	m_dAttackTimeScalar = 1.0;
	m_dDecayTimeScalar = 1.0;
	
	m_dSustainLevel = 1.0;
	m_dEnvelopeOutput = 0.0;

	m_dIncShutdown = 0.0;
	
	// --- user normally not allowed to set the time, this initilizes it
	m_dShutdownTime_mSec = 10.0; // mSec

	// --- states and flags
	m_uState = off;
	m_bSustainOverride = false;
	m_bReleasePending = false;
	m_bOutputEG = false;
	m_uEGMode = analog;
	setEGMode(m_uEGMode);
	m_bResetToZero = false;
	m_bLegatoMode = false; 

	// --- default modulation matrix inits
	m_pModulationMatrix = NULL;

	// --- everything is disconnected unless you use mod matrix
	m_uModDestEGOutput = SOURCE_NONE;
	m_uModDestBiasedEGOutput = SOURCE_NONE;
	m_uModSourceEGAttackScaling = DEST_NONE;
	m_uModSourceEGDecayScaling = DEST_NONE;
	m_uModSourceSustainOverride = DEST_NONE;
}

CEnvelopeGenerator::~CEnvelopeGenerator(void)
{
}

void CEnvelopeGenerator::setEGMode(UINT u)
{
	// --- save it
	m_uEGMode = u;

	// --- analog - use e^-5x
	if(m_uEGMode == analog)
	{
		// these are the e^-5x over/undershoot values
		// which will generate matching e^-5x curves
		m_dAttackTCO = exp(-0.5);  // fast attack
	//	m_dAttackTCO = exp(-5.0);  
		m_dDecayTCO = exp(-5.0);  
		m_dReleaseTCO = m_dDecayTCO; 
	}
	else
	{
		// digital is linear-in-dB so use
		// the -96dB noise floor for over/undershoot
		m_dAttackTCO = pow(10.0, -96.0/20.0);
		m_dDecayTCO = m_dAttackTCO;
		m_dReleaseTCO = m_dDecayTCO; 
	}
	/*
		NOTE: for Attack/Release only EGs, set these to give a smooth
		      charge/discharge curve since there is no Sustain bais
			  voltage on the cap:
		m_dDecayTCO = 0.99632;
		m_dReleaseTCO = 0.00368; 
	*/

	// --- recalc these
	calculateAttackTime();
	calculateDecayTime();
	calculateReleaseTime();
}

// reset
void CEnvelopeGenerator::reset()
{
	// --- clear scalars
	m_dAttackTimeScalar = 1.0;
	m_dDecayTimeScalar = 1.0;

	// --- state
	m_uState = off;
	
	// --- Note: sustain pedal may still be down
	//           so it is a sticky variable
	m_bReleasePending = false;
		
	// --- reset 
	setEGMode(m_uEGMode);

	// --- may be modified in noteOff()
	calculateReleaseTime();
	
	// --- if reset to zero, clear
	//     else let it stay frozen
	if(m_bResetToZero)
	{
		m_dEnvelopeOutput = 0.0;
	} 
}
	
void CEnvelopeGenerator::calculateAttackTime()
{
	// --- samples for the exponential rate
	double dSamples = m_dSampleRate*((m_dAttackTimeScalar*m_dAttackTime_mSec)/1000.0);

	// --- coeff and base for iterative exponential calculation
	m_dAttackCoeff = exp(-log((1.0 + m_dAttackTCO)/m_dAttackTCO)/dSamples);	
	m_dAttackOffset = (1.0 + m_dAttackTCO)*(1.0 - m_dAttackCoeff);
}

// --- changes to the decay rate OR the sustain level require
//     an update in analog mode
void CEnvelopeGenerator::calculateDecayTime()
{
	// --- samples for the exponential rate
	double dSamples = m_dSampleRate*((m_dDecayTimeScalar*m_dDecayTime_mSec)/1000.0);
	
	// --- coeff and base for iterative exponential calculation
	m_dDecayCoeff = exp(-log((1.0 + m_dDecayTCO)/m_dDecayTCO)/dSamples);
	m_dDecayOffset = (m_dSustainLevel - m_dDecayTCO)*(1.0 - m_dDecayCoeff);
}

void CEnvelopeGenerator::calculateReleaseTime()
{
	// --- samples for the exponential rate
	double dSamples = m_dSampleRate*(m_dReleaseTime_mSec/1000.0);

	// --- coeff and base for iterative exponential calculation
	m_dReleaseCoeff = exp(-log((1.0 + m_dReleaseTCO)/m_dReleaseTCO)/dSamples);
	m_dReleaseOffset = -m_dReleaseTCO*(1.0 - m_dReleaseCoeff);	
}

// --- go to release state; reset 
void CEnvelopeGenerator::noteOff()
{
	if(m_bSustainOverride)
	{
		// --- set m_bReleasePending flag
		m_bReleasePending = true;
		return;
	}

	// --- go directly to release state
	if(m_dEnvelopeOutput > 0)
		m_uState = release;
	else // sustain was already at zero
		m_uState = off;
}

// --- goto shutdown state
void CEnvelopeGenerator::shutDown()
{
	// --- legato mode - ignore
	if(m_bLegatoMode)
		return;
	
	// --- calculate the linear inc values based on current outputs
	m_dIncShutdown = -(1000.0*m_dEnvelopeOutput)/m_dShutdownTime_mSec/m_dSampleRate;

	// --- set state and reset counter
	m_uState = shutdown;

	// --- for sustain pedal
	m_bSustainOverride = false;
	m_bReleasePending = false;
}

