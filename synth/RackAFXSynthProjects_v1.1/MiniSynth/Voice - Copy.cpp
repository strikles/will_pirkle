#include "Voice.h"

CVoice::CVoice(void)
{
	m_bNoteOn = false;
	m_bNotePending = false;
	m_dSampleRate = 44100;

	m_dOscPitch = 440.0;
	m_dOscPitchPending = 440.0;
	m_dOscPitchPortamentoStart = 440.0;
	
	m_dModuloPortamento = 0.0;
	m_dPortamentoInc = 0.0;
	m_dPortamentoSemitones = 0.0;

	m_uLegatoMode = mono;

	m_pOsc1 = NULL;
	m_pOsc2 = NULL;
	m_pOsc3 = NULL;
	m_pOsc4 = NULL;

	m_dOsc1OutputLevel = 1.0;
	m_dOsc2OutputLevel = 1.0;
	m_dOsc3OutputLevel = 1.0;
	m_dOsc4OutputLevel = 1.0;

	m_pFilter1 = NULL;
	m_pFilter2 = NULL;

	m_dDefaultModIntensity = 1.0;
	m_dDefaultModRange = 1.0;

	m_dFilterKeyTrackIntensity = 1.0; // off

	m_dFilterModRange = FILTER_FC_MOD_RANGE;
	m_dOscFoModRange = OSC_FO_MOD_RANGE;
	m_dOscFoPitchBendModRange = OSC_PITCHBEND_MOD_RANGE;
	m_dAmpModRange = AMP_MOD_RANGE;
	m_dOscHardSyncModRange = OSC_HARD_SYNC_RATIO_RANGE;

	// for any voice-specific modulation
	m_uSourceFoRatio = DEST_NONE;

	// --- setup the mod matrix for defaults
	//     LFOs, EGs only
	//     Filters and Oscillators done in 
	//     derived class constructor
	m_ModulationMatrix.clearSources();

	m_LFO1.m_pModulationMatrix = &m_ModulationMatrix;
	m_LFO1.m_uModDestOutput1 = SOURCE_LFO1;
	m_LFO1.m_uModDestOutput2 = SOURCE_LFO1Q;
	
	m_LFO2.m_pModulationMatrix = &m_ModulationMatrix;
	m_LFO2.m_uModDestOutput1 = SOURCE_LFO2;
	m_LFO2.m_uModDestOutput2 = SOURCE_LFO2Q;

	m_EG1.m_pModulationMatrix = &m_ModulationMatrix;
	m_EG1.m_uModDestEGOutput = SOURCE_EG1;
	m_EG1.m_uModDestBiasedEGOutput = SOURCE_BIASED_EG1;

	m_EG2.m_pModulationMatrix = &m_ModulationMatrix;
	m_EG2.m_uModDestEGOutput = SOURCE_EG2;
	m_EG2.m_uModDestBiasedEGOutput = SOURCE_BIASED_EG2;

	m_DCA.m_pModulationMatrix = &m_ModulationMatrix;

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

void CVoice::prepareForPlay()
{
	// power on defaults
	setMIDIVolumeCC07(127);
	setMIDIPanCC10(64);
}

void CVoice::update()
{
	m_LFO1.updateOscillator();
	m_LFO2.updateOscillator();

	m_DCA.updateDCA();
}
	
void CVoice::reset()
{
	m_EG1.reset();
	m_EG2.reset();
	m_EG3.reset();
	m_EG4.reset();

	m_LFO1.reset();
	m_LFO2.reset();

	m_DCA.reset();
}
