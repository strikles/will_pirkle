#include "public.sdk/source/main/pluginfactoryvst3.h"
#include "VSTSynthProcessor.h"
#include "VSTSynthController.h"
#include "version.h"	// for versioning

#define stringPluginName "MiniSynth"

BEGIN_FACTORY_DEF ("Will Pirkle", 
				   "http://www.willpirkle.com", 
				   "mailto:will@willpirkle.com")

	// --- define our Processor Component; first argument is the FUID
	DEF_CLASS2 (INLINE_UID_FROM_FUID(Steinberg::Vst::MiniSynth::Processor::cid),
				PClassInfo::kManyInstances,//<- can have many instances
				kVstAudioEffectClass,
				stringPluginName,
				Vst::kDistributable,// <- our Processor and Controller are separate and distinct
				Vst::PlugType::kInstrumentSynth,
				FULL_VERSION_STR,
				kVstVersionString,
				Steinberg::Vst::MiniSynth::Processor::createInstance)

	// --- define our Controller Component; first argument is the FUID
	DEF_CLASS2 (INLINE_UID_FROM_FUID(Steinberg::Vst::MiniSynth::Controller::cid),
				PClassInfo::kManyInstances,
				kVstComponentControllerClass,
				stringPluginName,
				0,						// not used here
				"",						// not used here
				FULL_VERSION_STR,
				kVstVersionString,
				Steinberg::Vst::MiniSynth::Controller::createInstance)

END_FACTORY

bool InitModule () { return true; }
bool DeinitModule () { return true; }
