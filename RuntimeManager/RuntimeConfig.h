/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2025 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

#pragma once

#include <string>
#include <vector>

namespace WPEFramework
{
namespace Plugin
{
    enum ApplicationType
    {
        INTERACTIVE = 0,
	SYSTEM = 1 
    };

    struct RuntimeConfig
    {
        RuntimeConfig(): systemMemoryLimit(0), gpuMemoryLimit(0), command("/runtime/SkyBrowserLauncher"), appType(INTERACTIVE), appPath("/opt/youtube/YouTube.T18IAl"), runtimePath("/opt/youtube/com.sky.cobalt.Hn7UUm"), wanLanAccess(true), thunder(true), dial(true), resourceManagerClientEnabled(true), dialId(""), envVariables() {}
        ssize_t systemMemoryLimit;
        ssize_t gpuMemoryLimit;
        string command;
        ApplicationType appType;
        string appPath;
        string runtimePath;
        bool wanLanAccess;
        bool thunder;
        bool dial;
        bool resourceManagerClientEnabled;
        string dialId;
        std::vector<std::string> envVariables;
    };

} /* namespace Plugin */
} /* namespace WPEFramework */
