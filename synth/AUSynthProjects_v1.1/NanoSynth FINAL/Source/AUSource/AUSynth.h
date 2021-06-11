//
//  AUSynth.h
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

#include "AUInstrumentBase.h"
#include "AUSynthVersion.h"
#include "AUSynthStructures.h"
#include "synthfunctions.h"
#include "SynthParamLimits.h"

// --- for NanoSynth
// --- synth objects
#include "QBLimitedOscillator.h"
#include "LFO.h"
#include "EnvelopeGenerator.h"
#include "DCA.h"
#include "MoogLadderFilter.h"
#include "ModulationMatrix.h"

// --- one complete voice
struct nanoSynthVoice
{
	CQBLimitedOscillator m_Osc1;
	CQBLimitedOscillator m_Osc2;
	CLFO m_LFO1;
	CEnvelopeGenerator m_EG1;
	CMoogLadderFilter m_Filter1;
	CDCA m_DCA;
	CModulationMatrix m_ModulationMatrix;
};

#define MAX_VOICES 2

// --- our main AU Synth Object, derived from AUInstrumentBase
class AUSynth : public AUInstrumentBase
{
public:
    // --- const/dest
    AUSynth(AudioUnit inComponentInstance);
	virtual	~AUSynth();
    
    // --- AUInstrumentBase Overrides
    //
    // --- One-time init
	virtual OSStatus Initialize();
    
    // --- de-allocator (not used in our synths)
	virtual void Cleanup();
    
    // --- our version number, defined in AUSynthVersion.h
	virtual OSStatus Version() {return kAUSynthVersion;}
    
    // --- restore from presets
	OSStatus RestoreState(CFPropertyListRef plist);
   	
    // --- reset(); prepareForPlay();
    virtual OSStatus Reset(AudioUnitScope inScope,
                           AudioUnitElement inElement);

    // --- !!!the most important method: synthesizes audio!!!
	virtual OSStatus Render(AudioUnitRenderActionFlags& ioActionFlags,
                            const AudioTimeStamp& inTimeStamp,
                            UInt32 inNumberFrames);

    // --- host queries for information about our Paramaters (controls)
	virtual OSStatus GetParameterInfo(AudioUnitScope inScope,
                                      AudioUnitParameterID inParameterID,
                                      AudioUnitParameterInfo& outParameterInfo);
    
    // --- host queries for string-list controls we set up in above
    virtual OSStatus GetParameterValueStrings(AudioUnitScope inScope,
                                              AudioUnitParameterID inParameterID,
                                              CFArrayRef* outStrings);

     // --- host queries for Property info like MIDI and CocoaGUI capabilities
    virtual OSStatus GetPropertyInfo(AudioUnitPropertyID inID,
                                     AudioUnitScope inScope,
                                     AudioUnitElement inElement,
                                     UInt32& outDataSize,
                                     Boolean& outWritable);
	
    // --- host queries to get the Property info like MIDI Callback and Cocoa GUI factory
    //     results are returned cloaked as void*
	virtual OSStatus GetProperty(AudioUnitPropertyID inID,
                                 AudioUnitScope inScope,
                                 AudioUnitElement inElement,
                                 void* outData);
    
    // --- host calls to set a Property like MIDI callback
	virtual OSStatus SetProperty(AudioUnitPropertyID inID,
                                 AudioUnitScope inScope,
                                 AudioUnitElement inElement,
                                 const void* inData,
                                 UInt32 inDataSize);
    
    // --- MIDI Functions
    //
    // --- MIDI Note On
	virtual OSStatus StartNote(MusicDeviceInstrumentID inInstrument,
                               MusicDeviceGroupID inGroupID,
                               NoteInstanceID* outNoteInstanceID,
                               UInt32 inOffsetSampleFrame,
                               const MusicDeviceNoteParams &inParams);
    
    // --- MIDI Note Off
	virtual OSStatus StopNote(MusicDeviceGroupID inGroupID,
                              NoteInstanceID inNoteInstanceID,
                              UInt32 inOffsetSampleFrame);
    
    // --- MIDI Pitchbend (slightly different from all other CCs)
	virtual OSStatus HandlePitchWheel(UInt8 inChannel,
                                      UInt8 inPitch1,
                                      UInt8 inPitch2,
                                      UInt32 inStartFrame);
    
    // --- all other MIDI CC messages
    virtual OSStatus HandleControlChange(UInt8 inChannel,
                                         UInt8 inController,
                                         UInt8 inValue,
                                         UInt32	inStartFrame);

    // --- for ALL other MIDI messages you can get them here
    OSStatus HandleMidiEvent(UInt8 status,
                             UInt8 channel, 
                             UInt8 data1, 
                             UInt8 data2, 
                             UInt32 inStartFrame);

     // --- helper method for setting up Parameter Info
    void setAUParameterInfo(AudioUnitParameterInfo& outParameterInfo, 
                            CFStringRef paramName, 
                            CFStringRef paramUnits,  
                            Float32 fMinValue, 
                            Float32 fMaxValue, 
                            Float32 fDefaultValue,  
                            bool bLogControl = false, 
                            bool bStringListControl = false);
    
    // --- helper method for dealing with string-list control setup
    void setAUParameterStringList(CFStringRef stringList, 
                                  CFArrayRef* outStrings);
    
    // handle presets:
    virtual ComponentResult	GetPresets(CFArrayRef* outData)	const;    
    virtual OSStatus NewFactoryPresetSet(const AUPreset& inNewFactoryPreset);
    
    // CHALLENGE: use bool SetAFactoryPresetAsCurrent (const AUPreset & inPreset);
    //            to establish factory setting; see AUBase.h
    // 
    // CHALLENGE: implement an array of presets

    // --- example of a preset; you can have an array of them
    double factoryPreset[NUMBER_OF_SYNTH_PARAMETERS];

private:
    // --- NanoSynth Components
	CQBLimitedOscillator m_Osc1;
	CQBLimitedOscillator m_Osc2;
	CLFO m_LFO1;
   
    // NS3
	CEnvelopeGenerator m_EG1;
	CDCA m_DCA;
   
    // NS4
	CMoogLadderFilter m_Filter1;
    
    // NS MM 1
	// --- the Modulation Matrix
	CModulationMatrix m_GlobalModMatrix;
    
    // --- global params
	globalNanoSynthParams m_GlobalSynthParams;

    // voice stack
	nanoSynthVoice m_Voices[MAX_VOICES];
	int m_nPendingMIDINote[MAX_VOICES]; 		// if -1 no pending note
	int m_nPendingMIDIVelocity[MAX_VOICES]; 	// if -1 no pending velocity
    
	//--- for voice stealing
	void startNote(int nIndex, UINT uMIDINote, UINT uMIDIVelocity);
	void stealNote(int nIndex, UINT uPendingMIDINote,
                   UINT uPendingVelocity);

	// need these for mod matrix
	double m_dDefaultModIntensity;	// 1.0
	double m_dDefaultModRange;		// 1.0
	double m_dOscFoModRange;
	double m_dFilterModRange;
    double m_dOscFoPitchBendModRange;
	double m_dAmpModRange;

    // AU ONLY
    double m_dEG1OscIntensity;
    double m_dEG1DCAIntensity;
    double m_dFilterKeyTrackIntensity;

    // --- updates all voices at once
	void update();
    
	// --- for portamento
	double m_dLastNoteFrequency;
    
	// --- our receive channel
	UINT m_uMidiRxChannel;
};






