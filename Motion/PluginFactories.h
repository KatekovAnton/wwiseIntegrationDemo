/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the
"Apache License"); you may not use this file except in compliance with the
Apache License. You may obtain a copy of the Apache License at
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

Version: v2019.1.1  Build: 6977
Copyright (c) 2006-2019 Audiokinetic Inc.
*******************************************************************************/

#ifndef _AK_PLUGINSFACTORIES_H_
#define _AK_PLUGINSFACTORIES_H_

#include <AK/AkPlatforms.h>
#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/IAkPlugin.h>

#include <AK/Plugin/AkMotionGeneratorSourceFactory.h>
#include <AK/Plugin/AkMotionSourceSourceFactory.h>
#if (defined AK_WIN || defined AK_PS4 || defined AK_XBOXONE || defined AK_NX || (defined AK_ANDROID && !defined AK_LUMIN)) 
	#include <AK/Plugin/AkMotionSinkFactory.h>
#endif
#endif // _AK_PLUGINSFACTORIES_H_