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

#import "DXSynthView.h"
#import "SynthParamLimits.h"
#define KNOB_COUNT 128

#pragma mark ____ LISTENER CALLBACK DISPATCHER ____

// This listener responds to parameter changes, gestures, and property notifications
void EventListenerDispatcher (void *inRefCon, void *inObject, const AudioUnitEvent *inEvent, UInt64 inHostTime, Float32 inValue)
{
	DXSynthView *SELF = (DXSynthView *)inRefCon;
	[SELF eventListener:inObject event: inEvent value: inValue];
}

@implementation DXSynthView

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
    
    // --- create the control array
    controlArray = [[NSMutableArray alloc] init];
    
    // --- row 1
    [wpRotaryKnob_0 initControlWithName:@"Ratio" 
                           controlIndex:OP1_RATIO
                                    min:MIN_OP_RATIO 
                                    max:MAX_OP_RATIO 
                                    def:DEFAULT_OP_RATIO
                             voltOctave:NO 
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_0];
    [wpRotaryKnob_0 setKnobImageArray:knobImages];
    
    [wpRotaryKnob_1 initControlWithName:@"Attack" 
                           controlIndex:EG1_ATTACK_MSEC
                                    min:MIN_EG_ATTACK_TIME 
                                    max:MAX_EG_ATTACK_TIME 
                                    def:DEFAULT_EG_ATTACK_TIME
                             voltOctave:NO 
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_1];
    [wpRotaryKnob_1 setKnobImageArray:knobImages];

    [wpRotaryKnob_2 initControlWithName:@"Decay" 
                           controlIndex:EG1_DECAY_MSEC
                                    min:MIN_EG_DECAY_TIME 
                                    max:MAX_EG_DECAY_TIME 
                                    def:DEFAULT_EG_DECAY_TIME
                             voltOctave:NO 
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_2];
    [wpRotaryKnob_2 setKnobImageArray:knobImages];

    [wpRotaryKnob_3 initControlWithName:@"Sustain" 
                           controlIndex:EG1_SUSTAIN_LEVEL
                                    min:MIN_EG_SUSTAIN_LEVEL 
                                    max:MAX_EG_SUSTAIN_LEVEL 
                                    def:DEFAULT_EG_SUSTAIN_LEVEL 
                             voltOctave:NO 
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_3];
    [wpRotaryKnob_3 setKnobImageArray:knobImages];

    [wpRotaryKnob_4 initControlWithName:@"Release" 
                           controlIndex:EG1_RELEASE_MSEC
                                    min:MIN_EG_RELEASE_TIME 
                                    max:MAX_EG_RELEASE_TIME 
                                    def:DEFAULT_EG_RELEASE_TIME
                             voltOctave:NO 
                         integerControl:NO];    
    [controlArray addObject:wpRotaryKnob_4];
    [wpRotaryKnob_4 setKnobImageArray:knobImages];

    [wpRotaryKnob_5 initControlWithName:@"Output"
                           controlIndex:OP1_OUTPUT_LEVEL
                                    min:MIN_DX_OUTPUT_LEVEL 
                                    max:MAX_DX_OUTPUT_LEVEL 
                                    def:DEFAULT_DX_OUTPUT_LEVEL
                             voltOctave:NO 
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_5];
    [wpRotaryKnob_5 setKnobImageArray:knobImages];

    // --- row 2
    [wpRotaryKnob_6 initControlWithName:@"Ratio" 
                           controlIndex:OP2_RATIO
                                    min:MIN_OP_RATIO 
                                    max:MAX_OP_RATIO 
                                    def:DEFAULT_OP_RATIO
                             voltOctave:NO 
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_6];
    [wpRotaryKnob_6 setKnobImageArray:knobImages];

    [wpRotaryKnob_7 initControlWithName:@"Attack"
                           controlIndex:EG2_ATTACK_MSEC
                                    min:MIN_EG_ATTACK_TIME 
                                    max:MAX_EG_ATTACK_TIME 
                                    def:DEFAULT_EG_ATTACK_TIME
                             voltOctave:NO 
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_7];
    [wpRotaryKnob_7 setKnobImageArray:knobImages];

    [wpRotaryKnob_8 initControlWithName:@"Decay"
                           controlIndex:EG2_DECAY_MSEC
                                    min:MIN_EG_DECAY_TIME 
                                    max:MAX_EG_DECAY_TIME 
                                    def:DEFAULT_EG_DECAY_TIME
                             voltOctave:NO 
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_8];
    [wpRotaryKnob_8 setKnobImageArray:knobImages];

    [wpRotaryKnob_9 initControlWithName:@"Sustain"
                           controlIndex:EG2_SUSTAIN_LEVEL
                                    min:MIN_EG_SUSTAIN_LEVEL 
                                    max:MAX_EG_SUSTAIN_LEVEL 
                                    def:DEFAULT_EG_SUSTAIN_LEVEL
                             voltOctave:NO 
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_9];
    [wpRotaryKnob_9 setKnobImageArray:knobImages];

    [wpRotaryKnob_10 initControlWithName:@"Release"
                           controlIndex:EG2_RELEASE_MSEC
                                    min:MIN_EG_RELEASE_TIME 
                                    max:MAX_EG_RELEASE_TIME 
                                    def:DEFAULT_EG_RELEASE_TIME
                             voltOctave:NO 
                         integerControl:NO];    
    [controlArray addObject:wpRotaryKnob_10];
    [wpRotaryKnob_10 setKnobImageArray:knobImages];

    [wpRotaryKnob_11 initControlWithName:@"Output"
                           controlIndex:OP2_OUTPUT_LEVEL
                                    min:MIN_DX_OUTPUT_LEVEL 
                                    max:MAX_DX_OUTPUT_LEVEL 
                                    def:DEFAULT_DX_OUTPUT_LEVEL
                             voltOctave:NO 
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_11];
    [wpRotaryKnob_11 setKnobImageArray:knobImages];

    // --- row 3
    [wpRotaryKnob_12 initControlWithName:@"Ratio" 
                           controlIndex:OP3_RATIO
                                    min:MIN_OP_RATIO 
                                    max:MAX_OP_RATIO 
                                    def:DEFAULT_OP_RATIO
                             voltOctave:NO 
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_12];
    [wpRotaryKnob_12 setKnobImageArray:knobImages];

    [wpRotaryKnob_13 initControlWithName:@"Attack"
                           controlIndex:EG3_ATTACK_MSEC
                                    min:MIN_EG_ATTACK_TIME 
                                    max:MAX_EG_ATTACK_TIME 
                                    def:DEFAULT_EG_ATTACK_TIME
                             voltOctave:NO 
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_13];
    [wpRotaryKnob_13 setKnobImageArray:knobImages];

    [wpRotaryKnob_14 initControlWithName:@"Decay"
                           controlIndex:EG3_DECAY_MSEC
                                    min:MIN_EG_DECAY_TIME 
                                    max:MAX_EG_DECAY_TIME 
                                    def:DEFAULT_EG_DECAY_TIME
                             voltOctave:NO 
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_14];
    [wpRotaryKnob_14 setKnobImageArray:knobImages];

    [wpRotaryKnob_15 initControlWithName:@"Sustain"
                           controlIndex:EG3_SUSTAIN_LEVEL
                                    min:MIN_EG_SUSTAIN_LEVEL 
                                    max:MAX_EG_SUSTAIN_LEVEL 
                                    def:DEFAULT_EG_SUSTAIN_LEVEL
                             voltOctave:NO 
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_15];
    [wpRotaryKnob_15 setKnobImageArray:knobImages];

    [wpRotaryKnob_16 initControlWithName:@"Release"
                           controlIndex:EG3_RELEASE_MSEC
                                    min:MIN_EG_RELEASE_TIME 
                                    max:MAX_EG_RELEASE_TIME 
                                    def:DEFAULT_EG_RELEASE_TIME
                             voltOctave:NO 
                         integerControl:NO];    
    [controlArray addObject:wpRotaryKnob_16];
    [wpRotaryKnob_16 setKnobImageArray:knobImages];

    [wpRotaryKnob_17 initControlWithName:@"Output"
                           controlIndex:OP3_OUTPUT_LEVEL
                                    min:MIN_DX_OUTPUT_LEVEL 
                                    max:MAX_DX_OUTPUT_LEVEL 
                                    def:DEFAULT_DX_OUTPUT_LEVEL
                             voltOctave:NO 
                         integerControl:NO];
    [controlArray addObject:wpRotaryKnob_17];
    [wpRotaryKnob_17 setKnobImageArray:knobImages];

    // --- row 4
    [wpRotaryKnob_18 initControlWithName:@"Ratio" 
                            controlIndex:OP4_RATIO
                                     min:MIN_OP_RATIO 
                                     max:MAX_OP_RATIO 
                                     def:DEFAULT_OP_RATIO
                              voltOctave:NO 
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_18];
    [wpRotaryKnob_18 setKnobImageArray:knobImages];

    [wpRotaryKnob_19 initControlWithName:@"Attack"
                            controlIndex:EG4_ATTACK_MSEC
                                     min:MIN_EG_ATTACK_TIME 
                                     max:MAX_EG_ATTACK_TIME 
                                     def:DEFAULT_EG_ATTACK_TIME
                              voltOctave:NO 
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_19];
    [wpRotaryKnob_19 setKnobImageArray:knobImages];

    [wpRotaryKnob_20 initControlWithName:@"Decay"
                            controlIndex:EG4_DECAY_MSEC
                                     min:MIN_EG_DECAY_TIME 
                                     max:MAX_EG_DECAY_TIME 
                                     def:DEFAULT_EG_DECAY_TIME
                              voltOctave:NO 
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_20];
    [wpRotaryKnob_20 setKnobImageArray:knobImages];

    [wpRotaryKnob_21 initControlWithName:@"Sustain"
                            controlIndex:EG4_SUSTAIN_LEVEL
                                     min:MIN_EG_SUSTAIN_LEVEL 
                                     max:MAX_EG_SUSTAIN_LEVEL 
                                     def:DEFAULT_EG_SUSTAIN_LEVEL
                              voltOctave:NO 
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_21];
    [wpRotaryKnob_21 setKnobImageArray:knobImages];

    [wpRotaryKnob_22 initControlWithName:@"Release"
                            controlIndex:EG4_RELEASE_MSEC
                                     min:MIN_EG_RELEASE_TIME 
                                     max:MAX_EG_RELEASE_TIME 
                                     def:DEFAULT_EG_RELEASE_TIME
                              voltOctave:NO 
                          integerControl:NO];    
    [controlArray addObject:wpRotaryKnob_22];
    [wpRotaryKnob_22 setKnobImageArray:knobImages];

    [wpRotaryKnob_23 initControlWithName:@"Output"
                            controlIndex:OP4_OUTPUT_LEVEL
                                     min:MIN_DX_OUTPUT_LEVEL 
                                     max:MAX_DX_OUTPUT_LEVEL 
                                     def:DEFAULT_DX_OUTPUT_LEVEL
                              voltOctave:NO 
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_23];
    [wpRotaryKnob_23 setKnobImageArray:knobImages];

    [wpRotaryKnob_24 initControlWithName:@"Feedback"
                            controlIndex:OP4_FEEDBACK
                                     min:MIN_OP_FEEDBACK 
                                     max:MAX_OP_FEEDBACK 
                                     def:DEFAULT_OP_FEEDBACK
                              voltOctave:NO 
                          integerControl:NO];
    [controlArray addObject:wpRotaryKnob_24];
    [wpRotaryKnob_24 setKnobImageArray:knobImages];


    // --- voice row
    [wpOMG_0 initControlWithName:@"DX Algorithm"
                    controlIndex:VOICE_MODE
                      enumString:@"1,2,3,4,5,6,7,8" 
                             def:DEFAULT_VOICE_MODE
                        verySmallFont:NO]; // can try small font?
    [controlArray addObject:wpOMG_0];
    
    [wpRotaryKnob_25 initControlWithName:@"Portamento" 
                            controlIndex:PORTAMENTO_TIME_MSEC
                                     min:MIN_PORTAMENTO_TIME_MSEC 
                                     max:MAX_PORTAMENTO_TIME_MSEC 
                                     def:DEFAULT_PORTAMENTO_TIME_MSEC
                              voltOctave:NO 
                          integerControl:NO]; 
    [controlArray addObject:wpRotaryKnob_25];
    [wpRotaryKnob_25 setKnobImageArray:knobImages];
 
    [wpRotaryKnob_26 initControlWithName:@"Volume (dB)" 
                            controlIndex:OUTPUT_AMPLITUDE_DB
                                     min:MIN_OUTPUT_AMPLITUDE_DB 
                                     max:MAX_OUTPUT_AMPLITUDE_DB 
                                     def:DEFAULT_OUTPUT_AMPLITUDE_DB
                              voltOctave:NO 
                          integerControl:NO]; 
    [controlArray addObject:wpRotaryKnob_26];
    [wpRotaryKnob_26 setKnobImageArray:knobImages];
 
    // --- globals row
    [wpOMG_1 initControlWithName:@"Vel->Att"
                    controlIndex:VELOCITY_TO_ATTACK
                      enumString:@"OFF,ON" 
                             def:DEFAULT_VELOCITY_TO_ATTACK
                        verySmallFont:NO];
    [controlArray addObject:wpOMG_1];

    [wpOMG_2 initControlWithName:@"Note->Dcy"
                    controlIndex:NOTE_NUM_TO_DECAY
                      enumString:@"OFF,ON" 
                             def:DEFAULT_NOTE_TO_DECAY
                        verySmallFont:NO];
    [controlArray addObject:wpOMG_2];
    
    [wpOMG_3 initControlWithName:@"ResetToZero"
                    controlIndex:RESET_TO_ZERO
                      enumString:@"OFF,ON" 
                             def:DEFAULT_RESET_TO_ZERO
                        verySmallFont:NO];
    [controlArray addObject:wpOMG_3];

    [wpOMG_4 initControlWithName:@"Legato Mode"
                    controlIndex:LEGATO_MODE
                      enumString:@"OFF,ON" 
                             def:DEFAULT_LEGATO_MODE
                        verySmallFont:NO];
    [controlArray addObject:wpOMG_4];

    [wpRotaryKnob_27 initControlWithName:@"PBRange" 
                            controlIndex:PITCHBEND_RANGE
                                     min:MIN_PITCHBEND_RANGE 
                                     max:MAX_PITCHBEND_RANGE 
                                     def:DEFAULT_PITCHBEND_RANGE
                              voltOctave:NO 
                          integerControl:YES];  
    [controlArray addObject:wpRotaryKnob_27];
    [wpRotaryKnob_27 setKnobImageArray:knobImages];

    // --- LFO row
    [wpOMG_5 initControlWithName:@"LFO Waveform"
                    controlIndex:LFO1_WAVEFORM
                      enumString:@"sine,usaw,dsaw,tri,square,expo,rsh,qrsh" 
                             def:DEFAULT_LEGATO_MODE
                   verySmallFont:NO];
    [controlArray addObject:wpOMG_5];
  
    [wpRotaryKnob_28 initControlWithName:@"LFO Rate" 
                            controlIndex:LFO1_RATE
                                     min:MIN_LFO_RATE 
                                     max:MAX_LFO_RATE 
                                     def:DEFAULT_LFO_RATE
                              voltOctave:NO 
                          integerControl:NO];  
    [controlArray addObject:wpRotaryKnob_28];
    [wpRotaryKnob_28 setKnobImageArray:knobImages];
    [wpRotaryKnob_28 setMiniKnob:YES]; // <--- note: only set this AFTER imageArray has been set

    [wpRotaryKnob_29 initControlWithName:@"LFO Intensity"
                            controlIndex:LFO1_AMPLITUDE
                                     min:MIN_UNIPOLAR 
                                     max:MAX_UNIPOLAR 
                                     def:DEFAULT_UNIPOLAR
                              voltOctave:NO 
                          integerControl:NO];  
    [controlArray addObject:wpRotaryKnob_29];
    [wpRotaryKnob_29 setKnobImageArray:knobImages];

    [wpOMG_6 initControlWithName:@"Op1 Dest"
                    controlIndex:LFO1_DESTINATION_OP1
                      enumString:@"none,AmpMod,Vibrato" 
                             def:DEFAULT_DX_LFO_DESTINATION
                   verySmallFont:NO];
    [controlArray addObject:wpOMG_6];
    
    [wpOMG_7 initControlWithName:@"Op2 Dest"
                    controlIndex:LFO1_DESTINATION_OP2
                      enumString:@"none,AmpMod,Vibrato" 
                             def:DEFAULT_DX_LFO_DESTINATION
                   verySmallFont:NO];
    [controlArray addObject:wpOMG_7];
    
    [wpOMG_8 initControlWithName:@"Op3 Dest"
                    controlIndex:LFO1_DESTINATION_OP3
                      enumString:@"none,AmpMod,Vibrato" 
                             def:DEFAULT_DX_LFO_DESTINATION
                   verySmallFont:NO];
    [controlArray addObject:wpOMG_8];
    
    [wpOMG_9 initControlWithName:@"Op4 Dest"
                    controlIndex:LFO1_DESTINATION_OP4
                      enumString:@"none,AmpMod,Vibrato" 
                             def:DEFAULT_DX_LFO_DESTINATION
                   verySmallFont:NO];
    [controlArray addObject:wpOMG_9];
    
 
    // --- create the background image
     NSString* path = [bundle pathForResource:@"bluemetal" ofType: @"bmp"];
    
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
	//[backgroundColor setFill];
    NSColor *backgroundPattern = [NSColor colorWithPatternImage:backImage];
    [backgroundPattern setFill];
	NSRectFill(rect);	
   
    /*
    // --- do group frame background images
    float x, y, w, h, r;
    
    // --- column 1
    // 
    // --- top knob
    NSRect knob0Rect = [wpRotaryKnob_0 frame];
    y = NSMinY(knob0Rect);
    
    // --- bottom knob
    NSRect knob3Rect = [wpRotaryKnob_3 frame];
    w = NSWidth(knob3Rect);
    
    // --- height of col    
    h = y + 1.5*NSHeight(knob3Rect) - (NSMinY(knob3Rect)) + 20; 
    
    // --- group frame rect
    NSRect groupRect1 = NSMakeRect(0, 0, w, h);
    
    // --- get the image
    NSString* path = [[[NSBundle bundleForClass:[DXSynthView class]] pathForResource:@"groupframe" ofType: @"png"] autorelease];
    NSImage* image = [[NSImage alloc] initWithContentsOfURL:[NSURL fileURLWithPath:path]];
    
    float alpha = 1.0;
    
    // --- paint it
    if(image)[image drawInRect:groupRect1 fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:alpha];
    
    // --- column 2
    // 
    // --- top knob
    NSRect knob4Rect = [wpRotaryKnob_4 frame];
    x = NSMinX(knob4Rect); 
    y = NSMinY(knob4Rect);
    
    // --- group frame rect
    NSRect groupRect2 = NSMakeRect(x, 0, w, h);
    
    // --- paint it
    if(image)[image drawInRect:groupRect2 fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:alpha];
    
    // --- column 3
    // 
    // --- top knob
    NSRect knob8Rect = [wpRotaryKnob_8 frame];
    x = NSMinX(knob8Rect); 
    y = NSMinY(knob8Rect);
    
    // --- group frame rect
    NSRect groupRect3 = NSMakeRect(x, 0, w, h);
    
    // --- paint it
    if(image)[image drawInRect:groupRect3 fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:alpha];
    
    // --- column 4
    // 
    // --- top knob
    NSRect omg0Rect = [wpOMG_0 frame];
    x = NSMinX(omg0Rect); 
    y = NSMinY(omg0Rect);
    
    // --- group frame rect
    NSRect groupRect4 = NSMakeRect(x, 0, w, h);
    
    // --- paint it
    if(image)[image drawInRect:groupRect4 fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:alpha];
    
    // --- column 5
    // 
    // --- top knob
    NSRect knob16Rect = [wpRotaryKnob_16 frame];
    x = NSMinX(knob16Rect); 
    y = NSMinY(knob16Rect);
    
    // --- group frame rect
    NSRect groupRect5 = NSMakeRect(x, 0, w, h);
    
    // --- paint it
    if(image)[image drawInRect:groupRect5 fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:alpha];
    
    // --- column 6
    // 
    // --- top knob
    NSRect knob20Rect = [wpRotaryKnob_20 frame];
    x = NSMinX(knob20Rect); 
    y = NSMinY(knob20Rect);
    
    // --- group frame rect
    NSRect groupRect6 = NSMakeRect(x, 0, w, h);
    
    // --- paint it
    if(image)[image drawInRect:groupRect6 fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:alpha];
    
    // --- voice row
    // --- last knob
    NSRect omg1Rect = [wpOMG_1 frame];
    float x1 = NSMinX(omg1Rect); 
    x1 -= 8;
    
    NSRect knob26Rect = [wpRotaryKnob_26 frame];
    x = NSMinX(knob26Rect); 
    y = NSMinY(knob26Rect);
 //   w = NSWidth(knob26Rect);
    h = NSHeight(knob26Rect);

    w = 5 + x + NSWidth(knob26Rect) - x1;
    
    // --- group frame rect
 //   NSRect groupRect7 = NSMakeRect(x1, y-8, w, h+35);
    NSRect groupRect7 = NSMakeRect(x1, y-100, w, h+132);
    
    // --- paint it
    if(image)[image drawInRect:groupRect7 fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:alpha];
    
 */
    
    
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
