#include "Voice.h"

CVoice::CVoice(void)
{
	m_dPortamentoTime_mSec = 0.0;
	m_bNoteOn = false;
	m_uTimeStamp = 0;
	m_bNotePending = false;
	m_dSampleRate = 44100;
	m_uVoiceMode = 0;  // this will vary in meaning depending on synth
	m_dHSRatio = 1.0;
	m_dOscPitch = OSC_FO_DEFAULT;
	m_dOscPitchPending = OSC_FO_DEFAULT;
	m_dPortamentoStart = OSC_FO_DEFAULT;
	m_dModuloPortamento = 0.0;
	m_dPortamentoInc = 0.0;
	m_dPortamentoSemitones = 0.0;
	m_uLegatoMode = legato; // legato
	m_dDefaultModIntensity = 1.0;
	m_dDefaultModRange = 1.0;

	m_pOsc1 = NULL;
	m_pOsc2 = NULL;
	m_pOsc3 = NULL;
	m_pOsc4 = NULL;
	m_pFilter1 = NULL;
	m_pFilter2 = NULL;
	m_pGlobalSynthParams = NULL;
	m_pGlobalVoiceParams = NULL;
}

void CVoice::initializeModMatrix(CModulationMatrix* pMatrix)
{
	// --- The Mod matrix "wiring" for DEFAULTS for all synths
	// --- create a row for each source/destination pair
	modMatrixRow* pRow = NULL;

	// VELOCITY -> DCA VEL
	pRow = createModMatrixRow(SOURCE_VELOCITY,
							  DEST_DCA_VELOCITY,
							  &m_dDefaultModIntensity,
							  &m_dDefaultModRange,
							  TRANSFORM_NONE,
							  true); 
	pMatrix->addModMatrixRow(pRow);

	// PITCHBEND -> OSC FO
	pRow = createModMatrixRow(SOURCE_PITCHBEND,
							  DEST_ALL_OSC_FO,
							  &m_dDefaultModIntensity,
							  &m_pGlobalVoiceParams->dOscFoPitchBendModRange,
							  TRANSFORM_NONE,
							  true);
	pMatrix->addModMatrixRow(pRow);

	// MIDI Volume CC07
	pRow = createModMatrixRow(SOURCE_MIDI_VOLUME_CC07,
							  DEST_DCA_AMP,
							  &m_dDefaultModIntensity,
							  &m_pGlobalVoiceParams->dAmpModRange,
							  TRANSFORM_INVERT_MIDI_NORMALIZE,
							  true);
	pMatrix->addModMatrixRow(pRow);

	// MIDI Pan CC10
	pRow = createModMatrixRow(SOURCE_MIDI_PAN_CC10,
							  DEST_DCA_PAN,
							  &m_dDefaultModIntensity,
							  &m_dDefaultModRange,
							  TRANSFORM_MIDI_TO_PAN,
							  true);
	pMatrix->addModMatrixRow(pRow);

	// MIDI Sustain Pedal
	pRow = createModMatrixRow(SOURCE_SUSTAIN_PEDAL,
							  DEST_ALL_EG_SUSTAIN_OVERRIDE,
							  &m_dDefaultModIntensity,
							  &m_dDefaultModRange,
							  TRANSFORM_MIDI_SWITCH,
							  true);
	pMatrix->addModMatrixRow(pRow);

	// MIDI Mod Wheel (0->127) -> LFO1 Depth (0->1)
	// Book Challenge: Mod Wheel -> LFO1 Depth
	// --- LFO1 Source Amplitude
	
	// NOTE NUMBER -> FILTER Fc CONTROL
	pRow = createModMatrixRow(SOURCE_MIDI_NOTE_NUM,
							  DEST_ALL_FILTER_KEYTRACK,
							  &m_pGlobalVoiceParams->dFilterKeyTrackIntensity,
							  &m_dDefaultModRange,
							  TRANSFORM_NOTE_NUMBER_TO_FREQUENCY,
							  false); /* DISABLED BY DEFAULT */
	pMatrix->addModMatrixRow(pRow);

	// VELOCITY -> EG ATTACK SOURCE_VELOCITY
	// 0 velocity -> scalar = 1, normal attack time
	// 128 velocity -> scalar = 0, fastest (0) attack time;
	// We use TRANSFORM_MIDI_NORMALIZE and the inversion takes
	// place in the EG update()
	pRow = createModMatrixRow(SOURCE_VELOCITY,
					  DEST_ALL_EG_ATTACK_SCALING,
					  &m_dDefaultModIntensity,
					  &m_dDefaultModRange,
					  TRANSFORM_MIDI_NORMALIZE,
					  false); /* DISABLED BY DEFAULT */
	pMatrix->addModMatrixRow(pRow);

	// NOTE NUMBER -> EG DECAY SCALING
	// note#0 -> scalar = 1, normal decay time
	// note#128 -> scalar = 0, fastest (0) decay time
	// We use TRANSFORM_MIDI_NORMALIZE and the inversion takes
	// place in the EG update()
	pRow = createModMatrixRow(SOURCE_MIDI_NOTE_NUM,
					  DEST_ALL_EG_DECAY_SCALING,
					  &m_dDefaultModIntensity,
					  &m_dDefaultModRange,
					  TRANSFORM_MIDI_NORMALIZE,
					  false); /* DISABLED BY DEFAULT */
	pMatrix->addModMatrixRow(pRow);

}

CVoice::~CVoice(void)
{
}

void CVoice::setSampleRate(double dSampleRate)
{
	m_dSampleRate = dSampleRate;

	if(m_pOsc1)m_pOsc1->setSampleRate(dSampleRate);
	if(m_pOsc2)m_pOsc2->setSampleRate(dSampleRate);
	if(m_pOsc3)m_pOsc3->setSampleRate(dSampleRate);
	if(m_pOsc4)m_pOsc4->setSampleRate(dSampleRate);

	if(m_pFilter1)m_pFilter1->setSampleRate(dSampleRate);
	if(m_pFilter2)m_pFilter2->setSampleRate(dSampleRate);

	m_EG1.setSampleRate(dSampleRate);
	m_EG2.setSampleRate(dSampleRate);
	m_EG3.setSampleRate(dSampleRate);
	m_EG4.setSampleRate(dSampleRate);

	m_LFO1.setSampleRate(dSampleRate);
	m_LFO2.setSampleRate(dSampleRate);
}

// power on defaults, one time init
void CVoice::prepareForPlay()
{
	// --- clear source array
	m_ModulationMatrix.clearSources();

	// --- power on defaults
	m_ModulationMatrix.m_dSources[SOURCE_MIDI_VOLUME_CC07] = 127;
	m_ModulationMatrix.m_dSources[SOURCE_MIDI_PAN_CC10] = 64;

	// --- this sets up the DEFAULT CONNECTIONS!
	m_LFO1.m_pModulationMatrix = &m_ModulationMatrix;
	m_LFO1.m_uModDestOutput1 = SOURCE_LFO1;
	m_LFO1.m_uModDestOutput2 = SOURCE_LFO1Q;

	m_LFO2.m_pModulationMatrix = &m_ModulationMatrix;
	m_LFO2.m_uModDestOutput1 = SOURCE_LFO2;
	m_LFO2.m_uModDestOutput2 = SOURCE_LFO2Q;

	m_EG1.m_pModulationMatrix = &m_ModulationMatrix;
	m_EG1.m_uModDestEGOutput = SOURCE_EG1;
	m_EG1.m_uModDestBiasedEGOutput = SOURCE_BIASED_EG1;
	m_EG1.m_uModSourceEGAttackScaling = DEST_EG1_ATTACK_SCALING;
	m_EG1.m_uModSourceEGDecayScaling = DEST_EG1_DECAY_SCALING;
	m_EG1.m_uModSourceSustainOverride = DEST_EG1_SUSTAIN_OVERRIDE;

	m_EG2.m_pModulationMatrix = &m_ModulationMatrix;
	m_EG2.m_uModDestEGOutput = SOURCE_EG2;
	m_EG2.m_uModDestBiasedEGOutput = SOURCE_BIASED_EG2;
	m_EG2.m_uModSourceEGAttackScaling = DEST_EG2_ATTACK_SCALING;
	m_EG2.m_uModSourceEGDecayScaling = DEST_EG2_DECAY_SCALING;
	m_EG2.m_uModSourceSustainOverride = DEST_EG2_SUSTAIN_OVERRIDE;

	m_EG3.m_pModulationMatrix = &m_ModulationMatrix;
	m_EG3.m_uModDestEGOutput = SOURCE_EG3;
	m_EG3.m_uModDestBiasedEGOutput = SOURCE_BIASED_EG3;
	m_EG3.m_uModSourceEGAttackScaling = DEST_EG3_ATTACK_SCALING;
	m_EG3.m_uModSourceEGDecayScaling = DEST_EG3_DECAY_SCALING;
	m_EG3.m_uModSourceSustainOverride = DEST_EG3_SUSTAIN_OVERRIDE;

	m_EG4.m_pModulationMatrix = &m_ModulationMatrix;
	m_EG4.m_uModDestEGOutput = SOURCE_EG4;
	m_EG4.m_uModDestBiasedEGOutput = SOURCE_BIASED_EG4;
	m_EG4.m_uModSourceEGAttackScaling = DEST_EG4_ATTACK_SCALING;
	m_EG4.m_uModSourceEGDecayScaling = DEST_EG4_DECAY_SCALING;
	m_EG4.m_uModSourceSustainOverride = DEST_EG4_SUSTAIN_OVERRIDE;

	// --- DCA Setup: 
	m_DCA.m_pModulationMatrix = &m_ModulationMatrix;
	// m_DCA.m_uModSourceEG = DEST_NONE; // this must be connected on derived class, if used normally EG1
	m_DCA.m_uModSourceAmp_dB = DEST_DCA_AMP;
	m_DCA.m_uModSourceVelocity = DEST_DCA_VELOCITY;
	m_DCA.m_uModSourcePan = DEST_DCA_PAN;

	// --- oscillators
	if(m_pOsc1)
	{
		m_pOsc1->m_pModulationMatrix = &m_ModulationMatrix;
		m_pOsc1->m_uModSourceFo = DEST_OSC1_FO;
		m_pOsc1->m_uModSourcePulseWidth = DEST_OSC1_PULSEWIDTH;
		m_pOsc1->m_uModSourceAmp = DEST_OSC1_OUTPUT_AMP; 
	}
	if(m_pOsc2)
	{
		m_pOsc2->m_pModulationMatrix = &m_ModulationMatrix;
		m_pOsc2->m_uModSourceFo = DEST_OSC2_FO;
		m_pOsc2->m_uModSourcePulseWidth = DEST_OSC2_PULSEWIDTH;
		m_pOsc2->m_uModSourceAmp = DEST_OSC2_OUTPUT_AMP; 
	}
	if(m_pOsc3)
	{
		m_pOsc3->m_pModulationMatrix = &m_ModulationMatrix;
		m_pOsc3->m_uModSourceFo = DEST_OSC3_FO;
		m_pOsc3->m_uModSourcePulseWidth = DEST_OSC3_PULSEWIDTH;
		m_pOsc3->m_uModSourceAmp = DEST_OSC3_OUTPUT_AMP; 
	}
	if(m_pOsc4)
	{
		m_pOsc4->m_pModulationMatrix = &m_ModulationMatrix;
		m_pOsc4->m_uModSourceFo = DEST_OSC4_FO;
		m_pOsc4->m_uModSourcePulseWidth = DEST_OSC4_PULSEWIDTH;
		m_pOsc4->m_uModSourceAmp = DEST_OSC4_OUTPUT_AMP; 
	}

	// --- filters
	if(m_pFilter1)
	{
		m_pFilter1->m_pModulationMatrix = &m_ModulationMatrix;
		m_pFilter1->m_uModSourceFc = DEST_FILTER1_FC;
		m_pFilter1->m_uSourceFcControl = DEST_ALL_FILTER_KEYTRACK;
	}

	if(m_pFilter2)
	{
		m_pFilter2->m_pModulationMatrix = &m_ModulationMatrix;
		m_pFilter2->m_uModSourceFc = DEST_FILTER1_FC;
		m_pFilter2->m_uSourceFcControl = DEST_ALL_FILTER_KEYTRACK;
	}
}

void CVoice::update()
{
	// --- voice mode
	m_uVoiceMode = m_pGlobalVoiceParams->uVoiceMode;

	// --- update only if changed
	if(m_dPortamentoTime_mSec != m_pGlobalVoiceParams->dPortamentoTime_mSec)
	{
		m_dPortamentoTime_mSec = m_pGlobalVoiceParams->dPortamentoTime_mSec;
		
		if(m_dPortamentoTime_mSec == 0.0)
			m_dPortamentoInc = 0.0;
		else
			m_dPortamentoInc = 1000.0/m_dPortamentoTime_mSec/m_dSampleRate;
	}

	// --- hard sync ratio
	m_dHSRatio = m_pGlobalVoiceParams->dHSRatio;
}
	
void CVoice::reset()
{
	if(m_pOsc1)m_pOsc1->reset();
	if(m_pOsc2)m_pOsc2->reset();
	if(m_pOsc3)m_pOsc3->reset();
	if(m_pOsc4)m_pOsc4->reset();
	
	if(m_pFilter1)m_pFilter1->reset();
	if(m_pFilter2)m_pFilter2->reset();

	m_EG1.reset();
	m_EG2.reset();
	m_EG3.reset();
	m_EG4.reset();

	m_LFO1.reset();
	m_LFO2.reset();

	m_DCA.reset();
}
