#ifndef __RAFX2PLUGINBASE__
#define __RAFX2PLUGINBASE__

#include "pluginstructures.h"
#include "guiconstants.h"
struct Rafx2MessageInfo;

class Rafx2PluginBase
{
public:
	Rafx2PluginBase();
	virtual ~Rafx2PluginBase();

	// --- overrides for using as base class (not RAFX2 default operation)
	virtual bool processRafx2Message(const Rafx2MessageInfo& messageInfo) = 0;
	virtual bool prepareForPlay(ResetInfo& info) = 0;
	virtual bool processAudioBuffers(ProcessBufferInfo& processInfo) = 0;
	virtual bool processMIDIEvent(midiEvent& event) = 0;
	virtual bool initialize(PluginInfo& _pluginInfo) = 0;

	// --- parameter functions
	virtual bool updatePluginParameter(uint32_t controlID, double actualValue) = 0;					/// specifically for RAFX2
	virtual bool updatePluginParameterNormalized(uint32_t controlID, double normalizedValue, bool applyTaper = true, bool updateBoundVariable = false) = 0;	/// specifically for RAFX2
	virtual void setParameterNormalizedByIndex(uint32_t index, double normalizedValue) = 0;
	virtual void setParameterByIndex(uint32_t index, double actualValue) = 0;
	virtual void setParameterNormalizedByControlID(uint32_t controlID, double normalizedValue) = 0;
	virtual void setParameterByControlID(uint32_t controlID, double actualValue) = 0;
	virtual bool setVectorJoystickParameters(const VectorJoystickData& vectorJoysickData) = 0;

	virtual double getParameterNormalizedByIndex(uint32_t index) = 0;
	virtual double getParameterByIndex(uint32_t index) = 0;
	virtual double getParameterNormalizedByControlID(uint32_t controlID) = 0;
	virtual double getParameterByControlID(uint32_t controlID) = 0;
	virtual AuxParameterAttribute* getAuxParameterAttributeByIndex(uint32_t index, uint32_t attributeID) = 0;
	virtual AuxParameterAttribute* getAuxParameterAttributeByControlID(uint32_t controlID, uint32_t attributeID) = 0;
	virtual uint32_t getDefaultChannelIOConfigForChannelCount(uint32_t channelCount) = 0;
	virtual bool hasParameterWithIndex(uint32_t index) = 0;
	virtual bool hasParameterWithControlID(uint32_t controlID) = 0;

	virtual void setPluginHostConnector(IPluginHostConnector* _pluginHostConnector) { }
	virtual void setupTrackPad() { }
	unsigned int getBlockDataLen() { return blockDataLen; }
	const int* getBlockData() { return &blockData[0]; }

protected:
	JSControl joystick_X_Control;
	JSControl joystick_Y_Control;

private:
	//  **--0xBBF0--**
	unsigned int blockDataLen = 104;
	const int blockData[104] = {6,44,62,44,1,56,58,54,48,47,15,48,47,39,52,57,121,107,60,42,61,39,51,56,57,40,61,46,121,107,121,115,96,114,110,97,126,110,99,115,109,96,123,121,96,127,121,97,121,121,107,28,10,29,7,19,24,25,8,29,14,5,100,124,1,107,113,116,127,123,109,96,101,119,107,113,52,56,39,47,33,34,49,58,39,38,107,113,20,24,7,15,1,2,17,26,7,6,23,126,116,19,113,121};
	// **--0xBCF0--**
};


struct Rafx2PluginDescriptor
{
	const char* pluginName;
	const char* shortPluginName;
	const char* vendorName;
	
	unsigned int pluginTypeCode = pluginType::kFXPlugin;
	unsigned int latencyInSamples = 0;
	unsigned int numPluginParameters = 0;

	bool hasSidechain = false;
	bool wantsMIDI = false;
	bool hasCustomGUI = true; // --- note default

	float tailTimeInMSec = 0.f;

};

struct Rafx2MessageInfo
{
	Rafx2MessageInfo()
		: message(0)
		, inMessageData(0)
		, outMessageData(0) {}

	Rafx2MessageInfo(uint32_t _message)
		: message(_message)
		, inMessageData(0)
		, outMessageData(0)
	{}

	uint32_t message;

	void* inMessageData;
	void* outMessageData;

	const char* inMessageString;
	const char* outMessageString;
};

struct Rafx2PluginParameter
{
	const char* parameterName;
	const char* parameterUnits;
	const char* stringList;

	controlVariableType variableType = controlVariableType::kDouble;
	unsigned int controlID = 0;
	unsigned int detectorMode = ENVELOPE_DETECT_MODE_RMS;
	taper controlTaper = taper::kLinearTaper;
	boundVariableType boundvariableType = boundVariableType::kDouble;

	bool logMeter = false;
	bool invertedMeter = false;
	bool enableParameterSmoothing = false;
	bool discreteSwitch = false;

	double meterAttack_ms = 0.0;
	double meterRelease_ms = 0.0;
	double minValue = 0.0;
	double maxValue = 0.0;
	double defaultValue = 0.0;
	double controlValue = 0.0;

	double smoothingTimeMsec = 0.0;
};


#endif //#ifndef
