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
//	getMyComponentDirectory
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
 //   factoryPreset[NOISE_OSC_AMP_DB] = -12.0;
 	
    // --- define number of params (controls)
	Globals()->UseIndexedParameters(NUMBER_OF_SYNTH_PARAMETERS); //NUM_PARAMS
    
    // --- initialize the controls here!
	// --- these are defined in SynthParamLimits.h
    //
    Globals()->SetParameter(OSC_WAVEFORM, DEFAULT_PITCHED_OSC_WAVEFORM);
	Globals()->SetParameter(LFO1_WAVEFORM, DEFAULT_LFO_WAVEFORM);
 	Globals()->SetParameter(LFO1_RATE, DEFAULT_LFO_RATE);
 	Globals()->SetParameter(LFO1_AMPLITUDE, DEFAULT_UNIPOLAR);
	Globals()->SetParameter(LFO1_MODE, DEFAULT_LFO_MODE);

    Globals()->SetParameter(EG1_ATTACK_MSEC, DEFAULT_EG_ATTACK_TIME);
	Globals()->SetParameter(EG1_DECAY_MSEC, DEFAULT_EG_DECAY_TIME);
 	Globals()->SetParameter(EG1_SUSTAIN_LEVEL, DEFAULT_EG_SUSTAIN_LEVEL);
 	Globals()->SetParameter(EG1_RELEASE_MSEC, DEFAULT_EG_RELEASE_TIME);
	Globals()->SetParameter(OUTPUT_PAN, DEFAULT_PAN);
	Globals()->SetParameter(OUTPUT_AMPLITUDE_DB, DEFAULT_OUTPUT_AMPLITUDE_DB);
	Globals()->SetParameter(EG1_TO_DCA_INTENSITY, MAX_BIPOLAR); // NOTE!
    
	Globals()->SetParameter(RESET_TO_ZERO, DEFAULT_RESET_TO_ZERO);
	Globals()->SetParameter(LEGATO_MODE, DEFAULT_LEGATO_MODE); // NOTE!
    
	// receive on all channels
	m_uMidiRxChannel = MIDI_CH_ALL;
    
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	AUSynth::AUSynth
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AUSynth::~AUSynth()
{

}

void AUSynth::Cleanup()
{

    
}


// --- this will get called during the normal init OR if the sample rate changes; may be
//     redundant with initialize() below
ComponentResult	AUSynth::Reset(AudioUnitScope inScope, AudioUnitElement inElement)
{
    AUBase::Reset(inScope, inElement);
    
    // --- all notes off

    return noErr;
}

// --- this will get called during the normal init OR if the sample rate changes
ComponentResult AUSynth::Initialize()
{	    
    // --- init the base class
	AUInstrumentBase::Initialize();

    // --- inits
    m_Osc1.setSampleRate(GetOutput(0)->GetStreamFormat().mSampleRate);
    m_Osc2.setSampleRate(GetOutput(0)->GetStreamFormat().mSampleRate);
    m_Osc2.m_nCents = 2.5;

    m_LFO1.setSampleRate(GetOutput(0)->GetStreamFormat().mSampleRate);
    m_EG1.setSampleRate(GetOutput(0)->GetStreamFormat().mSampleRate);

    // --- big update
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
        case OSC_WAVEFORM:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Osc Waveform"), CFSTR(""), MIN_PITCHED_OSC_WAVEFORM, MAX_PITCHED_OSC_WAVEFORM, DEFAULT_PITCHED_OSC_WAVEFORM, false, true); // true = indexed strings
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
        case LFO1_MODE:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("LFO Mode"), CFSTR(""), MIN_LFO_MODE, MAX_LFO_MODE, DEFAULT_LFO_MODE, false, true); // true = indexed strings
            return noErr;
            break;
        }
        case EG1_ATTACK_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Attack"), CFSTR("mS"), MIN_EG_ATTACK_TIME, MAX_EG_ATTACK_TIME, DEFAULT_EG_ATTACK_TIME);
            return noErr;
            break;
        }
        case EG1_DECAY_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Decay"), CFSTR("mS"), MIN_EG_DECAY_TIME, MAX_EG_DECAY_TIME, DEFAULT_EG_DECAY_TIME);
            return noErr;
            break;
        }
        case EG1_SUSTAIN_LEVEL:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Sustain"), CFSTR(""), MIN_EG_SUSTAIN_LEVEL, MAX_EG_SUSTAIN_LEVEL, DEFAULT_EG_SUSTAIN_LEVEL);
            return noErr;
            break;
        }
        case EG1_RELEASE_MSEC:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Release"), CFSTR("mS"), MIN_EG_RELEASE_TIME, MAX_EG_RELEASE_TIME, DEFAULT_EG_RELEASE_TIME);
            return noErr;
            break;
        }
        case OUTPUT_PAN:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Pan"), CFSTR(""), MIN_PAN, MAX_PAN, DEFAULT_PAN);
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
        case RESET_TO_ZERO:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("ResetToZero"), CFSTR(""), MIN_ONOFF_SWITCH, MAX_ONOFF_SWITCH, DEFAULT_RESET_TO_ZERO, false, true);
            return noErr;
            break;
        }
        case LEGATO_MODE:
        {
            setAUParameterInfo(outParameterInfo, CFSTR("Legato Mode"), CFSTR(""), MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_LEGATO_MODE, false, true);
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
            case OSC_WAVEFORM:
            {
                setAUParameterStringList(CFSTR("SINE,SAW1,SAW2,SAW3,TRI,SQUARE,NOISE,PNOISE"), outStrings);
                return noErr;
                break;
            }
            case LFO1_WAVEFORM:
            {
                setAUParameterStringList(CFSTR("sine,usaw,dsaw,tri,square,expo,rsh,qrsh"), outStrings);
                return noErr;
                break;
            }
            case LFO1_MODE:
            {
                setAUParameterStringList(CFSTR("sync,shot,free"), outStrings);
                return noErr;
                break;
            }
            case RESET_TO_ZERO:
            case LEGATO_MODE:
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
    // update synth
    m_Osc1.m_uWaveform = Globals()->GetParameter(OSC_WAVEFORM);
	m_Osc2.m_uWaveform = Globals()->GetParameter(OSC_WAVEFORM);
    
	m_Osc1.update();
	m_Osc2.update();
    
	m_LFO1.m_uWaveform = Globals()->GetParameter(LFO1_WAVEFORM);
	m_LFO1.m_dAmplitude = Globals()->GetParameter(LFO1_AMPLITUDE);
	m_LFO1.m_dOscFo = Globals()->GetParameter(LFO1_RATE);
	m_LFO1.m_uLFOMode = Globals()->GetParameter(LFO1_MODE);
  
    m_EG1.setAttackTime_mSec(Globals()->GetParameter(EG1_ATTACK_MSEC));
	m_EG1.setDecayTime_mSec(Globals()->GetParameter(EG1_DECAY_MSEC));
	m_EG1.setSustainLevel(Globals()->GetParameter(EG1_SUSTAIN_LEVEL));
	m_EG1.setReleaseTime_mSec(Globals()->GetParameter(EG1_RELEASE_MSEC));
  
	m_EG1.m_bResetToZero = Globals()->GetParameter(RESET_TO_ZERO);
	m_EG1.m_bLegatoMode = Globals()->GetParameter(LEGATO_MODE);
    
	m_DCA.setPanControl(Globals()->GetParameter(OUTPUT_PAN));
	m_DCA.setAmplitude_dB(Globals()->GetParameter(OUTPUT_AMPLITUDE_DB));
	m_DCA.update();

	m_LFO1.update();
}


OSStatus AUSynth::Render(AudioUnitRenderActionFlags& ioActionFlags,
                         const AudioTimeStamp& inTimeStamp,
                         UInt32 inNumberFrames)
{
    // --- broadcast MIDI events
    PerformEvents(inTimeStamp);
    
    // --- do the mass update for this frame
    update();
   
    // --- get the number of channels
    AudioBufferList& bufferList = GetOutput(0)->GetBufferList();
    UInt32 numChans = bufferList.mNumberBuffers;

    // --- we only support mono/stereo
	if (numChans > 2) 
        return kAudioUnitErr_FormatNotSupported;

    // --- get pointers for buffer lists
    float* left = (float*)bufferList.mBuffers[0].mData;
    float* right = numChans == 2 ? (float*)bufferList.mBuffers[1].mData : NULL;
    
    // --- output "accumulator"
    double dOutL = 0.0;
    double dOutR = 0.0;
    
    // --- the frame processing loop
    for(UInt32 frame=0; frame<inNumberFrames; ++frame)
    {	
        // --- clear accumulators
        dOutL = 0.0;
        dOutR = 0.0;
        if(m_Osc1.m_bNoteOn)
        {
            double dLFO1Out = m_LFO1.doOscillate();
            m_Osc1.setFoModExp(dLFO1Out);
            m_Osc1.update();
            
            m_Osc2.setFoModExp(dLFO1Out);
            m_Osc2.update();
            
            double dOscOut = 0.5*m_Osc1.doOscillate() + 0.5*m_Osc2.doOscillate();
            
            double dEGOut = m_EG1.doEnvelope();
            
            m_DCA.setEGMod(dEGOut*Globals()->GetParameter(EG1_TO_DCA_INTENSITY));
            m_DCA.update();
            
            m_DCA.doDCA(dOscOut, dOscOut, dOutL, dOutR);
            
            // now check for note off
            //	enum {off, attack, decay, sustain, release, shutdown};
            if(m_EG1.getState() == 0) // off
            {
                m_Osc1.stopOscillator();
                m_Osc2.stopOscillator();
                m_LFO1.stopOscillator();
            }
        }
        // write out to buffer
        // --- mono
        left[frame] = dOutL;
        
        // --- stereo
        if(right) right[frame] = dOutR;
    }

	return noErr;
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
    
#ifdef LOG_MIDI
    printf("-- Note On Ch:%d Note:%d Vel:%d \n", uChannel, uMIDINote, uVelocity);
#endif
    
    // --- on
    m_Osc1.m_uMIDINoteNumber = uMIDINote;
    m_Osc1.m_dOscFo = midiFreqTable[uMIDINote];
    m_Osc1.update();
    
    m_Osc2.m_uMIDINoteNumber = uMIDINote;
    m_Osc2.m_dOscFo = midiFreqTable[uMIDINote];
    m_Osc2.update();
    
    // if note is playing, just change the pitch
    if(!m_Osc1.m_bNoteOn)
    {
        m_Osc1.startOscillator();
        m_Osc2.startOscillator();
    }
    
    m_LFO1.startOscillator();
    
    m_EG1.startEG();
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
    
#ifdef LOG_MIDI
    // --- NOTE: AU does not transmit note of velocity!
    printf("-- Note Off Ch:%d Note:%d \n", uChannel, uMIDINote);
#endif
		
    // -- off
    // --- turn off IF this is the proper note (last note played)
    if(uMIDINote == m_Osc1.m_uMIDINoteNumber)
        m_EG1.noteOff();
    
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
    UINT uChannel = (UINT)inChannel;
    
    // --- test channel/ignore
	if(m_uMidiRxChannel != MIDI_CH_ALL && uChannel != m_uMidiRxChannel)
		return noErr;
    
	switch(inController)
	{
        case MOD_WHEEL:
        {
            #ifdef LOG_MIDI
            printf("-- Mod Wheel Ch:%d Value:%d \n", uChannel, inValue);
            #endif
            break;
        }
        case VOLUME_CC07:
        {
            // --- NOTE: LOGIC 9 CAPTURES VOLUME FOR ITSELF ---
            #ifdef LOG_MIDI
            printf("-- Volume Ch:%d Value:%d \n", uChannel, inValue);
            #endif
            
            break;
        }
        case PAN_CC10:
        {
            // --- NOTE: LOGIC 9 CAPTURES PAN FOR ITSELF ---
            #ifdef LOG_MIDI
            printf("-- Pan Ch:%d Value:%d \n", uChannel, inValue);
            #endif
                    
            break;
        }
        case EXPRESSION_CC11:
        {
            #ifdef LOG_MIDI
            printf("-- Expression Ch:%d Value:%d \n", uChannel, inValue);
            #endif
                    
            break;
        }
        case SUSTAIN_PEDAL:
        {
            // --- 64 or greater = ON for all switch messages
            bool m_bSustainPedal = (UINT)inValue > 63 ? true : false;
                    
            #ifdef LOG_MIDI
            if(m_bSustainPedal)
                printf("-- Sustain Pedal ON");
            else
                printf("-- Sustain Pedal OFF");
            #endif
            break;
        }
        case ALL_NOTES_OFF:
        {
            #ifdef LOG_MIDI
            printf("-- All Notes Off!");
            #endif
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
    
	return true;
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
            outDataSize = sizeof (AudioUnitCocoaViewInfo);
            outWritable = false;         
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
            CFBundleRef bundle = CFBundleGetBundleWithIdentifier( CFSTR("developer.audiounit.yourname.NanoSynth") );
            
            if (bundle == NULL)
                return fnfErr;
            
            CFURLRef bundleURL = CFBundleCopyResourceURL( bundle,
                                                         CFSTR("CocoaSynthView"),	// this is the name of the cocoa bundle as specified in the CocoaViewFactory.plist
                                                         CFSTR("bundle"),			// this is the extension of the cocoa bundle
                                                         NULL);
            
            if (bundleURL == NULL)
                return fnfErr;
            
            CFStringRef className = CFSTR("NanoSynthViewFactory");	// name of the main class that implements the AUCocoaUIBase protocol
            
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






