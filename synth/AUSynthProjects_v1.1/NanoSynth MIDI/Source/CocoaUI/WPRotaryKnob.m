//
//  WPRotaryKnob.m
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


#import "WPRotaryKnob.h"

// --- number of images in a set
#define KNOB_COUNT 128

float fastlog2 (float x)
{
    union { float f; unsigned int i; } vx = { x };
    union { unsigned int i; float f; } mx = { (vx.i & 0x007FFFFF) | 0x3f000000 };
    float y = vx.i;
    y *= 1.1920928955078125e-7f;
    
    return y - 124.22551499f
    - 1.498030302f * mx.f 
    - 1.72587999f / (0.3520887068f + mx.f);
}

float fastpow2 (float p)
{
    float offset = (p < 0) ? 1.0f : 0.0f;
    float clipp = (p < -126) ? -126.0f : p;
    int w = clipp;
    float z = clipp - w + offset;
    union { unsigned int i; float f; } v = { (unsigned int)( (1 << 23) * (clipp + 121.2740575f + 27.7280233f / (4.84252568f - z) - 1.49012907f * z) ) };
    
    return v.f;
}


/************************************************************************************************************/
/* NOTE: It is important to rename ALL ui classes when using the XCode Audio Unit with Cocoa View template	*/
/*		 Cocoa has a flat namespace, and if you use the default filenames, it is possible that you will		*/
/*		 get a namespace collision with classes from the cocoa view of a previously loaded audio unit.		*/
/*		 We recommend that you use a unique prefix that includes the manufacturer name and unit name on		*/
/*		 all objective-C source files. You may use an underscore in your name, but please refrain from		*/
/*		 starting your class name with an undescore as these names are reserved for Apple.					*/
/************************************************************************************************************/
// --- Implememnt the WPEditBox stuff to enable editing and messaging the target
@implementation WPEditBoxNS

-(bool)isEditing
{
    return editing;
}
- (BOOL) acceptsFirstResponder 
{
    return YES; 
}
- (void)textDidBeginEditing:(NSNotification *)notification
{
    editing = true;
    if(target)
        [target performSelector:editStartedAction withObject:self];
}

- (void)textDidEndEditing:(NSNotification *)notification
{
    if(target)
        [target performSelector:editEndedAction withObject:self];
    editing = false;
}

-(void)setTarget:(id)control editStartedAction:(SEL)sel1 
                               editEndedAction:(SEL)sel2
{
    target = control;
    editStartedAction = sel1;
    editEndedAction = sel2;
}
@end

// --- cell implementation; nothing special to do
@implementation WPRotaryKnobCell
@end

// --- formatter for integer controls
@interface OnlyIntegerValueFormatter : NSNumberFormatter
@end

@implementation OnlyIntegerValueFormatter

- (BOOL)isPartialStringValid:(NSString*)partialString newEditingString:(NSString**)newString errorDescription:(NSString**)error
{
    if([partialString length] == 0) {
        return YES;
    }
    
    NSScanner* scanner = [NSScanner scannerWithString:partialString];
    
    if(!([scanner scanInt:0] && [scanner isAtEnd])) {
        NSBeep();
        return NO;
    }
    
    return YES;
}
@end

// --- Rotary Knob Implementation
@implementation WPRotaryKnobNS

// --- helper for edit box numbers only
-(BOOL)isNumericTextField:(NSTextField *)textField
{
    
    //Returns TRUE if numeric
    BOOL valid;
    NSCharacterSet *okchars = [NSCharacterSet characterSetWithCharactersInString:@"0123456789-."];
    NSCharacterSet *stringsFromField = [NSCharacterSet characterSetWithCharactersInString:[textField stringValue]];
    valid = [okchars isSupersetOfSet:stringsFromField];
    return valid;    
}
-(void)setKnobImageArray:(NSMutableArray*) imageArray
{
    knobImages = imageArray;
}

// --- designated initializer for NSControl
- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self == nil)
	return nil;
    
    // --- controlID default is -1 as flag that it is not set
    controlID = -1;
 
    // --- def scale is unity
    knobScale = 1.0;
    
    // --- default is linear
    voltPerOctaveControl = false;
    integerControl = false;
    hideEditBox = false;
     
    // --- limits and default - user values
    minValue = -1.0;
    maxValue = 1.0;
    defaultValue = -1.0;
    
    // --- init to defaults
    currentPosition.width = 0.0;
    currentPosition.height = -1.0;
    
    mouseDownPosition.width = currentPosition.width;
    mouseDownPosition.height = currentPosition.height;
    
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
    knobTextField =  [[NSTextField alloc] initWithFrame:textRect];
    
    // --- make transparent background
    [knobTextField setBezeled:NO];
    [knobTextField setEditable:NO];
    [knobTextField setDrawsBackground:NO];
    [knobTextField setAlignment:NSCenterTextAlignment];
    [knobTextField setFont:[NSFont systemFontOfSize:10]];
 //   [knobTextField setTextColor:[NSColor blackColor]];
    [knobTextField setTextColor:[NSColor lightGrayColor]];
  //  [knobTextField setTextColor:[NSColor whiteColor]];
 //   [knobTextField setTextColor:[NSColor blackColor]];

    // --- add it
    [self addSubview:knobTextField];
    
    // --- the buddy-Edit box control
    float editBoxWidth = w/1.8;
    NSRect editRect = NSMakeRect(editBoxWidth/2.4, 0, editBoxWidth, 13);
    
    // --- create the text field (edit box)
    controlEditBox =  [[WPEditBoxNS alloc] initWithFrame:editRect];
    [controlEditBox setBezeled:NO];
    [controlEditBox setEditable:YES];
    [controlEditBox setDrawsBackground:YES];
    [controlEditBox setAlignment:NSCenterTextAlignment];
    [controlEditBox setFont:[NSFont systemFontOfSize:10]];
    [controlEditBox setAlphaValue:0.999]; // <-- prevents "holes" in AbletonLive

    // --- I like transparent with lightGrey text as default
    [controlEditBox setBackgroundColor:[NSColor clearColor]];
    //  [controlEditBox setBackgroundColor:[NSColor blackColor]];
    
    [controlEditBox setTextColor:[NSColor lightGrayColor]];
    //   [controlEditBox setTextColor:[NSColor greenColor]];
    //[controlEditBox setTextColor:[NSColor blackColor]];
    //   [controlEditBox setTextColor:[NSColor whiteColor]];

    [controlEditBox setSelectable:YES];
    [controlEditBox setEnabled:YES];
   
    // --- setup messaging for end-edit 
    SEL selector1 = @selector(editBoxStartEdit:);
    SEL selector2 = @selector(editBoxEndEdit:);
    [controlEditBox setTarget:self 
            editStartedAction:selector1 
              editEndedAction:selector2];
        
    // --- add it
    [self addSubview:controlEditBox];
    
    return self;
}

- (void)dealloc
{
    [super dealloc];
}

// --- our cell! (must have this)
+ (Class) cellClass
{
    return [WPRotaryKnobCell class];
}

- (BOOL) acceptsFirstResponder{return YES;}
- (BOOL) canBecomeKeyWindow{return YES;}
- (BOOL)isOpaque{return NO;}

// --- send the action to the target as connected in interface builder
- (void)sendActionToTarget 
{
    if([self target] && [self action]) 
    {
        [self sendAction:[self action] to:[self target]];
    }
}

// --- initialize user range of values; if not called defaults 
//     to range =  [-1..+1] default = -1
- (void)initControlWithName:(NSString*)name 
               controlIndex:(int)index 
                        min:(float)minV 
                        max:(float)maxV 
                        def:(float)defaultV 
                 voltOctave:(bool)vpo 
             integerControl:(bool)intControl;
{
    controlName = name;
    controlID = index;
    minValue = minV;
    maxValue = maxV;
    defaultValue = defaultV;
    voltPerOctaveControl = vpo;
    
    // --- integer based
    if(intControl)
    {
        OnlyIntegerValueFormatter *formatter = [[OnlyIntegerValueFormatter alloc] init];
        [controlEditBox setFormatter:formatter];
    }
    
    // -- set control name
    [knobTextField setStringValue:controlName];
    
    // --- applyto knob and redraw
    [self setControlValue:defaultV];
}




// --- get the scale
- (float)scale
{
    return knobScale;
}
// --- set the scale
- (void)setKnobScale:(float)scale
{
    knobScale = scale;
}

// --- transparent background
- (void)setTransparentEditBoxBackground:(bool)transparent
{
    if(transparent)
        [controlEditBox setBackgroundColor:[NSColor clearColor]];
    else
        [controlEditBox setBackgroundColor:[NSColor blackColor]];
}

// -- for half-sized controls
- (void)setMiniKnob:(bool)mini
{
    // --- save state variable
    miniKnob = mini;
    float x, y, w, h;
    NSRect bounds = [self bounds];
    x = NSMinX(bounds);
    y = NSMinY(bounds);
    w = NSWidth(bounds);
    h = NSHeight(bounds);
    
    if(miniKnob)
    {
        // --- relocate the box to new frame
        NSImage* image = [knobImages objectAtIndex:0];
        
        // --- should never have this happen
        if(!image) return;
        
        // --- get size of knob image
        NSSize imageSize = [image size];
        
        float editBoxWidth = w/1.8;
        NSRect editRect = NSMakeRect(imageSize.width/2.0, 3, editBoxWidth, 13);
        [controlEditBox setFrame:editRect];
    }
    else
    {
        float editBoxWidth = w/1.8;
        NSRect editRect = NSMakeRect(editBoxWidth/2.4, 0, editBoxWidth, 13);
        [controlEditBox setFrame:editRect];
    }
}

- (int)controlID
{
    return controlID;
}


// --- helpers for log/VPO knobs
float calcValueExp(float low, float high, float userValue)
{
	float octaves = fastlog2(high/low);
	if(low == 0)
		return userValue;
	
	// exp control
	return low*fastpow2(userValue*octaves);
}
float calcInverseValueExp(float low, float high, float userValue)
{
	float octaves = fastlog2(high/low);
	if(low == 0)
		return userValue;
	
	return fastlog2(userValue/low)/octaves;
}

// --- cooked value
- (float)controlValue // user values
{
    float knobValue = 0.0;
    
    // --- convert our position [-1..+1] to cooked value
    if(voltPerOctaveControl)
    {
        // --- VPO; first convert to unipolar
        float uniValue = 0.5*currentPosition.height + 0.5;
        knobValue = calcValueExp(minValue, maxValue, uniValue);
    }
    else
    {
        // --- linear
        float raw = 0.5*currentPosition.height + 0.5;
        knobValue = (maxValue - minValue)*raw + minValue;
    }
    
    return knobValue;
}
// --- set the control's position and value
- (void)setControlValue:(float)userValue // user values
{
    
    float rawValue = 0.0;
    if(voltPerOctaveControl)
    {
        // --- convert to VPO raw [0..1]
        rawValue = calcInverseValueExp(minValue, maxValue, userValue);
    }
    else
    {
        // --- convert to raw [0..1]
        float fDiff = maxValue - minValue;
        rawValue = (userValue - minValue)/fDiff;
    }
    
    // --- convert to bipolar [-1..+1]
    rawValue = 2.0*rawValue - 1.0;
	
    // --- set on our Y-axis variable
    currentPosition.height = rawValue;
    
    // --- force redraw
	[self setNeedsDisplay:YES];
}

// --- for fc and other frequency based knobs
- (void)setVoltPerOctaveControl:(bool)vpo
{
    // --- save current value
    float value = [self controlValue];
    
    // --- make the switch
    voltPerOctaveControl = vpo;
    
    // --- move knob
    [self setControlValue:value];
}

// --- action methods for buddy edit box
- (void)editBoxStartEdit:(id)sender
{
    // could add customization during edit
    
}

- (void)editBoxEndEdit:(id)sender
{
    if(![self isNumericTextField:controlEditBox])
        return;
    
    NSString* value = [controlEditBox stringValue];
    double newKnobValue = [value doubleValue];
    
    if(newKnobValue != [self controlValue])
    {
        if(newKnobValue > maxValue) newKnobValue = maxValue;
        if(newKnobValue < minValue) newKnobValue = minValue;
        
       	float fDiff = maxValue - minValue;
        float fRawValue = (newKnobValue - minValue)/fDiff;
        
        fRawValue = 2.0*fRawValue - 1.0;
        
        currentPosition.height = fRawValue;
        
        // --- redraw (UNCOMMENT for non-AU evenent listener applications!)
        //[self setNeedsDisplay:YES];
        
        // --- send action to receiver
        [self sendActionToTarget];
    }
}

// --- get the mouse location offset (not used in my synth, but here if you want it)
- (CGSize)offset
{
    return CGSizeMake(currentPosition.width * knobScale, currentPosition.height * knobScale);
}

// --- store the mouse down location
- (void)setMouseDownOffset:(CGSize)location
{
    location = CGSizeMake(location.width / knobScale, location.height / knobScale);
    if (!CGSizeEqualToSize(mouseDownLocation, location)) 
    {
        mouseDownLocation = location;
    }
}

// --- while being dragged
- (void)setOffsetFromPoint:(NSPoint)point
{
    float radius;
    CGSize offset;
    NSRect bounds;
    
    bounds = [self bounds];
    offset.width = (point.x - NSMidX(bounds)) / (NSWidth(bounds) / 2);
    offset.height = (point.y - NSMidY(bounds)) / (NSHeight(bounds) / 2);
    radius = sqrt(offset.width * offset.width + offset.height * offset.height);
    if (radius > 1) {
        offset.width /= radius;
        offset.height /= radius;
    }
    
    // --- speed factor
    float speedFactor = miniKnob ? 1.8 : 2.2;
    
    offset.width = mouseDownPosition.width + speedFactor*(offset.width - mouseDownLocation.width);	
    offset.height = mouseDownPosition.height + speedFactor*(offset.height - mouseDownLocation.height);
 
    // --- bound
    if(offset.width > 1.0) offset.width = 1.0;
    if(offset.width < -1.0) offset.width = -1.0;
    if(offset.height > 1.0) offset.height = 1.0;
    if(offset.height < -1.0) offset.height = -1.0;
    
    if (!CGSizeEqualToSize(currentPosition, offset)) 
    {        
        // --- store the current knob position
        currentPosition = offset;
        
        // --- redraw (UNCOMMENT for non-AU evenent listener applications!)
        //[self setNeedsDisplay:YES];
 
        // --- send action to receiver
        [self sendActionToTarget];
    }
}

// --- get the location from a point
- (void)setMouseDownLocationFromPoint:(NSPoint)point
{
    float radius;
    CGSize offset;
    NSRect bounds;
    
    bounds = [self bounds];
    offset.width = (point.x - NSMidX(bounds)) / (NSWidth(bounds) / 2);
    offset.height = (point.y - NSMidY(bounds)) / (NSHeight(bounds) / 2);
    radius = sqrt(offset.width * offset.width + offset.height * offset.height);
    if (radius > 1) {
        offset.width /= radius;
        offset.height /= radius;
    }
    if (!CGSizeEqualToSize(mouseDownLocation, offset)) {
        mouseDownLocation = offset;
    }
}

// --- handler for mouse down messages
- (void)mouseDown:(NSEvent *)event
{
    NSPoint point;
    point = [self convertPoint:[event locationInWindow] fromView:nil];
    
    NSRect bounds;
    float x, y, w, h;
    
    bounds = [self bounds];
    x = NSMinX(bounds);
    y = NSMinY(bounds);
    w = NSWidth(bounds);
    h = NSHeight(bounds);
    float originX = x + w/2.0;
    float originY = y + h/2.0;
    
    // --- get the image
    NSImage* image = [knobImages objectAtIndex:0];
    
    // --- get size of knob image
    NSSize imageSize = [image size];

    if(miniKnob)
    {
        // --- calculate location within our bounds
        NSRect imageRect = NSMakeRect(0, 
                                      originY - imageSize.height/2.0 + 8, 
                                      imageSize.width/2.0, 
                                      imageSize.height/2.0);
        
        if(!NSPointInRect(point, imageRect))
           return;
    }
    else
    {
        NSRect imageRect = NSMakeRect(originX - imageSize.width/2.0, 
                                      originY - imageSize.height/2.0, 
                                      imageSize.width, 
                                      imageSize.height);
        
        if(!NSPointInRect(point, imageRect))
            return;
    }
  
    // --- save knob's bosition
    mouseDownPosition.width = currentPosition.width;
    mouseDownPosition.height = currentPosition.height; 
    
    //--- save mousedown location
    [self setMouseDownLocationFromPoint:point];
}

// --- handler for mouse down messages
- (void)mouseUp:(NSEvent *)event
{    
    // --- this is for Ableton Live; click on control to automate
    // --- send action to receiver
    [self sendActionToTarget];
}

// --- handle mouse move messages
- (void)mouseDragged:(NSEvent *)event
{
    NSRect bounds = [self bounds];
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    
    // --- is mouse in view rect?
    if ([self mouse:point inRect:bounds])
    {
        // --- set the knob position/value
        [self setOffsetFromPoint:point];
    }
}

// --- the drawing function
- (void)drawRect:(NSRect)rect
{
    NSRect bounds;
    float x, y, w, h, r;
        
    bounds = [self bounds];
    x = NSMinX(bounds);
    y = NSMinY(bounds);
    w = NSWidth(bounds);
    h = NSHeight(bounds);
    r = MIN(w / 2, h / 2);
    
    float originX = x + w/2.0;
    float originY = y + h/2.0;
    
    // --- control's vaue as text for edit box
    if(![controlEditBox isEditing])
    {
        NSString* valueString = [NSString stringWithFormat:@"%.2f", [self controlValue]];
        [controlEditBox setStringValue:valueString];
    }
    
    // --- figure out which knob to show based on Y-axis value: _offset.height
    NSUInteger arrayIndex = (NSUInteger)((0.5*currentPosition.height + 0.5) * 127.0);
    
    // --- get the image
    NSImage* image = [knobImages objectAtIndex:arrayIndex];

    // --- draw it
    if(image)
    {
        // --- get size of knob image
        NSSize imageSize = [image size];
        
        if(miniKnob)
        {
            // --- calculate location within our bounds
            NSRect imageRect = NSMakeRect(0, //originX - imageSize.width/4.0 
                                          originY - imageSize.height/2.0 + 7,
                                          imageSize.width/2.0, 
                                          imageSize.height/2.0);
            // --- draw the knob image
            [image drawInRect:imageRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
        }
        else
        {
            // --- calculate location within our bounds
            NSRect imageRect = NSMakeRect(originX - imageSize.width/2.0, 
                                          originY - imageSize.height/2.0, 
                                          imageSize.width, 
                                          imageSize.height);
            // --- draw the knob image
            [image drawInRect:imageRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];
        }
    }
}

@end
