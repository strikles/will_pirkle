//
//  WPTrackPad.m
//  DigiSynth
//
//  Created by William Pirkle on 1/26/14.
//  Copyright (c) 2014 University of Miami. All rights reserved.
//

#import "WPTrackPadVS.h"

// --- our custom Cell
@interface WPTrackPadCell : NSActionCell
{
    // --- nothing special for this cell
}
@end

@implementation WPTrackPadCell
@end

@implementation WPTrackPadAS

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        NSRect bounds;
        float w, h;
        bounds = [self bounds];
        w = NSWidth(bounds);
        h = NSHeight(bounds);
        
        // --- center the puck
        mousePoint.x = w/2.0;
        mousePoint.y = h/2.0;
    }
    
    return self;
}

// --- our cell! (must have this)
+ (Class) cellClass
{
    return [WPTrackPadCell class];
}

// --- store the mouse down location
- (void)setMouseDownOffset:(CGSize)location
{
    location = CGSizeMake(location.width / knobScale, location.height / knobScale);
    if (!CGSizeEqualToSize(mouseDownLocation, location)) {
        mouseDownLocation = location;
    }
}

- (BOOL)isOpaque
{
    return NO;
}

// --- send the action to the target as connected in interface builder
- (void)sendActionToTarget
{
    if ([self target] && [self action])
    {
        [self sendAction:[self action] to:[self target]];
    }
}

-(void)getControlCoordsX:(float*)x Y:(float*)y
{
    *x = currentPosition.width;
    *y = currentPosition.height;
}

// -- while being dragged
- (void)setOffsetFromPoint:(NSPoint)point
{
    CGSize offset;
    NSRect bounds;
    
    bounds = [self bounds];
    offset.width = (point.x - NSMidX(bounds)) / (NSWidth(bounds) / 2);
    offset.height = (point.y - NSMidY(bounds)) / (NSHeight(bounds) / 2);
    
    // --- so that the corners can get hit
    offset.width *= 1.2;
    offset.height *= 1.2;
    
    // --- bound
    if(offset.width > 1.0) offset.width = 1.0;
    if(offset.width < -1.0) offset.width = -1.0;
    if(offset.height > 1.0) offset.height = 1.0;
    if(offset.height < -1.0) offset.height = -1.0;
    
    if (!CGSizeEqualToSize(currentPosition, offset))
    {
        // --- store the current position
        currentPosition = offset;
        
        // --- redraw
        [self setNeedsDisplay:YES];
        
        // --- send action to receiver
        [self sendActionToTarget];
    }
}

// --- get the location from a point
- (void)setMouseDownLocationFromPoint:(NSPoint)point
{
    CGSize offset;
    NSRect bounds;
    
    bounds = [self bounds];
    offset.width = (point.x - NSMidX(bounds)) / (NSWidth(bounds) / 2);
    offset.height = (point.y - NSMidY(bounds)) / (NSHeight(bounds) / 2);
    
    if (!CGSizeEqualToSize(mouseDownLocation, offset)) {
        mouseDownLocation = offset;
    }
}

// --- handler for mouse down messages
- (void)mouseDown:(NSEvent *)event
{
    NSPoint point;
    point = [self convertPoint:[event locationInWindow] fromView:nil];
    mousePoint = point;
    
    // --- save position
    mouseDownPosition.width = currentPosition.width;
    mouseDownPosition.height = currentPosition.height;
    
    //--- save mousedown location
    [self setMouseDownLocationFromPoint:point];
}

// --- handle mouse move messages
- (void)mouseDragged:(NSEvent *)event
{
    NSRect bounds = [self bounds];
    NSPoint point = [self convertPoint:[event locationInWindow] fromView:nil];
    mousePoint = point;
    
    // --- is mouse in view rect?
    if ([self mouse:point inRect:bounds])
    {
        // --- set the position/value
        [self setOffsetFromPoint:point];
    }
}

// --- the drawing function
- (void)drawRect:(NSRect)rect
{
    NSRect bounds;
    float x, y, w, h;
    CGContextRef context;
    
    bounds = [self bounds];
    x = NSMinX(bounds);
    y = NSMinY(bounds);
    w = NSWidth(bounds);
    h = NSHeight(bounds);
    
    // --- get the GDI context
    context = [[NSGraphicsContext currentContext] graphicsPort];
    
    CGContextAddRect(context, NSRectToCGRect(bounds));
    [[NSColor lightGrayColor] setFill];
    NSRectFill(bounds);
    
    // --- draw the puck
    CGContextAddArc(context, mousePoint.x, mousePoint.y, 4, 0, 2*M_PI, true);
    CGContextSetGrayStrokeColor(context, 0.0, 1.0);
    CGContextStrokePath(context);
    
    // --- draw the border
    //
    // --- highlight side
    CGContextMoveToPoint(context, x, y);
    CGContextAddLineToPoint(context,  x+w, y);
    CGContextAddLineToPoint(context,  x+w, y+h);
    CGContextSetLineWidth(context, 2);
    CGContextSetGrayStrokeColor(context, 0.80, 1);
    CGContextStrokePath(context);
    
    // --- shadow side
    CGContextMoveToPoint(context, x+w, y+h);
    CGContextAddLineToPoint(context,  x, y+h);
    CGContextAddLineToPoint(context,  x, y);
    CGContextSetLineWidth(context, 2);
    CGContextSetGrayStrokeColor(context, 0.25, 1);
    CGContextStrokePath(context);
    
    // --- grid lines - add for the AniSynth version
    float rows = 4;
    float cols = 4;
    CGContextSetLineWidth(context, 1);
    CGContextSetGrayStrokeColor(context, 0.5, 1);
    
    CGContextMoveToPoint(context, x+(1/rows)*w, y);
    CGContextAddLineToPoint(context, x+(1/rows)*w, y+h);
    CGContextStrokePath(context);
    
    CGContextMoveToPoint(context, x+(2/rows)*w, y);
    CGContextAddLineToPoint(context, x+(2/rows)*w, y+h);
    CGContextStrokePath(context);
    
    CGContextMoveToPoint(context, x+(3/rows)*w, y);
    CGContextAddLineToPoint(context, x+(3/rows)*w, y+h);
    CGContextStrokePath(context);
    
    CGContextMoveToPoint(context, x, y+(1/cols)*h);
    CGContextAddLineToPoint(context, x+w, y+(1/cols)*h);
    CGContextStrokePath(context);
    
    CGContextMoveToPoint(context, x, y+(2/cols)*h);
    CGContextAddLineToPoint(context, x+w, y+(2/cols)*h);
    CGContextStrokePath(context);
    
    CGContextMoveToPoint(context, x, y+(3/cols)*h);
    CGContextAddLineToPoint(context, x+w, y+(3/cols)*h);
    CGContextStrokePath(context);
        
    // --- draw the angle-arms
    CGContextMoveToPoint(context, x, y);
    CGContextAddLineToPoint(context,  mousePoint.x,  mousePoint.y);
    CGContextMoveToPoint(context, x+w, y);
    CGContextAddLineToPoint(context,  mousePoint.x,  mousePoint.y);
    CGContextMoveToPoint(context, x+w, y+h);
    CGContextAddLineToPoint(context,  mousePoint.x,  mousePoint.y);
    CGContextMoveToPoint(context, x, y+h);
    CGContextAddLineToPoint(context,  mousePoint.x,  mousePoint.y);
    
    CGContextSetLineWidth(context, 1);
    CGContextSetGrayStrokeColor(context, 0.3, 1);
    CGContextStrokePath(context);
}

@end
