#pragma once
#include "DelayLine.h"
#include "pluginconstants.h"

class CStereoDelayFX
{
public:
	CStereoDelayFX(void);
	virtual ~CStereoDelayFX(void);

protected:
	// --- our delay lines
	CDelayLine m_LeftDelay;
	CDelayLine m_RightDelay;

	// --- user control variables
	double m_dDelayTime_mSec;
	double m_dFeedback_Pct;
	double m_dDelayRatio;	// -0.9 to + 0.9
	double m_dWetMix;		// 0 to 1.0

	double m_dTap2LDelayTime_mSec;
	double m_dTap2RDelayTime_mSec;

	// --- mode of operation
	UINT m_uMode;
	enum {norm,tap1,tap2,pong};

public:
	// --- set functions for owning objet to call
	void setMode(UINT u){m_uMode = u;}
	void setDelayTime_mSec(double d){m_dDelayTime_mSec = d;}
	void setFeedback_Pct(double d){m_dFeedback_Pct = d;}
	void setDelayRatio(double d){m_dDelayRatio = d;}
	void setWetMix(double d){m_dWetMix = d;}

	// --- functions for owning object
	void prepareForPlay(double dSampleRate);	
	void reset();
	void update();
	bool processAudio(double* pInputL, double* pInputR, double* pOutputL,double* pOutputR);

};
