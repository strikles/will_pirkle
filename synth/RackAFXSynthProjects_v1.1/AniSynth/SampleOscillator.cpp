#include "SampleOscillator.h"

CSampleOscillator::CSampleOscillator(void)
{
	// --- NULL
	m_pWaveSample = NULL;

	// --- clear the array
	memset(&m_WaveSamplePtrArray[0], 0, sizeof(CWaveData*)*128);

	// --- this is to allow the array to be shared
	m_ppWaveSamplePtrArray = &m_WaveSamplePtrArray[0];

	// --- clear outputs
	m_dLeftOutputSample = 0.0;
	m_dRightOutputSample = 0.0;

	// --- reset read index to top of buffer (address 0)
	m_dReadIndex = 0.0;

	// --- set defaults
	m_bSingleCycleSample = false;
	m_bPitchlessSample = false;
	m_bSharedWaveSample = false;
	m_bSharedWaveSamplePtrArray = false;
	m_uLoopMode = sustain; // default loop mode
}

CSampleOscillator::~CSampleOscillator(void)
{
	if(!m_bSharedWaveSample)
	{
		if(m_pWaveSample) delete m_pWaveSample;
	}

	if(!m_bSharedWaveSamplePtrArray)
	{
		CWaveData* pSample = NULL;
		CWaveData* pDeletedSample = NULL;
		for(int i=0; i<127; i++)
		{
			if(m_WaveSamplePtrArray[i])
			{
				pSample = m_WaveSamplePtrArray[i];
				if(pSample != pDeletedSample)
				{
					pDeletedSample = pSample;
					delete pSample;
				}
				m_WaveSamplePtrArray[i] = NULL;
			}
		}
	}
}

// load the wave file
bool CSampleOscillator::initWithFilePath(char* pSamplePath)
{
	// --- destroy if existing
	if(m_pWaveSample)
		delete m_pWaveSample;

	// --- create the new object
	m_pWaveSample = new CWaveData(pSamplePath);

	// --- did we find the sample?
	if(!m_pWaveSample->m_bWaveLoaded)
	{
		delete m_pWaveSample;
		m_pWaveSample = NULL;
		return false;
	}

	// --- not sharing if we just created it
	m_bSharedWaveSample = false;

	return true;
}

bool CSampleOscillator::setWaveDataPtr(CWaveData* pWaveData)
{
	// --- have sample?
	if(!pWaveData)
		return false;

	// --- just copy the pointer
	m_pWaveSample = pWaveData;

	// --- we are sharing so set flag
	m_bSharedWaveSample = true;

	return true;
}

#if defined _WINDOWS || defined _WINDLL

// assumes folder full of audio files
bool CSampleOscillator::initWithFolderPath(char* pSampleFolderPath)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	HANDLE hFirstFind;

	char* pFolderWildcard = addStrings(pSampleFolderPath, "\\*.wav");

	// find first file
	hFirstFind = FindFirstFileEx(pFolderWildcard, FindExInfoStandard, &FindFileData, FindExSearchNameMatch, NULL, 0);
	delete [] pFolderWildcard;

	if(hFirstFind == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	// clear out
	if(!m_bSharedWaveSamplePtrArray)
	{
		CWaveData* pSample = NULL;
		CWaveData* pDeletedSample = NULL;
		for(int i=0; i<127; i++)
		{
			if(m_WaveSamplePtrArray[i])
			{
				pSample = m_WaveSamplePtrArray[i];
				if(pSample != pDeletedSample)
				{
					pDeletedSample = pSample;
					delete pSample;
				}
				m_WaveSamplePtrArray[i] = NULL;
			}
		}
	}

	// save
	hFind = hFirstFind;

	// add first file
	char* pFileFolder = addStrings(pSampleFolderPath, "\\");
	bool bWorking = true;
	while(bWorking)
	{
		// make the path
		char* pFilePath = addStrings(pFileFolder, FindFileData.cFileName);

		// call sub-function to add the files
		addWaveSample(pFilePath);

		// delete path
		delete [] pFilePath;

		// find the next one and do it again until no more .wav files
		bWorking = FindNextFile(hFind, &FindFileData);
	}

	// delete path
	delete [] pFileFolder;

	// close the finder
	FindClose(hFirstFind);

	// now comb the array and replicate pointers
	int nLastIndex = -1;
	CWaveData* pLastSample = NULL;
	for(int i=0; i<127; i++)
	{
		if(m_WaveSamplePtrArray[i])
		{
			nLastIndex = i;
			pLastSample = m_WaveSamplePtrArray[i];
		}
	}

	if(!pLastSample)
		return false;// no samples : (

	// upper part first
	for(int i=127; i >=nLastIndex; i--)
	{
		m_WaveSamplePtrArray[i] = pLastSample;
	}

	int index = nLastIndex-1; // first index already has value in it
  	bWorking = index >= 0 ? true : false;

	while(bWorking)
	{
		if(!m_WaveSamplePtrArray[index])
			m_WaveSamplePtrArray[index] = pLastSample;
		else
			pLastSample = m_WaveSamplePtrArray[index];

		index--;

		if(index < 0)
			bWorking = false;
	}

	// this is to allow the array to be shared
	m_ppWaveSamplePtrArray = &m_WaveSamplePtrArray[0];

	m_bSharedWaveSamplePtrArray = false;

	return true;
}

#else // <--- MacOS version here --->

#import <CoreFoundation/CoreFoundation.h>

// assumes folder full of audio files
bool CSampleOscillator::initWithFolderPath(char* pSampleFolderPath)
{
    // clear out
	if(!m_bSharedWaveSamplePtrArray)
	{
		CWaveData* pSample = NULL;
		CWaveData* pDeletedSample = NULL;
		for(int i=0; i<127; i++)
		{
			if(m_WaveSamplePtrArray[i])
			{
				pSample = m_WaveSamplePtrArray[i];
				if(pSample != pDeletedSample)
				{
					pDeletedSample = pSample;
					delete pSample;
				}
				m_WaveSamplePtrArray[i] = NULL;
			}
		}
	}

    // --- iterate through the wave files in the folder
    CFStringRef path = CFStringCreateWithCString(NULL, pSampleFolderPath, kCFStringEncodingASCII);
    CFURLRef pathURL = CFURLCreateWithFileSystemPath(NULL, path, kCFURLPOSIXPathStyle, true);
    CFURLEnumeratorRef enumerator = CFURLEnumeratorCreateForDirectoryURL(NULL, pathURL, kCFURLEnumeratorSkipInvisibles, NULL);
    CFURLRef childURL;
    CFURLEnumeratorResult enumeratorResult;

    do // iterate
    {
        enumeratorResult = CFURLEnumeratorGetNextURL(enumerator, &childURL, NULL);
        if (enumeratorResult == kCFURLEnumeratorSuccess)
        {
            // --- convert to char*
            CFStringRef pathString = CFURLCopyPath(childURL);

            int nSize = CFStringGetLength(pathString);
            char* path = new char[nSize+1];
            memset(path, 0, (nSize+1)*sizeof(char));

            bool success = CFStringGetCString(pathString, path, nSize+1, kCFStringEncodingASCII);
            CFRelease(pathString);

            // call sub-function to add the files
            if(success) addWaveSample(path);

            // delete path
            delete [] path;
        }
        else if (enumeratorResult == kCFURLEnumeratorError)
        {
            return false;
        }
    } while (enumeratorResult != kCFURLEnumeratorEnd);

    // now comb the array and replicate pointers
	int nLastIndex = -1;
	CWaveData* pLastSample = NULL;
	for(int i=0; i<127; i++)
	{
		if(m_WaveSamplePtrArray[i])
		{
			nLastIndex = i;
			pLastSample = m_WaveSamplePtrArray[i];
		}
	}

	if(!pLastSample)
		return false;// no samples : (

	// upper part first
	for(int i=127; i >=nLastIndex; i--)
	{
		m_WaveSamplePtrArray[i] = pLastSample;
	}

	int index = nLastIndex-1; // first index already has value in it
  	bool bWorking = index >= 0 ? true : false;

	while(bWorking)
	{
		if(!m_WaveSamplePtrArray[index])
			m_WaveSamplePtrArray[index] = pLastSample;
		else
			pLastSample = m_WaveSamplePtrArray[index];

		index--;

		if(index < 0)
			bWorking = false;
	}

	// this is to allow the array to be shared
	m_ppWaveSamplePtrArray = &m_WaveSamplePtrArray[0];

	m_bSharedWaveSamplePtrArray = false;

	return true;
}


#endif

bool CSampleOscillator::setWaveSamplePtrArray(CWaveData** ppWaveSamplePtrArray)
{
	// --- test
	if(!ppWaveSamplePtrArray)
		return false;

	// --- just copy it
	m_ppWaveSamplePtrArray = ppWaveSamplePtrArray;

	// --- flag so we don't delete
	m_bSharedWaveSamplePtrArray = true;

	return true;
}

bool CSampleOscillator::addWaveSample(char* pSamplePath, int nMIDINoteNumber)
{
	// create the new object
	CWaveData* pSample = new CWaveData(pSamplePath);

	// has MIDI Note number?
	if(pSample && nMIDINoteNumber < 0)
		nMIDINoteNumber = pSample->m_uMIDINote;

	// add to wave table list
	m_ppWaveSamplePtrArray[nMIDINoteNumber] = pSample;

	return true;
}

void CSampleOscillator::reset()
{
	// --- base class first
	COscillator::reset();

	// --- clear outputs
	m_dLeftOutputSample = 0.0;
	m_dRightOutputSample = 0.0;

	// --- reset read index to top of buffer (address 0)
	m_dReadIndex = 0.0;

	//---  update
	update();
}

// recalculate due to change in fo
void CSampleOscillator::update()
{
	// --- this calculates the modulated pitch
	COscillator::update();

	// --- Global Parameters
	if(m_pGlobalOscParams)
	{
		m_uLoopMode = m_pGlobalOscParams->uLoopMode;
	}

	// --- check sample
	CWaveData* pSample = m_pWaveSample;
	if(!pSample)
		pSample = m_ppWaveSamplePtrArray[m_uMIDINoteNumber];

	if(!pSample) return;

	// --- pitchless is simple
	if(m_bPitchlessSample)
	{
		m_dInc = 1.0;
		return;
	}

	// --- get unity note frequency
	double dUnityFrequency = m_bSingleCycleSample ? (m_dSampleRate/((float)pSample->m_uSampleCount/(float)pSample->m_uNumChannels)) : midiFreqTable[pSample->m_uMIDINote];

	// equivalent length
	double dLength = m_dSampleRate/dUnityFrequency;

	// calculate increment
	// inc = L(fo/fs)
	m_dInc *= dLength;
}


// start/stop control
void CSampleOscillator::startOscillator()
{
	reset();
	m_bNoteOn = true;
}

void CSampleOscillator::stopOscillator()
{
	m_bNoteOn = false;
}

// CHALLENGE: modify this to accomodate Phase Modulation!
void CSampleOscillator::readSampleBuffer(CWaveData* pWaveSample, double& dReadIndex, double dInc, double& dLeftSample, double& dRightSample)
{
	// --- wavetable reads; starting with left channel
	int nReadIndex = (int)dReadIndex;

	// --- get FRAC part
	float fFrac = dReadIndex - nReadIndex;

	// --- mono or stereo file? CURRENTLY ONLY SUPPORTING THESE 2
	if(pWaveSample->m_uNumChannels == 1)
	{
		// setup second index for interpolation; wrap the buffer if needed
		int nReadIndexNext = nReadIndex + 1 > pWaveSample->m_uSampleCount-1 ? 0 :  nReadIndex + 1;

		// interpolate between the two
		dLeftSample = dLinTerp(0, 1, pWaveSample->m_pWaveBuffer[nReadIndex], pWaveSample->m_pWaveBuffer[nReadIndexNext], fFrac);
		dRightSample = dLeftSample;

		// inc for next time
		dReadIndex += dInc;
	}
	else if(pWaveSample->m_uNumChannels == 2)
	{
		// --- interpolate across interleaved buffer!
		int nReadIndexLeft = (int)dReadIndex * 2;

		// --- setup second index for interpolation; wrap the buffer if needed, we know last sample is Right channel
		//     so reset to top (the 0 after ?)
		int nReadIndexNextLeft = nReadIndexLeft + 2 > pWaveSample->m_uSampleCount-1 ? 0 :  nReadIndexLeft + 2;

		// --- interpolate between the two
		dLeftSample = dLinTerp(0, 1, pWaveSample->m_pWaveBuffer[nReadIndexLeft], pWaveSample->m_pWaveBuffer[nReadIndexNextLeft], fFrac);

		// --- do the right channel
		int nReadIndexRight = nReadIndexLeft + 1;

		// --- find the next one, skipping over, note wrap goes to index 1 ---> 1
		int nReadIndexNextRight = nReadIndexRight + 2 > pWaveSample->m_uSampleCount-1 ? 1 :  nReadIndexRight + 2;

		// --- interpolate between the two
		dRightSample = dLinTerp(0, 1, pWaveSample->m_pWaveBuffer[nReadIndexRight], pWaveSample->m_pWaveBuffer[nReadIndexNextRight], fFrac);

		// --- inc for next time
		dReadIndex += dInc;
	}
}

// returns the mono output and sets
// the m_dLeftOutputSample and m_dRightOutputSample
double CSampleOscillator::doOscillate(double* pAuxOutput)
{
	// --- clear in case of no sample
	m_dLeftOutputSample = 0.0;
	m_dRightOutputSample = 0.0;
	if(pAuxOutput) *pAuxOutput = 0.0;

	// --- get the sample pointer
	CWaveData* pSample = m_pWaveSample;
	if(!pSample)
		pSample = m_ppWaveSamplePtrArray[m_uMIDINoteNumber];

	// --- check and bail if no sample loaded
	if(!pSample || !m_bNoteOn)
		return 0.0;

	// --- one-shot sample
	if(m_dReadIndex < 0)
		return 0.0;

	// --- do the buffer read operation
	readSampleBuffer(pSample, m_dReadIndex, m_dInc, m_dLeftOutputSample, m_dRightOutputSample);

	// --- channel count
	double dChannels = (double)pSample->m_uNumChannels;

	// --- check for wrap
	if(pSample->m_uLoopCount > 0)
	{
		// --- use loop points for looping
		if(m_uLoopMode == sustain)
		{
			if(m_dReadIndex > (double)(pSample->m_uLoopEndIndex)/dChannels)
				m_dReadIndex = m_dReadIndex - (double)(pSample->m_uLoopEndIndex)/dChannels + (double)(pSample->m_uLoopStartIndex)/dChannels;
		}
		else if(m_uLoopMode == loop) // use end->start samples
		{
			if(m_dReadIndex > (double)(pSample->m_uSampleCount - dChannels - 1)/dChannels)
				m_dReadIndex = 0.0;
		}
		else if(m_uLoopMode == oneShot) // use end->start samples
		{
			if(m_dReadIndex > (double)(pSample->m_uSampleCount - dChannels - 1)/dChannels)
				m_dReadIndex = -1;
		}
	}

	// --- if no loop count, treat sustain like one-shot
	if(pSample->m_uLoopCount == 0)
	{
		if(m_uLoopMode == oneShot || m_uLoopMode == sustain)
		{
			if(m_dReadIndex > (double)(pSample->m_uSampleCount - dChannels - 1)/dChannels)
				m_dReadIndex = -1;
		}
		else if(m_uLoopMode == loop) // use end->start samples
		{
			if(m_dReadIndex > (double)(pSample->m_uSampleCount - dChannels - 1)/dChannels)
				m_dReadIndex = 0.0;
		}
	}

	// --- write to outputs
	if(m_pModulationMatrix)
	{
		// --- write our outputs into their destinations
		m_pModulationMatrix->m_dSources[m_uModDestOutput1] = m_dLeftOutputSample*m_dAmplitude*m_dAmpMod;

		// --- CSampleOscillator is stereo!
		m_pModulationMatrix->m_dSources[m_uModDestOutput2] = m_dRightOutputSample*m_dAmplitude*m_dAmpMod;
	}

	// --- aux is right
	if(pAuxOutput)
		*pAuxOutput = m_dRightOutputSample*m_dAmplitude;

	// --- for stand alone use
	return m_dLeftOutputSample*m_dAmplitude;
}
