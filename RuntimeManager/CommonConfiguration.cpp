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

#include "CommonConfiguration.h"

namespace WPEFramework
{
namespace Plugin
{
    CommonConfiguration::CommonConfiguration()
    {
    }

    CommonConfiguration::~CommonConfiguration()
    {
    }

    void CommonConfiguration::initialize()
    {
        mConsoleLogCap = 1 * 1024 * 1024; //.apps.limits.consoleLogBytes
        mAppsCpuSet = 0xff; //.apps.limits.cpuset
        mNonHomeAppMemoryLimit = 200 * 1024 * 1024; //.apps.limits.memory.defaultLimitBytes
        mNonHomeAppGpuLimit = 64 * 1024 * 1024; //.apps.limits.memory.defaultLimitBytes
        /* mVpuAccessBlacklist - apps.vpuAccessBlacklist */
        /* mAppsRequiringDBusList - apps.requireDBus */
	/* mMapiPorts = apps.mapi.ports */
        mResourceManagerClientEnabled = false; // .apps.essosResourceManager.enableClient
        mGstreamerRegistryEnabled = false; // apps.gstreamer.mapCachedRegistry
        mSvpEnabled = false; // apps.svp.enable
        mEnableUsbMassStorage = false; // apps.usbMassStorage.enable
        mIPv6Enabled = false; // apps.enableIPv6
        mIonHeapDefaultQuota = 256 * 1024 * 1024; // apps.limits.ion.defaultLimitBytes
        mDialServerPort = 8009; //.dial.server.port;
        mDialServerPathPrefix = ""; // dial.server.prefix
        mDialUsn = ""; //.dial.usn
        //TODO: GAP Dial
	/*
        if (mDialServerPathPrefix.empty())
        {
            auto dialUuid = AICommon::Uuid::createUuid();
            mDialServerPathPrefix = dialUuid.toString();
        }
        if (mDialUsn.empty())
        {
            AICommon::DthMacAddressProvider macAddressProvider;
            mDialUsn = AICommon::getDeviceDialUsn(macAddressProvider.getMac());
        }
	*/
    }

    size_t CommonConfiguration::getContainerConsoleLogCap()
    {
        return mConsoleLogCap;	
    }

    std::bitset<32> CommonConfiguration::getAppsCpuSet() const
    {
        return mAppsCpuSet;
    }

    ssize_t CommonConfiguration::getNonHomeAppMemoryLimit()
    {
        return mNonHomeAppMemoryLimit;
    }

    ssize_t CommonConfiguration::getNonHomeAppGpuLimit()
    {
        return mNonHomeAppGpuLimit;	
    }

    std::list<std::string> CommonConfiguration::getVpuAccessBlacklist() const
    {
        return mVpuAccessBlacklist;	    
    }

    std::list<std::string> CommonConfiguration::getAppsRequiringDBus() const
    {
        return mAppsRequiringDBus;
    }

    std::list<int> CommonConfiguration::getMapiPorts() const
    {
        return mMapiPorts;
    }

    bool CommonConfiguration::getResourceManagerClientEnabled() const
    {
        return mResourceManagerClientEnabled;
    }

    bool CommonConfiguration::getGstreamerRegistryEnabled()
    {
        return mGstreamerRegistryEnabled;
    }

    bool CommonConfiguration::getSvpEnabled()
    {
        return mSvpEnabled;
    }

    bool CommonConfiguration::getEnableUsbMassStorage()
    {
        return mEnableUsbMassStorage;
    }

    bool CommonConfiguration::getIPv6Enabled()
    {
        return mIPv6Enabled;
    }

    size_t CommonConfiguration::getIonHeapDefaultQuota() const
    {
        return mIonHeapDefaultQuota;
    }

    in_port_t CommonConfiguration::getDialServerPort() const
    {
        return mDialServerPort;
    }
    
    std::string CommonConfiguration::getDialServerPathPrefix() const
    { 
        return mDialServerPathPrefix;
    }
    
    std::string CommonConfiguration::getDialUsn() const
    {
        return mDialUsn;
    }

} /* namespace Plugin */
} /* namespace WPEFramework */
