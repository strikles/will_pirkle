// -----------------------------------------------------------------------------
//    ASPiK Plugin Kernel File:  plugincore.cpp
//
/**
    \file   plugincore.cpp
    \author Will Pirkle
    \date   17-September-2018
    \brief  Implementation file for PluginCore object
    		- http://www.aspikplugins.com
    		- http://www.willpirkle.com
*/
// -----------------------------------------------------------------------------
#include "plugincore.h"
#include "plugindescription.h"

/**
\brief PluginCore constructor is launching pad for object initialization

Operations:
- initialize the plugin description (strings, codes, numbers, see initPluginDescriptors())
- setup the plugin's audio I/O channel support
- create the PluginParameter objects that represent the plugin parameters (see FX book if needed)
- create the presets
*/
PluginCore::PluginCore()
{
    // --- describe the plugin; call the helper to init the static parts you setup in plugindescription.h
    initPluginDescriptors();

    // --- default I/O combinations
	// --- for FX plugins
	if (getPluginType() == kFXPlugin)
	{
		addSupportedIOCombination({ kCFMono, kCFMono });
		addSupportedIOCombination({ kCFMono, kCFStereo });
		addSupportedIOCombination({ kCFStereo, kCFStereo });
	}
	else // --- synth plugins have no input, only output
	{
		addSupportedIOCombination({ kCFNone, kCFMono });
		addSupportedIOCombination({ kCFNone, kCFStereo });
	}

	// --- for sidechaining, we support mono and stereo inputs; auxOutputs reserved for future use
	addSupportedAuxIOCombination({ kCFMono, kCFNone });
	addSupportedAuxIOCombination({ kCFStereo, kCFNone });

	// --- create the parameters
    initPluginParameters();

    // --- create the presets
    initPluginPresets();
}

/**
\brief create all of your plugin parameters here

\return true if parameters were created, false if they already existed
*/
bool PluginCore::initPluginParameters()
{
	if (pluginParameterMap.size() > 0)
		return false;

    // --- Add your plugin parameter instantiation code bewtween these hex codes
	// **--0xDEA7--**


	// --- Declaration of Plugin Parameter Objects 
	PluginParameter* piParam = nullptr;

	// --- continuous control: Threshold
	piParam = new PluginParameter(controlID::threshold_dB, "Threshold", "dB", controlVariableType::kDouble, -40.000000, 0.000000, -10.000000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&threshold_dB, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- continuous control: KneeWidth
	piParam = new PluginParameter(controlID::kneeWidth_dB, "KneeWidth", "dB", controlVariableType::kDouble, 0.000000, 20.000000, 10.000000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&kneeWidth_dB, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- continuous control: Ratio
	piParam = new PluginParameter(controlID::ratio, "Ratio", "", controlVariableType::kDouble, 1.000000, 20.000000, 1.000000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&ratio, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- continuous control: Attack
	piParam = new PluginParameter(controlID::attackTime_mSec, "Attack", "mSec", controlVariableType::kDouble, 1.000000, 100.000000, 5.000000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&attackTime_mSec, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- continuous control: Release
	piParam = new PluginParameter(controlID::releaseTime_mSec, "Release", "mSec", controlVariableType::kDouble, 1.000000, 5000.000000, 500.000000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&releaseTime_mSec, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- continuous control: Output Gain
	piParam = new PluginParameter(controlID::outputGain_dB, "Output Gain", "dB", controlVariableType::kDouble, -20.000000, 20.000000, 0.000000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&outputGain_dB, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- discrete control: Mode
	piParam = new PluginParameter(controlID::dynamicsMode, "Mode", "compress,expand", "compress");
	piParam->setBoundVariable(&dynamicsMode, boundVariableType::kInt);
	piParam->setIsDiscreteSwitch(true);
	addPluginParameter(piParam);

	// --- discrete control: Limiter/Gate
	piParam = new PluginParameter(controlID::hardLimitGate, "Limiter/Gate", "SWITCH OFF,SWITCH ON", "SWITCH OFF");
	piParam->setBoundVariable(&hardLimitGate, boundVariableType::kInt);
	piParam->setIsDiscreteSwitch(true);
	addPluginParameter(piParam);

	// --- discrete control: Soft Knee
	piParam = new PluginParameter(controlID::softKnee, "Soft Knee", "SWITCH OFF,SWITCH ON", "SWITCH OFF");
	piParam->setBoundVariable(&softKnee, boundVariableType::kInt);
	piParam->setIsDiscreteSwitch(true);
	addPluginParameter(piParam);

	// --- meter control: GR
	piParam = new PluginParameter(controlID::grMeter, "GR", 0.00, 0.00, ENVELOPE_DETECT_MODE_PEAK, meterCal::kLinearMeter);
	piParam->setInvertedMeter(true);
	piParam->setIsProtoolsGRMeter(true);
	piParam->setBoundVariable(&grMeter, boundVariableType::kFloat);
	addPluginParameter(piParam);

	// --- discrete control: Stereo Link
	piParam = new PluginParameter(controlID::stereoLink, "Stereo Link", "SWITCH OFF,SWITCH ON", "SWITCH OFF");
	piParam->setBoundVariable(&stereoLink, boundVariableType::kInt);
	piParam->setIsDiscreteSwitch(true);
	addPluginParameter(piParam);

	// --- Aux Attributes
	AuxParameterAttribute auxAttribute;

	// --- RAFX GUI attributes
	// --- controlID::threshold_dB
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483648);
	setParamAuxAttribute(controlID::threshold_dB, auxAttribute);

	// --- controlID::kneeWidth_dB
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483648);
	setParamAuxAttribute(controlID::kneeWidth_dB, auxAttribute);

	// --- controlID::ratio
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483648);
	setParamAuxAttribute(controlID::ratio, auxAttribute);

	// --- controlID::attackTime_mSec
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483648);
	setParamAuxAttribute(controlID::attackTime_mSec, auxAttribute);

	// --- controlID::releaseTime_mSec
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483648);
	setParamAuxAttribute(controlID::releaseTime_mSec, auxAttribute);

	// --- controlID::outputGain_dB
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483648);
	setParamAuxAttribute(controlID::outputGain_dB, auxAttribute);

	// --- controlID::dynamicsMode
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(805306368);
	setParamAuxAttribute(controlID::dynamicsMode, auxAttribute);

	// --- controlID::hardLimitGate
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(1073741824);
	setParamAuxAttribute(controlID::hardLimitGate, auxAttribute);

	// --- controlID::softKnee
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(1073741824);
	setParamAuxAttribute(controlID::softKnee, auxAttribute);

	// --- controlID::grMeter
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(134217728);
	setParamAuxAttribute(controlID::grMeter, auxAttribute);

	// --- controlID::stereoLink
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(1073741824);
	setParamAuxAttribute(controlID::stereoLink, auxAttribute);


	// **--0xEDA5--**
   
    // --- BONUS Parameter
    // --- SCALE_GUI_SIZE
    PluginParameter* piParamBonus = new PluginParameter(SCALE_GUI_SIZE, "Scale GUI", "tiny,small,medium,normal,large,giant", "normal");
    addPluginParameter(piParamBonus);

	// --- create the super fast access array
	initPluginParameterArray();

    return true;
}

/**
\brief initialize object for a new run of audio; called just before audio streams

Operation:
- store sample rate and bit depth on audioProcDescriptor - this information is globally available to all core functions
- reset your member objects here

\param resetInfo structure of information about current audio format

\return true if operation succeeds, false otherwise
*/
bool PluginCore::reset(ResetInfo& resetInfo)
{
    // --- save for audio processing
    audioProcDescriptor.sampleRate = resetInfo.sampleRate;
    audioProcDescriptor.bitDepth = resetInfo.bitDepth;

	// --- reset the processors
	for(int i=0; i< DYN_CHANNELS; i++)
		dynamicsProcessors[i].reset(resetInfo.sampleRate);

    // --- other reset inits
    return PluginBase::reset(resetInfo);
}

/**
\brief one-time initialize function called after object creation and before the first reset( ) call

Operation:
- saves structure for the plugin to use; you can also load WAV files or state information here
*/
bool PluginCore::initialize(PluginInfo& pluginInfo)
{
	// --- add one-time init stuff here

	return true;
}

/**
\brief do anything needed prior to arrival of audio buffers

Operation:
- syncInBoundVariables when preProcessAudioBuffers is called, it is *guaranteed* that all GUI control change information
  has been applied to plugin parameters; this binds parameter changes to your underlying variables
- NOTE: postUpdatePluginParameter( ) will be called for all bound variables that are acutally updated; if you need to process
  them individually, do so in that function
- use this function to bulk-transfer the bound variable data into your plugin's member object variables

\param processInfo structure of information about *buffer* processing

\return true if operation succeeds, false otherwise
*/
bool PluginCore::preProcessAudioBuffers(ProcessBufferInfo& processInfo)
{
    // --- sync internal variables to GUI parameters; you can also do this manually if you don't
    //     want to use the auto-variable-binding
    syncInBoundVariables();

    return true;
}

/**
\brief Update C++ Object Parameters from GUI control variables

See Designing Audio Effects Plugins in C++, 2nd Edition by Will Pirkle
www.willpirkle.com

Operation:
- call getParameters on the effect object to retrieve a data structure
- set the parameters with the GUI values in the structure
- call setParameters and pass the structure back to the object

\return none
*/
void PluginCore::updateParameters()
{
	DynamicsProcessorParameters params = dynamicsProcessors[0].getParameters();
	params.threshold_dB = threshold_dB;
	params.kneeWidth_dB = kneeWidth_dB;
	params.ratio = ratio;
	params.attackTime_mSec = attackTime_mSec;
	params.releaseTime_mSec = releaseTime_mSec;
	params.outputGain_dB = outputGain_dB;
	params.calculation = convertIntToEnum(dynamicsMode, dynamicsProcessorType);
	params.hardLimitGate = (hardLimitGate == 1);
	params.softKnee = (softKnee == 1);

	// --- update objects
	for (int i = 0; i< DYN_CHANNELS; i++)
		dynamicsProcessors[i].setParameters(params);

}

/**
\brief frame-processing method

Operation:
- decode the plugin type - for synth plugins, fill in the rendering code; for FX plugins, delete the if(synth) portion and add your processing code
- note that MIDI events are fired for each sample interval so that MIDI is tightly sunk with audio
- doSampleAccurateParameterUpdates will perform per-sample interval smoothing

\param processFrameInfo structure of information about *frame* processing

\return true if operation succeeds, false otherwise
*/
bool PluginCore::processAudioFrame(ProcessFrameInfo& processFrameInfo)
{
    // --- fire any MIDI events for this sample interval
    processFrameInfo.midiEventQueue->fireMidiEvents(processFrameInfo.currentFrame);

	// --- do per-frame updates; VST automation and parameter smoothing
	doSampleAccurateParameterUpdates();

	// --- Check is Synth Plugin and input/output existence:
	if (getPluginType() == kSynthPlugin || processFrameInfo.numAudioInChannels == 0 || processFrameInfo.numAudioOutChannels == 0)
		return false;

	// --- update GUI parameters: this is here for smoothing; you can optimize this
	//     by relocating the non-smoothed variable updates to preProcessAudioBuffers( )
	//     See the ASPiK documentation and Pirkle FX book
	// --- do left channel
	updateParameters();
	
	// --- do left channel (there will always be at least one channel if we get here)
	double xnL = processFrameInfo.audioInputFrame[0];//< framework input sample L

	// --- check the right channel's existence in case of stereo link
	double xnR = processFrameInfo.numAudioInChannels > 1 ? processFrameInfo.audioInputFrame[1] : 0.0;

	// --- detect external sidechain: process if it exists
	if (processFrameInfo.numAuxAudioInChannels > 0)
		dynamicsProcessors[0].processAuxInputAudioSample(processFrameInfo.auxAudioInputFrame[0]);

	// --- for stereo linking (no external keying)
	DynamicsProcessorParameters params = dynamicsProcessors[0].getParameters();
	if (stereoLink == 1 && processFrameInfo.numAudioInChannels >= 2)
	{
		// --- combine sidechain input
		double sidechain = 0.5*xnL + 0.5*xnR;

		// --- enable on object (this can be done outside of the process function)
		params.enableSidechain = true;
		dynamicsProcessors[0].setParameters(params);
		dynamicsProcessors[1].setParameters(params);

		// --- set the stereo link input
		dynamicsProcessors[0].processAuxInputAudioSample(sidechain);
		dynamicsProcessors[1].processAuxInputAudioSample(sidechain);
	}
	else if (params.enableSidechain)
	{
		params.enableSidechain = false;
		dynamicsProcessors[0].setParameters(params);
		dynamicsProcessors[1].setParameters(params);
	}

	// --- process the left sample
	double ynL = dynamicsProcessors[0].processAudioSample(xnL);

	// --- set Left channel part of gr meter value
	double grLeft = 0.0;
	double grRight = 0.0;

	// --- setup gain reduction metering
	DynamicsProcessorParameters paramsL = dynamicsProcessors[0].getParameters();
	grLeft = paramsL.gainReduction;
	grMeter = 1.0 - grLeft;

    // --- FX Plugin:
    if(processFrameInfo.channelIOConfig.inputChannelFormat == kCFMono &&
       processFrameInfo.channelIOConfig.outputChannelFormat == kCFMono)
    {
		// --- ynL
        processFrameInfo.audioOutputFrame[0] = ynL;//< framework output sample L

        return true; /// processed
    }

    // --- Mono-In/Stereo-Out
    else if(processFrameInfo.channelIOConfig.inputChannelFormat == kCFMono &&
       processFrameInfo.channelIOConfig.outputChannelFormat == kCFStereo)
    {
		// --- ynL
        processFrameInfo.audioOutputFrame[0] = ynL;//< framework output sample L
        processFrameInfo.audioOutputFrame[1] = ynL;//< framework output sample R

        return true; /// processed
    }

    // --- Stereo-In/Stereo-Out
    else if(processFrameInfo.channelIOConfig.inputChannelFormat == kCFStereo &&
       processFrameInfo.channelIOConfig.outputChannelFormat == kCFStereo)
    {
		// --- process right channel
		// double xnR = processFrameInfo.audioInputFrame[1]; <-- declared above for stereo linking
		double ynR = dynamicsProcessors[1].processAudioSample(xnR);

		// --- modify gain reduction meter
		DynamicsProcessorParameters paramsR = dynamicsProcessors[1].getParameters();
		grRight = paramsR.gainReduction;
		grMeter = 1.0 - (0.5*grLeft + 0.5*grRight);

		// --- ynL, ynR
		processFrameInfo.audioOutputFrame[0] = ynL;//< framework output sample L
		processFrameInfo.audioOutputFrame[1] = ynR;//< framework output sample R

		return true; /// processed
    }

    return false; /// NOT processed
}


/**
\brief do anything needed prior to arrival of audio buffers

Operation:
- updateOutBoundVariables sends metering data to the GUI meters

\param processInfo structure of information about *buffer* processing

\return true if operation succeeds, false otherwise
*/
bool PluginCore::postProcessAudioBuffers(ProcessBufferInfo& processInfo)
{
	// --- update outbound variables; currently this is meter data only, but could be extended
	//     in the future
	updateOutBoundVariables();

    return true;
}

/**
\brief update the PluginParameter's value based on GUI control, preset, or data smoothing (thread-safe)

Operation:
- update the parameter's value (with smoothing this initiates another smoothing process)
- call postUpdatePluginParameter to do any further processing

\param controlID the control ID value of the parameter being updated
\param controlValue the new control value
\param paramInfo structure of information about why this value is being udpated (e.g as a result of a preset being loaded vs. the top of a buffer process cycle)

\return true if operation succeeds, false otherwise
*/
bool PluginCore::updatePluginParameter(int32_t controlID, double controlValue, ParameterUpdateInfo& paramInfo)
{
    // --- use base class helper
    setPIParamValue(controlID, controlValue);

    // --- do any post-processing
    postUpdatePluginParameter(controlID, controlValue, paramInfo);

    return true; /// handled
}

/**
\brief update the PluginParameter's value based on *normlaized* GUI control, preset, or data smoothing (thread-safe)

Operation:
- update the parameter's value (with smoothing this initiates another smoothing process)
- call postUpdatePluginParameter to do any further processing

\param controlID the control ID value of the parameter being updated
\param normalizedValue the new control value in normalized form
\param paramInfo structure of information about why this value is being udpated (e.g as a result of a preset being loaded vs. the top of a buffer process cycle)

\return true if operation succeeds, false otherwise
*/
bool PluginCore::updatePluginParameterNormalized(int32_t controlID, double normalizedValue, ParameterUpdateInfo& paramInfo)
{
	// --- use base class helper, returns actual value
	double controlValue = setPIParamValueNormalized(controlID, normalizedValue, paramInfo.applyTaper);

	// --- do any post-processing
	postUpdatePluginParameter(controlID, controlValue, paramInfo);

	return true; /// handled
}

/**
\brief perform any operations after the plugin parameter has been updated; this is one paradigm for
	   transferring control information into vital plugin variables or member objects. If you use this
	   method you can decode the control ID and then do any cooking that is needed. NOTE: do not
	   overwrite bound variables here - this is ONLY for any extra cooking that is required to convert
	   the GUI data to meaninful coefficients or other specific modifiers.

\param controlID the control ID value of the parameter being updated
\param controlValue the new control value
\param paramInfo structure of information about why this value is being udpated (e.g as a result of a preset being loaded vs. the top of a buffer process cycle)

\return true if operation succeeds, false otherwise
*/
bool PluginCore::postUpdatePluginParameter(int32_t controlID, double controlValue, ParameterUpdateInfo& paramInfo)
{
    // --- now do any post update cooking; be careful with VST Sample Accurate automation
    //     If enabled, then make sure the cooking functions are short and efficient otherwise disable it
    //     for the Parameter involved
    /*switch(controlID)
    {
        case 0:
        {
            return true;    /// handled
        }

        default:
            return false;   /// not handled
    }*/

    return false;
}

/**
\brief has nothing to do with actual variable or updated variable (binding)

CAUTION:
- DO NOT update underlying variables here - this is only for sending GUI updates or letting you
  know that a parameter was changed; it should not change the state of your plugin.

WARNING:
- THIS IS NOT THE PREFERRED WAY TO LINK OR COMBINE CONTROLS TOGETHER. THE PROPER METHOD IS
  TO USE A CUSTOM SUB-CONTROLLER THAT IS PART OF THE GUI OBJECT AND CODE.
  SEE http://www.willpirkle.com for more information

\param controlID the control ID value of the parameter being updated
\param actualValue the new control value

\return true if operation succeeds, false otherwise
*/
bool PluginCore::guiParameterChanged(int32_t controlID, double actualValue)
{
	/*
	switch (controlID)
	{
		case controlID::<your control here>
		{

			return true; // handled
		}

		default:
			break;
	}*/

	return false; /// not handled
}

/**
\brief For Custom View and Custom Sub-Controller Operations

NOTES:
- this is for advanced users only to implement custom view and custom sub-controllers
- see the SDK for examples of use

\param messageInfo a structure containing information about the incoming message

\return true if operation succeeds, false otherwise
*/
bool PluginCore::processMessage(MessageInfo& messageInfo)
{
	// --- decode message
	switch (messageInfo.message)
	{
		// --- add customization appearance here
	case PLUGINGUI_DIDOPEN:
	{
		return false;
	}

	// --- NULL pointers so that we don't accidentally use them
	case PLUGINGUI_WILLCLOSE:
	{
		return false;
	}

	// --- update view; this will only be called if the GUI is actually open
	case PLUGINGUI_TIMERPING:
	{
		return false;
	}

	// --- register the custom view, grab the ICustomView interface
	case PLUGINGUI_REGISTER_CUSTOMVIEW:
	{

		return false;
	}

	case PLUGINGUI_REGISTER_SUBCONTROLLER:
	case PLUGINGUI_QUERY_HASUSERCUSTOM:
	case PLUGINGUI_USER_CUSTOMOPEN:
	case PLUGINGUI_USER_CUSTOMCLOSE:
	case PLUGINGUI_EXTERNAL_SET_NORMVALUE:
	case PLUGINGUI_EXTERNAL_SET_ACTUALVALUE:
	{

		return false;
	}

	default:
		break;
	}

	return false; /// not handled
}


/**
\brief process a MIDI event

NOTES:
- MIDI events are 100% sample accurate; this function will be called repeatedly for every MIDI message
- see the SDK for examples of use

\param event a structure containing the MIDI event data

\return true if operation succeeds, false otherwise
*/
bool PluginCore::processMIDIEvent(midiEvent& event)
{
	return true;
}

/**
\brief (for future use)

NOTES:
- MIDI events are 100% sample accurate; this function will be called repeatedly for every MIDI message
- see the SDK for examples of use

\param vectorJoysickData a structure containing joystick data

\return true if operation succeeds, false otherwise
*/
bool PluginCore::setVectorJoystickParameters(const VectorJoystickData& vectorJoysickData)
{
	return true;
}

/**
\brief use this method to add new presets to the list

NOTES:
- see the SDK for examples of use
- for non RackAFX users that have large paramter counts, there is a secret GUI control you
  can enable to write C++ code into text files, one per preset. See the SDK or http://www.willpirkle.com for details

\return true if operation succeeds, false otherwise
*/
bool PluginCore::initPluginPresets()
{
	// **--0xFF7A--**

	// --- Plugin Presets 
	int index = 0;
	PresetInfo* preset = nullptr;

	// --- Preset: Factory Preset
	preset = new PresetInfo(index++, "Factory Preset");
	initPresetParameters(preset->presetParameters);
	setPresetParameter(preset->presetParameters, controlID::threshold_dB, -10.000000);
	setPresetParameter(preset->presetParameters, controlID::kneeWidth_dB, 10.000000);
	setPresetParameter(preset->presetParameters, controlID::ratio, 1.000000);
	setPresetParameter(preset->presetParameters, controlID::attackTime_mSec, 5.000000);
	setPresetParameter(preset->presetParameters, controlID::releaseTime_mSec, 500.000000);
	setPresetParameter(preset->presetParameters, controlID::outputGain_dB, 0.000000);
	setPresetParameter(preset->presetParameters, controlID::dynamicsMode, -0.000000);
	setPresetParameter(preset->presetParameters, controlID::hardLimitGate, -0.000000);
	setPresetParameter(preset->presetParameters, controlID::softKnee, -0.000000);
	setPresetParameter(preset->presetParameters, controlID::stereoLink, -0.000000);
	addPreset(preset);


	// **--0xA7FF--**

    return true;
}

/**
\brief setup the plugin description strings, flags and codes; this is ordinarily done through the ASPiKreator or CMake

\return true if operation succeeds, false otherwise
*/
bool PluginCore::initPluginDescriptors()
{
    pluginDescriptor.pluginName = PluginCore::getPluginName();
    pluginDescriptor.shortPluginName = PluginCore::getShortPluginName();
    pluginDescriptor.vendorName = PluginCore::getVendorName();
    pluginDescriptor.pluginTypeCode = PluginCore::getPluginType();

	// --- describe the plugin attributes; set according to your needs
	pluginDescriptor.hasSidechain = kWantSidechain;
	pluginDescriptor.latencyInSamples = kLatencyInSamples;
	pluginDescriptor.tailTimeInMSec = kTailTimeMsec;
	pluginDescriptor.infiniteTailVST3 = kVSTInfiniteTail;

    // --- AAX
    apiSpecificInfo.aaxManufacturerID = kManufacturerID;
    apiSpecificInfo.aaxProductID = kAAXProductID;
    apiSpecificInfo.aaxBundleID = kAAXBundleID;  /* MacOS only: this MUST match the bundle identifier in your info.plist file */
    apiSpecificInfo.aaxEffectID = "aaxDeveloper.";
    apiSpecificInfo.aaxEffectID.append(PluginCore::getPluginName());
    apiSpecificInfo.aaxPluginCategoryCode = kAAXCategory;

    // --- AU
    apiSpecificInfo.auBundleID = kAUBundleID;
	apiSpecificInfo.auBundleName = kAUBundleName;   /* MacOS only: this MUST match the bundle identifier in your info.plist file */

    // --- VST3
    apiSpecificInfo.vst3FUID = PluginCore::getVSTFUID(); // OLE string format
    apiSpecificInfo.vst3BundleID = kVST3BundleID;/* MacOS only: this MUST match the bundle identifier in your info.plist file */
	apiSpecificInfo.enableVST3SampleAccurateAutomation = kVSTSAA;
	apiSpecificInfo.vst3SampleAccurateGranularity = kVST3SAAGranularity;

    // --- AU and AAX
    apiSpecificInfo.fourCharCode = PluginCore::getFourCharCode();

    return true;
}

// --- static functions required for VST3/AU only --------------------------------------------- //
const char* PluginCore::getPluginBundleName() { return kAUBundleName; }
const char* PluginCore::getPluginName(){ return kPluginName; }
const char* PluginCore::getShortPluginName(){ return kShortPluginName; }
const char* PluginCore::getVendorName(){ return kVendorName; }
const char* PluginCore::getVendorURL(){ return kVendorURL; }
const char* PluginCore::getVendorEmail(){ return kVendorEmail; }
const char* PluginCore::getAUCocoaViewFactoryName(){ return AU_COCOA_VIEWFACTORY_STRING; }
pluginType PluginCore::getPluginType(){ return kPluginType; }
const char* PluginCore::getVSTFUID(){ return kVSTFUID; }
int32_t PluginCore::getFourCharCode(){ return kFourCharCode; }
