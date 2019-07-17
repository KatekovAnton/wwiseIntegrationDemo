/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Version: v2019.1.1  Build: 6977
  Copyright (c) 2006-2019 Audiokinetic Inc.
*******************************************************************************/

#include "stdafx.h"
#include "SoundInputBase.h"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// SoundInputBase implementation
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
SoundInputBase::SoundInputBase()
	:m_playingID(AK_INVALID_PLAYING_ID)
{
}

SoundInputBase::~SoundInputBase() 
{
}

void SoundInputBase::SetPlayingID( AkPlayingID in_playingID )
{
	m_playingID = in_playingID;
}

AkPlayingID SoundInputBase::GetPlayingID()
{
	return m_playingID;
}
