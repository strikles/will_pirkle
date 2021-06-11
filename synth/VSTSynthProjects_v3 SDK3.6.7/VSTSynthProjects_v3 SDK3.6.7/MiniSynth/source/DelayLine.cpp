#include "DelayLine.h"
#include "pluginconstants.h"

CDelayLine::CDelayLine(void)
{
	// --- zero everything
	m_pBuffer = NULL;
	m_dDelay_ms = 0.0;
	m_dDelayInSamples = 0.0;
	m_nSampleRate = 0;

	// --- reset
	resetDelay();
}

CDelayLine::~CDelayLine(void)
{
	if(m_pBuffer)
		delete m_pBuffer;

	m_pBuffer = NULL;
}

void CDelayLine::init(int nDelayLength)
{
	// --- save for later
	m_nBufferSize = nDelayLength;

	// --- delete if existing
	if(m_pBuffer)
		delete m_pBuffer;

	// --- create
	m_pBuffer = new double[m_nBufferSize];

	// --- flush buffer
	memset(m_pBuffer, 0, m_nBufferSize*sizeof(double));
}

void CDelayLine::resetDelay()
{
	// --- flush buffer
	if(m_pBuffer)
		memset(m_pBuffer, 0, m_nBufferSize*sizeof(double));

	// --- init read/write indices
	m_nWriteIndex = 0; 
	m_nReadIndex = 0; 

	// --- cook
	cookVariables();
}

void CDelayLine::setDelay_mSec(double dmSec)
{
	m_dDelay_ms = dmSec;
	cookVariables();
}

void CDelayLine::cookVariables()
{
	// --- calculate fractional delay
	m_dDelayInSamples = m_dDelay_ms*((double)m_nSampleRate/1000.0);

	// --- subtract to make read index
	m_nReadIndex = m_nWriteIndex - (int)m_dDelayInSamples;

	// --- the check and wrap BACKWARDS if the index is negative
	if (m_nReadIndex < 0)
		m_nReadIndex += m_nBufferSize;	// amount of wrap is Read + Length
}


// --- read delay at the prescribed delay value
double CDelayLine::readDelay()
{
	// --- Read the output of the delay at m_nReadIndex
	double yn = m_pBuffer[m_nReadIndex];

	// --- Read the location ONE BEHIND yn at y(n-1)
	int nReadIndex_1 = m_nReadIndex - 1;
	if(nReadIndex_1 < 0)
		nReadIndex_1 = m_nBufferSize-1; // m_nBufferSize-1 is last location

	// --- get y(n-1)
	double yn_1 = m_pBuffer[nReadIndex_1];

	// --- get fractional component
	double dFracDelay = m_dDelayInSamples - (int)m_dDelayInSamples;
	
	// --- interpolate: (0, yn) and (1, yn_1) by the amount fracDelay
	return dLinTerp(0, 1, yn, yn_1, dFracDelay); // interp frac between them
}

// --- read delay from an arbitrary location given in mSec
double CDelayLine::readDelayAt(double dmSec)
{
	// --- local variales
	double dDelayInSamples = dmSec*((float)m_nSampleRate)/1000.0;

	// --- subtract to make read index
	int nReadIndex = m_nWriteIndex - (int)dDelayInSamples;

	// --- wrap if needed
	if (nReadIndex < 0)
		nReadIndex += m_nBufferSize;	// amount of wrap is Read + Length

	//---  Read the output of the delay at m_nReadIndex
	double yn = m_pBuffer[nReadIndex];

	// --- Read the location ONE BEHIND yn at y(n-1)
	int nReadIndex_1 = nReadIndex - 1;
	if(nReadIndex_1 < 0)
		nReadIndex_1 = m_nBufferSize-1; // m_nBufferSize-1 is last location

	// -- get y(n-1)
	double yn_1 = m_pBuffer[nReadIndex_1];

	// --- get the fractional component
	double dFracDelay = dDelayInSamples - (int)dDelayInSamples;
	
	// --- interpolate: (0, yn) and (1, yn_1) by the amount fracDelay
	return dLinTerp(0, 1, yn, yn_1, dFracDelay); // interp frac between them

}

// --- this is used internally; normally you don't call it directly
//     but you can if you want to extend the modes of operation
void CDelayLine::writeDelayAndInc(double dDelayInput)
{
	// --- write to the delay line
	m_pBuffer[m_nWriteIndex] = dDelayInput; // external feedback sample

	// --- increment the pointers and wrap if necessary
	m_nWriteIndex++;
	if(m_nWriteIndex >= m_nBufferSize)
		m_nWriteIndex = 0;

	m_nReadIndex++;
	if(m_nReadIndex >= m_nBufferSize)
		m_nReadIndex = 0;
}


// --- the normal way to use the object - this implements the delay
//     no feedback, no wet/dry - these must be done in owning object
bool CDelayLine::processAudio(double* pInput, double* pOutput)
{
	// read delayed output
	*pOutput = m_dDelayInSamples == 0 ? *pInput : readDelay();

	// write to the delay line
	writeDelayAndInc(*pInput);

	return true; // all OK
}

