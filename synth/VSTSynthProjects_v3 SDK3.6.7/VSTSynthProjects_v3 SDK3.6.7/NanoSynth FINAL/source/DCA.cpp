#include "DCA.h"

CDCA::CDCA(void)
{
	// --- initialize variables
	m_dAmplitudeControl = 1.0;
	m_dAmpMod_dB = 0.0;
	m_dGain = 1.0;
	m_dAmplitude_dB = 0.0;
	m_dEGMod = 1.0;
	m_dPanControl = 0.0;
	m_dPanMod = 0.0;
	m_uMIDIVelocity = 127;

	// --- default modulation matrix inits
	m_pModulationMatrix = NULL;

	// --- everything is disconnected unless you use mod matrix
	m_uModSourceEG = DEST_NONE;
	m_uModSourceAmp_dB = DEST_NONE;
	m_uModSourceVelocity = DEST_NONE;
	m_uModSourcePan = DEST_NONE;

	// --- disable 
	m_pGlobalDCAParams = NULL;
}

// --- destruction
CDCA::~CDCA(void)
{
}
