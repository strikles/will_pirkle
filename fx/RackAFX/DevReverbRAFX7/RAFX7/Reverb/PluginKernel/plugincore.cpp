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

	// --- continuous control: Reverb Time
	piParam = new PluginParameter(controlID::kRT, "Reverb Time", "", controlVariableType::kDouble, 0.000000, 1.000000, 0.500000, taper::kLogTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&kRT, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- continuous control: Damping
	piParam = new PluginParameter(controlID::lpf_g, "Damping", "", controlVariableType::kDouble, 0.000000, 1.000000, 0.500000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&lpf_g, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- continuous control: Low Shelf Fc
	piParam = new PluginParameter(controlID::lowShelf_fc, "Low Shelf Fc", "Hz", controlVariableType::kDouble, 20.000000, 2000.000000, 500.000000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&lowShelf_fc, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- continuous control: LowShelf Gain
	piParam = new PluginParameter(controlID::lowShelfBoostCut_dB, "LowShelf Gain", "dB", controlVariableType::kDouble, -20.000000, 20.000000, 0.000000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&lowShelfBoostCut_dB, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- continuous control: High Shelf Fc
	piParam = new PluginParameter(controlID::highShelf_fc, "High Shelf Fc", "Hz", controlVariableType::kDouble, 1000.000000, 5000.000000, 2000.000000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&highShelf_fc, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- continuous control: HighShelf Gain
	piParam = new PluginParameter(controlID::highShelfBoostCut_dB, "HighShelf Gain", "dB", controlVariableType::kDouble, -20.000000, 20.000000, 0.000000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&highShelfBoostCut_dB, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- continuous control: Wet Level
	piParam = new PluginParameter(controlID::wetLevel_dB, "Wet Level", "dB", controlVariableType::kDouble, -60.000000, 12.000000, -3.000000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&wetLevel_dB, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- continuous control: Dry Level
	piParam = new PluginParameter(controlID::dryLevel_dB, "Dry Level", "dB", controlVariableType::kDouble, -60.000000, 12.000000, -3.000000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&dryLevel_dB, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- continuous control: Max APF Dly
	piParam = new PluginParameter(controlID::apfDelayMax_mSec, "Max APF Dly", "mSec", controlVariableType::kDouble, 0.000000, 100.000000, 5.000000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&apfDelayMax_mSec, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- continuous control: APF Dly Wt
	piParam = new PluginParameter(controlID::apfDelayWeight_Pct, "APF Dly Wt", "%", controlVariableType::kDouble, 1.000000, 100.000000, 50.000000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&apfDelayWeight_Pct, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- continuous control: Max Fix Dly
	piParam = new PluginParameter(controlID::fixeDelayMax_mSec, "Max Fix Dly", "mSec", controlVariableType::kDouble, 0.000000, 100.000000, 5.000000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&fixeDelayMax_mSec, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- continuous control: Fix Dly Wt
	piParam = new PluginParameter(controlID::fixeDelayWeight_Pct, "Fix Dly Wt", "%", controlVariableType::kDouble, 1.000000, 100.000000, 50.000000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&fixeDelayWeight_Pct, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- continuous control: PreDelay Time
	piParam = new PluginParameter(controlID::preDelayTime_mSec, "PreDelay Time", "mSec", controlVariableType::kDouble, 0.000000, 100.000000, 25.000000, taper::kLinearTaper);
	piParam->setParameterSmoothing(true);
	piParam->setSmoothingTimeMsec(20.00);
	piParam->setBoundVariable(&preDelayTime_mSec, boundVariableType::kDouble);
	addPluginParameter(piParam);

	// --- discrete control: Density
	piParam = new PluginParameter(controlID::density, "Density", "thick,thin", "thick");
	piParam->setBoundVariable(&density, boundVariableType::kInt);
	piParam->setIsDiscreteSwitch(true);
	addPluginParameter(piParam);

	// --- meter control: IN
	piParam = new PluginParameter(controlID::leftVUMeterIn, "IN", 1.00, 200.00, ENVELOPE_DETECT_MODE_RMS, meterCal::kLogMeter);
	piParam->setBoundVariable(&leftVUMeterIn, boundVariableType::kFloat);
	addPluginParameter(piParam);

	// --- meter control: OUT
	piParam = new PluginParameter(controlID::leftVUMeterOut, "OUT", 1.00, 200.00, ENVELOPE_DETECT_MODE_RMS, meterCal::kLogMeter);
	piParam->setBoundVariable(&leftVUMeterOut, boundVariableType::kFloat);
	addPluginParameter(piParam);

	// --- Aux Attributes
	AuxParameterAttribute auxAttribute;

	// --- RAFX GUI attributes
	// --- controlID::kRT
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483648);
	setParamAuxAttribute(controlID::kRT, auxAttribute);

	// --- controlID::lpf_g
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483648);
	setParamAuxAttribute(controlID::lpf_g, auxAttribute);

	// --- controlID::lowShelf_fc
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483648);
	setParamAuxAttribute(controlID::lowShelf_fc, auxAttribute);

	// --- controlID::lowShelfBoostCut_dB
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483648);
	setParamAuxAttribute(controlID::lowShelfBoostCut_dB, auxAttribute);

	// --- controlID::highShelf_fc
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483648);
	setParamAuxAttribute(controlID::highShelf_fc, auxAttribute);

	// --- controlID::highShelfBoostCut_dB
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483648);
	setParamAuxAttribute(controlID::highShelfBoostCut_dB, auxAttribute);

	// --- controlID::wetLevel_dB
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483648);
	setParamAuxAttribute(controlID::wetLevel_dB, auxAttribute);

	// --- controlID::dryLevel_dB
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483648);
	setParamAuxAttribute(controlID::dryLevel_dB, auxAttribute);

	// --- controlID::apfDelayMax_mSec
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483654);
	setParamAuxAttribute(controlID::apfDelayMax_mSec, auxAttribute);

	// --- controlID::apfDelayWeight_Pct
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483654);
	setParamAuxAttribute(controlID::apfDelayWeight_Pct, auxAttribute);

	// --- controlID::fixeDelayMax_mSec
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483656);
	setParamAuxAttribute(controlID::fixeDelayMax_mSec, auxAttribute);

	// --- controlID::fixeDelayWeight_Pct
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483656);
	setParamAuxAttribute(controlID::fixeDelayWeight_Pct, auxAttribute);

	// --- controlID::preDelayTime_mSec
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(2147483648);
	setParamAuxAttribute(controlID::preDelayTime_mSec, auxAttribute);

	// --- controlID::density
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(805306368);
	setParamAuxAttribute(controlID::density, auxAttribute);

	// --- controlID::leftVUMeterIn
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(201326592);
	setParamAuxAttribute(controlID::leftVUMeterIn, auxAttribute);

	// --- controlID::leftVUMeterOut
	auxAttribute.reset(auxGUIIdentifier::guiControlData);
	auxAttribute.setUintAttribute(201326592);
	setParamAuxAttribute(controlID::leftVUMeterOut, auxAttribute);


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

	// --- reset the object
	reverb.reset(resetInfo.sampleRate);

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
	ReverbTankParameters params = reverb.getParameters();
	params.kRT = kRT;		// "Reverb Time"
	params.lpf_g = lpf_g;	// "Damping"

	params.lowShelf_fc = lowShelf_fc;
	params.lowShelfBoostCut_dB = lowShelfBoostCut_dB;
	params.highShelf_fc = highShelf_fc;
	params.highShelfBoostCut_dB = highShelfBoostCut_dB;

	params.preDelayTime_mSec = preDelayTime_mSec;
	params.wetLevel_dB = wetLevel_dB;
	params.dryLevel_dB = dryLevel_dB;

	// --- tweakers
	params.apfDelayMax_mSec = apfDelayMax_mSec;
	params.apfDelayWeight_Pct = apfDelayWeight_Pct;
	params.fixeDelayMax_mSec = fixeDelayMax_mSec;
	params.fixeDelayWeight_Pct = fixeDelayWeight_Pct;

	params.density = convertIntToEnum(density, reverbDensity);
	reverb.setParameters(params);
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

	// --- object does all the work on frames
	bool success = reverb.processAudioFrame(processFrameInfo.audioInputFrame,
											processFrameInfo.audioOutputFrame,
											processFrameInfo.numAudioInChannels,
											processFrameInfo.numAudioOutChannels);

	// --- VU metering
	leftVUMeterIn = processFrameInfo.audioInputFrame[0];
	leftVUMeterOut = processFrameInfo.audioOutputFrame[0];

    return success; ///
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
	setPresetParameter(preset->presetParameters, controlID::kRT, 0.500000);
	setPresetParameter(preset->presetParameters, controlID::lpf_g, 0.500000);
	setPresetParameter(preset->presetParameters, controlID::lowShelf_fc, 500.000000);
	setPresetParameter(preset->presetParameters, controlID::lowShelfBoostCut_dB, 0.000000);
	setPresetParameter(preset->presetParameters, controlID::highShelf_fc, 2000.000000);
	setPresetParameter(preset->presetParameters, controlID::highShelfBoostCut_dB, 0.000000);
	setPresetParameter(preset->presetParameters, controlID::wetLevel_dB, -3.000000);
	setPresetParameter(preset->presetParameters, controlID::dryLevel_dB, -3.000000);
	setPresetParameter(preset->presetParameters, controlID::apfDelayMax_mSec, 5.000000);
	setPresetParameter(preset->presetParameters, controlID::apfDelayWeight_Pct, 50.000000);
	setPresetParameter(preset->presetParameters, controlID::fixeDelayMax_mSec, 5.000000);
	setPresetParameter(preset->presetParameters, controlID::fixeDelayWeight_Pct, 50.000000);
	setPresetParameter(preset->presetParameters, controlID::preDelayTime_mSec, 25.000000);
	setPresetParameter(preset->presetParameters, controlID::density, -0.000000);
	addPreset(preset);

	// --- Preset: Warm Room
	preset = new PresetInfo(index++, "Warm Room");
	initPresetParameters(preset->presetParameters);
	setPresetParameter(preset->presetParameters, controlID::kRT, 0.856598);
	setPresetParameter(preset->presetParameters, controlID::lpf_g, 0.400000);
	setPresetParameter(preset->presetParameters, controlID::lowShelf_fc, 144.000000);
	setPresetParameter(preset->presetParameters, controlID::lowShelfBoostCut_dB, -20.000000);
	setPresetParameter(preset->presetParameters, controlID::highShelf_fc, 2000.000000);
	setPresetParameter(preset->presetParameters, controlID::highShelfBoostCut_dB, -8.000000);
	setPresetParameter(preset->presetParameters, controlID::wetLevel_dB, -10.000000);
	setPresetParameter(preset->presetParameters, controlID::dryLevel_dB, 0.000000);
	setPresetParameter(preset->presetParameters, controlID::apfDelayMax_mSec, 33.000000);
	setPresetParameter(preset->presetParameters, controlID::apfDelayWeight_Pct, 84.000000);
	setPresetParameter(preset->presetParameters, controlID::fixeDelayMax_mSec, 82.000000);
	setPresetParameter(preset->presetParameters, controlID::fixeDelayWeight_Pct, 84.000000);
	setPresetParameter(preset->presetParameters, controlID::preDelayTime_mSec, 25.000000);
	setPresetParameter(preset->presetParameters, controlID::density, -0.000000);
	addPreset(preset);

	// --- Preset: Large Hall
	preset = new PresetInfo(index++, "Large Hall");
	initPresetParameters(preset->presetParameters);
	setPresetParameter(preset->presetParameters, controlID::kRT, 0.901248);
	setPresetParameter(preset->presetParameters, controlID::lpf_g, 0.300000);
	setPresetParameter(preset->presetParameters, controlID::lowShelf_fc, 150.000000);
	setPresetParameter(preset->presetParameters, controlID::lowShelfBoostCut_dB, -20.000000);
	setPresetParameter(preset->presetParameters, controlID::highShelf_fc, 4000.000000);
	setPresetParameter(preset->presetParameters, controlID::highShelfBoostCut_dB, -6.000000);
	setPresetParameter(preset->presetParameters, controlID::wetLevel_dB, -12.000000);
	setPresetParameter(preset->presetParameters, controlID::dryLevel_dB, 0.000000);
	setPresetParameter(preset->presetParameters, controlID::apfDelayMax_mSec, 33.000000);
	setPresetParameter(preset->presetParameters, controlID::apfDelayWeight_Pct, 85.000000);
	setPresetParameter(preset->presetParameters, controlID::fixeDelayMax_mSec, 81.000000);
	setPresetParameter(preset->presetParameters, controlID::fixeDelayWeight_Pct, 100.000000);
	setPresetParameter(preset->presetParameters, controlID::preDelayTime_mSec, 25.000000);
	setPresetParameter(preset->presetParameters, controlID::density, -0.000000);
	addPreset(preset);

	// --- Preset: Bright Club
	preset = new PresetInfo(index++, "Bright Club");
	initPresetParameters(preset->presetParameters);
	setPresetParameter(preset->presetParameters, controlID::kRT, 0.849129);
	setPresetParameter(preset->presetParameters, controlID::lpf_g, 0.535000);
	setPresetParameter(preset->presetParameters, controlID::lowShelf_fc, 500.000000);
	setPresetParameter(preset->presetParameters, controlID::lowShelfBoostCut_dB, -3.000000);
	setPresetParameter(preset->presetParameters, controlID::highShelf_fc, 4000.000000);
	setPresetParameter(preset->presetParameters, controlID::highShelfBoostCut_dB, 6.000000);
	setPresetParameter(preset->presetParameters, controlID::wetLevel_dB, -15.000000);
	setPresetParameter(preset->presetParameters, controlID::dryLevel_dB, 0.000000);
	setPresetParameter(preset->presetParameters, controlID::apfDelayMax_mSec, 33.000000);
	setPresetParameter(preset->presetParameters, controlID::apfDelayWeight_Pct, 85.000000);
	setPresetParameter(preset->presetParameters, controlID::fixeDelayMax_mSec, 81.000000);
	setPresetParameter(preset->presetParameters, controlID::fixeDelayWeight_Pct, 80.000000);
	setPresetParameter(preset->presetParameters, controlID::preDelayTime_mSec, 10.000000);
	setPresetParameter(preset->presetParameters, controlID::density, 1.000000);
	addPreset(preset);

	// --- Preset: Small Club
	preset = new PresetInfo(index++, "Small Club");
	initPresetParameters(preset->presetParameters);
	setPresetParameter(preset->presetParameters, controlID::kRT, 0.849129);
	setPresetParameter(preset->presetParameters, controlID::lpf_g, 0.535000);
	setPresetParameter(preset->presetParameters, controlID::lowShelf_fc, 500.000000);
	setPresetParameter(preset->presetParameters, controlID::lowShelfBoostCut_dB, -3.000000);
	setPresetParameter(preset->presetParameters, controlID::highShelf_fc, 4000.000000);
	setPresetParameter(preset->presetParameters, controlID::highShelfBoostCut_dB, -6.000000);
	setPresetParameter(preset->presetParameters, controlID::wetLevel_dB, -15.000000);
	setPresetParameter(preset->presetParameters, controlID::dryLevel_dB, 0.000000);
	setPresetParameter(preset->presetParameters, controlID::apfDelayMax_mSec, 33.000000);
	setPresetParameter(preset->presetParameters, controlID::apfDelayWeight_Pct, 85.000000);
	setPresetParameter(preset->presetParameters, controlID::fixeDelayMax_mSec, 81.000000);
	setPresetParameter(preset->presetParameters, controlID::fixeDelayWeight_Pct, 80.000000);
	setPresetParameter(preset->presetParameters, controlID::preDelayTime_mSec, 10.000000);
	setPresetParameter(preset->presetParameters, controlID::density, 1.000000);
	addPreset(preset);

	// --- Preset: Tiny Room
	preset = new PresetInfo(index++, "Tiny Room");
	initPresetParameters(preset->presetParameters);
	setPresetParameter(preset->presetParameters, controlID::kRT, 0.614246);
	setPresetParameter(preset->presetParameters, controlID::lpf_g, 0.270000);
	setPresetParameter(preset->presetParameters, controlID::lowShelf_fc, 150.000000);
	setPresetParameter(preset->presetParameters, controlID::lowShelfBoostCut_dB, -20.000000);
	setPresetParameter(preset->presetParameters, controlID::highShelf_fc, 4000.000000);
	setPresetParameter(preset->presetParameters, controlID::highShelfBoostCut_dB, 0.000000);
	setPresetParameter(preset->presetParameters, controlID::wetLevel_dB, -12.000000);
	setPresetParameter(preset->presetParameters, controlID::dryLevel_dB, 0.000000);
	setPresetParameter(preset->presetParameters, controlID::apfDelayMax_mSec, 37.000000);
	setPresetParameter(preset->presetParameters, controlID::apfDelayWeight_Pct, 23.270000);
	setPresetParameter(preset->presetParameters, controlID::fixeDelayMax_mSec, 87.500000);
	setPresetParameter(preset->presetParameters, controlID::fixeDelayWeight_Pct, 100.000000);
	setPresetParameter(preset->presetParameters, controlID::preDelayTime_mSec, 30.500000);
	setPresetParameter(preset->presetParameters, controlID::density, -0.000000);
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
    apiSpecificInfo.auBundleID = kAUBundleID;   /* MacOS only: this MUST match the bundle identifier in your info.plist file */

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
