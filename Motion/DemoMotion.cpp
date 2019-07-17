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

// DemoMotion.cpp
/// \file
/// Defines all methods declared in DemoMotion.h.

#include "stdafx.h"

#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine

#include "Wwise_IDs.h"		// IDs generated by Wwise
#include "Menu.h"
#include "DemoMotion.h"
#include "IntegrationDemo.h"
#include "InputMgr.h"

#ifdef AK_NX
#include <nn/hid/hid_NpadJoyCommon.h>
#include <nn/hid/hid_NpadFullKey.h>
#include <nn/hid/hid_NpadJoyRight.h>
#include <nn/hid/hid_NpadJoyLeft.h>
#include <nn/hid/hid_NpadJoyDual.h>
#endif

/////////////////////////////////////////////////////////////////////
// DemoMotion Constant Member Initialization
/////////////////////////////////////////////////////////////////////
const AkGameObjectID DemoMotion::OBJ_FOR_PLAYER[MAX_PLAYERS] = { 200, 300, 400, 500 };	//Player-specific gameobjects/

//These defines are there as default for all platforms.
//Some platforms have a different way of specifying the player ID or the controller output type.  Check in ..\[PlatformName]\Plarform.h
#ifndef GET_PLAYER_ID
#define GET_PLAYER_ID(_index) _index
#endif

#if defined AK_XBOXONE
#define CONTROLLER_OUTPUT "Controller_Headphones"
#elif defined AK_PS4
#define CONTROLLER_OUTPUT "Pad_Output"
#endif

/////////////////////////////////////////////////////////////////////
// DemoMotion Public Methods
/////////////////////////////////////////////////////////////////////

DemoMotion::DemoMotion( Menu& in_ParentMenu ):MultiplayerPage( in_ParentMenu, "Motion Demo" )
{
	m_szHelp  = "This multiplayer demonstration shows Wwise's capabilities to also emit output "
				"as motion, in this case through a controller's force feedback mechanism.\n\n"
				"Select and press \"Close Door\" to simulate a door closing in the environment. "
				"This should emit a force feedback reaction felt by all players.\n\n"
				"Select and press \"Shoot Gun\" to simulate firing a weapon. This should emit a "
				"force feedback reaction only for the player who shot the weapon.\n\n"
				"NOTE: A player using a keyboard (Windows) should plug in a gamepad to participate.";
}

bool DemoMotion::Init()
{
	// ###### NOTE ######
	// The Motion plugin and Motion device type (eg. Rumble capable gamepads) have previously
	// been registered with the engine. See IntegrationDemo::InitWwise for details.

	// Load the Motion sound bank
	AkBankID bankID; // not used
	if ( AK::SoundEngine::LoadBank( "Motion.bnk", AK_DEFAULT_POOL_ID, bankID ) != AK_Success )
	{
		SetLoadFileErrorMessage( "Motion.bnk" );
		return false;
	}	

	// Register the "door" game object 
	AK::SoundEngine::RegisterGameObj(GAME_OBJECT_DOOR, "Door");
	
	//The door object is listened by all listeners, so it gives feedback to all players
	AK::SoundEngine::SetListeners(GAME_OBJECT_DOOR, &LISTENER_ID, 1);

#ifndef CONTROLLER_OUTPUT
	//This platform doesn't support a game controller output.  Reroute the bus to the main output.
	AK::SoundEngine::SetBusDevice("Game Pad Bus", "System");
#endif	

	// Initialize the page
	return MultiplayerPage::Init();;
}

void DemoMotion::Release()
{
	// Unregister the "Door" game object	
	AK::SoundEngine::UnregisterGameObj(GAME_OBJECT_DOOR);

	for (int i = 0; i < MAX_PLAYERS && m_bPlayerConnected[i]; i++)
	{
		// Unregister an object for this player
		AK::SoundEngine::UnregisterGameObj(OBJ_FOR_PLAYER[i]);		

		// Register the player's motion device
		AK::SoundEngine::RemoveOutput(m_PlayerMotionOutput[i]);
#ifdef CONTROLLER_OUTPUT
		AK::SoundEngine::RemoveOutput(m_PlayerAudioOutput[i]);
#endif
		
	}

	// Unload the soundbank
	AK::SoundEngine::UnloadBank("Motion.bnk", NULL);

	// Release the Page
	MultiplayerPage::Release();

#ifdef AK_NX
	nn::hid::SetSupportedNpadStyleSet(nn::hid::NpadStyleFullKey::Mask | nn::hid::NpadStyleHandheld::Mask | nn::hid::NpadStyleJoyDual::Mask | nn::hid::NpadStyleJoyLeft::Mask | nn::hid::NpadStyleJoyRight::Mask);	
	nn::hid::MergeSingleJoyAsDualJoy(nn::hid::NpadId::No1, nn::hid::NpadId::No2);	
	nn::hid::SetSupportedNpadStyleSet(nn::hid::NpadStyleFullKey::Mask | nn::hid::NpadStyleHandheld::Mask | nn::hid::NpadStyleJoyDual::Mask);
#endif
}


/////////////////////////////////////////////////////////////////////
// DemoMotion Private Methods
/////////////////////////////////////////////////////////////////////

void DemoMotion::InitControls()
{
	ButtonControl* newBtn;

	// Create the controls
	for ( int i = 0; i < MAX_PLAYERS; i++ )
	{
		// Add player i's Door Slide button
		newBtn = new ButtonControl( *this );
		newBtn->SetLabel( "Close Door" );
		newBtn->SetDelegate( (PageMFP)&DemoMotion::DoorSlide_Pressed );
		m_Controls[i].push_back( newBtn );

		// Add player i's Gun Shot button
		newBtn = new ButtonControl( *this );
		newBtn->SetLabel( "Shoot Gun" );
		newBtn->SetDelegate( (PageMFP)&DemoMotion::GunShot_Pressed );
		m_Controls[i].push_back( newBtn );
	}

#ifdef AK_NX
	// Add player i's Door Slide button
	newBtn = new ButtonControl(*this);
	newBtn->SetLabel("Toggle Dual/Single");
	newBtn->SetDelegate((PageMFP)&DemoMotion::DualSingle_Pressed);
	m_Controls[0].push_back(newBtn);
#endif
}

void DemoMotion::OnPlayerConnect( int in_iPlayerIndex )
{	
	// Register the player's motion device
	AKRESULT res = AK_Fail;
	const UniversalGamepad* gp = m_pParentMenu->Input()->GetGamepad((AkUInt16)in_iPlayerIndex + 1); // UniversalInput is 1-based.
	if (gp && gp->IsConnected())
	{
		char szPlayerName[] = "Player 0";
		szPlayerName[7] = (char)(in_iPlayerIndex + '0');
		AK::SoundEngine::RegisterGameObj(OBJ_FOR_PLAYER[in_iPlayerIndex], szPlayerName);

		//Each player-specific object listens to itself.  We can emit and listen on the same object.
		AK::SoundEngine::SetListeners(OBJ_FOR_PLAYER[in_iPlayerIndex], &OBJ_FOR_PLAYER[in_iPlayerIndex], 1);

		//The door object is listened by all listeners, so it gives feedback to all players
		AK::SoundEngine::AddListener(GAME_OBJECT_DOOR, OBJ_FOR_PLAYER[in_iPlayerIndex]);

#ifndef CONTROLLER_OUTPUT
		//This platform doesn't support a game controller output.  Set the listener of this object to be the main listener
		AK::SoundEngine::AddListener(OBJ_FOR_PLAYER[in_iPlayerIndex], LISTENER_ID);
#endif
		AkOutputSettings outputSettings("Wwise_Motion", gp->GetDeviceID());
		res = AK::SoundEngine::AddOutput(outputSettings, &m_PlayerMotionOutput[in_iPlayerIndex], &OBJ_FOR_PLAYER[in_iPlayerIndex], 1);
	}

	if (res != AK_Success)
		SetErrorMessage("Could not set up the motion device for new player");
	
#ifdef CONTROLLER_OUTPUT
	//On platforms that support a game controller with a speaker, we want to play the GunFire sound on that speaker.  
	//Add an output to do so, we may have up to 4.
	AkOutputSettings outputSettings(CONTROLLER_OUTPUT, GET_PLAYER_ID(in_iPlayerIndex));
	AK::SoundEngine::AddOutput(outputSettings, &m_PlayerAudioOutput[in_iPlayerIndex], &OBJ_FOR_PLAYER[in_iPlayerIndex], 1);
#endif
}

void DemoMotion::OnPlayerDisconnect( int in_iPlayerIndex )
{			
	AK::SoundEngine::UnregisterGameObj(OBJ_FOR_PLAYER[in_iPlayerIndex]);
	AK::SoundEngine::RemoveListener(GAME_OBJECT_DOOR, OBJ_FOR_PLAYER[in_iPlayerIndex]);		
	AK::SoundEngine::RemoveOutput(m_PlayerMotionOutput[in_iPlayerIndex]);		

#ifdef CONTROLLER_OUTPUT
	AK::SoundEngine::RemoveOutput(m_PlayerAudioOutput[in_iPlayerIndex]);
#endif
}

void DemoMotion::DoorSlide_Pressed( void*, ControlEvent* )
{
	AK::SoundEngine::PostEvent( AK::EVENTS::DOORSLIDING, GAME_OBJECT_DOOR );
}

void DemoMotion::GunShot_Pressed( void*, ControlEvent* in_pEvent )
{
	// Fire the gun for the player who shot
	if( in_pEvent->iPlayerIndex >= 1 && in_pEvent->iPlayerIndex <= MAX_PLAYERS )
		AK::SoundEngine::PostEvent( AK::EVENTS::GUNFIRE, OBJ_FOR_PLAYER[in_pEvent->iPlayerIndex - 1] );
}

void DemoMotion::Draw()
{
	MultiplayerPage::Draw();

#ifdef AK_NX
	int iHeight = m_pParentMenu->GetHeight();
	int iWidth = m_pParentMenu->GetWidth();
	int iInstructionsY = iHeight - 4 * GetLineHeight(DrawStyle_Text);
	if(nn::hid::GetNpadJoyAssignment(nn::hid::NpadId::No1) == nn::hid::NpadJoyAssignmentMode_Single)
	{		
		DrawTextOnScreen("Press the <Triggers> on each controller for A & B buttons.", iWidth / 4, iInstructionsY, DrawStyle_Text);
		DrawTextOnScreen("A,B,X,Y replaces the arrows.", iWidth / 4, iInstructionsY + GetLineHeight(DrawStyle_Text), DrawStyle_Text);
	}

	nn::hid::NpadStyleSet style = nn::hid::GetNpadStyleSet(nn::hid::NpadId::Handheld);
	if(style == nn::hid::NpadStyleHandheld::Mask)
	{
		DrawTextOnScreen("Disconnect Joys from console to test dual/single setups", iWidth / 4, iInstructionsY, DrawStyle_Text);
	}
#endif
}

#ifdef AK_NX
void DemoMotion::DualSingle_Pressed(void*, ControlEvent*)
{
	nn::hid::NpadStyleSet style = nn::hid::GetNpadStyleSet(nn::hid::NpadId::Handheld);
	if(style == nn::hid::NpadStyleHandheld::Mask)
		return;

	if(nn::hid::GetNpadJoyAssignment(nn::hid::NpadId::No1) == nn::hid::NpadJoyAssignmentMode_Dual)
	{
		nn::hid::SetSupportedNpadStyleSet(nn::hid::NpadStyleJoyLeft::Mask | nn::hid::NpadStyleJoyRight::Mask);
		nn::hid::SetNpadJoyAssignmentModeSingle(nn::hid::NpadId::No1);
		nn::hid::SetNpadJoyAssignmentModeSingle(nn::hid::NpadId::No2);
	}
	else
	{
		nn::hid::SetSupportedNpadStyleSet(nn::hid::NpadStyleFullKey::Mask | nn::hid::NpadStyleHandheld::Mask | nn::hid::NpadStyleJoyDual::Mask | nn::hid::NpadStyleJoyLeft::Mask | nn::hid::NpadStyleJoyRight::Mask);		
		nn::hid::MergeSingleJoyAsDualJoy(nn::hid::NpadId::No1, nn::hid::NpadId::No2);
		nn::hid::SetSupportedNpadStyleSet(nn::hid::NpadStyleFullKey::Mask | nn::hid::NpadStyleHandheld::Mask | nn::hid::NpadStyleJoyDual::Mask);
		
		//Also remove the controller (this is a particularity of the input system in the IntegrationDemo.  Your application should probably handle connections/disconnections already.		
		OnPlayerDisconnect(0);
		OnPlayerConnect(0);
	}
}
#endif