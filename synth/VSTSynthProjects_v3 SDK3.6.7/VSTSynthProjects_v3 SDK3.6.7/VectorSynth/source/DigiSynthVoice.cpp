#include "DigiSynthVoice.h"

CDigiSynthVoice::CDigiSynthVoice(void)
{
	// declare your oscillators and filters
	m_pOsc1 = &m_OscA;
	m_pOsc2 = &m_OscB;
	m_pOsc3 = NULL;
	m_pOsc4 = NULL;

	m_pFilter1 = &m_LeftSEMFilter;
	m_pFilter2 = &m_RightSEMFilter;

	// Challenge: allow user to change the split/layer range
	m_Osc1NoteRegion.uMIDINoteLow = 0;
	m_Osc1NoteRegion.uMIDINoteHigh = 72;
	m_Osc1NoteRegion.uMIDIVelocityLow = 0;
	m_Osc1NoteRegion.uMIDIVelocityHigh = 100;

	// NOTE: Note and Velocity ranges overlap!
	m_Osc2NoteRegion.uMIDINoteLow = 60;
	m_Osc2NoteRegion.uMIDINoteHigh = 127;
	m_Osc2NoteRegion.uMIDIVelocityLow = 70;
	m_Osc2NoteRegion.uMIDIVelocityHigh = 127;

	// for voice stealing
	m_EG1.m_bResetToZero = true;
	m_EG2.m_bResetToZero = true;
	m_EG3.m_bResetToZero = true;
	m_EG4.m_bResetToZero = true;

	m_LeftSEMFilter.m_uNLP = OFF;
	m_RightSEMFilter.m_uNLP = OFF;

	m_EG1.setEGMode(analog);
	m_EG1.m_bOutputEG = true; // our DCA EG
	
	// --- DCA Setup: set the source EG here
	m_DCA.m_uModSourceEG = DEST_DCA_EG;
}

void CDigiSynthVoice::initializeModMatrix(CModulationMatrix* pMatrix)
{
	// --- always first: call base class to create core and init with basic routings
	CVoice::initializeModMatrix(pMatrix);

	if(!pMatrix->getModMatrixCore()) return;

	// --- MiniSynth Specific Routings
	// --- create a row for each source/destination pair
	modMatrixRow* pRow = NULL;

	// LFO1 -> ALL OSC1 FC
	pRow = createModMatrixRow(SOURCE_LFO1,
							  DEST_ALL_OSC_FO,
							  &m_pGlobalVoiceParams->dLFO1OscModIntensity,
							  &m_pGlobalVoiceParams->dOscFoModRange,
							  TRANSFORM_NONE,
							  true);
	pMatrix->addModMatrixRow(pRow);
	
	// EG1 -> FILTER1 FC
	pRow = createModMatrixRow(SOURCE_BIASED_EG1,
							  DEST_ALL_FILTER_FC,
							  &m_pGlobalVoiceParams->dEG1Filter1ModIntensity,
							  &m_pGlobalVoiceParams->dFilterModRange,
							  TRANSFORM_NONE,
							  true);
	pMatrix->addModMatrixRow(pRow);

	// EG1 -> DCA EG
	pRow = createModMatrixRow(SOURCE_EG1,
							  DEST_DCA_EG,
							  &m_pGlobalVoiceParams->dEG1DCAAmpModIntensity,
							  &m_dDefaultModRange,
							  TRANSFORM_NONE,
							  true);
	pMatrix->addModMatrixRow(pRow);

	// EG1 -> ALL OSC1 FC
	pRow = createModMatrixRow(SOURCE_BIASED_EG1,
							  DEST_ALL_OSC_FO,
							  &m_pGlobalVoiceParams->dEG1OscModIntensity,
							  &m_pGlobalVoiceParams->dOscFoModRange,
							  TRANSFORM_NONE,
							  true);
	pMatrix->addModMatrixRow(pRow);

	// LFO1 -> FILTER1 FC
	pRow = createModMatrixRow(SOURCE_LFO1,
							  DEST_ALL_FILTER_FC,
							  &m_pGlobalVoiceParams->dLFO1Filter1ModIntensity,
							  &m_pGlobalVoiceParams->dFilterModRange,
							  TRANSFORM_NONE,
							  true); 
	pMatrix->addModMatrixRow(pRow);
	
	// LFO1 (-1 -> +1) -> DCA Amp Mod (0->1)
	pRow = createModMatrixRow(SOURCE_LFO1,
							  DEST_DCA_AMP,
							  &m_pGlobalVoiceParams->dLFO1DCAAmpModIntensity,
							  &m_pGlobalVoiceParams->dAmpModRange,
							  TRANSFORM_BIPOLAR_TO_UNIPOLAR,
							  true); 
	pMatrix->addModMatrixRow(pRow);

	// LFO1 (-1 -> +1) -> DCA Pan Mod (-1->1)
	pRow = createModMatrixRow(SOURCE_LFO1,
							  DEST_DCA_PAN,
							  &m_pGlobalVoiceParams->dLFO1DCAPanModIntensity,
							  &m_dDefaultModRange,
							  TRANSFORM_NONE,
							  true); 
	pMatrix->addModMatrixRow(pRow);
}

CDigiSynthVoice::~CDigiSynthVoice(void)
{
}

void CDigiSynthVoice::setSampleRate(double dSampleRate)
{
	CVoice::setSampleRate(dSampleRate);
}

void CDigiSynthVoice::prepareForPlay()
{
	CVoice::prepareForPlay();
	reset();
}

void CDigiSynthVoice::reset()
{
	CVoice::reset();
	m_dPortamentoInc = 0.0;
}

void CDigiSynthVoice::update()
{
	// --- voice specific updates
	if(!m_pGlobalVoiceParams) return;

	// --- always call base class first
	CVoice::update();
}