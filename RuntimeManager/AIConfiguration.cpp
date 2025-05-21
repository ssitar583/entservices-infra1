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

#include "AIConfiguration.h"

namespace WPEFramework
{
namespace Plugin
{
    AIConfiguration::AIConfiguration()
    {
    }

    AIConfiguration::~AIConfiguration()
    {
    }

    void AIConfiguration::initialize()
    {
        readFromCustomData();
    }

    size_t AIConfiguration::getContainerConsoleLogCap()
    {
        return mConsoleLogCap;	
    }

    std::bitset<32> AIConfiguration::getAppsCpuSet() const
    {
        return mAppsCpuSet;
    }

    ssize_t AIConfiguration::getNonHomeAppMemoryLimit()
    {
        return mNonHomeAppMemoryLimit;
    }

    ssize_t AIConfiguration::getNonHomeAppGpuLimit()
    {
        return mNonHomeAppGpuLimit;	
    }

    std::list<std::string> AIConfiguration::getVpuAccessBlacklist() const
    {
        return mVpuAccessBlacklist;	    
    }

    std::list<std::string> AIConfiguration::getAppsRequiringDBus() const
    {
        return mAppsRequiringDBus;
    }

    std::list<int> AIConfiguration::getMapiPorts() const
    {
        return mMapiPorts;
    }

    bool AIConfiguration::getResourceManagerClientEnabled() const
    {
        return mResourceManagerClientEnabled;
    }

    bool AIConfiguration::getGstreamerRegistryEnabled()
    {
        return mGstreamerRegistryEnabled;
    }

    bool AIConfiguration::getSvpEnabled()
    {
        return mSvpEnabled;
    }

    bool AIConfiguration::getEnableUsbMassStorage()
    {
        return mEnableUsbMassStorage;
    }

    bool AIConfiguration::getIPv6Enabled()
    {
        return mIPv6Enabled;
    }

    size_t AIConfiguration::getIonHeapDefaultQuota() const
    {
        return mIonHeapDefaultQuota;
    }

    in_port_t AIConfiguration::getDialServerPort() const
    {
        return mDialServerPort;
    }
    
    std::string AIConfiguration::getDialServerPathPrefix() const
    { 
        return mDialServerPathPrefix;
    }
    
    std::string AIConfiguration::getDialUsn() const
    {
        return mDialUsn;
    }

    std::list<std::string> AIConfiguration::getPreloads() const
    {
        return mPreloads;
    }

    std::list<std::string> AIConfiguration::getEnvs() const
    {
        return mEnvs;
    }

    std::map<std::string, size_t> AIConfiguration::getIonHeapQuotas() const
    {
        return mIonHeapQuotas;
    }

    void AIConfiguration::readFromCustomData()
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
        //mPreloads

        //TODO: SUPPORT Dial
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
        mEnvs.push_back("WESTEROS_SINK_AMLOGIC_USE_DMABUF=1");
        mEnvs.push_back("WESTEROS_GL_USE_AMLOGIC_AVSYNC=1");
        mEnvs.push_back("WESTEROS_SINK_USE_FREERUN=1");
        mEnvs.push_back("WESTEROS_GL_MODE=3840x2160x60");
        mEnvs.push_back("WESTEROS_GL_GRAPHICS_MAX_SIZE=1920x1080");
        mEnvs.push_back("WESTEROS_GL_USE_REFRESH_LOCK=1");
    }

    void AIConfiguration::readFromConfigFile()
    {
        //TODO SUPPORT DEVICE_FRIENDLYNAME
        //TODO SUPPORT DEVICE_MODEL_NUM
        //TODO SUPPORT PARTNER_ID
        //TODO SUPPORT REGION=USA,
        //TODO SUPPORT LANG=en_US
        //TODO SUPPORT STB_ENTITLEMENTS
    }
} /* namespace Plugin */
} /* namespace WPEFramework */
