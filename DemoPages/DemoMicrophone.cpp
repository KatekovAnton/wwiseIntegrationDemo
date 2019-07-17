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

// DemoMicrophone.cpp
/// \file 
/// Defines the methods declared in DemoMicrophone.h.

#include "stdafx.h"

#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine

#include "../WwiseProject/GeneratedSoundBanks/Wwise_IDs.h"		// IDs generated by Wwise
#include "Menu.h"
#include "DemoMicrophone.h"
#include "SoundInputMgr.h"

//The game object ID used for this demo, it can be anything (except 0 and 1, as stated in the RegisterGameObj() doc)
#define MICRO_GAME_OBJECT 1234

//////////////////////////////////////////////////////////////////////////
// Important note!  
// The microphone code is very platform-specific.  You will find the
// related code in SoundInput and SoundInputMgr classes, in the platform
// sub-folders of the Integration Demo.
//////////////////////////////////////////////////////////////////////////

DemoMicrophone::DemoMicrophone( Menu& in_ParentMenu ):Page( in_ParentMenu, "Microphone Demo" )
{
	m_bPlaying = false;
	m_bDelayed = false;
	m_szHelp  = "This demo shows how to use use the AudioInput plugin "
				"to feed the microphone data to Wwise.  Simply play the "
				"microphone event.  There is a delay effect that you "
				"can enable or disable to help distinguish between the "
				"real sound and the recording.";

	m_pInput = &SoundInput::Instance();
}

DemoMicrophone::~DemoMicrophone()
{
	if( m_pInput )
	{
		m_pInput->InputOff();
	}
}

bool DemoMicrophone::Init()
{
#ifdef AK_IOS
	
#endif // #ifdef AK_IOS
	
	if (!SoundInputMgr::Instance().Initialize())
	{
		SetErrorMessage(" Unable to Initialize Microphones.");
		return false;
	}

	// Load the sound bank
	AkBankID bankID; // Not used
	if ( AK::SoundEngine::LoadBank( "Microphone.bnk", AK_DEFAULT_POOL_ID, bankID ) != AK_Success )
	{
		SetLoadFileErrorMessage( "Microphone.bnk" );
		return false;
	}

	// Register the "Micro" game object on which the sound recorded from the micro will be played
	AK::SoundEngine::RegisterGameObj( MICRO_GAME_OBJECT, "Micro" );

	// Initialize the page
	return Page::Init();
}

void DemoMicrophone::Release()
{
	if( m_bPlaying )
		StopRecording();

	SoundInputMgr::Instance().Term();
	AK::SoundEngine::UnregisterGameObj( MICRO_GAME_OBJECT );
	AK::SoundEngine::UnloadBank( "Microphone.bnk", NULL );
	Page::Release();
}

void DemoMicrophone::Draw()
{
	Page::Draw();
}

void DemoMicrophone::InitControls()
{
	ButtonControl* newBtn;

	// Create the "Play Markers" button
	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Start Recording" );
	newBtn->SetDelegate( (PageMFP)&DemoMicrophone::PlayMicroButton_Pressed );
	m_Controls.push_back( newBtn );

	// Create the "Play Markers" button
	newBtn = new ButtonControl( *this );
	newBtn->SetLabel( "Enable Delay" );
	newBtn->SetDelegate( (PageMFP)&DemoMicrophone::DelayButton_Pressed );
	m_Controls.push_back( newBtn );
}
void DemoMicrophone::StartRecording()
{
	m_bPlaying = true;

	AkPlayingID playId = AK::SoundEngine::PostEvent( AK::EVENTS::PLAY_MICROPHONE, MICRO_GAME_OBJECT );
	m_pInput->SetPlayingID(playId);
}

void DemoMicrophone::StopRecording()
{
	m_bPlaying = false;

	AK::SoundEngine::PostEvent( AK::EVENTS::STOP_MICROPHONE, MICRO_GAME_OBJECT );

	// Call RenderAudio() here to tell the sound engine to process the STOP_MICROPHONE Event before.
	// InpufOff will be called.
	// Not calling it could cause a plugin to be terminated by the call to input Off, and could 
	// result into Notifications of the plug-in to have exited abnormally.
	AK::SoundEngine::RenderAudio();

	if( m_pInput )
		m_pInput->InputOff();
}

void DemoMicrophone::PlayMicroButton_Pressed( void* in_pSender, ControlEvent* )
{
	ButtonControl* sender = (ButtonControl*)in_pSender;
	if ( !m_bPlaying )
	{
		if (m_pInput && m_pInput->InputOn(/*We only support one microphone*/))
		{
			sender->SetLabel( "Stop Recording" );
			StartRecording();
		}
		else
		{
			SetErrorMessage("Could not start recording of microphone");
			return;
		}
	}
	else
	{
		
		sender->SetLabel( "Start Recording" );

		StopRecording();
	}
}

void DemoMicrophone::DelayButton_Pressed( void* in_pSender, ControlEvent* )
{
	ButtonControl* sender = (ButtonControl*)in_pSender;
	if ( !m_bDelayed )
	{
		m_bDelayed = true;
		sender->SetLabel( "Disable Delay" );

		AK::SoundEngine::PostEvent( AK::EVENTS::ENABLE_MICROPHONE_DELAY, MICRO_GAME_OBJECT );
	}
	else
	{
		m_bDelayed = false;
		sender->SetLabel( "Enable Delay" );

		AK::SoundEngine::PostEvent( AK::EVENTS::DISABLE_MICROPHONE_DELAY, MICRO_GAME_OBJECT );
	}
}
