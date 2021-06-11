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

// COMPONENT_ENTRY(AUSynth)
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
    factoryPreset[VOICE_MODE] = 3;
    factoryPreset[OP1_RATIO] = 1.00;
    factoryPreset[EG1_ATTACK_MSEC] = 100.0;
    factoryPreset[EG1_DECAY_MSEC] = 1000.0;
    factoryPreset[EG1_SUSTAIN_LEVEL] = 0.7;
    factoryPreset[EG1_RELEASE_MSEC] = 2000.0;
    factoryPreset[OP1_OUTPUT_LEVEL] = 75;
    
    factoryPreset[OP2_RATIO] = 1.23;
    factoryPreset[EG2_ATTACK_MSEC] = 750.0;
    factoryPreset[EG2_DECAY_MSEC] = 3000.0;
    factoryPreset[EG2_SUSTAIN_LEVEL] = 0.6;
    factoryPreset[EG2_RELEASE_MSEC] = 4500;
    factoryPreset[OP2_OUTPUT_LEVEL] = 54;
    
    factoryPreset[OP3_RATIO] = 1.23;
    factoryPreset[EG3_ATTACK_MSEC] = 750.0;
    factoryPreset[EG3_DECAY_MSEC] = 250;
    factoryPreset[EG3_SUSTAIN_LEVEL] = 0.9;
    factoryPreset[EG3_RELEASE_MSEC] = 250;
    factoryPreset[OP3_OUTPUT_LEVEL] = 99;

    factoryPreset[OP4_FEEDBACK] = 1.23;
    factoryPreset[OP4_RATIO] = 1.02;
    factoryPreset[EG4_ATTACK_MSEC] = 750.0;
    factoryPreset[EG4_DECAY_MSEC] = 900;
    factoryPreset[EG4_SUSTAIN_LEVEL] = 0.3;
    factoryPreset[EG4_RELEASE_MSEC] = 4300;
    factoryPreset[OP4_OUTPUT_LEVEL] = 75;
    
    factoryPreset[LFO1_WAVEFORM] = 3;
    factoryPreset[LFO1_AMPLITUDE] = 0.5;
    factoryPreset[LFO1_RATE] = 0.5;
    
    factoryPreset[LFO1_DESTINATION_OP1] = 0;
    factoryPreset[LFO1_DESTINATION_OP2] = 0;
    factoryPreset[LFO1_DESTINATION_OP3] = 0;
    factoryPreset[LFO1_DESTINATION_OP4] = 0;
    
    factoryPreset[PORTAMENTO_TIME_MSEC] = 0;
    factoryPreset[OUTPUT_AMPLITUDE_DB] = 12.0;
    factoryPreset[LEGATO_MODE] = 0;
    factoryPreset[RESET_TO_ZERO] = 1;
    factoryPreset[PITCHBEND_RANGE] = 1;
    factoryPreset[VELOCITY_TO_ATTACK] = 0;
    factoryPreset[NOTE_NUM_TO_DECAY] = 1;
    
    // --- define number of params (controls)
	Globals()->UseIndexedParameters(NUMBER_OF_SYNTH_PARAMETERS); //NUM_PARAMS
    
    // --- initialize the controls here!
	// --- these are defined in SynthParamLimits.h
    //
	Globals()->SetParameter(VOICE_MODE, DEFAULT_VOICE_MODE);
    
	Globals()->SetParameter(OP1_RATIO, DEFAULT_OP_RATIO);
 	Globals()->SetParameter(EG1_ATTACK_MSEC, DEFAULT_EG_ATTACK_TIME);
 	Globals()->SetParameter(EG1_DECAY_MSEC, DEFAULT_EG_DECAY_TIME);
 	Globals()->SetParameter(EG1_SUSTAIN_LEVEL, DEFAULT_EG_SUSTAIN_LEVEL);
 	Globals()->SetParameter(EG1_RELEASE_MSEC, DEFAULT_EG_RELEASE_TIME);
 	Globals()->SetParameter(OP1_OUTPUT_LEVEL, DEFAULT_DX_OUTPUT_LEVEL);
    
    Globals()->SetParameter(OP2_RATIO, DEFAULT_OP_RATIO);
 	Globals()->SetParameter(EG2_ATTACK_MSEC, DEFAULT_EG_ATTACK_TIME);
 	Globals()->SetParameter(EG2_DECAY_MSEC, DEFAULT_EG_DECAY_TIME);
 	Globals()->SetParameter(EG2_SUSTAIN_LEVEL, DEFAULT_EG_SUSTAIN_LEVEL);
 	Globals()->SetParameter(EG2_RELEASE_MSEC, DEFAULT_EG_RELEASE_TIME);
 	Globals()->SetParameter(OP2_OUTPUT_LEVEL, DEFAULT_DX_OUTPUT_LEVEL);
    
    Globals()->SetParameter(OP3_RATIO, DEFAULT_OP_RATIO);
 	Globals()->SetParameter(EG3_ATTACK_MSEC, DEFAULT_EG_ATTACK_TIME);
 	Globals()->SetParameter(EG3_DECAY_MSEC, DEFAULT_EG_DECAY_TIME);
 	Globals()->SetParameter(EG3_SUSTAIN_LEVEL, DEFAULT_EG_SUSTAIN_LEVEL);
 	Globals()->SetParameter(EG3_RELEASE_MSEC, DEFAULT_EG_RELEASE_TIME);
 	Globals()->SetParameter(OP3_OUTPUT_LEVEL, DEFAULT_DX_OUTPUT_LEVEL);
    
    Globals()->SetParameter(OP4_FEEDBACK, DEFAULT_OP_FEEDBACK);
    Globals()->SetParameter(OP4_RATIO, DEFAULT_OP_RATIO);
 	Globals()->SetParameter(EG4_ATTACK_MSEC, DEFAULT_EG_ATTACK_TIME);
 	Globals()->SetParameter(EG4_DECAY_MSEC, DEFAULT_EG_DECAY_TIME);
 	Globals()->SetParameter(EG4_SUSTAIN_LEVEL, DEFAULT_EG_SUSTAIN_LEVEL);
 	Globals()->SetParameter(EG4_RELEASE_MSEC, DEFAULT_EG_RELEASE_TIME);
 	Globals()->SetParameter(OP4_OUTPUT_LEVEL, DEFAULT_DX_OUTPUT_LEVEL);
  
 	Globals()->SetParameter(LFO1_WAVEFORM, DEFAULT_LFO_WAVEFORM);
   	Globals()->SetParameter(LFO1_AMPLITUDE, DEFAULT_UNIPOLAR);
  	Globals()->SetParameter(LFO1_RATE, DEFAULT_LFO_RATE);
    
  	Globals()->SetParameter(LFO1_DESTINATION_OP1, DEFAULT_DX_LFO_DESTINATION);
  	Globals()->SetParameter(LFO1_DESTINATION_OP2, DEFAULT_DX_LFO_DESTINATION);
  	Globals()->SetParameter(LFO1_DESTINATION_OP3, DEFAULT_DX_LFO_DESTINATION);
  	Globals()->SetParameter(LFO1_DESTINATION_OP4, DEFAULT_DX_LFO_DESTINATION);
    
    Globals()->SetParameter(PORTAMENTO_TIME_MSEC, DEFAULT_PORTAMENTO_TIME_MSEC);
    Globals()->SetParameter(OUTPUT_AMPLITUDE_DB, DEFAULT_OUTPUT_AMPLITUDE_DB);
  	Globals()->SetParameter(LEGATO_MODE, DEFAULT_LEGATO_MODE);
 	Globals()->SetParameter(RESET_TO_ZERO, DEFAULT_RESET_TO_ZERO);
 	Globals()->SetParameter(PITCHBEND_RANGE, DEFAULT_PITCHBEND_RANGE);
 	Globals()->SetParameter(VELOCITY_TO_ATTACK, DEFAULT_VELOCITY_TO_ATTACK);
 	Globals()->SetParameter(NOTE_NUM_TO_DECAY, DEFAULT_NOTE_TO_DECAY);

    // Finish initializations here
	m_dLastNoteFrequency = -1.0;
    
	// receive on all channels
	m_uMidiRxChannel = MIDI_CH_ALL;
    
    // load up voices
    for(int i=0; i<MAX_VOICES; i++)
    {
        // --- create voice
        m_pVoiceArray[i] = new CDXSynthVoice;
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
    
    for(int i=0; i<MAX_VOICES; i++)
    {
        if(m_pVoiceArray[i])
        {
            m_pVoiceArray[i]->setSampleRate(GetOutput(0)->GetStreamFormat().mSampleRate);
            m_pVoiceArray[i]->prepareForPlay();
        }
    }

    // --- update the synth
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
        case VOICE_MODE:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Voice Mode"), CFSTR(""), MIN_VOICE_MODE, MAX_VOICE_MODE, DEFAULT_VOICE_MODE, false, true); // not log, is indexed
            return noErr;
            break;
        }
        case OP1_RATIO:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op1 Ratio"), CFSTR("cents"), MIN_OP_RATIO, MAX_OP_RATIO, DEFAULT_OP_RATIO);
            return noErr;
            break;
        }
        case EG1_ATTACK_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op1 Attack"), CFSTR("mS"), MIN_EG_ATTACK_TIME, MAX_EG_ATTACK_TIME, DEFAULT_EG_ATTACK_TIME);
            return noErr;
            break;
        }
        case EG1_DECAY_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op1 Decay"), CFSTR("mS"), MIN_EG_DECAY_TIME, MAX_EG_DECAY_TIME, DEFAULT_EG_DECAY_TIME);
            return noErr;
            break;
        }
        case EG1_SUSTAIN_LEVEL:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op1 Sustain"), CFSTR(""), MIN_EG_SUSTAIN_LEVEL, MAX_EG_SUSTAIN_LEVEL, DEFAULT_EG_SUSTAIN_LEVEL);
            return noErr;
            break;
        }
        case EG1_RELEASE_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op1 Release"), CFSTR("mS"), MIN_EG_RELEASE_TIME, MAX_EG_RELEASE_TIME, DEFAULT_EG_RELEASE_TIME);
            return noErr;
            break;
        }
        case OP1_OUTPUT_LEVEL:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op1 Output"), CFSTR(""), MIN_DX_OUTPUT_LEVEL, MAX_DX_OUTPUT_LEVEL, DEFAULT_DX_OUTPUT_LEVEL);
            return noErr;
            break;
        }
        case OP2_RATIO:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op2 Ratio"), CFSTR("cents"), MIN_OP_RATIO, MAX_OP_RATIO, DEFAULT_OP_RATIO);
            return noErr;
            break;
        }
        case EG2_ATTACK_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op2 Attack"), CFSTR("mS"), MIN_EG_ATTACK_TIME, MAX_EG_ATTACK_TIME, DEFAULT_EG_ATTACK_TIME);
            return noErr;
            break;
        }
        case EG2_DECAY_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op2 Decay"), CFSTR("mS"), MIN_EG_DECAY_TIME, MAX_EG_DECAY_TIME, DEFAULT_EG_DECAY_TIME);
            return noErr;
            break;
        }
        case EG2_SUSTAIN_LEVEL:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op2 Sustain"), CFSTR(""), MIN_EG_SUSTAIN_LEVEL, MAX_EG_SUSTAIN_LEVEL, DEFAULT_EG_SUSTAIN_LEVEL);
            return noErr;
            break;
        }
        case EG2_RELEASE_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op2 Release"), CFSTR("mS"), MIN_EG_RELEASE_TIME, MAX_EG_RELEASE_TIME, DEFAULT_EG_RELEASE_TIME);
            return noErr;
            break;
        }
        case OP2_OUTPUT_LEVEL:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op2 Output"), CFSTR(""), MIN_DX_OUTPUT_LEVEL, MAX_DX_OUTPUT_LEVEL, DEFAULT_DX_OUTPUT_LEVEL);
            return noErr;
            break;
        }
        case OP3_RATIO:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op3 Ratio"), CFSTR("cents"), MIN_OP_RATIO, MAX_OP_RATIO, DEFAULT_OP_RATIO);
            return noErr;
            break;
        }
        case EG3_ATTACK_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op3 Attack"), CFSTR("mS"), MIN_EG_ATTACK_TIME, MAX_EG_ATTACK_TIME, DEFAULT_EG_ATTACK_TIME);
            return noErr;
            break;
        }
        case EG3_DECAY_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op3 Decay"), CFSTR("mS"), MIN_EG_DECAY_TIME, MAX_EG_DECAY_TIME, DEFAULT_EG_DECAY_TIME);
            return noErr;
            break;
        }
        case EG3_SUSTAIN_LEVEL:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op3 Sustain"), CFSTR(""), MIN_EG_SUSTAIN_LEVEL, MAX_EG_SUSTAIN_LEVEL, DEFAULT_EG_SUSTAIN_LEVEL);
            return noErr;
            break;
        }
        case EG3_RELEASE_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op3 Release"), CFSTR("mS"), MIN_EG_RELEASE_TIME, MAX_EG_RELEASE_TIME, DEFAULT_EG_RELEASE_TIME);
            return noErr;
            break;
        }
        case OP3_OUTPUT_LEVEL:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op3 Output"), CFSTR(""), MIN_DX_OUTPUT_LEVEL, MAX_DX_OUTPUT_LEVEL, DEFAULT_DX_OUTPUT_LEVEL);
            return noErr;
            break;
        }
            
        case OP4_RATIO:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op4 Ratio"), CFSTR("cents"), MIN_OP_RATIO, MAX_OP_RATIO, DEFAULT_OP_RATIO);
            return noErr;
            break;
        }
        case EG4_ATTACK_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op4 Attack"), CFSTR("mS"), MIN_EG_ATTACK_TIME, MAX_EG_ATTACK_TIME, DEFAULT_EG_ATTACK_TIME);
            return noErr;
            break;
        }
        case EG4_DECAY_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op4 Decay"), CFSTR("mS"), MIN_EG_DECAY_TIME, MAX_EG_DECAY_TIME, DEFAULT_EG_DECAY_TIME);
            return noErr;
            break;
        }
        case EG4_SUSTAIN_LEVEL:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op4 Sustain"), CFSTR(""), MIN_EG_SUSTAIN_LEVEL, MAX_EG_SUSTAIN_LEVEL, DEFAULT_EG_SUSTAIN_LEVEL);
            return noErr;
            break;
        }
        case EG4_RELEASE_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op4 Release"), CFSTR("mS"), MIN_EG_RELEASE_TIME, MAX_EG_RELEASE_TIME, DEFAULT_EG_RELEASE_TIME);
            return noErr;
            break;
        }
        case OP4_OUTPUT_LEVEL:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op4 Output"), CFSTR(""), MIN_DX_OUTPUT_LEVEL, MAX_DX_OUTPUT_LEVEL, DEFAULT_DX_OUTPUT_LEVEL);
            return noErr;
            break;
        }
        case OP4_FEEDBACK:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op4 Feedback"), CFSTR(""), MIN_OP_FEEDBACK, MAX_OP_FEEDBACK, DEFAULT_OP_FEEDBACK);
            return noErr;
            break;
        }
        case LFO1_WAVEFORM:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("LFO Waveform"), CFSTR(""), MIN_LFO_WAVEFORM, MAX_LFO_WAVEFORM, DEFAULT_LFO_WAVEFORM, false, true);
            return noErr;
            break;
        }
        case LFO1_AMPLITUDE:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("LFO Depth"), CFSTR(""), MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
            return noErr;
            break;
        }
        case LFO1_RATE:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("LFO Rate"), CFSTR("Hz"), MIN_LFO_RATE, MAX_LFO_RATE, DEFAULT_LFO_RATE);
            return noErr;
            break;
        }
        case LFO1_DESTINATION_OP1:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op1 LFO Dest"), CFSTR(""), MIN_DX_LFO_DESTINATION, MAX_DX_LFO_DESTINATION, DEFAULT_DX_LFO_DESTINATION, false, true); // true = indexed
            return noErr;
            break;
        }
        case LFO1_DESTINATION_OP2:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op2 LFO Dest"), CFSTR(""), MIN_DX_LFO_DESTINATION, MAX_DX_LFO_DESTINATION, DEFAULT_DX_LFO_DESTINATION, false, true); // true = indexed
            return noErr;
            break;
        }
        case LFO1_DESTINATION_OP3:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op3 LFO Dest"), CFSTR(""), MIN_DX_LFO_DESTINATION, MAX_DX_LFO_DESTINATION, DEFAULT_DX_LFO_DESTINATION, false, true); // true = indexed
            return noErr;
            break;
        }
        case LFO1_DESTINATION_OP4:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Op4 LFO Dest"), CFSTR(""), MIN_DX_LFO_DESTINATION, MAX_DX_LFO_DESTINATION, DEFAULT_DX_LFO_DESTINATION, false, true); // true = indexed
            return noErr;
            break;
        }
        case PORTAMENTO_TIME_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Portamento"), CFSTR("mS"), MIN_PORTAMENTO_TIME_MSEC, MAX_PORTAMENTO_TIME_MSEC, DEFAULT_PORTAMENTO_TIME_MSEC);
            return noErr;
            break;
        }
        case OUTPUT_AMPLITUDE_DB:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Volume"), CFSTR("dB"), MIN_OUTPUT_AMPLITUDE_DB, MAX_OUTPUT_AMPLITUDE_DB, DEFAULT_OUTPUT_AMPLITUDE_DB);
            return noErr;
            break;
        }
        case LEGATO_MODE:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Legato Mode"), CFSTR(""), MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_LEGATO_MODE, false, true);
            return noErr;
            break;
        }
        case RESET_TO_ZERO:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("ResetToZero"), CFSTR(""), MIN_ONOFF_SWITCH, MAX_ONOFF_SWITCH, DEFAULT_RESET_TO_ZERO, false, true);
            return noErr;
            break;
        }
        case PITCHBEND_RANGE:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("PBend Range"), CFSTR(""), MIN_PITCHBEND_RANGE, MAX_PITCHBEND_RANGE, DEFAULT_PITCHBEND_RANGE);
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
                setAUParameterStringList(CFSTR("DX1,DX2,DX3,DX4,DX5,DX6,DX7,DX8"), outStrings);
                return noErr;
                break;
            }
            case LFO1_WAVEFORM:
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
            case LFO1_DESTINATION_OP1:
            case LFO1_DESTINATION_OP2:
            case LFO1_DESTINATION_OP3:
            case LFO1_DESTINATION_OP4:
            {
                setAUParameterStringList(CFSTR("none,AmpMod,Vibrato"), outStrings);
                return noErr;
                break;
            }
                
            // --- all are OFF,ON 2-state switches
            case RESET_TO_ZERO:
            case VELOCITY_TO_ATTACK:
            case NOTE_NUM_TO_DECAY:
            {
                setAUParameterStringList(CFSTR("OFF,ON"), outStrings);
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
	// for FM synth, Voice Mode = FM Algorithm
	m_GlobalSynthParams.voiceParams.uVoiceMode = Globals()->GetParameter(VOICE_MODE);
	m_GlobalSynthParams.voiceParams.dOp4Feedback = Globals()->GetParameter(OP4_FEEDBACK)/100.0;
	m_GlobalSynthParams.voiceParams.dPortamentoTime_mSec = Globals()->GetParameter(PORTAMENTO_TIME_MSEC);
    
	// --- ranges
	m_GlobalSynthParams.voiceParams.dOscFoPitchBendModRange = Globals()->GetParameter(PITCHBEND_RANGE);
    
	// --- intensities
	m_GlobalSynthParams.voiceParams.dLFO1OscModIntensity = Globals()->GetParameter(LFO1_AMPLITUDE);
    
	// --- Oscillators:
	m_GlobalSynthParams.osc1Params.dAmplitude = calculateDXAmplitude(Globals()->GetParameter(OP1_OUTPUT_LEVEL));
	m_GlobalSynthParams.osc2Params.dAmplitude = calculateDXAmplitude(Globals()->GetParameter(OP2_OUTPUT_LEVEL));
	m_GlobalSynthParams.osc3Params.dAmplitude = calculateDXAmplitude(Globals()->GetParameter(OP3_OUTPUT_LEVEL));
	m_GlobalSynthParams.osc4Params.dAmplitude = calculateDXAmplitude(Globals()->GetParameter(OP4_OUTPUT_LEVEL));
    
	m_GlobalSynthParams.osc1Params.dFoRatio = Globals()->GetParameter(OP1_RATIO);
	m_GlobalSynthParams.osc2Params.dFoRatio = Globals()->GetParameter(OP2_RATIO);
	m_GlobalSynthParams.osc3Params.dFoRatio = Globals()->GetParameter(OP3_RATIO);
	m_GlobalSynthParams.osc4Params.dFoRatio = Globals()->GetParameter(OP4_RATIO);
    
	// --- EG1:
	m_GlobalSynthParams.eg1Params.dAttackTime_mSec = Globals()->GetParameter(EG1_ATTACK_MSEC);
	m_GlobalSynthParams.eg1Params.dDecayTime_mSec = Globals()->GetParameter(EG1_DECAY_MSEC);
	m_GlobalSynthParams.eg1Params.dSustainLevel = Globals()->GetParameter(EG1_SUSTAIN_LEVEL);
	m_GlobalSynthParams.eg1Params.dReleaseTime_mSec = Globals()->GetParameter(EG1_RELEASE_MSEC);
	m_GlobalSynthParams.eg1Params.bResetToZero = (bool)Globals()->GetParameter(RESET_TO_ZERO);
	m_GlobalSynthParams.eg1Params.bLegatoMode = (bool)Globals()->GetParameter(LEGATO_MODE);
    
	// --- EG2:
	m_GlobalSynthParams.eg2Params.dAttackTime_mSec = Globals()->GetParameter(EG2_ATTACK_MSEC);
	m_GlobalSynthParams.eg2Params.dDecayTime_mSec = Globals()->GetParameter(EG2_DECAY_MSEC);
	m_GlobalSynthParams.eg2Params.dSustainLevel = Globals()->GetParameter(EG2_SUSTAIN_LEVEL);
	m_GlobalSynthParams.eg2Params.dReleaseTime_mSec = Globals()->GetParameter(EG2_RELEASE_MSEC);
	m_GlobalSynthParams.eg2Params.bResetToZero = (bool)Globals()->GetParameter(RESET_TO_ZERO);
	m_GlobalSynthParams.eg2Params.bLegatoMode = (bool)Globals()->GetParameter(LEGATO_MODE);
    
	// --- EG3:
	m_GlobalSynthParams.eg3Params.dAttackTime_mSec = Globals()->GetParameter(EG3_ATTACK_MSEC);
	m_GlobalSynthParams.eg3Params.dDecayTime_mSec = Globals()->GetParameter(EG3_DECAY_MSEC);
	m_GlobalSynthParams.eg3Params.dSustainLevel = Globals()->GetParameter(EG3_SUSTAIN_LEVEL);
	m_GlobalSynthParams.eg3Params.dReleaseTime_mSec = Globals()->GetParameter(EG3_RELEASE_MSEC);
	m_GlobalSynthParams.eg3Params.bResetToZero = (bool)Globals()->GetParameter(RESET_TO_ZERO);
	m_GlobalSynthParams.eg3Params.bLegatoMode = (bool)Globals()->GetParameter(LEGATO_MODE);
    
	// --- EG4:
	m_GlobalSynthParams.eg4Params.dAttackTime_mSec = Globals()->GetParameter(EG4_ATTACK_MSEC);
	m_GlobalSynthParams.eg4Params.dDecayTime_mSec = Globals()->GetParameter(EG4_DECAY_MSEC);
	m_GlobalSynthParams.eg4Params.dSustainLevel = Globals()->GetParameter(EG4_SUSTAIN_LEVEL);
	m_GlobalSynthParams.eg4Params.dReleaseTime_mSec = Globals()->GetParameter(EG3_RELEASE_MSEC);
	m_GlobalSynthParams.eg4Params.bResetToZero = (bool)Globals()->GetParameter(RESET_TO_ZERO);
	m_GlobalSynthParams.eg4Params.bLegatoMode = (bool)Globals()->GetParameter(LEGATO_MODE);
    
	// --- LFO1:
	m_GlobalSynthParams.lfo1Params.uWaveform = Globals()->GetParameter(LFO1_WAVEFORM);
	m_GlobalSynthParams.lfo1Params.dOscFo = Globals()->GetParameter(LFO1_RATE);
    
	// --- DCA:
	m_GlobalSynthParams.dcaParams.dAmplitude_dB = Globals()->GetParameter(OUTPUT_AMPLITUDE_DB);
    
	// --- LFO1 Destination 1
	if(Globals()->GetParameter(LFO1_DESTINATION_OP1) == None)
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC1_OUTPUT_AMP, false);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC1_FO, false);
	}
	else if(Globals()->GetParameter(LFO1_DESTINATION_OP1) == AmpMod)
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC1_OUTPUT_AMP, true);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC1_FO, false);
	}
	else // vibrato
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC1_OUTPUT_AMP, false);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC1_FO, true);
	}
    
	// --- LFO1 Destination 2
	if(Globals()->GetParameter(LFO1_DESTINATION_OP2) == None)
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC2_OUTPUT_AMP, false);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC2_FO, false);
	}
	else if(Globals()->GetParameter(LFO1_DESTINATION_OP2) == AmpMod)
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC2_OUTPUT_AMP, true);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC2_FO, false);
	}
	else // vibrato
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC2_OUTPUT_AMP, false);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC2_FO, true);
	}
    
	// --- LFO1 Destination 3
	if(Globals()->GetParameter(LFO1_DESTINATION_OP3) == None)
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC3_OUTPUT_AMP, false);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC3_FO, false);
	}
	else if(Globals()->GetParameter(LFO1_DESTINATION_OP3) == AmpMod)
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC3_OUTPUT_AMP, true);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC3_FO, false);
	}
	else // vibrato
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC3_OUTPUT_AMP, false);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC3_FO, true);
	}
    
	// --- LFO1 Destination 4
	if(Globals()->GetParameter(LFO1_DESTINATION_OP4) == None)
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC4_OUTPUT_AMP, false);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC4_FO, false);
	}
	else if(Globals()->GetParameter(LFO1_DESTINATION_OP4) == AmpMod)
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC4_OUTPUT_AMP, true);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC4_FO, false);
	}
	else // vibrato
	{
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC4_OUTPUT_AMP, false);
		m_GlobalModMatrix.enableModMatrixRow(SOURCE_LFO1, DEST_OSC4_FO, true);
	}
    
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

    // needed???
  //  mAbsoluteSampleFrame += inNumberFrames;

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
CDXSynthVoice* AUSynth::getOldestVoice()
{
	int nTimeStamp = -1;
	CDXSynthVoice* pFoundVoice = NULL;
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
CDXSynthVoice* AUSynth::getOldestVoiceWithNote(UINT uMIDINote)
{
	int nTimeStamp = -1;
	CDXSynthVoice* pFoundVoice = NULL;
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
		CDXSynthVoice* pVoice = m_pVoiceArray[i];
        if(!pVoice) return noErr;
        
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
		CDXSynthVoice* pVoice = getOldestVoice();
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
		CDXSynthVoice* pVoice = getOldestVoiceWithNote(uMIDINote);
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
        if (inID == kAudioUnitProperty_CocoaUI)
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
            CFBundleRef bundle = CFBundleGetBundleWithIdentifier( CFSTR("developer.audiounit.yourname.DXSynth") );
            
            if (bundle == NULL)
                return fnfErr;
            
            CFURLRef bundleURL = CFBundleCopyResourceURL( bundle,
                                                         CFSTR("CocoaSynthView"),	// this is the name of the cocoa bundle as specified in the CocoaViewFactory.plist
                                                         CFSTR("bundle"),			// this is the extension of the cocoa bundle
                                                         NULL);
            
            if (bundleURL == NULL)
                return fnfErr;
            
            CFStringRef className = CFSTR("DXSynthViewFactory");	// name of the main class that implements the AUCocoaUIBase protocol
            
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
		if (inID == kAudioUnitProperty_MIDIOutputCallback)
        {
            // -- example see AudioUnitProperties.h for a list of props
            /*
             if (inID == kSomeProperty)
             {
             
             }*/
		}
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






