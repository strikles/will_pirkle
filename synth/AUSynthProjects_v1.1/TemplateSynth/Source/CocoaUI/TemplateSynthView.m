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

#import "TemplateSynthView.h"
#import "SynthParamLimits.h"
#define KNOB_COUNT 128

#pragma mark ____ LISTENER CALLBACK DISPATCHER ____

// This listener responds to parameter changes, gestures, and property notifications
void EventListenerDispatcher (void *inRefCon, void *inObject, const AudioUnitEvent *inEvent, UInt64 inHostTime, Float32 inValue)
{
	TemplateSynthView *SELF = (TemplateSynthView *)inRefCon;
	[SELF eventListener:inObject event: inEvent value: inValue];
}

@implementation TemplateSynthView

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
        NSString* path = [[bundle pathForResource:file ofType: @"png"] autorelease];
        if(path)
            image = [[NSImage alloc] initWithContentsOfURL:[NSURL fileURLWithPath:path]];
        if(image)
            [knobImages addObject:image];
    }
    
    // --- initialize the knobs
    //
    controlArray = [[NSMutableArray alloc] init];
    
    // --- SEE BOOK - init controls here
    
    
    // --- background PNG
    NSString* path = [[[NSBundle bundleForClass:[TemplateSynthView class]] pathForResource:@"medGreyBrushed" ofType: @"bmp"] autorelease];
    backImage = [[NSImage alloc] initWithContentsOfURL:[NSURL fileURLWithPath:path]];
    
//    NSLog(@"Hello!");
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
    
    // --- SEE BOOK or sample code
    
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
        // --- create the event listener and tell it the name of our Dispatcher function
        //     EventListenerDispatcher
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
    
    // --- parameters 0 -> NUMBER_OF_SYNTH_PARAMETERS-1
    //     notice the way additional params are added using mParameterID
    for(int i=0; i<NUMBER_OF_SYNTH_PARAMETERS; i++)
    {
        // --- change the parameterID for subsequent controls
        parameter.mParameterID = i;
        //AudioUnitGetParameter(buddyAU, i, kAudioUnitScope_Global, 0, &paramValue);
        if(AudioUnitGetParameter(buddyAU, i, kAudioUnitScope_Global, 0, &paramValue) != noErr)
            return;

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
