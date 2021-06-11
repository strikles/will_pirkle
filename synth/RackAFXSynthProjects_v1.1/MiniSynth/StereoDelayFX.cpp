#include "StereoDelayFX.h"

CStereoDelayFX::CStereoDelayFX(void)
{
	// --- zero everything
	m_dDelayTime_mSec = 0.0;
	m_dFeedback_Pct = 0.0;
	m_dDelayRatio = 0.0; // 1:1
	m_dWetMix = 0.5;	// 50%		
}

CStereoDelayFX::~CStereoDelayFX(void)
{
}

void CStereoDelayFX::reset()
{
	// --- flush buffers and reset index values
	m_LeftDelay.resetDelay();
	m_RightDelay.resetDelay();
}

// --- one time init
void CStereoDelayFX::prepareForPlay(double dSampleRate)
{
	// --- set sample rate first
	m_LeftDelay.setSampleRate((int)dSampleRate);
	m_RightDelay.setSampleRate((int)dSampleRate);

	// --- initialize to 2 sec max delay
	m_LeftDelay.init(2.0*dSampleRate);
	m_RightDelay.init(2.0*dSampleRate);

	// --- do the flush
	reset();
}

void CStereoDelayFX::update()
{
	// --- set the delay times based on the ratio control
	//     if (-) bias to left
	//     if (+) bias to right
	//     if 0.0 ratio is 1:1
	if(m_dDelayRatio < 0)
	{
		// --- note negation of ratio!
		m_LeftDelay.setDelay_mSec(-m_dDelayRatio*m_dDelayTime_mSec);
		m_RightDelay.setDelay_mSec(m_dDelayTime_mSec);
	}
	else if(m_dDelayRatio > 0)
	{
		m_LeftDelay.setDelay_mSec(m_dDelayTime_mSec);
		m_RightDelay.setDelay_mSec(m_dDelayRatio*m_dDelayTime_mSec);
	}
	else
	{
		m_LeftDelay.setDelay_mSec(m_dDelayTime_mSec);
		m_RightDelay.setDelay_mSec(m_dDelayTime_mSec);
	}
}

	
bool CStereoDelayFX::processAudio(double* pInputL, double* pInputR, 
								  double* pOutputL,double* pOutputR)
{
	// --- do the delays
	//     common components:
	double dLeftDelayOut = m_LeftDelay.readDelay();
	double dRightDelayOut = m_RightDelay.readDelay();

	// --- inputs to delays;default is norm
	double dLeftDelayIn = *pInputL + dLeftDelayOut*(m_dFeedback_Pct/100.0);
	double dRightDelayIn = *pInputR + dRightDelayOut*(m_dFeedback_Pct/100.0);

	// --- do the other modes:
	switch(m_uMode)
	{
		// --- NOTE: cross mode sounds identical to ping-pong if
		//     the input is mono (same signal applied to both)
		case cross:
		{
			// Left Input -> Left Delay; Left Feedback -> Right Delay
			// Right Input -> Right Delay; Right Feedback -> Left Delay
			dLeftDelayIn = *pInputL + dRightDelayOut*(m_dFeedback_Pct/100.0);
			dRightDelayIn = *pInputR + dLeftDelayOut*(m_dFeedback_Pct/100.0);
			break;
		}
		case pingpong:
		{
			// Left Input -> Right Delay; Left Feedback -> Right Delay
			// Right Input -> Left Delay; Right Feedback -> Left Delay
			dLeftDelayIn = *pInputR + dRightDelayOut*(m_dFeedback_Pct/100.0);
			dRightDelayIn = *pInputL + dLeftDelayOut*(m_dFeedback_Pct/100.0);
			break;
		}
	}

	// --- intermediate variables
	double dLeftOut = 0.0;
	double dRightOut = 0.0;

	// --- do the delay lines
	m_LeftDelay.processAudio(&dLeftDelayIn, &dLeftOut);
	m_RightDelay.processAudio(&dRightDelayIn, &dRightOut);

	// --- form outputs
	*pOutputL = *pInputL + m_dWetMix*dLeftOut;
	*pOutputR = *pInputR + m_dWetMix*dRightOut;

	// --- return All OK
	return true;
}
