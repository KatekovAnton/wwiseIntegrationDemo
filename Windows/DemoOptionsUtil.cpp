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

#include "DemoOptionsUtil.h"

#include "IntegrationDemo.h"
#include <vector>
#include <wrl/client.h>

namespace demoOptionsUtil
{
	void PopulateOutputDeviceOptions(std::vector<AkUInt32>& out_deviceIds, AkUInt32& out_activeDeviceIdx, ToggleControl& out_toggle)
	{
		out_activeDeviceIdx = 0;
		out_deviceIds.clear();

		// Because we call AK::SoundEngine::Init with default AkOutputSettings (part of AkInitSettings),
		// the OutputID of the primary output is the "System" shareset mixed with a device ID of 0.
		AkOutputDeviceID activeOutputDeviceId = IntegrationDemo::Instance().GetOutputDeviceId();
		if (activeOutputDeviceId == AK::SoundEngine::GetOutputID(IntegrationDemo::GetDefaultAudioDeviceSharesetId(), IntegrationDemo::GetDefaultAudioDeviceId()))
		{
			AkUInt32 defaultImmDeviceId = 0;
			AK::GetWindowsDevice(-1, defaultImmDeviceId, NULL, AkDeviceState_Active);
			activeOutputDeviceId = AK::SoundEngine::GetOutputID(IntegrationDemo::GetDefaultAudioDeviceSharesetId(), defaultImmDeviceId);
		}

		AkUInt32 immDeviceCount = AK::GetWindowsDeviceCount(AkDeviceState_Active);
		for (AkUInt32 i = 0; i < immDeviceCount; ++i)
		{
			AkUInt32 deviceId = 0;
			AK::GetWindowsDevice(i, deviceId, NULL, AkDeviceState_Active);

			const wchar_t* deviceNameWstr = AK::GetWindowsDeviceName(i, deviceId, AkDeviceState_Active);

			char deviceNameStr[64];
			WideCharToMultiByte(CP_UTF8, 0, deviceNameWstr, -1, deviceNameStr, sizeof(deviceNameStr) / sizeof(deviceNameStr[0]), NULL, NULL);

			out_toggle.AddOption(deviceNameStr);
			out_deviceIds.push_back(deviceId);

			// The DeviceID fetched from an ImmDevice is deterministic for the given hardware (even if the device is unplugged and plugged in).
			// For example, the deviceId can be saved to and reloaded from persistent storage for initialization of the sound engine.
			if (AK::SoundEngine::GetOutputID(IntegrationDemo::GetDefaultAudioDeviceSharesetId(), deviceId) == activeOutputDeviceId)
			{
				out_toggle.SetSelectedIndex(i);
				out_activeDeviceIdx = i;
			}
		}
	}
};