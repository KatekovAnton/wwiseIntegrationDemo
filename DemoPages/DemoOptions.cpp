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
#include "Platform.h"
#include "DemoOptions.h"
#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine

#include "IntegrationDemo.h"

#if (defined(AK_WIN) && !defined(AK_USE_UWP_API)) || defined(AK_PS4) || defined(AK_XBOXONE)
#include "DemoOptionsUtil.h"
#endif

// SpatialAudio configuration is disabled in IntegrationDemo because the related plug-ins are optional,
// and may not be present. If you do have the plug-in contents and want to try it, include the relevant
// plug-in factory, and link the relevant lib.
#define SPATIAL_AUDIO_PLUGIN_AVAILABLE 0

/// DemoOptions class constructor
DemoOptions::DemoOptions( Menu& in_ParentMenu ) : Page( in_ParentMenu, "Options Menu" )
	, m_activeDeviceIdx(0)
	, m_activePanningRule(AkPanningRule_Speakers)
	, m_activeChannelConfig(0)
	, m_spatialAudioRequested(false)
	, m_spatialAudioDeviceAvailable(false)
{
	m_szHelp = "This page provides options to configure the sound engine, the changes of which are reflected in the other demos.";
}

/// Initializes the demo.
/// \return True if successful and False otherwise.
bool DemoOptions::Init()
{
	// Initialize the page
	if (Page::Init())
	{
		updateOutputDevice();
		return true;
	}
	return false;
}

/// Releases resources used by the demo.
void DemoOptions::Release()
{
	Page::Release();
}

void DemoOptions::InitControls()
{
	ToggleControl* newToggle = NULL;

	newToggle = new ToggleControl(*this);
	newToggle->SetLabel("Device:");
#if (defined(AK_WIN) && !defined(AK_USE_UWP_API)) || defined(AK_PS4) || defined(AK_XBOXONE)
	demoOptionsUtil::PopulateOutputDeviceOptions(m_deviceIds, m_activeDeviceIdx, *newToggle);
#else
	m_deviceIds.push_back(0);
	newToggle->AddOption("Primary Output");
#endif
	newToggle->SetDelegate((PageMFP)&DemoOptions::outputDeviceOption_Changed);
	m_Controls.push_back(newToggle);

	// Note that despite providing these for all platforms, not all platforms support
	// all of these options equally. For example, Spatial Audio will be requested,
	// but unless the platform/device supports it (see: AK::SoundEngine::GetDeviceSpatialAudioSupport)
	// it will be ignored. Also, Speaker Configuration is handled automatically on most
	// platforms, and will be ignored.

#if SPATIAL_AUDIO_PLUGIN_AVAILABLE
	newToggle = new ToggleControl(*this);
	newToggle->SetLabel("Spatial Audio:");
	newToggle->AddOption("Disabled", (void*)false);
	newToggle->AddOption("Enabled", (void*)true);
	newToggle->SetDelegate((PageMFP)&DemoOptions::spatialAudioOption_Changed);
	m_Controls.push_back(newToggle);
#endif

	newToggle = new ToggleControl(*this);
	newToggle->SetLabel("Speaker Positioning:");
	newToggle->AddOption("Speakers", (void*)AkPanningRule_Speakers);
	newToggle->AddOption("Headphones", (void*)AkPanningRule_Headphones);
	newToggle->SetDelegate((PageMFP)&DemoOptions::speakerPosOption_Changed);
	m_Controls.push_back(newToggle);

	newToggle = new ToggleControl(*this);
	newToggle->SetLabel("Speaker Configuration:");
	newToggle->AddOption("Automatic", (void*)0);
#if defined(AK_NX) // Supported speaker configurations for NX are only 2.0 stereo or 5.1 surround
	newToggle->AddOption("Stereo", (void*)AK_SPEAKER_SETUP_STEREO);
	newToggle->AddOption("5.1 Surround", (void*)AK_SPEAKER_SETUP_5POINT1);
#else
	newToggle->AddOption("Monaural", (void*)AK_SPEAKER_SETUP_MONO);
	newToggle->AddOption("Stereo", (void*)AK_SPEAKER_SETUP_STEREO);
	newToggle->AddOption("5.1 Surround", (void*)AK_SPEAKER_SETUP_5POINT1);
	newToggle->AddOption("7.1 Surround", (void*)AK_SPEAKER_SETUP_7POINT1);
#endif
	newToggle->SetDelegate((PageMFP)&DemoOptions::speakerConfigOption_Changed);
	m_Controls.push_back(newToggle);
}

void DemoOptions::outputDeviceOption_Changed(void* in_pSender, ControlEvent* in_pControlEvent)
{
	ToggleControl& toggleControl = *((ToggleControl*)in_pSender);
	m_activeDeviceIdx = toggleControl.SelectedIndex();
	updateOutputDevice();
}

void DemoOptions::spatialAudioOption_Changed(void* in_pSender, ControlEvent* in_pControlEvent)
{
	ToggleControl& toggleControl = *((ToggleControl*)in_pSender);
	m_spatialAudioRequested = ((uintptr_t)toggleControl.SelectedValue()) != 0;
	updateOutputDevice();
}

void DemoOptions::speakerPosOption_Changed(void* in_pSender, ControlEvent* in_pControlEvent)
{
	ToggleControl& toggleControl = *((ToggleControl*)in_pSender);
	m_activePanningRule = (AkPanningRule)((uintptr_t)toggleControl.SelectedValue());
	updateOutputDevice();
}

void DemoOptions::speakerConfigOption_Changed(void* in_pSender, ControlEvent* in_pControlEvent)
{
	ToggleControl& toggleControl = *((ToggleControl*)in_pSender);
	m_activeChannelConfig = (AkUInt32)((uintptr_t)toggleControl.SelectedValue());
	updateOutputDevice();
}

void DemoOptions::updateOutputDevice()
{
	AkUInt32 newDeviceId = m_activeDeviceIdx < m_deviceIds.size() ? m_deviceIds[m_activeDeviceIdx] : 0;
	AkUInt32 newChannelConfig = m_activeChannelConfig;

	m_spatialAudioDeviceAvailable = (AK::SoundEngine::GetDeviceSpatialAudioSupport(newDeviceId) == AK_Success);

	AkOutputSettings newSettings;
	newSettings.audioDeviceShareset = (m_spatialAudioRequested && m_spatialAudioDeviceAvailable) ?
		IntegrationDemo::GetSpatialAudioSharesetId() : IntegrationDemo::GetDefaultAudioDeviceSharesetId();
#if defined(AK_PS4)
	// for PS4, we need to specify the "Pad_Output" device shareset if we want to output to controller speakers
	// ("Controller_Headphones" is also a supported shareset)
	if (newDeviceId != 0)
	{
		newSettings.audioDeviceShareset = AK::SoundEngine::GetIDFromString("Pad_Output");
		// clear the desired channel config - Pad_Output does support channel configurations except for AK_SPEAKER_SETUP_MONO
		newChannelConfig = 0;
	}
#endif

	newSettings.idDevice = newDeviceId;
	newSettings.ePanningRule = m_activePanningRule;
	if (newChannelConfig)
	{
		newSettings.channelConfig.SetStandard(newChannelConfig);
	}
	else
	{
		newSettings.channelConfig.Clear();
	}

	AK::SoundEngine::ReplaceOutput(newSettings,
		IntegrationDemo::Instance().GetOutputDeviceId(),
		&IntegrationDemo::Instance().GetOutputDeviceIdRef());
}

void DemoOptions::Draw()
{
	Page::Draw();
	DrawTextOnScreen(m_szHelp.c_str(), 70, 300, DrawStyle_Text);
#if SPATIAL_AUDIO_PLUGIN_AVAILABLE
	if (m_spatialAudioDeviceAvailable)
	{
		DrawTextOnScreen("Spatial Audio is available for the selected device", 70, 400, DrawStyle_Text);
	}
	else
	{
		DrawTextOnScreen("Spatial Audio is not available for the selected device", 70, 400, DrawStyle_Text);
	}
#endif
}



