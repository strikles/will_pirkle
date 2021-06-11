//
//  DXSynthView.m
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

#import "VectorSynthView.h"
#import "SynthParamLimits.h"
#define KNOB_COUNT 128

#pragma mark ____ LISTENER CALLBACK DISPATCHER ____

// This listener responds to parameter changes, gestures, and property notifications
void EventListenerDispatcher (void *inRefCon, void *inObject, const AudioUnitEvent *inEvent, UInt64 inHostTime, Float32 inValue)
{
	VectorSynthView *SELF = (VectorSynthView *)inRefCon;
	[SELF eventListener:inObject event: inEvent value: inValue];
}

@implementation VectorSynthView

-(void) awakeFromNib 
{
    // --- initialize the knobs 
    //
    NSBundle* bundle = [NSBundle bundleForClass:[self class]];
    
    // --- create image array
    knobImages = [[NSMutableArray alloc] initWithCapacity:KNOB_COUNT];
    for(int i=0; i<KNOB_COUNT; i++)
    {
        NSImage* image = nil;
        NSString* file = [NSString stringWithFormat:@"knob%04d", i];
        NSString* path = [bundle pathForResource:file ofType: @"png"];
        if(path)
            image = [[NSImage alloc] initWithContentsOfURL:[NSURL fileURLWithPath:path]];
        if(image)
            [knobImages addObject:image];
    }
    
    // --- initialize the knobs
    //
    controlArray = [[NSMutableArray alloc] init];
    
    // --- column 1
    [wpRotaryKnob_0 initControlWithName:@"A-C Out (dB)"
                           controlIndex:OSC1_AMPLITUDE_DB
                                    min:MIN_OUTPUT_AMPLITUDE_DB
                                    max:MAX_OUTPUT_AMPLITUDE_DB
                                    def:DEFAULT_OUTPUT_AMPLITUDE_DB
                             voltOctave:NO
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_0];
    [wpRotaryKnob_0 setKnobImageArray:knobImages];
    
    [wpRotaryKnob_1 initControlWithName:@"B-D Out (dB)"
                           controlIndex:OSC2_AMPLITUDE_DB
                                    min:MIN_OUTPUT_AMPLITUDE_DB
                                    max:MAX_OUTPUT_AMPLITUDE_DB
                                    def:DEFAULT_OUTPUT_AMPLITUDE_DB
                             voltOctave:NO
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_1];
    [wpRotaryKnob_1 setKnobImageArray:knobImages];

    /*    
     [wpRotaryKnob_2 initControlWithName:@"HS Ratio"
     controlIndex:HARD_SYNC_RATIO
     min:MIN_HARD_SYNC_RATIO
     max:MAX_HARD_SYNC_RATIO
     def:DEFAULT_HARD_SYNC_RATIO
     voltOctave:NO
     integerControl:NO];
     [controlArray addObject:wpRotaryKnob_2];
     [wpRotaryKnob_2 setKnobImageArray:knobImages];
     */
    
    [wpRotaryKnob_3 initControlWithName:@"Pitch EG Int"
                           controlIndex:EG1_TO_OSC_INTENSITY
                                    min:MIN_BIPOLAR
                                    max:MAX_BIPOLAR
                                    def:DEFAULT_BIPOLAR
                             voltOctave:NO
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_3];
    [wpRotaryKnob_3 setKnobImageArray:knobImages];

    // --- column 2
    [wpRotaryKnob_4 initControlWithName:@"Filter fc"
                           controlIndex:FILTER_FC
                                    min:MIN_FILTER_FC
                                    max:MAX_FILTER_FC
                                    def:DEFAULT_FILTER_FC
                             voltOctave:YES
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_4];
    [wpRotaryKnob_4 setKnobImageArray:knobImages];

    [wpRotaryKnob_5 initControlWithName:@"Filter Q"
                           controlIndex:FILTER_Q
                                    min:MIN_FILTER_Q
                                    max:MAX_FILTER_Q
                                    def:DEFAULT_FILTER_Q
                             voltOctave:NO
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_5];
    [wpRotaryKnob_5 setKnobImageArray:knobImages];

    [wpRotaryKnob_6 initControlWithName:@"Filter EG Int"
                           controlIndex:EG1_TO_FILTER_INTENSITY
                                    min:MIN_BIPOLAR
                                    max:MAX_BIPOLAR
                                    def:DEFAULT_BIPOLAR
                             voltOctave:NO
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_6];
    [wpRotaryKnob_6 setKnobImageArray:knobImages];

    [wpRotaryKnob_7 initControlWithName:@"KeyTrack Int"
                           controlIndex:FILTER_KEYTRACK_INTENSITY
                                    min:MIN_FILTER_KEYTRACK_INTENSITY
                                    max:MAX_FILTER_KEYTRACK_INTENSITY
                                    def:DEFAULT_FILTER_KEYTRACK_INTENSITY
                             voltOctave:NO
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_7];
    [wpRotaryKnob_7 setKnobImageArray:knobImages];

    // --- column 3
    [wpRotaryKnob_8 initControlWithName:@"Attack"
                           controlIndex:EG1_ATTACK_MSEC
                                    min:MIN_EG_ATTACK_TIME
                                    max:MAX_EG_ATTACK_TIME
                                    def:DEFAULT_EG_ATTACK_TIME
                             voltOctave:NO
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_8];
    [wpRotaryKnob_8 setKnobImageArray:knobImages];

    [wpRotaryKnob_9 initControlWithName:@"Decay/Release"
                           controlIndex:EG1_DECAY_RELEASE_MSEC
                                    min:MIN_EG_DECAYRELEASE_TIME
                                    max:MAX_EG_DECAYRELEASE_TIME
                                    def:DEFAULT_EG_DECAYRELEASE_TIME
                             voltOctave:NO
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_9];
    [wpRotaryKnob_9 setKnobImageArray:knobImages];

    [wpRotaryKnob_10 initControlWithName:@"Sustain"
                            controlIndex:EG1_SUSTAIN_LEVEL
                                     min:MIN_EG_SUSTAIN_LEVEL
                                     max:MAX_EG_SUSTAIN_LEVEL
                                     def:DEFAULT_EG_SUSTAIN_LEVEL
                              voltOctave:NO
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_10];
    [wpRotaryKnob_10 setKnobImageArray:knobImages];
    
    /* unused
     [wpRotaryKnob_11 initControlWithName:@"Sustain"
     controlIndex:EG1_SUSTAIN_LEVEL
     min:MIN_EG_SUSTAIN_LEVEL
     max:MAX_EG_SUSTAIN_LEVEL
     def:DEFAULT_EG_SUSTAIN_LEVEL
     voltOctave:NO
     integerControl:NO];
     [controlArray addObject:wpRotaryKnob_11]
     [wpRotaryKnob_0 setKnobImageArray:knobImages];
     */
    
    
    // --- column 4
    /* unused
     [wpRotaryKnob_12 initControlWithName:@"Sustain"
     controlIndex:EG1_SUSTAIN_LEVEL
     min:MIN_EG_SUSTAIN_LEVEL
     max:MAX_EG_SUSTAIN_LEVEL
     def:DEFAULT_EG_SUSTAIN_LEVEL
     voltOctave:NO
     integerControl:NO];
     [controlArray addObject:wpRotaryKnob_12];
     [wpRotaryKnob_0 setKnobImageArray:knobImages];
     */
    
    [wpOMG_0 initControlWithName:@"LFO Waveform"
                    controlIndex:LFO1_WAVEFORM
                      enumString:@"sine,usaw,dsaw,tri,square,expo,rsh,qrsh"
                             def:DEFAULT_LFO_WAVEFORM
                   verySmallFont:NO];
    [controlArray addObject:wpOMG_0];
    
    [wpRotaryKnob_13 initControlWithName:@"LFO Rate"
                            controlIndex:LFO1_RATE
                                     min:MIN_LFO_RATE
                                     max:MAX_LFO_RATE
                                     def:DEFAULT_UNIPOLAR
                              voltOctave:NO
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_13];
    [wpRotaryKnob_13 setKnobImageArray:knobImages];

    [wpRotaryKnob_14 initControlWithName:@"LFO Depth"
                            controlIndex:LFO1_AMPLITUDE
                                     min:MIN_UNIPOLAR
                                     max:MAX_UNIPOLAR
                                     def:DEFAULT_LFO_RATE
                              voltOctave:NO
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_14];
    [wpRotaryKnob_14 setKnobImageArray:knobImages];

    /*
     [wpRotaryKnob_15 initControlWithName:@"LFO Depth"
     controlIndex:LFO1_AMPLITUDE
     min:MIN_UNIPOLAR
     max:MAX_UNIPOLAR
     def:DEFAULT_LFO_RATE
     voltOctave:NO
     integerControl:NO];
     [controlArray addObject:wpRotaryKnob_15];
     [wpRotaryKnob_15 setKnobImageArray:knobImages];
     */
    
    // --- column 5
    [wpRotaryKnob_16 initControlWithName:@"LFO Cutoff Int"
                            controlIndex:LFO1_TO_FILTER_INTENSITY
                                     min:MIN_BIPOLAR
                                     max:MAX_BIPOLAR
                                     def:DEFAULT_BIPOLAR
                              voltOctave:NO
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_16];
    [wpRotaryKnob_16 setKnobImageArray:knobImages];

    [wpRotaryKnob_17 initControlWithName:@"LFO Pitch Int"
                            controlIndex:LFO1_TO_OSC_INTENSITY
                                     min:MIN_BIPOLAR
                                     max:MAX_BIPOLAR
                                     def:DEFAULT_BIPOLAR
                              voltOctave:NO
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_17];
    [wpRotaryKnob_17 setKnobImageArray:knobImages];

    [wpRotaryKnob_18 initControlWithName:@"LFO DCA Int"
                            controlIndex:LFO1_TO_DCA_INTENSITY
                                     min:MIN_UNIPOLAR
                                     max:MAX_UNIPOLAR
                                     def:MIN_UNIPOLAR
                              voltOctave:NO
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_18];
    [wpRotaryKnob_18 setKnobImageArray:knobImages];

    [wpRotaryKnob_19 initControlWithName:@"LFO Pan Int"
                            controlIndex:LFO1_TO_PAN_INTENSITY
                                     min:MIN_BIPOLAR
                                     max:MAX_BIPOLAR
                                     def:DEFAULT_BIPOLAR
                              voltOctave:NO
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_19];
    [wpRotaryKnob_19 setKnobImageArray:knobImages];

    // --- column 6
    [wpRotaryKnob_20 initControlWithName:@"Volume (dB)"
                            controlIndex:OUTPUT_AMPLITUDE_DB
                                     min:MIN_OUTPUT_AMPLITUDE_DB
                                     max:MAX_OUTPUT_AMPLITUDE_DB
                                     def:DEFAULT_OUTPUT_AMPLITUDE_DB
                              voltOctave:NO
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_20];
    [wpRotaryKnob_20 setKnobImageArray:knobImages];

    [wpRotaryKnob_21 initControlWithName:@"DCA EG Int"
                            controlIndex:EG1_TO_DCA_INTENSITY
                                     min:MIN_BIPOLAR
                                     max:MAX_BIPOLAR
                                     def:MAX_BIPOLAR
                              voltOctave:NO
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_21];
    [wpRotaryKnob_21 setKnobImageArray:knobImages];

    /* unused
     [wpRotaryKnob_22 initControlWithName:@"LFO Cutoff Int"
     controlIndex:LFO1_TO_FILTER_INTENSITY
     min:MIN_BIPOLAR
     max:MAX_BIPOLAR
     def:DEFAULT_BIPOLAR
     voltOctave:NO
     integerControl:NO]
     ;
     [controlArray addObject:wpRotaryKnob_22];
     [wpRotaryKnob_22 setKnobImageArray:knobImages];
     
     [wpRotaryKnob_23 initControlWithName:@"LFO Cutoff Int"
     controlIndex:LFO1_TO_FILTER_INTENSITY
     min:MIN_BIPOLAR
     max:MAX_BIPOLAR
     def:DEFAULT_BIPOLAR
     voltOctave:NO
     integerControl:NO];
     [controlArray addObject:wpRotaryKnob_23];
     [wpRotaryKnob_23 setKnobImageArray:knobImages];
     */
    
    // --- column 7 VectorJoystick stuff
    [wpOMG_8 initControlWithName:@"Rotor Waveform"
                    controlIndex:ROTOR_WAVEFORM
                      enumString:@"sine,usaw,dsaw,tri,square,expo,rsh,qrsh"
                             def:DEFAULT_LFO_WAVEFORM
                   verySmallFont:NO];
    [controlArray addObject:wpOMG_8];
    
    [wpRotaryKnob_28 initControlWithName:@"Orbit X"
                            controlIndex:ORBIT_X
                                     min:MIN_UNIPOLAR
                                     max:MAX_UNIPOLAR
                                     def:DEFAULT_UNIPOLAR
                              voltOctave:NO
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_28];
    [wpRotaryKnob_28 setKnobImageArray:knobImages];

    
    [wpRotaryKnob_29 initControlWithName:@"Orbit Y"
                            controlIndex:ORBIT_Y
                                     min:MIN_UNIPOLAR
                                     max:MAX_UNIPOLAR
                                     def:DEFAULT_UNIPOLAR
                              voltOctave:NO
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_29];
    [wpRotaryKnob_29 setKnobImageArray:knobImages];
  
    [wpRotaryKnob_30 initControlWithName:@"Rotor Rate"
                            controlIndex:ROTOR_RATE
                                     min:MIN_SLOW_LFO_RATE
                                     max:MAX_SLOW_LFO_RATE
                                     def:DEFAULT_SLOW_LFO_RATE
                              voltOctave:NO
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_30];
    [wpRotaryKnob_30 setKnobImageArray:knobImages];

    // --- voice row
    [wpOMG_1 initControlWithName:@"Vector Mode"
                    controlIndex:VOICE_MODE
                      enumString:@"manual,program"
                             def:DEFAULT_VOICE_MODE
                   verySmallFont:NO]; // can try small font?
    [controlArray addObject:wpOMG_1];
    
    [wpOMG_7 initControlWithName:@"Path Mode"
                    controlIndex:PATH_MODE
                      enumString:@"once,loop,sustain,bckfrth"
                             def:DEFAULT_LOOP_MODE
                   verySmallFont:NO]; // can try small font?
    [controlArray addObject:wpOMG_7];
    
    [wpRotaryKnob_24 initControlWithName:@"Detune"
                            controlIndex:DETUNE_CENTS
                                     min:MIN_DETUNE_CENTS
                                     max:MAX_DETUNE_CENTS
                                     def:DEFAULT_DETUNE_CENTS
                              voltOctave:NO
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_24];
    [wpRotaryKnob_24 setKnobImageArray:knobImages];

    [wpRotaryKnob_25 initControlWithName:@"Portamento"
                            controlIndex:PORTAMENTO_TIME_MSEC
                                     min:MIN_PORTAMENTO_TIME_MSEC
                                     max:MAX_PORTAMENTO_TIME_MSEC
                                     def:DEFAULT_PORTAMENTO_TIME_MSEC
                              voltOctave:NO
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_25];
    [wpRotaryKnob_25 setKnobImageArray:knobImages];

    [wpRotaryKnob_26 initControlWithName:@"Octave"
                            controlIndex:OCTAVE
                                     min:MIN_OCTAVE
                                     max:MAX_OCTAVE
                                     def:DEFAULT_OCTAVE
                              voltOctave:NO
                          integerControl:YES];
    [controlArray addObject:wpRotaryKnob_26];
    [wpRotaryKnob_26 setKnobImageArray:knobImages];
    
    // --- globals row
    [wpOMG_2 initControlWithName:@"Vel->Att"
                    controlIndex:VELOCITY_TO_ATTACK
                      enumString:@"OFF,ON"
                             def:DEFAULT_VELOCITY_TO_ATTACK
                   verySmallFont:NO];
    [controlArray addObject:wpOMG_2];
    
    [wpOMG_3 initControlWithName:@"Note->Dcy"
                    controlIndex:NOTE_NUM_TO_DECAY
                      enumString:@"OFF,ON"
                             def:DEFAULT_NOTE_TO_DECAY
                   verySmallFont:NO];
    [controlArray addObject:wpOMG_3];
    
    [wpOMG_4 initControlWithName:@"ResetToZero"
                    controlIndex:RESET_TO_ZERO
                      enumString:@"OFF,ON"
                             def:DEFAULT_RESET_TO_ZERO
                   verySmallFont:NO];
    [controlArray addObject:wpOMG_4];
    
    [wpOMG_5 initControlWithName:@"Filter KeyTrack"
                    controlIndex:FILTER_KEYTRACK
                      enumString:@"OFF,ON"
                             def:DEFAULT_FILTER_KEYTRACK
                   verySmallFont:NO];
    [controlArray addObject:wpOMG_5];
    
    [wpOMG_6 initControlWithName:@"Legato Mode"
                    controlIndex:LEGATO_MODE
                      enumString:@"OFF,ON"
                             def:DEFAULT_LEGATO_MODE
                   verySmallFont:NO];
    [controlArray addObject:wpOMG_6];
    
    [wpRotaryKnob_27 initControlWithName:@"PBRange"
                            controlIndex:PITCHBEND_RANGE
                                     min:MIN_PITCHBEND_RANGE
                                     max:MAX_PITCHBEND_RANGE
                                     def:DEFAULT_PITCHBEND_RANGE
                              voltOctave:NO
                          integerControl:YES];
    [controlArray addObject:wpRotaryKnob_27];
    [wpRotaryKnob_27 setKnobImageArray:knobImages];
    
    NSString* path = [[NSBundle bundleForClass:[VectorSynthView class]] pathForResource:@"medGreyBrushed" ofType: @"bmp"];
    backImage = [[NSImage alloc] initWithContentsOfURL:[NSURL fileURLWithPath:path]];
    
}

#pragma mark ____ (INIT /) DEALLOC ____
- (void)dealloc
{
    [self removeListeners];
	
    [backImage release];
    [backgroundColor release];
    [controlArray dealloc];
    [knobImages dealloc];

	[[NSNotificationCenter defaultCenter] removeObserver: self];
	
    [super dealloc];
}


#pragma mark ____ PUBLIC FUNCTIONS ____

- (void)setAU:(AudioUnit)inAU 
{
	// remove previous listeners
	if(buddyAU) 
		[self removeListeners];
	
	buddyAU = inAU;
    
	// add new listeners
	[self addListeners];
	
	// initial setup
	[self synchronizeUIWithParameterValues];
}

- (void)drawRect:(NSRect)rect
{
    NSColor *backgroundPattern = [NSColor colorWithPatternImage:backImage];
    [backgroundPattern setFill];
	NSRectFill(rect);	
    
    // --- do group frame background images
    float x, y, w, h;
    float yOffset = -7;
    
    // --- column 1
    //
    // --- bottom knob
    NSRect knob3Rect = [wpRotaryKnob_3 frame];
    x = NSMinX(knob3Rect);
    w = NSWidth(knob3Rect);
    
    // --- top knob
    NSRect knob0Rect = [wpRotaryKnob_0 frame];
    y = NSMinY(knob0Rect);
    
    // --- height of col
    h = y + NSHeight(knob0Rect) + 45;
    
    // --- group frame rect
    NSRect groupRect1 = NSMakeRect(x, yOffset, w, h);
    
    // --- get the image
    NSString* path = [[NSBundle bundleForClass:[VectorSynthView class]] pathForResource:@"groupframe" ofType: @"png"];
    NSImage* image = [[NSImage alloc] initWithContentsOfURL:[NSURL fileURLWithPath:path]];
    
    float alpha = 1.0;
    
    // --- paint it
    if(image)[image drawInRect:groupRect1 fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:alpha];
    
    // --- column 2
    //
    // --- top knob
    NSRect knob4Rect = [wpRotaryKnob_4 frame];
    x = NSMinX(knob4Rect);
    
    // --- group frame rect
    NSRect groupRect2 = NSMakeRect(x, yOffset, w, h);
    
    // --- paint it
    if(image)[image drawInRect:groupRect2 fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:alpha];
    
    // --- column 3
    //
    // --- top knob
    NSRect knob8Rect = [wpRotaryKnob_8 frame];
    x = NSMinX(knob8Rect);
    
    // --- group frame rect
    NSRect groupRect3 = NSMakeRect(x, yOffset, w, h);
    
    // --- paint it
    if(image)[image drawInRect:groupRect3 fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:alpha];
    
    // --- column 4
    //
    // --- top knob
    NSRect omg0Rect = [wpOMG_0 frame];
    x = NSMinX(omg0Rect);
    
    // --- group frame rect
    NSRect groupRect4 = NSMakeRect(x, yOffset, w, h);
    
    // --- paint it
    if(image)[image drawInRect:groupRect4 fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:alpha];
    
    // --- column 5
    //
    // --- top knob
    NSRect knob16Rect = [wpRotaryKnob_16 frame];
    x = NSMinX(knob16Rect);
    y = NSMinY(knob16Rect);
    
    // --- group frame rect
    NSRect groupRect5 = NSMakeRect(x, yOffset, w, h);
    
    // --- paint it
    if(image)[image drawInRect:groupRect5 fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:alpha];
    
    // --- column 6
    //
    // --- top knob
    NSRect knob20Rect = [wpRotaryKnob_20 frame];
    x = NSMinX(knob20Rect);
    
    // --- group frame rect
    NSRect groupRect6 = NSMakeRect(x, yOffset, w, h);
    
    // --- paint it
    if(image)[image drawInRect:groupRect6 fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:alpha];
    
    // --- column 7
    //
    // --- top knob
    NSRect omg8Rect = [wpOMG_8 frame];
    x = NSMinX(omg8Rect);
    
    // --- group frame rect
    NSRect groupRect7 = NSMakeRect(x, yOffset, w, h);
    
    // --- paint it
    if(image)[image drawInRect:groupRect7 fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:alpha];
    
    // --- voice row
    // --- last knob
    NSRect omg1Rect = [wpOMG_1 frame];
    float x1 = NSMinX(omg1Rect);
    x1 -= 8;
    
    NSRect knob26Rect = [wpRotaryKnob_26 frame];
    x = NSMinX(knob26Rect);
    y = NSMinY(knob26Rect);
    h = NSHeight(knob26Rect);
    
    w = 5 + x + NSWidth(knob26Rect) - x1;
    
    // --- group frame rect
    NSRect groupRect8 = NSMakeRect(x1, y-100, w, h+132);
    
    // --- paint it
    if(image)[image drawInRect:groupRect8 fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:alpha];
    
 	[super drawRect: rect];	// we call super to draw all other controls after we have filled the background
}


#pragma mark ____ INTERFACE ACTIONS ____

- (IBAction)WPRotaryKnobChanged:(id)sender
{
    if(![sender respondsToSelector:@selector(controlID)] || 
       ![sender respondsToSelector:@selector(controlValue)])
        return;
   
    if([sender controlID] >= 0) 
    {
        // --- get the knob's user-value
        Float32 floatValue = [sender controlValue];
            
        // --- make an AudioUnitParameter set in our AU buddy    
        AudioUnitParameter param = {buddyAU, [sender controlID], kAudioUnitScope_Global, 0 };
            
        // --- set the AU Parameter; this calls SetParameter() in the au 
        AUParameterSet(AUEventListener, sender, &param, (Float32)floatValue, 0);
    }
}

- (IBAction)WPOptionMenuItemChanged:(id)sender
{
    if(![sender respondsToSelector:@selector(controlID)] || 
       ![sender respondsToSelector:@selector(currentSelection)])
        return;
    
    if([sender controlID] >= 0) 
    {
        // --- get the OMG selection index
        Float32 floatValue = [sender currentSelection];
        
        // --- make an AudioUnitParameter set in our AU buddy    
        AudioUnitParameter param = {buddyAU, [sender controlID], kAudioUnitScope_Global, 0 };
        
        // --- set the AU Parameter; this calls SetParameter() in the au 
        AUParameterSet(AUEventListener, sender, &param, (Float32)floatValue, 0);
    }
}
- (IBAction)WPTrackPadChanged:(id)sender
{
    if(![sender respondsToSelector:@selector(getControlCoordsX:Y:)])
        return;

    // --- the WPTrackPad generates bi-polar x,y values on the ranges
    //     x:[-1..+1] and y:[-1..+1]
    float x,y;
    [sender getControlCoordsX:&x
                            Y:&y];
   
    // --- BUT the vector joystick in the Synth is unipolar
    //     so convert here first
    x = 0.5*x + 0.5;
    y = 0.5*y + 0.5;
    
    // --- make an AudioUnitParameter set in our AU buddy
    AudioUnitParameter paramX = {buddyAU, VECTORJOYSTICK_X, kAudioUnitScope_Global, 0 };
    
    // --- set the AU Parameter; this calls SetParameter() in the au
    AUParameterSet(AUEventListener, sender, &paramX, (Float32)x, 0);
    
    // --- make an AudioUnitParameter set in our AU buddy
    AudioUnitParameter paramY = {buddyAU, VECTORJOYSTICK_Y, kAudioUnitScope_Global, 0 };
    
    // --- set the AU Parameter; this calls SetParameter() in the au
    AUParameterSet(AUEventListener, sender, &paramY, (Float32)y, 0);
}

void addParamListener(AUEventListenerRef listener, void* refCon, AudioUnitEvent *inEvent)
{
    // uncomment if you want to receive/track mouse clicks and movements
	/*
     inEvent->mEventType = kAudioUnitEvent_BeginParameterChangeGesture;
	verify_noerr (AUEventListenerAddEventType(listener, refCon, inEvent));
	
	inEvent->mEventType = kAudioUnitEvent_EndParameterChangeGesture;
	verify_noerr (AUEventListenerAddEventType(listener, refCon, inEvent)); */
    
	// --- this is for syncing when factory presets are selected!
	inEvent->mEventType = kAudioUnitEvent_ParameterValueChange;
	verify_noerr ( AUEventListenerAddEventType(	listener, refCon, inEvent));	
}

#pragma mark ____ PRIVATE FUNCTIONS ____
- (void)addListeners 
{
	if (buddyAU) 
    {
        // --- create hte event listener; AUEventListener
		verify_noerr(AUEventListenerCreate(EventListenerDispatcher, self,
                                           CFRunLoopGetCurrent(), kCFRunLoopDefaultMode, 0.05, 0.05, 
                                           &AUEventListener)); 
        
        // --- start with first control 0
 		AudioUnitEvent auEvent;
        
        // --- parameter 0
		AudioUnitParameter parameter = {buddyAU, 0, kAudioUnitScope_Global, 0};
       
        // --- set param & add it
        auEvent.mArgument.mParameter = parameter;		
		addParamListener (AUEventListener, self, &auEvent);
	
        // --- parameters 1 -> NUMBER_OF_SYNTH_PARAMETERS-1
        //     notice the way additional params are added using mParameterID
        for(int i=1; i<NUMBER_OF_SYNTH_PARAMETERS; i++)
        {
    		auEvent.mArgument.mParameter.mParameterID = i;
            addParamListener (AUEventListener, self, &auEvent);
        
        }
	}
}

- (void)removeListeners 
{
	if (AUEventListener) verify_noerr (AUListenerDispose(AUEventListener));
	AUEventListener = NULL;
	buddyAU = NULL;
}

- (id)getControlWithIndex:(int)index
{
    for(id item in controlArray)
    {
        if([item controlID] == index)
            return item;
    }
    return nil;
}

- (void)synchronizeUIWithParameterValues 
{
    // get the parameter values
  	Float32 paramValue;
   
    // make an AudioUnitParameter get from our AU buddy          
	AudioUnitParameter parameter = {buddyAU, 0, kAudioUnitScope_Global, 0};
	
    // get the AU Parameter[0] 
	if(AudioUnitGetParameter(buddyAU, 0, kAudioUnitScope_Global, 0, &paramValue) != noErr)
        return;
    
    // --- update controls
    id control = [self getControlWithIndex:0];
    if(control && [control respondsToSelector:@selector(setControlValue)])
        [control setControlValue:paramValue];
    
    // --- parameters 1 -> NUMBER_OF_SYNTH_PARAMETERS-1
    //     notice the way additional params are added using mParameterID
    for(int i=1; i<NUMBER_OF_SYNTH_PARAMETERS; i++)
    {
        // --- change the parameterID for subsequent controls
        parameter.mParameterID = i;
        AudioUnitGetParameter(buddyAU, i, kAudioUnitScope_Global, 0, &paramValue);

        // --- update controls
        id control = [self getControlWithIndex:i];
        if(control && [control respondsToSelector:@selector(setControlValue:)])
            [control setControlValue:paramValue];        
    }
}

#pragma mark ____ LISTENER CALLBACK DISPATCHEE ____
//
// PRESETS:
//
// Handle kAudioUnitProperty_PresentPreset event
- (void)eventListener:(void *) inObject event:(const AudioUnitEvent *)inEvent value:(Float32)inValue 
{
	switch (inEvent->mEventType) 
    {
        // --- for presets this gets called for each parameter
		case kAudioUnitEvent_ParameterValueChange:	
        {
            // --- get the control from our array
            id control = [self getControlWithIndex:inEvent->mArgument.mParameter.mParameterID];
            
            // --- all of MY (WP) controls use the same method, setControlValue: so there is nothing else to do
            if(control && [control respondsToSelector:@selector(setControlValue:)])
                [control setControlValue:inValue];        
        }
	}
}

/* If we get a mouseDown, that means it was not in the graph view, or one of the text fields. 
   In this case, we should make the window the first responder. 
   This will deselect our text fields if they are active. */
- (void) mouseDown: (NSEvent *) theEvent
{
	[super mouseDown: theEvent];
	[[self window] makeFirstResponder: self];
}

- (BOOL) acceptsFirstResponder {
	return YES;
}

- (BOOL) becomeFirstResponder {	
	return YES;
}

- (BOOL) isOpaque {
	return NO;
}

@end
