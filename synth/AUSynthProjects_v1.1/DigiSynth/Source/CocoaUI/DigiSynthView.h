//
//  DXSynthView.h
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

#import <Cocoa/Cocoa.h>
#import <AudioUnit/AudioUnit.h>
#import <AudioToolbox/AudioToolbox.h>

#import "WPRotaryKnob.h"
#import "WPOptionMenuGroup.h"

/************************************************************************************************************/
/* NOTE: It is important to rename ALL ui classes when using the XCode Audio Unit with Cocoa View template	*/
/*		 Cocoa has a flat namespace, and if you use the default filenames, it is possible that you will		*/
/*		 get a namespace collision with classes from the cocoa view of a previously loaded audio unit.		*/
/*		 We recommend that you use a unique prefix that includes the manufacturer name and unit name on		*/
/*		 all objective-C source files. You may use an underscore in your name, but please refrain from		*/
/*		 starting your class name with an undescore as these names are reserved for Apple.					*/
/************************************************************************************************************/


@interface DigiSynthView : NSView
{
    // --- rotary knob groups
    //
    // --- rotary knob groups
    //
    // --- column 1
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_0;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_1;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_2;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_3;
    
    // --- column 2
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_4;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_5;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_6;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_7;
    
    // --- column 3
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_8;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_9;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_10;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_11; // unused
    
    // --- column 4
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_12;// unused
    IBOutlet WPOptionMenuGroupDS* wpOMG_0;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_13;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_14;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_15; // unused
    
    // --- column 5
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_16;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_17;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_18;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_19;
    
    // --- column 6
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_20;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_21;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_22; // unused
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_23; // unused
    
    // --- voice row
    IBOutlet WPOptionMenuGroupDS* wpOMG_1;
    IBOutlet WPOptionMenuGroupDS* wpOMG_2;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_24; // unused
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_25; // unused
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_26; // unused
    
    // --- globals row
    IBOutlet WPOptionMenuGroupDS* wpOMG_3;
    IBOutlet WPOptionMenuGroupDS* wpOMG_4;
    IBOutlet WPOptionMenuGroupDS* wpOMG_5;
    IBOutlet WPOptionMenuGroupDS* wpOMG_6;
    IBOutlet WPRotaryKnobDS* wpRotaryKnob_27; // unused
    
    // --- DigiSynth additions
    IBOutlet WPOptionMenuGroupDS* wpOMG_7;
    // --- array for controls
    NSMutableArray* controlArray;
    NSMutableArray* knobImages;
    
    // --- AU members
    AudioUnit 				buddyAU; // the AU we connect to
	AUEventListenerRef		AUEventListener;
	
    // --- a background color
	NSColor*                backgroundColor;	// the background color (pattern) of the view
    NSImage* backImage;
}

- (id)getControlWithIndex:(int)index;

#pragma mark ____ PUBLIC FUNCTIONS ____
- (void)setAU:(AudioUnit)inAU;

#pragma mark ____ INTERFACE ACTIONS ____

- (IBAction)WPRotaryKnobChanged:(id)sender;
- (IBAction)WPOptionMenuItemChanged:(id)sender;

#pragma mark ____ PRIVATE FUNCTIONS
- (void)synchronizeUIWithParameterValues;
- (void)addListeners;
- (void)removeListeners;

#pragma mark ____ LISTENER CALLBACK DISPATCHEE ____
- (void)eventListener:(void *) inObject event:(const AudioUnitEvent *)inEvent value:(Float32)inValue;
@end



