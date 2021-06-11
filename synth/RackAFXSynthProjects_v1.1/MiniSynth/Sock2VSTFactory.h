#pragma once
#include "plugin.h"
#include "audioeffectx.h"

class Sock2VSTFactory
{
public:
	Sock2VSTFactory(CPlugIn* pPlugInBuddy, audioMasterCallback audioMaster);
	~Sock2VSTFactory(void);

	AudioEffect* getVSTPlugIn();

protected:
	CPlugIn* m_pPlugInBuddy;
	audioMasterCallback m_AudioMaster;
};
