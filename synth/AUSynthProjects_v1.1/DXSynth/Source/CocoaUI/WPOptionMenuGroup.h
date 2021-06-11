//
//  WPOptionMenuGroup.h
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
#import <AppKit/AppKit.h>
@interface WPPopUpButtonCellDX : NSPopUpButtonCell 
{
    bool menuShowing;
    bool verySmallFont;
}
-(void)setVerySmallFont:(bool)smallFont;
@end

@interface WPOptionMenuGroupDX : NSControl
{
    // --- control name
    NSTextField* controlTextField;
    
    // --- name string
    NSString* controlName;
    
    // --- string of substrings, separated by commas eg: "LPF,HPF,BPF"
    NSString* enumString;
   
    // --- the ControlID for this group (optional, for WP Synth stuff)
    int controlID;
    
    // --- current selection
    int selectedIndex;

    // --- our pop-up menu
    NSPopUpButton* popUpButton;    
}

// --- this sets up the value range and init value
- (void)initControlWithName:(NSString*)name 
               controlIndex:(int)index 
                 enumString:(NSString*)enumStr 
                        def:(float)defaultV 
              verySmallFont:(bool)smallFont;

// --- action for menu buddy
- (void)menuItemChanged:(id)sender;

// --- get the current selection
- (float)currentSelection;

// --- set the current selection; note this function has the 
//     same name in the WPKnobControl and therefore is used
//     in a "double" way (see the view object) In this case
//     the userValue is cast to an (int) and is the selection
- (void)setControlValue:(float)userValue; // user values

- (int)controlID;

@end
