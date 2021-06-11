#include "MiniSynthVoice.h"

CMiniSynthVoice::CMiniSynthVoice(void)
{
	// 1) --- declare your oscillators and filters
	// --- oscillators
	m_pOsc1 = &m_Osc1;
	m_pOsc2 = &m_Osc2;
	m_pOsc3 = &m_Osc3;
	m_pOsc4 = &m_Osc4;

	// --- filters
	m_pFilter1 = &m_MoogLadderFilter;
	m_pFilter2 = NULL;

	// --- experiment with NLP
	// m_MoogLadderFilter.m_uNLP = ON;
    	
	// --- for passband gain comp in MOOG; can make user adjustable, the higher m_dAuxControl, the more passband gain 
	//     0 <= m_dAuxControl <= 1
	m_MoogLadderFilter.m_dAuxControl = 0.0; 

    // --- voice mode 0
    m_Osc1.m_uWaveform = SAW1;
    m_Osc2.m_uWaveform = SAW1;
    m_Osc3.m_uWaveform = SAW1;
    m_Osc4.m_uWaveform = NOISE;

	// 2) --- set any component specific stuff
	m_EG1.setEGMode(analog);
	m_EG1.m_bOutputEG = true; // our DCA EG
	
	// --- DCA Setup: set the source EG here
	m_DCA.m_uModSourceEG = DEST_DCA_EG;

}

void CMiniSynthVoice::initializeModMatrix(CModulationMatrix* pMatrix)
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

	// LFO1 -> PULSE WIDTH
	pRow = createModMatrixRow(SOURCE_LFO1,
							  DEST_ALL_OSC_PULSEWIDTH,
							  &m_dDefaultModIntensity,
							  &m_dDefaultModRange,
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

CMiniSynthVoice::~CMiniSynthVoice(void)
{
}

void CMiniSynthVoice::setSampleRate(double dSampleRate)
{
	CVoice::setSampleRate(dSampleRate);
}

void CMiniSynthVoice::prepareForPlay()
{
	CVoice::prepareForPlay();
	reset();
}

void CMiniSynthVoice::update()
{
	// --- voice specific updates
	if(!m_pGlobalVoiceParams) return;

	// --- save, base class overwrites
	UINT uCurrentVoiceMode = m_uVoiceMode;

	// --- always call base class first
	CVoice::update();

	// --- Voice Mode
	// --- only update if needed
	if(m_uVoiceMode != uCurrentVoiceMode) 
	{
		m_uVoiceMode = m_pGlobalVoiceParams->uVoiceMode;
					
		// osc3 is sub osc
		m_Osc3.m_nOctave = -1.0;

		// osc4 is always noise
		m_pGlobalSynthParams->osc4Params.uWaveform = NOISE;

		switch(m_uVoiceMode)
		{
			case Saw3:
				m_pGlobalSynthParams->osc1Params.uWaveform = SAW1;
				m_pGlobalSynthParams->osc2Params.uWaveform = SAW1;
				m_pGlobalSynthParams->osc3Params.uWaveform = SAW1;
				break;

			case Sqr3:
				m_pGlobalSynthParams->osc1Params.uWaveform = SQUARE;
				m_pGlobalSynthParams->osc2Params.uWaveform = SQUARE;
				m_pGlobalSynthParams->osc3Params.uWaveform = SQUARE;
				break;

			case Saw2Sqr: // note will loose sub-oct due to waveform combination
				m_pGlobalSynthParams->osc1Params.uWaveform = SAW1;
				m_pGlobalSynthParams->osc2Params.uWaveform = SQUARE;
				m_pGlobalSynthParams->osc3Params.uWaveform = SAW1;
				break;
		
			case Tri2Saw:
				m_pGlobalSynthParams->osc1Params.uWaveform = TRI;
				m_pGlobalSynthParams->osc2Params.uWaveform = SAW1;
				m_pGlobalSynthParams->osc3Params.uWaveform = TRI;
				break;

			case Tri2Sqr:
				m_pGlobalSynthParams->osc1Params.uWaveform = TRI;
				m_pGlobalSynthParams->osc2Params.uWaveform = SQUARE;
				m_pGlobalSynthParams->osc3Params.uWaveform = TRI;
				break;
		
			default:
				m_pGlobalSynthParams->osc1Params.uWaveform = SAW1;
				m_pGlobalSynthParams->osc2Params.uWaveform = SAW1;
				m_pGlobalSynthParams->osc3Params.uWaveform = SAW1;
				break;
		}

		m_Osc1.reset();
		m_Osc2.reset();
		m_Osc3.reset();
	}
}

void CMiniSynthVoice::reset()
{
	CVoice::reset();

	m_dPortamentoInc = 0.0;
	m_Osc1.m_uWaveform = SAW1;
	m_Osc2.m_uWaveform = SAW1;
	m_Osc3.m_uWaveform = SAW1;
	m_Osc4.m_uWaveform = NOISE;
}