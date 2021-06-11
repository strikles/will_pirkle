#pragma once

#ifndef __MoogLPF__
#define __MoogLPF__

#include "fxobjects.h"

/**
\struct MoogLPFParameters
\ingroup FX-Objects
\brief
Custom parameter structure for the MoogLPF object.

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
struct MoogLPFParameters
{
	MoogLPFParameters() {}

	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	MoogLPFParameters& operator=(const MoogLPFParameters& params)	// need this override for collections to work
	{
		// --- it is possible to try to make the object equal to itself
		//     e.g. thisObject = thisObject; so this code catches that
		//     trivial case and just returns this object
		if (this == &params)
			return *this;

		// --- copy from params (argument) INTO our variables
		fc = params.fc;
		Q = params.Q;
		bassBoost_Pct = params.bassBoost_Pct;
		enableAutoLimiter = params.enableAutoLimiter;

		// --- MUST be last
		return *this;
	}

	double fc = 0.0;				// --- Hz
	double Q = 1.0;					// --- Q = [1.0, 10.0]
	double bassBoost_Pct = 0.0;		// --- this will restore bass [0, 100] = [no compensation, full bass]
	bool enableAutoLimiter = false;
};


// --- MOOG LPF with Biquads
class MoogLPF : public IAudioSignalProcessor
{
public:
	MoogLPF(void) {}	/* C-TOR */
	~MoogLPF(void) {}	/* D-TOR */

public:
	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
	{
		// --- store the sample rate
		sampleRate = _sampleRate;

		// --- setup the audio filters
		AudioFilterParameters params = lowpassFilters[0].getParameters();
		params.algorithm = filterAlgorithm::kLPF1;
		params.fc = parameters.fc;

		for (int i = 0; i < 4; i++)
		{
			lowpassFilters[i].reset(sampleRate);
			lowpassFilters[i].setParameters(params);
		}

		// --- setup the peak limiter
		peakLimiter.reset(_sampleRate);

		// --- 3dB threshold is good compromise, sounds good
		peakLimiter.setThreshold_dB(-3.0);

		return true;
	}

	/** process MONO input */
	/**
	\param xn input
	\return the processed sample
	*/
	virtual double processAudioSample(double xn)
	{
		// --- gather the S(n) values
		double S1 = lowpassFilters[0].getS_value();
		double S2 = lowpassFilters[1].getS_value();
		double S3 = lowpassFilters[2].getS_value();
		double S4 = lowpassFilters[3].getS_value();

		// --- setup coefficients
		double G = lowpassFilters[0].getG_value();
		double dSigma = G*G*G*S1 + G*G*S2 + G*S3 + S4;
		double alpha0 = 1.0 / (1.0 + K*G*G*G*G);

		// --- bass compensation (see notes) gain = 1 + aK
		double compensationGain = 1.0 + (parameters.bassBoost_Pct / 100.0) * K;

		// --- scale input BEFORE entering loop
		xn *= compensationGain;

		// calculate input to first filter
		double u = (xn - K*dSigma)*alpha0;

		// --- process all filters in sequence
		double LPF0 = lowpassFilters[0].processAudioSample(u);
		double LPF1 = lowpassFilters[1].processAudioSample(LPF0);
		double LPF2 = lowpassFilters[2].processAudioSample(LPF1);
		double LPF3 = lowpassFilters[3].processAudioSample(LPF2);

		// --- peak limiter is OUTSIDE the loop! 
		double limiterOutput = parameters.enableAutoLimiter ? peakLimiter.processAudioSample(LPF3) : LPF3;

		// --- done
		return limiterOutput;
	}

	/** query to see if this object can process frames */
	virtual bool canProcessAudioFrame() { return false; } // <-- change this!

	/** process audio frame: implement this function if you answer "true" to above query */
	virtual bool processAudioFrame(const float* inputFrame,	/* ptr to one frame of data: pInputFrame[0] = left, pInputFrame[1] = right, etc...*/
					     float* outputFrame,
					     uint32_t inputChannels,
					     uint32_t outputChannels)
	{
		// --- do nothing
		return false; // NOT handled
	}


	/** get parameters: note use of custom structure for passing param data */
	/**
	\return TestObjectParameters custom data structure
	*/
	MoogLPFParameters getParameters()
	{
		return parameters;
	}

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param TestObjectParameters custom data structure
	*/
	void setParameters(const MoogLPFParameters& params)
	{
		// --- copy them; note you may choose to ignore certain items
		//     and copy the variables one at a time, or you may test
		//     to see if cook-able variables have changed; if not, then
		//     do not re-cook them as it just wastes CPU
		parameters = params;
		calculateCoefficients();
	}

	void calculateCoefficients()
	{
		// --- Q is 1 -> 10 for my plugins; just map it to 0 -> 4
		K = (4.0)*(parameters.Q - 1.0) / (10.0 - 1.0);

		// --- update filters
		AudioFilterParameters params = lowpassFilters[0].getParameters();
		params.fc = parameters.fc;

		for (int i = 0; i < 4; i++)
		{
			lowpassFilters[i].setParameters(params);
		}
	}

private:
	MoogLPFParameters parameters; ///< object parameters

	// --- array of 4 LPFs
	AudioFilter lowpassFilters[4];

	double K = 0.0; // feedback control

	// --- local variables used by this object
	double sampleRate = 0.0;	///< sample rate

	// --- optional peak limiter as saturator
	PeakLimiter peakLimiter;
};



#endif