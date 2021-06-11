//
//  WPOptionMenuGroup.m
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


#import "WPOptionMenuGroup.h"

// --- our custom Cell 
@interface WPOptionMenuCell : NSActionCell 
{
    // --- nothing special for this cell
}
@end

// --- cell implementation; nothing special to do
@implementation WPOptionMenuCell
@end

// --- custom button cell
@implementation WPPopUpButtonCellMS

// --- this attaches the menu with the item names and changes the font color
//     to black
- (void)attachPopUpWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    NSArray *itemArray = [self itemArray];
    int fontSize = verySmallFont ? 8 : 10;

    NSDictionary *attributes = [NSDictionary
                                dictionaryWithObjectsAndKeys:
                                [NSColor blackColor], NSForegroundColorAttributeName,
                                [NSFont systemFontOfSize:fontSize],
                                NSFontAttributeName, nil];
    
    for(int i = 0; i < [itemArray count]; i++) 
    {
        NSMenuItem *item = [itemArray objectAtIndex:i];
  
        NSAttributedString *as = [[NSAttributedString alloc] 
                                  initWithString:[item title]
                                  attributes:attributes];
        
        [item setAttributedTitle:as];
    }
    
    menuShowing = true;
}

// --- notify target
- (void)dismissPopUp
{
    menuShowing = false;
    [_target performSelector:_action withObject:self];

}

// --- the drawRect equivalent; this changes the font color to another color
//     for normal drawing when menu is not showing
- (void)drawBezelWithFrame:(NSRect)frame inView:(NSView *)controlView
{
    if(!menuShowing)
    {
        NSArray *itemArray = [self itemArray];
        int fontSize = verySmallFont ? 8 : 10;
        
        NSDictionary *attributes = [NSDictionary
                                    dictionaryWithObjectsAndKeys:
                                    [NSColor greenColor], NSForegroundColorAttributeName,
                                    [NSFont systemFontOfSize:fontSize],
                                    NSFontAttributeName, nil];
        
        for(int i = 0; i < [itemArray count]; i++) 
        {
            NSMenuItem *item = [itemArray objectAtIndex:i];
             
            NSAttributedString *as = [[NSAttributedString alloc] 
                                      initWithString:[item title]
                                      attributes:attributes];
            
            [item setAttributedTitle:as];
        }
    }
}

// --- currently not used
-(void)setVerySmallFont:(bool)smallFont
{
    verySmallFont = smallFont;
}

@end


@implementation WPOptionMenuGroupMS

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) 
    {
        NSRect bounds;
        float x, y, w, h, r;
        
        bounds = [self bounds];
        x = NSMinX(bounds);
        y = NSMinY(bounds);
        w = NSWidth(bounds);
        h = NSHeight(bounds);
        r = MIN(w / 2, h / 2);
        
        // --- text field
        controlName = @"control";
        
        NSRect textRect = NSMakeRect(0, h-16, w, 12);
        
        // --- create the text field
        controlTextField =  [[NSTextField alloc] initWithFrame:textRect];
        // --- make transparent background
        [controlTextField setBezeled:NO];
        [controlTextField setEditable:NO];
        [controlTextField setDrawsBackground:NO];
        [controlTextField setAlignment:NSCenterTextAlignment];
        [controlTextField setFont:[NSFont systemFontOfSize:10]];
    //    [controlTextField setTextColor:[NSColor blackColor]];
        [controlTextField setTextColor:[NSColor lightGrayColor]];
   //     [controlTextField setTextColor:[NSColor whiteColor]];

        [controlTextField setStringValue:controlName];
        
        // --- add it
        [self addSubview:controlTextField];
        
        // --- popup manu
        NSRect popUpRect = NSMakeRect(4, 0, w-4, 20);

        // --- alloc/init
        popUpButton = [[NSPopUpButton alloc] initWithFrame:popUpRect pullsDown:NO];
        
        // --- set attributes
        [popUpButton setFont:[NSFont systemFontOfSize:10]];
        [popUpButton setTransparent:YES];
        
        // --- make and give it a cell
        WPPopUpButtonCellMS* cell = [[WPPopUpButtonCellMS alloc] init];
        [cell setFont:[NSFont systemFontOfSize:10]];
       
        // --- setup action messaging
        SEL selector = @selector(menuItemChanged:);
        [cell setTarget:self];
        [cell setAction:selector];
        
        // --- set the cell
        [popUpButton setCell:cell];
        
        // --- add it
        [self addSubview:popUpButton];
         
        selectedIndex = -1;
    }
    
    return self;
}
// --- our cell! (must have this)
+ (Class) cellClass
{
    return [WPOptionMenuCell class];
}

- (int)controlID
{
    return controlID;
}

- (float)currentSelection
{
    return (float)selectedIndex;
}
 
// --- cell notifies us with action
- (void)menuItemChanged:(id)sender
{
    // --- get the index
    selectedIndex = [popUpButton indexOfSelectedItem];
    
    // --- notify target (the owning view( with action
    if ([self target] && [self action]) 
    {
        [self sendAction:[self action] to:[self target]];
    }
}

// --- set the selected item
- (void)setControlValue:(float)userValue // user values
{
    [popUpButton selectItemAtIndex:(NSInteger)userValue];
}

// --- setup control
- (void)initControlWithName:(NSString*)name 
               controlIndex:(int)index
                 enumString:(NSString*)enumStr 
                        def:(float)defaultV 
              verySmallFont:(bool)smallFont
{
    controlName = name;
    controlID = index;
    
    // --- load up the popup button
    NSArray* strings = [enumStr componentsSeparatedByString:@","];

    for(NSString* item in strings)
    {
        [popUpButton addItemWithTitle:item];
    }
    
    [[popUpButton cell] setVerySmallFont:smallFont];
    [controlTextField setStringValue:controlName];
    
    // --- select first item
    [popUpButton selectItemAtIndex:defaultV];
}

- (void)drawRect:(NSRect)dirtyRect
{
    // --- draw the black background for menu items
    NSRect bounds = [popUpButton bounds];
    float x, y, w, h;
    x = NSMinX(bounds);
    y = NSMinY(bounds);
    w = NSWidth(bounds);
    h = NSHeight(bounds);

    NSRect rect = NSMakeRect(x+10, y+2, w-15, h-2);
    [[NSColor blackColor] setFill];
    NSRectFill(rect);
}

@end
