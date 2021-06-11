// --- CMAKE generated variables for your plugin

#include "pluginstructures.h"

#ifndef _plugindescription_h
#define _plugindescription_h

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)
#define AU_COCOA_VIEWFACTORY_STRING STR(AU_COCOA_VIEWFACTORY_NAME)
#define AU_COCOA_VIEW_STRING STR(AU_COCOA_VIEW_NAME)

// --- AU Plugin Cocoa View Names (flat namespace) 
#define AU_COCOA_VIEWFACTORY_NAME AUCocoaViewFactory_C6AAC0E6FB63334AB0399EA06C3D2323
#define AU_COCOA_VIEW_NAME AUCocoaView_C6AAC0E6FB63334AB0399EA06C3D2323

// --- BUNDLE IDs (MacOS Only) 
const char* kAAXBundleID = "developer.aax.moogbiquadladder.bundleID";
const char* kAUBundleID = "developer.au.moogbiquadladder.bundleID";
const char* kVST3BundleID = "developer.vst3.moogbiquadladder.bundleID";

// --- Plugin Names 
const char* kPluginName = "MoogBiquadLadder";
const char* kShortPluginName = "MoogBiquadLadde";
const char* kAUBundleName = "MoogBiquadLadder_AU";

// --- Plugin Type 
const pluginType kPluginType = pluginType::kFXPlugin;

// --- VST3 UUID 
const char* kVSTFUID = "{c6aac0e6-fb63-334a-b039-9ea06c3d2323}";

// --- 4-char codes 
const int32_t kFourCharCode = 'MogL';
const int32_t kAAXProductID = 'MogL';
const int32_t kManufacturerID = 'ASPK';

// --- Vendor information 
const char* kVendorName = "ASPiK User";
const char* kVendorURL = "www.yourcompany.com";
const char* kVendorEmail = "help@yourcompany.com";

// --- Plugin Options 
const bool kWantSidechain = false;
const uint32_t kLatencyInSamples = 0;
const double kTailTimeMsec = 0.000000;
const bool kVSTInfiniteTail = false;
const bool kVSTSAA = false;
const uint32_t kVST3SAAGranularity = 1;
const uint32_t kAAXCategory = aaxPlugInCategory_EQ;

#endif
