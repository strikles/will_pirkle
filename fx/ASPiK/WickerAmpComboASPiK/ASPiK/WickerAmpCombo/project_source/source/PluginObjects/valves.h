#pragma once

#ifndef __ValvesTubes__
#define __ValvesTubes__

#include "fxobjects.h"

// --- helper
inline double calcMappedVariableOnRange(double inLow, double inHigh, 
										double outLow, double outHigh, 
										double control, bool convertFromDB = false)
{
	// --- mapper 
	double slope = (outHigh - outLow) / (inHigh - inLow);
	double yn = outLow + slope * (control - inLow);
	if (convertFromDB)
		return pow(10.0, yn / 20.0);
	else
		return yn;
}


/**
\struct ClassAValveParameters
\ingroup FX-Objects
\brief parameter structure for ClassAValve object

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
struct ClassAValveParameters
{
	ClassAValveParameters() {}

	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	ClassAValveParameters& operator=(const ClassAValveParameters& params)	// need this override for collections to work
	{
		// --- it is possible to try to make the object equal to itself
		//     e.g. thisObject = thisObject; so this code catches that
		//     trivial case and just returns this object
		if (this == &params)
			return *this;

		// --- copy from params (argument) INTO our variables
		lowFrequencyShelf_Hz = params.lowFrequencyShelf_Hz;
		lowFrequencyShelfGain_dB = params.lowFrequencyShelfGain_dB;
		dcBlockingLF_Hz = params.dcBlockingLF_Hz;
		millerHF_Hz = params.millerHF_Hz;
		clipPointPositive = params.clipPointPositive;
		clipPointNegative = params.clipPointNegative;
		dcShiftCoefficient = params.dcShiftCoefficient;
		dcOffsetDetected = params.dcOffsetDetected;
		gridConductionThreshold = params.gridConductionThreshold;
		waveshaperSaturation = params.waveshaperSaturation;
		dcOffsetDetected = params.dcOffsetDetected;
		inputGain = params.inputGain;
		outputGain = params.outputGain;
		integratorFc = params.integratorFc;

		// --- MUST be last
		return *this;
	}
	
	// --- for emulation of LF shelf from cathode bypass cap
	double lowFrequencyShelf_Hz = 100.0;
	double lowFrequencyShelfGain_dB = 0.0;

	// --- for emulation of output HPF for DC blocking
	double dcBlockingLF_Hz = 10.0;

	// --- for emulation of parasitic Miller Cap that reduces HF
	double millerHF_Hz = 10000.0; 
	
	// --- can adjust this too
	double integratorFc = 5.0;

	// --- clip and threshold points
	double clipPointPositive = 4.0;		// --- SPICE data
	double clipPointNegative = -1.5;	// --- book design uses these curves
	double gridConductionThreshold = 1.5;		// --- using max input swing

	double dcShiftCoefficient = 1.0;	// --- this can scale the added DC offet amount; makes a HUGE difference! 
	double waveshaperSaturation = 2.0;	// --- the (k) value

	// --- I/O scaling: note that you can play with these to really modify scaling
	double inputGain = 1.5;				// --- this is because our bias is -1.5V
	double outputGain = 1.0;			// --- reduce output back to [-1, +1]

	// --- return data (optional)
	double dcOffsetDetected = 0.0;
};


/**
\class ClassAValve
\ingroup FX-Objects
\brief
Emulates a Class-A 12AX7 tube (valve)

Audio I/O:
- Processes mono input to mono output.

Control I/F:
- Use ClassAValveParameters structure to get/set object params.

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
class ClassAValve : public IAudioSignalProcessor
{
public:
	ClassAValve(void) {}	/* C-TOR */
	~ClassAValve(void) {}	/* D-TOR */

public:
	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
	{
		// --- do any other per-audio-run inits here
		sampleRate = _sampleRate;

		// --- integrators
		lossyIntegrator[0].reset(_sampleRate);
		lossyIntegrator[1].reset(_sampleRate);

		ZVAFilterParameters params = lossyIntegrator[0].getParameters();
		params.filterAlgorithm = vaFilterAlgorithm::kSVF_LP;
		params.Q = 0.707; // 5 Hz
		params.fc = 5.0;	// 5 Hz
		lossyIntegrator[0].setParameters(params);
		lossyIntegrator[1].setParameters(params);

		// --- other filters
		//
		// --- low shelf
		lowShelvingFilter.reset(_sampleRate);
		AudioFilterParameters filterParams = lowShelvingFilter.getParameters();
		filterParams.algorithm = filterAlgorithm::kLowShelf;
		filterParams.fc = parameters.lowFrequencyShelf_Hz;
		filterParams.boostCut_dB = parameters.lowFrequencyShelfGain_dB;
		lowShelvingFilter.setParameters(filterParams);

		// --- output HPF
		dcBlockingFilter.reset(_sampleRate);
		filterParams = dcBlockingFilter.getParameters();
		filterParams.algorithm = filterAlgorithm::kHPF1;
		filterParams.fc = parameters.dcBlockingLF_Hz;
		filterParams.boostCut_dB = 0.0;
		dcBlockingFilter.setParameters(filterParams);

		// --- LPF (upper edge)
		upperBandwidthFilter.reset(_sampleRate);
		filterParams = upperBandwidthFilter.getParameters();
		filterParams.algorithm = filterAlgorithm::kLPF2;
		filterParams.fc = parameters.millerHF_Hz;
		filterParams.boostCut_dB = 0.0;
		upperBandwidthFilter.setParameters(filterParams);
		
		return true;
	}
	virtual bool canProcessAudioFrame() { return false; }

	// --- do the valve emulation
	virtual double processAudioSample(double xn)
	{
		double yn = 0.0;

		// --- input scaling
		xn *= parameters.inputGain;

		// --- (1) grid conduction check, must be done prior to waveshaping
		xn = doValveGridConduction(xn, parameters.gridConductionThreshold);
		
		// --- (2) detect the DC offset that the clipping may have caused
		double dcOffset = lossyIntegrator[0].processAudioSample(xn);

		// --- process only negative DC bias shifts
		dcOffset = fmin(dcOffset, 0.0);
		
		// --- save this - user may indicate it in a meter if they want
		//     Note that this is a bipolar value, but we only do DC shift for 
		//     *negative* values so meters should be aware
		parameters.dcOffsetDetected = fabs(dcOffset*parameters.dcShiftCoefficient);

		// --- (3) do the main emulation
		yn = doValveEmulation(xn, 
								parameters.waveshaperSaturation, 
								parameters.gridConductionThreshold,
								parameters.dcOffsetDetected,
								parameters.clipPointPositive, 
								parameters.clipPointNegative);
		
		// --- (4) do final filtering
		//
		// --- remove DC
		yn = dcBlockingFilter.processAudioSample(yn);
		
		// --- LF Shelf
		yn = lowShelvingFilter.processAudioSample(yn);

		// --- HF Edge
		yn = upperBandwidthFilter.processAudioSample(yn);

		// --- (5) final output scaling and inversion
		yn *= -parameters.outputGain;

		return yn;
	}


	/** get parameters: note use of custom structure for passing param data */
	/**
	\return ValveEmulatorParameters custom data structure
	*/
	ClassAValveParameters getParameters()
	{
		return parameters;
	}

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param ValveEmulatorParameters custom data structure
	*/
	void setParameters(const ClassAValveParameters& params)
	{
		// --- low shelf
		AudioFilterParameters filterParams = lowShelvingFilter.getParameters();
		if (filterParams.fc != params.lowFrequencyShelf_Hz || 
			filterParams.boostCut_dB != params.lowFrequencyShelfGain_dB)
		{
			filterParams.fc = params.lowFrequencyShelf_Hz;
			filterParams.boostCut_dB = params.lowFrequencyShelfGain_dB;
			lowShelvingFilter.setParameters(filterParams);
		}

		// --- output HPF
		filterParams = dcBlockingFilter.getParameters();
		if (filterParams.fc != params.dcBlockingLF_Hz)
		{
			filterParams.fc = params.dcBlockingLF_Hz;
			dcBlockingFilter.setParameters(filterParams);
		}

		// --- LPF (upper edge)
		filterParams = upperBandwidthFilter.getParameters();
		if (filterParams.fc != params.millerHF_Hz)
		{
			filterParams.fc = params.millerHF_Hz;
			upperBandwidthFilter.setParameters(filterParams);
		}

		ZVAFilterParameters paramsLI = lossyIntegrator[0].getParameters();
		paramsLI.fc = params.integratorFc;
		lossyIntegrator[0].setParameters(paramsLI);
		lossyIntegrator[1].setParameters(paramsLI);

		// --- save
		parameters = params;
	}


private:
	ClassAValveParameters parameters; ///< object parameters

	// --- local variables used by this object
	double sampleRate = 0.0;	///< sample rate

	// --- emulate grid conduction, found using SPICE simulations with 12AX7
	inline double doValveGridConduction(double xn, double gridConductionThreshold)
	{
		if (xn > 0.0)
		{
			// --- check how far above clip level we are
			double clipDelta = xn - gridConductionThreshold;
			clipDelta = fmax(clipDelta, 0.0);
			double compressionFactor = 0.4473253 + 0.5451584*exp(-0.3241584*clipDelta);
			return compressionFactor*xn;
		}
		else
			return xn;
	}

	// --- main triode emulation - plenty of room here for experimentation
	inline double doValveEmulation(double xn, double k, double gridConductionThreshold,
									double variableDCOffset, double clipPointPos,
									double clipPointNeg)
	{
		// --- add the offset detected
		xn += variableDCOffset;
		double yn = 0.0;

		// --- NOTE: the whole transfer function is shifted so that the normal
		//           DC operating point is 0.0 to prevent massive
		//           clicks at the beginning of a selection due to the temporary DC offset
		//           that occurs before the HPF can react
		//
		// --- top portion is arraya
		if (xn > gridConductionThreshold) // +1.5 is where grid conduction starts
		{
			if (xn > clipPointPos)
				yn = clipPointPos;
			else
			{
				// --- scaling to get into the first quadrant for Arraya @ (0,0)
				xn -= gridConductionThreshold;

				// --- note that the signal should be clipped/compressed prior to calling this
				if (clipPointPos > 1.0)
					xn /= (clipPointPos - gridConductionThreshold);

				yn = xn*(3.0 / 2.0)*(1.0 - (xn*xn) / 3.0);

				// --- scale by clip point positive
				yn *= (clipPointPos - gridConductionThreshold);

				// --- undo scaling
				yn += gridConductionThreshold;
			}
		}
		else if (xn > 0.0) // --- ultra linear region
		{
			// --- fundamentally linear region of 3/2 power law
			yn = xn;
		}
		else // botom portion is tanh( ) waveshaper - EXPERIMENT!!
		{
			if (xn < clipPointNeg)
				yn = clipPointNeg;
			else
			{
				// --- clip normalize
				if (clipPointNeg < -1.0)
					xn /= fabs(clipPointNeg);

				// --- the waveshaper
				yn = tanh(k*xn) / tanh(k);

				// --- undo clip normalize
				yn *= fabs(clipPointNeg);
			}
		}
		return yn;
	}

	ZVAFilter lossyIntegrator[2];
	AudioFilter lowShelvingFilter;
	AudioFilter dcBlockingFilter;
	AudioFilter upperBandwidthFilter;
};

// --- chooser for algorithm
enum class classBType{ pirkle, poletti };

/**
\struct ClassBValveParameters
\ingroup FX-Objects
\brief
Custom data structure for ClassBValve object

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
struct ClassBValveParameters
{
	ClassBValveParameters() {}

	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	ClassBValveParameters& operator=(const ClassBValveParameters& params)	// need this override for collections to work
	{
		// --- it is possible to try to make the object equal to itself
		//     e.g. thisObject = thisObject; so this code catches that
		//     trivial case and just returns this object
		if (this == &params)
			return *this;

		// --- copy from params (argument) INTO our variables
		algorithm = params.algorithm;
		outputHPF_fc = params.outputHPF_fc;
		outputLPF_fc = params.outputLPF_fc;
		fixedBiasVoltage = params.fixedBiasVoltage;
		dcShiftCoefficient = params.dcShiftCoefficient;
		clipPointPositive = params.clipPointPositive;
		waveshaperSaturation = params.waveshaperSaturation;
		dcOffsetDetectedPos = params.dcOffsetDetectedPos;
		dcOffsetDetectedNeg = params.dcOffsetDetectedNeg;
		asymWaveshaper_g = params.asymWaveshaper_g;
		asymWaveshaper_Lp = params.asymWaveshaper_Lp;
		asymWaveshaper_Ln = params.asymWaveshaper_Ln;
		symWaveshaper_g = params.symWaveshaper_g;
		symWaveshaper_LpLn = params.symWaveshaper_LpLn;
		inputGain = params.inputGain;
		outputGain = params.outputGain;

		// --- MUST be last
		return *this;
	}

	// --- filter stuff
	classBType algorithm = classBType::pirkle;

	// --- for emulation of output transformer, blocks DC and bandpasses the signal
	double outputHPF_fc = 10.0;
	double outputLPF_fc = 20480.0;

	// --- I/O scaling
	double inputGain = 50.0;			// --- this sets the effective sensitivity
	double outputGain = 0.53;			// --- reduce output back to [-1, +1]

	// --- Pirkle Coefficients
	double fixedBiasVoltage = -1.5;		// --- for my waveshaper, this is 1/2 of the actual cutoff bias (so this represents -3.0V)
	double clipPointPositive = 1.5;		// --- again, this is different for this waveshaper; this represents +3V
	double dcShiftCoefficient = 0.5;	// --- this is 1/2 the normal value because each tube will contribute 1/2 shift 
	double waveshaperSaturation = 1.2;	// --- the (k) value

	// --- Poletti Coefficients
	// --- my own versions, not same as patent - alot to experiment with here!!
	//
	// --- asymmetrical waveshaper
	double asymWaveshaper_g = 1.70;		// --- gain
	double asymWaveshaper_Lp = 23.6;	// --- positive limit
	double asymWaveshaper_Ln = 0.5;		// --- negative limit

	// --- symmetrical waveshaper
	double symWaveshaper_g = 4.0;		// --- gain
	double symWaveshaper_LpLn = 1.01;	// --- positive and negative limit

	// --- return data (optional, Pirkle algorithm only)
	double dcOffsetDetectedPos = 0.0;
	double dcOffsetDetectedNeg = 0.0;
};


/**
\class ClassBValvePair
\ingroup FX-Objects
\brief

Audio I/O:
- Processes mono input to mono output.
- *** Optionally, process frame *** Modify this according to your object functionality

Control I/F:
- Use ClassBValveParameters structure to get/set object params.

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
class ClassBValvePair : public IAudioSignalProcessor
{
public:
	ClassBValvePair(void) {}	/* C-TOR */
	~ClassBValvePair(void) {}	/* D-TOR */

public:
	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
	{
		// --- do any other per-audio-run inits here
		sampleRate = _sampleRate;

		// --- integrators
		lossyIntegrator[0].reset(_sampleRate);
		lossyIntegrator[1].reset(_sampleRate);

		ZVAFilterParameters params = lossyIntegrator[0].getParameters();
		params.filterAlgorithm = vaFilterAlgorithm::kLPF1;
		params.fc = 5.0; // 1 - 10 Hz
		lossyIntegrator[0].setParameters(params);
		lossyIntegrator[1].setParameters(params);

		// --- other filters
		//
		// --- output HPF
		dcBlockingFilter[0].reset(_sampleRate);
		AudioFilterParameters filterParams = dcBlockingFilter[0].getParameters();
		filterParams.algorithm = filterAlgorithm::kHPF1;
		filterParams.fc = parameters.outputHPF_fc;
		dcBlockingFilter[0].setParameters(filterParams);
		dcBlockingFilter[1].setParameters(filterParams);

		// --- LPF (upper edge)
		upperBandwidthFilter.reset(_sampleRate);
		filterParams = upperBandwidthFilter.getParameters();
		filterParams.algorithm = filterAlgorithm::kLPF2;
		filterParams.fc = parameters.outputLPF_fc;
		upperBandwidthFilter.setParameters(filterParams);

		return true;
	}

	virtual bool canProcessAudioFrame() { return false; }

	// --- do the valve emulation
	virtual double processAudioSample(double xn)
	{
		double yn = 0.0;

		// --- add input gain
		xn *= parameters.inputGain;

		// --- two choices here: Poletti or Pirkle
		if (parameters.algorithm == classBType::poletti)
		{
			// --- (1) asymmetrical waveshaping
			double yn_pos = doPolettiWaveShaper(xn, parameters.asymWaveshaper_g, parameters.asymWaveshaper_Lp, parameters.asymWaveshaper_Ln);
			double yn_neg = doPolettiWaveShaper(xn, parameters.asymWaveshaper_g, parameters.asymWaveshaper_Ln, parameters.asymWaveshaper_Lp);

			// --- (2) block DC
			yn_pos = dcBlockingFilter[0].processAudioSample(yn_pos);
			yn_neg = dcBlockingFilter[1].processAudioSample(yn_neg);

			// --- (3) symmetrical waveshaping
			yn_pos = doPolettiWaveShaper(yn_pos, parameters.symWaveshaper_g, parameters.symWaveshaper_LpLn, parameters.symWaveshaper_LpLn);
			yn_neg = doPolettiWaveShaper(yn_neg, parameters.symWaveshaper_g, parameters.symWaveshaper_LpLn, parameters.symWaveshaper_LpLn);

			// --- (4) combine output branches
			yn = yn_pos + yn_neg;
		}
		else // pirkle
		{
			// --- (1) create two branches, invert signal in one
			double xn_Pos = xn;
			double xn_Neg = -xn; // (-) branch input

			// --- (2) check grid conduction
			xn_Pos = doValveGridConduction(xn_Pos);
			xn_Neg = doValveGridConduction(xn_Neg);

			// --- (3) detect DC offset or pos and neg branches
			double dcOffsetPos = lossyIntegrator[0].processAudioSample(xn_Pos);
			double dcOffsetNeg = lossyIntegrator[1].processAudioSample(xn_Neg);
			parameters.dcOffsetDetectedPos = dcOffsetPos;
			parameters.dcOffsetDetectedNeg = dcOffsetNeg;

			// --- only use (-) DC offset
			dcOffsetPos = fmin(dcOffsetPos, 0.0);
			dcOffsetNeg = fmin(dcOffsetPos, 0.0);

			// --- (4) do the shaper
			double yn_Pos = doPirkleWaveShaper(xn_Pos, parameters.waveshaperSaturation, parameters.fixedBiasVoltage, dcOffsetPos*parameters.dcShiftCoefficient);
			double yn_Neg = doPirkleWaveShaper(xn_Neg, parameters.waveshaperSaturation, parameters.fixedBiasVoltage, dcOffsetNeg*parameters.dcShiftCoefficient);

			// --- (5) combine branches, note inversion prior to recombination to re-flip signal
			yn = yn_Pos - yn_Neg;
		}

		// --- here you can adjust the bandpass nature of the output transformer if you like
		//
		// --- LF Edge
		yn = dcBlockingFilter[0].processAudioSample(yn);

		// --- HF Edge
		yn = upperBandwidthFilter.processAudioSample(yn);

		// --- final output scaling
		yn *= parameters.outputGain;

		return yn;
	}


	/** get parameters: note use of custom structure for passing param data */
	/**
	\return ValveEmulatorParameters custom data structure
	*/
	ClassBValveParameters getParameters()
	{
		return parameters;
	}

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param ValveEmulatorParameters custom data structure
	*/
	void setParameters(const ClassBValveParameters& params)
	{
		// --- output HPF
		AudioFilterParameters filterParams = dcBlockingFilter[0].getParameters();
		if (filterParams.fc != params.outputHPF_fc)
		{
			filterParams.fc = params.outputHPF_fc;
			dcBlockingFilter[0].setParameters(filterParams);
			dcBlockingFilter[1].setParameters(filterParams);
		}

		// --- LPF (upper edge)
		filterParams = upperBandwidthFilter.getParameters();
		if (filterParams.fc != params.outputLPF_fc)
		{
			filterParams.fc = params.outputLPF_fc;
			upperBandwidthFilter.setParameters(filterParams);
		}

		// --- save
		parameters = params;
	}


private:
	ClassBValveParameters parameters; ///< object parameters

	// --- local variables used by this object
	double sampleRate = 0.0;	///< sample rate

	// --- emulate grid conduction, found using SPICE simulations with 12AX7
	inline double doValveGridConduction(double xn)
	{
		if (xn > 0.0)
		{
			// --- check how far above clip level we are
			double clipDelta = xn - parameters.clipPointPositive;
			clipDelta = fmax(clipDelta, 0.0);
			double compressionFactor = 0.4473253 + 0.5451584*exp(-0.3241584*clipDelta);
			return compressionFactor*xn;
		}
		else
			return xn;
	}

	// --- Poletti waveshaper (see patent for documentation)
	double doPolettiWaveShaper(double xn, double g, double Ln, double Lp)
	{
		double yn = 0.0;
		if (xn <= 0)
			yn = (g * xn) / (1.0 - ((g * xn) / Ln));
		else
			yn = (g * xn) / (1.0 + ((g * xn) / Lp));
		return yn;
	}

	// --- Pirkle waveshaper (see book addendum)
	double doPirkleWaveShaper(double xn, double g, double fixedDCoffset, double variableDCOffset)
	{
		xn += fixedDCoffset;
		xn += variableDCOffset;
		double yn = 1.5*atan(g*xn) / atan(g);
		return yn;
	}

	ZVAFilter lossyIntegrator[2];
	AudioFilter dcBlockingFilter[2];
	AudioFilter upperBandwidthFilter;
};

// -----------------------------------------------------------------------------
//
// --- DISTORTION FILTERS
//
// -----------------------------------------------------------------------------

// --- chooser for algorithm
enum class complexLPFPreset { resonant, normal, bright };
enum class complexLPFOrder { twoPole, fourPole };

/**
\struct ComplexLPFParameters
\ingroup FX-Objects
\brief
Custom data structure for ComplexLPF object

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
struct ComplexLPFParameters
{
	ComplexLPFParameters() {}

	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	ComplexLPFParameters& operator=(const ComplexLPFParameters& params)	// need this override for collections to work
	{
		// --- it is possible to try to make the object equal to itself
		//     e.g. thisObject = thisObject; so this code catches that
		//     trivial case and just returns this object
		if (this == &params)
			return *this;

		// --- copy from params (argument) INTO our variables
		algorithm = params.algorithm;
		filterGain_dB = params.filterGain_dB;
		filterOrder = params.filterOrder;

		// --- MUST be last
		return *this;
	}

	// --- filter stuff
	complexLPFPreset algorithm = complexLPFPreset::normal;
	double filterGain_dB = 0.0;
	complexLPFOrder filterOrder = complexLPFOrder::fourPole;
};


/**
\class ComplexLPF
\ingroup FX-Objects
\brief

Audio I/O:
- Processes mono input to mono output.
- *** Optionally, process frame *** Modify this according to your object functionality

Control I/F:
- Use ComplexLPFParameters structure to get/set object params.

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
class ComplexLPF : public IAudioSignalProcessor
{
public:
	ComplexLPF(void) {}	/* C-TOR */
	~ComplexLPF(void) {}	/* D-TOR */

public:
	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
	{
		// --- do any other per-audio-run inits here
		sampleRate = _sampleRate;

		// --- LPF (upper edge)
		lowPassFilters[0].reset(_sampleRate);
		lowPassFilters[1].reset(_sampleRate);
		AudioFilterParameters filterParams = lowPassFilters[0].getParameters();
		filterParams.algorithm = filterAlgorithm::kLPF2;
		lowPassFilters[0].setParameters(filterParams);
		lowPassFilters[1].setParameters(filterParams);

		return true;
	}

	virtual bool canProcessAudioFrame() { return false; }

	// --- do the valve emulation
	virtual double processAudioSample(double xn)
	{
		double yn = lowPassFilters[0].processAudioSample(xn);
		if(parameters.filterOrder == complexLPFOrder::fourPole)
			yn = lowPassFilters[1].processAudioSample(yn);

		yn *= filterGain;

		return yn;
	}


	/** get parameters: note use of custom structure for passing param data */
	/**
	\return ValveEmulatorParameters custom data structure
	*/
	ComplexLPFParameters getParameters()
	{
		return parameters;
	}

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param ValveEmulatorParameters custom data structure
	*/
	void setParameters(const ComplexLPFParameters& params)
	{
		// --- save & update
		if(params.filterGain_dB != parameters.filterGain_dB)
			filterGain = pow(10.0, params.filterGain_dB / 20.0);

		parameters = params;
		updateFilters();
	}

	// --- update based on presets
	inline void updateFilters()
	{
		AudioFilterParameters filterParams_0 = lowPassFilters[0].getParameters();
		AudioFilterParameters filterParams_1 = lowPassFilters[0].getParameters();
		if (parameters.algorithm == complexLPFPreset::resonant)
		{
			filterParams_0.fc = 1230.0;
			filterParams_0.Q = 1.5;
			filterParams_1.fc = 2700.0;
			filterParams_1.Q = 5.0;
		}
		else if (parameters.algorithm == complexLPFPreset::normal)
		{
			filterParams_0.fc = 3000.0;
			filterParams_0.Q = 0.9;
			filterParams_1.fc = 3500.0;
			filterParams_1.Q = 0.9;
		}
		else if (parameters.algorithm == complexLPFPreset::bright)
		{
			filterParams_0.fc = 2000.0;
			filterParams_0.Q = 1.2;
			filterParams_1.fc = 4000.0;
			filterParams_1.Q = 1.9;
		}

		lowPassFilters[0].setParameters(filterParams_0);
		lowPassFilters[1].setParameters(filterParams_1);
	}


private:
	ComplexLPFParameters parameters; ///< object parameters
	double sampleRate = 0.0;
	double filterGain = 1.0;

	AudioFilter lowPassFilters[2];
};

/**
\struct BigMuffParameters
\ingroup FX-Objects
\brief
Custom data structure for BigMuff object

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
struct BigMuffToneControlParameters
{
	BigMuffToneControlParameters() {}

	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	BigMuffToneControlParameters& operator=(const BigMuffToneControlParameters& params)	// need this override for collections to work
	{
		// --- it is possible to try to make the object equal to itself
		//     e.g. thisObject = thisObject; so this code catches that
		//     trivial case and just returns this object
		if (this == &params)
			return *this;

		// --- copy from params (argument) INTO our variables
		toneControl_010 = params.toneControl_010;
		filterGain_dB = params.filterGain_dB;

		// --- MUST be last
		return *this;
	}

	// --- filter stuff
	double toneControl_010 = 5.0; // tone, 1->10
	double filterGain_dB = 0.0;
};


/**
\class BigMuff
\ingroup FX-Objects
\brief

Audio I/O:
- Processes mono input to mono output.
- *** Optionally, process frame *** Modify this according to your object functionality

Control I/F:
- Use BigMuffParameters structure to get/set object params.

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
class BigMuffToneControl : public IAudioSignalProcessor
{
public:
	BigMuffToneControl(void) {}	/* C-TOR */
	~BigMuffToneControl(void) {}	/* D-TOR */

public:
	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
	{
		// --- do any other per-audio-run inits here
		sampleRate = _sampleRate;

		// --- LPF (upper edge)
		lpf.reset(_sampleRate);
		AudioFilterParameters filterParams = lpf.getParameters();
		filterParams.algorithm = filterAlgorithm::kLPF1;
		filterParams.fc = 150.0;
		lpf.setParameters(filterParams);

		// --- High Shelf (lower edge, mimic passive losses)
		hsf.reset(_sampleRate);
		filterParams = hsf.getParameters();
		filterParams.algorithm = filterAlgorithm::kHiShelf;
		filterParams.fc = 3000.0;
		filterParams.boostCut_dB = +18.0;
		hsf.setParameters(filterParams);

		return true;
	}

	virtual bool canProcessAudioFrame() { return false; }

	// --- filter it
	virtual double processAudioSample(double xn)
	{
		double hsfGain = 0.1258; // -18dB
		double ynLP = lpf.processAudioSample(xn);
		double ynHP = hsfGain*hsf.processAudioSample(xn);
		double yn = (1.0 - filterBlend) * ynLP + filterBlend * ynHP;
		return filterGain * yn;
	}


	/** get parameters: note use of custom structure for passing param data */
	/**
	\return ValveEmulatorParameters custom data structure
	*/
	BigMuffToneControlParameters getParameters()
	{
		return parameters;
	}

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param ValveEmulatorParameters custom data structure
	*/
	void setParameters(const BigMuffToneControlParameters& params)
	{
		if (params.filterGain_dB != parameters.filterGain_dB)
			filterGain = pow(10.0, params.filterGain_dB / 20.0);

		parameters = params;

		// --- this maps qControl = 0 -> 10   to   K = 0 -> 1
		filterBlend = (parameters.toneControl_010) / (10.0);
	}

private:
	BigMuffToneControlParameters parameters; ///< object parameters
	double sampleRate = 0.0;
	double filterGain = 1.0;
	double filterBlend = 0.5;

	AudioFilter lpf;
	AudioFilter hsf;
};


enum class variBPFEdgeSlope { sixdB_oct, twelvedB_oct };

/**
\struct BigMuffParameters
\ingroup FX-Objects
\brief
Custom data structure for BigMuff object

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
struct VariBPFParameters
{
	VariBPFParameters() {}

	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	VariBPFParameters& operator=(const VariBPFParameters& params)	// need this override for collections to work
	{
		// --- it is possible to try to make the object equal to itself
		//     e.g. thisObject = thisObject; so this code catches that
		//     trivial case and just returns this object
		if (this == &params)
			return *this;

		lfEdgeSlope = params.lfEdgeSlope;
		hfEdgeSlope = params.hfEdgeSlope;

		variBPFControl_010 = params.variBPFControl_010;

		// --- copy from params (argument) INTO our variables
		start_FL_Hz = params.start_FL_Hz;
		end_FL_Hz = params.end_FL_Hz;
		start_FH_Hz = params.start_FH_Hz;
		end_FH_Hz = params.end_FH_Hz;

		startFilterGain_dB = params.startFilterGain_dB;
		endFilterGain_dB = params.endFilterGain_dB;

		// --- MUST be last
		return *this;
	}

	// --- allows customization of BPF slopes
	variBPFEdgeSlope lfEdgeSlope = variBPFEdgeSlope::twelvedB_oct;
	variBPFEdgeSlope hfEdgeSlope = variBPFEdgeSlope::twelvedB_oct;

	// --- single control 
	double variBPFControl_010 = 5.0;

	// --- filter spec: start and stop low/high band edges
	double start_FL_Hz = 50.0;
	double end_FL_Hz = 500.0;
	double start_FH_Hz = 5000.0;
	double end_FH_Hz = 2000.0;

	// --- start and end gains
	double startFilterGain_dB = 0.0;
	double endFilterGain_dB = 0.0;
};


/**
\class VariBPF
\ingroup FX-Objects
\brief

Audio I/O:
- Processes mono input to mono output.
- *** Optionally, process frame *** Modify this according to your object functionality

Control I/F:
- Use VariBPFParameters structure to get/set object params.

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
class VariBPF : public IAudioSignalProcessor
{
public:
	VariBPF(void) {}	/* C-TOR */
	~VariBPF(void) {}	/* D-TOR */

public:
	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
	{
		// --- do any other per-audio-run inits here
		sampleRate = _sampleRate;

		parameters.hfEdgeSlope = variBPFEdgeSlope::sixdB_oct;
		parameters.lfEdgeSlope = variBPFEdgeSlope::twelvedB_oct;

		// --- LPF (upper edge)
		lpf.reset(_sampleRate);
		AudioFilterParameters filterParams = lpf.getParameters();

		if(parameters.hfEdgeSlope == variBPFEdgeSlope::sixdB_oct)
			filterParams.algorithm = filterAlgorithm::kLPF1;
		else
			filterParams.algorithm = filterAlgorithm::kLPF2;
		
		filterParams.fc = parameters.start_FH_Hz;
		lpf.setParameters(filterParams);

		// --- HPF (lower edge)
		hpf.reset(_sampleRate);
		filterParams = hpf.getParameters();

		if (parameters.lfEdgeSlope == variBPFEdgeSlope::sixdB_oct)
			filterParams.algorithm = filterAlgorithm::kHPF1;
		else
			filterParams.algorithm = filterAlgorithm::kHPF2;

		filterParams.fc = parameters.start_FL_Hz;
		hpf.setParameters(filterParams);

		return true;
	}

	virtual bool canProcessAudioFrame() { return false; }

	// --- filter it
	virtual double processAudioSample(double xn)
	{
		double hpOut = hpf.processAudioSample(xn);
		double yn = lpf.processAudioSample(hpOut);
		return currentFilterGain * yn;
	}

	/** get parameters: note use of custom structure for passing param data */
	/**
	\return ValveEmulatorParameters custom data structure
	*/
	VariBPFParameters getParameters()
	{
		return parameters;
	}

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param ValveEmulatorParameters custom data structure
	*/
	void setParameters(const VariBPFParameters& params)
	{
		parameters = params;

		// --- this maps variBPF control = 0 -> 10   to   G = 0 -> 1
		double variBlend = (parameters.variBPFControl_010) / (10.0);

		double modFilterGain_dB = doUnipolarModulationFromMin(variBlend, 
											parameters.startFilterGain_dB,
											parameters.endFilterGain_dB);

		currentFilterGain = pow(10.0, modFilterGain_dB / 20.0);

		// --- current modulated values
		current_FL = doUnipolarModulationFromMin(variBlend, parameters.start_FL_Hz, parameters.end_FL_Hz);
		current_FH = doUnipolarModulationFromMin(variBlend, parameters.start_FH_Hz, parameters.end_FH_Hz);
		
		// --- LPF (upper edge)
		AudioFilterParameters filterParams = lpf.getParameters();
		filterParams.fc = current_FH;

		if (parameters.hfEdgeSlope == variBPFEdgeSlope::sixdB_oct)
			filterParams.algorithm = filterAlgorithm::kLPF1;
		else
			filterParams.algorithm = filterAlgorithm::kLPF2;

		lpf.setParameters(filterParams);

		// --- HPF (lower edge)
		filterParams = hpf.getParameters();
		filterParams.fc = current_FL;

		if (parameters.lfEdgeSlope == variBPFEdgeSlope::sixdB_oct)
			filterParams.algorithm = filterAlgorithm::kHPF1;
		else
			filterParams.algorithm = filterAlgorithm::kHPF2;

		hpf.setParameters(filterParams);
	}

private:
	VariBPFParameters parameters; ///< object parameters
	double sampleRate = 0.0;
	double currentFilterGain = 1.0;
	double current_FL = 0.0;
	double current_FH = 0.0;
	AudioFilter lpf;
	AudioFilter hpf;
};

enum class contourType { none, normal, mid_boost };

/**
\struct ToneStackParameters
\ingroup FX-Objects
\brief
Custom data structure for ToneStack object

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
struct ToneStackParameters
{
	ToneStackParameters() {}

	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	ToneStackParameters& operator=(const ToneStackParameters& params)	// need this override for collections to work
	{
		// --- it is possible to try to make the object equal to itself
		//     e.g. thisObject = thisObject; so this code catches that
		//     trivial case and just returns this object
		if (this == &params)
			return *this;

		// --- copy from params (argument) INTO our variables
		contour = params.contour;
		LFToneControl_010 = params.LFToneControl_010;
		HFToneControl_010 = params.HFToneControl_010;
		MFToneControl_010 = params.MFToneControl_010;

		// --- MUST be last
		return *this;
	}

	// --- filter stuff
	contourType contour = contourType::normal;
	double LFToneControl_010 = 5.0; // tone, 0->10
	double HFToneControl_010 = 5.0; // tone, 0->10
	double MFToneControl_010 = 5.0; // tone, 0->10
};


/**
\class ToneStack
\ingroup FX-Objects
\brief

Audio I/O:
- Processes mono input to mono output.
- *** Optionally, process frame *** Modify this according to your object functionality

Control I/F:
- Use BigMuffParameters structure to get/set object params.

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
class ToneStack : public IAudioSignalProcessor
{
public:
	ToneStack(void) {}	/* C-TOR */
	~ToneStack(void) {}	/* D-TOR */

public:
	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
	{
		// --- do any other per-audio-run inits here
		sampleRate = _sampleRate;

		// --- LSF
		lowShelfFilter.reset(_sampleRate);
		AudioFilterParameters filterParams = lowShelfFilter.getParameters();
		filterParams.algorithm = filterAlgorithm::kLowShelf;
		filterParams.fc = 60.0;
		filterParams.boostCut_dB = 0.0;
		lowShelfFilter.setParameters(filterParams);

		// --- HSF
		highShelfFilter.reset(_sampleRate);
		filterParams = highShelfFilter.getParameters();
		filterParams.algorithm = filterAlgorithm::kHiShelf;
		filterParams.fc = 2400.0;
		filterParams.boostCut_dB = 0.0;
		highShelfFilter.setParameters(filterParams);

		// --- Para
		midParametricFilter.reset(_sampleRate);
		filterParams = midParametricFilter.getParameters();
		filterParams.algorithm = filterAlgorithm::kCQParaEQ;
		filterParams.fc = 500.0;
		filterParams.Q = 1.0;
		filterParams.boostCut_dB = 0.0;
		midParametricFilter.setParameters(filterParams);

		// --- contour filters
		contourBPF.reset(_sampleRate);
		filterParams = contourBPF.getParameters();
		filterParams.algorithm = filterAlgorithm::kBPF2;
		filterParams.fc = 50.00;
		filterParams.Q = 0.222;
		contourBPF.setParameters(filterParams);

		contourHPF.reset(_sampleRate);
		filterParams = contourHPF.getParameters();
		filterParams.algorithm = filterAlgorithm::kHPF1;
		filterParams.fc = 750.0;
		contourHPF.setParameters(filterParams);

		return true;
	}

	virtual bool canProcessAudioFrame() { return false; }

	// --- filter it
	virtual double processAudioSample(double xn)
	{
		double cn = xn;
		double ynCFB = contourBPF.processAudioSample(cn);
		double ynCFH = contourHPF.processAudioSample(cn);

		if (parameters.contour != contourType::none)
		{
			// --- preset gains
			//
			// --- BPF: fc = 50Hz Q = 0.222 Gain = +3.5dB
			// --- HPF: fc = 750Hz			Gain = +2.0dB
			//
			//	   double contourBPFGain = pow(10.0, 3.5 / 20.0);
			//	   double contourHPFGain = pow(10.0, 2.0 / 20.0);;
			cn = contourBPFGain * ynCFB + contourHPFGain * ynCFH;
		}

		double ynLP = lowShelfFilter.processAudioSample(cn);
		double ynHP = highShelfFilter.processAudioSample(ynLP);
		double ynMB = midParametricFilter.processAudioSample(ynHP);
		return ynMB;
	}


	/** get parameters: note use of custom structure for passing param data */
	/**
	\return ValveEmulatorParameters custom data structure
	*/
	ToneStackParameters getParameters()
	{
		return parameters;
	}

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param ValveEmulatorParameters custom data structure
	*/
	void setParameters(const ToneStackParameters& params)
	{
		parameters = params;

		// --- contour presets
		AudioFilterParameters filterParams = contourHPF.getParameters();
		if(parameters.contour == contourType::normal)
			filterParams.fc = 750.0;
		else if (parameters.contour == contourType::mid_boost)
			filterParams.fc = 250.0;
		
		// --- this will reject same settings...
		contourHPF.setParameters(filterParams);

		// --- this maps qControl = 1 -> 10   to   K = 0 -> 1
		double LF_gain = parameters.LFToneControl_010  / 10.0;
		double MF_gain = parameters.MFToneControl_010 / 10.0;
		double HF_gain = parameters.HFToneControl_010 / 10.0;

		// --- convert to dB
		double LF_gain_dB = doUnipolarModulationFromMin(LF_gain, -12.0, +12.0);
		double MF_gain_dB = doUnipolarModulationFromMin(MF_gain, -6.0, +6.0);
		double HF_gain_dB = doUnipolarModulationFromMin(HF_gain, -8.0, +8.0);

		// --- LSF
		filterParams = lowShelfFilter.getParameters();
		filterParams.boostCut_dB = LF_gain_dB;
		lowShelfFilter.setParameters(filterParams);

		// --- HSF
		filterParams = highShelfFilter.getParameters();
		filterParams.boostCut_dB = HF_gain_dB;
		highShelfFilter.setParameters(filterParams);

		// --- Para
		filterParams = midParametricFilter.getParameters();
		filterParams.boostCut_dB = MF_gain_dB;
		midParametricFilter.setParameters(filterParams);
	}

private:
	ToneStackParameters parameters; ///< object parameters
	double sampleRate = 0.0;

	AudioFilter lowShelfFilter;
	AudioFilter highShelfFilter;
	AudioFilter midParametricFilter;

	AudioFilter contourBPF;
	AudioFilter contourHPF;

	double contourBPFGain = pow(10.0, 3.5 / 20.0);
	double contourHPFGain = pow(10.0, 2.0 / 20.0);
};


/**
\struct TurboDistortoParameters
\ingroup FX-Objects
\brief
Custom data structure for ComplexLPF object

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
struct TurboDistortoParameters
{
	TurboDistortoParameters() {}

	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	TurboDistortoParameters& operator=(const TurboDistortoParameters& params)	// need this override for collections to work
	{
		// --- it is possible to try to make the object equal to itself
		//     e.g. thisObject = thisObject; so this code catches that
		//     trivial case and just returns this object
		if (this == &params)
			return *this;

		// --- copy from params (argument) INTO our variables
		distortionControl_010 = params.distortionControl_010;
		toneControl_010 = params.toneControl_010;
		outputGain_010 = params.outputGain_010;
		inputGainTweak_010 = params.inputGainTweak_010;

		engageTurbo = params.engageTurbo;
		bypassFilter = params.bypassFilter;

		// --- MUST be last
		return *this;
	}

	// --- distortion controls
	double distortionControl_010 = 0.0;
	double toneControl_010 = 0.0;
	double outputGain_010 = 5.0;

	// --- input volume tweaker
	double inputGainTweak_010 = 5.0; // -20 to +20 dB of gain/attenuation

	// --- switches
	bool engageTurbo = false;
	bool bypassFilter = false;
};


/**
\class TurboDistorto
\ingroup FX-Objects
\brief

Audio I/O:
- Processes mono input to mono output.
- *** Optionally, process frame *** Modify this according to your object functionality

Control I/F:
- Use TurboDistortoParameters structure to get/set object params.

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
class TurboDistorto : public IAudioSignalProcessor
{
public:
	TurboDistorto(void) {}	/* C-TOR */
	~TurboDistorto(void) {}	/* D-TOR */

public:
	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
	{
		// --- do any other per-audio-run inits here
		sampleRate = _sampleRate;

		// --- BMP Filter
		toneFilter.reset(_sampleRate);
		BigMuffToneControlParameters bmParams = toneFilter.getParameters();
		bmParams.filterGain_dB = +14.0;
		toneFilter.setParameters(bmParams);

		// --- setup VariBPF
		variBPF.reset(_sampleRate);
		VariBPFParameters vBPF = variBPF.getParameters();
		vBPF.startFilterGain_dB = -10.0;
		vBPF.endFilterGain_dB = -2.0;
		vBPF.hfEdgeSlope = variBPFEdgeSlope::twelvedB_oct;
		vBPF.lfEdgeSlope = variBPFEdgeSlope::sixdB_oct;
		vBPF.start_FL_Hz = 150.0;
		vBPF.start_FH_Hz = 5800.0;
		vBPF.end_FL_Hz = 500.0;
		vBPF.end_FH_Hz = 2500.0;
		variBPF.setParameters(vBPF);

		return true;
	}

	virtual bool canProcessAudioFrame() { return false; }

	// --- do the valve emulation
	virtual double processAudioSample(double xn)
	{
		// --- convert distortion knob into waveshaper saturation value from 1 -> 50
		double saturation = calcMappedVariableOnRange(0.0, 10.0,
													  1.0, 50.0,
													  parameters.distortionControl_010);
		double yn = inputGain * xn;

		// --- turbo adds pre-distortion
		if (parameters.engageTurbo)
			yn = tanh((saturation / 2.0)*yn) / tanh(saturation / 2.0);

		// --- normal shaper
		yn = tanh(saturation*yn) / tanh(saturation);

		// --- VariBPF
		yn = variBPF.processAudioSample(yn);

		// --- optional filter
		if (!parameters.bypassFilter)
			yn = toneFilter.processAudioSample(yn);

		return outputGain*yn;
	}


	/** get parameters: note use of custom structure for passing param data */
	/**
	\return ValveEmulatorParameters custom data structure
	*/
	TurboDistortoParameters getParameters()
	{
		return parameters;
	}

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param ValveEmulatorParameters custom data structure
	*/
	void setParameters(const TurboDistortoParameters& params)
	{
		parameters = params;

		// --- find the output gain amount
		outputGain = calcMappedVariableOnRange(0.0, 10.0, 
												-40.0, +20.0, 
												parameters.outputGain_010,
												true);

		// --- find the input tweak amount
		inputGain = calcMappedVariableOnRange(0.0, 10.0,
											 -20.0, +20.0,
											 parameters.inputGainTweak_010,
											 true);

		// --- update the VariBPF 
		VariBPFParameters vbpfParams = variBPF.getParameters();
		vbpfParams.variBPFControl_010 = parameters.distortionControl_010;
		variBPF.setParameters(vbpfParams);

		// --- update the Big Muff Tone 
		BigMuffToneControlParameters bmParams = toneFilter.getParameters();
		bmParams.toneControl_010 = parameters.toneControl_010;
		toneFilter.setParameters(bmParams);
	}

private:
	TurboDistortoParameters parameters; ///< object parameters
	double sampleRate = 0.0;
	double inputGain = 1.0;
	double outputGain = 1.0;

	VariBPF variBPF;
	BigMuffToneControl toneFilter;
};

const unsigned int PREAMP_TRIODES = 4;
enum class ampGainStructure { low, medium, high };

/**
\struct OneMarkAmpParameters
\ingroup FX-Objects
\brief
Custom data structure for OneMarkAmp object

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
struct OneMarkAmpParameters
{
	OneMarkAmpParameters() {}

	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	OneMarkAmpParameters& operator=(const OneMarkAmpParameters& params)	// need this override for collections to work
	{
		// --- it is possible to try to make the object equal to itself
		//     e.g. thisObject = thisObject; so this code catches that
		//     trivial case and just returns this object
		if (this == &params)
			return *this;

		// --- copy from params (argument) INTO our variables
		volume1_010 = params.volume1_010;
		volume2_010 = params.volume2_010;
		inputHPF_010 = params.inputHPF_010;
		toneStackParameters = params.toneStackParameters;
		masterVolume_010 = params.masterVolume_010;
		tubeCompression_010 = params.tubeCompression_010;

		bright = params.bright;
		ampGainStyle = params.ampGainStyle;

		for (int i = 0; i < PREAMP_TRIODES; i++)
			dcShift[i] = params.dcShift[i];

		// --- MUST be last
		return *this;
	}

	// --- amp controls are all 0 -> 10
	double volume1_010 = 0.0;
	double volume2_010 = 0.0;
	double inputHPF_010 = 0.0;
	double tubeCompression_010 = 0.0;
	double masterVolume_010 = 0.0;

	// --- tone stack can take care of itself
	ToneStackParameters toneStackParameters;
	
	// --- for monitoring preamp tube DC shifts
	double dcShift[PREAMP_TRIODES] = { 0.0, 0.0, 0.0, 0.0 };

	// --- switches
	bool bright = false;
	ampGainStructure ampGainStyle = ampGainStructure::medium;
};

/**
\class OneMarkAmp
\ingroup FX-Objects
\brief

Audio I/O:
- Processes mono input to mono output.
- *** Optionally, process frame *** Modify this according to your object functionality

Control I/F:
- Use TurboDistortoParameters structure to get/set object params.

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
class OneMarkAmp : public IAudioSignalProcessor
{
public:
	OneMarkAmp(void) {}	/* C-TOR */
	~OneMarkAmp(void) {}	/* D-TOR */

public:
	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
	{
		// --- do any other per-audio-run inits here
		sampleRate = _sampleRate;
		
		triodes[T1].reset(sampleRate);
		ClassAValveParameters triodeParams = triodes[T1].getParameters();
		triodeParams.lowFrequencyShelf_Hz = 10.0;
		triodeParams.lowFrequencyShelfGain_dB = -10.0;
		triodeParams.integratorFc = 1.0;
		triodeParams.millerHF_Hz = 20000.0;
		triodeParams.dcBlockingLF_Hz = 8.0;
		triodeParams.outputGain = pow(10.0, -3.0 / 20.0);
		triodeParams.dcShiftCoefficient = 1.0;
		triodes[T1].setParameters(triodeParams);

		triodes[T2].reset(sampleRate);
		triodeParams = triodes[T2].getParameters();
		triodeParams.lowFrequencyShelf_Hz = 10.0;
		triodeParams.lowFrequencyShelfGain_dB = -10.0;
		triodeParams.integratorFc = 1.0;
		triodeParams.millerHF_Hz = 9000.0;
		triodeParams.dcBlockingLF_Hz = 32.0;
		triodeParams.outputGain = pow(10.0, +5.0 / 20.0);
		triodeParams.dcShiftCoefficient = 0.20;
		triodes[T2].setParameters(triodeParams);

		triodes[T3].reset(sampleRate);
		triodeParams = triodes[T3].getParameters();
		triodeParams.lowFrequencyShelf_Hz = 10.0;
		triodeParams.lowFrequencyShelfGain_dB = -10.0;
		triodeParams.integratorFc = 1.0;
		triodeParams.millerHF_Hz = 7000.0;
		triodeParams.dcBlockingLF_Hz = 40.0;
		triodeParams.outputGain = pow(10.0, +6.0 / 20.0);
		triodeParams.dcShiftCoefficient = 0.50;
		triodes[T3].setParameters(triodeParams);

		triodes[T4].reset(sampleRate);
		triodeParams = triodes[T4].getParameters();
		triodeParams.lowFrequencyShelf_Hz = 10.0;
		triodeParams.lowFrequencyShelfGain_dB = -10.0;
		triodeParams.integratorFc = 1.0;
		triodeParams.millerHF_Hz = 6400.0;
		triodeParams.dcBlockingLF_Hz = 43.0;
		triodeParams.outputGain = pow(10.0, -20.0 / 20.0);
		triodeParams.dcShiftCoefficient = 0.52;
		triodes[T4].setParameters(triodeParams);
		
		outputPentodes.reset(sampleRate);
		ClassBValveParameters pentodeParams = outputPentodes.getParameters();
		pentodeParams.algorithm = classBType::pirkle;
		pentodeParams.inputGain = pow(10.0, 20.0 / 20.0);
		pentodeParams.outputGain = pow(10.0, -20.0 / 20.0);
		outputPentodes.setParameters(pentodeParams);

		toneStack.reset(sampleRate);

		inputHPF.reset(sampleRate);
		AudioFilterParameters hpfParams = inputHPF.getParameters();
		hpfParams.algorithm = filterAlgorithm::kHPF2;
		inputHPF.setParameters(hpfParams);

		outputHPF.reset(sampleRate);
		hpfParams = outputHPF.getParameters();
		hpfParams.algorithm = filterAlgorithm::kHPF2;
		hpfParams.fc = 5.0;
		outputHPF.setParameters(hpfParams);

		outputLPF.reset(sampleRate);
		AudioFilterParameters lpfParams = outputLPF.getParameters();
		lpfParams.algorithm = filterAlgorithm::kLPF2;
		outputLPF.setParameters(lpfParams);

		return true;
	}

	virtual bool canProcessAudioFrame() { return false; }

	// --- do the valve emulation
	virtual double processAudioSample(double xn)
	{
		// --- first triode
		//double t1OutSDS = triodes[0].processAudioSample(0.0);
		//return t1OutSDS;

		// --- remove DC, remove bass 
		double hpfOut = inputHPF.processAudioSample(xn);

		// --- "volume 1" control
		double t1In = hpfOut * inputGain;

		// --- first triode
		double t1Out = triodes[0].processAudioSample(t1In);
		
		// --- add pre-drive
		t1Out *= driveGain;

		// --- cascade of triodes
		//     NOTE: leaving this verbose so you can experiment, use less or more triodes...
		double t2Out = triodes[1].processAudioSample(t1Out);
		double t3Out = triodes[2].processAudioSample(t2Out);
		double t4Out = triodes[3].processAudioSample(t3Out);

		// --- class B drive gain
		t4Out *= tubeComress;
		
		// --- class B model
		double classBOut = outputPentodes.processAudioSample(t4Out);

		// --- tone stack (note: relocating makes a big difference)
		double toneStackOut = toneStack.processAudioSample(classBOut);

		// --- speaker sim
		double dcBlock = outputHPF.processAudioSample(toneStackOut);
		double outputLPFout = outputLPF.processAudioSample(dcBlock);

		// --- for metering the DC Shifts only - can ignore if you want
		for(int i=0; i< PREAMP_TRIODES; i++)
			parameters.dcShift[i] = triodes[i].getParameters().dcOffsetDetected;

		// --- the FIRST meter is binary
		if (parameters.dcShift[0] > 0.125)
			parameters.dcShift[0] = 1.0;

		return outputLPFout * outputGain;
	}

	/** get parameters: note use of custom structure for passing param data */
	/**
	\return ValveEmulatorParameters custom data structure
	*/
	OneMarkAmpParameters getParameters()
	{
		return parameters;
	}

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param ValveEmulatorParameters custom data structure
	*/
	void setParameters(const OneMarkAmpParameters& params)
	{
		parameters = params;

		// --- simulate different gain structures with waveshaper saturation
		double saturation = 1.0;
		if (parameters.ampGainStyle == ampGainStructure::medium)
			saturation = 2.0;
		else if (parameters.ampGainStyle == ampGainStructure::high)
			saturation = 5.0;

		// --- update
		for (int i = 0; i < PREAMP_TRIODES; i++)
		{
			ClassAValveParameters triodeParams = triodes[i].getParameters();
			triodeParams.waveshaperSaturation = saturation;
			triodes[i].setParameters(triodeParams);
		}

		// --- input gain
		if (parameters.volume1_010 == 0.0)
			inputGain = 0.0;
		else
		{
			inputGain = calcMappedVariableOnRange(0.0, 10.0,
				-60.0, +20.0,
				parameters.volume1_010,
				true);
		}

		// --- drive gain
		driveGain = calcMappedVariableOnRange(0.0, 10.0,
				-20.0, +20.0,
				parameters.volume2_010,
				true);

		// --- HPF
		AudioFilterParameters hpfParams = inputHPF.getParameters();
		hpfParams.fc = calcMappedVariableOnRange(0.0, 10.0,
			5.0, 1000.0,
			parameters.inputHPF_010);
		inputHPF.setParameters(hpfParams);

		// --- ToneStack
		toneStack.setParameters(parameters.toneStackParameters);

		// --- output gain amount
		outputGain = calcMappedVariableOnRange(0.0, 10.0,
			-60.0, +12.0,
			parameters.masterVolume_010,
			true);

		tubeComress = calcMappedVariableOnRange(0.0, 10.0,
			-6.0, +24.0,
			parameters.tubeCompression_010,
			true);

		// --- speaker simulator
		AudioFilterParameters lpfParams = outputLPF.getParameters();
		lpfParams.fc = parameters.bright ? 3200.0 : 2450.0;
		lpfParams.Q = parameters.bright ? 0.707 : 1.4;
		outputLPF.setParameters(lpfParams);
	}

private:
	OneMarkAmpParameters parameters; ///< object parameters
	double sampleRate = 0.0;

	double inputGain = 1.0;
	double driveGain = 1.0;
	double tubeComress = 1.0;
	double outputGain = 1.0;

	enum { T1, T2, T3, T4 };
	ClassAValve triodes[4];
	ClassBValvePair outputPentodes;
	ToneStack toneStack;
	AudioFilter outputLPF;
	AudioFilter inputHPF;
	AudioFilter outputHPF;
};



/**
\struct OneMarkAmpParameters
\ingroup FX-Objects
\brief
Custom data structure for OneMarkAmp object

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
struct WickerAmpParameters
{
	WickerAmpParameters() {}

	/** all FXObjects parameter objects require overloaded= operator so remember to add new entries if you add new variables. */
	WickerAmpParameters& operator=(const WickerAmpParameters& params)	// need this override for collections to work
	{
		// --- it is possible to try to make the object equal to itself
		//     e.g. thisObject = thisObject; so this code catches that
		//     trivial case and just returns this object
		if (this == &params)
			return *this;

		// --- copy from params (argument) INTO our variables
		ampParameters = params.ampParameters;
		masterReverb_010 = params.masterReverb_010;
		dcShiftTriode_0 = params.dcShiftTriode_0;

		// --- MUST be last
		return *this;
	}

	// --- only need one since both channels are identical
	OneMarkAmpParameters ampParameters;

	// --- our reverb
	double masterReverb_010 = 0.0;

	// --- DC shift warning meter
	double dcShiftTriode_0 = 0.0;
};

/**
\class WickerAmp
\ingroup FX-Objects
\brief

Audio I/O:
- Processes mono input to mono output.
- *** Optionally, process frame *** Modify this according to your object functionality

Control I/F:
- Use TurboDistortoParameters structure to get/set object params.

\author <Your Name> <http://www.yourwebsite.com>
\remark <Put any remarks or notes here>
\version Revision : 1.0
\date Date : 2019 / 01 / 31
*/
class WickerAmp : public IAudioSignalProcessor
{
public:
	WickerAmp(void) {}	/* C-TOR */
	~WickerAmp(void) {}	/* D-TOR */

public:
	/** reset members to initialized state */
	virtual bool reset(double _sampleRate)
	{
		// --- do any other per-audio-run inits here
		sampleRate = _sampleRate;

		tubeAmpChannel[0].reset(sampleRate);
		tubeAmpChannel[1].reset(sampleRate);
		
		reverb.reset(sampleRate);

		ReverbTankParameters verbParams = reverb.getParameters();
		verbParams.kRT = 0.901248;
		verbParams.lpf_g = 0.300000;
		verbParams.lowShelf_fc = 150.000000;
		verbParams.lowShelfBoostCut_dB = -20.000000;
		verbParams.highShelf_fc = 4000.000000;
		verbParams.highShelfBoostCut_dB = -6.000000;
		verbParams.wetLevel_dB = -12.000000;
		verbParams.dryLevel_dB = 0.000000;
		verbParams.apfDelayMax_mSec = 33.000000;
		verbParams.apfDelayWeight_Pct = 85.000000;
		verbParams.fixeDelayMax_mSec = 81.000000;
		verbParams.fixeDelayWeight_Pct = 100.000000;
		verbParams.preDelayTime_mSec = 25.000000;
		verbParams.density = reverbDensity::kThick;
		reverb.setParameters(verbParams);

		return true;
	}

	virtual bool canProcessAudioFrame() { return true; }

	// --- do the valve emulation
	virtual double processAudioSample(double xn)
	{
		float input[2] = { xn, xn };
		float output[2] = { 0.0, 0.0 };
		processAudioFrame(input, output, 1, 1);
		return output[0];
	}

	/** process STEREO audio amp in frames */
	virtual bool processAudioFrame(const float* inputFrame,		/* ptr to one frame of data: pInputFrame[0] = left, pInputFrame[1] = right, etc...*/
		float* outputFrame,
		uint32_t inputChannels,
		uint32_t outputChannels)
	{
		// --- make sure we have input and outputs
		if (inputChannels == 0 || outputChannels == 0)
			return false;

		// --- pick up inputs
		//
		// --- LEFT channel - know we must have one
		double xnL = inputFrame[0];
		double ynL = tubeAmpChannel[0].processAudioSample(xnL);
		OneMarkAmpParameters ampParams = tubeAmpChannel[0].getParameters();
		double leftDCMeter = ampParams.dcShift[0];

		// --- RIGHT channel (duplicate left input if mono-in)
		double xnR = inputChannels > 1 ? inputFrame[1] : xnL;

		// --- right
		double ynR = tubeAmpChannel[1].processAudioSample(xnR);
		ampParams = tubeAmpChannel[1].getParameters();
		double rightDCMeter = ampParams.dcShift[0];

		// --- LED meter
		parameters.dcShiftTriode_0 = fmax(leftDCMeter, rightDCMeter);

		float reverbInput[2] = { ynL , ynR };
		float reverbOutput[2] = { 0.0 , 0.0 };

		reverb.processAudioFrame(reverbInput, reverbOutput, 2, 2);

		// --- set left channel
		outputFrame[0] = reverbOutput[0];

		// --- set right channel
		outputFrame[1] = reverbOutput[1];

		return true;
	}

	/** get parameters: note use of custom structure for passing param data */
	/**
	\return ValveEmulatorParameters custom data structure
	*/
	WickerAmpParameters getParameters()
	{
		return parameters;
	}

	/** set parameters: note use of custom structure for passing param data */
	/**
	\param ValveEmulatorParameters custom data structure
	*/
	void setParameters(const WickerAmpParameters& params)
	{
		parameters = params;

		tubeAmpChannel[0].setParameters(parameters.ampParameters);
		tubeAmpChannel[1].setParameters(parameters.ampParameters);

		double reverbWet_dB = parameters.masterReverb_010 == 0.0 ? -96.0 : 
														calcMappedVariableOnRange(0.0, 10.0,
														-60.0, 0.0,
														parameters.masterReverb_010);

		ReverbTankParameters verbParams = reverb.getParameters();
		verbParams.wetLevel_dB = reverbWet_dB;
		reverb.setParameters(verbParams);
	}

private:
	WickerAmpParameters parameters; ///< object parameters
	double sampleRate = 0.0;

	// --- two channels of tube amp goodness
	OneMarkAmp tubeAmpChannel[2];

	// --- reverb
	ReverbTank reverb;
};

#endif