//
//  MIDIOutputCallbackHelper.h
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

#ifndef MiniSynth_MIDIOutputCallbackHelper_h
#define MiniSynth_MIDIOutputCallbackHelper_h
#import <CoreMIDI/MIDIServices.h>

// --- MIDI callback object
class MIDIOutputCallbackHelper 
{
public:
    // --- constructor
    MIDIOutputCallbackHelper() {mMIDIMessageList.reserve(16); mMIDICallbackStruct.midiOutputCallback = NULL;}
	virtual ~MIDIOutputCallbackHelper() {};
	
	void SetCallbackInfo(AUMIDIOutputCallback callback, void *userData)
    {
        mMIDICallbackStruct.midiOutputCallback = callback; 
        mMIDICallbackStruct.userData = userData;
    }
	
    // --- add a MIDI event to our vector
	void AddEvent(UInt8	status,
                  UInt8	channel,
                  UInt8	data1,
                  UInt8	data2, 
                  UInt32 inStartFrame );
    
    // --- for timestamping MIDI events
	void FireAtTimeStamp(const AudioTimeStamp &inTimeStamp);
    
protected:
    // --- vector for holding 
	typedef std::vector<MIDIMessageInfoStruct> MIDIMessageList;
	
private:
    // --- stuff to handle MIDI buffering
	MIDIPacketList* PacketList() {return (MIDIPacketList *)mBuffersAllocated;}
	Byte mBuffersAllocated[1024];
	
    // --- callback struct for getting MIDI Info
	AUMIDIOutputCallbackStruct mMIDICallbackStruct;
    
    // --- the MIDI message list
	MIDIMessageList mMIDIMessageList;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	MIDIOutputCallbackHelper::AddEvent
//
// 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void MIDIOutputCallbackHelper::AddEvent(UInt8 status, UInt8 channel,
										UInt8 data1, UInt8 data2, 
										UInt32 inStartFrame) {
	MIDIMessageInfoStruct info = {status, channel, data1, data2, inStartFrame};	
	mMIDIMessageList.push_back(info);
}

void MIDIOutputCallbackHelper::FireAtTimeStamp(const AudioTimeStamp &inTimeStamp) {
	if (!mMIDIMessageList.empty() && mMIDICallbackStruct.midiOutputCallback != 0) {
		// synthesize the packet list and call the MIDIOutputCallback
		// iterate through the vector and get each item
        
		std::vector<MIDIMessageInfoStruct>::iterator myIterator;
		MIDIPacketList *pktlist = PacketList();
        
		for (myIterator = mMIDIMessageList.begin(); myIterator != mMIDIMessageList.end(); myIterator++) {
			MIDIMessageInfoStruct item = *myIterator;
			
			MIDIPacket *pkt = MIDIPacketListInit(pktlist);
			bool tooBig = false;
			
			Byte data[4] = {item.status, item.channel, item.data1, item.data2};
			if ((pkt = MIDIPacketListAdd(pktlist, sizeof(mBuffersAllocated), pkt, item.startFrame, 4, const_cast<Byte*>(data))) == NULL)
				tooBig = true;
            
			if (tooBig) {	// send what we have and then clear the buffer and send again
				// issue the callback with what we got
				OSStatus result = mMIDICallbackStruct.midiOutputCallback(mMIDICallbackStruct.userData, &inTimeStamp, 0, pktlist);
				if (result != noErr)
					printf("error calling output callback: %d", (int) result);
				
				// clear stuff we've already processed, and fire again
				mMIDIMessageList.erase(mMIDIMessageList.begin(), myIterator);
				this->FireAtTimeStamp(inTimeStamp);
				return;
			}
		}
		
		// fire callback
		OSStatus result = mMIDICallbackStruct.midiOutputCallback(mMIDICallbackStruct.userData, &inTimeStamp, 0, pktlist);
		if (result != noErr)
			printf("error calling output callback: %d", (int) result);
        
		mMIDIMessageList.clear();
	}
}

#endif
