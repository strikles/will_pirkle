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


@interface MiniSynthView : NSView
{
    // --- rotary knob groups
    //
    // --- column 1
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_0;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_1;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_2;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_3;
    
    // --- column 2
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_4;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_5;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_6;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_7;
    
    // --- column 3
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_8;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_9;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_10;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_11; // unused
    
    // --- column 4
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_12;// unused
    IBOutlet WPOptionMenuGroupMS* wpOMG_0;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_13;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_14;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_15; // unused
    
    // --- column 5
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_16;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_17;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_18;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_19;
    
    // --- column 6
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_20;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_21;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_22; // unused
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_23; // unused
    
    // --- voice row
    IBOutlet WPOptionMenuGroupMS* wpOMG_1;
    IBOutlet WPOptionMenuGroupMS* wpOMG_2;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_24; // unused
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_25; // unused
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_26; // unused
    
    // --- globals row
    IBOutlet WPOptionMenuGroupMS* wpOMG_3;
    IBOutlet WPOptionMenuGroupMS* wpOMG_4;
    IBOutlet WPOptionMenuGroupMS* wpOMG_5;
    IBOutlet WPOptionMenuGroupMS* wpOMG_6;
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_27; // unused
    
    // --- delay FX controls
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_28; // delay time
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_29; // delay feedback
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_30; // delay ratio
    IBOutlet WPRotaryKnobMS* wpRotaryKnob_31; // delay mix
    IBOutlet WPOptionMenuGroupMS* wpOMG_7; // delay mode
 
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

//@property (nonatomic, copy) WPRotaryKnobMS* wpRotaryKnob_0;

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



