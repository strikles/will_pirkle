#pragma once
#include "oscillator.h"

class CSampleOscillator : public COscillator
{
public:
	CSampleOscillator(void);
	~CSampleOscillator(void);

	// sample contains one-cycle
	bool m_bSingleCycleSample; 

	// if pitchless, inc = 1
	bool m_bPitchlessSample;  
	
	UINT m_uLoopMode; 
	enum {sustain, loop, oneShot};// 0 = Sustain, 1 = Loop, 2 = 1-shot
	
	CWaveData* getWaveData(){return m_pWaveSample;}
	CWaveData** getWaveSamplePtrArray(){return &m_WaveSamplePtrArray[0];}

protected:
	// table (buffer) read index values
	double m_dReadIndex;	// NOTE its a double!
	
	bool m_bSharedWaveSample;
	bool m_bSharedWaveSamplePtrArray;

	// single sample
	CWaveData* m_pWaveSample;
	
	// multi-samples
	CWaveData*  m_WaveSamplePtrArray[128];
	CWaveData**  m_ppWaveSamplePtrArray;

	// we can be mono or stereo
	double m_dLeftOutputSample;
	double m_dRightOutputSample;

	// a sub-function to read the sample buffer
	// pWaveSample = the buffer of samples
	// dReadIndex, dInc = the read-index/inc pair for this operation
	// dLeftSample, dRightSample = the two outputs; if mono sample, right is a duplicate of left
	void readSampleBuffer(CWaveData* pWaveSample, double& dReadIndex, double dInc, double& dLeftSample, double& dRightSample);

public:
	// --- init globals
	inline virtual void initGlobalParameters(globalOscillatorParams* pGlobalOscParams)
	{
		// --- always call base class first to store pointer
		COscillator::initGlobalParameters(pGlobalOscParams);

		// --- add any CSampleOscillator specifics here
		m_pGlobalOscParams->uLoopMode = m_uLoopMode;
	}

	// load the wave file
	bool addWaveSample(char* pSamplePath, int nMIDINoteNumber = -1);
	
	// get information about sample
	inline int getNumChannels()
	{
		if(!m_pWaveSample)
			return -1;
		return m_pWaveSample->m_uNumChannels;
	}
	
	// get our MIDI note
	inline UINT getSampleMIDINote()
	{
		if(!m_pWaveSample)
			return 0;
		return m_pWaveSample->m_uMIDINote;
	}

	// virtual overrides	
	virtual void reset();

	// recalculate due to change in fo
	virtual void update();

	// start/stop control
	virtual void startOscillator();
	virtual void stopOscillator();
	
	// init a single sample
	virtual bool initWithFilePath(char* pSamplePath);
	virtual bool setWaveDataPtr(CWaveData* pWaveData);
	
	// for multi-samples or drums; init with a folder of wav files
	virtual bool initWithFolderPath(char* pSamplePath);
	virtual bool setWaveSamplePtrArray(CWaveData** ppWaveSamplePtrArray);

	// to check if we have a sample loaded and ready to play
	virtual bool sampleIsLoaded(){return !m_pWaveSample ? false : m_pWaveSample->m_bWaveLoaded;}

	// render a sample - this calculates the outputs
	// returns m_dLeftOutputSample;
	// *pAuxOutput = m_dRightOutputSample;
	virtual double doOscillate(double* pAuxOutput = NULL); // pure abstract 
};
