#ifndef __synthparamlimits__
#define __synthparamlimits__


//-----------------------------------------------------------------------------
// VectorSynth Parameters
//-----------------------------------------------------------------------------
enum {
    // --- note these are arranged by columns/rows in the final GUI
    OSC1_AMPLITUDE_DB, // NOTE: for VectorSynth this is OSC A&C
    OSC2_AMPLITUDE_DB, // NOTE: for VectorSynth this is OSC B&D
	EG1_TO_OSC_INTENSITY,
    
   	FILTER_FC,
 	FILTER_Q,
	EG1_TO_FILTER_INTENSITY,
    FILTER_KEYTRACK_INTENSITY,

    EG1_ATTACK_MSEC,
	EG1_DECAY_RELEASE_MSEC,
	EG1_SUSTAIN_LEVEL,
    
	LFO1_WAVEFORM,
	LFO1_RATE,
	LFO1_AMPLITUDE,
    
    LFO1_TO_FILTER_INTENSITY,
	LFO1_TO_OSC_INTENSITY,
	LFO1_TO_DCA_INTENSITY,
	LFO1_TO_PAN_INTENSITY,
    
    OUTPUT_AMPLITUDE_DB,
	EG1_TO_DCA_INTENSITY,
    
	VOICE_MODE,
	DETUNE_CENTS,
	PORTAMENTO_TIME_MSEC,
    OCTAVE,
	
    VELOCITY_TO_ATTACK,
	NOTE_NUM_TO_DECAY,
	RESET_TO_ZERO,
	FILTER_KEYTRACK,
	LEGATO_MODE,
	PITCHBEND_RANGE,
    
    // --- Append more params here: you must append if using VSTGUI,
    //     or you will need to re-do some/all of your Tags
    LOOP_MODE,
    
    // --- VectorSynth additions
	VECTORJOYSTICK_X, // unipolar
	VECTORJOYSTICK_Y, // unipolar
	PATH_MODE,        // 0,1
	ORBIT_X,// unipolar
	ORBIT_Y,// uni0.25polar
	ROTOR_RATE,// LFO rate
	ROTOR_WAVEFORM, // LFO waveform

	NUMBER_OF_SYNTH_PARAMETERS // always last
};

// --- define the HI, LO and DEFAULT values for our controls
#define MIN_PITCHED_OSC_WAVEFORM 0
#define MAX_PITCHED_OSC_WAVEFORM 7
#define DEFAULT_PITCHED_OSC_WAVEFORM 0

#define MIN_VOICE_MODE 0
//#define MAX_VOICE_MODE 3  // NOTE: 3 for DigiSynth!!!!!!
#define MAX_VOICE_MODE 1  // NOTE: 1 for VectorSynth!!!!!!
#define DEFAULT_VOICE_MODE 0

#define MIN_LOOP_MODE 0
#define MAX_LOOP_MODE 2
#define DEFAULT_LOOP_MODE 0

// VS
#define MIN_PATH_MODE 0
#define MAX_PATH_MODE 3
#define DEFAULT_PATH_MODE 0

#define MIN_OCTAVE -4
#define MAX_OCTAVE +4
#define DEFAULT_OCTAVE 0

#define MIN_HARD_SYNC_RATIO 1.0
#define MAX_HARD_SYNC_RATIO 4.0
#define DEFAULT_HARD_SYNC_RATIO 1.0

#define MIN_NOISE_OSC_AMP_DB -96.0
#define MAX_NOISE_OSC_AMP_DB 0.0
#define DEFAULT_NOISE_OSC_AMP_DB -96.0

#define MIN_PULSE_WIDTH_PCT 1.0
#define MAX_PULSE_WIDTH_PCT 99.0
#define DEFAULT_PULSE_WIDTH_PCT 50.0

#define MIN_DETUNE_CENTS -50.0
#define MAX_DETUNE_CENTS 50.0
#define DEFAULT_DETUNE_CENTS 0.0

#define MIN_PORTAMENTO_TIME_MSEC 0.0
#define MAX_PORTAMENTO_TIME_MSEC 5000.0
#define DEFAULT_PORTAMENTO_TIME_MSEC 0.0

// for all EG Int EXCEPT LFO->PAN Int
#define MIN_BIPOLAR -1.0
#define MAX_BIPOLAR 1.0
#define DEFAULT_BIPOLAR 0.0

#define MIN_UNIPOLAR 0.0
#define MAX_UNIPOLAR 1.0
#define DEFAULT_UNIPOLAR 0.0
#define DEFAULT_UNIPOLAR_HALF 0.5

#define MIN_FILTER_FC 80.0
#define MAX_FILTER_FC 18000.0
#define DEFAULT_FILTER_FC 10000.0
#define FILTER_RAW_MAP 0.5
#define FILTER_COOKED_MAP 1800.0

#define MIN_FILTER_Q 1.0
#define MAX_FILTER_Q 10.0
#define DEFAULT_FILTER_Q 1.0

// sync, one shot, free
#define MIN_LFO_MODE 0
#define MAX_LFO_MODE 2
#define DEFAULT_LFO_MODE 0

#define MIN_LFO_RATE 0.02
#define MAX_LFO_RATE 20.0
#define DEFAULT_LFO_RATE 0.5

#define MIN_SLOW_LFO_RATE 0.02
#define MAX_SLOW_LFO_RATE 2.0
#define DEFAULT_SLOW_LFO_RATE 0.1

#define MIN_LFO_WAVEFORM 0
#define MAX_LFO_WAVEFORM 7
#define DEFAULT_LFO_WAVEFORM 0	

#define MIN_EG_ATTACK_TIME 0.0
#define MAX_EG_ATTACK_TIME 5000.0
#define DEFAULT_EG_ATTACK_TIME 100.0	

#define MIN_EG_DECAY_TIME 0.0
#define MAX_EG_DECAY_TIME 5000.0
#define DEFAULT_EG_DECAY_TIME 100.0	

#define MIN_EG_DECAYRELEASE_TIME 0.0
#define MAX_EG_DECAYRELEASE_TIME 10000.0
#define DEFAULT_EG_DECAYRELEASE_TIME 1000.0	

#define MIN_EG_SUSTAIN_LEVEL 0.0
#define MAX_EG_SUSTAIN_LEVEL 1.0
#define DEFAULT_EG_SUSTAIN_LEVEL 0.707	

#define MIN_EG_RELEASE_TIME 0.0
#define MAX_EG_RELEASE_TIME 10000.0
#define DEFAULT_EG_RELEASE_TIME 2000.0	

#define MIN_PAN -1
#define MAX_PAN 1
#define DEFAULT_PAN 0

#define MIN_OUTPUT_AMPLITUDE_DB -96.0
#define MAX_OUTPUT_AMPLITUDE_DB 24.0
#define DEFAULT_OUTPUT_AMPLITUDE_DB 0.0	

// NOTE these are +/- units, so +/-1 to +/-12 semis
#define MIN_PITCHBEND_RANGE 1
#define MAX_PITCHBEND_RANGE 12
#define DEFAULT_PITCHBEND_RANGE 1	

// on/off switches
#define MIN_ONOFF_SWITCH 0
#define MAX_ONOFF_SWITCH 1
#define DEFAULT_ONOFF_SWITCH 0	

#define MIN_FILTER_KEYTRACK_INTENSITY 0.5
#define MAX_FILTER_KEYTRACK_INTENSITY 2.0
#define DEFAULT_FILTER_KEYTRACK_INTENSITY 0.5

#define MIN_MIDI_VALUE 0
#define MAX_MIDI_VALUE 127
#define DEFAULT_MIDI_VALUE 64

#define DEFAULT_LEGATO_MODE 0
#define DEFAULT_RESET_TO_ZERO 0
#define DEFAULT_FILTER_KEYTRACK 0
#define DEFAULT_VELOCITY_TO_ATTACK 0
#define DEFAULT_NOTE_TO_DECAY 0
#define DEFAULT_MIDI_PITCHBEND 0
#define DEFAULT_MIDI_MODWHEEL 0
#define DEFAULT_MIDI_VOLUME 127
#define DEFAULT_MIDI_PAN 64
#define DEFAULT_MIDI_EXPRESSION 0


#endif	// __synthparamlimits__

