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

// DemoMIDIMetronome.cpp
/// \file 
/// Defines the methods declared in DemoMIDIMetronome.h.

#include "stdafx.h"

#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine

#include "../WwiseProject/GeneratedSoundBanks/Wwise_IDs.h"		// IDs generated by Wwise
#include "Menu.h"
#include "DemoMIDIMetronome.h"


DemoMIDIMetronome* DemoMIDIMetronome::m_pCallbackObj = NULL;
CAkLock DemoMIDIMetronome::m_lockCallback;

#define Min( _a_, _b_ ) ( (_a_) < (_b_) ? _a_ : _b_ )
#define Max( _a_, _b_ ) ( (_a_) > (_b_) ? _a_ : _b_ )

/////////////////////////////////////////////////////////////////////
// DemoMIDIMetronome Public Methods
/////////////////////////////////////////////////////////////////////

DemoMIDIMetronome::DemoMIDIMetronome( Menu& in_ParentMenu )
	: Page( in_ParentMenu, "MIDI API Demo (Metronome)" )
	, m_eventID( AK_INVALID_UNIQUE_ID)
	, m_uPostCntr(0)
	, m_uPostLenSamples( 512 )
	, m_uSamplesPerCallback(512)
	, m_uCallbackCntr(0)
	, m_dblMsPerCallback(10)
	, m_dblInterPostTimeMs(1000)
	, m_dblNextPostTimeMs(0)
{
	m_szHelp  = "This page demonstrates the use of the MIDI API. "
				"Press the \"Start Metronome\" button to simulate "
				"an active metronome.  Then select the \"BPM\" slider "
				"and press LEFT or RIGHT on the <<DIRECTIONAL_TYPE>> "
				"to change its value.\n\n"
				"The demo uses a registered callback function to "
				"post MIDI events to the sound engine via the"
				"\"PostMIDIOnEvent\" function.";
}

bool DemoMIDIMetronome::Init()
{
	bool bResult = true;

	// Load the sound bank
	AkBankID bankID; // Not used
	if ( AK::SoundEngine::LoadBank( "Metronome.bnk", AK_DEFAULT_POOL_ID, bankID ) != AK_Success )
	{
		SetLoadFileErrorMessage( "Metronome.bnk" );
		bResult = false;
	}
	
	// Register the "Metronome" game object
	if ( AK::SoundEngine::RegisterGameObj( GAME_OBJECT_METRONOME, "Metronome" ) != AK_Success )
	{
		SetErrorMessage( "Can't register Game Object" );
		bResult = false;
	}

	// Undo init if something went wrong
	if ( ! bResult )
	{
		AK::SoundEngine::UnregisterGameObj( GAME_OBJECT_METRONOME );
		AK::SoundEngine::UnloadBank( "Metronome.bnk", NULL );
		return false;
	}

	// Initialize the page
	m_bPlayingMetronome = false;
	return Page::Init();
}

void DemoMIDIMetronome::Release()
{
	// Register callback function
	ReleaseCallback();

	// Stop all MIDI posts
	StopMIDIPosts();

	// Unregister the Car game object
	AK::SoundEngine::UnregisterGameObj( GAME_OBJECT_METRONOME );

	// Unload the soundbank
	AK::SoundEngine::UnloadBank( "Metronome.bnk", NULL );

	// Release the page
	Page::Release();
}


/////////////////////////////////////////////////////////////////////
// DemoMIDIMetronome Private Methods
/////////////////////////////////////////////////////////////////////

void DemoMIDIMetronome::InitControls()
{
	ButtonControl* newButton;
	NumericControl* newNumeric;

	// Add the Start Engine button
	newButton = new ButtonControl( *this );
	newButton->SetLabel( "Start Metronome" );
	newButton->SetDelegate( (PageMFP)&DemoMIDIMetronome::PlayMetronomeButton_Pressed );
	m_Controls.push_back( newButton );
	
	// Add the RPM Numeric control
	newNumeric = new NumericControl( *this );
	newNumeric->SetLabel( "BPM:" );
	newNumeric->SetMinValue( 1 );
	newNumeric->SetMaxValue( 960 );
	newNumeric->SetValue( 60 );
	newNumeric->SetIncrement( 1 );
	newNumeric->SetInitialIncrement( 1 );
	newNumeric->SetDelegate( (PageMFP)&DemoMIDIMetronome::BPMNumeric_ValueChanged );
	newNumeric->CallDelegate( NULL ); // Force the BPM to the correct initial value
	m_Controls.push_back( newNumeric );
}

void DemoMIDIMetronome::PlayMetronomeButton_Pressed( void* in_pSender, ControlEvent* )
{
	ButtonControl* sender = (ButtonControl*)in_pSender;

	if ( m_bPlayingMetronome )
		sender->SetLabel( "Start Metronome" );
	else
		sender->SetLabel( "Stop Metronome" );

	m_bPlayingMetronome = !m_bPlayingMetronome;

	{
		m_lockCallback.Lock();

		if ( m_bPlayingMetronome )
			PrepareCallback();
		else
			ReleaseCallback();

		m_lockCallback.Unlock();
	}
}

void DemoMIDIMetronome::BPMNumeric_ValueChanged( void* in_pSender, ControlEvent* )
{
	NumericControl* pCtrl = (NumericControl*)in_pSender;

	{
		m_lockCallback.Lock();

		m_dblInterPostTimeMs = 60000 / (AkReal64)pCtrl->GetValue();

		AkReal64 dblThisTimeMs = (AkReal64)m_uCallbackCntr * m_dblMsPerCallback;
		AkReal64 dblMaybeNextMs = dblThisTimeMs + m_dblInterPostTimeMs;
		if ( dblMaybeNextMs < m_dblNextPostTimeMs )
			m_dblNextPostTimeMs = dblMaybeNextMs;

		AkReal64 dblPostLengthMs = Min( m_dblMsPerCallback, m_dblInterPostTimeMs );
		m_uPostLenSamples = (AkUInt32)( (dblPostLengthMs / m_dblMsPerCallback) * m_uSamplesPerCallback );
		m_uPostLenSamples = Min( m_uPostLenSamples, m_uSamplesPerCallback );

		m_lockCallback.Unlock();
	}
}

bool DemoMIDIMetronome::PrepareCallback()
{
	// Get ID of event used to post MIDI
	m_eventID = AK::SoundEngine::GetIDFromString( "Metronome_PostMIDI" );

	// Get audio settings info; this is thread safe
	AkAudioSettings audioSettings;
	if ( AK::SoundEngine::GetAudioSettings( audioSettings ) != AK_Success )
		return false;

	// Get lock and setup callback
	m_lockCallback.Lock();

	bool bResult = true;

	if ( m_pCallbackObj != NULL )
		bResult = false;
	else
	{
		if ( AK::SoundEngine::RegisterGlobalCallback( &StaticCallback, AkGlobalCallbackLocation_PreProcessMessageQueueForRender ) != AK_Success )
			bResult = false;
		else
			m_pCallbackObj = this;
	}

	// Setup callback context
	if ( bResult == true )
	{
		m_uSamplesPerCallback = audioSettings.uNumSamplesPerFrame;
		m_uPostLenSamples = Max( m_uPostLenSamples, m_uSamplesPerCallback );
		m_uCallbackCntr = 0;
		m_dblMsPerCallback = ( (AkReal64)audioSettings.uNumSamplesPerFrame / (AkReal64)audioSettings.uNumSamplesPerSecond ) * 1000;
		// m_dblInterPostTimeMs : done by delegate at init
		m_dblNextPostTimeMs = 0;
	}

	m_lockCallback.Unlock();

	return bResult;
}

void DemoMIDIMetronome::ReleaseCallback()
{
	// Get lock and release callback
	{
		m_lockCallback.Lock();

		if ( m_pCallbackObj == this )
		{
			AK::SoundEngine::UnregisterGlobalCallback( &StaticCallback, AkGlobalCallbackLocation_PreProcessMessageQueueForRender );
			m_pCallbackObj = NULL;
		}

		m_lockCallback.Unlock();
	}
}

void DemoMIDIMetronome::ObjectCallback()
{
	// Calculate current frame and next frame times
	AkReal64 dblThisTimeMs = (AkReal64)m_uCallbackCntr * m_dblMsPerCallback;
	AkReal64 dblNextTimeMs = dblThisTimeMs + m_dblMsPerCallback;

	// Failsafe!
	if ( dblThisTimeMs > m_dblNextPostTimeMs )
	{
		m_dblNextPostTimeMs = dblThisTimeMs;
	}

	// Must we post this frame?
	if ( m_dblNextPostTimeMs >= dblThisTimeMs && m_dblNextPostTimeMs <= dblNextTimeMs )
	{
		// Calculate sample offset, relative to current frame, to post MIDI eventss
		AkReal64 dblPercentOffset = (m_dblNextPostTimeMs - dblThisTimeMs) / m_dblMsPerCallback;
		AkUInt32 uSampleOffset = (AkUInt32)( dblPercentOffset * m_uSamplesPerCallback );

		// Post MIDI note-on and note-off
		PostMIDIEvents( uSampleOffset );

		// Update post time in context
		m_dblNextPostTimeMs += m_dblInterPostTimeMs;
	}

	// Update context
	++m_uCallbackCntr;
}

void DemoMIDIMetronome::StaticCallback(AK::IAkGlobalPluginContext *, AkGlobalCallbackLocation, void *)
{
	// Must seize lock because of multi-threading
	{
		m_lockCallback.Lock();

		if ( m_pCallbackObj )
			m_pCallbackObj->ObjectCallback();

		m_lockCallback.Unlock();
	}
}

void DemoMIDIMetronome::PostMIDIEvents( AkUInt32 in_uSampleOffset )
{
	AkMIDIPost aPosts[2];

	const AkUInt8 byNote = ( ( m_uPostCntr % 4 ) == 0 ) ? 70 : 60;

	// Note-on
	AkMIDIPost& noteOn = aPosts[0];
	noteOn.byType = AK_MIDI_EVENT_TYPE_NOTE_ON;
	noteOn.byChan = 0;
	noteOn.NoteOnOff.byNote = byNote;
	noteOn.NoteOnOff.byVelocity = 72;
	noteOn.uOffset = in_uSampleOffset;

	// Note-off
	AkMIDIPost& noteOff = aPosts[1];
	noteOff.byType = AK_MIDI_EVENT_TYPE_NOTE_OFF;
	noteOff.byChan = 0;
	noteOff.NoteOnOff.byNote = byNote;
	noteOff.NoteOnOff.byVelocity = 0;
	noteOff.uOffset = in_uSampleOffset + m_uPostLenSamples;

	// Post events
	AK::SoundEngine::PostMIDIOnEvent( m_eventID, GAME_OBJECT_METRONOME, aPosts, 2 );

	// Call the sound engine to render audio (to process posted events)
	//
	//	VERY IMPORTANT: do not allow RenderAudio to generate an audio frame!!
	//		We are here because of a callback from RenderAudio, which is
	//		already about to generate an audio frame.  We simply want to
	//		add to that soon-to-be-generated frame.
	AK::SoundEngine::RenderAudio( false );

	// Update post counter
	++m_uPostCntr;
}

void DemoMIDIMetronome::StopMIDIPosts()
{
	// Must seize lock because of multi-threading
	{
		m_lockCallback.Lock();
		if (m_eventID != AK_INVALID_UNIQUE_ID)
			AK::SoundEngine::StopMIDIOnEvent( m_eventID, GAME_OBJECT_METRONOME );

		m_lockCallback.Unlock();
	}
}
