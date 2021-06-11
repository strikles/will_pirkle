//
//  AUSynth.cpp
//
//  Created by Will Pirkle
//  Copyright (c) 2014 Will Pirkle All rights reserved.
/*
 The Software is provided by Will Pirkle on an "AS IS" basis.
 Will Pirkle MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL Will Pirkle BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "AUSynth.h"
 
#define LOG_MIDI 1

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// getMyComponentDirectory()
//
// returns the directory where the .component resides
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
char* getMyComponentDirectory(CFStringRef bundleID)
{
    if (bundleID != NULL)
    {
        CFBundleRef helixBundle = CFBundleGetBundleWithIdentifier( bundleID );
        if(helixBundle != NULL)
        {
            CFURLRef bundleURL = CFBundleCopyBundleURL ( helixBundle );
            if(bundleURL != NULL)
            {
                CFURLRef componentFolderPathURL = CFURLCreateCopyDeletingLastPathComponent(NULL, bundleURL);
                
                CFStringRef myComponentPath = CFURLCopyFileSystemPath(componentFolderPathURL, kCFURLPOSIXPathStyle);
                CFRelease(componentFolderPathURL);
                
                if(myComponentPath != NULL)
                {
                    int nSize = CFStringGetLength(myComponentPath);
                    char* path = new char[nSize+1];
                    memset(path, 0, (nSize+1)*sizeof(char));
                    
                    bool success = CFStringGetCString(myComponentPath, path, nSize+1, kCFStringEncodingASCII);
                    CFRelease(myComponentPath);
                    
                    if(success) return path;
                    else return NULL;
                }
                CFRelease(bundleURL);
            }
        }
        CFRelease(bundleID);
    }
    return NULL;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#pragma mark AUSynth Methods

static const int numPresets = 1;

static AUPreset presetNames[numPresets] = 
{
    // --- {index, Preset Name} 
    //     add more with commas
    // {0, CFSTR("Factory Preset")},		
    // {1, CFSTR("Another Preset")} //<-- no comma	
    {0, CFSTR("Factory Preset")}		
};

AUDIOCOMPONENT_ENTRY(AUMusicDeviceFactory, AUSynth)

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynth::AUSynth
//
// This synth has No inputs, One output
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AUSynth::AUSynth(AudioUnit inComponentInstance)
	: AUInstrumentBase(inComponentInstance, 0, 1)
{
    // --- create inpu, ouyput ports, groups and parts
	CreateElements();
    
    // --- setup default factory preset (as example)
    factoryPreset[OSC1_AMPLITUDE_DB] = 0.0;
    factoryPreset[OSC2_AMPLITUDE_DB] = 0.0;

    factoryPreset[FILTER_FC] = 750.0;
    factoryPreset[FILTER_Q] = 8.9;
    factoryPreset[EG1_TO_FILTER_INTENSITY] = 1.0;
    factoryPreset[FILTER_KEYTRACK_INTENSITY] = 0.5;
    factoryPreset[EG1_ATTACK_MSEC] = 150.0;
    factoryPreset[EG1_DECAY_RELEASE_MSEC] = 3500.0;
    factoryPreset[EG1_SUSTAIN_LEVEL] = 0.8;
    factoryPreset[LFO1_WAVEFORM] = 3;
    factoryPreset[LFO1_RATE] = 0.22;
    factoryPreset[LFO1_AMPLITUDE] = 0.5;
    factoryPreset[LFO1_TO_FILTER_INTENSITY] = 0.25;
    factoryPreset[LFO1_TO_OSC_INTENSITY] = 0;
    factoryPreset[LFO1_TO_DCA_INTENSITY] = 0.0;
    factoryPreset[LFO1_TO_PAN_INTENSITY] = 0.0;
    factoryPreset[OUTPUT_AMPLITUDE_DB] = 12.0;
    factoryPreset[EG1_TO_DCA_INTENSITY] = 1.0;
    factoryPreset[VOICE_MODE] = 3;
    factoryPreset[DETUNE_CENTS] = 1.23;
    factoryPreset[PORTAMENTO_TIME_MSEC] = 10.0;
    factoryPreset[OCTAVE] = 1;
    factoryPreset[VELOCITY_TO_ATTACK] = 0;
    factoryPreset[NOTE_NUM_TO_DECAY] = 1;
    factoryPreset[RESET_TO_ZERO] = 1;
    factoryPreset[FILTER_KEYTRACK] = 0;
    factoryPreset[LEGATO_MODE] = 0;
    factoryPreset[PITCHBEND_RANGE] = 3;

    // --- DigiSynth
    factoryPreset[LOOP_MODE] = 0;
    
    // -- VS specific
    factoryPreset[VECTORJOYSTICK_X] = 0.5;
    factoryPreset[VECTORJOYSTICK_Y] = 0.5;
    factoryPreset[PATH_MODE] = 1;
    factoryPreset[ORBIT_X] = 0.75;
    factoryPreset[ORBIT_Y] = 2.25;
    factoryPreset[ROTOR_RATE] = 0.1;
    factoryPreset[ROTOR_WAVEFORM] = 0;

    // --- define number of params (controls)
	Globals()->UseIndexedParameters(NUMBER_OF_SYNTH_PARAMETERS); //NUM_PARAMS
    
    // --- initialize the controls here!
	// --- these are defined in SynthParamLimits.h
    //
	Globals()->SetParameter(OSC1_AMPLITUDE_DB, DEFAULT_OUTPUT_AMPLITUDE_DB);
	Globals()->SetParameter(OSC2_AMPLITUDE_DB, DEFAULT_OUTPUT_AMPLITUDE_DB);
 	Globals()->SetParameter(EG1_TO_OSC_INTENSITY, DEFAULT_BIPOLAR);
	Globals()->SetParameter(FILTER_FC, DEFAULT_FILTER_FC);
 	Globals()->SetParameter(FILTER_Q, DEFAULT_FILTER_Q);
	Globals()->SetParameter(EG1_TO_FILTER_INTENSITY, DEFAULT_BIPOLAR);
    Globals()->SetParameter(FILTER_KEYTRACK_INTENSITY, DEFAULT_FILTER_KEYTRACK_INTENSITY);
    Globals()->SetParameter(EG1_ATTACK_MSEC, DEFAULT_EG_ATTACK_TIME);
 	Globals()->SetParameter(EG1_DECAY_RELEASE_MSEC, DEFAULT_EG_DECAYRELEASE_TIME);
 	Globals()->SetParameter(EG1_SUSTAIN_LEVEL, DEFAULT_EG_SUSTAIN_LEVEL);
 	Globals()->SetParameter(LFO1_WAVEFORM, DEFAULT_LFO_WAVEFORM);
	Globals()->SetParameter(LFO1_RATE, DEFAULT_LFO_RATE);
 	Globals()->SetParameter(LFO1_AMPLITUDE, DEFAULT_UNIPOLAR);
  	Globals()->SetParameter(LFO1_TO_FILTER_INTENSITY, DEFAULT_BIPOLAR);
 	Globals()->SetParameter(LFO1_TO_OSC_INTENSITY, DEFAULT_BIPOLAR);
	Globals()->SetParameter(LFO1_TO_DCA_INTENSITY, DEFAULT_UNIPOLAR);
 	Globals()->SetParameter(LFO1_TO_PAN_INTENSITY, DEFAULT_UNIPOLAR);
    Globals()->SetParameter(OUTPUT_AMPLITUDE_DB, DEFAULT_OUTPUT_AMPLITUDE_DB);
 	Globals()->SetParameter(EG1_TO_DCA_INTENSITY, MAX_BIPOLAR);
	Globals()->SetParameter(VOICE_MODE, DEFAULT_VOICE_MODE);
	Globals()->SetParameter(DETUNE_CENTS, DEFAULT_DETUNE_CENTS);
 	Globals()->SetParameter(PORTAMENTO_TIME_MSEC, DEFAULT_PORTAMENTO_TIME_MSEC);
    Globals()->SetParameter(OCTAVE, DEFAULT_OCTAVE);
 	Globals()->SetParameter(VELOCITY_TO_ATTACK, DEFAULT_VELOCITY_TO_ATTACK);
 	Globals()->SetParameter(NOTE_NUM_TO_DECAY, DEFAULT_NOTE_TO_DECAY);
  	Globals()->SetParameter(RESET_TO_ZERO, DEFAULT_RESET_TO_ZERO);
 	Globals()->SetParameter(FILTER_KEYTRACK, DEFAULT_FILTER_KEYTRACK);
    Globals()->SetParameter(LEGATO_MODE, DEFAULT_LEGATO_MODE);
 	Globals()->SetParameter(PITCHBEND_RANGE, DEFAULT_PITCHBEND_RANGE);
   
    // -- VS specific
    Globals()->SetParameter(VECTORJOYSTICK_X, DEFAULT_UNIPOLAR_HALF);
 	Globals()->SetParameter(VECTORJOYSTICK_Y, DEFAULT_UNIPOLAR_HALF);
 	Globals()->SetParameter(PATH_MODE, DEFAULT_PATH_MODE);
 	Globals()->SetParameter(ORBIT_X, DEFAULT_UNIPOLAR);
 	Globals()->SetParameter(ORBIT_Y, DEFAULT_UNIPOLAR);
 	Globals()->SetParameter(ROTOR_RATE, DEFAULT_SLOW_LFO_RATE);
 	Globals()->SetParameter(ROTOR_WAVEFORM, DEFAULT_LFO_WAVEFORM);

    // Finish initializations here
	m_dLastNoteFrequency = -1.0;
    
	// receive on all channels
	m_uMidiRxChannel = MIDI_CH_ALL;
   
    // load up voices
    for(int i=0; i<MAX_VOICES; i++)
    {
        // --- create voice
        m_pVoiceArray[i] = new CVectorSynthVoice;
        if(!m_pVoiceArray[i]) return;
        
        // --- global params (MUST BE DONE before setting up mod matrix!)
        m_pVoiceArray[i]->initGlobalParameters(&m_GlobalSynthParams);
    }
    
    // --- use the first voice to setup the MmM
    m_pVoiceArray[0]->initializeModMatrix(&m_GlobalModMatrix);
    
    // --- then set the mod matrix cores on the rest of the voices
    for(int i=0; i<MAX_VOICES; i++)
    {
        // --- all matrices share a common core array of matrix rows
        m_pVoiceArray[i]->setModMatrixCore
        (m_GlobalModMatrix.getModMatrixCore());
    }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynth::AUSynth
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AUSynth::~AUSynth()
{
    // --- delete on master ONLY
    m_GlobalModMatrix.deleteModMatrix();
    
    // --- delete voices
    for(int i=0; i<MAX_VOICES; i++)
    {
        if(m_pVoiceArray[i])
            delete m_pVoiceArray[i];
    }
}

// --- load samples from files or folders
bool AUSynth::loadSamples()
{
    if(!m_pVoiceArray[0]) return false;
    
    char* componentFolder = getMyComponentDirectory(CFSTR("developer.audiounit.yourname.VectorSynth"));
    
    // Add your code here
    // load 4 sets of multis
	// ------- FOR MULTI SAMPLES ------- //
	char* pPath0 = addStrings(componentFolder,"/MultiSamples/Heaver");
	char* pPath1 = addStrings(componentFolder,"/MultiSamples/OldFlatty");
	char* pPath2 = addStrings(componentFolder,"/MultiSamples/Divider");
	char* pPath3 = addStrings(componentFolder,"/MultiSamples/FuzzVibe");
    
	if(!m_pVoiceArray[0]->initOscWithFolderPath(0, pPath0, false, false))
		return false;
	if(!m_pVoiceArray[0]->initOscWithFolderPath(1, pPath1, false, false))
		return false;
	if(!m_pVoiceArray[0]->initOscWithFolderPath(2, pPath2, false, false))
		return false;
	if(!m_pVoiceArray[0]->initOscWithFolderPath(3, pPath3, false, false))
		return false;
    
	// --- then use the other function to copy the sample pointerrs
	//     so they share pointers to same buffers of data
	for(int i=1; i<MAX_VOICES; i++)
	{
		// --- init false, false = NOT single-sample,
		//     NOT pitchless sample
		m_pVoiceArray[i]->initAllOscWithDigiSynthVoice(m_pVoiceArray[0], false, false);
	}
    
	// always delete what comes back from addStrings()
	delete [] pPath0;
	delete [] pPath1;
	delete [] pPath2;
	delete [] pPath3;
	delete [] componentFolder;
    
    return true;
}

void AUSynth::Cleanup()
{

    
}

// --- this will get called during the normal init OR if the sample rate changes; may be
//     redundant with initialize() below
ComponentResult	AUSynth::Reset(AudioUnitScope inScope, AudioUnitElement inElement)
{
    AUBase::Reset(inScope, inElement);
  
    for(int i=0; i<MAX_VOICES; i++)
    {
        if(m_pVoiceArray[i])
        {
            m_pVoiceArray[i]->setSampleRate(GetOutput(0)->GetStreamFormat().mSampleRate);
            m_pVoiceArray[i]->prepareForPlay();
        }
    }
  
    return noErr;
}

// --- this will get called during the normal init OR if the sample rate changes
ComponentResult AUSynth::Initialize()
{	    
    // --- init the base class
	AUInstrumentBase::Initialize();
   
	// clear
	m_dLastNoteFrequency = -1.0;
  
    // --- inits
    for(int i=0; i<MAX_VOICES; i++)
    {
        if(m_pVoiceArray[i])
        {
            m_pVoiceArray[i]->setSampleRate(GetOutput(0)->GetStreamFormat().mSampleRate);
            m_pVoiceArray[i]->prepareForPlay();
        }
    }
    
    // --- load the samples into the oscillators
    loadSamples();

    // --- update synth
    update();
  
	return noErr;
}

// --- helper function to set param info
void AUSynth::setAUParameterInfo(AudioUnitParameterInfo& outParameterInfo, 
                                 CFStringRef paramName, 
                                 CFStringRef paramUnits,
                                 Float32 fMinValue, 
                                 Float32 fMaxValue, 
                                 Float32 fDefaultValue, 
                                 bool bLogControl, 
                                 bool bStringListControl)
{
    // --- set flags
    outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable;
    outParameterInfo.flags += kAudioUnitParameterFlag_IsReadable;
    // outParameterInfo.flags += kAudioUnitParameterFlag_IsHighResolution; // more sig digits printed
    
    // --- set Name and Units
    AUBase::FillInParameterName (outParameterInfo, paramName, false);
    if(bStringListControl)
         outParameterInfo.unit = kAudioUnitParameterUnit_Indexed;
    else
    {
        outParameterInfo.unit = kAudioUnitParameterUnit_CustomUnit; // allows any unit string
        outParameterInfo.unitName = paramUnits;
    }
    
    // --- is log control?
    if(bLogControl)
        outParameterInfo.flags += kAudioUnitParameterFlag_DisplayLogarithmic;
    
    // --- set min, max, default
    outParameterInfo.minValue = fMinValue;
    outParameterInfo.maxValue = fMaxValue;
    outParameterInfo.defaultValue = fDefaultValue;
 
}

// --- host will query repeatedly based on param count we sepecified in constructor
ComponentResult	AUSynth::GetParameterInfo(AudioUnitScope inScope,
                                          AudioUnitParameterID inParameterID,
                                          AudioUnitParameterInfo& outParameterInfo)
{
    // --- we only handle Global params
    if (inScope != kAudioUnitScope_Global) return kAudioUnitErr_InvalidScope;

    // --- decode the paramters, use our built-in helper function setAUParameterInfo()
    switch(inParameterID)
    {
        case OSC1_AMPLITUDE_DB:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Osc1 Output"), CFSTR("dB"), MIN_OUTPUT_AMPLITUDE_DB, MAX_OUTPUT_AMPLITUDE_DB, DEFAULT_OUTPUT_AMPLITUDE_DB);
            return noErr;
            break;
        }
        case OSC2_AMPLITUDE_DB:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Osc2 Output"), CFSTR("dB"), MIN_OUTPUT_AMPLITUDE_DB, MAX_OUTPUT_AMPLITUDE_DB, DEFAULT_OUTPUT_AMPLITUDE_DB);
            return noErr;
            break;
        }
        case EG1_TO_OSC_INTENSITY:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("OSC EG Int"), CFSTR(""), MIN_BIPOLAR, MAX_BIPOLAR, DEFAULT_BIPOLAR);
            return noErr;
            break;
        }
        case FILTER_FC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Filter fc"), CFSTR("Hz"), MIN_FILTER_FC, MAX_FILTER_FC, DEFAULT_FILTER_FC, true); // true = log control
            return noErr;
            break;
        }
        case FILTER_Q:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Filter Q"), CFSTR(""), MIN_FILTER_Q, MAX_FILTER_Q, DEFAULT_FILTER_Q);
            return noErr;
            break;
        }
        case EG1_TO_FILTER_INTENSITY:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Filter EG Int"), CFSTR(""), MIN_BIPOLAR, MAX_BIPOLAR, DEFAULT_BIPOLAR);
            return noErr;
            break;
        }
        case FILTER_KEYTRACK_INTENSITY:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Filter KeyTrack Int"), CFSTR(""), MIN_FILTER_KEYTRACK_INTENSITY, MAX_FILTER_KEYTRACK_INTENSITY, DEFAULT_FILTER_KEYTRACK_INTENSITY);
            return noErr;
            break;
        }
        case EG1_ATTACK_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Attack"), CFSTR("mS"), MIN_EG_ATTACK_TIME, MAX_EG_ATTACK_TIME, DEFAULT_EG_ATTACK_TIME);
            return noErr;
            break;
        }
        case EG1_DECAY_RELEASE_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Decay/Release"), CFSTR("mS"), MIN_EG_DECAYRELEASE_TIME, MAX_EG_DECAYRELEASE_TIME, DEFAULT_EG_DECAYRELEASE_TIME);
            return noErr;
            break;
        }
        case EG1_SUSTAIN_LEVEL:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Sustain"), CFSTR(""), MIN_EG_SUSTAIN_LEVEL, MAX_EG_SUSTAIN_LEVEL, DEFAULT_EG_SUSTAIN_LEVEL);
            return noErr;
            break;
        }
        case LFO1_WAVEFORM:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("LFO Waveform"), CFSTR(""), MIN_LFO_WAVEFORM, MAX_LFO_WAVEFORM, DEFAULT_LFO_WAVEFORM, false, true);
            return noErr;
            break;
        }
        case LFO1_RATE:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("LFO Rate"), CFSTR("Hz"), MIN_LFO_RATE, MAX_LFO_RATE, DEFAULT_LFO_RATE);
            return noErr;
            break;
        }
        case LFO1_AMPLITUDE:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("LFO Amp"), CFSTR(""), MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
            return noErr;
            break;
        }
        case LFO1_TO_FILTER_INTENSITY:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("LFO Cutoff Int"), CFSTR(""), MIN_BIPOLAR, MAX_BIPOLAR, DEFAULT_BIPOLAR);
            return noErr;
            break;
        }
        case LFO1_TO_OSC_INTENSITY:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("LFO Pitch Int"), CFSTR(""), MIN_BIPOLAR, MAX_BIPOLAR, DEFAULT_BIPOLAR);
            return noErr;
            break;
        }
        case LFO1_TO_DCA_INTENSITY:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("LFO Amp Int"), CFSTR(""), MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
            return noErr;
            break;
        }
        case LFO1_TO_PAN_INTENSITY:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("LFO Pan Int"), CFSTR(""), MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
            return noErr;
            break;
        }
        case OUTPUT_AMPLITUDE_DB:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Volume"), CFSTR("dB"), MIN_OUTPUT_AMPLITUDE_DB, MAX_OUTPUT_AMPLITUDE_DB, DEFAULT_OUTPUT_AMPLITUDE_DB);
            return noErr;
            break;
        }
        case EG1_TO_DCA_INTENSITY:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("DCA EG Int"), CFSTR(""), MIN_BIPOLAR, MAX_BIPOLAR, MAX_BIPOLAR); // NOTE: max so we hear notes!
            return noErr;
            break;
        }
        case VOICE_MODE:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Vector Mode"), CFSTR(""), MIN_VOICE_MODE, MAX_VOICE_MODE, DEFAULT_VOICE_MODE, false, true); // not log, is indexed
            return noErr;
            break;
        }
        case DETUNE_CENTS:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Detune Cents"), CFSTR("cents"), MIN_DETUNE_CENTS, MAX_DETUNE_CENTS, DEFAULT_DETUNE_CENTS);
            return noErr;
            break;
        }
        case PORTAMENTO_TIME_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Portamento"), CFSTR("mS"), MIN_PORTAMENTO_TIME_MSEC, MAX_PORTAMENTO_TIME_MSEC, DEFAULT_PORTAMENTO_TIME_MSEC);
            return noErr;
            break;
        }
        case OCTAVE:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Octave"), CFSTR(""), MIN_OCTAVE, MAX_OCTAVE, DEFAULT_OCTAVE);
            return noErr;
            break;
        }
        case VELOCITY_TO_ATTACK:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Vel->Att Scale"), CFSTR(""), MIN_ONOFF_SWITCH, MAX_ONOFF_SWITCH, DEFAULT_VELOCITY_TO_ATTACK, false, true);
            return noErr;
            break;
        }
        case NOTE_NUM_TO_DECAY:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Note->Dcy Scale"), CFSTR(""), MIN_ONOFF_SWITCH, MAX_ONOFF_SWITCH, DEFAULT_NOTE_TO_DECAY, false, true);
            return noErr;
            break;
        }
        case RESET_TO_ZERO:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("ResetToZero"), CFSTR(""), MIN_ONOFF_SWITCH, MAX_ONOFF_SWITCH, DEFAULT_RESET_TO_ZERO, false, true);
            return noErr;
            break;
        }
        case FILTER_KEYTRACK:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Filter KeyTrack"), CFSTR(""), MIN_ONOFF_SWITCH, MAX_ONOFF_SWITCH, DEFAULT_FILTER_KEYTRACK, false, true);
            return noErr;
            break;
        }
        case LEGATO_MODE:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Legato Mode"), CFSTR(""), MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_LEGATO_MODE, false, true);
            return noErr;
            break;
        }
        case PITCHBEND_RANGE:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("PBend Range"), CFSTR(""), MIN_PITCHBEND_RANGE, MAX_PITCHBEND_RANGE, DEFAULT_PITCHBEND_RANGE);
            return noErr;
            break;
        }
        // --- DigiSynth Additions
        case LOOP_MODE:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Loop Mode"), CFSTR(""), MIN_LOOP_MODE, MAX_LOOP_MODE, DEFAULT_LOOP_MODE, false, true); // not log, is indexed
            return noErr;
            break;
        }
       
        case PATH_MODE:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Path Mode"), CFSTR(""), MIN_PATH_MODE, MAX_PATH_MODE, DEFAULT_PATH_MODE, false, true); // not log, is indexed
            return noErr;
            break;
        }
        
        // --- vector synth additions
        case VECTORJOYSTICK_X:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Joystick X"), CFSTR(""), MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR_HALF);
            return noErr;
            break;
        }
        case VECTORJOYSTICK_Y:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Joystick Y"), CFSTR(""), MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR_HALF);
            return noErr;
            break;
        }
            
        case ORBIT_X:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Orbit X"), CFSTR(""), MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
            return noErr;
            break;
        }
        case ORBIT_Y:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Orbit Y"), CFSTR(""), MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
            return noErr;
            break;
        }
        case ROTOR_RATE:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Rotor Rate"), CFSTR("Hz"), MIN_SLOW_LFO_RATE, MAX_SLOW_LFO_RATE, DEFAULT_SLOW_LFO_RATE);
            return noErr;
            break;
        }
        case ROTOR_WAVEFORM:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Rotor Waveform"), CFSTR(""), MIN_LFO_WAVEFORM, MAX_LFO_WAVEFORM, DEFAULT_LFO_WAVEFORM, false, true); // not log, is indexed
            return noErr;
            break;
        }
    }
    
    return kAudioUnitErr_InvalidParameter;
}

// --- helper function for loading string list controls
void AUSynth::setAUParameterStringList(CFStringRef stringList, CFArrayRef* outStrings)
{
    // --- create array from comma-delimited string
    CFArrayRef strings = CFStringCreateArrayBySeparatingStrings(NULL, stringList, CFSTR(","));
    
    // --- set in outStrings with copy function
    *outStrings = CFArrayCreateCopy(NULL, strings);
}

// --- this will get called for each param we specified as bStringListControl (aka "indexed") above
// --- this fills the default I/F Dropown Boxes with the enumerated strings
ComponentResult	AUSynth::GetParameterValueStrings(AudioUnitScope inScope,
                                                  AudioUnitParameterID inParameterID,
                                                  CFArrayRef* outStrings)
{
    
    if(inScope == kAudioUnitScope_Global)
    {
        
        if (outStrings == NULL)
            return noErr;
      
        
        // --- decode the ID value and set the string list; I do it this way to match the "enum UINT" described
        //     in the book; take the strings from the GUI tables and embed here
        switch(inParameterID)
        {
            case VOICE_MODE:
            {
                setAUParameterStringList(CFSTR("manual,program"), outStrings);
                return noErr;
                break;
            }
            case LFO1_WAVEFORM:
            case ROTOR_WAVEFORM:
            {
                setAUParameterStringList(CFSTR("sine,usaw,dsaw,tri,square,expo,rsh,qrsh"), outStrings);
                return noErr;
                break;
            }
            case LEGATO_MODE:
            {
                setAUParameterStringList(CFSTR("mono,legato"), outStrings);
                return noErr;
                break;
            }
                
                // --- all are OFF,ON 2-state switches
            case RESET_TO_ZERO:
            case FILTER_KEYTRACK:
            case VELOCITY_TO_ATTACK:
            case NOTE_NUM_TO_DECAY:
            {
                setAUParameterStringList(CFSTR("OFF,ON"), outStrings);
                return noErr;
                break;
            }
            case LOOP_MODE:
            {
                setAUParameterStringList(CFSTR("sustain,loop,oneShot"), outStrings);
                return noErr;
                break;
            }
            case PATH_MODE:
            {
                setAUParameterStringList(CFSTR("once,loop,sustain,bckfrth"), outStrings);
                return noErr;
                break;
            }
        }
    }
    
    return kAudioUnitErr_InvalidParameter;
}


void AUSynth::update()
{
    // --- update global parameters
	//
	// --- Voice:
 	m_GlobalSynthParams.voiceParams.uVoiceMode = Globals()->GetParameter(VOICE_MODE);
	m_GlobalSynthParams.voiceParams.dPortamentoTime_mSec = Globals()->GetParameter(PORTAMENTO_TIME_MSEC);
   
    // --- VS Specific
	m_GlobalSynthParams.voiceParams.uVectorPathMode = Globals()->GetParameter(PATH_MODE);
	m_GlobalSynthParams.voiceParams.dOrbitXAmp = Globals()->GetParameter(ORBIT_X);
	m_GlobalSynthParams.voiceParams.dOrbitYAmp = Globals()->GetParameter(ORBIT_Y);
	
    // --- VST3/AU ONLY ---
	// --- calculate the vector joystick mix values based on x,y location (x,y are 0->1)
	calculateVectorJoystickValues(Globals()->GetParameter(VECTORJOYSTICK_X),
                                  Globals()->GetParameter(VECTORJOYSTICK_Y),
								  m_GlobalSynthParams.voiceParams.dAmplitude_A,
								  m_GlobalSynthParams.voiceParams.dAmplitude_B,
								  m_GlobalSynthParams.voiceParams.dAmplitude_C,
								  m_GlobalSynthParams.voiceParams.dAmplitude_D,
								  m_GlobalSynthParams.voiceParams.dAmplitude_ACmix,
								  m_GlobalSynthParams.voiceParams.dAmplitude_BDmix);

	// --- ranges
	m_GlobalSynthParams.voiceParams.dOscFoPitchBendModRange = Globals()->GetParameter(PITCHBEND_RANGE);
    
	// --- intensities
	m_GlobalSynthParams.voiceParams.dFilterKeyTrackIntensity = Globals()->GetParameter(FILTER_KEYTRACK_INTENSITY);
	m_GlobalSynthParams.voiceParams.dLFO1Filter1ModIntensity = Globals()->GetParameter(LFO1_TO_FILTER_INTENSITY);
	m_GlobalSynthParams.voiceParams.dLFO1OscModIntensity = Globals()->GetParameter(LFO1_TO_OSC_INTENSITY);
	m_GlobalSynthParams.voiceParams.dLFO1DCAAmpModIntensity = Globals()->GetParameter(LFO1_TO_DCA_INTENSITY);
	m_GlobalSynthParams.voiceParams.dLFO1DCAPanModIntensity = Globals()->GetParameter(LFO1_TO_PAN_INTENSITY);
    
	m_GlobalSynthParams.voiceParams.dEG1OscModIntensity = Globals()->GetParameter(EG1_TO_OSC_INTENSITY);
	m_GlobalSynthParams.voiceParams.dEG1Filter1ModIntensity = Globals()->GetParameter(EG1_TO_FILTER_INTENSITY);
	m_GlobalSynthParams.voiceParams.dEG1DCAAmpModIntensity = Globals()->GetParameter(EG1_TO_DCA_INTENSITY);
    
	// --- Oscillators:
    double dOscAmplitude = Globals()->GetParameter(OSC1_AMPLITUDE_DB) == -96.0 ? 0.0 : pow(10.0, Globals()->GetParameter(OSC1_AMPLITUDE_DB)/20.0);
	m_GlobalSynthParams.osc1Params.dAmplitude = dOscAmplitude;
	
	dOscAmplitude = Globals()->GetParameter(OSC2_AMPLITUDE_DB) == -96.0 ? 0.0 : pow(10.0, Globals()->GetParameter(OSC2_AMPLITUDE_DB)/20.0);
	m_GlobalSynthParams.osc2Params.dAmplitude = dOscAmplitude;
    
	// --- loop mode
	m_GlobalSynthParams.osc1Params.uLoopMode = Globals()->GetParameter(LOOP_MODE);
	m_GlobalSynthParams.osc2Params.uLoopMode = Globals()->GetParameter(LOOP_MODE);
    
	// --- octave
	m_GlobalSynthParams.osc1Params.nOctave = Globals()->GetParameter(OCTAVE);
	m_GlobalSynthParams.osc2Params.nOctave = Globals()->GetParameter(OCTAVE);
    
	// --- detuning for DigiSynth
	m_GlobalSynthParams.osc1Params.nCents = Globals()->GetParameter(DETUNE_CENTS);
	m_GlobalSynthParams.osc2Params.nCents = Globals()->GetParameter(DETUNE_CENTS);
    
	// --- Filter:
	m_GlobalSynthParams.filter1Params.dFcControl = Globals()->GetParameter(FILTER_FC);
	m_GlobalSynthParams.filter1Params.dQControl = Globals()->GetParameter(FILTER_Q);
	m_GlobalSynthParams.filter2Params.dFcControl = Globals()->GetParameter(FILTER_FC);
	m_GlobalSynthParams.filter1Params.dQControl = Globals()->GetParameter(FILTER_Q);
    
	// --- LFO1:
	m_GlobalSynthParams.lfo1Params.uWaveform = Globals()->GetParameter(LFO1_WAVEFORM);
	m_GlobalSynthParams.lfo1Params.dAmplitude = Globals()->GetParameter(LFO1_AMPLITUDE);
	m_GlobalSynthParams.lfo1Params.dOscFo = Globals()->GetParameter(LFO1_RATE);
	
    // --- LFO2:
	m_GlobalSynthParams.lfo2Params.uWaveform = Globals()->GetParameter(ROTOR_WAVEFORM);
	m_GlobalSynthParams.lfo2Params.dOscFo = Globals()->GetParameter(ROTOR_RATE);
    
	// --- EG1:
	m_GlobalSynthParams.eg1Params.dAttackTime_mSec = Globals()->GetParameter(EG1_ATTACK_MSEC);
	m_GlobalSynthParams.eg1Params.dDecayTime_mSec = Globals()->GetParameter(EG1_DECAY_RELEASE_MSEC);
	m_GlobalSynthParams.eg1Params.dSustainLevel = Globals()->GetParameter(EG1_SUSTAIN_LEVEL);
	m_GlobalSynthParams.eg1Params.dReleaseTime_mSec = Globals()->GetParameter(EG1_DECAY_RELEASE_MSEC);
	m_GlobalSynthParams.eg1Params.bResetToZero = (bool)Globals()->GetParameter(RESET_TO_ZERO);
	m_GlobalSynthParams.eg1Params.bLegatoMode = (bool)Globals()->GetParameter(LEGATO_MODE);
    
	// --- DCA:
	m_GlobalSynthParams.dcaParams.dAmplitude_dB = Globals()->GetParameter(OUTPUT_AMPLITUDE_DB);
	
	// --- enable/disable mod matrix stuff
	if((bool)Globals()->GetParameter(VELOCITY_TO_ATTACK) == 1)
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_VELOCITY, DEST_ALL_EG_ATTACK_SCALING, true); // enable
	else
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_VELOCITY, DEST_ALL_EG_ATTACK_SCALING, false);
    
	if((bool)Globals()->GetParameter(NOTE_NUM_TO_DECAY) == 1)
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_MIDI_NOTE_NUM, DEST_ALL_EG_DECAY_SCALING, true); // enable
	else
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_MIDI_NOTE_NUM, DEST_ALL_EG_DECAY_SCALING, false);
    
	if((bool)Globals()->GetParameter(FILTER_KEYTRACK) == 1)
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_MIDI_NOTE_NUM, DEST_ALL_FILTER_KEYTRACK, true); // enable
	else
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_MIDI_NOTE_NUM, DEST_ALL_FILTER_KEYTRACK, false);
}


OSStatus AUSynth::Render(AudioUnitRenderActionFlags& ioActionFlags,
                         const AudioTimeStamp& inTimeStamp,
                         UInt32 inNumberFrames)
{
    // --- broadcast MIDI events
    PerformEvents(inTimeStamp);
    
    // --- do the mass update for this frame
    update();
    AudioBufferList& bufferList = GetOutput(0)->GetBufferList();
    UInt32 numChans = bufferList.mNumberBuffers;
    
    // --- we only support mono/stereo
	if (numChans > 2)
        return kAudioUnitErr_FormatNotSupported;
    
    // --- get pointers for buffer lists
    float* left = (float*)bufferList.mBuffers[0].mData;
    float* right = numChans == 2 ? (float*)bufferList.mBuffers[1].mData : NULL;
    
    double dLeftAccum = 0.0;
	double dRightAccum = 0.0;
    
    float fMix = 0.25; // -12dB HR per note

    // --- the frame processing loop
    for(UInt32 frame=0; frame<inNumberFrames; ++frame)
    {	
        // --- zero out for each trip through loop
        dLeftAccum = 0.0;
        dRightAccum = 0.0;
        double dLeft = 0.0;
        double dRight = 0.0;
        
        // --- synthesize and accumulate each note's sample
        for(int i=0; i<MAX_VOICES; i++)
        {
            // --- render
            if(m_pVoiceArray[i])
                m_pVoiceArray[i]->doVoice(dLeft, dRight);
            
            // --- accumulate and scale
            dLeftAccum += fMix*(float)dLeft;
            dRightAccum += fMix*(float)dRight;
        }

        // --- accumulate in output buffers
        // --- mono
        left[frame] = dLeftAccum;
        
        // --- stereo
        if(right) right[frame] = dRightAccum; 
    }

	return noErr;
}

// --- increment the timestamp for new note events
void AUSynth::incrementVoiceTimestamps()
{
	for(int i=0; i<MAX_VOICES; i++)
	{
		if(m_pVoiceArray[i])
		{
			// if on, inc
			if(m_pVoiceArray[i]->m_bNoteOn)
				m_pVoiceArray[i]->m_uTimeStamp++;
		}
	}
}

// --- get oldest note
CVectorSynthVoice* AUSynth::getOldestVoice()
{
	int nTimeStamp = -1;
	CVectorSynthVoice* pFoundVoice = NULL;
	for(int i=0; i<MAX_VOICES; i++)
	{
		if(m_pVoiceArray[i])
		{
			// if on and older, save
			// highest timestamp is oldest
			if(m_pVoiceArray[i]->m_bNoteOn && (int)m_pVoiceArray[i]->m_uTimeStamp > nTimeStamp)
			{
				pFoundVoice = m_pVoiceArray[i];
				nTimeStamp = (int)m_pVoiceArray[i]->m_uTimeStamp;
			}
		}
	}
	return pFoundVoice;
}

// --- get oldest voice with a MIDI note also
CVectorSynthVoice* AUSynth::getOldestVoiceWithNote(UINT uMIDINote)
{
	int nTimeStamp = -1;
	CVectorSynthVoice* pFoundVoice = NULL;
	for(int i=0; i<MAX_VOICES; i++)
	{
		if(m_pVoiceArray[i])
		{
			// if on and older and same MIDI note, save
			// highest timestamp is oldest
			if(m_pVoiceArray[i]->canNoteOff() &&
               (int)m_pVoiceArray[i]->m_uTimeStamp > nTimeStamp &&
               m_pVoiceArray[i]->m_uMIDINoteNumber == uMIDINote)
			{
				pFoundVoice = m_pVoiceArray[i];
				nTimeStamp = (int)m_pVoiceArray[i]->m_uTimeStamp;
			}
		}
	}
	return pFoundVoice;
}

// --- Note On Event handler
OSStatus AUSynth::StartNote(MusicDeviceInstrumentID inInstrument,
                            MusicDeviceGroupID inGroupID,
                            NoteInstanceID *outNoteInstanceID,
                            UInt32 inOffsetSampleFrame,
                            const MusicDeviceNoteParams &inParams)
{
    UINT uMIDINote = (UINT)inParams.mPitch;
    UINT uVelocity = (UINT)inParams.mVelocity;
    UINT uChannel = (UINT)inGroupID;
    
   	// --- test channel/ignore; inGroupID = MIDI ch 0->15
	if(m_uMidiRxChannel != MIDI_CH_ALL && uChannel != m_uMidiRxChannel)
		return noErr;
    
	bool bStealNote = true;
	for(int i=0; i<MAX_VOICES; i++)
	{
		CVectorSynthVoice* pVoice = m_pVoiceArray[i];
        if(!pVoice) return false; // should not happen
        
		// if we have a free voice, turn on
		if(!pVoice->m_bNoteOn)
		{
			// do this first
			incrementVoiceTimestamps();
            
			// then note on
			pVoice->noteOn(uMIDINote, uVelocity, midiFreqTable[uMIDINote], m_dLastNoteFrequency);
			
#ifdef LOG_MIDI
            printf("-- Note On Ch:%d Note:%d Vel:%d \n", uChannel, uMIDINote, uVelocity);
#endif
            
			// save
			m_dLastNoteFrequency = midiFreqTable[uMIDINote];
			bStealNote = false;
			break;
		}
	}
    
	if(bStealNote)
	{
		// steal oldest note
		CVectorSynthVoice* pVoice = getOldestVoice();
		if(pVoice)
		{
			// do this first
			incrementVoiceTimestamps();
            
			// then note on
			pVoice->noteOn(uMIDINote, uVelocity, midiFreqTable[uMIDINote], m_dLastNoteFrequency);
            
#ifdef LOG_MIDI
            printf("-- Note Stolen! Ch:%d Note:%d Vel:%d \n", uChannel, uMIDINote, uVelocity);
#endif
 		}
        
		// save
		m_dLastNoteFrequency = midiFreqTable[uMIDINote];
	}
    
	return noErr;
}

// --- Note Off handler
OSStatus AUSynth::StopNote(MusicDeviceGroupID inGroupID,
                           NoteInstanceID inNoteInstanceID,
                           UInt32 inOffsetSampleFrame)
{
    UINT uMIDINote = (UINT)inNoteInstanceID;
    UINT uChannel = (UINT)inGroupID;
    
    // --- test channel/ignore; inGroupID = MIDI ch 0->15
	if(m_uMidiRxChannel != MIDI_CH_ALL && uChannel != m_uMidiRxChannel)
		return noErr;
    
	// find and turn off
	// may have multiple notes sustaining; this ensures the oldest
	// note gets the event by starting at top of stack
	for(int i=0; i<MAX_VOICES; i++)
	{
		CVectorSynthVoice* pVoice = getOldestVoiceWithNote(uMIDINote);
		if(pVoice)
		{
            // --- call the function
			pVoice->noteOff(uMIDINote);
            
#ifdef LOG_MIDI
            // --- NOTE: AU does not transmit note of velocity!
            printf("-- Note Off Ch:%d Note:%d \n", uChannel, uMIDINote);
#endif
		}
	}
	
    return noErr;
}

// -- Pitch Bend handler
OSStatus AUSynth::HandlePitchWheel(UInt8 inChannel,
                                   UInt8 inPitch1,
                                   UInt8 inPitch2,
                                   UInt32 inStartFrame)
{
    UINT uChannel = (UINT)inChannel;
    
    // --- test channel/ignore
	if(m_uMidiRxChannel != MIDI_CH_ALL && uChannel != m_uMidiRxChannel)
		return noErr;
    
    // --- convert 14-bit concatentaion of inPitch1, inPitch2
    int nActualPitchBendValue = (int) ((inPitch1 & 0x7F) | ((inPitch2 & 0x7F) << 7));
    float fNormalizedPitchBendValue = (float) (nActualPitchBendValue - 0x2000) / (float) (0x2000);
    
#ifdef LOG_MIDI
    printf("-- Pitch Bend Ch:%d int:%d float:%f \n", uChannel, nActualPitchBendValue, fNormalizedPitchBendValue);
#endif
    
    // --- set in voices
    for(int i=0; i<MAX_VOICES; i++)
    {
        // --- send to matrix
        if(m_pVoiceArray[i])
            m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_PITCHBEND] = fNormalizedPitchBendValue;
    }
    return noErr;
}

/*
 NOTE: if using Logic, Volume and Pan will not be transmitted
 // --- NOTE: Logic hooks the Volume and Pan controls
 // --- But since MIDI CC 7 and 10 (volume and pan respectively) are reserved by the main channel strip controls,
 //     it's best to use MIDI CC 11 (expression) to automate volume effects
 //     http://www.soundonsound.com/sos/apr08/articles/logictech_0408.htm
 //
 There is no way to prevent Logic from using CC#7 messages from being applied to control channel strip faders.
 Suggest you use CC#11 instead, with the following proviso...
 
 On some plugins and instruments, CC#11 does nothing but control volume. On other plugins/instruments, CC#11 is programmed to control volume and timbre (brightness) simultaneously. This is a feature of the programming of the plugin or instrument and not an inherent quality of CC#11 data. In such a case, higher CC#11 values make a sound both louder and brighter, and vice versa. If in fact your instruments respond to CC#11 only with a change in volume then you might as well not try and fight city hall: use CC#11 as your volume control.
 */
// --- CC handler
OSStatus AUSynth::HandleControlChange(UInt8	inChannel,
                                      UInt8 inController,
                                      UInt8 inValue,
                                      UInt32 inStartFrame)
{
	// --- Handle other MIDI messages we are interested in
    UINT uChannel = (UINT)inChannel;
    
    // --- test channel/ignore
	if(m_uMidiRxChannel != MIDI_CH_ALL && uChannel != m_uMidiRxChannel)
		return noErr;
        
	switch(inController)
	{
        case VOLUME_CC07:
        {
            // --- NOTE: LOGIC 9 CAPTURES VOLUME FOR ITSELF ---
#ifdef LOG_MIDI
            printf("-- Volume Ch:%d Value:%d \n", uChannel, inValue);
#endif
            for(int i=0; i<MAX_VOICES; i++)
            {
                if(m_pVoiceArray[i])
                    m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_MIDI_VOLUME_CC07] = (UINT)inValue;
            }
            break;
        }
        case PAN_CC10:
        {
            // --- NOTE: LOGIC 9 CAPTURES PAN FOR ITSELF ---
#ifdef LOG_MIDI
            printf("-- Pan Ch:%d Value:%d \n", uChannel, inValue);
#endif
            
            for(int i=0; i<MAX_VOICES; i++)
            {
                if(m_pVoiceArray[i])
                    m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_MIDI_PAN_CC10] = (UINT)inValue;
            }
            break;
        }
        case EXPRESSION_CC11:
        {
#ifdef LOG_MIDI
            printf("-- Expression Ch:%d Value:%d \n", uChannel, inValue);
#endif
            
            for(int i=0; i<MAX_VOICES; i++)
            {
                if(m_pVoiceArray[i])
                    m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_MIDI_EXPRESSION_CC11] = (UINT)inValue;
            }
            break;
        }
        case MOD_WHEEL:
        {
#ifdef LOG_MIDI
            printf("-- Mod Wheel Ch:%d Value:%d \n", uChannel, inValue);
#endif
            
            for(int i=0; i<MAX_VOICES; i++)
            {
                if(m_pVoiceArray[i])
                    m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_MODWHEEL] = (UINT)inValue;
            }
            break;
        }
        case SUSTAIN_PEDAL:
        {
            bool bSustainPedal = (UINT)inValue > 63 ? true : false;
            
#ifdef LOG_MIDI
            if(bSustainPedal)
                printf("-- Sustain Pedal ON");
            else
                printf("-- Sustain Pedal OFF");
#endif
            
            for(int i=0; i<MAX_VOICES; i++)
            {
                if(m_pVoiceArray[i])
                    m_pVoiceArray[i]->m_ModulationMatrix.m_dSources[SOURCE_SUSTAIN_PEDAL] = (UINT)inValue;
            }
            
            break;
        }
        case ALL_NOTES_OFF:
        {
            // --- NOTE: some clients may trap this
#ifdef LOG_MIDI
            printf("-- All Notes Off!");
#endif
            for(int i=0; i<MAX_VOICES; i++)
            {
                if(m_pVoiceArray[i])
                    m_pVoiceArray[i]->noteOff(m_pVoiceArray[i]->m_uMIDINoteNumber);
            }
            break;
        }
            // --- all other controllers
        default:
        {
#ifdef LOG_MIDI
            if(inController != RESET_ALL_CONTROLLERS) // ignore these
                printf("-- CC Ch:%d Num:%d Value:%d \n", uChannel, inController, inValue);
#endif
            break;
        }
            
    }
    
 	return noErr;
}



OSStatus AUSynth::HandleMidiEvent(UInt8 status,
                                  UInt8 channel,
                                  UInt8 data1,
                                  UInt8 data2,
                                  UInt32 inStartFrame)
{
    UINT uChannel = (UINT)channel;
    
	// test channel/ignore
	if(m_uMidiRxChannel != MIDI_CH_ALL && uChannel != m_uMidiRxChannel)
		return false;
    
    switch(status)
	{
		case POLY_PRESSURE:
		{
#ifdef LOG_MIDI
            printf("-- Poly Pressure Ch:%d Note:%d Value:%d \n", uChannel, (UINT)data1, (UINT)data2);
#endif
            
			break;
		}
            
        case PROGRAM_CHANGE:
        {
#ifdef LOG_MIDI
            printf("-- Program Change Num Ch:%d Value:%d \n", uChannel, (UINT)data1);
#endif
            
            break;
        }
        case CHANNEL_PRESSURE:
        {
#ifdef LOG_MIDI
            printf("-- Channel Pressure Value Ch:%d Value:%d \n", uChannel, (UINT)data1);
#endif
            
            break;
        }
    }
    
    // --- call base class to do its thing
	return AUMIDIBase::HandleMidiEvent(status, channel, data1, data2, inStartFrame);
}

OSStatus AUSynth::GetPropertyInfo(AudioUnitPropertyID inID,
                                  AudioUnitScope inScope,
                                  AudioUnitElement inElement,
                                  UInt32& outDataSize,
                                  Boolean& outWritable)
{
	if (inScope == kAudioUnitScope_Global) 
    {
		if(inID == kAudioUnitProperty_CocoaUI)
        {
            outWritable = false;
            outDataSize = sizeof (AudioUnitCocoaViewInfo);
            
			return noErr;
		}
 	}
    
    // --- call base class to do its thing    
	return MusicDeviceBase::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
}

OSStatus AUSynth::GetProperty(AudioUnitPropertyID inID,
                              AudioUnitScope inScope,
                              AudioUnitElement inElement,
                              void* outData)
{
	if (inScope == kAudioUnitScope_Global) 
    {
        if(inID == kAudioUnitProperty_CocoaUI)
        {
            // Look for a resource in the main bundle by name and type. 
            CFBundleRef bundle = CFBundleGetBundleWithIdentifier( CFSTR("developer.audiounit.yourname.VectorSynth") );
            
            if (bundle == NULL)
                return fnfErr;
            
            CFURLRef bundleURL = CFBundleCopyResourceURL( bundle,
                                                         CFSTR("CocoaSynthView"),	// this is the name of the cocoa bundle as specified in the CocoaViewFactory.plist
                                                         CFSTR("bundle"),			// this is the extension of the cocoa bundle
                                                         NULL);
            
            if (bundleURL == NULL)
                return fnfErr;
            
            CFStringRef className = CFSTR("VectorSynthViewFactory");	// name of the main class that implements the AUCocoaUIBase protocol
            
            AudioUnitCocoaViewInfo cocoaInfo;
            cocoaInfo.mCocoaAUViewBundleLocation = bundleURL;
            cocoaInfo.mCocoaAUViewClass[0] = className;

            *((AudioUnitCocoaViewInfo *)outData) = cocoaInfo;
            
            return noErr;
        }
	}
    
    // --- call base class to do its thing
	return AUBase::GetProperty (inID, inScope, inElement, outData);
}

OSStatus AUSynth::SetProperty(AudioUnitPropertyID inID,
                              AudioUnitScope inScope,
                              AudioUnitElement inElement,
                              const void* inData,
                              UInt32 inDataSize)
{
	if (inScope == kAudioUnitScope_Global) 
    {
        // -- example see AudioUnitProperties.h for a list of props
		/*
         if (inID == kSomeProperty)
         {
         
         }*/
	}
	return kAudioUnitErr_InvalidProperty;
}

ComponentResult AUSynth::RestoreState(CFPropertyListRef plist)
{
	return AUInstrumentBase::RestoreState(plist);
}

OSStatus AUSynth::GetPresets(CFArrayRef *outData) const
{
    // --- this is used to determine if presets are supported 
    //     which in this unit they are so we implement this method!
	if (outData == NULL) return noErr;
    
	// --- make the array
	CFMutableArrayRef theArray = CFArrayCreateMutable (NULL, numPresets, NULL);
    
    // --- copy our preset names
	for (int i = 0; i < numPresets; ++i) 
    {
		CFArrayAppendValue (theArray, &presetNames[i]);
    }
    
    // --- set
    *outData = (CFArrayRef)theArray;
  
	return noErr;
}

OSStatus AUSynth::NewFactoryPresetSet(const AUPreset& inNewFactoryPreset)
{
    // --- parse the preset
	SInt32 chosenPreset = inNewFactoryPreset.presetNumber;
    
    if (chosenPreset < 0 || chosenPreset >= numPresets)
		return kAudioUnitErr_InvalidPropertyValue;

    // --- only have one preset, could have array of them as challenge
    for(int i=0; i<NUMBER_OF_SYNTH_PARAMETERS; i++)
    {
        Globals()->SetParameter(i, factoryPreset[i]);
    }
 
    return noErr;
}






