//
//  WPTrackPad.h
//  DigiSynth
//
//  Created by William Pirkle on 1/26/14.
//  Copyright (c) 2014 University of Miami. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface WPTrackPadVS : NSControl
{
    // --- these are raw positions -1 to +1 ordinarily
    CGSize currentPosition;  // current position
    CGSize mouseDownPosition; // position of knob when mousedown
    
    // --- mouse down location
    CGSize mouseDownLocation;
    
    // --- mouse point location
    NSPoint mousePoint;
    
    // --- default is 1.0
    float knobScale;
}
// --- get the XY coordinates of mouse
-(void)getControlCoordsX:(float*)x Y:(float*)y;


@end