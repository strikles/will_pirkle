#include "DXSynthVoice.h"

CDXSynthVoice::CDXSynthVoice(void)
{
	// --- declare your oscillators and filters
	m_pOsc1 = &m_Op1;
	m_pOsc2 = &m_Op2;
	m_pOsc3 = &m_Op3;
	m_pOsc4 = &m_Op4;

	// --- clear our new variables
	m_dOp1Feedback = 0.0;
	m_dOp2Feedback = 0.0;
	m_dOp3Feedback = 0.0;
	m_dOp4Feedback = 0.0;
}

void CDXSynthVoice::initializeModMatrix(CModulationMatrix* pMatrix)
{
	// --- always first: call base class to create core and init with basic routings
	CVoice::initializeModMatrix(pMatrix);

	if(!pMatrix->getModMatrixCore()) return;

	// --- DX SYNTH SPECIFIC MOD MATRIX - different from the others because only uses a 
	//     singe LFO Intensity control for all LFO mod routings (demonstratig an alternate
	//     way to do this)
	//
	// --- these are also OFF by default but you can easily allow the user 
	//     to enable/disable
	modMatrixRow* pRow = NULL;

	// LFO1 -> DEST_OSC1_OUTPUT_AMP (Amplitude Modulation)
	pRow = createModMatrixRow(SOURCE_LFO1,
							  DEST_OSC1_OUTPUT_AMP,
							  &m_pGlobalVoiceParams->dLFO1OscModIntensity,
							  &m_dDefaultModRange, // this is used for AM not tremolo
							  TRANSFORM_BIPOLAR_TO_UNIPOLAR,
							  false); /* DISABLED BY DEFAULT */
	pMatrix->addModMatrixRow(pRow);
	
	// LFO1 -> DEST_OSC1_FO (Vibrato)
	pRow = createModMatrixRow(SOURCE_LFO1,
							  DEST_OSC1_FO,
							  &m_pGlobalVoiceParams->dLFO1OscModIntensity,
							  &m_pGlobalVoiceParams->dOscFoModRange, // this is used for vibrato
							  TRANSFORM_NONE,
							  false); /* DISABLED BY DEFAULT */
	pMatrix->addModMatrixRow(pRow);


	// LFO1 -> DEST_OSC2_OUTPUT_AMP (Amplitude Modulation)
	pRow = createModMatrixRow(SOURCE_LFO1,
							  DEST_OSC2_OUTPUT_AMP,
							  &m_pGlobalVoiceParams->dLFO1OscModIntensity,
							  &m_dDefaultModRange, // this is used for AM not tremolo
							  TRANSFORM_BIPOLAR_TO_UNIPOLAR,
							  false); /* DISABLED BY DEFAULT */
	pMatrix->addModMatrixRow(pRow);
	
	// LFO1 -> DEST_OSC2_FO (Vibrato)
	pRow = createModMatrixRow(SOURCE_LFO1,
							  DEST_OSC2_FO,
							  &m_pGlobalVoiceParams->dLFO1OscModIntensity,
							  &m_pGlobalVoiceParams->dOscFoModRange, // this is used for vibrato
							  TRANSFORM_NONE,
							  false); /* DISABLED BY DEFAULT */
	pMatrix->addModMatrixRow(pRow);


	// LFO1 -> DEST_OSC31_OUTPUT_AMP (Amplitude Modulation)
	pRow = createModMatrixRow(SOURCE_LFO1,
							  DEST_OSC3_OUTPUT_AMP,
							  &m_pGlobalVoiceParams->dLFO1OscModIntensity,
							  &m_dDefaultModRange, // this is used for AM not tremolo
							  TRANSFORM_BIPOLAR_TO_UNIPOLAR,
							  false); /* DISABLED BY DEFAULT */
	pMatrix->addModMatrixRow(pRow);
	
	// LFO1 -> DEST_OSC3_FO (Vibrato)
	pRow = createModMatrixRow(SOURCE_LFO1,
							  DEST_OSC3_FO,
							  &m_pGlobalVoiceParams->dLFO1OscModIntensity,
							  &m_pGlobalVoiceParams->dOscFoModRange, // this is used for vibrato
							  TRANSFORM_NONE,
							  false); /* DISABLED BY DEFAULT */
	pMatrix->addModMatrixRow(pRow);

	// LFO1 -> DEST_OSC4_OUTPUT_AMP (Amplitude Modulation)
	pRow = createModMatrixRow(SOURCE_LFO1,
							  DEST_OSC4_OUTPUT_AMP,
							  &m_pGlobalVoiceParams->dLFO1OscModIntensity,
							  &m_dDefaultModRange, // this is used for AM not tremolo
							  TRANSFORM_BIPOLAR_TO_UNIPOLAR,
							  false); /* DISABLED BY DEFAULT */
	pMatrix->addModMatrixRow(pRow);
	
	// LFO1 -> DEST_OSC4_FO (Vibrato)
	pRow = createModMatrixRow(SOURCE_LFO1,
							  DEST_OSC4_FO,
							  &m_pGlobalVoiceParams->dLFO1OscModIntensity,
							  &m_pGlobalVoiceParams->dOscFoModRange, // this is used for vibrato
							  TRANSFORM_NONE,
							  false); /* DISABLED BY DEFAULT */
	pMatrix->addModMatrixRow(pRow);
}

CDXSynthVoice::~CDXSynthVoice(void)
{
}

void CDXSynthVoice::setSampleRate(double dSampleRate)
{
	CVoice::setSampleRate(dSampleRate);
}

void CDXSynthVoice::prepareForPlay()
{
	CVoice::prepareForPlay();
	reset();
}

void CDXSynthVoice::reset()
{
	CVoice::reset();
	m_dPortamentoInc = 0.0;
}

void CDXSynthVoice::update()
{
	// --- voice specific updates
	if(!m_pGlobalVoiceParams) return;

	// --- always call base class first
	CVoice::update();

	// --- new DX additions
	m_dOp1Feedback = m_pGlobalVoiceParams->dOp1Feedback;
	m_dOp2Feedback = m_pGlobalVoiceParams->dOp2Feedback;		
	m_dOp3Feedback = m_pGlobalVoiceParams->dOp3Feedback;
	m_dOp4Feedback = m_pGlobalVoiceParams->dOp4Feedback;
}