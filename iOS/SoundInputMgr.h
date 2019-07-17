/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Version: v2019.1.1  Build: 6590
  Copyright (c) 2006-2019 Audiokinetic Inc.
*******************************************************************************/

#pragma once

#include "../Common/SoundInputMgrBase.h"

// SoundInputMgr definition.
class SoundInputMgr : public SoundInputMgrBase
{
public:
	// Get the static instance.
	static SoundInputMgrBase& Instance();

	virtual bool Initialize();
	virtual void Term();

	// Hidden constructor set to prevent multiple instances of SoundInputMgr.
private:
	SoundInputMgr() {}								// Private empty constructor
	virtual ~SoundInputMgr() {}
	SoundInputMgr(const SoundInputMgr&);			// Prevent copy-construction
	SoundInputMgr& operator=(const SoundInputMgr&);	// Prevent assignment
};
