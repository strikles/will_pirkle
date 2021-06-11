#include "QBLimitedOscillator.h"

CQBLimitedOscillator::CQBLimitedOscillator(void)
{
}

CQBLimitedOscillator::~CQBLimitedOscillator(void)
{
}

void CQBLimitedOscillator::reset()
{
	COscillator::reset();

	// --- saw/tri starts at 0.5
	if(m_uWaveform == SAW1 || m_uWaveform == SAW2 ||
	   m_uWaveform == SAW3 || m_uWaveform == TRI)
	{
		m_dModulo = 0.5;
	}
}

void CQBLimitedOscillator::startOscillator()
{
	reset();
	m_bNoteOn = true;
}

void CQBLimitedOscillator::stopOscillator()
{
	m_bNoteOn = false;
}
