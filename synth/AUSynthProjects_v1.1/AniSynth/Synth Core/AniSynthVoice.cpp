#include "AniSynthVoice.h"

CAniSynthVoice::CAniSynthVoice(void)
{
	m_ppOscArray = NULL;
	m_nCellsPerSide = 0;
	m_nCells = 0;
	m_nCurrentCell = -1;

	// --- connect our filters
	m_pFilter1 = &m_LeftDiodeFilter;
	m_pFilter2 = &m_RightDiodeFilter;
	
	// --- experiment with NLP
	m_LeftDiodeFilter.m_uNLP = OFF;
	m_RightDiodeFilter.m_uNLP = OFF;

	// --- for passband gain comp in MOOG; see book
	m_LeftDiodeFilter.m_dAuxControl = 0.0;
	m_RightDiodeFilter.m_dAuxControl = 0.0;
}

void CAniSynthVoice::initializeModMatrix(CModulationMatrix* pMatrix)
{
	// --- always first: call base class to create core and init with basic routings
	CVectorSynthVoice::initializeModMatrix(pMatrix);
}

CAniSynthVoice::~CAniSynthVoice(void)
{
	if(m_ppOscArray)
	{
		int nArraySize = pow(m_nCellsPerSide + 1.0, 2.0);

		for(int i=0; i<nArraySize; i++)
			delete m_ppOscArray[i];

		delete [] m_ppOscArray;
	}
}

bool CAniSynthVoice::createOscArray(int nCellsPerSide)
{
	// --- delete if existing
	if(m_ppOscArray)
	{
		// --- size	
		int nArraySize = pow(m_nCellsPerSide + 1.0, 2.0);

		for(int i=0; i<nArraySize; i++)
			delete m_ppOscArray[i];
		delete [] m_ppOscArray;
	}

	// --- size	(use argument)
	int nArraySize = pow(nCellsPerSide + 1.0, 2.0);

	// --- create new array of pointers
	m_ppOscArray = new CSampleOscillator*[36];

	// --- validate
	if(!m_ppOscArray)
        return false;

	// --- load up with new oscillator pointers
	for(int i=0; i<nArraySize; i++)
		m_ppOscArray[i] = new CSampleOscillator;

	// --- store for later
	m_nCellsPerSide = nCellsPerSide;
	m_nCells = pow(m_nCellsPerSide, 2.0);

	return true;
}

void CAniSynthVoice::prepareForPlay()
{
	CVectorSynthVoice::prepareForPlay();

	// --- for ANISYNTH these MUST all be single cycle
	((CSampleOscillator*)m_pOsc1)->m_bSingleCycleSample = true;
	((CSampleOscillator*)m_pOsc2)->m_bSingleCycleSample = true;
	((CSampleOscillator*)m_pOsc3)->m_bSingleCycleSample = true;
	((CSampleOscillator*)m_pOsc4)->m_bSingleCycleSample = true;

	((CSampleOscillator*)m_pOsc1)->m_bPitchlessSample = false;
	((CSampleOscillator*)m_pOsc2)->m_bPitchlessSample = false;
	((CSampleOscillator*)m_pOsc3)->m_bPitchlessSample = false;
	((CSampleOscillator*)m_pOsc4)->m_bPitchlessSample = false;

	if(m_nCells != 0)
		setActiveCell((m_nCells - 1)/2.0);
}
