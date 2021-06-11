//-----------------------------------------------------------------------------
// Project     : VST SDK
// Version     : 3.6.0
//
// Category    : Examples
// Filename    : public.sdk/samples/vst/note_expression_synth/source/note_expression_synth_controller.cpp
// Created by  : Steinberg, 02/2010
// Description :
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2013, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "VSTSynthController.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/base/futils.h"
#include "pluginterfaces/vst/ivstmidicontrollers.h"
#include "base/source/fstring.h"
#include "logscale.h"
#include "base/source/fstreamer.h"
#include "synthfunctions.h"
#include "synthparamlimits.h" // param limits file

namespace Steinberg {
namespace Vst {
namespace AniSynth {

// --- the unique identifier (use guidgen.exe to generate)
FUID Controller::cid (0x1EF1BD21, 0x28794BA6, 0x8ED44629, 0xC7EF217C);

// this defines a logarithmig scaling for the filter Fc control
LogScale<ParamValue> filterLogScale (0.0,		/* VST GUI Variable MIN */
									 1.0,		/* VST GUI Variable MAX */
									 80.0,		/* filter fc LOW */
									 18000.0,	/* filter fc HIGH */
									 0.5,		/* the value at position 0.5 will be: */
									 1800.0);	/* 1800 Hz */

/*
	Controller::initialize()

	This is where we describe the Default GUI controls and link them to control IDs.
	This must be done if you want a Custom GUI too so you have to implement this no matter what.
*/
tresult PLUGIN_API Controller::initialize(FUnknown* context)
{
	// --- base class does its thing
	tresult result = EditController::initialize(context);

	// --- now define the controls
	if (result == kResultTrue)
	{
	// Init parameters
		Parameter* param;

		// --- These do not have to be in any particular order; but following the AU version too
				//	RangeParameter() constructor example:
		//	param = new RangeParameter(USTRING("Octave"), /* title */
		//						       OCTAVE,			  /* ID tag */
		//							   USTRING(""),		  /* units */
		//							   MIN_OCTAVE, MAX_OCTAVE, DEFAULT_OCTAVE); /* min, max, default */
		param = new RangeParameter(USTRING("Osc1 Output"), OSC1_AMPLITUDE_DB, USTRING("dB"),
								   MIN_OUTPUT_AMPLITUDE_DB, MAX_OUTPUT_AMPLITUDE_DB, DEFAULT_OUTPUT_AMPLITUDE_DB);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("Osc2 Output"), OSC2_AMPLITUDE_DB, USTRING("dB"),
								   MIN_OUTPUT_AMPLITUDE_DB, MAX_OUTPUT_AMPLITUDE_DB, DEFAULT_OUTPUT_AMPLITUDE_DB);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("OSC EG Int"), EG1_TO_OSC_INTENSITY, USTRING(""),
								   MIN_BIPOLAR, MAX_BIPOLAR, DEFAULT_BIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		//	LogScaleParameter() constructor example:
		//	param = new LogScaleParameter<ParamValue> (USTRING("Filter fc"),  /* title */
		//						                      FILTER_FC,			  /* ID tag */
		//											  filterLogScale,		  /* log scale descriptor, see above in file */
		//											  (USTRING("Hz"));        /* Units */
		param = new LogScaleParameter<ParamValue> (USTRING("Filter fc"), FILTER_FC, filterLogScale, (USTRING("Hz")));
		// NOTE: for LogScaleParameter, need to manually set default value (10kHz for filter fc)
		param->setNormalized(param->toNormalized(DEFAULT_FILTER_FC));
		param->setPrecision(1);
		parameters.addParameter (param);

		param = new RangeParameter(USTRING("Filter Q"), FILTER_Q, USTRING(""),
								   MIN_FILTER_Q, MAX_FILTER_Q, DEFAULT_FILTER_Q);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("Filter EG Int"), EG1_TO_FILTER_INTENSITY, USTRING(""),
								   MIN_BIPOLAR, MAX_BIPOLAR, DEFAULT_BIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("Filter KeyTrack Int"), FILTER_KEYTRACK_INTENSITY, USTRING(""),
								   MIN_FILTER_KEYTRACK_INTENSITY, MAX_FILTER_KEYTRACK_INTENSITY, DEFAULT_FILTER_KEYTRACK_INTENSITY);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("Attack"), EG1_ATTACK_MSEC, USTRING("mS"),
								   MIN_EG_ATTACK_TIME, MAX_EG_ATTACK_TIME, DEFAULT_EG_ATTACK_TIME);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("Decay/Release"), EG1_DECAY_RELEASE_MSEC, USTRING("mS"),
								   MIN_EG_DECAYRELEASE_TIME, MAX_EG_DECAYRELEASE_TIME, DEFAULT_EG_DECAYRELEASE_TIME);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("Sustain"), EG1_SUSTAIN_LEVEL, USTRING(""),
								   MIN_EG_SUSTAIN_LEVEL, MAX_EG_SUSTAIN_LEVEL, DEFAULT_EG_SUSTAIN_LEVEL);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		//	StringListParameter() constructor example:
		//	param = new StringListParameter (USTRING("Voice Mode"), /* title */
		//								     VOICE_MODE);			/* ID tag */
		StringListParameter* enumStringParam = new StringListParameter (USTRING("LFO Waveform"), LFO1_WAVEFORM);
		// now add the strings for the list IN THE SAME ORDER AS DECLARED IN THE enum in Synth Project
		enumStringParam->appendString(USTRING("sine"));
		enumStringParam->appendString(USTRING("usaw"));
		enumStringParam->appendString(USTRING("dsaw"));
		enumStringParam->appendString(USTRING("tri"));
		enumStringParam->appendString(USTRING("square"));
		enumStringParam->appendString(USTRING("expo"));
		enumStringParam->appendString(USTRING("rsh"));
		enumStringParam->appendString(USTRING("qrsh"));
		parameters.addParameter(enumStringParam);

		param = new RangeParameter(USTRING("LFO Rate"), LFO1_RATE, USTRING("Hz"),
								   MIN_LFO_RATE, MAX_LFO_RATE, DEFAULT_LFO_RATE);
		param->setPrecision(2); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("LFO Amp"), LFO1_AMPLITUDE, USTRING(""),
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("LFO Cutoff Int"), LFO1_TO_FILTER_INTENSITY,USTRING(""),
								   MIN_BIPOLAR, MAX_BIPOLAR, DEFAULT_BIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("LFO Pitch Int"), LFO1_TO_OSC_INTENSITY, USTRING(""),
								   MIN_BIPOLAR, MAX_BIPOLAR, DEFAULT_BIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("LFO Amp Int"), LFO1_TO_DCA_INTENSITY, USTRING(""),
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("LFO Pan Int"), LFO1_TO_PAN_INTENSITY, USTRING(""),
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("Volume"), OUTPUT_AMPLITUDE_DB, USTRING("dB"),
								   MIN_OUTPUT_AMPLITUDE_DB, MAX_OUTPUT_AMPLITUDE_DB, DEFAULT_OUTPUT_AMPLITUDE_DB);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("DCA EG Int"), EG1_TO_DCA_INTENSITY, USTRING(""),
								   MIN_BIPOLAR, MAX_BIPOLAR, MAX_BIPOLAR); // note def to 1
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		enumStringParam = new StringListParameter (USTRING("Vector Mode"), VOICE_MODE);
		// now add the strings for the list IN THE SAME ORDER AS DECLARED IN THE enum in Synth Project
		enumStringParam->appendString(USTRING("manual"));		// <- 0
		enumStringParam->appendString(USTRING("program"));		// <- 1
		parameters.addParameter (enumStringParam);

		param = new RangeParameter(USTRING("Detune"), DETUNE_CENTS, USTRING("cents"),
								   MIN_DETUNE_CENTS, MAX_DETUNE_CENTS, DEFAULT_DETUNE_CENTS);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("Portamento"), PORTAMENTO_TIME_MSEC, USTRING("mS"),
								   MIN_PORTAMENTO_TIME_MSEC, MAX_PORTAMENTO_TIME_MSEC, DEFAULT_PORTAMENTO_TIME_MSEC);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("Octave"), OCTAVE, USTRING(""),
								   MIN_OCTAVE, MAX_OCTAVE, DEFAULT_OCTAVE);
		param->setPrecision(0); // <--- this is an Integer (int) based value, so 0 = no fractional sig digits
		parameters.addParameter(param);

		enumStringParam = new StringListParameter (USTRING("Vel->Att Scale"), VELOCITY_TO_ATTACK);
		enumStringParam->appendString(USTRING("OFF"));
		enumStringParam->appendString(USTRING("ON"));
		parameters.addParameter(enumStringParam);

		enumStringParam = new StringListParameter (USTRING("Note->Dcy Scale"), NOTE_NUM_TO_DECAY);
		enumStringParam->appendString(USTRING("OFF"));
		enumStringParam->appendString(USTRING("ON"));
		parameters.addParameter(enumStringParam);

		enumStringParam = new StringListParameter (USTRING("ResetToZero"), RESET_TO_ZERO);
		enumStringParam->appendString(USTRING("OFF"));
		enumStringParam->appendString(USTRING("ON"));
		parameters.addParameter(enumStringParam);

		enumStringParam = new StringListParameter (USTRING("Filter KeyTrack"), FILTER_KEYTRACK);
		enumStringParam->appendString(USTRING("OFF"));
		enumStringParam->appendString(USTRING("ON"));
		parameters.addParameter(enumStringParam);

		enumStringParam = new StringListParameter (USTRING("Legato Mode"), LEGATO_MODE);
		enumStringParam->appendString(USTRING("mono"));
		enumStringParam->appendString(USTRING("legato"));
		parameters.addParameter(enumStringParam);

		param = new RangeParameter(USTRING("PBend Range"), PITCHBEND_RANGE, USTRING(""),
								   MIN_PITCHBEND_RANGE, MAX_PITCHBEND_RANGE, DEFAULT_PITCHBEND_RANGE);
		param->setPrecision(0); // integer!
		parameters.addParameter(param);

		// added for vector joystick support, VST3 only
		param = new RangeParameter(USTRING("VectorJoystick X"), VECTORJOYSTICK_X, USTRING(""), MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR_HALF);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("VectorJoystick Y"), VECTORJOYSTICK_Y, USTRING(""), MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR_HALF);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		enumStringParam = new StringListParameter (USTRING("Path Mode"), PATH_MODE);
		// now add the strings for the list IN THE SAME ORDER AS DECLARED IN THE enum in Synth Project
		enumStringParam->appendString(USTRING("once"));	// <- 0
		enumStringParam->appendString(USTRING("loop"));		// <- 1
		enumStringParam->appendString(USTRING("sustain"));	// <- 3
		enumStringParam->appendString(USTRING("bckfrth"));	// <- 4
		parameters.addParameter (enumStringParam);

		param = new RangeParameter(USTRING("Orbit X"), ORBIT_X, USTRING(""), MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("Orbit Y"), ORBIT_Y, USTRING(""), MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("Rotor Rate"), ROTOR_RATE, USTRING("Hz"), MIN_SLOW_LFO_RATE, MAX_SLOW_LFO_RATE, DEFAULT_SLOW_LFO_RATE);
		param->setPrecision(2); // fractional sig digits
		parameters.addParameter(param);

		enumStringParam = new StringListParameter (USTRING("Rotor Waveform"), ROTOR_WAVEFORM);
		enumStringParam->appendString(USTRING("sine"));
		enumStringParam->appendString(USTRING("usaw"));
		enumStringParam->appendString(USTRING("dsaw"));
		enumStringParam->appendString(USTRING("tri"));
		enumStringParam->appendString(USTRING("square"));
		enumStringParam->appendString(USTRING("expo"));
		enumStringParam->appendString(USTRING("rsh"));
		enumStringParam->appendString(USTRING("qrsh"));
		parameters.addParameter(enumStringParam);

		// MIDI Params - these have no knobs in main GUI but do have to appear in default
		// NOTE: this is for VST3 ONLY! Not needed in AU or RAFX
		param = new RangeParameter(USTRING("PitchBend"), MIDI_PITCHBEND, USTRING(""),
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("MIDI Vol"), MIDI_VOLUME_CC7, USTRING(""),
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("MIDI Pan"), MIDI_PAN_CC10, USTRING(""),
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("MIDI Wheel"), MIDI_MODWHEEL, USTRING(""),
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("MIDI Expression"), MIDI_EXPRESSION_CC11, USTRING(""),
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("MIDI Channel Pressure"), MIDI_CHANNEL_PRESSURE, USTRING(""), 
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("MIDI Sustain Pedal"), MIDI_SUSTAIN_PEDAL, USTRING(""), 
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);

		param = new RangeParameter(USTRING("All Notes Off"), MIDI_ALL_NOTES_OFF, USTRING(""), 
								   MIN_UNIPOLAR, MAX_UNIPOLAR, DEFAULT_UNIPOLAR);
		param->setPrecision(1); // fractional sig digits
		parameters.addParameter(param);
	}

	return kResultTrue;
}

/*
	Controller::terminate()
	The End.
*/
tresult PLUGIN_API Controller::terminate()
{
	return EditController::terminate();
}

/*
	Controller::setParamNormalizedFromFile()
	This sets the normalized (raw) parameter value
	from the file's cooked (plain) value

	tag - the Index value of a control
	value - the cooked value from the file
*/
tresult PLUGIN_API Controller::setParamNormalizedFromFile(ParamID tag, ParamValue value)
{
	// --- get the parameter
	Parameter* pParam = EditController::getParameterObject(tag);

	// --- verify pointer
	if(!pParam) return kResultFalse;

	//  --- convert serialized value to normalized (raw)
	return setParamNormalized(tag, pParam->toNormalized(value));
}

// NEED TO IMPLEMENMT THISSS! For restoring the UI state
// it is already done in the processor correctly but needs to be set here too
tresult PLUGIN_API Controller::setComponentState(IBStream* fileStream)
{
	IBStreamer s(fileStream, kLittleEndian);
	uint64 version = 0;
	double dDoubleParam = 0;

	// --- needed to convert to our UINT reads
	uint32 udata = 0;
	int32 data = 0;

	// read the version
	if(!s.readInt64u(version)) return kResultFalse;

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(OSC1_AMPLITUDE_DB, dDoubleParam);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(OSC2_AMPLITUDE_DB, dDoubleParam);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(EG1_TO_OSC_INTENSITY, dDoubleParam);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(FILTER_FC, dDoubleParam);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(FILTER_Q, dDoubleParam);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(EG1_TO_FILTER_INTENSITY, dDoubleParam);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(FILTER_KEYTRACK_INTENSITY, dDoubleParam);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(EG1_ATTACK_MSEC, dDoubleParam);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(EG1_DECAY_RELEASE_MSEC, dDoubleParam);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(EG1_SUSTAIN_LEVEL, dDoubleParam);

	if(!s.readInt32u(udata)) return kResultFalse;
	else
		setParamNormalizedFromFile(LFO1_WAVEFORM, (ParamValue)udata);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(LFO1_RATE, dDoubleParam);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(LFO1_AMPLITUDE, dDoubleParam);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(LFO1_TO_FILTER_INTENSITY, dDoubleParam);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(LFO1_TO_OSC_INTENSITY, dDoubleParam);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(LFO1_TO_DCA_INTENSITY, dDoubleParam);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(LFO1_TO_PAN_INTENSITY, dDoubleParam);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(OUTPUT_AMPLITUDE_DB, dDoubleParam);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(EG1_TO_DCA_INTENSITY, dDoubleParam);

	if(!s.readInt32u(udata)) return kResultFalse;
	else
		setParamNormalizedFromFile(VOICE_MODE, (ParamValue)udata);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(DETUNE_CENTS, dDoubleParam);

	if(!s.readDouble(dDoubleParam)) return kResultFalse;
	else
		setParamNormalizedFromFile(PORTAMENTO_TIME_MSEC, dDoubleParam);

	if(!s.readInt32u(udata)) return kResultFalse;
	else
		setParamNormalizedFromFile(OCTAVE, (ParamValue)udata);

	if(!s.readInt32u(udata)) return kResultFalse;
	else
		setParamNormalizedFromFile(VELOCITY_TO_ATTACK, (ParamValue)udata);

	if(!s.readInt32u(udata)) return kResultFalse;
	else
		setParamNormalizedFromFile(NOTE_NUM_TO_DECAY, (ParamValue)udata);

	if(!s.readInt32u(udata)) return kResultFalse;
	else
		setParamNormalizedFromFile(RESET_TO_ZERO, (ParamValue)udata);

	if(!s.readInt32u(udata)) return kResultFalse;
	else
		setParamNormalizedFromFile(FILTER_KEYTRACK, (ParamValue)udata);

	if(!s.readInt32u(udata)) return kResultFalse;
	else
		setParamNormalizedFromFile(LEGATO_MODE, (ParamValue)udata);

	if(!s.readInt32(data)) return kResultFalse;
	else
		setParamNormalizedFromFile(PITCHBEND_RANGE, (ParamValue)data);

	// digisynth stuff
	if(!s.readInt32u(udata))
		return kResultFalse;
	else
		setParamNormalizedFromFile(LOOP_MODE, (ParamValue)udata);

	// vector synth stuff
	if(!s.readDouble(dDoubleParam))
		return kResultFalse;
	else
		setParamNormalizedFromFile(VECTORJOYSTICK_X, dDoubleParam);

	if(!s.readDouble(dDoubleParam))
		return kResultFalse;
	else
		setParamNormalizedFromFile(VECTORJOYSTICK_Y, dDoubleParam);

	if(!s.readInt32u(udata))
		return kResultFalse;
	else
		setParamNormalizedFromFile(PATH_MODE, (ParamValue)udata);

	if(!s.readDouble(dDoubleParam))
		return kResultFalse;
	else
		setParamNormalizedFromFile(ORBIT_X, dDoubleParam);

	if(!s.readDouble(dDoubleParam))
		return kResultFalse;
	else
		setParamNormalizedFromFile(ORBIT_Y, dDoubleParam);

	if(!s.readDouble(dDoubleParam))
		return kResultFalse;
	else
		setParamNormalizedFromFile(ROTOR_RATE, dDoubleParam);

	if(!s.readInt32u(udata))
		return kResultFalse;
	else
		setParamNormalizedFromFile(ROTOR_WAVEFORM, (ParamValue)udata);

	// --- do next version...
	if (version >= 1)
	{
		// add v1 stuff here
	}

	return kResultTrue;
}


/*
	Controller::getParamNormalized()
	Called by client to get the noramalized parameter for drawing the GUI.
*/
ParamValue PLUGIN_API Controller::getParamNormalized(ParamID tag)
{
	// ordinarily the base class can handle this on its own, since we set the min,max,default
	// when we constructed the params. But if you need to massage the value any more, here
	// is where you can do it.
	ParamValue paramValue = EditController::getParamNormalized(tag);

	//switch(tag) // same as RAFX uControlID
	//{
	//	case VOICE_MODE:
	//	{
	//		// get the parameter, then do something with it...
	//		Parameter* pParam = EditController::getParameterObject(tag);

	//		ParamValue = ...
	//		break;
	//	}

	return paramValue;
}

/*
	Controller::setParamNormalized()
	Called by GUI when a control is changed to update itself.
*/
tresult PLUGIN_API Controller::setParamNormalized (ParamID tag, ParamValue value)
{
	// --- this is called to update the GUI, eg when you turn a knob, this would
	//     update the edit control with the new value
	return EditController::setParamNormalized(tag, value);;
}

/* --- IMIDIMapping Interface
	Controller::getMidiControllerAssignment()

	The client queries this 129 times for 130 possible control messages, see ivstsmidicontrollers.h for 
	the VST defines for kPitchBend, kCtrlModWheel, etc... for each MIDI Channel in our Event Bus

	We respond with our ControlID value that we'll use to process the MIDI Messages in Processor::process().

	On the default GUI, these controls will actually move with the MIDI messages, but we don't want that on 
	the final UI so that we can have any Modulation Matrix mapping we want.
*/
tresult PLUGIN_API Controller::getMidiControllerAssignment(int32 busIndex, int16 channel, CtrlNumber midiControllerNumber, ParamID& id/*out*/)
{
	// NOTE: we only have one EventBus(0)
	//       but it has 16 channels on it
	if(busIndex == 0)
	{
		id = -1;
		switch (midiControllerNumber)
		{
			// these messages handled in the Processor::process() method
			case kPitchBend: 
				id = MIDI_PITCHBEND; 
				break;
			case kCtrlModWheel: 
				id = MIDI_MODWHEEL; 
				break;
			case kCtrlVolume: 
				id = MIDI_VOLUME_CC7; 
				break;
			case kCtrlPan: 
				id = MIDI_PAN_CC10; 
				break;
			case kCtrlExpression: 
				id = MIDI_EXPRESSION_CC11; 
				break;
			case kAfterTouch: 
				id = MIDI_CHANNEL_PRESSURE; 
				break;
			case kCtrlSustainOnOff: 
				id = MIDI_SUSTAIN_PEDAL; 
				break;
			case kCtrlAllNotesOff: 
				id = MIDI_ALL_NOTES_OFF; 
				break;
		}
		if (id == -1)
		{
			id = 0;
			return kResultFalse;
		}
		else
			return kResultTrue;
	}

	return kResultFalse;
}
/*
	Controller::createView()
	Create our VSTGUI editor from the UIDESC file.
*/
IPlugView* PLUGIN_API Controller::createView(FIDString _name)
{
	ConstString name(_name);
	if(name == ViewType::kEditor)
	{
		// --- create the editor using the .uidesc file (in our Resources, edited with WYSIWYG editor in client)
		return new VST3Editor (this, "Editor", "vstgui.uidesc");
	}
	return 0;
}

IController* Controller::createSubController (UTF8StringPtr _name, const IUIDescription* description, VST3Editor* editor)
{
	// --- get the name string to test
	ConstString name (_name);

	if (name == "VectorJoystick")
	{
		Parameter* jsX = getParameterObject(VECTORJOYSTICK_X);
		Parameter* jsY = getParameterObject(VECTORJOYSTICK_Y);
		PadController* padController = new PadController(editor, this, jsX, jsY);
		return padController;
	}

	return NULL;
}
}}} // namespaces
