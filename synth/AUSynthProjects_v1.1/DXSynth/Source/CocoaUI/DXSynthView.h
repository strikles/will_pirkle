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


@interface DXSynthView : NSView
{
    // --- rotary knob groups
    //
    // --- row 1
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_0;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_1;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_2;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_3;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_4;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_5;
    
    // --- row 2
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_6;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_7;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_8;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_9;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_10;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_11;
    
    // --- row 3
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_12;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_13;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_14;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_15;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_16;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_17;
    
    // --- row 4
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_18;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_19;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_20;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_21;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_22;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_23;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_24;
    
    // --- voice row
    IBOutlet WPOptionMenuGroupDX* wpOMG_0;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_25;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_26;
    
    // --- globals row
    IBOutlet WPOptionMenuGroupDX* wpOMG_1;
    IBOutlet WPOptionMenuGroupDX* wpOMG_2;
    IBOutlet WPOptionMenuGroupDX* wpOMG_3;
    IBOutlet WPOptionMenuGroupDX* wpOMG_4;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_27;
    
    // --- LFO row
    IBOutlet WPOptionMenuGroupDX* wpOMG_5;
    IBOutlet WPOptionMenuGroupDX* wpOMG_6;
    IBOutlet WPOptionMenuGroupDX* wpOMG_7;
    IBOutlet WPOptionMenuGroupDX* wpOMG_8;
    IBOutlet WPOptionMenuGroupDX* wpOMG_9;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_28;
    IBOutlet WPRotaryKnobDX* wpRotaryKnob_29;
    
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

- (void)setAU:(AudioUnit)inAU;

- (IBAction)WPRotaryKnobChanged:(id)sender;
- (IBAction)WPOptionMenuItemChanged:(id)sender;

- (void)synchronizeUIWithParameterValues;
- (void)addListeners;
- (void)removeListeners;

- (void)eventListener:(void *) inObject event:(const AudioUnitEvent *)inEvent value:(Float32)inValue;
@end



